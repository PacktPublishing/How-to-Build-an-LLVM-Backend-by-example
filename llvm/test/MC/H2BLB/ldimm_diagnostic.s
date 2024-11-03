// RUN: not llvm-mc -triple=h2blb %s  -o /dev/null 2>&1 | FileCheck %s

// Check that we reject the ldi16 when it is ill-formed.

// CHECK: error: unrecognized instruction mnemonic
// CHECK-NEXT: ldi13
ldi13 r0, 12

// CHECK: error: invalid operand for instruction
// CHECK: ldi16 d0
ldi16 d0, 12

// CHECK: error: too few operands for instruction
// CHECK: ldi16 #r1
ldi16 #r1, r1, r3

// CHECK: error: immediate must be an integer in range [0, 127].
// CHECK: ldi16 r1, r1
ldi16 r1, r1

// CHECK: error: immediate must be an integer in range [0, 127].
// CHECK: ldi16 r1, 128
ldi16 r1, 128

// Same thing for the 32-bit variant

// CHECK: error: invalid operand for instruction
// CHECK: ldi32 r0
ldi32 r0, 12

// CHECK: error: too few operands for instruction
// CHECK: ldi32 #d1
ldi32 #d1, d1, d3

// CHECK: error: immediate must be an integer in range [0, 127].
// CHECK: ldi32 d1, r1
ldi32 d1, r1

// CHECK: error: immediate must be an integer in range [0, 127].
// CHECK: ldi32 d1, 128
ldi32 d1, 128
