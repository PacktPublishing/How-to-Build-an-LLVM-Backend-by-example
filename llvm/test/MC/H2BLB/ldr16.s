// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For ldr16 encoding from high bits to low bits:
// opc<5> = 0b00011
// is32Bit<1> = 0
// dst<3>, base<3>, <offset><4>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -----7-------------- 0
//   |       |               |
// 0b000110<dst><base><offset>

// Check that we can parse and print back our only instruction.
// Encoding goes:
// 0b000110<010><001><0101>
//  high byte - low byte
// 0b000110<01-0><001><0101>
// => 0x19    - 0x15
// CHECK: ldr16 r2, r1, 5
// ENCODING: [0x15,0x19]
ldr16 r2, r1, 5
