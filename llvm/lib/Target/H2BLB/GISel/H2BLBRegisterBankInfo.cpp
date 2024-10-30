//===- H2BLBRegisterBankInfo.cpp ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the RegisterBankInfo class for H2BLB
//===----------------------------------------------------------------------===//

#include "H2BLBRegisterBankInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "h2blb-reg-bank-info"

#define GET_TARGET_REGBANK_IMPL
#include "H2BLBGenRegisterBank.inc"

using namespace llvm;

namespace llvm {
namespace H2BLB {

const RegisterBankInfo::PartialMapping PartMappings[] = {
    // clang-format off
    {0, 16, GPRBRegBank},
    {0, 32, GPRBRegBank},
    // clang-format on
};

enum PartialMappingIdx {
  PMI_GPRB16 = 0,
  PMI_GPRB32 = 1,
};

const RegisterBankInfo::ValueMapping ValueMappings[] = {
    // Invalid value mapping.
    {nullptr, 0},
    // Maximum 3 GPR operands; 16 bit.
    {&PartMappings[PMI_GPRB16], 1},
    {&PartMappings[PMI_GPRB16], 1},
    {&PartMappings[PMI_GPRB16], 1},
    // Maximum 3 GPR operands; 32 bit.
    {&PartMappings[PMI_GPRB32], 1},
    {&PartMappings[PMI_GPRB32], 1},
    {&PartMappings[PMI_GPRB32], 1},
};

enum ValueMappingIdx {
  InvalidIdx = 0,
  GPRB16Idx = 1,
  GPRB32Idx = 4,
};
} // namespace H2BLB
} // namespace llvm


H2BLBRegisterBankInfo::H2BLBRegisterBankInfo(const TargetRegisterInfo &TRI)
    : H2BLBGenRegisterBankInfo() {}

const RegisterBankInfo::InstructionMapping &
H2BLBRegisterBankInfo::getInstrMapping(const MachineInstr &MI) const {
  const unsigned Opc = MI.getOpcode();

  // Try the default logic for non-generic instructions that are either copies
  // or already have some operands assigned to banks.
  if (!isPreISelGenericOpcode(Opc) || Opc == TargetOpcode::G_PHI) {
    const InstructionMapping &Mapping = getInstrMappingImpl(MI);
    if (Mapping.isValid())
      return Mapping;
  }

  const MachineFunction &MF = *MI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();

  unsigned NumOperands = MI.getNumOperands();
  const ValueMapping *GPR16ValueMapping = &H2BLB::ValueMappings[H2BLB::GPRB16Idx];
  const ValueMapping *GPR32ValueMapping = &H2BLB::ValueMappings[H2BLB::GPRB32Idx];

  switch (Opc) {
  case TargetOpcode::G_ADD:
  case TargetOpcode::G_SUB:
  case TargetOpcode::G_SHL:
  case TargetOpcode::G_ASHR:
  case TargetOpcode::G_LSHR:
  case TargetOpcode::G_AND:
  case TargetOpcode::G_OR:
  case TargetOpcode::G_XOR:
  case TargetOpcode::G_PTR_ADD:
  case TargetOpcode::G_PTRTOINT:
  case TargetOpcode::G_INTTOPTR: {
    LLT Ty = MRI.getType(MI.getOperand(0).getReg());
    TypeSize Size = Ty.getSizeInBits();

    const ValueMapping *Mapping = Size == 16? GPR16ValueMapping: GPR32ValueMapping;

#ifndef NDEBUG
    // Make sure all the operands are using similar size and type.
    for (unsigned Idx = 1; Idx != NumOperands; ++Idx) {
      LLT OpTy = MRI.getType(MI.getOperand(Idx).getReg());
      assert(Ty.isVector() == OpTy.isVector() &&
             "Operand has incompatible type");
      // Don't check size for GPR.
      if (OpTy.isVector())
        assert(Size == OpTy.getSizeInBits() && "Operand has incompatible size");
    }
#endif // End NDEBUG

    return getInstructionMapping(DefaultMappingID, 1, Mapping, NumOperands);
  }
  default: {

  SmallVector<const ValueMapping *, 4> OpdsMapping(NumOperands);

    // By default map all scalars to GPR.
    for (unsigned Idx = 0; Idx < NumOperands; ++Idx) {
       auto &MO = MI.getOperand(Idx);
       if (!MO.isReg() || !MO.getReg())
         continue;
       LLT Ty = MRI.getType(MO.getReg());
       if (!Ty.isValid())
         continue;

       OpdsMapping[Idx] = Ty.getSizeInBits() == 16? GPR16ValueMapping : GPR32ValueMapping;
    }
  return getInstructionMapping(DefaultMappingID, /*Cost=*/1,
                               getOperandsMapping(OpdsMapping), NumOperands);
  }
}
llvm_unreachable("Switch should have covered everything");
}

const RegisterBank &
H2BLBRegisterBankInfo::getRegBankFromRegClass(const TargetRegisterClass &RC,
                                              LLT Ty) const {
  switch (RC.getID()) {
  default:
    llvm_unreachable("Register class not supported");
  case H2BLB::GPR16RegClassID:
  case H2BLB::GPR32RegClassID:
  case H2BLB::GPR16spRegClassID:
  case H2BLB::OnlySPRegClassID:
    return getRegBank(H2BLB::GPRBRegBankID);
  }
}
