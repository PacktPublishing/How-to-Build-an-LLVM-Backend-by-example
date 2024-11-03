// RUN: not llvm-mc -triple=h2blb %s  -o /dev/null 2>&1 | FileCheck %s

// Check that we reject the ldr<s|z>extsp8 when it is ill-formed.

// CHECK: error: unrecognized instruction mnemonic
// CHECK-NEXT: ldrsextsp16
ldrsextsp16 r0, sp, 12

// CHECK: error: invalid operand for instruction
// CHECK: ldrsextsp8 d0
ldrsextsp8 d0, sp, 12

// CHECK: error: too few operands for instruction
// CHECK: ldrsextsp8 #r1
ldrsextsp8 #r1, sp, 3

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: ldrsextsp8 r1, sp
ldrsextsp8 r1, sp, r1

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: ldrsextsp8 r1, sp, 160
ldrsextsp8 r1, sp, 160

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: ldrsextsp8 r1, sp, -65
ldrsextsp8 r1, sp, -65

// CHECK: error: unrecognized instruction mnemonic
// CHECK-NEXT: ldrzextsp16
ldrzextsp16 r0, sp, 12

// CHECK: error: invalid operand for instruction
// CHECK: ldrzextsp8 d0
ldrzextsp8 d0, sp, 12

// CHECK: error: too few operands for instruction
// CHECK: ldrzextsp8 #r1
ldrzextsp8 #r1, sp, 3

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: ldrzextsp8 r1, sp
ldrzextsp8 r1, sp, r1

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: ldrzextsp8 r1, sp, 160
ldrzextsp8 r1, sp, 160

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: ldrzextsp8 r1, sp, -65
ldrzextsp8 r1, sp, -65
