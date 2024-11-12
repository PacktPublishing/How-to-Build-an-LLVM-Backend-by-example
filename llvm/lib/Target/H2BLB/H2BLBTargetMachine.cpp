//===-- H2BLBTargetMachine.cpp - Define TargetMachine for H2BLB -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about H2BLB target spec.
//
//===----------------------------------------------------------------------===//

#include "H2BLBTargetMachine.h"
#include "H2BLB.h"
#include "H2BLBTargetObjectFile.h"
#include "H2BLBTargetTransformInfo.h"
#include "TargetInfo/H2BLBTargetInfo.h" // For getTheH2BLBTarget.
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/InitializePasses.h" // For initializeGlobalISel.
#include "llvm/MC/TargetRegistry.h" // For RegisterTargetMachine.
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CodeGen.h"  // For CodeGenOptLevel.
#include "llvm/Support/Compiler.h" // For LLVM_EXTERNAL_VISIBILITY.
#include <memory>

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeH2BLBTarget() {
  // Register the target so that external tools can instantiate it.
  RegisterTargetMachine<H2BLBTargetMachine> X(getTheH2BLBTarget());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeH2BLBSimpleConstantPropagationPass(PR);
  initializeH2BLBMandatoryPreLegalizerCombinerPass(PR);
  initializeH2BLBMandatoryPostLegalizerCombinerPass(PR);
  initializeGlobalISel(PR);
}

static std::unique_ptr<TargetLoweringObjectFile> createTLOF(const Triple &TT) {
  if (TT.isOSBinFormatELF())
    return std::make_unique<H2BLB_ELFTargetObjectFile>();
  if (TT.isOSBinFormatMachO())
    return std::make_unique<H2BLB_MachoTargetObjectFile>();
  // Other format not supported yet.
  return nullptr;
}

// TODO: Share this with Clang.
static const char *H2BLBDataLayoutStr =
    "e-p:16:16:16-n16:32-i32:32:32-i16:16:16-i1:8:8-f32:32:32-v32:32:32";

H2BLBTargetMachine::H2BLBTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       std::optional<Reloc::Model> RM,
                                       std::optional<CodeModel::Model> CM,
                                       CodeGenOptLevel OL, bool JIT)
    : LLVMTargetMachine(T, H2BLBDataLayoutStr, TT, CPU, FS, Options,
                        // Use the simplest relocation by default.
                        RM ? *RM : Reloc::Static, CM ? *CM : CodeModel::Small,
                        OL),
      TLOF(createTLOF(getTargetTriple())) {
  initAsmInfo();
}

H2BLBTargetMachine::~H2BLBTargetMachine() = default;

const H2BLBSubtarget *
H2BLBTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  StringRef CPU = CPUAttr.isValid() ? CPUAttr.getValueAsString() : TargetCPU;
  StringRef FS = FSAttr.isValid() ? FSAttr.getValueAsString() : TargetFS;

  // Eventually, we'll want to hook up a different subtarget based on at the
  // target feature, target cpu, and tune cpu attached to F, but as of now,
  // the target doesn't support anything fancy so we just have one subtarget
  // for everything.
  if (!SubtargetSingleton)
    SubtargetSingleton =
        std::make_unique<H2BLBSubtarget>(TargetTriple, CPU, FS, *this);
  return SubtargetSingleton.get();
}

TargetTransformInfo
H2BLBTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(H2BLBTTIImpl(this, F));
}

void H2BLBTargetMachine::registerPassBuilderCallbacks(PassBuilder &PB) {
#define GET_PASS_REGISTRY "H2BLBPassRegistry.def"
#include "llvm/Passes/TargetPassRegistry.inc"

  PB.registerPipelineStartEPCallback(
      [](ModulePassManager &MPM, OptimizationLevel OptLevel) {
        // Do not add optimization passes if we are in O0.
        if (OptLevel == OptimizationLevel::O0)
          return;
        FunctionPassManager FPM;
        FPM.addPass(H2BLBSimpleConstantPropagationNewPass());
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
      });
}

TargetPassConfig *H2BLBTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new H2BLBPassConfig(*this, PM);
}

H2BLBPassConfig::H2BLBPassConfig(LLVMTargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

bool H2BLBPassConfig::addInstSelector() {
  addPass(createH2BLBISelDAG(getH2BLBTargetMachine()));
  return false;
}

bool H2BLBPassConfig::addIRTranslator() {
  addPass(new IRTranslator(getOptLevel()));
  return false;
}

void H2BLBPassConfig::addPreLegalizeMachineIR() {
  addPass(createH2BLBMandatoryPreLegalizerCombiner());
}

bool H2BLBPassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

void H2BLBPassConfig::addPreRegBankSelect() {
  addPass(createH2BLBMandatoryPostLegalizerCombiner());
}

bool H2BLBPassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}

bool H2BLBPassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect(getOptLevel()));
  return false;
}

void H2BLBPassConfig::addIRPasses() {
  // Add the regular IR passes before putting our passes.
  TargetPassConfig::addIRPasses();
  if (getOptLevel() != CodeGenOptLevel::None)
    addPass(createH2BLBSimpleConstantPropagationPassForLegacyPM());
}
