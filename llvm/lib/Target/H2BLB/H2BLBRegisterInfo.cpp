//===-- H2BLBRegisterInfo.cpp - H2BLB Register Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the H2BLB implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "H2BLBRegisterInfo.h"
#include "H2BLBFrameLowering.h"
#include "MCTargetDesc/H2BLBMCTargetDesc.h" // For the enum of the regclasses.
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "H2BLBGenRegisterInfo.inc"
using namespace llvm;

H2BLBRegisterInfo::H2BLBRegisterInfo() : H2BLBGenRegisterInfo(Register()) {}

const MCPhysReg *
H2BLBRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return nullptr;
}

BitVector H2BLBRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  return Reserved;
}
bool H2BLBRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj, unsigned FIOperandNum,
                                            RegScavenger *RS) const {
  return false;
}

Register H2BLBRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return Register();
}
