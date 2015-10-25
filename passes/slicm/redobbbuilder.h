#ifndef REDOBBBUILDER_H
#define REDOBBBUILDER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <algorithm>

using namespace llvm;

namespace ucw{
struct SLICM;
class RedoBBBuilder
{
    typedef std::pair<Instruction*, Value*> InstrPair;
    struct first_eq_with
    {
        first_eq_with(Instruction *other) : mine(other) { }
        bool operator ()(const InstrPair &obj) const { return obj.first == mine; }
        Instruction *mine;
    };

    SLICM *pass;

    DenseMap<Value *, Value *> MemAddrToFlagMap;
    DenseMap<Instruction *, Value *> InstToStvarMap;
    DenseMap<LoadInst*, BasicBlock *> LdToRedoBB;
    DenseMap<StoreInst *, SmallVector<Value *, 2>> StoreCheckedByFlagMap;
    DenseSet<Instruction *> CheckingInstrs;

public:
    RedoBBBuilder(SLICM *pass) : pass(pass) { }

    /// Create redo/rest BB struction at LoadInst `I`
    /// check flag is created if necessary
    BasicBlock *CreateRedoBB(LoadInst& I);

    /// Add instruction `Inst` to `LD`'s redoBB
    void AddToRedoBB(Instruction *Inst, LoadInst *LD);

    /// Patch consumers of each Instruction added in redoBB to use their stack value instead
    void PatchOutputs();

    bool IsRedoCode(Instruction &I) const;

    bool ShouldIgnoreForHoist(Instruction &I) const
    {
        return IsRedoCode(I) || isCheckingCode(I);
    }

private:
    /// Create check flag for `LD`, which is set to true if
    /// the memory at `LD.getOperand(0)` is changed
    Value *createCheckFlag(LoadInst &I);

    /// check if value in memory at `memAddr` was changed when accessing `val`
    void insertCheck(Value* memAddr, Value* val);

    /// Create a stack value to store `Inst`'s output
    Value *createStackValue(Instruction *Inst);

    /// Change all consumers of `I` to use stack variable `var` instead
    void patchOutputFor(Instruction *I, Value *var);

    bool isCurrentTopLoop(Instruction &I) const;
    bool isCurrentTopLoop(BasicBlock *BB) const;

    // should we check this instruction
    bool shouldCheck(Instruction &I) const
    {
        return isa<StoreInst>(I) && !isCheckingCode(I) && !IsRedoCode(I);
    }

    bool isCheckingCode(Instruction &I) const
    {
        return CheckingInstrs.count(&I) > 0;
    }

public:
    static bool IsRedoBB(BasicBlock *BB)
    {
        return BB->getName().endswith(".redo");
    }
};

}

#endif // REDOBBBUILDER_H
