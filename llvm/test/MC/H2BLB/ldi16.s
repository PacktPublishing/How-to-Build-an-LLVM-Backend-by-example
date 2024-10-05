// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For ldi16 encoding from high bits to low bits:
// opc<5> = 0b00010
// is32Bit<1> = 0
// dst<3>, imm<7>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -----7------ 0
//   |       |       |
// 0b000100<dst><imm7>

// Check that we can parse and print back our only instruction.
// Encoding goes:
// 0b000100<101><0001100>
//  high byte - low byte
// 0b000100<10-1><0001100>
// => 0x12    - 0x8c
// CHECK: ldi16 r5, 12
// ENCODING: [0x8c,0x12]
ldi16 r5, 12

// Encoding goes:
// 0b000100<111><1111111>
//  high byte - low byte
// 0b000100<11-1><1111111>
// => 0x13    - 0xff
// CHECK: ldi16 r7, 127
// ENCODING: [0xff,0x13]
ldi16 r7, 127
