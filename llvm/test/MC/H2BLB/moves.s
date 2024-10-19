// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For mov<16|32> encoding from high bits to low bits:
// opc<5> = 0b00110
// is32Bit<1> = 0|1
// dst<3>, src<3>, 0b0000
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -------------7--------- 0
//   |               |          |
// 0b00110<is32Bit><dst><src>0000

// Check that we can parse and print back our instruction.
// mov16
// Encoding goes:
// 0b001100<010><001>0000
//  high byte - low byte
// 0b001100<01-0><001>0000
// => 0x31    - 0x10
// CHECK: mov16 r2, r1
// ENCODING: [0x10,0x31]
mov16 r2, r1

// mov32
// Encoding goes:
// 0b001101<011><000>0000
//  high byte - low byte
// 0b001101<01-1><000>0000
// => 0x35    - 0x80
// CHECK: mov32 d3, d0
// ENCODING: [0x80,0x35]
mov32 d3, d0
