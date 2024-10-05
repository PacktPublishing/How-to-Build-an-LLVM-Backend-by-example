include "H2BLBInstrFormats.td"

// SelectionDAG target specific nodes.
def H2BLBreturnglue : SDNode<"H2BLBISD::RETURN_GLUE", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;


def ADDi16rr : H2BLBBinaryInstruction<"addi16",
                                      /*opcode=*/0,
                                      /*is32Bit=*/false>;

let Uses = [R0], isReturn = 1, isTerminator = 1, isBarrier = 1 in
def RETURN : H2BLBNoOperandInstruction<"ret",
                                       /*opcode=*/0b00001,
                                       /*is32Bit=*/false>;
