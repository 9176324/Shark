#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

library		  = $(module)
specialcflags = -DUNICODE

asobjs=

ccarchobjs= \
	$(OBJ)\mcaevent.obj

ccobjs= \
	$(OBJ)\wmi.obj		 	\
	$(OBJ)\register.obj		\
	$(OBJ)\api.obj		 	\
	$(OBJ)\notify.obj		\
	$(OBJ)\callouts.obj		\
	$(OBJ)\tracelog.obj		\
	$(OBJ)\traceapi.obj		\
	$(OBJ)\tracesup.obj		\
	$(OBJ)\secure.obj		\
	$(OBJ)\globalog.obj		\
	$(OBJ)\wmidata.obj		\
	$(OBJ)\dataprov.obj		\
	$(OBJ)\alloc.obj		\
	$(OBJ)\chunk.obj		\
	$(OBJ)\ds.obj		 	\
	$(OBJ)\consumer.obj		\
	$(OBJ)\enabdisa.obj		\
	$(OBJ)\smbios.obj		\
	$(OBJ)\mca.obj		 	\
	$(OBJ)\provider.obj		

!include $(ntos)\BUILD\makefile.build

