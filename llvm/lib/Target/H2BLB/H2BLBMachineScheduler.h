//===- H2BLBMachineScheduler.h - Custom H2BLB MI scheduler ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Custom H2BLB MI scheduler.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBMACHINESCHEDULER_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBMACHINESCHEDULER_H

#include "llvm/CodeGen/MachineScheduler.h"

namespace llvm {

/// A MachineSchedStrategy implementation for H2BLB pre RA scheduling.
class H2BLBPreRASchedStrategy : public GenericScheduler {
public:
  H2BLBPreRASchedStrategy(const MachineSchedContext *C) : GenericScheduler(C) {}

protected:
  bool tryCandidate(SchedCandidate &Cand, SchedCandidate &TryCand,
                    SchedBoundary *Zone) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_H2BLB_H2BLBMACHINESCHEDULER_H
