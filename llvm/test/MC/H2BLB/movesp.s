// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For mov<to|from>sp encoding from high bits to low bits:
// opc<5> = 0b00111
// toSP<1> = 0|1
// dst<3>, src<3>, 0b0000
// The display of the encoding is low to high, byte per byte.
// So:
//   15 ----------7--------- 0
//   |            |          |
// 0b00110<toSP><dst><src>0000

// Check that we can parse and print back our instruction.
// movtosp
// Encoding goes:
// 0b00111<1><000><101>0000
//  high byte   -   low byte
// 0b00111<1><00-0><101>0000
// => 0x3c      -   0x50
// CHECK: movtosp sp, r5
// ENCODING: [0x50,0x3c]
movtosp sp, r5

// movfromsp
// Encoding goes:
// 0b00111<0><011><000>0000
//  high byte   -   low byte
// 0b00111<0><01-1><000>0000
// => 0x39      -   0x80
// CHECK: movfromsp r3, sp
// ENCODING: [0x80,0x39]
movfromsp r3, sp
