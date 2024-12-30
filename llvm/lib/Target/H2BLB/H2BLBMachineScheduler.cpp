//===- H2BLBMachineScheduler.cpp - MI Scheduler for H2BLB -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "H2BLBMachineScheduler.h"
#include "MCTargetDesc/H2BLBMCTargetDesc.h"

using namespace llvm;

bool H2BLBPreRASchedStrategy::tryCandidate(SchedCandidate &Cand,
                                           SchedCandidate &TryCand,
                                           SchedBoundary *Zone) const {
  bool BetterCand = GenericScheduler::tryCandidate(Cand, TryCand, Zone);
  // Try our specific heuristic only when TryCand isn't selected or
  // selected as node order.
  if (BetterCand && TryCand.Reason != NodeOrder && TryCand.Reason != NoCand)
    return true;

  // If we are in the same scheduling region use the fact that one of the
  // candidate is a widening mul to prioritize it.
  if (Zone != nullptr) {

    if (TryCand.SU->getInstr()->mayLoad()) {
      TryCand.Reason = Stall;
      return true;
    }
    unsigned Opc = TryCand.SU->getInstr()->getOpcode();
    if (Opc == H2BLB::WIDENING_SMUL) {
      TryCand.Reason = Stall;
      return true;
    }
  }

  return TryCand.Reason != NoCand;
}
