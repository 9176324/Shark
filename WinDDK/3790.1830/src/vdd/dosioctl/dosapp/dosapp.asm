        name    @filename
;*--------------------------------------------------------------------*
;
;       DOSAPP
;
;   This DOS application opens a character mode device driver
;   (DOSDRV00), and issues an IOCTL read to the device. It then
;   displays the DWORD value it read from the IOCTL.
;
;*--------------------------------------------------------------------*

        include dosapp.inc
        include xio.inc
        include isvbop.inc

;*--------------------------------------------------------------------*
;
;       DATA
;
;*--------------------------------------------------------------------*

_DATA   segment word public 'DATA'


DriverName db  "DOSDRV00",0

hDriver dw      ?
errret  dw      ?
DriverInfo dd   ?

infmsg  db      'IOCTL Read returns: '
infmsgx dd      2 dup (?)
        db      cr,lf
infmsgl equ     $-infmsg

errmsg  db      'Open failed, Error='
errmsgx dd      ?
        db      cr,lf
errmsgl equ     $-errmsg

_DATA   ends


_TEXT   segment word public 'CODE'
        assume cs:_TEXT,ds:_DATA,es:_DATA

;*--------------------------------------------------------------------*
;
;       MAIN procedure
;
;*--------------------------------------------------------------------*

@filename proc  far
        mov     ax, _DATA                   ;get addressibility
        mov     ds, ax
        mov     es, ax

        mov     ah, DOSOPENFILE
        mov     al, 00010010b               ;r/w, deny all
        mov     dx, OFFSET DriverName
        int     21h                         ;open device
        jc      opfail
        mov     hDriver, ax                 ;save handle

        mov     ah, DOSIOCTL
        mov     al, 2                       ;IOCTL read
        mov     bx, hDriver
        mov     cx, 4                       ;# of bytes
        mov     dx, offset DriverInfo
        int     21h                         ;issue IOCTL to device

        hxtrans DriverInfo, infmsgx, 4      ;make DWORD ASCII
        Writel  infmsg                      ;output to console

        mov     ah, DOSCLOSEFILE
        mov     bx, hDriver
        int     21h                         ;close the driver
        jmp     exit

opfail:
        mov     errret, ax
        hxtrans errret, errmsgx, 2
        Writel  errmsg                      ;write msg if open failed

exit:
        mov     ax,4c00h
        int     21h

@filename endp



;*--------------------------------------------------------------------*
;
;       hextrans
;
;       This subroutine formats hex values into ASCII
;
;       ENTRY:
;               CX    = size in bytes of operand
;               DS:SI-> input hex value (in memory)
;               ES:DI-> output area
;
;       USES:
;               AX, SI, DI
;*--------------------------------------------------------------------*

hextrans proc    near

        push    bx
        push    cx

        cmp     cx, 0           ; must be higher
        jna     hextexit        ; nope
        add     si, cx          ; point to end of value
        dec     si              ; now pointing at last byte
hext1:
        push    cx
        lodsb                   ; get hex byte
        sub     si, 2           ; make this a decrement

        mov     bx, ax          ; save for next nibble
        mov     cx, 4           ; isolate next four bits
        shr     ax, cl          ; get top nibble

        cvt_nibble              ; make it ascii
        stosb                   ; save it in destination
        mov     ax, bx          ; retrieve original byte
        cvt_nibble              ; make it ascii
        stosb                   ; save it in destination
        pop     cx
        loop    hext1

hextexit:
        pop     cx
        pop     bx
        ret                     ; back to caller

hextrans endp

_TEXT   ends

        end     @filename

