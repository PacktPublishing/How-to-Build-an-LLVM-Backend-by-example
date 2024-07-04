//===-- MCTargetDesc/H2BLBMCAsmInfo.h - H2BLB MCAsm Interface ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// Declaration of the H2BLB MCAsmInfos.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_MCTARGETDESC_H2BLBMCASMINFO_H
#define LLVM_LIB_TARGET_H2BLB_MCTARGETDESC_H2BLBMCASMINFO_H

#include "llvm/MC/MCAsmInfoDarwin.h"
#include "llvm/MC/MCAsmInfoELF.h"
namespace llvm {

class Triple;

class H2BLBMCAsmInfoELF : public MCAsmInfoELF {
public:
  explicit H2BLBMCAsmInfoELF(const Triple &TT, const MCTargetOptions &Options);
};

class H2BLBMCAsmInfoDarwin : public MCAsmInfoDarwin {
public:
  explicit H2BLBMCAsmInfoDarwin(const Triple &TT,
                                const MCTargetOptions &Options);
};
} // namespace llvm
#endif
