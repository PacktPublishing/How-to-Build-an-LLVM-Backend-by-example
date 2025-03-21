; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --version 5
; RUN: opt -S -o - %s | FileCheck %s
; Regular function calls must use symbols be defined or declared in the same
; module.
; However, for intrinsics, the middle-end is able to add the declarations
; on the fly.
; Therefore, this test passes if the h2blb--darwin intrinsics are properly hooked up.

target triple="h2blb--darwin"

define i32 @widening_smul(i16 %a, i16 %b) {
; CHECK-LABEL: define i32 @widening_smul(
; CHECK-SAME: i16 [[A:%.*]], i16 [[B:%.*]]) {
; CHECK-NEXT:    [[RES:%.*]] = call i32 @llvm.h2blb.widening.smul(i16 [[A]], i16 [[B]])
; CHECK-NEXT:    ret i32 [[RES]]
;
  %res = call i32 @llvm.h2blb.widening.smul(i16 %a, i16 %b)
  ret i32 %res
}

define i32 @widening_umul(i16 %a, i16 %b) {
; CHECK-LABEL: define i32 @widening_umul(
; CHECK-SAME: i16 [[A:%.*]], i16 [[B:%.*]]) {
; CHECK-NEXT:    [[RES:%.*]] = call i32 @llvm.h2blb.widening.umul(i16 [[A]], i16 [[B]])
; CHECK-NEXT:    ret i32 [[RES]]
;
  %res = call i32 @llvm.h2blb.widening.umul(i16 %a, i16 %b)
  ret i32 %res
}
