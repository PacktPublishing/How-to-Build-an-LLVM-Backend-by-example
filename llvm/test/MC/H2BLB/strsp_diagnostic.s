// RUN: not llvm-mc -triple=h2blb %s  -o /dev/null 2>&1 | FileCheck %s

// Check that we reject the strsp when it is ill-formed.

// CHECK: error: invalid operand for instruction
// CHECK: strsp16 d0
strsp16 d0, sp, 12

// CHECK: error: too few operands for instruction
// CHECK: strsp16 #r1
strsp16 #r1, sp, 3

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: strsp16 r1, sp, r1
strsp16 r1, sp, r1

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: strsp16 r1, sp, -73
strsp16 r1, sp, -73

// CHECK: error: invalid operand for instruction
// CHECK: strsp16 r1, r3, 0
strsp16 r1, r3, 0

// Same checks for the 32-bit variant.

// CHECK: error: invalid operand for instruction
// CHECK: strsp32 r0
strsp32 r0, sp, 12

// CHECK: error: too few operands for instruction
// CHECK: strsp32 #d1
strsp32 #d1, sp, 3

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: strsp32 d1, sp, r1
strsp32 d1, sp, r1

// CHECK: error: immediate must be an integer in range [-64, 63].
// CHECK: strsp32 d1, sp, -73
strsp32 d1, sp, -73

// CHECK: error: invalid operand for instruction
// CHECK: strsp32 d1, r3, 0
strsp32 d1, r3, 0
