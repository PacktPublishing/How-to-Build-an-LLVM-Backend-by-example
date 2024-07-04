; RUN: llc -O0 -mtriple h2blb %s -debug-pass=Structure --stop-before=peephole-opt -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-O0
; RUN: llc -O1 -mtriple h2blb %s -debug-pass=Structure --stop-before=peephole-opt -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-Opt
; RUN: llc -O2 -mtriple h2blb %s -debug-pass=Structure --stop-before=peephole-opt -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-Opt
; RUN: llc -O3 -mtriple h2blb %s -debug-pass=Structure --stop-before=peephole-opt -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-Opt

; Check that the H2BLB custom codegen pipeline includes the target specific
; simple constant propagation pass, but only when the optimizations are enabled.

; CHECK-O0-NOT: H2BLB simple constant propagation
; CHECK-Opt: H2BLB simple constant propagation
