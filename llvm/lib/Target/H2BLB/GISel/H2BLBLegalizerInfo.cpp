//===- H2BLBLegalizerInfo.h --------------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the Machinelegalizer class for H2BLB
//===----------------------------------------------------------------------===//

#include "H2BLBLegalizerInfo.h"
#include "H2BLBSubtarget.h"
#include "llvm/CodeGen/GlobalISel/LegalizerHelper.h"
#include "llvm/CodeGen/GlobalISel/MIPatternMatch.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGenTypes/LowLevelType.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "h2blb-legalinfo"

using namespace llvm;
using namespace LegalizeActions;
using namespace MIPatternMatch;

H2BLBLegalizerInfo::H2BLBLegalizerInfo(const H2BLBSubtarget &ST) : ST(ST) {
  const LLT p0 = LLT::pointer(0, 16);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);

  // Constants
  getActionDefinitionsBuilder(
      {TargetOpcode::G_CONSTANT, TargetOpcode::G_IMPLICIT_DEF})
      .legalFor({p0, s16, s32})
      .widenScalarToNextPow2(0)
      .clampScalar(0, s16, s32);

  // Load and store.
  // Note that technically the G_<S|Z>EXTLOAD don't need as many legal types,
  // but this is convenient to reuse the same rules for all loads and stores.
  getActionDefinitionsBuilder({TargetOpcode::G_LOAD, TargetOpcode::G_SEXTLOAD,
                               TargetOpcode::G_ZEXTLOAD, TargetOpcode::G_STORE})
      .legalForTypesWithMemDesc({{s8, p0, s8, 8},
                                 {s16, p0, s8, 8}, // anyext/truncstore
                                 {s16, p0, s16, 8},
                                 {s32, p0, s32, 8}})
      .clampScalar(0, s16, s32)
      .lowerIf([=](const LegalityQuery &Query) {
        return Query.Types[0].isScalar() &&
               Query.Types[0] != Query.MMODescrs[0].MemoryTy;
      })
      .legalIf([=](const LegalityQuery &Query) {
        TypeSize Size = Query.Types[0].getSizeInBits();
        return Size == 16 || Size == 32;
      });

  // Pointer-handling.
  getActionDefinitionsBuilder(TargetOpcode::G_FRAME_INDEX).legalFor({p0});
  getActionDefinitionsBuilder(TargetOpcode::G_PTR_ADD).legalFor({{p0, s16}});

  // Arithmetic.
  getActionDefinitionsBuilder({TargetOpcode::G_ADD, TargetOpcode::G_AND})
      .legalFor({s16, s32})
      .clampScalar(0, s16, s32);

  getActionDefinitionsBuilder(TargetOpcode::G_MUL)
      .customIf([=](const LegalityQuery &Query) {
        const auto &DstTy = Query.Types[0];
        return !DstTy.isVector() && DstTy.getSizeInBits() == 32;
      });

  getActionDefinitionsBuilder(TargetOpcode::G_EXTRACT_VECTOR_ELT)
      .legalIf([=](const LegalityQuery &Q) {
        return Q.Types[0].getSizeInBits() == 16 &&
               Q.Types[1].getSizeInBits() == 32;
      });
  getLegacyLegalizerInfo().computeTables();
}

bool H2BLBLegalizerInfo::legalizeCustom(
    LegalizerHelper &Helper, MachineInstr &MI,
    LostDebugLocObserver &LocObserver) const {
  MachineIRBuilder &MIRBuilder = Helper.MIRBuilder;
  MachineRegisterInfo &MRI = *MIRBuilder.getMRI();
  GISelChangeObserver &Observer = Helper.Observer;
  switch (MI.getOpcode()) {
  default:
    // No idea what to do.
    return false;
  case TargetOpcode::G_MUL:
    return legalizeMul(MI, MRI, MIRBuilder, Observer);
  }
  llvm_unreachable("expected switch to return");
}

bool H2BLBLegalizerInfo::legalizeMul(MachineInstr &MI, MachineRegisterInfo &MRI,
                                     MachineIRBuilder &MIRBuilder,
                                     GISelChangeObserver &Observer) const {
  assert(MI.getOpcode() == TargetOpcode::G_MUL);

  Register ValReg = MI.getOperand(0).getReg();
  const LLT ValTy = MRI.getType(ValReg);
  (void)ValTy;
  assert(ValTy == LLT::scalar(32) &&
         "Custom legalization description doesn't match implementation");

  // Check if the MUL is fed by two s|zext and if so let is go through.
  Register LHS = MI.getOperand(1).getReg();
  Register RHS = MI.getOperand(2).getReg();
  Register PlainLHS, PlainRHS;
  bool isSigned;
  if (mi_match(LHS, MRI, m_GSExt(m_Reg(PlainLHS))) &&
      mi_match(RHS, MRI, m_GSExt(m_Reg(PlainRHS))))
    isSigned = true;
  else if (mi_match(LHS, MRI, m_GZExt(m_Reg(PlainLHS))) &&
           mi_match(RHS, MRI, m_GZExt(m_Reg(PlainRHS))))
    isSigned = false;
  else
    return false;

  LLT s16 = LLT::scalar(16);
  if (MRI.getType(PlainLHS) != s16 || MRI.getType(PlainRHS) != s16)
    return false;

  const TargetInstrInfo &TII = *ST.getInstrInfo();
  unsigned Opcode = isSigned ? H2BLB::WIDENING_SMUL : H2BLB::WIDENING_UMUL;
  Observer.changingInstr(MI);
  MI.setDesc(TII.get(Opcode));
  auto UpdateOperand = [](MachineOperand &MO, Register NewReg) {
    MO.setReg(NewReg);
    // The previous operand may have been the last use of the previous register.
    // This may not be the case of the NewReg, so conservatively drop the last
    // use flag.
    MO.setIsKill(false);
  };
  UpdateOperand(MI.getOperand(1), PlainLHS);
  UpdateOperand(MI.getOperand(2), PlainRHS);
  constrainSelectedInstRegOperands(MI, TII, *MRI.getTargetRegisterInfo(),
                                   *ST.getRegBankInfo());

  Observer.changedInstr(MI);
  return true;
}
