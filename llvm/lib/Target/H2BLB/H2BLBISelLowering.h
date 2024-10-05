//===-- H2BLBISelLowering.h - H2BLB DAG Lowering Interface --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that H2BLB uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_H2BLB_H2BLBISELLOWERING_H
#define LLVM_LIB_TARGET_H2BLB_H2BLBISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

namespace H2BLBISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RETURN_GLUE,
};
}

class H2BLBSubtarget;
class H2BLBTargetMachine;

class H2BLBTargetLowering : public TargetLowering {
  const H2BLBSubtarget &Subtarget;

public:
  explicit H2BLBTargetLowering(const TargetMachine &TM,
                               const H2BLBSubtarget &STI);

  /// This method returns a target specific FastISel object, or null if the
  /// target does not support "fast" ISel.
  FastISel *createFastISel(FunctionLoweringInfo &funcInfo,
                           const TargetLibraryInfo *libInfo) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;

  // This method returns the name of a target specific DAG node.
  const char *getTargetNodeName(unsigned Opcode) const override;
};
} // end namespace llvm

#endif // LLVM_LIB_TARGET_H2BLB_H2BLBISELLOWERING_H
