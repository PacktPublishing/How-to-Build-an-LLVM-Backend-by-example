//===-- H2BLBFrameLowering.cpp - H2BLB Frame Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the H2BLB implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "H2BLBFrameLowering.h"
#include "H2BLBInstrInfo.h"
#include "H2BLBSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool H2BLBFrameLowering::hasFP(const MachineFunction &MF) const {
  return false;
}

void H2BLBFrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {}

void H2BLBFrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {}
