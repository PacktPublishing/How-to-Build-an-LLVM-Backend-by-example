// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For ret encoding from high bits to low bits:
// opc<5> = 1
// is32Bit<1> = 0
// dst<3>, src0<3>, src1<3>, <spare==0><1>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -----7-0
//   |       | |
// 0b0000010...0

// Encoding goes:
// 0b000010...0
//  high byte - low byte
// 0b00001000 - 0...0
// => 0x08    - 0x0
// CHECK: ret
// ENCODING: [0x00,0x08]
ret
