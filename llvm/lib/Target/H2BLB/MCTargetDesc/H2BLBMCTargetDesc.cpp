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

#include "H2BLBInstPrinter.h"
#include "H2BLBMCAsmInfo.h"
#include "TargetInfo/H2BLBTargetInfo.h" // For getTheH2BLBTarget.
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h" // For RegisterMCAsmInfoFn.
#include "llvm/Support/Compiler.h"  // For LLVM_EXTERNAL_VISIBILITY.
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

#define GET_SUBTARGETINFO_MC_DESC
#include "H2BLBGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "H2BLBGenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#define GET_INSTRINFO_MC_HELPERS
#include "H2BLBGenInstrInfo.inc"

static MCRegisterInfo *createH2BLBMCRegisterInfo(const Triple &Triple) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitH2BLBMCRegisterInfo(X, H2BLB::R7);
  return X;
}

static MCInstrInfo *createH2BLBMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitH2BLBMCInstrInfo(X);
  return X;
}

static MCSubtargetInfo *
createH2BLBMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createH2BLBMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCAsmInfo *createH2BLBMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TheTriple,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI;
  if (TheTriple.isOSBinFormatMachO())
    MAI = new H2BLBMCAsmInfoDarwin(TheTriple, Options);
  else if (TheTriple.isOSBinFormatELF())
    MAI = new H2BLBMCAsmInfoELF(TheTriple, Options);
  else
    report_fatal_error("Binary format not supported");

  return MAI;
}

static MCInstPrinter *createH2BLBMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new H2BLBInstPrinter(MAI, MII, MRI);
  return nullptr;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeH2BLBTargetMC() {
  Target &TheTarget = getTheH2BLBTarget();

  // Register the MC asm info.
  RegisterMCAsmInfoFn X(TheTarget, createH2BLBMCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheTarget, createH2BLBMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheTarget, createH2BLBMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheTarget,
                                          createH2BLBMCSubtargetInfo);
  // Register the MCInst to asm printer.
  TargetRegistry::RegisterMCInstPrinter(TheTarget, createH2BLBMCInstPrinter);

  // Register the MC code emitter.
  TargetRegistry::RegisterMCCodeEmitter(getTheH2BLBTarget(),
                                        createH2BLBMCCodeEmitter);
}
