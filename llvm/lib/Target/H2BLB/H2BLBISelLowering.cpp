//===-- H2BLBISelLowering.cpp - H2BLB DAG Lowering Implementation -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the interfaces that H2BLB uses to lower LLVM code
// into a selection DAG.
//
//===----------------------------------------------------------------------===//

#include "H2BLBISelLowering.h"
#include "H2BLBSubtarget.h"
#include "H2BLBTargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "h2blb-lowering"

H2BLBTargetLowering::H2BLBTargetLowering(const TargetMachine &TM)
    : TargetLowering(TM) {}

// Calling Convention Implementation
#include "H2BLBGenCallingConv.inc"
