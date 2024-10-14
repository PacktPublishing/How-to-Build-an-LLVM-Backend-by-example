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
#include "H2BLB.h"
#include "H2BLBCallingConvention.h"
#include "H2BLBSubtarget.h"
#include "H2BLBTargetMachine.h"

#include "llvm/CodeGen/MachineFrameInfo.h"

using namespace llvm;

#define DEBUG_TYPE "h2blb-lowering"

H2BLBTargetLowering::H2BLBTargetLowering(const TargetMachine &TM,
                                         const H2BLBSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  addRegisterClass(MVT::i16, &H2BLB::GPR16RegClass);
  addRegisterClass(MVT::f16, &H2BLB::GPR16RegClass);
  addRegisterClass(MVT::i32, &H2BLB::GPR32RegClass);
  addRegisterClass(MVT::f32, &H2BLB::GPR32RegClass);

  // Tell the generic implementation that we are done with setting up our
  // register classes.
  computeRegisterProperties(Subtarget.getRegisterInfo());
}

FastISel *
H2BLBTargetLowering::createFastISel(FunctionLoweringInfo &funcInfo,
                                    const TargetLibraryInfo *libInfo) const {
  return H2BLB::createFastISel(funcInfo, libInfo);
}

SDValue
H2BLBTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                 bool IsVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 const SDLoc &DL, SelectionDAG &DAG) const {

  SmallVector<CCValAssign> RetValLocs;
  MachineFunction &MF = DAG.getMachineFunction();

  CCState CCInfo(CallConv, IsVarArg, MF, RetValLocs, *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_H2BLB_Common);

  SDValue Glue;
  SmallVector<SDValue> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (size_t i = 0, e = RetValLocs.size(); i != e; ++i) {
    CCValAssign &VA = RetValLocs[i];
    assert(VA.isRegLoc() && "stack return not yet implemented");
    assert(VA.getLocInfo() == CCValAssign::Full &&
           "extension/truncation of any sort, not yet implemented");

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Glue);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  // The return must have the last value of the chain.
  // Update it now.
  RetOps[0] = Chain;

  // Add the glue if we have it.
  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(H2BLBISD::RETURN_GLUE, DL, MVT::Other, RetOps);
}

SDValue H2BLBTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  if (IsVarArg)
    report_fatal_error("variadic functions, not yet implemented");

  MachineFunction &MF = DAG.getMachineFunction();
  if (MF.getFunction().hasStructRetAttr())
    report_fatal_error("aggregate returns, not yet implemented");

  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_H2BLB_Common);

  for (size_t I = 0; I < ArgLocs.size(); ++I) {
    auto &VA = ArgLocs[I];
    SDValue ArgValue;

    if (VA.isRegLoc()) {
      if (VA.getLocInfo() != CCValAssign::Full)
        report_fatal_error("partial type, not yet implemented");

      // Arguments passed in registers
      EVT RegVT = VA.getLocVT();
      TypeSize TySizeInBits = RegVT.getSizeInBits();
      const TargetRegisterClass *DstRC = nullptr;
      switch (TySizeInBits) {
      default:
        report_fatal_error("argument type, not yet implemented");
      case 16:
        DstRC = &H2BLB::GPR16RegClass;
        break;
      case 32:
        DstRC = &H2BLB::GPR32RegClass;
        break;
      }
      Register VReg = RegInfo.createVirtualRegister(DstRC);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

    } else {
      assert(VA.isMemLoc() && "CCValAssign is neither reg nor mem");
      unsigned ArgOffset = VA.getLocMemOffset();
      if (VA.getLocInfo() != CCValAssign::Full)
        report_fatal_error("support only value directly in the stack");
      unsigned ArgSize = VA.getValVT().getSizeInBits() / 8;

      int FrameIdx =
          MFI.CreateFixedObject(ArgSize, ArgOffset, /*IsImmutable=*/true);

      // Create load nodes to retrieve arguments from the stack.
      SDValue FrameIdxNode =
          DAG.getFrameIndex(FrameIdx, getPointerTy(DAG.getDataLayout()));
      MachinePointerInfo PtrInfo =
          MachinePointerInfo::getFixedStack(MF, FrameIdx);

      // We support only full loads for now, so no extension whatsoever.
      ISD::LoadExtType ExtType = ISD::NON_EXTLOAD;
      MVT MemVT = VA.getValVT();

      ArgValue = DAG.getExtLoad(ExtType, DL, VA.getLocVT(), Chain, FrameIdxNode,
                                PtrInfo, MemVT);
    }
    InVals.push_back(ArgValue);
  }

  return Chain;
}

void H2BLBTargetLowering::finalizeLowering(MachineFunction &MF) const {
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  Register SavedLR = MRI.createVirtualRegister(&H2BLB::GPR16RegClass);
  Register LR = H2BLB::R0;

  MachineBasicBlock &EntryMBB = MF.front();
  BuildMI(EntryMBB, EntryMBB.begin(), DebugLoc(), TII.get(TargetOpcode::COPY),
          SavedLR)
      .addReg(LR);
  EntryMBB.addLiveIn(LR);

  for (MachineBasicBlock &MaybeExitMBB : MF) {
    if (!MaybeExitMBB.succ_empty())
      continue;
    assert(MaybeExitMBB.getFirstTerminator() != MaybeExitMBB.end() &&
           "Exit block must have a terminator");
    assert(MaybeExitMBB.getFirstTerminator()->getOpcode() == H2BLB::RETURN &&
           "Exit block must end with return");
    BuildMI(MaybeExitMBB, MaybeExitMBB.getFirstTerminator(), DebugLoc(),
            TII.get(TargetOpcode::COPY), LR)
        .addReg(SavedLR);
  }

  TargetLowering::finalizeLowering(MF);
}

const char *H2BLBTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((H2BLBISD::NodeType)Opcode) {
  case H2BLBISD::FIRST_NUMBER:
    break;
  case H2BLBISD::RETURN_GLUE:
    return "H2BLBISD::RETURN_GLUE";
  case H2BLBISD::CALL:
    return "H2BLBISD::CALL";
  }
  return nullptr;
}
