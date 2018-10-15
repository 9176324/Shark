# Makefile for dosapp sample application

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

all: dosapp.exe

clean:
    if exist *.obj      del *.obj
    if exist *.exe      del *.exe
    if exist *.map      del *.map
    if exist *.sym      del *.sym

dosapp.exe: dosapp.obj
    link /stack:1024 dosapp;


