//=-- H2BLBTargetMachine.h - Define TargetMachine for H2BLB -------*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the H2BLB specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBTARGETMACHINE_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBTARGETMACHINE_H

#include "H2BLBSubtarget.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetMachine.h"
#include <memory>
#include <optional>

namespace llvm {

class H2BLBTargetMachine : public LLVMTargetMachine {
  mutable std::unique_ptr<H2BLBSubtarget> SubtargetSingleton;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

public:
  H2BLBTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     std::optional<Reloc::Model> RM,
                     std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                     bool JIT);
  ~H2BLBTargetMachine() override;

  const H2BLBSubtarget *getSubtargetImpl(const Function &F) const override;
  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  // Register the target specific passes that this backend offers.
  void registerPassBuilderCallbacks(PassBuilder &PB) override;
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
};

class H2BLBPassConfig : public TargetPassConfig {
public:
  H2BLBPassConfig(LLVMTargetMachine &TM, PassManagerBase &PM);

  bool addInstSelector() override;
};

} // end namespace llvm

#endif
