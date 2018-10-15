#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

library		  = ntos$(module)

!if "$(targ)" == "i386"
asobjs= \
	$(OBJ)\debug2.obj		\
	$(OBJ)\stringsp.obj		\
	$(OBJ)\ioaccess.obj		\
	$(OBJ)\largeint.obj		\
	$(OBJ)\movemem.obj		\
	$(OBJ)\raise.obj		\
	$(OBJ)\stkwalk.obj		\
	$(OBJ)\xcptmisc.obj		\
	$(OBJ)\slist.obj

ccarchobjs= \
	$(OBJ)\context.obj		\
	$(OBJ)\debug3.obj		\
	$(OBJ)\divlarge.obj		\
	$(OBJ)\exdsptch.obj		\
	$(OBJ)\getcalr.obj 
!else
asobjs= \
	$(OBJ)\capture.obj   \
	$(OBJ)\chkstk.obj    \
	$(OBJ)\debugstb.obj  \
	$(OBJ)\movemem.obj   \
	$(OBJ)\slist.obj     \
	$(OBJ)\xcptmisc.obj

ccarchobjs= \
	$(OBJ)\context.obj   \
	$(OBJ)\exdsptch.obj  \
	$(OBJ)\misalign.obj  \
	$(OBJ)\raise.obj     \
	$(OBJ)\stkwalk.obj
!endif


ccobjs= \
	$(OBJ)\byteswap.obj		\
	$(OBJ)\acledit.obj		\
	$(OBJ)\add2stra.obj		\
	$(OBJ)\add2strw.obj		\
	$(OBJ)\assert.obj		\
	$(OBJ)\atom.obj			\
	$(OBJ)\avltable.obj		\
	$(OBJ)\bitmap.obj		\
	$(OBJ)\cnvint.obj		\
	$(OBJ)\debug.obj		\
	$(OBJ)\eballoc.obj		\
	$(OBJ)\environ.obj		\
	$(OBJ)\error.obj		\
	$(OBJ)\excptdbg.obj		\
	$(OBJ)\gentable.obj		\
	$(OBJ)\guid.obj			\
	$(OBJ)\imagedir.obj		\
	$(OBJ)\rtlnthdr.obj		\
	$(OBJ)\intbits.obj		\
	$(OBJ)\ldrrsrc.obj		\
	$(OBJ)\ldrreloc.obj		\
	$(OBJ)\lookup.obj		\
	$(OBJ)\message.obj		\
	$(OBJ)\nls.obj			\
	$(OBJ)\nlsxlat.obj		\
	$(OBJ)\pctohdr.obj		\
	$(OBJ)\peb.obj			\
	$(OBJ)\prefix.obj		\
	$(OBJ)\regutil.obj		\
	$(OBJ)\rtlassig.obj		\
	$(OBJ)\rtldata.obj		\
	$(OBJ)\rtlexec.obj		\
	$(OBJ)\sertl.obj		\
	$(OBJ)\splay.obj		\
	$(OBJ)\str2adda.obj		\
	$(OBJ)\str2addw.obj		\
	$(OBJ)\string.obj		\
	$(OBJ)\stktrace.obj		\
	$(OBJ)\sysvol.obj		\
	$(OBJ)\time.obj			\
	$(OBJ)\tracedb.obj

!include $(ntos)\BUILD\makefile.build

