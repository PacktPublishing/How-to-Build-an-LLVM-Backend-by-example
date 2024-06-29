//===-- BuiltinsH2BLB.td - H2BLB Builtin function database --*- tablegen -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the H2BLB-specific builtin function database. Users of
// this file must define the BUILTIN macro to make use of this information.
//
//===----------------------------------------------------------------------===//

include "clang/Basic/BuiltinsBase.td"

def WideningSignedMultiply : TargetBuiltin {
  let Spellings = ["__builtin_h2blb_widening_smul"];
  let Attributes = [NoThrow, Const];
  let Prototype = "int(short, short)";
}

def WideningUnsignedMultiply : TargetBuiltin {
  let Spellings = ["__builtin_h2blb_widening_umul"];
  let Attributes = [NoThrow, Const];
  let Prototype = "unsigned int(unsigned short, unsigned short)";
}
