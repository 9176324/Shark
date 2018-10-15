# This file contains project settings common to all the binaries. This file
# is included in the sources files of driver, lib, app, and saruon.
# This file basically specified the include path for header files and the
# targetpath to place the final built binaries.

INCLUDES= ..\shared

!IFNDEF BUILD_ALT_DIR
SAMPLE_COMMON_DIR=..\disk
!ELSE
SAMPLE_COMMON_DIR=..\disk\$(BUILD_ALT_DIR)
!ENDIF

TARGETPATH=$(SAMPLE_COMMON_DIR)
