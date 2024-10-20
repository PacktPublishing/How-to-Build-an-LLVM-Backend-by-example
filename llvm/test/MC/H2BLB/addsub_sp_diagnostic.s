// RUN: not llvm-mc -triple=h2blb %s  -o /dev/null 2>&1 | FileCheck %s

// Check that we reject the strsp when it is ill-formed.

// CHECK: error: invalid operand for instruction
// CHECK: addsp d0
addsp d0, sp, 12

// CHECK: error: too few operands for instruction
// CHECK: addsp #r1
addsp #r1, sp, 3

// CHECK: error: immediate must be an integer in range [0, 1023].
// CHECK: addsp sp, sp, r1
addsp sp, sp, r1

// CHECK: error: immediate must be an integer in range [0, 1023].
// CHECK: addsp sp, sp, -73
addsp sp, sp, -73

// CHECK: error: invalid operand for instruction
// CHECK: addsp sp, r3, 0
addsp sp, r3, 0

// CHECK: error: immediate must be an integer in range [0, 1023].
// CHECK: addsp sp, sp, 1024
addsp sp, sp, 1024

// Same checks for the subsp variant.

// CHECK: error: invalid operand for instruction
// CHECK: subsp d0
subsp d0, sp, 12

// CHECK: error: too few operands for instruction
// CHECK: subsp #r1
subsp #r1, sp, 3

// CHECK: error: immediate must be an integer in range [0, 1023].
// CHECK: subsp sp, sp, r1
subsp sp, sp, r1

// CHECK: error: immediate must be an integer in range [0, 1023].
// CHECK: subsp sp, sp, -73
subsp sp, sp, -73

// CHECK: error: invalid operand for instruction
// CHECK: subsp sp, r3, 0
subsp sp, r3, 0

// CHECK: error: immediate must be an integer in range [0, 1023].
// CHECK: subsp sp, sp, 1024
subsp sp, sp, 1024
