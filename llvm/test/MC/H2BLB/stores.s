// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For str<16|32> encoding from high bits to low bits:
// opc<5> = 0b00101
// is32Bit<1> = 0|1
// dst<3>, base<3>, <offset><4>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -------------7-------------- 0
//   |               |               |
// 0b00101<is32Bit><dst><base><offset>

// str16
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b00101<0><010><001><0101>
//  high byte   -   low byte
// 0b00101<0><01-0><001><0101>
// => 0x29      -   0x15
// CHECK: str16 r2, r1, 5
// ENCODING: [0x15,0x29]
str16 r2, r1, 5

// str32
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b00101<1><001><110><0111>
//  high byte   -   low byte
// 0b00101<1><00-1><110><0111>
// => 0x2c      -   0xe7
// CHECK: str32 d1, r6, 7
// ENCODING: [0xe7,0x2c]
str32 d1, r6, 7
