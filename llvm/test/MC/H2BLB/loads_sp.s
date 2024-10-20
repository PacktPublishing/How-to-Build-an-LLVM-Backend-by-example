// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For ldrsp<16|32> encoding from high bits to low bits:
// opc<5> = 0b01001
// is32Bit<1> = 0|1
// src<3>, <offset><7>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -------------7-------- 0
//   |               |         |
// 0b01001<is32Bit><dst><offset>

// ldrsp16
// Check that we can parse and print back our inldruction.
// Encoding goes:
// 0b01001<0><010><0010101>
//  high byte   -   low byte
// 0b01001<0><01-0><0010101>
// => 0x49      -   0x15
// CHECK: ldrsp16 r2, sp, 21
// ENCODING: [0x15,0x49]
ldrsp16 r2, sp, 21

// ldrsp32
// Check that we can parse and print back our inldruction.
// Encoding goes:
// 0b01001<1><001><0000111>
//  high byte   -   low byte
// 0b01001<1><00-1><000><0111>
// => 0x4c      -   0x87
// CHECK: ldrsp32 d1, sp, 7
// ENCODING: [0x87,0x4c]
ldrsp32 d1, sp, 7
