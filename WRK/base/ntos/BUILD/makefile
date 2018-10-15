#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

kernel		  = wrk$(machine)

asobjs=
ccobjs=
ccarchobjs=

default: kernelexp kernellib kernelexe

nodefault=1
!include $(ntos)\BUILD\makefile.build

linklibpath = -LIBPATH:$(topobj) -LIBPATH:PREBUILT\$(targ)
ntoswrklib  = ntoswrk.lib
ntosarchlib = ntosarch.lib
bootlibs = bootvid.lib sdbapint.lib kdcom.lib 

fullkernel	  = EXE\$(kernel)

# kernel link definitions
LINKFLAGS = -IGNORE:4087,4001,4010,4037,4039,4065,4070,4078,4087,4089,4221,4198 -WX -NODEFAULTLIB -machine:$(machine) $(linklibpath)
LINK 	  = link.exe -nologo
LINKEDIT  = link.exe -edit -nologo

LIB  = $(LIB) $(linklibpath)

!if "$(targ)" == "i386"
archlinkopts = -safeseh -functionpadmin:5 -debugtype:cv,fixup       -STACK:0x40000,0x2000 -align:0x1000 
hotpatch 	 = -stub:PREBUILT\i386\stub512.com
entrypoint	 = KiSystemStartup@4 
!else
archlinkopts = -functionpadmin:6 -debugtype:cv,fixup,pdata -STACK:0x80000,0x2000 
hotpatch 	 = PREBUILT\amd64\hotpatch.obj
LINKFLAGS    = -IGNORE:4108,4088,4218,4218,4235 $(LINKFLAGS)
LIBFLAGS     = -IGNORE:4108,4088,4218,4218,4235 $(LIBFLAGS)
entrypoint	 = KiSystemStartup
!endif

ntosmerge    = -merge:PAGECONST=PAGE -merge:INITCONST=INIT -merge:INITDATA=INIT -merge:PAGELKCONST=PAGELK \
 -merge:PAGEVRFY_CONST=PAGEVRFY -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -merge:.rdata=.text 

ntosversion  = -release -version:5.2 -osversion:5.2 -subsystem:native,5.02 

ntoslinkopts = $(ntosversion) $(ntosmerge) -SECTION:INIT,d -OPT:REF -OPT:ICF -INCREMENTAL:NO \
 -FULLBUILD -debug $(archlinkopts) -opt:nowin98 -pdbcompress -driver

kernelexp:
	copy ..\init\ntoskrnl.src+..\init\$(targ)def.src $(OBJ)\$(kernel).pp
	$(CC) $(CFLAGS0) -EP $(OBJ)\$(kernel).pp > $(fullkernel).def
	-del $(OBJ)\$(kernel).pp
	$(LIB) $(LIBFLAGS) -IGNORE:4001 $(OBJ)\*.lib $(ntoswrklib) -def:$(fullkernel).def -out:$(fullkernel).lib

kernellib:
	$(LIB) $(fullkernel).lib $(ntosarchlib) -out:$(fullkernel).lib

kernelexe:
	$(LINK) $(LINKFLAGS) $(ntoslinkopts) -out:$(fullkernel).exe -map:$(fullkernel).map -pdb:$(fullkernel).pdb -entry:$(entrypoint) \
		$(hotpatch) PREBUILT\$(targ)\ntoskrnl.res $(OBJ)\ntkrnlmp.obj $(OBJ)\*.lib $(ntoswrklib) hal.lib $(fullkernel).exp $(bootlibs)
	$(LINKEDIT) -section:.rsrc,!d $(fullkernel).exe 

