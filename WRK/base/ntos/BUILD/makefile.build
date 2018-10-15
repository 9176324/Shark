#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

!if !defined(ntos) || !defined(pub) || !defined(module) || !defined(topobj) || !defined(targ) || ("$(targ)" != "i386" && "$(targ)" != "amd64")
!ERROR Usage: nmake ntos=ntosdir pub=pubdir module=ntossubdir targ=[i386|amd64]
!endif

!if "$(targ)" == "i386"
targdefs  = -D_X86_=1 -Di386=1 -DSTD_CALL -DFPO=0
targaopts = -safeseh -coff -Zm
targcopts = -Gm- -Gz -GX- -G6 -Ze -Gi- -QIfdiv- -Z7 -Oxs -Oy-
targlopts =	
machine   = x86
archml    = ml
!else
targdefs  = -D_WIN64 -D_AMD64_ -DAMD64
targaopts = 
targcopts = -Wp64 -Oxt -EHs-c- /Oxt -Gs12288 -GL- -MT -U_MT
targlopts =	-IGNORE:4108,4088,4218,4218,4235
machine   = amd64
archml    = ml64
!endif

tempdir   = $(topobj)\temp
ipub 	  = $(pub)\internal
baseinc   = $(ntos)\..\inc

incs 	  = -I..\$(targ) -I. -I$(ntos)\$(module) -I$(ntos)\inc -I$(pub)\ddk\inc -I$(ipub)\ds\inc -I$(ipub)\sdktools\inc \
			-I$(baseinc) -I$(ipub)\base\inc -I$(pub)\sdk\inc -I$(pub)\sdk\inc\crt -I$(pub)\halkit\inc

defs 	  = $(targdefs) -DCONDITION_HANDLING=1 -DNT_INST=0 -DWIN32=100 -D_NT1X_=100 -DWINNT=1 \
			-D_WIN32_WINNT=0x0502 -DWINVER=0x0502 -D_WIN32_IE=0x0603 -DWIN32_LEAN_AND_MEAN=1 -DDBG=0 -DDEVL=1 \
			-D__BUILDMACHINE__=WRK1.2(university) -DNDEBUG  -D_NTSYSTEM_ -DNT_SMT -DNTOS_KERNEL_RUNTIME=1

aopts	  = -Cx -Zi $(targaopts)
copts 	  = -Zl -Zp8 -Gy -cbstring -W3 -WX -GR- -GF -GS $(targcopts)
compilerwarnings = -FI$(ntos)\BUILD\WARNING.h
 
AS		  = $(archml).exe -nologo
AFLAGS	  = $(aopts) $(incs) -Foobj$(targ)\ $(defs) $(specialaflags)

CC		  = cl.exe -nologo
CFLAGS0	  = $(copts) $(incs) -Foobj$(targ)\ $(defs) $(specialcflags)
CFLAGS	  = $(CFLAGS0) $(compilerwarnings)

LIBFLAGS  = $(targlopts) -IGNORE:4010,4037,4039,4065,4070,4078,4087,4089,4221,4198 -WX -nodefaultlib -machine:$(machine)
LIB		  = lib.exe -nologo

OBJ = obj$(targ)

!ifndef nodefault
default: build $(localtargets)
!endif

# assembly files
{..\$(targ)\}.asm{$(OBJ)\}.obj::
	$(AS) $(AFLAGS) -c $<

# arch-specific C files
{..\$(targ)\}.c{$(OBJ)\}.obj::
	$(CC) $(CFLAGS) -c $<

# C files
{..\}.c{$(OBJ)\}.obj::
	$(CC) $(CFLAGS) -c $<

# library
$(topobj)\$(library).lib: $(asobjs) $(ccarchobjs) $(ccobjs)
	@echo linking $(library).lib
	$(LIB) $(LIBFLAGS) -out:$@ $**

# pseudo targets
build: $(topobj)\$(library).lib

clean: clean0 $(localclean)

clean0:
	-del $(asobjs) $(ccarchobjs) $(ccobjs) $(extraobjs)

