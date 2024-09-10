// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For addi16 encoding from high bits to low bits:
// opc<5> = 0
// is32Bit<1> = 0
// dst<3>, src0<3>, src1<3>, <spare==0><1>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -----7------------- 0
//   |       |              |
// 0b000000<dst><src0><src1>0

// Check that we can parse and print back our only instruction.
// Encoding goes:
// 0b000000<000><001><011>0
//  high byte - low byte
// 0b000000<00-0><001><011>0
// => 0x00    - 0x16
// CHECK: addi16 r0, r1, r3
// ENCODING: [0x16,0x00]
addi16 r0, r1, r3

// Add more testing for the encoding.
// Encoding goes:
// 0b000000<011><111><101>0
//  high byte - low byte
// 0b000000<01-1><111><101>0
// => 0x01    - 0xfa
// CHECK: addi16 r3, r7, r5
// ENCODING: [0xfa,0x01]
addi16 r3, r7, r5
