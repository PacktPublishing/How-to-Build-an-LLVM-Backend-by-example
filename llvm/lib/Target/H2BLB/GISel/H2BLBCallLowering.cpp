//===--- H2BLBCallLowering.cpp - Call lowering ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the lowering of LLVM calls to machine code calls for
/// GlobalISel.
///
//===----------------------------------------------------------------------===//

#include "H2BLBCallLowering.h"

#include "H2BLBCallingConvention.h"
#include "H2BLBISelLowering.h"
#include "H2BLBRegisterInfo.h"
#include "H2BLBSubtarget.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/Analysis.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/GlobalISel/Utils.h"
#include "llvm/CodeGen/LowLevelTypeUtils.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/CodeGenTypes/MachineValueType.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>

#define DEBUG_TYPE "h2blb-call-lowering"

using namespace llvm;

H2BLBCallLowering::H2BLBCallLowering(const H2BLBTargetLowering &TLI)
    : CallLowering(&TLI) {}

// Account for i1/i8/i16 stack passed value hack
static LLT getStackValueStoreTypeHack(const CCValAssign &VA) {
  const MVT ValVT = VA.getValVT();
  return (ValVT == MVT::i8 || ValVT == MVT::i16) ? LLT(ValVT)
                                                 : LLT(VA.getLocVT());
}

namespace {

struct H2BLBIncomingValueAssigner : public CallLowering::IncomingValueAssigner {
  H2BLBIncomingValueAssigner(CCAssignFn *AssignFn_, CCAssignFn *AssignFnVarArg_,
                             const H2BLBSubtarget &Subtarget_, bool IsReturn)
      : IncomingValueAssigner(AssignFn_, AssignFnVarArg_) {}

  bool assignArg(unsigned ValNo, EVT OrigVT, MVT ValVT, MVT LocVT,
                 CCValAssign::LocInfo LocInfo,
                 const CallLowering::ArgInfo &Info, ISD::ArgFlagsTy Flags,
                 CCState &State) override {
    if (State.isVarArg() || Flags.isByVal())
      return false;

    bool Res = AssignFn(ValNo, ValVT, LocVT, LocInfo, Flags, State);
    StackSize = State.getStackSize();
    return Res;
  }
};

struct H2BLBOutgoingValueAssigner : public CallLowering::OutgoingValueAssigner {
  H2BLBOutgoingValueAssigner(CCAssignFn *AssignFn_, CCAssignFn *AssignFnVarArg_)
      : OutgoingValueAssigner(AssignFn_, AssignFnVarArg_) {}

  bool assignArg(unsigned ValNo, EVT OrigVT, MVT ValVT, MVT LocVT,
                 CCValAssign::LocInfo LocInfo,
                 const CallLowering::ArgInfo &Info, ISD::ArgFlagsTy Flags,
                 CCState &State) override {
    if (State.isVarArg() || Flags.isByVal())
      return false;

    bool Res = AssignFn(ValNo, ValVT, LocVT, LocInfo, Flags, State);
    StackSize = State.getStackSize();
    return Res;
  }
};

struct IncomingArgHandler : public CallLowering::IncomingValueHandler {
  IncomingArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
      : IncomingValueHandler(MIRBuilder, MRI) {}

  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO,
                           ISD::ArgFlagsTy Flags) override {
    auto &MFI = MIRBuilder.getMF().getFrameInfo();

    int FI = MFI.CreateFixedObject(Size, Offset, /*IsImmutable=*/true);
    MPO = MachinePointerInfo::getFixedStack(MIRBuilder.getMF(), FI);
    auto AddrReg = MIRBuilder.buildFrameIndex(LLT::pointer(0, 16), FI);
    return AddrReg.getReg(0);
  }

  LLT getStackValueStoreType(const DataLayout &DL, const CCValAssign &VA,
                             ISD::ArgFlagsTy Flags) const override {
    // For pointers, we just need to fixup the integer types reported in the
    // CCValAssign.
    if (Flags.isPointer())
      return CallLowering::ValueHandler::getStackValueStoreType(DL, VA, Flags);
    return getStackValueStoreTypeHack(VA);
  }

  void assignValueToReg(Register ValVReg, Register PhysReg,
                        const CCValAssign &VA) override {
    markPhysRegUsed(PhysReg);
    IncomingValueHandler::assignValueToReg(ValVReg, PhysReg, VA);
  }

  void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                            const MachinePointerInfo &MPO,
                            const CCValAssign &VA) override {
    MachineFunction &MF = MIRBuilder.getMF();

    LLT ValTy(VA.getValVT());
    LLT LocTy(VA.getLocVT());

    assert(ValTy == LocTy && "extensions not implemented");

    auto MMO = MF.getMachineMemOperand(
        MPO, MachineMemOperand::MOLoad | MachineMemOperand::MOInvariant, MemTy,
        inferAlignFromPtrInfo(MF, MPO));
    assert(VA.getLocInfo() != CCValAssign::LocInfo::ZExt &&
           "zero extension not supported");
    assert(VA.getLocInfo() != CCValAssign::LocInfo::SExt &&
           "sign extension not supported");
    MIRBuilder.buildLoad(ValVReg, Addr, *MMO);
  }

  /// How the physical register gets marked varies between formal
  /// parameters (it's a basic-block live-in), and a call instruction
  /// (it's an implicit-def of the BL).
  virtual void markPhysRegUsed(MCRegister PhysReg) = 0;
};

struct FormalArgHandler : public IncomingArgHandler {
  FormalArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
      : IncomingArgHandler(MIRBuilder, MRI) {}

  void markPhysRegUsed(MCRegister PhysReg) override {
    MIRBuilder.getMRI()->addLiveIn(PhysReg);
    MIRBuilder.getMBB().addLiveIn(PhysReg);
  }
};

struct CallReturnHandler : public IncomingArgHandler {
  CallReturnHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI,
                    MachineInstrBuilder MIB)
      : IncomingArgHandler(MIRBuilder, MRI), MIB(MIB) {}

  void markPhysRegUsed(MCRegister PhysReg) override {
    MIB.addDef(PhysReg, RegState::Implicit);
  }

  MachineInstrBuilder MIB;
};

struct OutgoingArgHandler : public CallLowering::OutgoingValueHandler {
  OutgoingArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI,
                     MachineInstrBuilder MIB)
      : OutgoingValueHandler(MIRBuilder, MRI), MIB(MIB) {}

  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO,
                           ISD::ArgFlagsTy Flags) override {
    MachineFunction &MF = MIRBuilder.getMF();
    LLT p0 = LLT::pointer(0, 16);
    LLT s16 = LLT::scalar(16);

    if (!SPReg)
      SPReg = MIRBuilder.buildCopy(p0, Register(H2BLB::SP)).getReg(0);

    auto OffsetReg = MIRBuilder.buildConstant(s16, Offset);

    auto AddrReg = MIRBuilder.buildPtrAdd(p0, SPReg, OffsetReg);

    MPO = MachinePointerInfo::getStack(MF, Offset);
    return AddrReg.getReg(0);
  }

  /// We need to fixup the reported store size for certain value types because
  /// we invert the interpretation of ValVT and LocVT in certain cases. This is
  /// for compatability with the DAG call lowering implementation, which we're
  /// currently building on top of.
  LLT getStackValueStoreType(const DataLayout &DL, const CCValAssign &VA,
                             ISD::ArgFlagsTy Flags) const override {
    if (Flags.isPointer())
      return CallLowering::ValueHandler::getStackValueStoreType(DL, VA, Flags);
    return LLT(VA.getLocVT());
  }

  void assignValueToReg(Register ValVReg, Register PhysReg,
                        const CCValAssign &VA) override {
    // Attach the phys reg on the instruction itself (call or return
    // instruction).
    MIB.addUse(PhysReg, RegState::Implicit);
    assert((VA.getLocInfo() == CCValAssign::Full ||
            VA.getLocInfo() == CCValAssign::BCvt) &&
           "Extension not supported");
    // Assign the value to the phys reg.
    MIRBuilder.buildCopy(PhysReg, ValVReg);
  }

  void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                            const MachinePointerInfo &MPO,
                            const CCValAssign &VA) override {
    MachineFunction &MF = MIRBuilder.getMF();
    auto MMO = MF.getMachineMemOperand(MPO, MachineMemOperand::MOStore, MemTy,
                                       inferAlignFromPtrInfo(MF, MPO));
    MIRBuilder.buildStore(ValVReg, Addr, *MMO);
  }

  // Builder that points on the outgoing instructions (call or return).
  MachineInstrBuilder MIB;

  // Cache the SP register vreg if we need it more than once in this call site.
  Register SPReg;
};
} // namespace

bool H2BLBCallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                    const Value *Val, ArrayRef<Register> VRegs,
                                    FunctionLoweringInfo &FLI) const {
  // The implicit use of LR is already part of the instruction description,
  // hence we do not have to add it.
  auto MIB = MIRBuilder.buildInstrNoInsert(H2BLB::RETURN);
  assert(((Val && !VRegs.empty()) || (!Val && VRegs.empty())) &&
         "Return value without a vreg");

  bool Success = true;
  if (!FLI.CanLowerReturn) {
    insertSRetStores(MIRBuilder, Val->getType(), VRegs, FLI.DemoteRegister);
  } else if (!VRegs.empty()) {
    MachineFunction &MF = MIRBuilder.getMF();
    const Function &F = MF.getFunction();

    MachineRegisterInfo &MRI = MF.getRegInfo();
    const H2BLBTargetLowering &TLI = *getTLI<H2BLBTargetLowering>();
    auto &DL = F.getDataLayout();
    LLVMContext &Ctx = Val->getType()->getContext();

    // Expand any value that may span several arguments (e.g., struct).
    SmallVector<EVT, 4> SplitEVTs;
    ComputeValueVTs(TLI, DL, Val->getType(), SplitEVTs);
    assert(VRegs.size() == SplitEVTs.size() &&
           "For each split Type there should be exactly one VReg.");

    SmallVector<ArgInfo, 8> SplitArgs;
    CallingConv::ID CC = F.getCallingConv();

    for (unsigned i = 0; i < SplitEVTs.size(); ++i) {
      Register CurVReg = VRegs[i];
      ArgInfo CurArgInfo = ArgInfo{CurVReg, SplitEVTs[i].getTypeForEVT(Ctx), 0};
      setArgFlags(CurArgInfo, AttributeList::ReturnIndex, DL, F);
      if (TLI.getNumRegistersForCallingConv(Ctx, CC, SplitEVTs[i]) == 1) {
        MVT NewVT = TLI.getRegisterTypeForCallingConv(Ctx, CC, SplitEVTs[i]);
        // Some types will need extending as specified by the CC.
        if (EVT(NewVT) != SplitEVTs[i])
          report_fatal_error("Extension not implemented yet");
      }
      splitToValueTypes(CurArgInfo, SplitArgs, DL, CC);
    }

    H2BLBOutgoingValueAssigner Assigner(RetCC_H2BLB_Common, RetCC_H2BLB_Common);
    OutgoingArgHandler Handler(MIRBuilder, MRI, MIB);
    Success = determineAndHandleAssignments(Handler, Assigner, SplitArgs,
                                            MIRBuilder, CC, F.isVarArg());
  }

  MIRBuilder.insertInstr(MIB);
  return Success;
}

bool H2BLBCallLowering::canLowerReturn(MachineFunction &MF,
                                       CallingConv::ID CallConv,
                                       SmallVectorImpl<BaseArgInfo> &Outs,
                                       bool IsVarArg) const {
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs,
                 MF.getFunction().getContext());

  return !IsVarArg && checkReturn(CCInfo, Outs, RetCC_H2BLB_Common);
}

bool H2BLBCallLowering::lowerFormalArguments(MachineIRBuilder &MIRBuilder,
                                             const Function &F,
                                             ArrayRef<ArrayRef<Register>> VRegs,
                                             FunctionLoweringInfo &FLI) const {
  MachineFunction &MF = MIRBuilder.getMF();
  MachineBasicBlock &MBB = MIRBuilder.getMBB();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  auto &DL = F.getDataLayout();

  if (F.isVarArg())
    return false;

  SmallVector<ArgInfo, 8> SplitArgs;

  // Insert the hidden sret parameter if the return value won't fit in the
  // return registers.
  if (!FLI.CanLowerReturn)
    insertSRetIncomingArgument(F, SplitArgs, FLI.DemoteRegister, MRI, DL);

  unsigned i = 0;
  for (auto &Arg : F.args()) {
    if (DL.getTypeStoreSize(Arg.getType()).isZero())
      continue;

    ArgInfo OrigArg{VRegs[i], Arg, i};
    setArgFlags(OrigArg, i + AttributeList::FirstArgIndex, DL, F);

    splitToValueTypes(OrigArg, SplitArgs, DL, F.getCallingConv());
    ++i;
  }

  if (!MBB.empty())
    MIRBuilder.setInstr(*MBB.begin());

  CCAssignFn *AssignFn = CC_H2BLB_Common;

  CallLowering::IncomingValueAssigner Assigner(AssignFn, AssignFn);
  FormalArgHandler Handler(MIRBuilder, MRI);
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(F.getCallingConv(), F.isVarArg(), MF, ArgLocs, F.getContext());
  if (!determineAssignments(Assigner, SplitArgs, CCInfo) ||
      !handleAssignments(Handler, SplitArgs, CCInfo, ArgLocs, MIRBuilder))
    return false;

  // Move back to the end of the basic block.
  MIRBuilder.setMBB(MBB);

  return true;
}

bool H2BLBCallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                  CallLoweringInfo &Info) const {
  MachineFunction &MF = MIRBuilder.getMF();
  const Function &F = MF.getFunction();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  auto &DL = F.getDataLayout();
  const H2BLBSubtarget &Subtarget = MF.getSubtarget<H2BLBSubtarget>();

  SmallVector<ArgInfo, 8> OutArgs;
  for (auto &OrigArg : Info.OrigArgs)
    splitToValueTypes(OrigArg, OutArgs, DL, Info.CallConv);

  SmallVector<ArgInfo, 8> InArgs;
  if (!Info.OrigRet.Ty->isVoidTy())
    splitToValueTypes(Info.OrigRet, InArgs, DL, Info.CallConv);

  CCAssignFn *AssignFnFixed;
  CCAssignFn *AssignFnVarArg;
  AssignFnFixed = AssignFnVarArg = CC_H2BLB_Common;

  MachineInstrBuilder CallSeqStart;
  CallSeqStart = MIRBuilder.buildInstr(H2BLB::ADJCALLSTACKDOWN);

  // We don't support indirect calls.
  if (!Info.Callee.isGlobal() && !Info.Callee.isSymbol())
    return false;
  auto MIB = MIRBuilder.buildInstrNoInsert(H2BLB::CALL);
  MIB->addOperand(MF, Info.Callee);

  // Tell the call which registers are clobbered.
  const auto &TRI = *Subtarget.getRegisterInfo();

  H2BLBOutgoingValueAssigner Assigner(AssignFnFixed, AssignFnVarArg);
  // Do the actual argument marshalling.
  OutgoingArgHandler Handler(MIRBuilder, MRI, MIB);
  if (!determineAndHandleAssignments(Handler, Assigner, OutArgs, MIRBuilder,
                                     Info.CallConv, Info.IsVarArg))
    return false;

  const uint32_t *Mask = TRI.getCallPreservedMask(MF, Info.CallConv);

  MIB.addRegMask(Mask);

  // Now we can add the actual call instruction to the correct basic block.
  MIRBuilder.insertInstr(MIB);

  CallSeqStart.addImm(Assigner.StackSize).addImm(0);
  MIRBuilder.buildInstr(H2BLB::ADJCALLSTACKUP)
      .addImm(Assigner.StackSize)
      .addImm(0);


  // Finally we can copy the returned value back into its virtual-register. In
  // symmetry with the arguments, the physical register must be an
  // implicit-define of the call instruction.
  if (Info.OrigRet.Ty->isVoidTy())
    return true;

  if (!Info.CanLowerReturn) {
    insertSRetLoads(MIRBuilder, Info.OrigRet.Ty, Info.OrigRet.Regs,
                    Info.DemoteRegister, Info.DemoteStackIndex);
    return true;
  }

  CallReturnHandler CallRetHandler(MIRBuilder, MRI, MIB);
  if (!OutArgs.empty() && OutArgs[0].Flags[0].isReturned())
    return false;

  H2BLBOutgoingValueAssigner OutValAssigner(RetCC_H2BLB_Common,
                                            RetCC_H2BLB_Common);
  return determineAndHandleAssignments(CallRetHandler, OutValAssigner, InArgs,
                                       MIRBuilder, Info.CallConv, Info.IsVarArg,
                                       std::nullopt);
}
