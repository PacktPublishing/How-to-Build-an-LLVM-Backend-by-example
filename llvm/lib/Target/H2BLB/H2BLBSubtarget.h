//===- H2BLBSubtarget.h - Define Subtarget for the H2BLB ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the H2BLB specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBSUBTARGET_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBSUBTARGET_H

#include "H2BLBISelLowering.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

namespace llvm {

class TargetMachine;
class Triple;

class H2BLBSubtarget : public TargetSubtargetInfo {
  virtual void anchor();
  H2BLBTargetLowering TLInfo;

public:
  H2BLBSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                 const TargetMachine &TM);
  const H2BLBTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_H2BLB_H2BLBSUBTARGET_H
