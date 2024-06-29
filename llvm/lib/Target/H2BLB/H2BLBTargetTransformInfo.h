//==- H2BLBTargetTransformInfo.cpp - H2BLB specific TTI pass -*- C++ -*-==//
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

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBTARGETTRANSFORMINFO_H

#include "H2BLBSubtarget.h"
#include "H2BLBTargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"

namespace llvm {

class H2BLBTTIImpl : public BasicTTIImplBase<H2BLBTTIImpl> {
  using BaseT = BasicTTIImplBase<H2BLBTTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  // Supply the minimum required APIs.
  const H2BLBSubtarget &ST;
  const H2BLBTargetLowering &TLI;

  const H2BLBSubtarget *getST() const { return &ST; }
  const H2BLBTargetLowering *getTLI() const { return &TLI; }

public:
  explicit H2BLBTTIImpl(const H2BLBTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(*TM->getSubtargetImpl(F)),
        TLI(*ST.getTargetLowering()) {}

  /// \name Vector TTI Implementations
  /// @{
  unsigned getLoadVectorFactor(unsigned VF, unsigned LoadSize,
                               unsigned ChainSizeInBytes,
                               VectorType *VecTy) const;
  /// @}
};

} // end namespace llvm
#endif // LLVM_LIB_TARGET_H2BLB_H2BLBTARGETTRANSFORMINFO_H
