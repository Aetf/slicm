#include "redobbbuilder.h"
#include "slicm.h"

using namespace ucw;

/// Create check flag for `LD`, which is set to true if
/// the memory at `LD.getOperand(0)` is changed
/// returns the created flag address value
Value *RedoBBBuilder::createCheckFlag(LoadInst &LD)
{
    DEBUG(dbgs() << "Creating check flag for :" << LD << "\n");

    // reuse flag for the same memory address
    Value *memAddr = LD.getOperand(0);
    Value *flag = MemAddrToFlagMap[memAddr];
    if (flag) {
        DEBUG(dbgs() << "    reuse existing flag.\n");
    } else {
        // create flag variable
        BasicBlock *prepre = pass->getOrCreatePrePreheader();
        BasicBlock *postPre = pass->getOrCreatePostPreheader();
        flag = new AllocaInst(Type::getInt1Ty(LD.getContext()),
                              LD.getName() + ".flag",
                              prepre->getTerminator());

        new StoreInst(ConstantInt::getFalse(LD.getContext()),
                    flag, postPre->getTerminator());

        MemAddrToFlagMap[memAddr] = flag;
    }

    // insert check to potential stores
    // insert check to all potential alias address users
    for (auto pointerRec : pass->getAliasSetForLoadSrc(&LD)) {
        insertCheck(memAddr, pointerRec.getValue());
    }

    return flag;
}

/// check if value in memory at `memAddr` was changed when accessing `val`
void RedoBBBuilder::insertCheck(Value *memAddr, Value* val)
{
    Value *flag = MemAddrToFlagMap[memAddr];
    assert(flag  && "Must call CreateCheckFlag before insertCheck");

    for (auto it = val->use_begin(), ite = val->use_end(); it != ite; it++) {
        if (StoreInst *SI = dyn_cast<StoreInst>(*it)) {
            // skip those not in current top loop
            if (!isCurrentTopLoop(*SI)) { continue; }

            // skip instruction we don't interested in
            if (!shouldCheck(*SI)) { continue; }

            // skip those we have checked
            if (flag == StoreCheckedByFlagMap[SI]) { continue; }

            DEBUG(dbgs() << "Inserting check for (" << *SI << "  ) in "
                        << SI->getParent()->getName() << "\n");

            // check if after the store, the memore address changed
            // %orig        = load %memAddr
            // the STORE we checking
            // %modified    = load %memAddr
            // %cmp         = icmp ne, %orig, %modified
            // %oldflgval   = load %flag
            // %newflgval   = or %oldflgval, %cmp
            // store  %newflgval, %flag
            // next instr after STORE we checking
            LoadInst *orig = new LoadInst(memAddr, "", SI);

            Instruction *next = SI->getNextNode();
            LoadInst *modified = new LoadInst(memAddr, "", next);
            CmpInst *cmp = CmpInst::Create(Instruction::ICmp,
                                           CmpInst::ICMP_NE,
                                           orig, modified,
                                           "", next);
            LoadInst *oldflgval = new LoadInst(flag, "", next);
            BinaryOperator *newflgval = BinaryOperator::Create(Instruction::Or,
                                                               oldflgval, cmp,
                                                               "", next);
            StoreInst *st = new StoreInst(newflgval, flag, next);
            StoreCheckedByFlagMap[SI] = flag;

            CheckingInstrs.insert(orig);
            CheckingInstrs.insert(modified);
            CheckingInstrs.insert(cmp);
            CheckingInstrs.insert(oldflgval);
            CheckingInstrs.insert(newflgval);
            CheckingInstrs.insert(st);
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

    return redoBB;
}

/// Add instruction `Inst` to `LD`'s redoBB
void RedoBBBuilder::AddToRedoBB(Instruction *Inst, LoadInst *LD)
{
    BasicBlock *redoBB = LdToRedoBB[LD];
    assert(redoBB && "Create redoBB first!");

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

    DEBUG(dbgs() << "Cloned (" << *Inst << ") as (" << *newInst << ") to redoBB\n");
}

/// Create a stack value to store `Inst`'s output
Value *RedoBBBuilder::createStackValue(Instruction *Inst)
{
    if (Value *var = InstToStvarMap[Inst]) {
        DEBUG(dbgs() << "Reuse existing stack value");
        return var;
    }

    DEBUG(dbgs() << "Creating stack value for (" << *Inst << ")\n");
    BasicBlock *prepre = pass->getOrCreatePrePreheader();
    BasicBlock *postpre = pass->getOrCreatePostPreheader();

    AllocaInst *var = new AllocaInst(Inst->getType(),
                                        Inst->getName() + ".var",
                                        prepre->getTerminator());
    new StoreInst(Inst, var, postpre->getTerminator());
    InstToStvarMap[Inst] = var;
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
    DEBUG(dbgs() << "Patching instructions' output\n");
    for (auto it = I->use_begin(), ite = I->use_end(); it != ite; it++) {
        if (Instruction *userInst = dyn_cast<Instruction>(*it)) {
            if (userInst->getParent() == pass->Preheader
                || userInst->getParent() == pass->getOrCreatePostPreheader()) {
                continue;
            }
            LoadInst *ld = new LoadInst(var, "", userInst);
            userInst->replaceUsesOfWith(I, ld);

            // Mark ld as unable for hoist
            CheckingInstrs.insert(ld);
        }
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
