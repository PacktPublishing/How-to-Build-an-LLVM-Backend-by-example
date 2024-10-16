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
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "H2BLBGenRegisterInfo.inc"
using namespace llvm;

H2BLBRegisterInfo::H2BLBRegisterInfo() : H2BLBGenRegisterInfo(H2BLB::R0) {}

const MCPhysReg *
H2BLBRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

const uint32_t *
H2BLBRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID CC) const {
  return CSR_RegMask;
}

BitVector H2BLBRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  // Reserve the link and stack registers so that the register allocator doesn't
  // touch them.
  Reserved.set(H2BLB::R0);
  markSuperRegs(Reserved, H2BLB::SP);
  return Reserved;
}
bool H2BLBRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj, unsigned FIOperandNum,
                                            RegScavenger *RS) const {

  assert(SPAdj == 0 && "unhandled SP adjustment in call sequence?");

  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  DebugLoc DL = MI.getDebugLoc();

  MachineOperand &FIOp = MI.getOperand(FIOperandNum);
  int Index = MI.getOperand(FIOperandNum).getIndex();

  int64_t Offset = MFI.getObjectOffset(Index);
  // TODO: replace stackslot with SP + offset

  return false;
}

Register H2BLBRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return Register();
}
