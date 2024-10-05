// REQUIRES: h2blb-registered-target
// RUN: %clang --target=h2blb %s -o - -S | FileCheck %s

// Check that we can connect clang from the driver (as opposed to cc1) all
// the way to assembly code.

// CHECK-LABEL: empty
// CHECK: ret
void empty() {
}
