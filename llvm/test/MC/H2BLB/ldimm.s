// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For ldi16|32 encoding from high bits to low bits:
// opc<5> = 0b00010
// is32Bit<1> = 0|1
// dst<3>, imm<7>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -------------7------ 0
//   |               |       |
// 0b00010<is32Bit><dst><imm7>

// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b00010<0><101><0001100>
//  high byte - low byte
// 0b00010<0><10-1><0001100>
// => 0x12    - 0x8c
// CHECK: ldi16 r5, 12
// ENCODING: [0x8c,0x12]
ldi16 r5, 12

// Encoding goes:
// 0b00010<0><111><1111111>
//  high byte - low byte
// 0b00010<0><11-1><1111111>
// => 0x13    - 0xff
// CHECK: ldi16 r7, 127
// ENCODING: [0xff,0x13]
ldi16 r7, 127

// 32-bit

// Encoding goes:
// 0b00010<1><001><0001100>
//  high byte - low byte
// 0b00010<1><00-1><0001100>
// => 0x14    - 0x8c
// CHECK: ldi32 d1, 12
// ENCODING: [0x8c,0x14]
ldi32 d1, 12

// Encoding goes:
// 0b00010<1><011><1111111>
//  high byte - low byte
// 0b00010<1><01-1><1111111>
// => 0x15    - 0xff
// CHECK: ldi32 d3, 127
// ENCODING: [0xff,0x15]
ldi32 d3, 127
