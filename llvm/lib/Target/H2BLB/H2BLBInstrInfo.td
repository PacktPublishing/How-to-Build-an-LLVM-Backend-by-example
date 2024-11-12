//===- H2BLBInstrFormats.td - H2BLB Instruction Information --*- tblgen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Describe H2BLB instructions
//
//===----------------------------------------------------------------------===//

include "H2BLBInstrFormats.td"

// SelectionDAG target specific nodes.
def H2BLBreturnglue : SDNode<"H2BLBISD::RETURN_GLUE", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def H2BLBcallseq_start : SDNode<"ISD::CALLSEQ_START",
                                // Two arguments:
                                // Number of bytes allocated within the
                                // callseq_start/end region and the number
                                // of bytes allocated outside.
                                SDCallSeqStart<[ SDTCisVT<0, i32>,
                                                 SDTCisVT<1, i32> ]>,
                                [SDNPHasChain, SDNPOutGlue]>;
def H2BLBcallseq_end : SDNode<"ISD::CALLSEQ_END",
                              // Same arguments as callseq_start.
                              SDCallSeqEnd<[ SDTCisVT<0, i32>,
                                             SDTCisVT<1, i32> ]>,
                              [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def H2BLBcall : SDNode<"H2BLBISD::CALL",
                       // Type profile:
                       // 0: no result.
                       // -1: a variadic number of inputs.
                       // The first operand must be a pointer (to the call
                       // target)
                       SDTypeProfile<0, -1, [SDTCisPtrTy<0>]>,
                       [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                        SDNPVariadic]>;


let Pattern = [(set GPR16:$dst, (add GPR16:$src0, GPR16:$src1))] in
def ADDi16rr : H2BLBBinaryInstruction<"addi16",
                                      /*opcode=*/0,
                                      /*is32Bit=*/false>;
let Pattern = [(set (i32 GPR32:$dst), (add (i32 GPR32:$src0),
                                           (i32 GPR32:$src1)))] in
def ADDi32rr : H2BLBBinaryInstruction<"addi32",
                                      /*opcode=*/0,
                                      /*is32Bit=*/true>;

let Uses = [R0], isReturn = 1, isTerminator = 1, isBarrier = 1,
Pattern = [(H2BLBreturnglue)] in
def RETURN : H2BLBNoOperandInstruction<"ret",
                                       /*opcode=*/0b00001,
                                       /*is32Bit=*/false>;

let isAsCheapAsAMove = 1, isMoveImm = 1, isReMaterializable = 1 in
def LD16imm7 : H2BLBInstruction<"ldi16", "$dst, $imm7",
                                 /*opcode=*/0b00010,
                                 /*is32Bit=*/0,
                                 /*oops=*/(outs GPR16:$dst),
                                 /*iops=*/(ins uimm7:$imm7)> {
  bits<3> dst;
  bits<7> imm7;
  let Inst{9-7} = dst;
  let Inst{6-0} = imm7;
  let Pattern = [(set GPR16:$dst, uimm7:$imm7)];
}

def LD16imm16 : H2BLBPseudoInstruction<(outs GPR16:$dst), (ins i16imm:$src),
                                       [(set GPR16:$dst, imm:$src)]>;
def LD32imm32 : H2BLBPseudoInstruction<(outs GPR32:$dst), (ins i32imm:$src),
                                       [(set (i32 GPR32:$dst), imm:$src)]>;

// Load a register.
def LDR16 : H2BLBLoadInstruction<"ldr16",
                                 /*is32Bit=*/0> {
  let Pattern = [(set GPR16:$dst, (load (addrmode GPR16:$addr, uimm4:$offset)))];
}
def LDR32 : H2BLBLoadInstruction<"ldr32",
                                 /*is32Bit=*/1> {
  let Pattern = [(set (i32 GPR32:$dst), (load (addrmode GPR16:$addr, uimm4:$offset)))];
}

let isCall = 1, Defs = [R0], Uses = [SP] in
def CALL : H2BLBInstruction<"call", "$tgt",
                                 /*opcode=*/0b00100,
                                 /*is32Bit=*/0,
                                 /*oops=*/(outs),
                                 /*iops=*/(ins i16imm:$tgt)> {
  bits<11> tgt;
  let Inst{10-0} = tgt;
  let Pattern = [(H2BLBcall tglobaladdr:$tgt)];
}

let hasSideEffects = 1, isCodeGenOnly = 1 in {
let Defs = [SP], Uses = [SP] in {
def ADJCALLSTACKDOWN : H2BLBPseudoInstruction<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(H2BLBcallseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP : H2BLBPseudoInstruction<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                            [(H2BLBcallseq_end timm:$amt1, timm:$amt2)]>;
}
}

// Store a register.
def STR16 : H2BLBStoreInstruction<"str16",
                                 /*is32Bit=*/0> {
  let Pattern = [(store (i16 GPR16:$src), (addrmode GPR16:$addr, uimm4:$offset))];
}

def STR32 : H2BLBStoreInstruction<"str32",
                                 /*is32Bit=*/1> {
  let Pattern = [(store (i32 GPR32:$src), (addrmode GPR16:$addr, uimm4:$offset))];
}

def TRUNCSTR8: H2BLBInstruction<"truncstr8", "$src, $addr, $offset",
                                 /*opcode=*/0b01110,
                                 /*unusedBit=*/0,
                                 /*oops=*/(outs),
                                 /*iops=*/(ins GPR16:$src, GPR16:$addr, uimm4:$offset)> {
  let mayStore = 1;
  bits<3> src;
  bits<3> addr;
  bits<4> offset;
  let Inst{9-7} = src;
  let Inst{6-4} = addr;
  let Inst{3-0} = offset;
  let Pattern = [(truncstorei8 (i16 GPR16:$src), (addrmode GPR16:$addr, uimm4:$offset))];
}
def : Pat<(store (i8 (trunc GPR16:$src)), (addrmode GPR16:$addr, uimm4:$offset)), (TRUNCSTR8 GPR16:$src, GPR16:$addr, uimm4:$offset)>;

def : Pat<(i16 (trunc (i32 GPR32:$src))),
          (i16 (EXTRACT_SUBREG GPR32:$src, sub_low16))>;

def MOV16 : H2BLBMoveInstruction<"mov16", /*is32Bit=*/0>;
def MOV32 : H2BLBMoveInstruction<"mov32", /*is32Bit=*/1>;


def MOVTOSP: H2BLBInstruction<"movtosp", "$sp, $src",
                                 /*opcode=*/0b00111,
                                 /*toSP=*/1,
                                 /*oops=*/(outs OnlySP:$sp),
                                 /*iops=*/(ins GPR16:$src)> {
  bits<3> sp;
  bits<3> src;
  let Inst{9-7} = sp;
  let Inst{6-4} = src;
  let Inst{3-0} = 0;
}

def MOVFROMSP: H2BLBInstruction<"movfromsp", "$dst, $sp",
                                 /*opcode=*/0b00111,
                                 /*toSP=*/0,
                                 /*oops=*/(outs GPR16:$dst),
                                 /*iops=*/(ins OnlySP:$sp)> {
  bits<3> dst;
  bits<3> sp;
  let Inst{9-7} = dst;
  let Inst{6-4} = sp;
  let Inst{3-0} = 0;
}
def : Pat<(i16 (frameindex:$ptr)), (MOVFROMSP (i16 (to_tframeindex $ptr)))>;

let AddedComplexity = 1 in {
def STRSP16: H2BLBStoreSPInstruction<"strsp16", /*is32Bit=*/0> {
  let Pattern = [(store (i16 GPR16:$src), (spaddrmode OnlySP:$sp, simm7:$offset))];
}
def STRSP32: H2BLBStoreSPInstruction<"strsp32", /*is32Bit=*/1> {
  let Pattern = [(store (i32 GPR32:$src), (spaddrmode OnlySP:$sp, simm7:$offset))];
}

def LDRSP16: H2BLBLoadSPInstruction<"ldrsp16", /*is32Bit=*/0> {
  let Pattern = [(set GPR16:$dst, (load (spaddrmode OnlySP:$sp, simm7:$offset)))];
}
def LDRSP32: H2BLBLoadSPInstruction<"ldrsp32", /*is32Bit=*/1> {
  let Pattern = [(set (i32 GPR32:$dst), (load (spaddrmode OnlySP:$sp, simm7:$offset)))];
}

def LDRSEXTSP8: H2BLBExtLoadSPInstruction<"ldrsextsp8", /*isZExt=*/0> {
  let Pattern = [(set GPR16:$dst, (sextload (spaddrmode OnlySP:$sp, simm7:$offset)))];
}
def LDRZEXTSP8: H2BLBExtLoadSPInstruction<"ldrzextsp8", /*isZExt=*/1> {
  let Pattern = [(set GPR16:$dst, (zextload (spaddrmode OnlySP:$sp, simm7:$offset)))];
}
} // end AddedComplexity.

// Match any_ext(load) on zextload.
def : Pat<(i16 (extload (spaddrmode OnlySP:$sp, simm7:$offset))), (LDRZEXTSP8 OnlySP:$sp, simm7:$offset)>;

def ADDSP : H2BLBBinarySPInstruction<"addsp",
                                     /*opcode=*/0b01010,
                                     /*isAdd=*/true>;
def SUBSP : H2BLBBinarySPInstruction<"subsp",
                                     /*opcode=*/0b01010,
                                     /*isAdd=*/false>;
