//===- H2BLBSchedModel.td - H2BLB scheduling model -----------*- tblgen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Describe the scheduling model of the H2BLB target.
//
//===----------------------------------------------------------------------===//

def H2BLBDefaultModel : SchedMachineModel {
  let IssueWidth = 1;
  let LoadLatency = 3;

  let CompleteModel = 0;
}
