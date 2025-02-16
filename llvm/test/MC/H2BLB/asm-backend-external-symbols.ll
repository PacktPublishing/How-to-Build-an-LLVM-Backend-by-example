; RUN: llc -o - %s -verify-machineinstrs --filetype=asm -mtriple "h2blb-darwin-" | FileCheck %s --check-prefix=ASM
; RUN: llc -o %t.o %s -verify-machineinstrs --filetype=obj -mtriple "h2blb-darwin-"
; RUN: llvm-objdump --section-headers --macho --reloc %t.o | FileCheck %s --check-prefix=OBJ

; First check that the asm looks like we think to see if the offsets make sense.
; We expect the first call instruction to be 6 bytes from the beginning of the
; file, because each instruction is 2 bytes and we have 3 instructions before
; the call. There's no offset in the instruction itself as the symbol resolution
; sits in the 11 less significant bits (i.e., the first 11 bits).
; Similarly, the second call should be 4 bytes later (total 10 bytes) since it
; should happen two instructions later.
;
; Finally, the total size of the text section (the code) should be 20 bytes
; (10 instructions * 2 bytes).
; ASM-LABEL: callTwoFcts:
; ASM: subsp
; ASM-NEXT: strsp32
; ASM-NEXT: mov16
; ASM-NEXT: call oneArgi16
; ASM-NEXT: mov16
; ASM-NEXT: call twoArgsi16
; ASM-NEXT: mov16
; ASM-NEXT: ldrsp32
; ASM-NEXT: addsp
; ASM-NEXT: ret

; Second, check that the relocation is at offset 6 (respecitvely 10),
; is marked as PC relative, references the proper symbol, and
; that the symbol is marked external:
; OBJ: Relocation information (__TEXT,__text) 2 entries
; OBJ-NEXT: address  pcrel length extern type    scattered symbolnum/value
; OBJ-NEXT: 0000000a True  word   True   2       False     twoArgsi16
; OBJ-NEXT: 00000006 True  word   True   2       False     oneArgi16
;
; Finally, check the size of the text section: 20 bytes: 0x14.
; OBJ: Sections:
; OBJ-NEXT: Idx Name          Size     VMA      Type
; OBJ-NEXT:  0 __text        00000014 00000000 TEXT
define i16 @callTwoFcts(i16 %arg) {
  %res = call i16 @oneArgi16(i16 %arg)
  %res2 = call i16 @twoArgsi16(i16 %res, i16 %res)
  ret i16 %res2
}

declare i16 @oneArgi16(i16 %arg)
declare i16 @twoArgsi16(i16 %arg, i16 %arg2)
