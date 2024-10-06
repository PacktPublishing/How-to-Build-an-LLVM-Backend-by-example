//=== H2BLBCallingConvention.h - H2BLB CC entry points ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the entry points for H2BLB calling convention analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBCALLINGCONVENTION_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBCALLINGCONVENTION_H

#include "llvm/CodeGen/CallingConvLower.h"

namespace llvm {
bool CC_H2BLB_Common(unsigned ValNo, MVT ValVT, MVT LocVT,
                     CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                     CCState &State);
bool RetCC_H2BLB_Common(unsigned ValNo, MVT ValVT, MVT LocVT,
                        CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                        CCState &State);
} // namespace llvm

#endif
