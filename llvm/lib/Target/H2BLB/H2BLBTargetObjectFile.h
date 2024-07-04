//===-- H2BLBTargetObjectFile.h - H2BLB Object Info -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

/// This implementation is used for H2BLB ELF targets (Linux in particular).
class H2BLB_ELFTargetObjectFile : public TargetLoweringObjectFileELF {
public:
  H2BLB_ELFTargetObjectFile();
};

/// This TLOF implementation is used for Darwin.
class H2BLB_MachoTargetObjectFile : public TargetLoweringObjectFileMachO {
public:
  H2BLB_MachoTargetObjectFile();
};

} // end namespace llvm

#endif
