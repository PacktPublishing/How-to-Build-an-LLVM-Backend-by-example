; RUN: opt -O0 -mtriple=h2blb -debug-pass-manager %s -S -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-O0
; RUN: opt -O1 -mtriple=h2blb -debug-pass-manager %s -S -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-Opt
; RUN: opt -O2 -mtriple=h2blb -debug-pass-manager %s -S -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-Opt
; RUN: opt -O3 -mtriple=h2blb -debug-pass-manager %s -S -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-Opt

; Check that the simple constant propagation pass is only added in the
; optimizing pipelines.
; CHECK-O0-NOT: H2BLBSimpleConstantPropagationNewPass
; CHECK-Opt: H2BLBSimpleConstantPropagationNewPass

; The tested function doesn't matter.
define i32 @foo(i32 noundef %arg) {
foo.bb:
  %i = shl i32 5, 3
  %i1 = icmp ne i32 %arg, 0
  br i1 %i1, label %foo.bb2, label %foo.bb4

foo.bb2:
  %i3 = sdiv i32 %i, 5
  br label %foo.bb6

foo.bb4:
  %i5 = or i32 %i, 3855
  br label %foo.bb6

foo.bb6:
  %.0 = phi i32 [ %i3, %foo.bb2 ], [ %i5, %foo.bb4 ]
  ret i32 %.0
}
