#
#
# Copyright (c) 2018 by blindtiger. All rights reserved.
#
# The contents of this file are subject to the Mozilla Public License Version
# 2.0 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original e is blindtiger.
#
#

LABS=..\..\..

!if "$(PLATFORM)" == "x86"
ARCTARG = I386
PATH = C:\Windows;C:\Windows\System32;$(LABS)\WRK\tools\x86
!else if "$(PLATFORM)" == "x64"
ARCTARG = AMD64
PATH = C:\Windows;C:\Windows\System32;$(LABS)\WRK\tools\amd64
!else
!ERROR Usage: nmake (PLATFORM=x86|x64) (PROJ=ProjectName) (SLND=SolutionDir) [build|clean|rebuild]
!endif

BIND = $(SLND)Build\Bins\$(ARCTARG)

PROJTARG = $(BIND)\$(PROJ)
PROJD = $(SLND)Projects\$(PROJ)

BUILD: $(PROJTARG).exe

OBJD = $(SLND)Build\Objs\$(PROJ)\$(ARCTARG)

!if "$(ARCTARG)" == "I386"
PROJENTRY = Startup
!else
PROJENTRY = Startup
!endif

ASOBJS = \
    $(OBJD)\$(ARCTARG).obj

CCARCHOBJS = \
    

CCOBJS = \
    $(OBJD)\$(PROJ).obj \
    $(OBJD)\Print.obj \
    $(OBJD)\Sysload.obj
    
RCOBJS = $(OBJD)\$(PROJ).res

PROJOBJS = $(ASOBJS) $(CCARCHOBJS) $(CCOBJS) $(RCOBJS)

PROJSRCS = \
    $(ARCTARG)\$(ARCTARG).asm \
    $(PROJ).c \
    Print.c \
    Sysload.c

!if "$(ARCTARG)" == "I386"
TARGDEFS = /D_X86_=1 /Di386=1 /DSTD_CALL /DFPO=0
TARGAOPTS = /safeseh /coff /Zm
TARGCOPTS = /Gz /Gm- /GX- /G7 /Ze /Gi- /QIfdiv- /Od /Oy-
TARGLOPTS = 
HOTPATCH = /stub:$(LABS)\WRK\base\ntos\BUILD\PREBUILT\$(ARCTARG)\STUB512.com
ARCH = X86
ARCHML = ml
!else
TARGDEFS = /D_WIN64 /D_AMD64_ /DAMD64
TARGAOPTS = 
TARGCOPTS = /Wp64 /Od /EHs-c- /Gs12288
TARGLOPTS = /IGNORE:4108,4088,4218,4218,4235
HOTPATCH = $(LABS)\WRK\base\ntos\BUILD\PREBUILT\$(ARCTARG)\HOTPATCH.obj
ARCH = AMD64
ARCHML = ml64
!endif

PROJLOPTS = /SUBSYSTEM:CONSOLE /ENTRY:$(PROJENTRY) /DEF:$(PROJ).def

INCS = \
    /I$(LABS)\WRK\public\ddk\inc \
    /I$(LABS)\WRK\public\halkit\inc \
    /I$(LABS)\WRK\public\internal\base\inc \
    /I$(LABS)\WRK\public\internal\ds\inc \
    /I$(LABS)\WRK\public\internal\sdktools\inc \
    /I$(LABS)\WRK\public\sdk\inc \
    /I$(LABS)\WRK\public\sdk\inc\crt \
    /I$(LABS)\WRK\base\inc \
    /I$(LABS)\WRK\base\ntos\inc \
    /I$(LABS)\WinDDK\3790.1830\inc\wnet \
    /I$(LABS)\WinDDK\3790.1830\inc\crt\gl \
    /I$(SLND)\Include \
    /I$(PROJD)

DEFS = /DUNICODE /D_UNICODE $(TARGDEFS) -D_$(ARCH)_ -D$(ARCH) -DDBG=1

COPTS = /Z7 /Zl /Zp8 /Gy /cbstring /W3 /WX /GR- /GF /GS $(TARGCOPTS)
COMPILERWARNINGS = /FI$(LABS)\WRK\base\ntos\BUILD\WARNING.h /FI$(SLND)\Include\WARNING.h

AOPTS = /Cx /Zi /Zd $(TARGAOPTS)
AS = $(ARCHML).exe /nologo
AFLAGS = $(AOPTS) $(INCS) /Fo$(OBJD)\ $(DEFS) $(SPECIALAFLAGS)

CC = cl.exe /nologo
CFLAGS = $(COPTS) $(INCS) /Fo$(OBJD)\ $(DEFS) $(SPECIALCFLAGS) $(COMPILERWARNINGS)

RC = rc.exe
RFLAGS = $(INCS) /fo$(RCOBJS)


LINKLIBPATH = \
    /LIBPATH:$(LABS)\WinDDK\3790.1830\lib\wnet\$(ARCTARG) \
    /LIBPATH:$(LABS)\WinDDK\3790.1830\lib\crt\$(ARCTARG) \
    /LIBPATH:$(SLND)\Lib\$(ARCTARG)

LINKIGNORE = /IGNORE:4087,4001,4010,4037,4039,4065,4070,4078,4087,4089,4221,4198
BUILDLIBS = ntdll.lib bufferoverflow.lib \
!if "$(ARCTARG)" == "I386"
    \
!else if "$(ARCTARG)" == "AMD64"
    gshandler.obj gshandlerseh.obj \
!endif
    kernel32.lib msvcrt.lib shell32.lib

LINKFLAGS = $(LINKIGNORE) /WX /NODEFAULTLIB /machine:$(ARCH) /debug /debugtype:cv,fixup
LINK = link.exe /nologo

LINKPROJNAMES = /out:$(PROJTARG).exe /map:$(PROJTARG).map /pdb:$(PROJTARG).pdb

$(PROJTARG).exe: $(PROJOBJS)
    @$(LINK) $(LINKFLAGS) $(PROJLOPTS) $(LINKPROJNAMES) $** $(HOTPATCH) $(LINKLIBPATH) $(BUILDLIBS)

# assembly files
{$(ARCTARG)\}.asm{$(OBJD)\}.obj::
    @$(AS) $(AFLAGS) /c $<

# arch-specific C files
{$(ARCTARG)\}.c{$(OBJD)\}.obj::
    @$(CC) $(CFLAGS) /c $<

# C files
.c{$(OBJD)\}.obj::
    @$(CC) $(CFLAGS) /c $<

# RC files
.rc{$(OBJD)\}.res::
    @$(RC) $(RFLAGS) $<

# CLEAN pseudo targets

CLEAN:
    @if exist $(OBJD)\*.res del /F /Q $(OBJD)\*.res
    @if exist $(OBJD)\*.obj del /F /Q $(OBJD)\*.obj
    @if exist $(PROJTARG).exe del /F /Q $(PROJTARG).exe
    @if exist $(PROJTARG).sys del /F /Q $(PROJTARG).sys
    @if exist $(PROJTARG).dll del /F /Q $(PROJTARG).dll
    @if exist $(PROJTARG).lib del /F /Q $(PROJTARG).lib
    @if exist $(PROJTARG).exe del /F /Q $(PROJTARG).exe
    @if exist $(PROJTARG).exp del /F /Q $(PROJTARG).exp
    @if exist $(PROJTARG).map del /F /Q $(PROJTARG).map
    @if exist $(PROJTARG).pdb del /F /Q $(PROJTARG).pdb

REBUILD: CLEAN BUILD
