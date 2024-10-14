// RUN: llvm-mc -triple=h2blb %s  -o - | FileCheck %s
// RUN: llvm-mc -triple=h2blb %s  -o - --show-encoding | FileCheck --check-prefix=ENCODING %s

// For call encoding from high bits to low bits:
// opc<5> = 0b00100
// target<11>
// The display of the encoding is low to high, byte per byte.
// So:
//   15 ---7---- 0
//   |     |     |
// 0b00100<target>

// Check that we can parse and print back.
// The target is a fix-up at this point, so from the encoding perspective
// this is just 0.
// Encoding goes:
// 0b00100<0000000000>
//  high byte - low byte
// 0b00100<000-00000000>
// => 0x20    - 0x00
// CHECK: call bar
// ENCODING: [0x00,0x20]
call bar
