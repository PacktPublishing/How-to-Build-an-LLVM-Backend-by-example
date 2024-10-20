// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For strsp<16|32> encoding from high bits to low bits:
// opc<5> = 0b01000
// is32Bit<1> = 0|1
// src<3>, <offset><7>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -------------7-------- 0
//   |               |         |
// 0b01000<is32Bit><dst><offset>

// strsp16
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b01000<0><010><0010101>
//  high byte   -   low byte
// 0b01000<0><01-0><0010101>
// => 0x41      -   0x15
// CHECK: strsp16 r2, sp, 21
// ENCODING: [0x15,0x41]
strsp16 r2, sp, 21

// strsp32
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b01000<1><001><0000111>
//  high byte   -   low byte
// 0b01000<1><00-1><000><0111>
// => 0x44      -   0x87
// CHECK: strsp32 d1, sp, 7
// ENCODING: [0x87,0x44]
strsp32 d1, sp, 7
