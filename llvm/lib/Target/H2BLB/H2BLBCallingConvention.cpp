//=== H2BLBCallingConvention.cpp - H2BLB CC impl ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the table-generated and custom routines for the H2BLB
// Calling Convention.
//
//===----------------------------------------------------------------------===//

#include "H2BLBCallingConvention.h"
#include "MCTargetDesc/H2BLBMCTargetDesc.h" // For H2BLB reg enums.
using namespace llvm;

// TableGen provides definitions of the calling convention analysis entry
// points.
#include "H2BLBGenCallingConv.inc"
