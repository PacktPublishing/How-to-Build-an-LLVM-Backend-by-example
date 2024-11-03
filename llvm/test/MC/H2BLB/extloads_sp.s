// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For ldr<s|z>extsp8 encoding from high bits to low bits:
// opc<5> = 0b01011
// isZExt<1> = 0|1
// dst<3>, <offset><7>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 ------------7-------- 0
//   |              |         |
// 0b01001<isZExt><dst><offset>

// ldrsextsp8
// Check that we can parse and print back our inldruction.
// Encoding goes:
// 0b01011<0><010><0010101>
//  high byte   -   low byte
// 0b01011<0><01-0><0010101>
// => 0x59      -   0x15
// CHECK: ldrsextsp8 r2, sp, 21
// ENCODING: [0x15,0x59]
ldrsextsp8 r2, sp, 21

// ldrzextsp32
// Check that we can parse and print back our inldruction.
// Encoding goes:
// 0b01011<1><001><0000111>
//  high byte   -   low byte
// 0b01011<1><00-1><0000111>
// => 0x5c      -   0x87
// CHECK: ldrzextsp8 r1, sp, 7
// ENCODING: [0x87,0x5c]
ldrzextsp8 r1, sp, 7
