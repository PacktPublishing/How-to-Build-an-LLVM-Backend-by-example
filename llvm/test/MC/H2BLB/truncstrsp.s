// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For strsp<16|32> encoding from high bits to low bits:
// opc<5> = 0b01110
// unused<1> = 0
// src<3>, <addr>, <offset><4>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -----7-------------- 0
//   |       |               |
// 0b011100<src><addr><offset>

// truncstr8
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b011100<010><001><0101>
//  high byte - low byte
// 0b011100<01-0><001><0101>
// => 0x71    -   0x15
// CHECK: truncstr8 r2, r1, 5
// ENCODING: [0x15,0x71]
truncstr8 r2, r1, 5

// strsp32
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b011100<001><000><0111>
//  high byte - low byte
// 0b011100<00-1><000><0111>
// => 0x70    -   0x87
// CHECK: truncstr8 r1, r0, 7
// ENCODING: [0x87,0x70]
truncstr8 r1, r0, 7
