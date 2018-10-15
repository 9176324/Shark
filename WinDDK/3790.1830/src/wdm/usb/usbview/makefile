#############################################################################
#
#   Copyright (C) Microsoft Corporation 1992-1997
#   All Rights Reserved.
#
#   makefile for USBVIEW
#
#############################################################################

# Paths are relative to the directory containing this file.

!IFDEF WIN95_BUILD

# Root of the SLM tree (i.e. the directory containing the DEV project).
!IFNDEF ROOT
ROOT = ..\..\..\..
!ENDIF

DEFAULTVERDIR   = retail
VERSIONLIST     = retail debug
IS_32           = TRUE

COMMONMKFILE    = makefile.mk

!include $(ROOT)\DEV\MASTER.MK


!ELSE

!IF DEFINED(_NT_TARGET_VERSION)
!	IF $(_NT_TARGET_VERSION)>=0x501
!		INCLUDE $(NTMAKEENV)\makefile.def
!	ELSE
#               Only warn once per directory
!               INCLUDE $(NTMAKEENV)\makefile.plt
!               IF "$(BUILD_PASS)"=="PASS1"
!		    message BUILDMSG: Warning : The sample "$(MAKEDIR)" is not valid for the current OS target.
!               ENDIF
!	ENDIF
!ELSE
!	INCLUDE $(NTMAKEENV)\makefile.def
!ENDIF

!ENDIF


