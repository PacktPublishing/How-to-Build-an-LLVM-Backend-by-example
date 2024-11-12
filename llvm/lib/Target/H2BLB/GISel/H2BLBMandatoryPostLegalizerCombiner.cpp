//=== H2BLBMandatoryPostLegalizerCombiner.cpp -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Post-legalization lowering for instructions.
///
/// This is used to offload pattern matching from the selector.
///
/// For example, this combiner will notice that a G_UNMERGE is actually
/// a G_EXTRACT_VECTOR_ELT, etc.
///
//===----------------------------------------------------------------------===//

#include "H2BLB.h"
#include "H2BLBSubtarget.h"
#include "H2BLBTargetMachine.h"
#include "MCTargetDesc/H2BLBMCTargetDesc.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/GISelChangeObserver.h"
#include "llvm/CodeGen/GlobalISel/GenericMachineInstrs.h"
#include "llvm/CodeGen/GlobalISel/LegalizerHelper.h"
#include "llvm/CodeGen/GlobalISel/MIPatternMatch.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/GlobalISel/Utils.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include <optional>

#define GET_GICOMBINER_DEPS
#include "H2BLBGenMandatoryPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_DEPS

#define DEBUG_TYPE "h2blb-mandatory-postlegalizer-combiner"

using namespace llvm;
using namespace MIPatternMatch;

namespace {

#define GET_GICOMBINER_TYPES
#include "H2BLBGenMandatoryPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_TYPES

bool matchScalarizeVectorUnmerge(MachineInstr &MI, MachineRegisterInfo &MRI) {
  auto &Unmerge = cast<GUnmerge>(MI);
  Register Src1Reg = Unmerge.getReg(Unmerge.getNumOperands() - 1);
  const LLT SrcTy = MRI.getType(Src1Reg);
  if (SrcTy.getSizeInBits() != 32)
    return false;
  return SrcTy.isVector() && !SrcTy.isScalable() &&
         Unmerge.getNumOperands() == (unsigned)SrcTy.getNumElements() + 1;
}

void applyScalarizeVectorUnmerge(MachineInstr &MI, MachineRegisterInfo &MRI,
                                 MachineIRBuilder &B) {
  auto &Unmerge = cast<GUnmerge>(MI);
  Register Src1Reg = Unmerge.getReg(Unmerge.getNumOperands() - 1);
  const LLT SrcTy = MRI.getType(Src1Reg);
  assert((SrcTy.isVector() && !SrcTy.isScalable()) &&
         "Expected a fixed length vector");

  for (int I = 0; I < SrcTy.getNumElements(); ++I)
    B.buildExtractVectorElementConstant(Unmerge.getReg(I), Src1Reg, I);
  MI.eraseFromParent();
}

class H2BLBMandatoryPostLegalizerCombinerImpl : public Combiner {
protected:
  mutable CombinerHelper Helper;
  const H2BLBMandatoryPostLegalizerCombinerImplRuleConfig &RuleConfig;
  const H2BLBSubtarget &STI;

public:
  H2BLBMandatoryPostLegalizerCombinerImpl(
      MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
      GISelCSEInfo *CSEInfo,
      const H2BLBMandatoryPostLegalizerCombinerImplRuleConfig &RuleConfig,
      const H2BLBSubtarget &STI);

  static const char *getName() { return "H2BLBMandatoryPostLegalizerCombiner"; }

  bool tryCombineAll(MachineInstr &I) const override;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "H2BLBGenMandatoryPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "H2BLBGenMandatoryPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_IMPL

H2BLBMandatoryPostLegalizerCombinerImpl::
    H2BLBMandatoryPostLegalizerCombinerImpl(
        MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
        GISelCSEInfo *CSEInfo,
        const H2BLBMandatoryPostLegalizerCombinerImplRuleConfig &RuleConfig,
        const H2BLBSubtarget &STI)
    : Combiner(MF, CInfo, TPC, /*KB*/ nullptr, CSEInfo),
      Helper(Observer, B, /*IsPreLegalize*/ false), RuleConfig(RuleConfig),
      STI(STI),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "H2BLBGenMandatoryPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

class H2BLBMandatoryPostLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  H2BLBMandatoryPostLegalizerCombiner();

  StringRef getPassName() const override {
    return "H2BLBMandatoryPostLegalizerCombiner";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  H2BLBMandatoryPostLegalizerCombinerImplRuleConfig RuleConfig;
};
} // end anonymous namespace

void H2BLBMandatoryPostLegalizerCombiner::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  MachineFunctionPass::getAnalysisUsage(AU);
}

H2BLBMandatoryPostLegalizerCombiner::H2BLBMandatoryPostLegalizerCombiner()
    : MachineFunctionPass(ID) {
  if (!RuleConfig.parseCommandLineOption())
    report_fatal_error("Invalid rule identifier");
}

bool H2BLBMandatoryPostLegalizerCombiner::runOnMachineFunction(
    MachineFunction &MF) {
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::FailedISel))
    return false;
  assert(MF.getProperties().hasProperty(
             MachineFunctionProperties::Property::Legalized) &&
         "Expected a legalized function?");
  auto *TPC = &getAnalysis<TargetPassConfig>();
  const Function &F = MF.getFunction();

  const H2BLBSubtarget &ST = MF.getSubtarget<H2BLBSubtarget>();
  CombinerInfo CInfo(/*AllowIllegalOps*/ false, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, /*OptEnabled=*/true,
                     F.hasOptSize(), F.hasMinSize());
  H2BLBMandatoryPostLegalizerCombinerImpl Impl(
      MF, CInfo, TPC, /*CSEInfo*/ nullptr, RuleConfig, ST);
  return Impl.combineMachineInstrs();
}

char H2BLBMandatoryPostLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(H2BLBMandatoryPostLegalizerCombiner, DEBUG_TYPE,
                      "Lower H2BLB MachineInstrs after legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_END(H2BLBMandatoryPostLegalizerCombiner, DEBUG_TYPE,
                    "Lower H2BLB MachineInstrs after legalization", false,
                    false)

namespace llvm {
Pass *createH2BLBMandatoryPostLegalizerCombiner() {
  return new H2BLBMandatoryPostLegalizerCombiner();
}
} // end namespace llvm
