        title  "Interlocked Support"
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
;   intrlock.asm
;
; Abstract:
;
;   This module implements functions to support interlocked operations.
;
;--

include ksamd64.inc

        subttl  "ExInterlockedAddLargeInteger"
;++
;
; LARGE_INTEGER
; ExInterlockedAddLargeInteger (
;     __inout PLARGE_INTEGER Addend,
;     __in LARGE_INTEGER Increment,
;     __inout PKSPIN_LOCK Lock
;     )
;
; Routine Description:
;
;   This function performs an interlocked add of an increment value to an
;   addend variable of type unsigned large integer. The initial value of
;   the addend variable is returned as the function value.
;
;   N.B. The specification of this function requires that the given lock
;        must be used to synchronize the update even though on AMD64 the
;        operation can actually be done atomically without using the lock.
;
; Arguments:
;
;   Addend (rcx) - Supplies a pointer to a variable whose value is to be
;       adjusted by the increment value.
;
;   Increment (rdx) - Supplies the increment value to be added to the
;       addend variable.
;
;   Lock (r8) - Supplies a pointer to a spin lock to be used to synchronize
;       access to the addend variable.
;
; Return Value:
;
;   The initial value of the addend variable is returned.
;
;--

        NESTED_ENTRY ExInterlockedAddLargeInteger, _TEXT$00

        push_eflags                     ; push processor flags

        END_PROLOGUE

        mov     rax, rdx                ; copy increment value

        AcquireSpinLockDisable [r8]     ; acquire spin lock, interrupts disabled

        xadd    [rcx], rax              ; compute sum of addend and increment 

        ReleaseSpinLockEnable [r8]      ; release spin lock

        add     rsp, 8                  ; deallocate stack frame
        ret                             ; return

        NESTED_END ExInterlockedAddLargeInteger, _TEXT$00

        subttl  "Interlocked Add Unsigned Long"
;++
;
; ULONG
; ExInterlockedAddUlong (
;     __inout PULONG Addend,
;     __in ULONG Increment,
;     __inout PKSPIN_LOCK Lock
;     )
;
; Routine Description:
;
;   This function performs an interlocked add of an increment value to an
;   addend variable of type unsigned long. The initial value of the addend
;   variable is returned as the function value.
;
;   N.B. The specification of this function requires that the given lock
;        must be used to synchronize the update even though on AMD64 the
;        operation can actually be done atomically without using the lock.
;
; Arguments:
;
;   Addend (rcx) - Supplies a pointer to a variable whose value is to be
;       adjusted by the increment value.
;
;   Increment (edx) - Supplies the increment value to be added to the
;       addend variable.
;
;   Lock (r8) - Supplies a pointer to a spin lock to be used to synchronize
;       access to the addend variable.
;
; Return Value:
;
;   The initial value of the addend variable.
;
;--

        NESTED_ENTRY ExInterlockedAddUlong, _TEXT$00

        push_eflags                     ; push processor flags

        END_PROLOGUE

        mov     eax, edx                ; copy increment value

        AcquireSpinLockDisable [r8]     ; acquire spin lock, ints disabled

        xadd    [rcx], eax              ; compute sum of addend and increment

        ReleaseSpinLockEnable [r8]      ; release spin lock

        add     rsp, 8                  ; deallocate stack frame
        ret                             ; return

        NESTED_END ExInterlockedAddUlong, _TEXT$00

        subttl  "Interlocked Insert Head List"
;++
;
; PLIST_ENTRY
; ExInterlockedInsertHeadList (
;     __inout PLIST_ENTRY ListHead,
;     __inout PLIST_ENTRY ListEntry,
;     __inout PKSPIN_LOCK Lock
;     )
;
; Routine Description:
;
;   This function inserts an entry at the head of a doubly linked list
;   so that access to the list is synchronized in a multiprocessor system.
;
; Arguments:
;
;   ListHead (rcx) - Supplies a pointer to the head of the doubly linked
;       list into which an entry is to be inserted.
;
;   ListEntry (rdx) - Supplies a pointer to the entry to be inserted at the
;       head of the list.
;
;   Lock (r8) - Supplies a pointer to a spin lock to be used to synchronize
;       access to the list.
;
; Return Value:
;
;   Pointer to entry that was at the head of the list or NULL if the list
;   was empty.
;
;--

        NESTED_ENTRY ExInterlockedInsertHeadList, _TEXT$00

        push_eflags                     ; push processor flags

        END_PROLOGUE

        prefetchw [rcx]                 ; prefetch entry for write

        AcquireSpinLockDisable [r8]     ; acquire spin lock, ints disabled

        mov     rax, LsFlink[rcx]       ; get address of first entry
        mov     LsFlink[rdx], rax       ; set next link in entry
        mov     LsBlink[rdx], rcx       ; set back link in entry
        mov     LsFlink[rcx], rdx       ; set next link in head
        mov     LsBlink[rax], rdx       ; set back link in next

        ReleaseSpinLockEnable [r8]      ; release spin lock

        xor     rcx, rax                ; check if list was empty
        cmovz   rax, rcx                ; if z, list was empty
        add     rsp, 8                  ; deallocate stack frame
        ret                             ; return

        NESTED_END ExInterlockedInsertHeadList, _TEXT$00

        subttl  "Interlocked Insert Tail List"
;++
;
; PLIST_ENTRY
; ExInterlockedInsertTailList (
;     __inout PLIST_ENTRY ListHead,
;     __inout PLIST_ENTRY ListEntry,
;     __inout PKSPIN_LOCK Lock
;     )
;
; Routine Description:
;
;   This function inserts an entry at the tail of a doubly linked list
;   so that access to the list is synchronized in a multiprocessor system.
;
; Arguments:
;
;   ListHead (rcx) - Supplies a pointer to the head of the doubly linked
;       list into which an entry is to be inserted.
;
;   ListEntry (rdx) - Supplies a pointer to the entry to be inserted at the
;       tail of the list.
;
;   Lock (r8) - Supplies a pointer to a spin lock to be used to synchronize
;       access to the list.
;
; Return Value:
;
;   Pointer to entry that was at the tail of the list or NULL if the list
;   was empty.
;
;--

        NESTED_ENTRY ExInterlockedInsertTailList, _TEXT$00

        push_eflags                     ; push processor flags

        END_PROLOGUE

        prefetchw [rcx]                 ; prefetch entry for write

        AcquireSpinLockDisable [r8]     ; acquire spin lock, ints disabled

        mov     rax, LsBlink[rcx]       ; get address of last entry
        mov     LsFlink[rdx], rcx       ; set next link in entry
        mov     LsBlink[rdx], rax       ; set back link in entry
        mov     LsBlink[rcx], rdx       ; set back link in head
        mov     LsFlink[rax], rdx       ; set next link in last

        ReleaseSpinLockEnable [r8]      ; release spin lock

        xor     rcx, rax                ; check if list was empty
        cmovz   rax, rcx                ; if z, list was empty
        add     rsp, 8                  ; deallocate stack frame
        ret                             ; return

        NESTED_END ExInterlockedInsertTailList, _TEXT$00

        subttl  "Interlocked Remove Head List"
;++
;
; PLIST_ENTRY
; ExInterlockedRemoveHeadList (
;     __inout PLIST_ENTRY ListHead,
;     __inout PKSPIN_LOCK Lock
;     )
;
; Routine Description:
;
;   This function removes an entry from the head of a doubly linked list
;   so that access to the list is synchronized in a multiprocessor system.
;   If there are no entries in the list, then a value of NULL is returned.
;   Otherwise, the address of the entry that is removed is returned as the
;   function value.
;
; Arguments:
;
;   ListHead (rcx) - Supplies a pointer to the head of the doubly linked
;       list from which an entry is to be removed.
;
;   Lock (rdx) - Supplies a pointer to a spin lock to be used to synchronize
;       access to the list.
;
; Return Value:
;
;   The address of the entry removed from the list, or NULL if the list is
;   empty.
;
;--

        NESTED_ENTRY ExInterlockedRemoveHeadList, _TEXT$00

        push_eflags                     ; push processor flags

        END_PROLOGUE

        AcquireSpinLockDisable [rdx]    ; acquire spin lock

        mov     rax, LsFlink[rcx]       ; get address of first entry
        cmp     rax, rcx                ; check if list is empty
        je      short EiRH10            ; if e, list is empty
        mov     r8, LsFlink[rax]        ; get address of next entry
        mov     LsFlink[rcx], r8        ; set address of first entry
        mov     LsBlink[r8], rcx        ; set back in next entry

EiRH10: ReleaseSpinLockEnable [rdx]     ; release spin lock

        xor     rcx, rax                ; check if list was empty
        cmovz   rax, rcx                ; if z, list was empty
        add     rsp, 8                  ; deallocate stack frame
        ret                             ; return

        NESTED_END ExInterlockedRemoveHeadList, _TEXT$00

        subttl  "Interlocked Pop Entry List"
;++
;
; PSINGLE_LIST_ENTRY
; ExInterlockedPopEntryList (
;     __inout PSINGLE_LIST_ENTRY ListHead,
;     __inout PKSPIN_LOCK Lock
;     )
;
; Routine Description:
;
;   This function removes an entry from the front of a singly linked list
;   so that access to the list is synchronized in a multiprocessor system.
;   If there are no entries in the list, then a value of NULL is returned.
;   Otherwise, the address of the entry that is removed is returned as the
;   function value.
;
; Arguments:
;
;   ListHead (rcx) - Supplies a pointer to the head of the singly linked
;       list from which an entry is to be removed.
;
;   Lock (rdx) - Supplies a pointer to a spin lock to be used to synchronize
;       access to the list.
;
; Return Value:
;
;   The address of the entry removed from the list, or NULL if the list is
;   empty.
;
;--

        NESTED_ENTRY ExInterlockedPopEntryList, _TEXT$00

        push_eflags                     ; push processor flags

        END_PROLOGUE

        AcquireSpinLockDisable [rdx]    ; acquire spin lock, ints disabled

        mov     rax, [rcx]              ; get address of first entry
        test    rax, rax                ; check if list is empty
        jz      short EiPE10            ; if z, list is empty
        mov     r8, [rax]               ; get address of next entry
        mov     [rcx], r8               ; set address of first entry

EiPE10: ReleaseSpinLockEnable [rdx]     ; release spin lock

        add     rsp, 8                  ; deallocate stack frame
        ret                             ; return

        NESTED_END ExInterlockedPopEntryList, _TEXT$00

        subttl  "Interlocked Push Entry List"
;++
;
; PSINGLE_LIST_ENTRY
; ExInterlockedPushEntryList (
;     __inout PSINGLE_LIST_ENTRY ListHead,
;     __inout PSINGLE_LIST_ENTRY ListEntry,
;     __inout PKSPIN_LOCK Lock
;     )
;
; Routine Description:
;
;   This function inserts an entry at the head of a singly linked list
;   so that access to the list is synchronized in a multiprocessor system.
;
; Arguments:
;
;   ListHead (rcx) - Supplies a pointer to the head of the singly linked
;       list into which an entry is to be inserted.
;
;   ListEntry (rdx) - Supplies a pointer to the entry to be inserted at the
;       head of the list.
;
;   Lock (r8) - Supplies a pointer to a spin lock to be used to synchronize
;       access to the list.
;
; Return Value:
;
;   Previous contents of ListHead. NULL implies list went from empty to not
;   empty.
;
;--

        NESTED_ENTRY ExInterlockedPushEntryList, _TEXT$00

        push_eflags                     ; push processor flags

        END_PROLOGUE

        prefetchw [rcx]                 ; prefetch entry for write

        AcquireSpinLockDisable [r8]     ; acquire spin lock

        mov     rax, [rcx]              ; get address of first entry
        mov     [rdx], rax              ; set address of next entry
        mov     [rcx], rdx              ; set address of first entry

        ReleaseSpinLockEnable [r8]      ; release spin lock

        add     rsp, 8                  ; deallocate stack frame
        ret                             ;

        NESTED_END ExInterlockedPushEntryList, _TEXT$00

        end

