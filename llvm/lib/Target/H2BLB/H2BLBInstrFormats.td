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
