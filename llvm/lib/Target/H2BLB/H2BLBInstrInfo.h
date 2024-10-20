//===-- H2BLBInstrInfo.h - H2BLB Instruction Information --------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBINSTRINFO_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBINSTRINFO_H

#include "MCTargetDesc/H2BLBMCTargetDesc.h" // For all the opcodes' enum.
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "H2BLBGenInstrInfo.inc"

namespace llvm {

class H2BLBInstrInfo : public H2BLBGenInstrInfo {
public:
  H2BLBInstrInfo();

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI, Register SrcReg,
                           bool isKill, int FI, const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI,
                           Register VReg) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI, Register DestReg,
                            int FI, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI,
                            Register VReg) const override;

  /// Callback to materialize a register-to-regiter copy before \p MI in
  /// \p MBB. The copy to materialize is DestReg = COPY SrcReg. The opcode
  /// of the COPY needs to be the actual target-specific opcode.
  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;
};
} // namespace llvm

#endif
