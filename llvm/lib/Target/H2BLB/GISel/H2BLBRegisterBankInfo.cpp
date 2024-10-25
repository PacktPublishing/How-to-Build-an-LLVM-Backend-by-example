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

H2BLBRegisterBankInfo::H2BLBRegisterBankInfo(const TargetRegisterInfo &TRI)
    : H2BLBGenRegisterBankInfo() {}
