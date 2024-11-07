// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For and<16|32> encoding from high bits to low bits:
// opc<5> = 0b01100
// is32Bit<1> = 0|1
// dst<3>, src0<3>, src1<3>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -------------7------------- 0
//   |               |              |
// 0b01100<is32Bit><dst><src0><src1>0

// and16
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b01100<0><010><001><101>0
//  high byte   -   low byte
// 0b01100<0><01-0><001><101>0
// => 0x61      -   0x1a
// CHECK: and16 r2, r1, r5
// ENCODING: [0x1a,0x61]
and16 r2, r1, r5

// and32
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b01100<1><001><011><010>0
//  high byte   -   low byte
// 0b01100<1><00-1><011><010>0
// => 0x64      -   0xb4
// CHECK: and32 d1, d3, d2
// ENCODING: [0xb4,0x64]
and32 d1, d3, d2
