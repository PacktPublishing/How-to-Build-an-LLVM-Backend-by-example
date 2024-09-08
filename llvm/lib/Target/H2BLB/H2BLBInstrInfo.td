include "H2BLBInstrFormats.td"

def ADDi16rr : H2BLBInstruction<"addi16", "$dst, $src0, $src1",
                                (outs GPR16:$dst),
                                (ins GPR16:$src0, GPR16:$src1)>;
