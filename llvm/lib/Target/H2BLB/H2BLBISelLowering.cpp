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
  addRegisterClass(MVT::v2i16, &H2BLB::GPR32RegClass);

  setOperationAction(ISD::ADD, MVT::v2i16, Expand);

  // The only truncstore we have is from i16 to i8.
  setTruncStoreAction(MVT::i32, MVT::i16, Expand);

  setOperationAction(ISD::FADD, MVT::f32, LibCall);

  setOperationAction(ISD::MUL, MVT::i32, Custom);

  // Tell the generic implementation that we are done with setting up our
  // register classes.
  computeRegisterProperties(Subtarget.getRegisterInfo());
}

FastISel *
H2BLBTargetLowering::createFastISel(FunctionLoweringInfo &funcInfo,
                                    const TargetLibraryInfo *libInfo) const {
  return H2BLB::createFastISel(funcInfo, libInfo);
}

bool H2BLBTargetLowering::allowsMisalignedMemoryAccesses(
    EVT VT, unsigned AddrSpace, Align Alignment, MachineMemOperand::Flags Flags,
    unsigned *Fast) const {
  return true;
}

bool H2BLBTargetLowering::allowsMisalignedMemoryAccesses(
    LLT Ty, unsigned AddrSpace, Align Alignment, MachineMemOperand::Flags Flags,
    unsigned *Fast) const {
  return true;
}

bool H2BLBTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context) const {
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs,
                 MF.getFunction().getContext());

  return !IsVarArg && CCInfo.CheckReturn(Outs, RetCC_H2BLB_Common);
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
    assert((VA.getLocInfo() == CCValAssign::Full ||
            VA.getLocInfo() == CCValAssign::BCvt) &&
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
      if (VA.getLocInfo() != CCValAssign::Full &&
          VA.getLocInfo() != CCValAssign::BCvt)
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

SDValue H2BLBTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                       SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins = CLI.Ins;

  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  MachineFunction &MF = DAG.getMachineFunction();
  SDLoc &DL = CLI.DL;

  // H2BLB target does not support tail call optimization.
  CLI.IsTailCall = false;

  // Ditto for variadic arguments, though unlike tail calls, this is not
  // an optimization, therefore if it is requested, we must bail out.
  if (CLI.IsVarArg)
    report_fatal_error("Var args not yet implemented");
  bool IsVarArg = false;

  switch (CallConv) {
  default:
    report_fatal_error("unsupported calling convention: " + Twine(CallConv));
  case CallingConv::Fast:
  case CallingConv::C:
    break;
  }

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_H2BLB_Common);

  unsigned NumBytes = CCInfo.getStackSize();

  // FIXME: Technically we would nedd to check that we support the
  // flags requested in Outs.
  for (const ISD::OutputArg &Out : Outs) {
    ISD::ArgFlagsTy OutFlags = Out.Flags;
    if (OutFlags.isByVal())
      report_fatal_error("Unsupported attribute");
  }

  SDValue InGlue;
  // Now that we collected all the registers, start the call sequence.
  auto PtrVT = getPointerTy(MF.getDataLayout());
  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SDValue StackPtr = DAG.getCopyFromReg(Chain, DL, H2BLB::SP,
                                        getPointerTy(DAG.getDataLayout()));

  SmallVector<std::pair<Register, SDValue>> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  // Walk arg assignments
  for (size_t i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue &Arg = OutVals[i];

    if (VA.getLocInfo() != CCValAssign::Full)
      report_fatal_error("extensions not yet implemented: " +
                         Twine(VA.getLocInfo()));

    // Push arguments into RegsToPass vector
    if (VA.isRegLoc()) {
      RegsToPass.emplace_back(VA.getLocReg(), Arg);
      continue;
    }
    assert(VA.isMemLoc() && "Expected stack argument");
    SDValue DstAddr;
    MachinePointerInfo DstInfo;

    unsigned OpSize = VA.getValVT().getSizeInBits();
    OpSize = (OpSize + 7) / 8;
    unsigned LocMemOffset = VA.getLocMemOffset();
    SDValue PtrOff = DAG.getIntPtrConstant(LocMemOffset, DL);
    DstAddr = DAG.getNode(ISD::ADD, DL, PtrVT, StackPtr, PtrOff);
    DstInfo = MachinePointerInfo::getStack(MF, LocMemOffset);

    SDValue Store = DAG.getStore(Chain, DL, Arg, DstAddr, DstInfo);
    MemOpChains.push_back(Store);
  }

  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  // Build a sequence of copy-to-reg nodes chained together with token chain
  // and flag operands which copy the outgoing args into the appropriate regs.
  // We do this and not in the previous loop to chain the registers as close
  // as possible to the actual call.
  for (auto &RegToPass : RegsToPass) {
    Chain =
        DAG.getCopyToReg(Chain, DL, RegToPass.first, RegToPass.second, InGlue);
    InGlue = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that all the generic code cannot
  // mess with it.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, PtrVT,
                                        G->getOffset(), 0);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), PtrVT, 0);
  else
    report_fatal_error("other calls not implemented");

  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known to be live into the call.
  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

  const TargetRegisterInfo &TRI = *Subtarget.getRegisterInfo();
  const uint32_t *Mask = TRI.getCallPreservedMask(MF, CallConv);

  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InGlue.getNode())
    Ops.push_back(InGlue);

  // The call will return a chain & a flag for retval copies to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(H2BLBISD::CALL, DL, NodeTys, Ops);
  InGlue = Chain.getValue(1);

  // Propagate any NoMerge attribute that we may have.
  DAG.addNoMergeSiteInfo(Chain.getNode(), CLI.NoMerge);

  // Finish the call sequence.
  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, InGlue, DL);
  InGlue = Chain.getValue(1);

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RetValLocs;
  CCState CCRetInfo(CallConv, IsVarArg, MF, RetValLocs, *DAG.getContext());

  CCRetInfo.AnalyzeCallResult(Ins, RetCC_H2BLB_Common);

  // Copy all of the result registers out of their specified physreg.
  for (size_t i = 0, e = RetValLocs.size(); i != e; ++i) {
    CCValAssign &VA = RetValLocs[i];
    assert(VA.isRegLoc() && "stack return not yet implemented");
    assert(VA.getLocInfo() == CCValAssign::Full &&
           "extension/truncation of any sort, not yet implemented");

    Chain = DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), VA.getValVT(), InGlue)
                .getValue(1);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    InGlue = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

SDValue H2BLBTargetLowering::lowerMUL(SDValue Op, SelectionDAG &DAG) const {
  assert(Op.getOpcode() == ISD::MUL);

  const EVT ValTy = Op.getValueType();
  assert(ValTy == MVT::i32 &&
         "Custom legalization description doesn't match implementation");

  // Check if the MUL is fed by two s|zext and if so let is go through.
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  bool isSigned;
  if (LHS->getOpcode() == ISD::SIGN_EXTEND &&
      RHS->getOpcode() == ISD::SIGN_EXTEND)
    isSigned = true;
  else if (LHS->getOpcode() == ISD::ZERO_EXTEND &&
           RHS->getOpcode() == ISD::ZERO_EXTEND)
    isSigned = false;
  else
    return SDValue();

  SDValue PlainLHS = LHS.getOperand(0);
  SDValue PlainRHS = RHS.getOperand(0);
  if (PlainLHS.getValueType() != MVT::i16 ||
      PlainRHS.getValueType() != MVT::i16)
    return SDValue();

  unsigned Opcode =
      isSigned ? H2BLBISD::WIDENING_SMUL : H2BLBISD::WIDENING_UMUL;
  SDValue NewVal = DAG.getNode(Opcode, SDLoc(Op), ValTy, PlainLHS, PlainRHS);
  return NewVal;
}

SDValue H2BLBTargetLowering::LowerOperation(SDValue Op,
                                            SelectionDAG &DAG) const {
  LLVM_DEBUG(dbgs() << "Custom lowering: ");
  LLVM_DEBUG(Op.dump());

  switch (Op.getOpcode()) {
  default:
    llvm_unreachable("unimplemented operand");
    return SDValue();
  case ISD::MUL:
    return lowerMUL(Op, DAG);
  }
}

MachineBasicBlock *
H2BLBTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                 MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("Custom inserter not yet implemented");
  case H2BLB::LD16imm16:
  case H2BLB::LD32imm32:
    return emitLDimm(MI);
  case H2BLB::RET_PSEUDO:
    return emitRET_PSEUDO(MI);
  case H2BLB::PTR_ADD16rr:
    MI.setDesc(TII.get(H2BLB::ADDi16rr));
    break;
  }
  return BB;
}

MachineBasicBlock *H2BLBTargetLowering::emitLDimm(MachineInstr &MI) const {
  assert(MI.getOpcode() == H2BLB::LD16imm16 ||
         MI.getOpcode() == H2BLB::LD32imm32);
  bool Is32Bit = MI.getOpcode() == H2BLB::LD32imm32;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  // When using FastISel we may end up with this pseudo whereas the
  // immediate may fit the encoding already.
  // Try re-selecting the instruction.
  int64_t RawImmVal = MI.getOperand(1).getImm();
  assert(Is32Bit || (RawImmVal <= std::numeric_limits<int16_t>::max() &&
                     RawImmVal >= std::numeric_limits<int16_t>::min()) &&
                        "immediate value out of range");
  uint32_t ImmVal = RawImmVal & (Is32Bit ? 0xFFFFFFFF : 0xFFFF);
  // We build the whole constant backward to reuse the 7 constant.
  // Here is the sequence for 16-bit:
  // v2 = LDimm 7-bit chunk2
  // acc = v2
  // acc <<= 7
  // v1 = LDimm 7-bit chunk1
  // acc |= v1
  // acc <<= 7
  // v0 = LDimm 7-bit chunk0
  // acc |= v0
  Register Accumulator;
  Register FinalReg = MI.getOperand(0).getReg();
  bool PreviousChunksAreZero = true;
  constexpr unsigned ChunkSize = 7;
  constexpr unsigned ChunkMask = (1 << ChunkSize) - 1;
  const MCInstrDesc &LdImmDesc =
      Is32Bit ? TII.get(H2BLB::LD32imm7) : TII.get(H2BLB::LD16imm7);
  MachineInstr *LastMI;
  Register ChunkSizeReg;
  const TargetRegisterClass *AccRC =
      Is32Bit ? &H2BLB::GPR32RegClass : &H2BLB::GPR16RegClass;
  unsigned ShlOpc = Is32Bit ? H2BLB::SHL32rr : H2BLB::SHL16rr;
  unsigned OrOpc = Is32Bit ? H2BLB::OR32rr : H2BLB::OR16rr;

  auto MaterializeChunkSizeReg = [&]() {
    if (ChunkSizeReg == H2BLB::NoRegister) {
      // The constant used in the shift is always a 16-bit value.
      ChunkSizeReg = MRI.createVirtualRegister(&H2BLB::GPR16RegClass);
      BuildMI(MBB, MI, MI.getDebugLoc(), TII.get(H2BLB::LD16imm7), ChunkSizeReg)
          .addImm(ChunkSize);
    }
  };

  for (int ChunkNb = (16 + 16 * Is32Bit) / ChunkSize; ChunkNb != -1;
       --ChunkNb) {
    unsigned ChunkShift = ChunkNb * ChunkSize;
    uint32_t CurChunk = ImmVal & (ChunkMask << ChunkShift);
    CurChunk >>= ChunkShift;
    if (!PreviousChunksAreZero) {
      Register NewAcc = MRI.createVirtualRegister(AccRC);
      assert(Accumulator != H2BLB::NoRegister &&
             "We should have seen a non-zero chunk");
      MaterializeChunkSizeReg();
      LastMI = BuildMI(MBB, MI, MI.getDebugLoc(), TII.get(ShlOpc), NewAcc)
                   .addReg(Accumulator)
                   .addReg(ChunkSizeReg);
      Accumulator = NewAcc;
    }
    Register CurChunkReg = MRI.createVirtualRegister(AccRC);
    if (ChunkNb == 0 ) {
      MI.setDesc(LdImmDesc);
      MI.getOperand(0).setReg(CurChunkReg);
      MI.getOperand(1).setImm(CurChunk);
      if (CurChunk || PreviousChunksAreZero)
        LastMI = &MI;
    } else if (CurChunk) {
      LastMI = BuildMI(MBB, MI, MI.getDebugLoc(), LdImmDesc, CurChunkReg)
                   .addImm(CurChunk);
    }
    if (CurChunk) {
      if (PreviousChunksAreZero)
        Accumulator = LastMI->getOperand(0).getReg();
      else {
        assert(Accumulator != H2BLB::NoRegister &&
               "We should have seen a non-zero chunk");
        Register NewAcc = MRI.createVirtualRegister(AccRC);
        // We tweak the insertion point here because we use MI as the LDImm for
        // the last chunk, thus if we have to insert something, we have to
        // insert it after MI, not before.
        LastMI = BuildMI(MBB,
                         (ChunkNb == 0) ? std::next(MI.getIterator())
                                        : MI.getIterator(),
                         MI.getDebugLoc(), TII.get(OrOpc), NewAcc)
                     .addReg(Accumulator)
                     .addReg(CurChunkReg);
        Accumulator = NewAcc;
      }
      PreviousChunksAreZero = false;
    }
  }
  LastMI->getOperand(0).setReg(FinalReg);
  return &MBB;
}

MachineBasicBlock *H2BLBTargetLowering::emitRET_PSEUDO(MachineInstr &MI) const {
  assert(MI.getOpcode() == H2BLB::RET_PSEUDO);
  MachineBasicBlock &MBB = *MI.getParent();
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  MI.setDesc(TII.get(H2BLB::RETURN));
  MI.addOperand(MachineOperand::CreateReg(H2BLB::R0, /*IsDef=*/false,
                                          /*IsImplicit=*/true));
  return &MBB;
}

void H2BLBTargetLowering::finalizeLowering(MachineFunction &MF) const {
  // GISel already call this method so don't call it twice.
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::Selected))
    return;
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
    if (MaybeExitMBB.getFirstTerminator() == MaybeExitMBB.end()) {
      // Check if this is an unreachable block.
      assert(MaybeExitMBB.pred_empty() && &MaybeExitMBB != &*MF.begin() &&
             "Exit block must have a terminator");
      continue;
    }
    assert(
        (MaybeExitMBB.getFirstTerminator()->getOpcode() == H2BLB::RETURN ||
         MaybeExitMBB.getFirstTerminator()->getOpcode() == H2BLB::RET_PSEUDO) &&
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
  case H2BLBISD::WIDENING_SMUL:
    return "H2BLBISD::WIDENING_SMUL";
  case H2BLBISD::WIDENING_UMUL:
    return "H2BLBISD::WIDENING_UMUL";
  }
  return nullptr;
}
