#include "redobbbuilder.h"
#include "slicm.h"

using namespace ucw;

/// Create check flag for `LD`, which is set to true if
/// the memory at `LD.getOperand(0)` is changed
/// returns the created flag address value
Value *RedoBBBuilder::createCheckFlag(LoadInst &LD)
{
    // reuse flag for the same memory address
    Value *flag = LdToFlagMap[&LD];
    if (flag) {
        DEBUG(dbgs() << "    Reuse existing flag.\n");
    } else {
        // create flag variable
        BasicBlock *prepre = pass->getOrCreatePrePreheader();
        BasicBlock *postPre = pass->getOrCreatePostPreheader();
        flag = new AllocaInst(Type::getInt1Ty(LD.getContext()),
                              LD.getName() + ".flag",
                              prepre->getTerminator());

        new StoreInst(ConstantInt::getFalse(LD.getContext()),
                    flag, postPre->getTerminator());

        LdToFlagMap[&LD] = flag;
    }

    DEBUG(dbgs() << "        Created check flag '" << flag->getName() << "' for (" << LD << "  )\n");

    // insert check to potential stores
    // insert check to all potential alias address users
    for (auto pointerRec : pass->getAliasSetForLoadSrc(&LD)) {
        insertCheck(pointerRec.getValue(), LD.getOperand(0), flag);
    }

    return flag;
}

/// check if value in memory at `memAddr` was changed when accessing `val`, store result into flag
void RedoBBBuilder::insertCheck(Value *val, Value *memAddr, Value* flag)
{
    for (auto it = val->use_begin(), ite = val->use_end(); it != ite; it++) {
        if (StoreInst *SI = dyn_cast<StoreInst>(*it)) {
            // skip those not in current top loop
            if (!isCurrentTopLoop(*SI)) { continue; }

            // skip instruction we don't interested in
            if (!shouldCheck(*SI)) { continue; }

            // if we have checked this store for this memAddr
            CmpInst *chkRes = 0;
            if (StoreToCheckMap.count(SI)) {
                for (auto pair : StoreToCheckMap[SI]) {
                    if (pair.first == memAddr) {
                        chkRes = pair.second;
                        DEBUG(dbgs() << "        Found existing check '" << chkRes->getName()
                                    << "' for (" << *SI << "  ) in "
                                    << SI->getParent()->getName() << "\n");
                    }
                }
            }
            if (!chkRes) {
                // check if before and after the store, the memore address changed
                //
                // %orig        = load %memAddr
                // the STORE we checking
                // %modified    = load %memAddr
                // %cmp         = icmp ne, %orig, %modified
                LoadInst *orig = new LoadInst(memAddr, "", SI);

                Instruction *next = SI->getNextNode();
                LoadInst *modified = new LoadInst(memAddr, "", next);
                chkRes = CmpInst::Create(Instruction::ICmp,
                                         CmpInst::ICMP_NE,
                                         orig, modified,
                                         "chk", next);

                CheckingInstrs.insert(orig);
                CheckingInstrs.insert(modified);
                CheckingInstrs.insert(chkRes);

                // set consitant name
                orig->setName(chkRes->getName() + ".orig");
                modified->setName(chkRes->getName() + ".mod");

                StoreToCheckMap[SI].push_back({memAddr, chkRes});

                DEBUG(dbgs() << "        Inserted check '" << chkRes->getName()
                            << "' for (" << *SI << "  ) in "
                            << SI->getParent()->getName() << "\n");
            }


            // if we have stored the check result to the flag
            Check pair = {memAddr, chkRes};
            for (auto f : CheckToFlagMap[pair]) {
                if (f == flag) {
                    DEBUG(dbgs() << "        Existing flag store found\n");
                    return;
                }
            }

            DEBUG(dbgs() << "            Check result stored to " << flag->getName() << "\n");
            // merge old value and new value with or
            //
            // %oldflgval   = load %flag
            // %newflgval   = or %oldflgval, %chkRes
            // store  %newflgval, %flag
            // the next instr after STORE we checking
            Instruction *next = chkRes->getNextNode();
            LoadInst *oldflgval = new LoadInst(flag, flag->getName() + ".oldval", next);
            auto *newflgval = BinaryOperator::Create(Instruction::Or,
                                                     oldflgval, chkRes,
                                                     flag->getName() + ".newval",
                                                     next);
            StoreInst *st = new StoreInst(newflgval, flag, next);
            CheckingInstrs.insert(oldflgval);
            CheckingInstrs.insert(newflgval);
            CheckingInstrs.insert(st);

            CheckToFlagMap[pair].push_back(flag);
        }
    }
}

/// Create redo/rest BB struction at LoadInst `I`
/// check flag is created if necessary
BasicBlock *RedoBBBuilder::CreateRedoBB(LoadInst &I)
{
    assert(LdToRedoBB[&I] == 0 && "CreateRedoBB should only be called once on each LoadInst");

    Value *flag = createCheckFlag(I);

    // build home/rest/redo structure first
    BasicBlock *homeBB = I.getParent();
    BasicBlock *redoBB = SplitBlock(homeBB, &I, pass);
    redoBB->setName(homeBB->getName() + ".redo");
    LdToRedoBB[&I] = redoBB;

    assert(isCurrentTopLoop(redoBB) && "LoopInfo should be updated");

    BasicBlock *restBB = SplitBlock(redoBB, I.getNextNode(), pass);
    restBB->setName(homeBB->getName() + ".rest");

    homeBB->getTerminator()->eraseFromParent();
    // load flag into register, link homeBB to redo and rest
    LoadInst *reg = new LoadInst(flag, "", homeBB);
    CheckingInstrs.insert(reg);

    BranchInst::Create(redoBB, restBB, reg, homeBB);

    // reset flag in redoBB
    new StoreInst(ConstantInt::getFalse(I.getContext()),
                  flag, redoBB->getTerminator());

    // add load itself to redoBB
    AddToRedoBB(&I, &I);

    return redoBB;
}

/// Add instruction `Inst` to `LD`'s redoBB
void RedoBBBuilder::AddToRedoBB(Instruction *Inst, LoadInst *LD)
{
    BasicBlock *redoBB = LdToRedoBB[LD];
    assert(redoBB && "Create redoBB first!");

    DEBUG(dbgs() << "        Adding '" << Inst->getName() << "' to redoBB '" << redoBB->getName()
                << "', belongs to '" << LD->getName() << "'\n");

    auto newInst = Inst->clone();
    if (!Inst->getName().empty()) {
        newInst->setName(Inst->getName() + ".redo");
    }

    // store output to stack variable
    Value *var = createStackValue(Inst);

    // add to redoBB
    Instruction *end = redoBB->getTerminator()->getPrevNode();
    assert(isa<StoreInst>(end) && "Malformed redoBB");

    newInst->insertBefore(end);
    new StoreInst(newInst, var, end);

    DEBUG(dbgs() << "            Added as (" << *newInst << "  )\n");
}

/// Create a stack value to store `Inst`'s output
Value *RedoBBBuilder::createStackValue(Instruction *Inst)
{
    if (Value *var = InstToStvarMap[Inst]) {
        DEBUG(dbgs() << "            Reuse existing stack value '" << var->getName() << "'\n");
        return var;
    }

    BasicBlock *prepre = pass->getOrCreatePrePreheader();
    BasicBlock *postpre = pass->getOrCreatePostPreheader();

    AllocaInst *var = new AllocaInst(Inst->getType(),
                                        Inst->getName() + ".var",
                                        prepre->getTerminator());
    new StoreInst(Inst, var, postpre->getTerminator());
    InstToStvarMap[Inst] = var;

    DEBUG(dbgs() << "            Created stack value '" << var->getName() << "' for (" << *Inst << "  )\n");

    return var;
}

/// Patch consumers of each Instruction added in redoBB to use their stack value instead
void RedoBBBuilder::PatchOutputs()
{
    for (auto pair : InstToStvarMap) {
        patchOutputFor(pair.first, pair.second);
    }
}

/// Change all consumers of `I` to use stack variable `var` instead
void RedoBBBuilder::patchOutputFor(Instruction *I, Value *var)
{
    // must build the list first, because we are changing it
    SmallVector<Instruction*, 8> usages;
    for (auto it = I->use_begin(), ite = I->use_end(); it != ite; it++) {
        if (Instruction *userInst = dyn_cast<Instruction>(*it)) {
            if (userInst->getParent() == pass->Preheader
                || userInst->getParent() == pass->getOrCreatePostPreheader()) {
                continue;
            }
            usages.push_back(userInst);
        }
    }
    for (auto userInst : usages) {
        DEBUG(dbgs() << "    Change (" << *userInst << "  ) to use stack variable '"
                    << var->getName() <<"'\n");

        LoadInst *ld = new LoadInst(var, "", userInst);
        userInst->replaceUsesOfWith(I, ld);

        // Mark ld as unable for hoist
        CheckingInstrs.insert(ld);
    }
}

bool RedoBBBuilder::IsRedoCode(Instruction& I) const
{
    auto BB = I.getParent();
    return IsRedoBB(BB) || pass->isPrePre(BB) || pass->isPostPre(BB);
}

bool RedoBBBuilder::isCurrentTopLoop(Instruction& I) const
{
    return isCurrentTopLoop(I.getParent());
}

bool RedoBBBuilder::isCurrentTopLoop(BasicBlock* BB) const
{
    return pass->CurLoop->contains(BB) && !pass->inSubLoop(BB);
}
