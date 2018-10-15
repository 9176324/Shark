#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

library		  = $(module)
specialcflags = -I..\..\ke

!if "$(targ)" == "i386"
asobjs=

ccarchobjs= \
	$(OBJ)\psctx386.obj		\
	$(OBJ)\psldt.obj		\
	$(OBJ)\psvdm.obj
!else
asobjs= \
	$(OBJ)\psctxwrap.obj

ccarchobjs= \
	$(OBJ)\psctxamd64.obj	\
	$(OBJ)\psldt.obj		\
	$(OBJ)\psvdm.obj
!endif

ccobjs= \
	$(OBJ)\psspnd.obj   	\
	$(OBJ)\pscid.obj    	\
	$(OBJ)\psimpers.obj 	\
	$(OBJ)\psinit.obj   	\
	$(OBJ)\psdelete.obj 	\
	$(OBJ)\psquery.obj  	\
	$(OBJ)\create.obj   	\
	$(OBJ)\kulookup.obj 	\
	$(OBJ)\psquota.obj  	\
	$(OBJ)\psctx.obj    	\
	$(OBJ)\psopen.obj   	\
	$(OBJ)\psjob.obj    	\
	$(OBJ)\security.obj 	\
	$(OBJ)\pshelper.obj 	\
	$(OBJ)\psenum.obj

!include $(ntos)\BUILD\makefile.build

