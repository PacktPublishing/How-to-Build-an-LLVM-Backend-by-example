//===- H2BLBInstrFormats.td - H2BLB Instruction Formats ------*- tblgen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Describe H2BLB instructions format here
//
//===----------------------------------------------------------------------===//

// classes for operand types.
class AsmImmRange<int Low, int High> : AsmOperandClass {
  let Name = "Imm" # Low # "_" # High;
  let DiagnosticType = "InvalidImm" # Low # "_" # High;
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isImmInRange<" # Low # "," # High # ">";
}

def uimm7 : Operand<i16>, ImmLeaf<i16, [{return Imm >= 0 && Imm < 128;}]> {
  let ParserMatchClass = AsmImmRange<0, 127>;
}

def uimm4 : Operand<i16>, ImmLeaf<i16, [{return Imm >= 0 && Imm < 16;}]> {
  let ParserMatchClass = AsmImmRange<0, 15>;
}

def addrmode : ComplexPattern<iPTR, 2, "selectAddrMode", []>;


// Instruction templates.

class H2BLBInstruction<string asm, string operands,
                       bits<5> opcode = 0,
                       bit is32Bit = 0,
                       dag oops = (outs), dag iops = (ins)>
          : Instruction<> {
  let Namespace = "H2BLB";
  let AsmString = !strconcat(asm, "\t", operands);
  let OutOperandList = oops;
  let InOperandList = iops;
  // Encoding information.
  // +---------------------------------------------------------------+
  // | Bit index                                                     |
  // +--------+------------+--------+---------+---------+------------+
  // | 15 - 11|     10     |  9 - 7 |   6 - 4 |   3 - 1 |     0      |
  // +--------+------------+--------+---------+---------+------------+
  // | <5>opc | <1>is32Bit | <3>dst | <3>src0 | <3>src1 | <1><spare> |
  // +--------+------------+--------+---------+---------+------------+
  bits<16> Inst;
  let Inst{15-11} = opcode;
  let Inst{10} = is32Bit;
  let Inst{0} = 0;
  // oops and iops are handled by the other classes.
}

// Pseudo instructions (don't have encoding information) and must
// disappear before the MC layer.
class H2BLBPseudoInstruction<dag oops, dag iops, list<dag> pattern>
    : H2BLBInstruction</*asm=*/"", /*operands=*/"", 0, 0, oops, iops> {
  let Pattern        = pattern;
  let isCodeGenOnly  = 1;
  let isPseudo       = 1;
}

class H2BLBBinaryInstruction<string asm,
                             bits<5> opcode = 0,
                             bit is32Bit = 0,
                             RegisterClass SrcRC = !if(is32Bit, GPR32, GPR16),
                             RegisterClass DstRC = SrcRC>
          : H2BLBInstruction<asm, "$dst, $src0, $src1", opcode, is32Bit,
                             (outs DstRC:$dst), (ins SrcRC:$src0, SrcRC:$src1)> {
  bits<3> dst;
  bits<3> src0;
  bits<3> src1;
  let Inst{9-7} = dst;
  let Inst{6-4} = src0;
  let Inst{3-1} = src1;
}

class H2BLBNoOperandInstruction<string asm,
                             bits<5> opcode = 0,
                             bit is32Bit = 0>
          : H2BLBInstruction<asm, /*operands=*/"", opcode, is32Bit,
                             (outs), (ins)> {
  let Inst{9-1} = 0;
}
