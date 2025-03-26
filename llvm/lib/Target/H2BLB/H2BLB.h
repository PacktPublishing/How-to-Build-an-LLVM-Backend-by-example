//===-- H2BLB.h - H2BLB specific passes ---------------------------*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file hold the declarations for the H2BLB-specific passes for
// both the legacy and new pass managers.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_H2BLB_H2BLB_H
#define LLVM_LIB_TARGET_H2BLB_H2BLB_H
#include "llvm/IR/PassManager.h" // For PassInfoMixin.
#include "llvm/PassRegistry.h"

namespace llvm {
class Function;
class Pass;
class PassRegistry;

class H2BLBSimpleConstantPropagationNewPass
    : public llvm::PassInfoMixin<H2BLBSimpleConstantPropagationNewPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM);
};

void initializeH2BLBSimpleConstantPropagationPass(PassRegistry &);
Pass *createH2BLBSimpleConstantPropagationPassForLegacyPM();
} // end namespace llvm.
#endif
