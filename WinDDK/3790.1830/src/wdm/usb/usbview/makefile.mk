#############################################################################
#
#   Copyright (C) Microsoft Corporation 1996-1998
#   All Rights Reserved.
#
#   makefile.mk for USBVIEW
#
#############################################################################

ROOT            = ..\..\..\..\..
NAME            = USBVIEW
SRCDIR          = ..
IS_32           = TRUE
WANT_C1132      = TRUE
WANT_WDMDDK     = TRUE
IS_DDK          = TRUE

L32EXE          = $(NAME).exe
L32RES          = .\$(NAME).res
L32LIBSNODEP    = kernel32.lib user32.lib gdi32.lib comctl32.lib libc.lib cfgmgr32.lib
TARGETS         = $(L32EXE)
DEPENDNAME      = $(SRCDIR)\depend.mk
RCFLAGS         = -I$(ROOT)\DEV\INC

# Enable read-only string pooling to coalesce all the redundant strings
#
CFLAGS          = -GF

L32OBJS         =   usbview.obj \
                    enum.obj    \
                    display.obj \
                    debug.obj   \
                    devnode.obj \
                    dispaud.obj

!INCLUDE $(ROOT)\DEV\MASTER.MK

