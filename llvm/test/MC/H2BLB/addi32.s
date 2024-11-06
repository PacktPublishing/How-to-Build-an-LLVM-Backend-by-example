// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For addi32 encoding from high bits to low bits:
// opc<5> = 0
// is32Bit<1> = 1
// dst<3>, src0<3>, src1<3>, <spare==0><1>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -----7------------- 0
//   |       |              |
// 0b000001<dst><src0><src1>0

// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b000001<000><001><011>0
//  high byte - low byte
// 0b000001<00-0><001><011>0
// => 0x04    - 0x16
// CHECK: addi32 d0, d1, d3
// ENCODING: [0x16,0x04]
addi32 d0, d1, d3

// Add more testing for the encoding.
// Encoding goes:
// 0b000001<011><001><001>0
//  high byte - low byte
// 0b000001<01-1><001><001>0
// => 0x05    - 0x92
// CHECK: addi32 d3, d1, d1
// ENCODING: [0x92,0x05]
addi32 d3, d1, d1
