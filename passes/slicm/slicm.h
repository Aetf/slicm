#ifndef SLICM_H
#define SLICM_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Target/TargetLibraryInfo.h"

using namespace llvm;

namespace ucw {
class RedoBBBuilder;
struct SLICM : public LoopPass
{
    static char ID; // Pass identification, replacement for typeid
    SLICM() : LoopPass(ID)
    {
        // initializeSLICMPass(*PassRegistry::getPassRegistry()); // 583 - commented out
    }

    virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

    /// This transformation requires natural loop information & requires that
    /// loop preheaders be inserted into the CFG...
    ///
    virtual void getAnalysisUsage(AnalysisUsage &AU) const
    {
        // AU.setPreservesCFG();                   // 583 - commented out
        AU.addRequired<DominatorTree>();
        AU.addRequired<LoopInfo>();
        // AU.addRequiredID(LoopSimplifyID);       // 583 - commented out
        AU.addRequired<AliasAnalysis>();
        // AU.addPreserved<AliasAnalysis>();       // 583 - commented out
        // AU.addPreserved("scalar-evolution");    // 583 - commented out
        // AU.addPreservedID(LoopSimplifyID);      // 583 - commented out
        AU.addRequired<TargetLibraryInfo>();
        AU.addRequired<ProfileInfo>();
        //AU.addRequired<LAMPLoadProfile>();
    }

    using llvm::Pass::doFinalization;

    bool doFinalization()
    {
        assert(LoopToAliasSetMap.empty() && "Didn't free loop alias sets");
        return false;
    }

private:
    AliasAnalysis *AA;       // Current AliasAnalysis information
    LoopInfo      *LI;       // Current LoopInfo
    DominatorTree *DT;       // Dominator Tree for the current Loop.

    DataLayout *TD;          // DataLayout for constant folding.
    TargetLibraryInfo *TLI;  // TargetLibraryInfo for constant folding.

    // State that is updated as we process loops.
    bool Changed;            // Set to true when we change anything.
    BasicBlock *Preheader;   // The preheader block of the current loop...
    BasicBlock *PostPreheader;  // The block after preheader...
    BasicBlock *PrePreheader; // The block before preheader...
    Loop *CurLoop;           // The current loop we are working on...
    AliasSetTracker *CurAST; // AliasSet information for the current loop...
    bool MayThrow;           // The current loop contains an instruction which
                             // may throw, thus preventing code motion of
                             // instructions with side effects.
    DenseMap<Loop *, AliasSetTracker *> LoopToAliasSetMap;
    DenseMap<Instruction *, SmallVector<LoadInst*, 2>> SpeculateHoisted;

    // Helper class for speculative hoist
    RedoBBBuilder *RBBB;

    void ClearState();

    /// cloneBasicBlockAnalysis - Simple Analysis hook. Clone alias set info.
    void cloneBasicBlockAnalysis(BasicBlock *From, BasicBlock *To, Loop *L);

    /// deleteAnalysisValue - Simple Analysis hook. Delete value V from alias
    /// set.
    void deleteAnalysisValue(Value *V, Loop *L);

    /// SinkRegion - Walk the specified region of the CFG (defined by all blocks
    /// dominated by the specified block, and that are in the current loop) in
    /// reverse depth first order w.r.t the DominatorTree.  This allows us to
    /// visit uses before definitions, allowing us to sink a loop body in one
    /// pass without iteration.
    ///
    void SinkRegion(DomTreeNode *N);

    /// HoistRegion - Walk the specified region of the CFG (defined by all
    /// blocks dominated by the specified block, and that are in the current
    /// loop) in depth first order w.r.t the DominatorTree.  This allows us to
    /// visit definitions before uses, allowing us to hoist a loop body in one
    /// pass without iteration.
    ///
    void HoistRegion(DomTreeNode *N);

    /// inSubLoop - Little predicate that returns true if the specified basic
    /// block is in a subloop of the current one, not the current one itself.
    ///
    bool inSubLoop(BasicBlock *BB)
    {
        assert(CurLoop->contains(BB) && "Only valid if BB is IN the loop");
        return LI->getLoopFor(BB) != CurLoop;
    }

    /// sink - When an instruction is found to only be used outside of the loop,
    /// this function moves it to the exit blocks and patches up SSA form as
    /// needed.
    ///
    void sink(Instruction &I);

    /// hoist - When an instruction is found to only use loop invariant operands
    /// that is safe to hoist, this instruction is called to do the dirty work.
    ///
    void hoist(Instruction &I);

    /// speculativeHoist - When an instruction is found unsafe to hoist, but the program
    /// correctness can be kept with fix up code, hoist it.
    ///
    void speculativeHoist(Instruction &I);

    /// isSafeToExecuteUnconditionally - Only sink or hoist an instruction if it
    /// is not a trapping instruction or if it is a trapping instruction and is
    /// guaranteed to execute.
    ///
    bool isSafeToExecuteUnconditionally(Instruction &I);

    /// isGuaranteedToExecute - Check that the instruction is guaranteed to
    /// execute.
    ///
    bool isGuaranteedToExecute(Instruction &I);

    /// pointerInvalidatedByLoop - Return true if the body of this loop may
    /// store into the memory location pointed to by V.
    ///
    bool pointerInvalidatedByLoop(Value *V, uint64_t Size,
                                  const MDNode *TBAAInfo)
    {
        // Check to see if any of the basic blocks in CurLoop invalidate *V.
        return CurAST->getAliasSetForPointer(V, Size, TBAAInfo).isMod();
    }

    AliasSet &getAliasSetForLoadSrc(LoadInst* LI);

    BasicBlock *getOrCreatePostPreheader();
    BasicBlock *getOrCreatePrePreheader();
    bool canSinkOrHoistInst(Instruction& I, bool* speculative = 0);
    bool isHoistableInstr(Instruction &I);
    bool canSpeculativeHoist(LoadInst& I);
    bool isNotUsedInLoop(Instruction &I);
    bool hasLoopInvariantOperands(Instruction &I);
    void maybeResultOfSpeculativeHoist(Instruction &I);

    bool isPrePre(BasicBlock *BB)
    {
        return BB->getName().endswith(".prepre");
    }

    bool isPostPre(BasicBlock *BB)
    {
        return BB->getName().endswith(".postpre");
    }

    void PromoteAliasSet(AliasSet &AS,
                         SmallVectorImpl<BasicBlock *> &ExitBlocks,
                         SmallVectorImpl<Instruction *> &InsertPts);
    friend class RedoBBBuilder;
};
}

#endif // SLICM_H
