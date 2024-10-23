//===- H2BLBCallLowering.h - Call lowering ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file describes how to lower LLVM calls to machine code calls.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBCALLLOWERING_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBCALLLOWERING_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/IR/CallingConv.h"
#include <cstdint>

namespace llvm {

class H2BLBTargetLowering;
class MachineIRBuilder;
class Type;

class H2BLBCallLowering : public CallLowering {
public:
  H2BLBCallLowering(const H2BLBTargetLowering &TLI);

  /// Lower outgoing return values, described / by \p Val, into the specified
  /// virtual registers \p VRegs.
  ///
  /// \p FLI is required for sret demotion.
  ///
  /// \return True if the lowering succeeds, false otherwise.
  bool lowerReturn(MachineIRBuilder &MIRBuilder, const Value *Val,
                   ArrayRef<Register> VRegs,
                   FunctionLoweringInfo &FLI) const override;

  /// Check whether the return values described by \p Outs can fit into the
  /// return registers.
  /// If false is returned, an sret-demotion is performed.
  bool canLowerReturn(MachineFunction &MF, CallingConv::ID CallConv,
                      SmallVectorImpl<BaseArgInfo> &Outs,
                      bool IsVarArg) const override;

  /// Lowers the incoming (formal) arguments, described by \p VRegs.
  /// Each argument must end up in the related virtual registers described
  /// by \p VRegs.
  /// In other words, the first argument should end up in \c VRegs[0],
  /// the second in \c VRegs[1], and so on. For each argument, there will be one
  /// register for each non-aggregate type, as returned by \c computeValueLLTs.
  /// \p MIRBuilder is set to the proper insertion for the argument
  /// lowering. \p FLI is required for sret demotion.
  ///
  /// \return True if the lowering succeeded, false otherwise.
  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs,
                            FunctionLoweringInfo &FLI) const override;

  /// Lower the given call instruction, including argument and return value
  /// marshalling.
  ///
  /// \return true if the lowering succeeded, false otherwise.
  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_H2BLB_H2BLBCALLLOWERING_H
