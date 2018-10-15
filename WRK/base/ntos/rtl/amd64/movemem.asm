        title  "Memory functions"
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
;   movemem.asm
;
; Abstract:
;
;   This module implements functions to fill, copy , and compare blocks of
;   memory.
;
;--

include ksamd64.inc

        extern  memset:proc

        subttl "Compare Memory"
;++
;
; SIZE_T
; RtlCompareMemory (
;     IN PVOID Source1,
;     IN PVOID Source2,
;     IN SIZE_T Length
;     )
;
; Routine Description:
;
;   This function compares two unaligned blocks of memory and returns the
;   number of bytes that compared equal.
;
; Arguments:
;
;   Source1 (rcx) - Supplies a pointer to the first block of memory to
;       compare.
;
;   Source2 (rdx) - Supplies a pointer to the second block of memory to
;       compare.
;
;   Length (r8) - Supplies the Length, in bytes, of the memory to be
;       compared.
;
; Return Value:
;
;   The number of bytes that compared equal is returned as the function
;   value. If all bytes compared equal, then the length of the original
;   block of memory is returned.
;
;--

CmFrame struct
        SavedRsi dq ?                   ; saved nonvolatile registers
        SavedRdi dq ?                   ;
CmFrame ends

        NESTED_ENTRY RtlCompareMemory, _TEXT$00

        alloc_stack (sizeof CmFrame)    ; allocate stack frame
        save_reg rsi, CmFrame.SavedRsi  ; save nonvolatile registers
        save_reg rdi, CmFrame.SavedRdi  ; 

        END_PROLOGUE

        mov     rsi, rcx                ; set address of first string
        mov     rdi, rdx                ; set address of second string
        xor     edx, ecx                ; check if compatible alignment
        and     edx, 07h                ;
        jnz     short RlCM50            ; if nz, incompatible alignment
        cmp     r8, 8                   ; check if length to align
        jb      short RlCM50            ; if b, insufficient alignment length

;
; Buffer alignment is compatible and there are enough bytes for alignment.
;

        mov     r9, rdi                 ; copy destination address
        neg     ecx                     ; compute alignment length
        and     ecx, 07h                ; 
        jz      short RlCM10            ; if z, buffers already aligned
        sub     r8, rcx                 ; reduce count by align length
   repe cmpsb                           ; compare bytes to alignment
        jnz     short RlCM30            ; if nz, not all bytes matched
RlCM10: mov     rcx, r8                 ;
        and     rcx, -8                 ; check if any quarwords to compare
        jz      short RlCM20            ; if z, no quadwords to compare
        sub     r8, rcx                 ; reduce length by compare count
        shr     rcx, 3                  ; compute number of quadwords
   repe cmpsq                           ; compare quadwords
        jz      short RlCM20            ; if z, all quadwords compared
        inc     rcx                     ; increment remaining count
        sub     rsi, 8                  ; back up source address
        sub     rdi, 8                  ; back up destination address
        shl     rcx, 3                  ; compute uncompared bytes
RlCM20: add     r8, rcx                 ; compute residual bytes to compare
        jz      short RlCM40            ; if z, all bytes compared equal
        mov     rcx, r8                 ; set remaining bytes to compare
   repe cmpsb                           ; compare bytes
        jz      short RlCM40            ; if z, all byte compared equal
RlCM30: dec     rdi                     ; back up destination address
RlCM40: sub     rdi, r9                 ; compute number of bytes matched
        mov     rax, rdi                ;
        mov     rsi, CmFrame.SavedRsi[rsp] ; restore nonvolatile registers
        mov     rdi, CmFrame.SavedRdi[rsp] ;
        add     rsp, (sizeof CmFrame)   ; deallocate stack frame
        ret                             ; return

;
; Buffer alignment is incompatible or there is less than 8 bytes to compare.
;

RlCM50: test    r8, r8                  ; test if any bytes to compare
        jz      short RlCM60            ; if z, no bytes to compare
        mov     rcx, r8                 ; set number of bytes to compare
   repe cmpsb                           ; compare bytes
        jz      short RlCM60            ; if z, all bytes compared equal
        inc     rcx                     ; increment remaining count
        sub     r8, rcx                 ; compute number of bytes matched
RlCM60: mov     rax, r8                 ;
        mov     rsi, CmFrame.SavedRsi[rsp] ; restore nonvolatile registers
        mov     rdi, CmFrame.SavedRdi[rsp] ;
        add     rsp, (sizeof CmFrame)   ; deallocate stack frame
        ret                             ; return

        NESTED_END RtlCompareMemory, _TEXT$00

        subttl  "Compare Memory 32-bits"
;++
;
; SIZE_T
; RtlCompareMemoryUlong (
;     IN PVOID Source,
;     IN SIZE_T Length,
;     IN ULONG Pattern
;     )
;
; Routine Description:
;
;   This function compares a block of dword aligned memory with a specified
;   pattern 32-bits at a time.
;
;   N.B. The low two bits of the length are assumed to be zero and are
;        ignored.
;
; Arguments:
;
;   Source (rcx) - Supplies a pointer to the block of memory to compare.
;
;   Length (rdx) - Supplies the length, in bytes, of the memory to compare.       compare.
;
;   Pattern (r8d) - Supplies the pattern to be compared against.
;
; Return Value:
;
;   The number of bytes that compared equal is returned as the function
;   value. If all bytes compared equal, then the length of the original
;   block of memory is returned.
;
;--

        NESTED_ENTRY RtlCompareMemoryUlong, _TEXT$00

        alloc_stack 8                   ; allocate stack frame
        save_reg rdi, 0                 ; save nonvolatile register

        END_PROLOGUE

        mov     rdi, rcx                ; set destination address
        shr     rdx, 2                  ; compute number of dwords
        jz      short RlCU10            ; if z, no dwords to compare
        mov     rcx, rdx                ; set length of compare in dwords
        mov     eax, r8d                ; set comparison pattern
   repe scasd                           ; compare memory with pattern
        jz      short RlCU10            ; if z, all dwords compared
        inc     rcx                     ; increment remaining count
        sub     rdx, rcx                ; compute number of bytes matched
RlCU10: lea     rax, [rdx*4]            ; compute successful compare in bytes
        mov     rdi, [rsp]              ; restore nonvolatile register
        add     rsp, 8                  ; deallocate stack frame
        ret                             ; return

        NESTED_END RtlCompareMemoryUlong, _TEXT$00

        subttl  "Copy Memory NonTemporal"
;++
;
; VOID
; RtlCopyMemoryNonTemporal (
;     OUT VOID UNALIGNED *Destination,
;     IN CONST VOID UNALIGNED * Sources,
;     IN SIZE_T Length
;     )
;
; Routine Description:
;
;   This function copies nonoverlapping from one buffer to another using
;   nontemporal moves that do not pollute the cache.
;
; Arguments:
;
;   Destination (rcx) - Supplies a pointer to the destination buffer.
;
;   Sources (rdx) - Supplies a pointer to the source buffer.
;
;   Length (r8) - Supplies the length, in bytes, of the copy operation.
;
; Return Value:
;
;   None.
;
;--

CACHE_BLOCK equ 01000h                  ; nontemporal move block size

        LEAF_ENTRY RtlCopyMemoryNonTemporal, _TEXT$00

        sub     rdx, rcx                ; compute relative address of source
        cmp     r8, 64 + 8              ; check if 64 + 8 bytes to move
        jb      RlNT50                  ; if b, skip non-temporal move

;
; Align the destination to a 8-byte boundary.
;

        test    cl, 07h                 ; check if destination 8-byte aligned
        jz      RlNT30                  ; if z,  already aligned
        mov     rax, [rdx + rcx]        ; read alignment bytes
        add     r8,  rcx                ; adjust remaining bytes
        movnti  [rcx], rax              ; copy alignment bytes (sfence later) 
        add     rcx, 8                  ; compute aligned destination address
        and     rcx, -8                 ; 
        sub     r8, rcx                 ; adjust remaining bytes 
        jmp     RlNT30                  ; jump to move 64-byte blocks

;
; Copy 64-byte blocks.
;

        align   16

RlNT10: prefetchnta [rdx + rax]         ; prefetch 64 bytes
        add     rax, 64                 ; advance destination address
        dec     r9                      ; decrement count
        jnz     RlNT10                  ; if nz, more to prefetch
        sub     rax, rcx                ; reset move count
RlNT20: mov     r9, [rdx + rcx]         ; copy 64-byte blocks
        mov     r10, [rdx + rcx + 8]    ;
        movnti  [rcx], r9               ; 
        movnti  [rcx + 8], r10          ; 
        mov     r9, [rdx + rcx + 16]    ;
        mov     r10, [rdx + rcx + 24]   ;
        movnti  [rcx + 16], r9          ; 
        movnti  [rcx + 24], r10         ; 
        add     rcx, 64                 ; advance destination address
        mov     r9, [rdx + rcx - 32]    ; 
        mov     r10, [rdx + rcx - 24]   ; 
        movnti  [rcx - 32], r9          ; 
        movnti  [rcx - 24], r10         ; 
        mov     r9, [rdx + rcx - 16]    ;
        mov     r10, [rdx + rcx - 8]    ; 
        movnti  [rcx - 16], r9          ; 
        movnti  [rcx - 8], r10          ; 
        sub     rax, 64                 ; subtract number of bytes moved
        jnz     short RlNT20            ; if nz, more 64-byte blocks to move
RlNT30: cmp     r8, 64                  ; check if more than 64 bytes to move
        jb      RlNT40                  ; if b, no more 64 bytes
        mov     rax, rcx                ; save destination address
        mov     r9, CACHE_BLOCK/64      ; number of 64-byte blocks to move
        sub     r8, CACHE_BLOCK         ; reduce bytes to move
        jae     RlNT10                  ; if ae, more CACHE_BLOCK bytes to move 
        add     r8, CACHE_BLOCK         ; adjust to remaining bytes
        mov     r9, r8                  ; compute number of 64-byte blocks left
        shr     r9, 6                   ;
        and     r8, 64 - 1              ; reduce bytes to move
        jmp     RlNT10                  ; continue to move rest 64-byte blocks
RlNT40:                                 ;
   lock or      byte ptr [rsp], 0       ; flush data to memory

;
; Move residual bytes.
;

RlNT50: sub     r8, 8                   ; subtract out 8-byte block
        jae     short RlNT60            ; if ae, more than 8 bytes left
        add     r8, 8                   ; adjust to remaining bytes
        jnz     short RlNT80            ; if nz, more bytes to move
        ret

        align   16

        db      066h, 066h, 090h
        db      066h, 066h, 090h
        db      066h, 066h, 090h

RlNT60: mov     r9, 8                   ; load constant to r9 
RlNT70: mov     rax, [rdx + rcx]        ; move 8-byte block
        add     rcx, r9                 ; advance to next 8-byte block
        mov     [rcx - 8], rax          ;
        sub     r8, r9                  ; subtract out 8-byte block
        jae     short RlNT70            ; if ae, more 8-byte blocks
        add     r8, r9                  ; adjust to remaining bytes
        jnz     short RlNT80            ; if nz, more bytes to move
        ret

        align   16

RlNT80: mov     al, [rdx + rcx]         ; move bytes
        mov     [rcx], al               ;
        inc     rcx                     ; increment source address
        dec     r8                      ; decrement byte count
        jnz     short RlNT80            ; if nz, more bytes to move
        ret                             ; return

        LEAF_END RtlCopyMemoryNonTemporal, _TEXT$00

        subttl  "Fill Memory"
;++
;
; VOID
; RtlFillMemory (
;     IN VOID UNALIGNED *Destination,
;     IN SIZE_T Length,
;     IN UCHAR Fill
;     )
;
; Routine Description:
;
;   This function fills a block of unaligned memory with a specified pattern.
;
; Arguments:
;
;   Destination (rcx) - Supplies a pointer to the memory to fill.
;
;   Length (rdx) - Supplies the length, in bytes, of the memory to fill.
;
;   Fill (r8b) - Supplies the value to fill memory with.
;
; Return Value:
;
;   None.
;
;--

        LEAF_ENTRY RtlFillMemory, _TEXT$00

        xchg    r8, rdx                 ; exchange length and pattern
        jmp     memset                  ; finish in common code

        LEAF_END RtlFillMemory, _TEXT$00

        subttl  "Prefetch Memory NonTemporal"
;++
;
; VOID
; RtlPrefetchMemoryNonTemporal (
;     IN CONST PVOID Source,
;     IN SIZE_T Length
;     )
;
; Routine Description:
;
;   This function prefetches memory at Source, for Length bytes into the
;   closest cache to the processor.
;
; Arguments:
;
;   Source (rcx) - Supplies a pointer to the memory to be prefetched.
;
;   Length (rdx) - Supplies the length, in bytes, of the operation.
;
; Return Value:
;
;   None.
;
;--

        LEAF_ENTRY RtlPrefetchMemoryNonTemporal, _TEXT$00

RlPF10: prefetchnta 0[rcx]              ; prefetch line
        add     rcx, 64                 ; increment address to prefetch
        sub     rdx, 64                 ; subtract number of bytes prefetched
        ja      RlPF10                  ; if above zero, more bytes to move
        ret                             ; return

        LEAF_END RtlPrefetchMemoryNonTemporal, _TEXT$00

        subttl  "Zero Memory"
;++
;
; VOID
; RtlZeroMemory (
;     IN VOID UNALIGNED *Destination,
;     IN SIZE_T Length
;     )
;
; Routine Description:
;
;   This function fills a block of unaligned memory with zero.
;
; Arguments:
;
;   Destination (rcx) - Supplies a pointer to the memory to fill.
;
;   Length (rdx) - Supplies the length, in bytes, of the memory to fill.
;
; Return Value:
;
;   None.
;
;--

        LEAF_ENTRY RtlZeroMemory, _TEXT$00

        mov     r8, rdx                 ; set length
        xor     edx, edx                ; set fill pattern
        jmp     memset                  ; finish in common code

        LEAF_END RtlZeroMemory, _TEXT$00

        end

