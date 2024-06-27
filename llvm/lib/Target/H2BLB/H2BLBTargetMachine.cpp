//===-- H2BLBTargetMachine.cpp - Define TargetMachine for H2BLB -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about H2BLB target spec.
//
//===----------------------------------------------------------------------===//

#include "H2BLBTargetMachine.h"
#include "TargetInfo/H2BLBTargetInfo.h" // For getTheH2BLBTarget.
#include "llvm/MC/TargetRegistry.h"     // For RegisterTargetMachine.
#include "llvm/Support/Compiler.h"      // For LLVM_EXTERNAL_VISIBILITY.

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeH2BLBTarget() {
  // Register the target so that external tools can instantiate it.
  RegisterTargetMachine<H2BLBTargetMachine> X(getTheH2BLBTarget());
}

// TODO: Share this with Clang.
static const char *H2BLBDataLayoutStr =
    "e-p:16:16:16-n16:32-i32:32:32-i16:16:16-i1:8:8-f32:32:32-v32:32:32";

H2BLBTargetMachine::H2BLBTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       std::optional<Reloc::Model> RM,
                                       std::optional<CodeModel::Model> CM,
                                       CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, H2BLBDataLayoutStr, TT, CPU, FS, Options,
                               // Use the simplest relocation by default.
                               RM ? *RM : Reloc::Static,
                               CM ? *CM : CodeModel::Small, OL) {}

H2BLBTargetMachine::~H2BLBTargetMachine() = default;
