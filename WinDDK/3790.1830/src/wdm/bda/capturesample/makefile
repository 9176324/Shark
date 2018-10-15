!IF 0

Copyright (C) Microsoft Corporation, 1999 - 1999

Module Name:
    makefile.

!ENDIF

!if "$(DDK_TARGET_OS)"=="Win2K"
!message This sample is not intended to target the Windows 2000 platform.
!elseif defined(IA64) || defined(AMD64)
!message This sample is not intended to target 64-bit platforms.
!else
!INCLUDE $(NTMAKEENV)\makefile.def
!endif
