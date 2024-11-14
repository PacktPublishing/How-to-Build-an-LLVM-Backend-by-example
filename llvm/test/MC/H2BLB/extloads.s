// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For ldr<s|z>extsp8 encoding from high bits to low bits:
// opc<5> = 0b01101
// isZExt<1> = 0|1
// dst<3>, src<3>, <offset><4>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 ------------7------------- 0
//   |              |              |
// 0b01101<isZExt><dst><src><offset>

// ldrsext8
// Check that we can parse and print back our inldruction.
// Encoding goes:
// 0b01101<0><010><101><1111>
//  high byte   -   low byte
// 0b01101<0><01-0><101><1111>
// => 0x69      -   0x5f
// CHECK: ldrsext8 r2, r5, 15
// ENCODING: [0x5f,0x69]
ldrsext8 r2, r5, 15

// ldrzext8
// Check that we can parse and print back our inldruction.
// Encoding goes:
// 0b01101<1><001><111>0111>
//  high byte   -   low byte
// 0b01101<1><00-1><111><0111>
// => 0x6c      -   0xf7
// CHECK: ldrzext8 r1, r7, 7
// ENCODING: [0xf7,0x6c]
ldrzext8 r1, r7, 7
