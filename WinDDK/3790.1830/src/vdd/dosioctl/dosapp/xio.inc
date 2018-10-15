;----------------------------- E Q U A T E S -----------------------------

cr      equ     0dh
lf      equ     0ah


;----------------------------- M A C R O S ------------------------------
Writel  MACRO  addr
        push   ax
        push   bx
        push   cx
        push   dx

        mov    dx,offset &addr         ;Print
        mov    cx,&addr&l
        mov    bx,1                    ;stdout
        mov    ah,40h                  ;write
        int    21h

        pop    dx
        pop    cx
        pop    bx
        pop    ax
        ENDM

hxtrans MACRO  source, dest, length
        mov    cx, length
        mov    si, offset source
        mov    di, offset dest
        call   hextrans
        ENDM

cvt_nibble MACRO
        and     ax, 0fh
        add     ax, '0'         ; convert to ASCII
        cmp     ax, '9'         ; is it 0-9?
        jbe     @f              ; yes, jump
        add     ax, 'A'-'9'-1   ; make it 'A-F'
@@:
        ENDM

