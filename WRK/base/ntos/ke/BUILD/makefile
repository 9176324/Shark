#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

library		  = $(module)

!if "$(targ)" == "i386"
asobjs= \
	$(OBJ)\mpipia.obj   	\
	$(OBJ)\abiosa.obj   	\
	$(OBJ)\callout.obj  	\
	$(OBJ)\clockint.obj 	\
	$(OBJ)\ctxswap.obj  	\
	$(OBJ)\cpu.obj      	\
	$(OBJ)\emv86.obj    	\
	$(OBJ)\emxcptn.obj  	\
	$(OBJ)\int.obj      	\
	$(OBJ)\intsup.obj   	\
	$(OBJ)\i386pcr.obj  	\
	$(OBJ)\instemul.obj 	\
	$(OBJ)\ldtsup2.obj  	\
	$(OBJ)\newsysbg.obj 	\
	$(OBJ)\procstat.obj 	\
	$(OBJ)\spindbg.obj  	\
	$(OBJ)\spinlock.obj 	\
	$(OBJ)\sysstubs.obj 	\
	$(OBJ)\systable.obj 	\
	$(OBJ)\threadbg.obj 	\
	$(OBJ)\timindex.obj 	\
	$(OBJ)\trap.obj     	\
	$(OBJ)\zero.obj

ccarchobjs= \
	$(OBJ)\abiosc.obj	   	\
	$(OBJ)\allproc.obj	   	\
	$(OBJ)\apcuser.obj	  	\
	$(OBJ)\biosc.obj	  	\
	$(OBJ)\callback.obj   	\
	$(OBJ)\exceptn.obj	    \
	$(OBJ)\flush.obj	    \
	$(OBJ)\flushtb.obj	    \
	$(OBJ)\gdtsup.obj	    \
	$(OBJ)\intobj.obj	    \
	$(OBJ)\iopm.obj	    	\
	$(OBJ)\i386init.obj		\
	$(OBJ)\kernlini.obj		\
	$(OBJ)\largepag.obj		\
	$(OBJ)\ldtsup.obj	    \
	$(OBJ)\misc.obj	    	\
	$(OBJ)\mtrr.obj	    	\
	$(OBJ)\mtrramd.obj	    \
	$(OBJ)\pat.obj	        \
	$(OBJ)\thredini.obj		\
	$(OBJ)\trapc.obj
!else
asobjs= \
	$(OBJ)\apcint.obj		\
	$(OBJ)\callout.obj		\
	$(OBJ)\clockint.obj		\
	$(OBJ)\cpuid.obj		\
	$(OBJ)\ctxswap.obj		\
	$(OBJ)\dpcint.obj		\
	$(OBJ)\idle.obj			\
	$(OBJ)\intipi.obj		\
	$(OBJ)\intsup.obj		\
	$(OBJ)\miscs.obj		\
	$(OBJ)\procstat.obj		\
	$(OBJ)\profint.obj		\
	$(OBJ)\start.obj		\
	$(OBJ)\sysstubs.obj		\
	$(OBJ)\systable.obj		\
	$(OBJ)\threadbg.obj		\
	$(OBJ)\trap.obj			\
	$(OBJ)\zero.obj

ccarchobjs= \
	$(OBJ)\allproc.obj		\
	$(OBJ)\apcuser.obj		\
	$(OBJ)\callback.obj		\
	$(OBJ)\decode.obj		\
	$(OBJ)\exceptn.obj		\
	$(OBJ)\flush.obj		\
	$(OBJ)\flushtb.obj		\
	$(OBJ)\initkr.obj		\
	$(OBJ)\runtime.obj		\
	$(OBJ)\intobj.obj		\
	$(OBJ)\ipi.obj			\
	$(OBJ)\misc.obj			\
	$(OBJ)\pat.obj			\
	$(OBJ)\queuelock.obj	\
	$(OBJ)\spinlock.obj		\
	$(OBJ)\thredini.obj

!endif
			
ccobjs= \
	$(OBJ)\apcobj.obj		\
	$(OBJ)\apcsup.obj		\
	$(OBJ)\balmgr.obj		\
	$(OBJ)\bugcheck.obj		\
	$(OBJ)\config.obj		\
	$(OBJ)\debug.obj		\
	$(OBJ)\devquobj.obj		\
	$(OBJ)\dpclock.obj		\
	$(OBJ)\dpcobj.obj		\
	$(OBJ)\dpcsup.obj		\
	$(OBJ)\eventobj.obj		\
	$(OBJ)\gateobj.obj		\
	$(OBJ)\hifreqlk.obj		\
	$(OBJ)\idsched.obj		\
	$(OBJ)\interobj.obj		\
	$(OBJ)\kernldat.obj		\
	$(OBJ)\kevutil.obj		\
	$(OBJ)\kiinit.obj		\
	$(OBJ)\miscc.obj		\
	$(OBJ)\mutntobj.obj		\
	$(OBJ)\procobj.obj		\
	$(OBJ)\profobj.obj		\
	$(OBJ)\queueobj.obj		\
	$(OBJ)\raisexcp.obj		\
	$(OBJ)\semphobj.obj		\
	$(OBJ)\thkemul.obj		\
	$(OBJ)\thredobj.obj		\
	$(OBJ)\thredsup.obj		\
	$(OBJ)\timerobj.obj		\
	$(OBJ)\timersup.obj		\
	$(OBJ)\wait.obj		    \
	$(OBJ)\waitsup.obj		\
	$(OBJ)\xipi.obj		    \
	$(OBJ)\yield.obj

!include $(ntos)\BUILD\makefile.build

