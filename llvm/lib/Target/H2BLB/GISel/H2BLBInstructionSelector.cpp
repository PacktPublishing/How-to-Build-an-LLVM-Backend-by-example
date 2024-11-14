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
#include "llvm/CodeGen/GlobalISel/MIPatternMatch.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/IntrinsicsH2BLB.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "h2blb-gisel"

using namespace llvm;
using namespace MIPatternMatch;

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

  ComplexRendererFns selectAddrMode(MachineOperand &Root) const;
  ComplexRendererFns selectSPAddrMode(MachineOperand &Root) const;

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
    MRI.setRegClass(Reg, Size == 16 ? &H2BLB::GPR16RegClass
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
  case TargetOpcode::G_FRAME_INDEX: {
    // Frame index are ultimately SP + something.
    // We'll materialize the ADD when we know the offset at frame lowering time.
    // This may require an emergency spill slot.
    I.setDesc(TII.get(H2BLB::MOVFROMSP));
    return constrainSelectedInstRegOperands(I, TII, TRI, RBI);
  }

  default:
    if (selectImpl(I, *CoverageInfo))
      return true;
  }
  return false;
}

InstructionSelector::ComplexRendererFns
H2BLBInstructionSelector::selectAddrMode(MachineOperand &Root) const {
  // Here we check if the SP addr mode applies before checking this one.
  // Technically this is a hack because it is supposed to be handled at
  // the .td level with the AddedComplexity. However TableGen does
  // something unexpected with the AddedComplexity with extload.
  if (selectSPAddrMode(Root) != std::nullopt)
    return std::nullopt;

  MachineRegisterInfo &MRI =
      Root.getParent()->getParent()->getParent()->getRegInfo();

  // Loads and stores through the stack need to go through the SP-based
  // addressing mode.
  MachineInstr *RootDef = MRI.getVRegDef(Root.getReg());
  if (RootDef->getOpcode() == TargetOpcode::G_FRAME_INDEX)
    return std::nullopt;

  Register BaseReg = RootDef->getOperand(0).getReg();
  uint64_t Offset = 0;
  // Do some matching of ADD + immediate and fold if it fits.
  if (RootDef->getOpcode() == TargetOpcode::G_ADD ||
      RootDef->getOpcode() == TargetOpcode::G_PTR_ADD) {
    std::optional<ValueAndVReg> MaybeConstantInt;
    if (mi_match(RootDef->getOperand(2).getReg(), MRI,
                 m_GCst(MaybeConstantInt))) {
      uint64_t CstImm = MaybeConstantInt->Value.getZExtValue();
      if (CstImm < 16) {
        BaseReg = RootDef->getOperand(1).getReg();
        Offset = CstImm;
      }
    }
  }

  return {{
      [=](MachineInstrBuilder &MIB) { MIB.addReg(BaseReg); },
      [=](MachineInstrBuilder &MIB) { MIB.addImm(Offset); },
  }};
}

InstructionSelector::ComplexRendererFns
H2BLBInstructionSelector::selectSPAddrMode(MachineOperand &Root) const {
  if (!Root.isReg())
    return std::nullopt;

  MachineRegisterInfo &MRI =
      Root.getParent()->getParent()->getParent()->getRegInfo();

  MachineInstr *RootDef = MRI.getVRegDef(Root.getReg());
  if (RootDef->getOpcode() == TargetOpcode::G_FRAME_INDEX)
    return {{
        [=](MachineInstrBuilder &MIB) {
          MIB.addFrameIndex(RootDef->getOperand(1).getIndex());
        },
        [=](MachineInstrBuilder &MIB) { MIB.addImm(0); },
    }};

  auto MaybeSPReg = [](MachineInstr &MI) -> bool {
    if (MI.getOpcode() == TargetOpcode::COPY) {
      Register Reg = MI.getOperand(1).getReg();
      return Reg == H2BLB::SP;
    }
    return false;
  };

  auto MaybeFrameIndex = [](MachineInstr &MI) -> std::optional<int> {
    if (MI.getOpcode() == TargetOpcode::G_FRAME_INDEX)
      return MI.getOperand(1).getIndex();
    return std::nullopt;
  };

  auto MatchSPRegOrIndex = [&MaybeFrameIndex,
                            &MaybeSPReg](MachineInstr &MI,
                                         std::optional<int> &Index) -> bool {
    std::optional<int> MaybeIndex = MaybeFrameIndex(MI);
    if (MaybeIndex) {
      Index = *MaybeIndex;
      return true;
    }
    return MaybeSPReg(MI);
  };

  // On a match, if MaybeIndex is set we are dealing with a FrameIndex,
  // otherwise we are dealing with SP.
  std::optional<int> MaybeIndex;
  bool Matched = false;
  int64_t CstImm = 0;

  switch (RootDef->getOpcode()) {
  case TargetOpcode::COPY:
  case TargetOpcode::G_FRAME_INDEX:
    Matched = MatchSPRegOrIndex(*RootDef, MaybeIndex);
    break;
  case TargetOpcode::G_PTR_ADD: {
    // Do some matching of ADD + immediate and fold if it fits.
    std::optional<ValueAndVReg> MaybeConstantInt;
    if (!mi_match(RootDef->getOperand(2).getReg(), MRI,
                  m_GCst(MaybeConstantInt)))
      return std::nullopt;

    CstImm = MaybeConstantInt->Value.getZExtValue();
    if (CstImm < -64 || CstImm >= 64)
      return std::nullopt;

    MachineInstr *BaseDef = MRI.getVRegDef(RootDef->getOperand(1).getReg());
    Matched = MatchSPRegOrIndex(*BaseDef, MaybeIndex);
    break;
  }
  default:
    Matched = false;
    break;
  }

  if (!Matched)
    return std::nullopt;

  return {{
      [=](MachineInstrBuilder &MIB) {
        if (MaybeIndex)
          MIB.addFrameIndex(*MaybeIndex);
        else
          MIB.addReg(H2BLB::SP);
      },
      [=](MachineInstrBuilder &MIB) { MIB.addImm(CstImm); },
  }};
}

namespace llvm {
InstructionSelector *
H2BLB::createInstructionSelector(const H2BLBTargetMachine &TM,
                                 const H2BLBSubtarget &Subtarget,
                                 const H2BLBRegisterBankInfo &RBI) {
  return new H2BLBInstructionSelector(TM, Subtarget, RBI);
}
} // namespace llvm
