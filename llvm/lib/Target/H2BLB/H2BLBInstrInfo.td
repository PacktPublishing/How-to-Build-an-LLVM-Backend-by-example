include "H2BLBInstrFormats.td"

// SelectionDAG target specific nodes.
def H2BLBreturnglue : SDNode<"H2BLBISD::RETURN_GLUE", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;


def ADDi16rr : H2BLBBinaryInstruction<"addi16",
                                      /*opcode=*/0,
                                      /*is32Bit=*/false>;

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

// Load a register.
let mayLoad = 1 in
def LDR16 : H2BLBInstruction<"ldr16", "$dst, $addr, $offset",
                                 /*opcode=*/0b00011,
                                 /*is32Bit=*/0,
                                 /*oops=*/(outs GPR16:$dst),
                                 /*iops=*/(ins GPR16:$addr, uimm4:$offset)> {
  bits<3> dst;
  bits<3> addr;
  bits<4> offset;
  let Inst{9-7} = dst;
  let Inst{6-4} = addr;
  let Inst{3-0} = offset;
  let Pattern = [(set GPR16:$dst, (load (addrmode GPR16:$addr, uimm4:$offset)))];
}

let isCall = 1, Defs = [R0], Uses = [SP] in
def CALL : H2BLBInstruction<"call", "$tgt",
                                 /*opcode=*/0b00100,
                                 /*is32Bit=*/0,
                                 /*oops=*/(outs),
                                 /*iops=*/(ins i16imm:$tgt)> {
  bits<11> tgt;
  let Inst{10-0} = tgt;
}
