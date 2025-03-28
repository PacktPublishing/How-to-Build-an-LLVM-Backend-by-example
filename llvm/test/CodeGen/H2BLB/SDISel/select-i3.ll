; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py UTC_ARGS: --version 5
; RUN: llc -fast-isel=0 %s -mtriple=h2blb--darwin -o - -enable-misched=false | FileCheck %s

define void @clampi3(ptr %src, ptr %dst) {
; CHECK-LABEL: clampi3:
; CHECK:       # %bb.0:
; CHECK-NEXT:    ldrzext8 r1, r1, 0
; CHECK-NEXT:    ldi16 r3, 2
; CHECK-NEXT:    addi16 r1, r1, r3
; CHECK-NEXT:    ldi16 r3, 7
; CHECK-NEXT:    and16 r1, r1, r3
; CHECK-NEXT:    truncstr8 r1, r2, 0
; CHECK-NEXT:    ret
  %val = load i3, ptr %src
  %res = add i3 %val, 2
  store i3 %res, ptr %dst
  ret void
}
