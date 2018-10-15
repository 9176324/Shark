#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

library		  = $(module)
specialcflags = -I..\..\ke

!if "$(targ)" == "i386"
asobjs= \
	$(OBJ)\intrlock.obj	 	\
	$(OBJ)\intrlfst.obj	 	\
	$(OBJ)\splocks.obj
ccarchobjs=
!else
asobjs= \
	$(OBJ)\hifreqlk.obj	 	\
	$(OBJ)\hrderror.obj	 	\
	$(OBJ)\intrlock.obj
ccarchobjs= \
	$(OBJ)\wowinfo.obj
!endif
			

ccobjs= \
	$(OBJ)\callback.obj		\
	$(OBJ)\callperf.obj		\
	$(OBJ)\delay.obj		\
	$(OBJ)\event.obj		\
	$(OBJ)\eventpr.obj		\
	$(OBJ)\exatom.obj		\
	$(OBJ)\exdata.obj		\
	$(OBJ)\exinit.obj		\
	$(OBJ)\fmutexc.obj		\
	$(OBJ)\handle.obj		\
	$(OBJ)\harderr.obj		\
	$(OBJ)\lookasid.obj		\
	$(OBJ)\luid.obj			\
	$(OBJ)\mutant.obj		\
	$(OBJ)\nbqueue.obj		\
	$(OBJ)\pool.obj			\
	$(OBJ)\probe.obj		\
	$(OBJ)\profile.obj		\
	$(OBJ)\raise.obj		\
	$(OBJ)\resource.obj		\
	$(OBJ)\semphore.obj		\
	$(OBJ)\sysinfo.obj		\
	$(OBJ)\timer.obj		\
	$(OBJ)\worker.obj		\
	$(OBJ)\zone.obj			\
	$(OBJ)\uuid.obj			\
	$(OBJ)\win32.obj		\
	$(OBJ)\rundown.obj		\
	$(OBJ)\pushlock.obj		\
	$(OBJ)\keyedevent.obj

!include $(ntos)\BUILD\makefile.build

