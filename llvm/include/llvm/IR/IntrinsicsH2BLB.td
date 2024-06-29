//===- IntrinsicsH2BLB.td - Defines H2BLB intrinsics -------*- tablegen -*-===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the H2BLB-specific intrinsics.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Definitions for all H2BLB intrinsics.
//
// All H2BLB intrinsics start with "llvm.h2blb.".
let TargetPrefix = "h2blb" in {
  def int_h2blb_widening_smul :
  Intrinsic<[llvm_i32_ty], [llvm_i16_ty, llvm_i16_ty]>;
  def int_h2blb_widening_umul :
  Intrinsic<[llvm_i32_ty], [llvm_i16_ty, llvm_i16_ty]>;
} // end TargetPrefix = "h2blb"
