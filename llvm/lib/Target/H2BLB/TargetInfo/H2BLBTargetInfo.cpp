//===-- H2BLBTargetInfo.cpp - H2BLB Target Implementation -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "H2BLBTargetInfo.h"
#include "llvm/MC/TargetRegistry.h" // For RegisterTarget.
#include "llvm/Support/Compiler.h"  // For LLVM_EXTERNAL_VISIBILITY.
#include "llvm/TextAPI/Target.h"    // For Target class.

using namespace llvm;

Target &llvm::getTheH2BLBTarget() {
  static Target TheH2BLBTarget;
  return TheH2BLBTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeH2BLBTargetInfo() {
  RegisterTarget<Triple::h2blb, /*HasJIT=*/false> X(
      getTheH2BLBTarget(), /*Name=*/"h2blb",
      /*Desc=*/"How to build an LLVM backend by example",
      /*BackendName=*/"H2BLB");
}
