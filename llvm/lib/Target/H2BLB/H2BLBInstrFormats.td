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
  string StrLow = !if(!lt(Low, 0), "N" # !mul(Low, -1), "" # Low);
  let Name = "Imm" # StrLow # "_" # High;
  let DiagnosticType = "InvalidImm" # StrLow # "_" # High;
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isImmInRange<" # Low # "," # High # ">";
}

def u16imm7 : Operand<i16>, ImmLeaf<i16, [{return Imm >= 0 && Imm < 128;}]> {
  let ParserMatchClass = AsmImmRange<0, 127>;
}
def u32imm7 : Operand<i32>, ImmLeaf<i32, [{return Imm >= 0 && Imm < 128;}]> {
  let ParserMatchClass = AsmImmRange<0, 127>;
}
def simm7 : Operand<i16>, ImmLeaf<i16, [{return Imm >= -64 && Imm < 63;}]> {
  let ParserMatchClass = AsmImmRange<-64, 63>;
}

def uimm10 : Operand<i16>, ImmLeaf<i16, [{return Imm >= 0 && Imm < 1024;}]> {
  let ParserMatchClass = AsmImmRange<0, 1023>;
}
def uimm4 : Operand<i16>, ImmLeaf<i16, [{return Imm >= 0 && Imm < 16;}]> {
  let ParserMatchClass = AsmImmRange<0, 15>;
}

def addrmode : ComplexPattern<iPTR, 2, "selectAddrMode", []>;
def spaddrmode : ComplexPattern<iPTR, 2, "selectSPAddrMode", []>;

def p0 : LLT;

def gi_addrmode :
  GIComplexOperandMatcher<p0, "selectAddrMode">,
  GIComplexPatternEquiv<addrmode>;

def gi_spaddrmode :
  GIComplexOperandMatcher<p0, "selectSPAddrMode">,
  GIComplexPatternEquiv<spaddrmode>;

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

class H2BLBWidenIMul<string asm, bit isSign>
          : H2BLBInstruction<asm, /*operands=*/"$dst, $src0, $src1",
                             /*opcode=*/0, isSign, /*oops=*/(outs GPR32:$dst),
                             /*iops=*/(ins GPR16:$src0, GPR16:$src1)> {
  bits<2> dst;
  bits<3> src0;
  bits<3> src1;
  // Since dst is between d0 and d3, we know two bits are enough.
  // Save one bit for more opcodes.
  // For a real target, we wouldn't need to scrap bits on the go :).
  let Inst{9} = 0;
  let Inst{8-7} = dst;
  let Inst{6-4} = src0;
  let Inst{3-1} = src1;
  // This is how we differentiate ourselves from the encoding of ADDi16rr.
  let Inst{0} = 1;
}

class H2BLBLoadImmediate<string asm,
                         bits<5> opcode,
                         bit is32Bit,
                         RegisterClass DstRC = !if(is32Bit, GPR32, GPR16),
                         Operand ImmTy = !if(is32Bit, u32imm7, u16imm7)>
          : H2BLBInstruction<asm, "$dst, $imm7", opcode, is32Bit,
                                 /*oops=*/(outs DstRC:$dst),
                                 /*iops=*/(ins ImmTy:$imm7)> {
  let isAsCheapAsAMove = 1;
  let isMoveImm = 1;
  let isReMaterializable = 1;
  bits<3> dst;
  bits<7> imm7;
  let Inst{9-7} = dst;
  let Inst{6-0} = imm7;
  let Pattern = [(set DstRC:$dst, ImmTy:$imm7)];
}

class H2BLBStoreInstruction<string asm,
                             bit is32Bit = 0,
                             RegisterClass SrcRC = !if(is32Bit, GPR32, GPR16)>
          : H2BLBInstruction<asm, "$src, $addr, $offset", /*opcode=*/0b00101, is32Bit,
                             (outs), (ins SrcRC:$src, GPR16:$addr, uimm4:$offset)> {
  let mayStore = 1;
  bits<3> src;
  bits<3> addr;
  bits<4> offset;
  let Inst{9-7} = src;
  let Inst{6-4} = addr;
  let Inst{3-0} = offset;
}

class H2BLBLoadInstruction<string asm,
                           bit is32Bit = 0,
                           RegisterClass DstRC = !if(is32Bit, GPR32, GPR16)>
          : H2BLBInstruction<asm, "$dst, $addr, $offset",
                                 /*opcode=*/0b00011,
                                 is32Bit,
                                 /*oops=*/(outs DstRC:$dst),
                                 /*iops=*/(ins GPR16:$addr, uimm4:$offset)> {
  let mayLoad = 1;
  bits<3> dst;
  bits<3> addr;
  bits<4> offset;
  let Inst{9-7} = dst;
  let Inst{6-4} = addr;
  let Inst{3-0} = offset;
}

class H2BLBMoveInstruction<string asm,
                           bit is32Bit = 0,
                           RegisterClass RC = !if(is32Bit, GPR32, GPR16)>
          : H2BLBInstruction<asm, "$dst, $src",
                                 /*opcode=*/0b00110,
                                 is32Bit,
                                 /*oops=*/(outs RC:$dst),
                                 /*iops=*/(ins RC:$src)> {
  let isMoveReg = 1;
  let isAsCheapAsAMove = 1;
  bits<3> dst;
  bits<3> src;
  let Inst{9-7} = dst;
  let Inst{6-4} = src;
  let Inst{3-0} = 0;
}

class H2BLBStoreSPInstruction<string asm,
                              bit is32Bit = 0,
                              RegisterClass SrcRC = !if(is32Bit, GPR32, GPR16)>
          : H2BLBInstruction<asm, "$src, $sp, $offset", /*opcode=*/0b01000, is32Bit,
                             (outs), (ins SrcRC:$src, OnlySP:$sp, simm7:$offset)> {
  let mayStore = 1;
  bits<3> src;
  bits<7> offset;
  let Inst{9-7} = src;
  let Inst{6-0} = offset;
}

class H2BLBLoadSPInstruction<string asm,
                              bit is32Bit = 0,
                              RegisterClass DstRC = !if(is32Bit, GPR32, GPR16)>
          : H2BLBInstruction<asm, "$dst, $sp, $offset", /*opcode=*/0b01001, is32Bit,
                             (outs DstRC:$dst), (ins OnlySP:$sp, simm7:$offset)> {
  let mayLoad = 1;
  bits<3> dst;
  bits<7> offset;
  let Inst{9-7} = dst;
  let Inst{6-0} = offset;
}

class H2BLBExtLoadSPInstruction<string asm,
                              bit isZExt = 0>
          : H2BLBInstruction<asm, "$dst, $sp, $offset", /*opcode=*/0b01011, isZExt,
                             (outs GPR16:$dst), (ins OnlySP:$sp, simm7:$offset)> {
  let mayLoad = 1;
  bits<3> dst;
  bits<7> offset;
  let Inst{9-7} = dst;
  let Inst{6-0} = offset;
}

class H2BLBBinarySPInstruction<string asm,
                               bits<5> opcode,
                               bit isAdd>
          : H2BLBInstruction<asm, "sp, sp, $imm", opcode, isAdd,
                             (outs OnlySP:$dst), (ins OnlySP:$src, uimm10:$imm)> {
  bits<10> imm;
  let Inst{9-0} = imm;
}
