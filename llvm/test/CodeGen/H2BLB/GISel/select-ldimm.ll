; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py UTC_ARGS: --version 5
; RUN: llc -mtriple h2blb--darwin -o - %s -global-isel=1 -global-isel-abort=1 -verify-machineinstrs | FileCheck %s
target triple="h2blb--darwin"

define i16 @ldi16() {
; CHECK-LABEL: ldi16:
; CHECK:       # %bb.0:
; CHECK-NEXT:    ldi16 r1, 13
; CHECK-NEXT:    ret
  ret i16 13
}

define i32 @ldi32() {
; CHECK-LABEL: ldi32:
; CHECK:       # %bb.0:
; CHECK-NEXT:    ldi32 d1, 123
; CHECK-NEXT:    ret
  ret i32 123
}

; Check that we can expand constant that do not fit in our i7 range.

; We expect:
; 3 << 14 | 1 << 7 | 4
;
; We do << 14 with two << 7.
; This is not most efficient code, but it works.
define i16 @retCst() {
; CHECK-LABEL: retCst:
; CHECK:       # %bb.0:
; CHECK-NEXT:    ldi16 r1, 3
; CHECK-NEXT:    ldi16 r2, 7
; CHECK-NEXT:    shl16 r1, r1, r2
; CHECK-NEXT:    ldi16 r3, 1
; CHECK-NEXT:    or16 r1, r1, r3
; CHECK-NEXT:    shl16 r1, r1, r2
; CHECK-NEXT:    ldi16 r2, 4
; CHECK-NEXT:    or16 r1, r1, r2
; CHECK-NEXT:    ret
  ret i16 49284
}

; We expect:
; 2 << 14 | 1
define i16 @retCst2() {
; CHECK-LABEL: retCst2:
; CHECK:       # %bb.0:
; CHECK-NEXT:    ldi16 r1, 2
; CHECK-NEXT:    ldi16 r2, 7
; CHECK-NEXT:    shl16 r1, r1, r2
; CHECK-NEXT:    shl16 r1, r1, r2
; CHECK-NEXT:    ldi16 r2, 1
; CHECK-NEXT:    or16 r1, r1, r2
; CHECK-NEXT:    ret
  ret i16 32769
}
