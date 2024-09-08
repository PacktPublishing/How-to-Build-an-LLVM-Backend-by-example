class H2BLBInstruction<string asm, string operands, dag oops = (outs), dag iops = (ins)> : Instruction<> {
  let Namespace = "H2BLB";
  let AsmString = !strconcat(asm, "\t", operands);
  let OutOperandList = oops;
  let InOperandList = iops;
}
