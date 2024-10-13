// RUN: not llvm-mc -triple=h2blb %s  -o /dev/null 2>&1 | FileCheck %s

// Check that we reject the ldi16 when it is ill-formed.

// CHECK: error: unrecognized instruction mnemonic
// CHECK-NEXT: ldr13
ldr13 r0, r1, 12

// CHECK: error: invalid operand for instruction
// CHECK: ldr16 d0
ldr16 d0, r1, 12

// CHECK: error: too few operands for instruction
// CHECK: ldr16 #r1
ldr16 #r1, r1, 3

// CHECK: error: immediate must be an integer in range [0, 15].
// CHECK: ldr16 r1, r1
ldr16 r1, r1, r1

// CHECK: error: immediate must be an integer in range [0, 15].
// CHECK: ldr16 r1, r2, 16
ldr16 r1, r2, 16
