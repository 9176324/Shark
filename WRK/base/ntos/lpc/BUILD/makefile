#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

library		  = $(module)

!if "$(targ)" == "i386"
asobjs= \
	$(OBJ)\lpcmove.obj
ccarchobjs=
!else
asobjs=
ccarchobjs=
!endif

ccobjs= \
	$(OBJ)\lpcinit.obj		\
	$(OBJ)\lpccreat.obj		\
	$(OBJ)\lpclistn.obj		\
	$(OBJ)\lpcconn.obj		\
	$(OBJ)\lpccompl.obj		\
	$(OBJ)\lpcsend.obj		\
	$(OBJ)\lpcrecv.obj		\
	$(OBJ)\lpcreply.obj		\
	$(OBJ)\lpcquery.obj		\
	$(OBJ)\lpcqueue.obj		\
	$(OBJ)\lpcclose.obj		\
	$(OBJ)\lpcpriv.obj


!include $(ntos)\BUILD\makefile.build

