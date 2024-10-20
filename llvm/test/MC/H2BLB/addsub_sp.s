// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For add|subsp encoding from high bits to low bits:
// opc<5> = 0b01010
// isAdd<1> = 0|1
// <offset><10>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 -----------7--- 0
//   |             |    |
// 0b01010<isAdd><offset>

// addsp
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b01010<1><0000010101>
//  high byte   -   low byte
// 0b01010<1><00-0><0010101>
// => 0x54      -   0x15
// CHECK: addsp sp, sp, 21
// ENCODING: [0x15,0x54]
addsp sp, sp, 21

// subsp
// Check that we can parse and print back our instruction.
// Encoding goes:
// 0b01010<0><1111010111>
//  high byte   -   low byte
// 0b01010<0><11-11010111>
// => 0x53      -   0xd7
// CHECK: subsp sp, sp, 983
// ENCODING: [0xd7,0x53]
subsp sp, sp, 983
