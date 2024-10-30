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
#include "llvm/CodeGen/TargetInstrInfo.h"
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
  // Reserve the stack register so that the register allocator doesn't
  // touch it.
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

  DebugLoc DL = MI.getDebugLoc();

  MachineOperand &FIOp = MI.getOperand(FIOperandNum);
  int Index = MI.getOperand(FIOperandNum).getIndex();

  // The offset is from the incoming stack pointer.
  // Since our stack pointer is adjusted to the current frame before we
  // issue the load and store, we need to adjust the offset accordingly.
  // I.e., incoming sp pointer + offset
  //       == (current sp pointer + framesize) + offset
  // Note: incoming sp pointer is current sp pointer + framesize, because the
  // stack grows down.
  int64_t Offset = MFI.getObjectOffset(Index);
  Offset += MFI.getStackSize();
  assert(FIOperandNum == 1 && "Stack argument is expected to be the second "
                              "operand for both loads and stores");
  switch (MI.getOpcode()) {
  case H2BLB::LDRSEXTSP8:
  case H2BLB::LDRZEXTSP8:
  case H2BLB::STRSP16:
  case H2BLB::LDRSP16:
  case H2BLB::STRSP32:
  case H2BLB::LDRSP32:
    FIOp.ChangeToRegister(H2BLB::SP, /*IsDef=*/false);
    Offset += MI.getOperand(2).getImm();
    // If the offset doesn't fit, we need to expand into:
    // off = ldimm offset
    // base = movefromsp sp
    // addr = addi16rr base, off
    // loadOrStoreNoSP16 addr, 0
    //
    // SUBSP
    // movfromsp
    // loadOrStore
    // ADDSP
    assert(Offset >= -64 && Offset < 63 && "Offset must fit 7 bits for now");
    MI.getOperand(2).setImm(Offset);
    break;
  case H2BLB::MOVFROMSP: {
    FIOp.ChangeToRegister(H2BLB::SP, /*IsDef=*/false);

    // If the offset is zero, the MOVFROMSP is enough.
    if (Offset == 0)
      break;
    // SUBSP
    // movfromsp
    // ADDSP
    // Note that there are other possible sequence to, but this one doesn't
    // require to use the register scavenger.
    llvm_unreachable("Not yet implemented");
    break;
  }
  default:
    llvm_unreachable("frame index used on unknown instruction");
  }

  return false;
}

Register H2BLBRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return Register();
}
