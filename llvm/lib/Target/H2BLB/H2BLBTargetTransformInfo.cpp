//===- H2BLBTargetTransformInfo.cpp - H2BLB specific TTI pass ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
/// \file
/// This file implements a TargetTransformInfo analysis pass specific to the
/// H2BLB target machine. It uses the target's detailed information to provide
/// more precise answers to certain TTI queries, while letting the target
/// independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#include "H2BLBTargetTransformInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/IntrinsicsH2BLB.h"

using namespace llvm;

#define DEBUG_TYPE "h2blbtti"

unsigned H2BLBTTIImpl::getLoadVectorFactor(unsigned VF, unsigned LoadSize,
                                           unsigned ChainSizeInBytes,
                                           VectorType *VecTy) const {
  // We support <2 x i16> loads.
  unsigned ElemSize = VecTy->getScalarSizeInBits();
  if (ElemSize != 16)
    return 0;

  return std::min(VF, 2u);
}
