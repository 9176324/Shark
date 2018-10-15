!IF 0

Copyright (C) Microsoft Corporation, 1998 - 1998

Module Name:

    makefile.

!ENDIF

#
# if building in a DDK environment
#
!IF defined(DDK_TARGET_OS)

#
# ensure that said build environment is at least Windows XP
# 0x500 == Windows 2000
# 0x501 == Windows XP
# 0x502 == Windows .NET
#
!    IF defined(_NT_TARGET_VERSION) && $(_NT_TARGET_VERSION)>=0x501
!        INCLUDE $(NTMAKEENV)\makefile.def
!    ELSE
!        message BUILDMSG: Warning : The sample "$(MAKEDIR)" is not valid for the current OS target.
!    ENDIF

!ELSE

#
# not a DDK environment, probably RAZZLE, so build
#
!    INCLUDE $(NTMAKEENV)\makefile.def

!ENDIF

