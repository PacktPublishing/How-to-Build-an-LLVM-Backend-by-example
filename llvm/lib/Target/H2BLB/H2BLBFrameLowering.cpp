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
#include "llvm/Support/Error.h"

using namespace llvm;

bool H2BLBFrameLowering::hasFP(const MachineFunction &MF) const {
  return false;
}

void H2BLBFrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  // Get the number of bytes to allocate from the FrameInfo.
  unsigned NumBytes = MFI.getStackSize();

  if (NumBytes > 0) {
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    BuildMI(MBB, MBB.begin(), DebugLoc(), TII->get(H2BLB::SUBSP), H2BLB::SP)
        .addReg(H2BLB::SP)
        .addImm(NumBytes);
  }
}

void H2BLBFrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  // Get the number of bytes to allocate from the FrameInfo.
  unsigned NumBytes = MFI.getStackSize();

  if (NumBytes > 0) {
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    BuildMI(MBB, MBB.getFirstTerminator(), DebugLoc(), TII->get(H2BLB::ADDSP),
            H2BLB::SP)
        .addReg(H2BLB::SP)
        .addImm(NumBytes);
  }
}

MachineBasicBlock::iterator H2BLBFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  unsigned Opc = MI->getOpcode();

  // The call frame should always be included in the stack frame in the
  // prologue.
  assert(hasReservedCallFrame(MF) && "H2BLB doesn't have a FP register");

  if (Opc != TII->getCallFrameSetupOpcode() &&
      Opc != TII->getCallFrameDestroyOpcode())
    report_fatal_error("Unexpected frame pseudo instruction");

  if (MI->getOperand(1).getImm() != 0)
    report_fatal_error("Callee pop count not supported");

  return MBB.erase(MI);
}
