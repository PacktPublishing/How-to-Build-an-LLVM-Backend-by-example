// RUN: not llvm-mc -triple=h2blb %s  -o /dev/null 2>&1 | FileCheck %s

// Check that we reject the strsp when it is ill-formed.

// CHECK: error: invalid operand for instruction
// CHECK: wsmul r0
wsmul r0, r0, r0

// CHECK: error: too few operands for instruction
// CHECK: wsmul #d1
wsmul #d1, r0, r3

// CHECK: error: invalid operand for instruction
// CHECK: wsmul d3, r1, 1
wsmul d3, r1, 1

// Same checks for the unsigned variant.

// CHECK: error: invalid operand for instruction
// CHECK: wumul r0
wumul r0, r0, r0

// CHECK: error: too few operands for instruction
// CHECK: wumul #d1
wumul #d1, r0, r3

// CHECK: error: invalid operand for instruction
// CHECK: wumul d3, r1, 1
wumul d3, r1, 1
