//===- H2BLBInstructionSelector.cpp ------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the InstructionSelector class for
/// H2BLB.
//===----------------------------------------------------------------------===//

#include "H2BLB.h"
#include "H2BLBInstrInfo.h"
#include "H2BLBRegisterBankInfo.h"
#include "H2BLBSubtarget.h"
#include "H2BLBTargetMachine.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/IntrinsicsH2BLB.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "h2blb-gisel"

using namespace llvm;

namespace {

#define GET_GLOBALISEL_PREDICATE_BITSET
#include "H2BLBGenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATE_BITSET

class H2BLBInstructionSelector : public InstructionSelector {
public:
  H2BLBInstructionSelector(const H2BLBTargetMachine &TM,
                           const H2BLBSubtarget &STI,
                           const H2BLBRegisterBankInfo &RBI);

  bool select(MachineInstr &I) override;
  static const char *getName() { return DEBUG_TYPE; }

private:
  /// tblgen generated 'select' implementation that is used as the initial
  /// selector for the patterns that do not require complex C++.
  bool selectImpl(MachineInstr &I, CodeGenCoverage &CoverageInfo) const;

  const H2BLBInstrInfo &TII;
  const H2BLBRegisterInfo &TRI;
  const H2BLBRegisterBankInfo &RBI;

#define GET_GLOBALISEL_PREDICATES_DECL
#include "H2BLBGenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_DECL

#define GET_GLOBALISEL_TEMPORARIES_DECL
#include "H2BLBGenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_DECL
};

} // namespace

#define GET_GLOBALISEL_IMPL
#include "H2BLBGenGlobalISel.inc"
#undef GET_GLOBALISEL_IMPL

H2BLBInstructionSelector::H2BLBInstructionSelector(
    const H2BLBTargetMachine &TM, const H2BLBSubtarget &STI,
    const H2BLBRegisterBankInfo &RBI)
    : TII(*STI.getInstrInfo()), TRI(*STI.getRegisterInfo()), RBI(RBI),
#define GET_GLOBALISEL_PREDICATES_INIT
#include "H2BLBGenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_INIT
#define GET_GLOBALISEL_TEMPORARIES_INIT
#include "H2BLBGenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_INIT
{
}

static void setRegisterClassForOperands(MachineInstr &I,
                                        MachineRegisterInfo &MRI) {
  for (MachineOperand &MO : I.operands()) {
    Register Reg = MO.getReg();
    if (Reg.isPhysical())
      continue;
    const TargetRegisterClass *RC = MRI.getRegClassOrNull(Reg);
    if (RC)
      continue;
    unsigned Size = MRI.getType(Reg).getSizeInBits();
    MRI.setRegClass(Reg, Size == 16 ? &H2BLB::GPR16spRegClass
                                    : &H2BLB::GPR32RegClass);
  }
}

bool H2BLBInstructionSelector::select(MachineInstr &I) {
  unsigned Opc = I.getOpcode();
  if (!isPreISelGenericOpcode(Opc) && Opc != TargetOpcode::PHI &&
      Opc != TargetOpcode::COPY)
    return true;

  MachineBasicBlock &MBB = *I.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  switch (Opc) {
  case TargetOpcode::G_IMPLICIT_DEF:
    I.setDesc(TII.get(TargetOpcode::IMPLICIT_DEF));
    setRegisterClassForOperands(I, MRI);
    return true;
  case TargetOpcode::G_PHI:
    I.setDesc(TII.get(TargetOpcode::PHI));
    [[fallthrough]];
  case TargetOpcode::PHI:
  case TargetOpcode::COPY:
    // For PHIs and COPYs, we only need to assigned a register class.
    setRegisterClassForOperands(I, MRI);
    return true;
  default:
    if (selectImpl(I, *CoverageInfo))
      return true;
  }
  return false;
}

namespace llvm {
InstructionSelector *
H2BLB::createInstructionSelector(const H2BLBTargetMachine &TM,
                                 const H2BLBSubtarget &Subtarget,
                                 const H2BLBRegisterBankInfo &RBI) {
  return new H2BLBInstructionSelector(TM, Subtarget, RBI);
}
} // namespace llvm
