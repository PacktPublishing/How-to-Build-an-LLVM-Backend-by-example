// RUN: not llvm-mc -triple=h2blb %s  -o /dev/null 2>&1 | FileCheck %s

// Check that we reject the addi16 when it is ill-formed.

// CHECK: error: unrecognized instruction mnemonic
// CHECK-NEXT: addi13
addi13 r0, r1, r3

// CHECK: error: invalid operand for instruction
// CHECK: addi16 d0
addi16 d0, r1, r3

// CHECK: error: too few operands for instruction
// CHECK: addi16 #r1
addi16 #r1, r1, r3

// CHECK: error: too few operands for instruction
// CHECK: addi16 r1
addi16 r1, r1

