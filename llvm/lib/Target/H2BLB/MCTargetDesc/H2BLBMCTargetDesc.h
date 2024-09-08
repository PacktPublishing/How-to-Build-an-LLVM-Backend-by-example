//===-- H2BLBMCTargetDesc.h - H2BLB Target Descriptions ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// Provides H2BLB specific target descriptions.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_LIB_TARGET_H2BLB_MCTARGETDESC_H2BLBMCTARGETDESC_H
#define LLVM_LIB_TARGET_H2BLB_MCTARGETDESC_H2BLBMCTARGETDESC_H

#include <cstdint> // For int16_t and so on used in the .inc files.

// Defines symbolic names for H2BLB registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "H2BLBGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "H2BLBGenSubtargetInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "H2BLBGenInstrInfo.inc"

#endif
