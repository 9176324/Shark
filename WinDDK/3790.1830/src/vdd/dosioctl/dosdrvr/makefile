# Makefile for dosdrvr stub device driver

!IFNDEF BUILDMSG
BUILDMSG=
!ENDIF

.SUFFIXES:
.SUFFIXES: .c .obj .lst .exe .exc .exs .com .sal .cod .sil .inc .skl .cla .cl1 .ctl .asm .idx .msg

MAKE    =nmake
asm     =masm
awarn   =-W1
aflags  =-Mx -t $(awarn) $(extasw)
ainc    =-I. -I$(BASEDIR)\inc


.asm.obj:
        $(asm) $(ainc) $(aflags) $*.asm;

.asm.lst:
        $(asm) -l $(ainc) $(aflags) $*.asm;

all: dosdrvr.sys

clean:
    if exist *.obj      del *.obj
    if exist *.exe      del *.exe
    if exist *.map      del *.map
    if exist *.sym      del *.sym
    if exist *.sys      del *.sys

dosdrvr.sys: dosdrvr.obj
    link dosdrvr;
    exe2bin dosdrvr.exe dosdrvr.sys


