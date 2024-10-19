//===-- H2BLBInstrInfo.cpp - H2BLB Instruction Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the H2BLB implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "H2BLBInstrInfo.h"
#include "H2BLB.h"
#include "H2BLBRegisterInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <iterator>

#define GET_INSTRINFO_CTOR_DTOR
#include "H2BLBGenInstrInfo.inc"

using namespace llvm;

H2BLBInstrInfo::H2BLBInstrInfo() : H2BLBGenInstrInfo() {}

void H2BLBInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 const DebugLoc &DL, MCRegister DestReg,
                                 MCRegister SrcReg, bool KillSrc) const {
  const TargetRegisterInfo &TRI =
      *MBB.getParent()->getSubtarget().getRegisterInfo();
  unsigned Opc = TRI.getMinimalPhysRegClass(DestReg) == &H2BLB::GPR16RegClass
                     ? H2BLB::MOV16
                     : H2BLB::MOV32;
  if (SrcReg == H2BLB::SP) {
    assert(TRI.getMinimalPhysRegClass(DestReg) == &H2BLB::GPR16RegClass &&
           "Dest reg for stack must be 16-bit");
    Opc = H2BLB::MOVFROMSP;
  } else if (DestReg == H2BLB::SP) {
    assert(TRI.getMinimalPhysRegClass(SrcReg) == &H2BLB::GPR16RegClass &&
           "Src reg for stack must be 16-bit");
    Opc = H2BLB::MOVTOSP;
  }
  BuildMI(MBB, MI, MI->getDebugLoc(), get(Opc), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
}
