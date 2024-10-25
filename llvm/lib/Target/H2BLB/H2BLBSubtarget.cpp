//===- H2BLBSubtarget.cpp - H2BLB Subtarget Information ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the H2BLB specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#include "H2BLBSubtarget.h"
#include "GISel/H2BLBCallLowering.h"
#include "GISel/H2BLBLegalizerInfo.h"
#include "GISel/H2BLBRegisterBankInfo.h"
#include "H2BLB.h" // For H2BLB::createInstructionSelector.
#include "H2BLBTargetMachine.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "h2blb-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "H2BLBGenSubtargetInfo.inc"

// Pin the vtable to this file.
void H2BLBSubtarget::anchor() {}

H2BLBSubtarget::H2BLBSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                               const TargetMachine &TM)
    : H2BLBGenSubtargetInfo(TT, CPU, /*TuneCPU=*/"", FS), FrameLowering(*this),
      TLInfo(TM, *this) {
  CallLoweringInfo.reset(new H2BLBCallLowering(*getTargetLowering()));
  Legalizer.reset(new H2BLBLegalizerInfo(*this));
  auto *RBI = new H2BLBRegisterBankInfo(*getRegisterInfo());
  RegBankInfo.reset(RBI);
  InstrSelector.reset(H2BLB::createInstructionSelector(
      *static_cast<const H2BLBTargetMachine *>(&TM), *this, *RBI));
}

const CallLowering *H2BLBSubtarget::getCallLowering() const {
  return CallLoweringInfo.get();
}

const LegalizerInfo *H2BLBSubtarget::getLegalizerInfo() const {
  return Legalizer.get();
}

const RegisterBankInfo *H2BLBSubtarget::getRegBankInfo() const {
  return RegBankInfo.get();
}

InstructionSelector *H2BLBSubtarget::getInstructionSelector() const {
  return InstrSelector.get();
}
