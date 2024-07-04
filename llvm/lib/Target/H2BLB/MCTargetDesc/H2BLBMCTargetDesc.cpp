//===-- H2BLBMCTargetDesc.cpp - H2BLB Target Descriptions -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides H2BLB specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "H2BLBMCTargetDesc.h"
#include "TargetInfo/H2BLBTargetInfo.h" // For getTheH2BLBTarget.
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Compiler.h"  // For LLVM_EXTERNAL_VISIBILITY.
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

#define GET_SUBTARGETINFO_MC_DESC
#include "H2BLBGenSubtargetInfo.inc"

static MCRegisterInfo *createH2BLBMCRegisterInfo(const Triple &Triple) {
  MCRegisterInfo *X = new MCRegisterInfo();
  // TODO: Fill out the register info.
  return X;
}

static MCInstrInfo *createH2BLBMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  // TODO: Fill out the instr info.
  return X;
}

static MCSubtargetInfo *
createH2BLBMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createH2BLBMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeH2BLBTargetMC() {
  Target &TheTarget = getTheH2BLBTarget();

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheTarget, createH2BLBMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheTarget, createH2BLBMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheTarget,
                                          createH2BLBMCSubtargetInfo);
}
