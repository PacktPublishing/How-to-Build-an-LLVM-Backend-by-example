//===-- simpleConstantPropagation.cpp - Simple cst prop opt -------*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a simple constant propagation pass and hooks it up in
// both the legacy and new pass managers.
//
//===----------------------------------------------------------------------===//

#include "H2BLB.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/PostOrderIterator.h" // For ReversePostOrderTraversal.
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"       // To instantiate RPOTraversal.
#include "llvm/IR/Constants.h" // For ConstantInt.
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h" // For BinaryOperator, etc.
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"          // For FunctionPass & INITIALIZE_PASS.
#include "llvm/Support/Debug.h" // For LLVM_DEBUG, dbgs(), etc.

#include <optional>

#define DEBUG_TYPE "h2blb-simple-cst-prop"

using namespace llvm;

// Helper function to deal with binary instructions.
static Value *visitBinary(Instruction &Instr, LLVMContext &Ctxt,
                          std::optional<APInt> (*Computation)(const APInt &,
                                                              const APInt &)) {
  assert(isa<BinaryOperator>(Instr) && "This is meant for binary instruction");

  auto *LHS = dyn_cast<ConstantInt>(Instr.getOperand(0));
  auto *RHS = dyn_cast<ConstantInt>(Instr.getOperand(1));
  if (!LHS || !RHS)
    return nullptr;

  // FIXME: Technically this API is not precise enough because we may want to
  // produce poison value for e.g., a division by zero.
  std::optional<APInt> Res = Computation(LHS->getValue(), RHS->getValue());
  if (!Res.has_value())
    return nullptr;
  auto NewConstant = ConstantInt::get(Ctxt, *Res);
  return NewConstant;
}

// Takes \p Foo and apply a simple constant propagation optimization.
// \returns true if \p Foo was modified (i.e., something had been constant
// propagated), false otherwise.
static bool simpleConstantPropagation(Function &Foo) {
  LLVMContext &Ctxt = Foo.getParent()->getContext();
  bool MadeChanges = false;

  ReversePostOrderTraversal<Function *> RPOT(&Foo);
  for (BasicBlock *BB : RPOT) {
    // Early increment to be able to remove the instruction that we replaced
    // on-the-fly. The alternative is to accumulate the instructions to remove
    // in a worklist and delete them afterwards.
    for (Instruction &Instr : make_early_inc_range(*BB)) {
      Value *NewConstant = nullptr;
      switch (Instr.getOpcode()) {
      case Instruction::Add:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              return A + B;
            });
        break;
      case Instruction::Sub:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              return A - B;
            });
        break;
      case Instruction::Mul:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              return A * B;
            });
        break;
      case Instruction::SDiv:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              if (B.isZero())
                return std::nullopt;
              return A.sdiv(B);
            });
        break;
      case Instruction::UDiv:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              if (B.isZero())
                return std::nullopt;
              return A.udiv(B);
            });
        break;
      case Instruction::Shl:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              return A.shl(B);
            });
        break;
      case Instruction::LShr:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              return A.lshr(B);
            });
        break;
      case Instruction::AShr:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              return A.ashr(B);
            });
        break;
      case Instruction::And:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              return A & B;
            });
        break;
      case Instruction::Or:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              return A | B;
            });
        break;
      case Instruction::Xor:
        NewConstant = visitBinary(
            Instr, Ctxt,
            [](const APInt &A, const APInt &B) -> std::optional<APInt> {
              return A ^ B;
            });
        break;

      default:
        break;
      }
      if (NewConstant) {
        Instr.replaceAllUsesWith(NewConstant);
        Instr.eraseFromParent();
        MadeChanges = true;
      }
    }
  }
  return MadeChanges;
}

namespace {
// The scope we want to modify is a Function.
class H2BLBSimpleConstantPropagation : public FunctionPass {
public:
  // Used for the pass registry.
  static char ID;

  H2BLBSimpleConstantPropagation() : FunctionPass(ID) {
    // Technically we do not need to put this in the constructor.
    // Usually this call lives in the uber InitializeAllXXX.
    initializeH2BLBSimpleConstantPropagationPass(
        *PassRegistry::getPassRegistry());
  }

  // Main function of a FunctionPass.
  // Returns true if any change was made to F.
  bool runOnFunction(Function &F) override {
    LLVM_DEBUG(dbgs() << "Simple constant propagation Legacy called on "
                      << F.getName() << '\n');
    return simpleConstantPropagation(F);
  }
};
} // End anonymous namespace.

// Anchor for this pass' ID.
char H2BLBSimpleConstantPropagation::ID = 0;

PreservedAnalyses
H2BLBSimpleConstantPropagationNewPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  LLVM_DEBUG(dbgs() << "Simple constant propagation New PM on " << F.getName()
                    << "\n");
  bool MadeChanges = simpleConstantPropagation(F);
  if (!MadeChanges)
    return PreservedAnalyses::all();
  // Even if we made changes, we didn't touched the CFG.
  // So everything on that is still current.
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

// Initialize function used for the pass registration.
// This hooks up the command line option and gives general information
// about the pass' properties.
// This macro generates a llvm::initialize##passImplementationName##Pass
// function.
INITIALIZE_PASS(/*passImplementationName=*/H2BLBSimpleConstantPropagation,
                /*commandLineArgName=*/DEBUG_TYPE,
                /*name=*/"H2BLB simple constant propagation",
                /*isCFGOnly=*/false,
                /*isAnalysis=*/false)

Pass *createH2BLBSimpleConstantPropagationPassForLegacyPM() {
  return new H2BLBSimpleConstantPropagation();
}
