#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

subargs=/$(MAKEFLAGS) ntos=$(MAKEDIR) pub=$(MAKEDIR)\..\..\public

!if defined(x86) && !defined(amd64)
subargs  = $(subargs) targ=i386   topobj=$(MAKEDIR)\BUILD\obji386
targ     = i386
lmachine = x86

!else if !defined(x86) &&  defined(amd64)
subargs  = $(subargs) targ=amd64  topobj=$(MAKEDIR)\BUILD\objamd64
targ     = amd64
lmachine = amd64

!else
!ERROR Usage:  nmake (x86=|amd64=)  [clean]
!endif

OBJ = obj$(targ)

modules = rtl config ex ob se mm ke ps io\iomgr io cache lpc dbgk raw fstub fsrtl wmi perf init
buildtargets = $(modules: =.build ).build
cleantargets = $(modules: =.clean ).clean

default: checktoolpath $(buildtargets) kernel

checktoolpath:
	checktoolarch $(lmachine)

kernel:
	cd $(MAKEDIR)\BUILD
		@$(MAKE) $(subargs) module=BUILD
	@cd $(MAKEDIR)

$(buildtargets):
	cd $(MAKEDIR)\$(@R)\BUILD
		@$(MAKE) $(subargs) module=$(@R)
	@cd $(MAKEDIR)

clean: $(cleantargets) clean0

clean0:
	-del $(MAKEDIR)\BUILD\$(OBJ)\** $(MAKEDIR)\BUILD\EXE\** /Q

$(cleantargets):
	cd $(MAKEDIR)\$(@R)\BUILD
		@$(MAKE) $(subargs) module=$(@R) clean
	@cd $(MAKEDIR)

#################### DEBUG
kernelexe:
	cd $(MAKEDIR)\BUILD
		@$(MAKE) $(subargs) module=BUILD kernelexe

