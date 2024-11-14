// RUN: not llvm-mc -triple=h2blb %s  -o /dev/null 2>&1 | FileCheck %s

// Check that we reject the ldr<s|z>ext8 when it is ill-formed.

// CHECK: error: unrecognized instruction mnemonic
// CHECK-NEXT: ldrsext16
ldrsext16 r0, sp, 12

// CHECK: error: invalid operand for instruction
// CHECK: ldrsext8 d0
ldrsext8 d0, sp, 12

// CHECK: error: invalid operand for instruction
// CHECK: ldrsext8 r0, sp
ldrsext8 r0, sp, 12

// CHECK: error: too few operands for instruction
// CHECK: ldrsext8 #r1
ldrsext8 #r1, sp, 3

// CHECK: error: immediate must be an integer in range [0, 15].
// CHECK: ldrsext8 r1, r3
ldrsext8 r1, r3, r1

// CHECK: error: immediate must be an integer in range [0, 15].
// CHECK: ldrsext8 r1, r3, 160
ldrsext8 r1, r3, 160

// CHECK: error: immediate must be an integer in range [0, 15].
// CHECK: ldrsext8 r1, r3, -65
ldrsext8 r1, r3, -65

// CHECK: error: unrecognized instruction mnemonic
// CHECK-NEXT: ldrzextsp16
ldrzextsp16 r0, sp, 12

// CHECK: error: invalid operand for instruction
// CHECK: ldrzext8 d0
ldrzext8 d0, sp, 12

// CHECK: error: invalid operand for instruction
// CHECK: ldrzext8 r0, sp
ldrzext8 r0, sp, 12

// CHECK: error: too few operands for instruction
// CHECK: ldrzext8 #r1
ldrzext8 #r1, sp, 3

// CHECK: error: immediate must be an integer in range [0, 15].
// CHECK: ldrzext8 r1, r3
ldrzext8 r1, r3, r1

// CHECK: error: immediate must be an integer in range [0, 15].
// CHECK: ldrzext8 r1, r3, 160
ldrzext8 r1, r3, 160

// CHECK: error: immediate must be an integer in range [0, 15].
// CHECK: ldrzext8 r1, r3, -65
ldrzext8 r1, r3, -65
