let Entry = 1 in
def CC_H2BLB_Common : CallingConv<[]>;

let Entry = 1 in
def RetCC_H2BLB_Common : CallingConv<[
  CCIfType<[i16], CCAssignToReg<[R1, R2, R3]>>,
  CCIfType<[i32], CCAssignToReg<[D1]>>
]>;

def CSR : CalleeSavedRegs<(add D2, D3)>;
