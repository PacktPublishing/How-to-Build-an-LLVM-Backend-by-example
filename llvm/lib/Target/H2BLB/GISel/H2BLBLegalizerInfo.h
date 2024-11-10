//===- H2BLBLegalizerInfo.h --------------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file declares the targeting of the Machinelegalizer class for H2BLB
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_GISEL_H2BLBMACHINELEGALIZER_H
#define LLVM_LIB_TARGET_H2BLB_GISEL_H2BLBMACHINELEGALIZER_H

#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"

namespace llvm {

class H2BLBSubtarget;

/// This class provides the information for the H2BLB target legalizer for
/// GlobalISel.
class H2BLBLegalizerInfo : public LegalizerInfo {
public:
  H2BLBLegalizerInfo(const H2BLBSubtarget &ST);
};
} // namespace llvm
#endif
