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
	$(OBJ)\pae.obj
ccarchobjs= \
	$(OBJ)\init386.obj		\
	$(OBJ)\data386.obj		\
	$(OBJ)\procpae.obj		\
	$(OBJ)\procx86.obj		\
	$(OBJ)\paesup.obj		\
	$(OBJ)\setmodfy.obj
!else
asobjs=
ccarchobjs= \
	$(OBJ)\dataamd.obj		\
	$(OBJ)\initamd.obj

!endif

ccobjs= \
	$(OBJ)\acceschk.obj		\
	$(OBJ)\addrsup.obj		\
	$(OBJ)\allocpag.obj		\
	$(OBJ)\allocvm.obj		\
	$(OBJ)\buildmdl.obj		\
	$(OBJ)\creasect.obj		\
	$(OBJ)\deleteva.obj		\
	$(OBJ)\dmpaddr.obj		\
	$(OBJ)\extsect.obj		\
	$(OBJ)\flushbuf.obj		\
	$(OBJ)\flushsec.obj		\
	$(OBJ)\forksup.obj		\
	$(OBJ)\freevm.obj		\
	$(OBJ)\hypermap.obj		\
	$(OBJ)\iosup.obj		\
	$(OBJ)\lockvm.obj		\
	$(OBJ)\mapcache.obj		\
	$(OBJ)\mapview.obj		\
	$(OBJ)\miglobal.obj		\
	$(OBJ)\mmfault.obj		\
	$(OBJ)\mminit.obj		\
	$(OBJ)\mmsup.obj		\
	$(OBJ)\mmquota.obj		\
	$(OBJ)\modwrite.obj		\
	$(OBJ)\pagfault.obj		\
	$(OBJ)\pfndec.obj		\
	$(OBJ)\pfnlist.obj		\
	$(OBJ)\physical.obj		\
	$(OBJ)\procsup.obj		\
	$(OBJ)\protect.obj		\
	$(OBJ)\querysec.obj		\
	$(OBJ)\queryvm.obj		\
	$(OBJ)\readwrt.obj		\
	$(OBJ)\sectsup.obj		\
	$(OBJ)\session.obj		\
	$(OBJ)\sessload.obj		\
	$(OBJ)\shutdown.obj		\
	$(OBJ)\sysload.obj		\
	$(OBJ)\sysptes.obj		\
	$(OBJ)\umapview.obj		\
	$(OBJ)\vadtree.obj		\
	$(OBJ)\wslist.obj		\
	$(OBJ)\wsmanage.obj		\
	$(OBJ)\wstree.obj		\
	$(OBJ)\wrtfault.obj		\
	$(OBJ)\wrtwatch.obj		\
	$(OBJ)\zeropage.obj

!include $(ntos)\BUILD\makefile.build

