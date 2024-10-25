//===- H2BLBRegisterBankInfo.h -----------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file declares the targeting of the RegisterBankInfo class for H2BLB.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBREGISTERBANKINFO_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBREGISTERBANKINFO_H

#include "MCTargetDesc/H2BLBMCTargetDesc.h"
#include "llvm/CodeGen/RegisterBankInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGBANK_DECLARATIONS
#include "H2BLBGenRegisterBank.inc"

namespace llvm {
class TargetRegisterInfo;

class H2BLBGenRegisterBankInfo : public RegisterBankInfo {
protected:
#define GET_TARGET_REGBANK_CLASS
#include "H2BLBGenRegisterBank.inc"
};

class H2BLBRegisterBankInfo final : public H2BLBGenRegisterBankInfo {
public:
  H2BLBRegisterBankInfo(const TargetRegisterInfo &TRI);
};
} // namespace llvm

#endif
