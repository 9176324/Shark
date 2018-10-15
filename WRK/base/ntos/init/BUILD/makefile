#
# Copyright (c) Microsoft Corporation. All rights reserved. 
# 
# You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
# If you do not agree to the terms, do not use the code.
# 

library		  = $(module)

asobjs=

ccarchobjs=

ccobjs= \
	$(OBJ)\initos.obj

nullobj 	 = ntkrnlmp.obj
extraobjs 	 = $(OBJ)\$(nullobj)
localtargets = nullobj
localclean	 = nullclean

!include $(ntos)\BUILD\makefile.build

nullobj: $(extraobjs)
	@for %%i in ($**) do xcopy $** $(topobj)\ /Y

nullclean:
	-del $(topobj)\$(nullobj)

