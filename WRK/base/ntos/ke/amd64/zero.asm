        title  "Zero Page"
;++
;
; Copyright (c) Microsoft Corporation. All rights reserved. 
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
; Module Name:
;
;   zero.asm
;
; Abstract:
;
;   This module implements the architecture dependent code necessary to
;   zero pages of memory in the fastest possible way.
;
;--

include ksamd64.inc

        subttl  "Zero Single Page"
;++
;
; VOID
; KeZeroSinglePage (
;     IN PVOID PageBase
;     )
;
; Routine Description:
;
;   This routine zeros the specified page of memory using normal moves.
;
; Arguments:
;
;   PageBase (rcx) - Supplies the address of the pages to zero.
;
; Return Value:
;
;    None.
;
;--

        LEAF_ENTRY KeZeroSinglePage, _TEXT$00

        xor     eax, eax                ; clear register
        mov     edx, PAGE_SIZE / 64     ; set number of 64 byte blocks

        align   16

KiZP10: mov     [rcx], rax              ; zero 64-byte block
        mov     8[rcx], rax             ;
        mov     16[rcx], rax            ;
        add     rcx, 64                 ; advance to next block
        mov     (24 - 64)[rcx], rax     ;
        mov     (32 - 64)[rcx], rax     ;
        dec     edx                     ; decrement loop count
        mov     (40 - 64)[rcx], rax     ;
        mov     (48 - 64)[rcx], rax     ;
        mov     (56 - 64)[rcx], rax     ;
        jnz     short KiZP10            ; if nz, more bytes to zero
        ret                             ; return

        LEAF_END KeZeroSinglePage, _TEXT$00

        subttl  "Zero Pages"
;++
;
; VOID
; KeZeroPages (
;     IN PVOID PageBase,
;     IN SIZE_T NumberOfBytes
;     )
;
; Routine Description:
;
;   This routine zeros the specified pages of memory using nontemporal moves.
;
; Arguments:
;
;   PageBase (rcx) - Supplies the address of the pages to zero.
;
;   NumberOfBytes (rdx) - Supplies the number of bytes to zero.  Always a PAGE_SIZE multiple.
;
; Return Value:
;
;    None.
;
;--

        LEAF_ENTRY KeZeroPages, _TEXT$00

        xor     eax, eax                ; clear register
        shr     rdx, 7                  ; number of 128 byte chunks (loop count)

        align   16

KiZS10: movnti  [rcx], rax              ; zero 128-byte block
        movnti  [rcx +  8], rax         ;
        movnti  [rcx + 16], rax         ;
        movnti  [rcx + 24], rax         ;
        movnti  [rcx + 32], rax         ;
        movnti  [rcx + 40], rax         ;
        movnti  [rcx + 48], rax         ;
        movnti  [rcx + 56], rax         ;
        add     rcx, 128                ; advance to next block
        movnti  [rcx - 64], rax         ;
        movnti  [rcx - 56], rax         ;
        movnti  [rcx - 48], rax         ;
        movnti  [rcx - 40], rax         ;
        movnti  [rcx - 32], rax         ;
        movnti  [rcx - 24], rax         ;
        movnti  [rcx - 16], rax         ;
        movnti  [rcx -  8], rax         ;
        dec     rdx                     ; decrement loop count
        jnz     short KiZS10            ; if nz, more bytes to zero
   lock or      byte ptr [rsp], 0       ; flush data to memory
        ret                             ; return

        LEAF_END KeZeroPages, _TEXT$00

;++
;
; VOID
; KeCopyPage (
;     IN PVOID Destination,
;     IN PVOID Source
;     )
;
; Routine Description:
;
;   This routine copies a page of memory using nontemporal moves.
;
; Arguments:
;
;   Destination (rcx) - Supplies the address of the target page.
;
;   Source (rdx) - Supplies the address of the source page.
;
; Return Value:
;
;    None.
;
;--

        LEAF_ENTRY KeCopyPage, _TEXT$00

;
; Set rcx and rdx to the end of their respective pages
;

        add     rcx, PAGE_SIZE
        add     rdx, PAGE_SIZE

;
; Load the entire source page into the L0 cache
;

        mov     rax, -PAGE_SIZE

        align   16

KiCP10: prefetchnta [rdx + rax + 64 * 0]
        prefetchnta [rdx + rax + 64 * 1]
        prefetchnta [rdx + rax + 64 * 2]
        prefetchnta [rdx + rax + 64 * 3]
        add     rax, 64*4
        jnz     short KiCP10

;
; Now copy from L0 cache to the target
;

        mov     rax, -PAGE_SIZE

        align   16

KiCP20: movdqa  xmm0, [rdx + rax + 16 * 0]
        movdqa  xmm1, [rdx + rax + 16 * 1]
        movdqa  xmm2, [rdx + rax + 16 * 2]
        movdqa  xmm3, [rdx + rax + 16 * 3]
        movntdq [rcx + rax + 16 * 0], xmm0
        movntdq [rcx + rax + 16 * 1], xmm1
        movntdq [rcx + rax + 16 * 2], xmm2
        movntdq [rcx + rax + 16 * 3], xmm3
        add     rax, 16*4
        jnz     short KiCP20
        sfence
        ret

        LEAF_END KeCopyPage, _TEXT$00

        end

