//=== H2BLBMandatoryPreLegalizerCombiner.cpp ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass does combining of machine instructions at the generic MI level,
// before the legalizer.
//
//===----------------------------------------------------------------------===//

#include "H2BLB.h"
#include "H2BLBTargetMachine.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/GlobalISel/MIPatternMatch.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"

#define GET_GICOMBINER_DEPS
#include "H2BLBGenMandatoryPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_DEPS

#define DEBUG_TYPE "h2blb-mandatory-prelegalizer-combiner"

using namespace llvm;
using namespace MIPatternMatch;
namespace {
#define GET_GICOMBINER_TYPES
#include "H2BLBGenMandatoryPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_TYPES

bool matchInsertVectorElt(MachineInstr &MI,
                          SmallVectorImpl<Register> &MatchInfo) {
  assert(MI.getOpcode() == TargetOpcode::G_INSERT_VECTOR_ELT);
  // Try to find each element of the vector to produce a BUILD_VECTOR.
  Register DstReg = MI.getOperand(0).getReg();
  const MachineRegisterInfo &MRI = MI.getParent()->getParent()->getRegInfo();
  uint16_t NumElts = MRI.getType(DstReg).getNumElements();
  MatchInfo.resize(NumElts);
  BitVector IndicesFound(NumElts);
  MachineInstr *CurMI = &MI;
  do {
    switch (CurMI->getOpcode()) {
    case TargetOpcode::G_INSERT_VECTOR_ELT: {
      std::optional<ValueAndVReg> MaybeVectorLane;
      if (!mi_match(CurMI->getOperand(3).getReg(), MRI,
                    m_GCst(MaybeVectorLane)))
        return false;
      uint64_t VectorLaneIdx = MaybeVectorLane->Value.getZExtValue();
      // If we already found the contain of this lane, move on.
      // We're following the use-def chain so by construction the previous
      // definition that set this lane is the right one.
      if (!IndicesFound.test(VectorLaneIdx)) {
        IndicesFound.set(VectorLaneIdx);
        MatchInfo[VectorLaneIdx] = CurMI->getOperand(2).getReg();
      }
      CurMI = MRI.getUniqueVRegDef(CurMI->getOperand(1).getReg());
      break;
    }
    case TargetOpcode::G_BUILD_VECTOR: {
      for (const auto &[Idx, MO] : enumerate(CurMI->operands())) {
        // Skip the definition
        if (Idx == 0)
          continue;
        // Adjust Idx to match the vector lane index.
        --Idx;
        if (!IndicesFound.test(Idx)) {
          MatchInfo[Idx] = MO.getReg();
          IndicesFound.set(Idx);
        }
      }
      CurMI = nullptr;
      break;
    }
    case TargetOpcode::IMPLICIT_DEF:
    case TargetOpcode::G_IMPLICIT_DEF: {
      BitVector MissingIndices = IndicesFound.flip();
      for (unsigned Idx : MissingIndices.set_bits()) {
        MatchInfo[Idx] = H2BLB::NoRegister;
      }
      IndicesFound = BitVector(NumElts, true);
    }
      [[fallthrough]];
    default:
      CurMI = nullptr;
      break;
    }
  } while (CurMI != nullptr && !IndicesFound.all());
  return IndicesFound.all();
}

void applyInsertVectorElt(MachineInstr &MI,
                          SmallVectorImpl<Register> &MatchInfo) {
  assert(MI.getOpcode() == TargetOpcode::G_INSERT_VECTOR_ELT);
  MachineIRBuilder MIB(MI);
  Register UndefReg;
  Register DstReg = MI.getOperand(0).getReg();
  for (Register &Reg : MatchInfo) {
    if (Reg == H2BLB::NoRegister) {
      if (UndefReg == H2BLB::NoRegister) {
        LLT EltTy = MIB.getMRI()->getType(DstReg).getElementType();
        UndefReg = MIB.buildUndef(EltTy).getReg(0);
      }
      Reg = UndefReg;
    }
  }

  MIB.buildBuildVector(DstReg, MatchInfo);
  MI.eraseFromParent();
}

class H2BLBMandatoryPreLegalizerCombinerImpl : public Combiner {
protected:
  mutable CombinerHelper Helper;
  const H2BLBMandatoryPreLegalizerCombinerImplRuleConfig &RuleConfig;
  const H2BLBSubtarget &STI;

public:
  H2BLBMandatoryPreLegalizerCombinerImpl(
      MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
      GISelKnownBits &KB, GISelCSEInfo *CSEInfo,
      const H2BLBMandatoryPreLegalizerCombinerImplRuleConfig &RuleConfig,
      const H2BLBSubtarget &STI);

  static const char *getName() { return "H2BLBMandatoryPreLegalizerCombiner"; }

  bool tryCombineAll(MachineInstr &I) const override;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "H2BLBGenMandatoryPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "H2BLBGenMandatoryPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_IMPL

H2BLBMandatoryPreLegalizerCombinerImpl::H2BLBMandatoryPreLegalizerCombinerImpl(
    MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
    GISelKnownBits &KB, GISelCSEInfo *CSEInfo,
    const H2BLBMandatoryPreLegalizerCombinerImplRuleConfig &RuleConfig,
    const H2BLBSubtarget &STI)
    : Combiner(MF, CInfo, TPC, &KB, CSEInfo),
      Helper(Observer, B, /*IsPreLegalize*/ true, &KB), RuleConfig(RuleConfig),
      STI(STI),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "H2BLBGenMandatoryPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

// Pass boilerplate
// ================

class H2BLBMandatoryPreLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  H2BLBMandatoryPreLegalizerCombiner();

  StringRef getPassName() const override {
    return "H2BLBMandatoryPreLegalizerCombiner";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  H2BLBMandatoryPreLegalizerCombinerImplRuleConfig RuleConfig;
};
} // end anonymous namespace

void H2BLBMandatoryPreLegalizerCombiner::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  AU.addRequired<GISelKnownBitsAnalysis>();
  AU.addPreserved<GISelKnownBitsAnalysis>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

H2BLBMandatoryPreLegalizerCombiner::H2BLBMandatoryPreLegalizerCombiner()
    : MachineFunctionPass(ID) {
  initializeH2BLBMandatoryPreLegalizerCombinerPass(
      *PassRegistry::getPassRegistry());

  if (!RuleConfig.parseCommandLineOption())
    report_fatal_error("Invalid rule identifier");
}

bool H2BLBMandatoryPreLegalizerCombiner::runOnMachineFunction(
    MachineFunction &MF) {
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::FailedISel))
    return false;
  auto &TPC = getAnalysis<TargetPassConfig>();

  const Function &F = MF.getFunction();
  GISelKnownBits *KB = &getAnalysis<GISelKnownBitsAnalysis>().get(MF);

  const H2BLBSubtarget &ST = MF.getSubtarget<H2BLBSubtarget>();

  CombinerInfo CInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, /*EnableOpt*/ false,
                     F.hasOptSize(), F.hasMinSize());
  // Disable fixed-point iteration in the Combiner. This improves compile-time
  // at the cost of possibly missing optimizations. See PR#94291 for details.
  CInfo.MaxIterations = 1;

  H2BLBMandatoryPreLegalizerCombinerImpl Impl(MF, CInfo, &TPC, *KB,
                                              /*CSEInfo*/ nullptr, RuleConfig,
                                              ST);
  return Impl.combineMachineInstrs();
}

char H2BLBMandatoryPreLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(H2BLBMandatoryPreLegalizerCombiner, DEBUG_TYPE,
                      "Combine H2BLB machine instrs before legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelKnownBitsAnalysis)
INITIALIZE_PASS_DEPENDENCY(GISelCSEAnalysisWrapperPass)
INITIALIZE_PASS_END(H2BLBMandatoryPreLegalizerCombiner, DEBUG_TYPE,
                    "Combine H2BLB machine instrs before legalization", false,
                    false)

namespace llvm {
Pass *createH2BLBMandatoryPreLegalizerCombiner() {
  return new H2BLBMandatoryPreLegalizerCombiner();
}
} // end namespace llvm
