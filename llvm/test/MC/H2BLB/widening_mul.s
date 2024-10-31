// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For strsp<16|32> encoding from high bits to low bits:
// opc<5> = 0b00000
// isSign<1> = 0|1
// dst<2>, src1<3>, src2<3>
// opc<6> = 1
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -------------7------------ 0
//   |               |             |
// 0b00000<isSign><dst><src1><src2>1

// wsmul
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b00000<1><010><001><010>1
//  high byte   -   low byte
// 0b00000<1><01-0><001><010>1
// => 0x05      -   0x15
// CHECK: wsmul d2, r1, r2
// ENCODING: [0x15,0x05]
wsmul d2, r1, r2

// wumul
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b00000<0><010><111><001>1
//  high byte   -   low byte
// 0b00000<0><01-0><111><001>1
// => 0x01      -   0x73
// CHECK: wumul d2, r7, r1
// ENCODING: [0x73,0x01]
wumul d2, r7, r1
