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
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGenTypes/LowLevelType.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "h2blb-legalinfo"

using namespace llvm;
using namespace LegalizeActions;

H2BLBLegalizerInfo::H2BLBLegalizerInfo(const H2BLBSubtarget &ST) {
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
      .clampScalar(0, s16, s32);

  // Pointer-handling.
  getActionDefinitionsBuilder(TargetOpcode::G_FRAME_INDEX).legalFor({p0});
  getActionDefinitionsBuilder(TargetOpcode::G_PTR_ADD).legalFor({{p0, s16}});

  // Arithmetic.
  getActionDefinitionsBuilder(TargetOpcode::G_ADD)
      .legalFor({s16, s32})
      .clampScalar(0, s16, s32);

  getLegacyLegalizerInfo().computeTables();
}
