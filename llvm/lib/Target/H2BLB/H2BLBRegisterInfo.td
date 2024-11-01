//===-- H2BLBRegisterInfo.td - Descr the H2BLB reginfo ----*- tablegen -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Register description of the H2BLB target.
//
// The register hierarchy looks like:
//        +-----------+-----+-----+
// 32-bit |  d0 |  d1 |  d2 |  d3 |
//        +--+--+--+--+--+--+--+--+
// 16-bit |r0|r1|r2|r3|r4|r5|r6|r7|
//        +--+--+--+--+--+--+--+--+
//===----------------------------------------------------------------------===//
let Namespace = "H2BLB" in {
class H2BLBReg<bits<16> enc, string name, list<Register> subregs = []>
        : Register<name> {
  let HWEncoding = enc;
  let SubRegs = subregs;
  // When the subregs are specified, they cover the full register.
  // Note: Remember that !empty means the function empty, not "not empty".
  let CoveredBySubRegs = !if(!empty(subregs), false, true);
}

// Subreg definitions.
def sub_low16 : SubRegIndex<16>;
def sub_high16 : SubRegIndex<16, 16>;

// Register definitions.
def R0 : H2BLBReg<0, "r0">;
def R1 : H2BLBReg<1, "r1">;
def R2 : H2BLBReg<2, "r2">;
def R3 : H2BLBReg<3, "r3">;
def R4 : H2BLBReg<4, "r4">;
def R5 : H2BLBReg<5, "r5">;
def R6 : H2BLBReg<6, "r6">;
def R7 : H2BLBReg<7, "r7">;

let SubRegIndices = [sub_low16, sub_high16] in {
  def D0 : H2BLBReg<0, "d0", [R0, R1]>;
  def D1 : H2BLBReg<1, "d1", [R2, R3]>;
  def D2 : H2BLBReg<2, "d2", [R4, R5]>;
  def D3 : H2BLBReg<3, "d3", [R6, R7]>;
}

// Stack register, it is its own thing.
def SP : H2BLBReg<0, "sp">;

// Register classes.
def GPR16 : RegisterClass<"H2BLB", [i16], 16, (sequence "R%u", 0, 7)>;
def GPR32 : RegisterClass<"H2BLB", [i32,v2i16], 32, (sequence "D%u", 0, 3)>;
def OnlySP : RegisterClass<"H2BLB", [i16], 16, (add SP)>;
def GPR16sp : RegisterClass<"H2BLB", [i16], 16, (add GPR16, OnlySP)>;
} // end namespace H2BLB.
