//===-- H2BLBInstrInfo.cpp - H2BLB Instruction Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the H2BLB implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "H2BLBInstrInfo.h"
#include "H2BLB.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <iterator>

#define GET_INSTRINFO_CTOR_DTOR
#include "H2BLBGenInstrInfo.inc"

using namespace llvm;

H2BLBInstrInfo::H2BLBInstrInfo() : H2BLBGenInstrInfo() {}
