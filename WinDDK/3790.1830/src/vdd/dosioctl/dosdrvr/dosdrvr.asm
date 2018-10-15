        name    dosdrvr
        title   'DOSDRVR - Stub driver for Application based intercept under NT'
;******************************************************************************
;
;       DOSDRVR
;
;       This DOS character device driver demonstrates basic communication
;       with a NT VDD. It handles DOS OPEN, CLOSE and IOCTL Read requests.
;
;   Operation:
;
;       INIT - Issues RegisterModule() call to get VDD handle.
;
;       OPEN, CLOSE, IOCTL Read - The VDD exposes a private interface
;       that correspond to these DOS calls. The driver basically passes
;       these calls from a DOS application on to the VDD using the
;       DispatchCall() function.
;
;******************************************************************************


_TEXT   segment byte public 'CODE'

        assume  cs:_TEXT,ds:_TEXT,es:NOTHING

        org     0

        include isvbop.inc

MaxCmd  equ     24                      ; Maximum DOS command value

; VDD Command codes

VDDOPEN equ     1
VDDCLOSE equ    2
VDDINFO equ     3

Header:                                 ; Device Header
        DD      -1
        DW      0c840h
        DW      DrvStrat
        DW      DrvIntr
        DB      'DOSDRV00'

RHPtr   DD      ?                       ; Pointer to Request Header


DllName DB      "IOCTLVDD.DLL",0
InitFunc  DB    "VDDRegisterInit",0
DispFunc  DB    "VDDDispatch",0

hVDD    DW      ?

Dispatch:                               ; Interrupt routine command code
        DW      Init
        DW      DrvNOP               ; MediaChk
        DW      DrvNOP               ; BuildBPB
        DW      IoctlRd
        DW      DrvNOP               ; Read
        DW      DrvNOP               ; NdRead
        DW      DrvNOP               ; InpStat
        DW      DrvNOP               ; InpFlush
        DW      DrvNOP               ; Write
        DW      DrvNOP               ; WriteVfy
        DW      DrvNOP               ; OutStat
        DW      DrvNOP               ; OutFlush
        DW      DrvNOP               ; IoctlWt
        DW      DevOpen
        DW      DevClose
        DW      DrvNOP               ; RemMedia
        DW      DrvNOP               ; OutBusy
        DW      Error
        DW      Error
        DW      DrvNOP               ; GenIOCTL
        DW      Error
        DW      Error
        DW      Error
        DW      DrvNOP               ; GetLogDev
        DW      DrvNOP               ; SetLogDev

;******************************************************************************
;
;       DrvStrat
;       DrvIntr
;
;       The following routines are standard, required DOS driver routines.
;       The DrvIntr routine uses the "Dispatch" table to call the appropriate
;       subroutine, based on the driver request.
;
;******************************************************************************
DrvStrat proc   far              ; Strategy Routine

        mov     word ptr cs:[RhPtr],bx
        mov     word ptr cs:[RhPtr+2],es
        ret

DrvStrat endp

DrvIntr proc    far                     ; INterrupt routine

        push    ax                      ; Save registers
        push    bx
        push    cx
        push    dx
        push    ds
        push    es
        push    di
        push    si
        push    bp

        push    cs
        pop     ds                      ; DS = CS

        les     di,[RHPtr]              ; ES:DI = request header

        mov     bl,es:[di+2]
        xor     bh,bh                   ; BX = command code
        cmp     bx,MaxCmd
        jle     FIntr1

        call    Error                   ; Unknown command
        jmp     FIntr2

FIntr1:
        shl     bx,1
        call    word ptr [bx+Dispatch]  ; call command routine
        les     di,[RhPtr]              ; ES:DI = request header

FIntr2:
        or      ax,0100h                ; Set Done bit in the status
        mov     es:[di+3],ax            ; Store the status

        pop     bp                      ; restore registers
        pop     si
        pop     di
        pop     es
        pop     ds
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

DrvIntr endp

DrvNOP  proc    near
        xor     ax,ax
        ret
DrvNOP  endp

Error   proc    near
        mov     ax,8003h                           ; Bad Command Code
        ret
Error   endp


;******************************************************************************
;
;       IoctlRd
;
;       This routine is entered when a DOS application issues an IOCTL READ
;       to this driver. The target buffer address for the IOCTL is passed
;       on to the VDD.
;
;******************************************************************************
IoctlRd proc    near

        push    es
        push    di

        mov     ax, word ptr cs:[hVDD]       ; VDD handle
        mov     dx, VDDINFO
        mov     cx, es:[di+18]               ; size of buffer
        les     di, es:[di+14]               ; pointer to target buffer
        DispatchCall

        pop     di
        pop     es

        jnc     @f                           ; jif Success

        call    Error                        ; Operation Failed
        ret

@@:
        mov     es:[di+18], cx               ; # of bytes read
        xor     ax,ax
        ret

IoctlRd endp

;******************************************************************************
;
;       DevOpen
;
;       This routine is entered when a DOS application does a DOS OPEN to
;       this driver. The open request is passed along to the VDD.
;
;******************************************************************************
DevOpen proc    near

        mov     ax, word ptr cs:[hVDD]       ; VDD handle
        mov     dx, VDDOPEN                  ; Open file
        DispatchCall
        jnc     @f                           ; jif Success
        call    Error                        ; Operation Failed
        ret
@@:
        xor     ax,ax
        ret

DevOpen endp

;******************************************************************************
;
;       DevClose
;
;       This routine is entered when a DOS application issues a CLOSE on the
;       handle for the driver. A close request is issued to the VDD.
;
;******************************************************************************
DevClose proc   near

        mov     ax, word ptr cs:[hVDD]       ; VDD handle
        mov     dx, VDDCLOSE                 ; Close file

        DispatchCall
        jnc     @f                           ; jif Success
        call    Error                        ; Operation Failed
        ret
@@:
        xor     ax,ax
        ret

DevClose endp


;******************************************************************************
;
;       INIT
;
;       This routine is entered when the VDM is booting. The code in this
;       routine is only present during initialization. Here, a RegisterModule
;       is issued to get a handle to the VDD. This handle is then used by
;       the DispatchCall's in the other driver routines.
;
;       If RegisterModule returns with error, then the driver indicates to
;       DOS that an error occurred.
;
;******************************************************************************
Init    proc    near
        push    es
        push    di                                   ; Save Request Header add

        push    ds
        pop     es

        ; Load ioctlvdd.dll
        mov     si, offset DllName                   ; ds:si = dll name
        mov     di, offset InitFunc                  ; es:di = init routine
        mov     bx, offset DispFunc                  ; ds:bx = dispatch routine

        RegisterModule
        jnc     saveHVDD                             ; NC -> Success

        call    Error                                ; Indicate failure

        pop     di
        pop     es
        mov     byte ptr es:[di+13],0                ; unit supported 0
        mov     word ptr es:[di+14],offset Header    ; Unload this device
        mov     word ptr es:[di+16],cs
        mov     si, offset Header
        and     [si+4],8FFFh                         ; clear bit 15 for failure
        ret

saveHVDD:
        mov     [hVDD],ax

        pop     di
        pop     es
        mov     word ptr es:[di+14],offset Init      ; Free Memory address
        mov     word ptr es:[di+16],cs
        xor     ax,ax                                ; return success
        ret
Init    endp

_TEXT   ends

        end

