//===-- H2BLB.td - Describe the H2BLB Target Machine ------*- tablegen -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is the top level entry point for the H2BLB target.
//
//===----------------------------------------------------------------------===//

// Include the generic target definitions.
include "llvm/Target/Target.td"

include "H2BLBRegisterInfo.td"
include "H2BLBCallingConvention.td"
include "H2BLBInstrInfo.td"

def : ProcessorModel<"generic", NoSchedModel, /*Features=*/[]>;

def H2BLBInstrInfo : InstrInfo;

def H2BLBAsmParser : AsmParser;

def H2BLB : Target {
  let InstructionSet = H2BLBInstrInfo;
  let AssemblyParsers = [H2BLBAsmParser];
}
