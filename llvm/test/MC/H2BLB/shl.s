// RUN: llvm-mc -triple=h2blb--darwin %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb--darwin %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For shl<16|32> encoding from high bits to low bits:
// opc<5> = 0b01111
// is32Bit<1> = 0|1
// dst<3>, src0<3>, src1<3>, <spare==0><1>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -------------7------------- 0
//   |               |              |
// 0b01111<is32Bit><dst><src0><src1>0

// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b01111<0><011><111><101>0
//  high byte   -   low byte
// 0b01111<0><01-1><111><101>0
// => 0x79      - 0xfa
// CHECK: shl16 r3, r7, r5
// ENCODING: [0xfa,0x79]
shl16 r3, r7, r5

// Add more testing for the encoding.
// Encoding goes:
// 0b01111<1><000><001><011>0
//  high byte   -   low byte
// 0b01111<1><00-0><001><011>0
// => 0x7c      -  0x16
// CHECK: shl32 d0, d1, r3
// ENCODING: [0x16,0x7c]
shl32 d0, d1, r3
