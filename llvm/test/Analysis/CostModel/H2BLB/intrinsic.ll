; RUN: opt -mtriple=h2blb -passes="print<cost-model>" -cost-kind=code-size -disable-output %s 2>&1 | FileCheck %s --check-prefix=SIZE
; RUN: opt -mtriple=h2blb -passes="print<cost-model>" -cost-kind=latency -disable-output %s 2>&1 | FileCheck %s --check-prefix=LATENCY
;
; Check that the cost of the intrinsics is the same for cost size, but that the
; smul one is more expensive than the umul one.

; SIZE: Printing analysis 'Cost Model Analysis' for function 'widening_smul':
; SIZE: Cost Model: Found an estimated cost of 1 for instruction:   %res = call i32 @llvm.h2blb.widening.smul(i16 %a, i16 %b)

; LATENCY: Printing analysis 'Cost Model Analysis' for function 'widening_smul':
; LATENCY: Cost Model: Found an estimated cost of 4 for instruction:   %res = call i32 @llvm.h2blb.widening.smul(i16 %a, i16 %b)
define i32 @widening_smul(i16 %a, i16 %b) {
  %res = call i32 @llvm.h2blb.widening.smul(i16 %a, i16 %b)
  ret i32 %res
}

; SIZE: Printing analysis 'Cost Model Analysis' for function 'widening_umul':
; SIZE: Cost Model: Found an estimated cost of 1 for instruction:   %res = call i32 @llvm.h2blb.widening.umul(i16 %a, i16 %b)

; LATENCY: Printing analysis 'Cost Model Analysis' for function 'widening_umul':
; LATENCY: Cost Model: Found an estimated cost of 1 for instruction:   %res = call i32 @llvm.h2blb.widening.umul(i16 %a, i16 %b)
define i32 @widening_umul(i16 %a, i16 %b) {
  %res = call i32 @llvm.h2blb.widening.umul(i16 %a, i16 %b)
  ret i32 %res
}
