!IF 0

Copyright (c) 1994-1996  Microsoft Corporation

Module Name:

    amd64mk.inc

Abstract:

        This module contains the x86 specific build controls.  It is included
        by makefile.def.

Revision History:

!ENDIF

DELAYLOAD_SUPPORTED=1
!ifndef DELAYLOAD_VERSION
DELAYLOAD_VERSION=0x0200
!endif

#
# AMD64 option control
#
UMBASE=$(UMBASE:*=amd64)
UMLIBS=$(UMLIBS:*=amd64)
NTRES=$(NTRES:*=amd64)
UMRES=$(UMRES:*=amd64)
UMOBJS=$(UMOBJS:*=amd64)
LINKLIBS=$(LINKLIBS:*=amd64)
GDI_DRIVER_LIBS=$(GDI_DRIVER_LIBS:*=amd64)
DLLBASE=$(DLLBASE:*=amd64)
DLLDEF=$(DLLDEF:*=amd64)
MACHINE_TARGETLIBS=$(MACHINE_TARGETLIBS:*=amd64)
!ifdef USE_MSHTML_PDB_RULES
MACHINE_TARGETLIBS=$(MACHINE_TARGETLIBS:????????=*)
!endif
TARGET=$(TARGET:*=amd64)
DYNLINK_LIB=$(DYNLINK_LIB:*=amd64)
TARGETEXEFILES=$(TARGETEXEFILES:*=amd64)
TARGETLIBFILES=$(TARGETLIBFILES:*=amd64)
TARGETOBJFILES=$(TARGETOBJFILES:*=amd64)
UMOBJFILES=$(UMOBJFILES:*=amd64)
UMEXEFILES=$(UMEXEFILES:*=amd64)
HEADERFILE=$(HEADERFILE:*=amd64)
HEADEROBJNAME=$(HEADEROBJNAME:*=amd64)
HEADEROBJ=$(HEADEROBJ:*=amd64)
PRECOMPILED=$(PRECOMPILED:*=amd64)
PRECOMPILED_CXX=$(PRECOMPILED_CXX:*=amd64)
PRECOMPILED_TARGET=$(PRECOMPILED_TARGET:*=amd64)
MFC_STATIC_LIB=$(MFC_STATIC_LIB:*=amd64)
CRT_LIB_PATH=$(CRT_LIB_PATH:*=amd64)
SDK_LIB_PATH=$(SDK_LIB_PATH:*=amd64)
DDK_LIB_PATH=$(DDK_LIB_PATH:*=amd64)
IFSKIT_LIB_PATH=$(IFSKIT_LIB_PATH:*=amd64)
HALKIT_LIB_PATH=$(HALKIT_LIB_PATH:*=amd64)
PROCESSOR_LIB_PATH=$(PROCESSOR_LIB_PATH:*=amd64)
ORDER=$(ORDER:*=amd64)
OBJLIBFILES=$(OBJLIBFILES:*=amd64)
MISCFILES=$(MISCFILES) $(AMD64_MISCFILES)
SOURCES=$(SOURCES) $(AMD64_SOURCES)

ADDITIONAL_ATL_LIBS = $(ADDITIONAL_ATL_LIBS) $(ATL_LIB_PATH)\atlthunk.lib

!ifdef NTTARGETFILES
NTTARGETFILES=$(NTTARGETFILES:*=amd64)
!endif
!ifdef NTTARGETFILE0
NTTARGETFILE0=$(NTTARGETFILE0:*=amd64)
!endif
!ifdef NTTARGETFILE1
NTTARGETFILE1=$(NTTARGETFILE1:*=amd64)
!endif

!ifdef PROJECT_LIB_PATH
PROJECT_LIB_PATH=$(PROJECT_LIB_PATH:*=amd64)
!endif

ENTRY_SUFFIX=
GDI_ENTRY_SUFFIX=

DEFAULT_STACKRESERVE=0x80000
DEFAULT_STACKCOMMIT=0x2000

ADDITIONAL_ATL_LIBS = $(ADDITIONAL_ATL_LIBS)

!IFDEF AMD64_WARNING_LEVEL
MSC_WARNING_LEVEL=$(AMD64_WARNING_LEVEL)
!ENDIF

MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /Wp64 $(COMPILER_WX_SWITCH)

!ifdef AMD64_PERFFLAGS
PERFFLAGS = $(AMD64_PERFFLAGS)
!endif

# Set MSC_OPTIMIZATION.
# Order of precedence:
#   Platform specific override
#   Environment variable
#   System Default

!if defined(AMD64_OPTIMIZATION)
MSC_OPTIMIZATION=$(AMD64_OPTIMIZATION)
!elseif !defined(MSC_OPTIMIZATION)
MSC_OPTIMIZATION=/Oxt
!endif

LINK_TIME_CODE_GENERATION_MSC_FLAG=
!if defined(LINK_TIME_CODE_GENERATION) && !defined(FORCENATIVEOBJECT)
# Some objects must be built native, so turn off GL for those
LINK_TIME_CODE_GENERATION_MSC_FLAG=/GL
!endif

DBGFLAGS=$(DBGFLAGS) $(MSC_OPTIMIZATION) $(LINK_TIME_CODE_GENERATION_MSC_FLAG)

WIN32_LIBS=$(WIN32_LIBS) $(LIBRARY_PATH)\ntdll.lib

!IFDEF AMD64_CPPFLAGS
MSC_CPPFLAGS=$(AMD64_CPPFLAGS)
!ENDIF

!ifdef NO_STRING_POOLING
STRING_POOLING =
!else
!ifdef NO_READONLY_STRINGS
STRING_POOLING = /Gf
!else
STRING_POOLING = /GF
!endif
!endif

!ifdef USE_NATIVE_EH
!if "$(USE_NATIVE_EH)" == "ASYNC"
EH_FLAGS=/EHa
!else
!if "$(USE_NATIVE_EH)" == "CTHROW"
EH_FLAGS=/EHsc-
!else
EH_FLAGS=/EHsc
!endif
!endif
!else
EH_FLAGS=/EHs-c-
!endif

! ifdef USE_RTTI
RTTI_FLAGS=/GR
! else
RTTI_FLAGS=/GR-
! endif

!if "$(BUFFER_OVERFLOW_CHECKS)" == "" || \
    "$(BUFFER_OVERFLOW_CHECKS)" == "NO_NTDLL"
BUFFER_OVERFLOW_CHECKS=1
!endif

!if "$(BUFFER_OVERFLOW_CHECKS)" == "1" || \
    ("$(BUFFER_OVERFLOW_CHECKS)" == "CHK" && !$(FREEBUILD))
! if "$(TARGETTYPE)" == "DRIVER" || \
    "$(TARGETTYPE)" == "EXPORT_DRIVER" || \
    "$(TARGETTYPE)" == "DRIVER_LIBRARY"
!  if "$(DRIVERTYPE)" == "WDM" || "$(DRIVERTYPE)" == "wdm"
BO_FLAGS=/GS
BO_LIB=$(SDK_LIB_PATH)\BufferOverflowK.lib
DRIVER_ENTRY=GsDriverEntry
!  else
!   if "$(DRIVERTYPE)" != "VXD"
BO_FLAGS=/GS
BO_LIB=$(SDK_LIB_PATH)\BufferOverflowK.lib
DRIVER_ENTRY=GsDriverEntry
!   endif
!  endif
! else
!  if "$(TARGETTYPE)" == "GDI_DRIVER"
BO_FLAGS=/GS
BO_LIB=$(SDK_LIB_PATH)\BufferOverflowGDI.lib
GDI_DRIVER_ENTRY=GsDrvEnableDriver
!  else
!   if  "$(TARGETTYPE)" == "MINIPORT"
BO_FLAGS=/GS
BO_LIB=$(SDK_LIB_PATH)\BufferOverflowK.lib
DRIVER_ENTRY=GsDriverEntry
!   else
!    if "$(TARGETTYPE)" == "DYNLINK"
!     if !defined(RESOURCE_ONLY_DLL) 
!      if !(("$(UMTYPE)"== "nt") || ("$(UMTYPE)"=="ntss"))
BO_FLAGS=/GS
BO_LIB=$(SDK_LIB_PATH)\BufferOverflowU.lib
!       if "$(DLLENTRY)" == "-noentry"
!        if defined(USE_MSVCRT) || defined(USE_LIBCMT)
DLLENTRY=-entry:_DllMainCRTStartup
!        else
DLLENTRY=-entry:_DllMainCRTStartupForGS
!        endif
!       endif
!      endif
!     endif
!    else
!     if ("$(UMTYPE)"== "nt") || ("$(UMTYPE)"=="ntss")
BO_FLAGS=/GS
!      IF "$(NTTEST)" != ""
BO_LIB=$(SDK_LIB_PATH)\BufferOverflowK.lib
!      else
UMENTRY=-entry:NtProcessStartupForGS
BO_LIB=$(SDK_LIB_PATH)\BufferOverflow.lib
!      endif
!     else
BO_FLAGS=/GS
BO_LIB=$(SDK_LIB_PATH)\BufferOverflowU.lib
!     endif
!    endif
!   endif
!  endif
! endif
!else
! if "$(BUFFER_OVERFLOW_CHECKS)" == "NTDLL"
# Needed for EXEs that can't link to kernel32.dll (smss.exe, sprestrt.exe, etc)
BO_FLAGS=/GS
!  if ("$(UMTYPE)"== "nt") || ("$(UMTYPE)"=="ntss")
UMENTRY=-entry:NtProcessStartupForGS
!  endif
!  if "$(TARGETTYPE)" == "DYNLINK" && !defined(RESOURCE_ONLY_DLL) && "$(DLLENTRY)" == "-noentry"
!   if "$(TARGETNAME)" != "ntdll"
DLLENTRY=-entry:_DllMainCRTStartupForGS
!   endif
!  endif
BO_LIB=$(SDK_LIB_PATH)\BufferOverflow.lib
! else
!  if "$(BUFFER_OVERFLOW_CHECKS)" == "GDI"
# Needed for LIBs linked into drivers of type GDI_DRIVER
BO_FLAGS=/GS
BO_LIB=$(SDK_LIB_PATH)\BufferOverflowGDI.lib
!  else
!   if "$(BUFFER_OVERFLOW_CHECKS)" == "COMPILE_ONLY"
BO_FLAGS=/GS
!   endif
!  endif
! endif
!endif  # Overflow checks


!ifndef AMD64_CPU_OPTIMIZATION
AMD64_CPU_OPTIMIZATION=
!endif

!if defined(AMD64_STRUCT_PACKING)
MSC_STRUCT_PACKING=$(AMD64_STRUCT_PACKING)
!elseif !defined(MSC_STRUCT_PACKING)
MSC_STRUCT_PACKING=/Zp8
!endif

STDFLAGS= /c /Zl $(MSC_STRUCT_PACKING) /Gy $(CBSTRING) $(MSC_WARNING_LEVEL) $(MSC_CALL_TYPE) \
          $(ERATTA_FLAGS) $(AMD64_CPU_OPTIMIZATION) $(EH_FLAGS) \
          $(RTTI_FLAGS) $(STRING_POOLING) $(BO_FLAGS)

!if $(FREEBUILD)
ASM_DBG_DEFINES=-DDBG=0
!else
AMD64_ADBGFLAGS=$(AMD64_ADBGFLAGS) /Zi
!endif

DBGFLAGS=$(DBGFLAGS) /Zi
AMD64_ADBGFLAGS=$(AMD64_ADBGFLAGS) /Zi
LINKER_FLAGS=$(LINKER_FLAGS) -debug -debugtype:cv,fixup,pdata

!ifndef USE_PDB_TO_COMPILE
DBGFLAGS=$(DBGFLAGS:/Zi=-Z7)
DBGFLAGS=$(DBGFLAGS:-Zi=-Z7)
!else
! ifndef USE_MSHTML_PDB_RULES
!  if "$(TARGETTYPE)" == "LIBRARY"
DBGFLAGS=$(DBGFLAGS) /Fd$(TARGETPATH)\$(TARGET_DIRECTORY)\$(TARGETNAME).pdb
!  else
DBGFLAGS=$(DBGFLAGS) /Fd$(MAKEDIR)\$(_OBJ_DIR)\AMD64^\
!  endif
! else
!  ifndef TARGETPDB
TARGETPDB=$(TARGETNAME)
!  endif
!  ifndef ROOT
ROOT=$(MAKEDIR)
!  endif
DBGFLAGS=$(DBGFLAGS) /Fd$(ROOT)\$(_OBJ_DIR)\amd64\$(TARGETPDB).pdb
! endif
!endif

!ifndef NO_BROWSER_INFO
! ifdef BROWSER_INFO
DBGFLAGS=$(DBGFLAGS) /FR$(MAKEDIR)\$(_OBJ_DIR)\amd64^\
! endif
!endif

#
# amd64 option control
#

!IF "$(HALTYPE)" == ""
HALDEF=
!ELSE
HALDEF=-D$(HALTYPE)=1
!ENDIF

STD_DEFINES=-D_WIN64 -D_AMD64_ -DAMD64 $(HALDEF) $(MSC_CALL_DEFINE) $(STD_DEFINES)

#
# This indicates that we are compiling for a siulated (i.e. slow)
# environment.
#
# Remove this when we get a faster environment.
#

STD_DEFINES=$(STD_DEFINES) -D_AMD64_SIMULATOR_PERF_ -D_SKIP_IF_SIMULATOR_

#
# This indicates that we need to enable temp. workarounds for simulator
# bugs
#

STD_DEFINES=$(STD_DEFINES) -D_AMD64_SIMULATOR_

#
# This is here to enable (and help us find later) workarounds for AMD64
# tools such as MIDL.  Workarounds must have a tool fix in the works.
#

STD_DEFINES=$(STD_DEFINES) -D_AMD64_WORKAROUND_
MIDL_FLAGS=$(MIDL_FLAGS) -D_AMD64_WORKAROUND_


CDEFINES=$(STD_DEFINES) $(TARGET_DBG_DEFINES) \
              $(LIBC_DEFINES) $(C_DEFINES) $(NET_C_DEFINES) $(MFC_DEFINES)
CFLAGS=$(AMD64_FLAGS) $(NTAMD64FLAGS) $(STDFLAGS) $(DBGFLAGS) $(PERFFLAGS) $(USER_C_FLAGS)
AMD64_ASMFLAGS=$(AMD64_ADBGFLAGS) $(STD_DEFINES) $(ASM_DBG_DEFINES)\
              $(TARGET_DBG_DEFINES) $(ASM_DEFINES)

AMD64_CDEFINES=$(CDEFINES)
AMD64_CFLAGS=$(CFLAGS)

CC_NAME = cl -nologo

C_COMPILER_NAME     = $(CC_NAME)
CXX_COMPILER_NAME   = $(CC_NAME)
C_PREPROCESSOR_NAME = $(CC_NAME)
!ifndef AMD64_ASSEMBLER_NAME
AMD64_ASSEMBLER_NAME  = ml64
!endif

GLOBAL_C_FLAGS = -Iamd64\ -I. $(INCPATH1) $(CDEFINES) $(CFLAGS)

!if !$(FREEBUILD)
GLOBAL_C_FLAGS = $(GLOBAL_C_FLAGS) -d2home
!endif

GLOBAL_C_FLAGS=$(CRT_BUILD_FLAGS) $(GLOBAL_C_FLAGS) 

NP_COMPILER_FLAGS = $(GLOBAL_C_FLAGS) $(COMPILER_WARNINGS)

C_COMPILER_FLAGS = $(NP_COMPILER_FLAGS) $(PRECOMPILED)
CXX_COMPILER_FLAGS = $(NP_COMPILER_FLAGS) $(PRECOMPILED_CXX) $(MSC_CPPFLAGS) $(NTCPPFLAGS)
C_PREPROCESSOR_FLAGS = $(GLOBAL_C_FLAGS) -EP -Tc

C_PREPROCESSOR = $(C_PREPROCESSOR_NAME) $(C_PREPROCESSOR_FLAGS)
C_ASM_PREPROCESSOR = $(C_PREPROCESSOR_NAME) $(ASM_DEFINES) $(C_PREPROCESSOR_FLAGS)
C_COMPILER     = $(C_COMPILER_NAME) $(C_COMPILER_FLAGS)
NP_C_COMPILER  = $(C_COMPILER_NAME) $(NP_COMPILER_FLAGS)
CXX_COMPILER   = $(CXX_COMPILER_NAME) $(CXX_COMPILER_FLAGS)

AMD64_ASSEMBLER=$(AMD64_ASSEMBLER_NAME) -c -Cx -nologo -Iamd64\ -I. $(INCPATH1) $(AMD64_ASMFLAGS)

ECHO_MSG=$(C_COMPILER) $<
ECHO_MSG_P=$(NP_C_COMPILER) $**
ECHO_CXX_MSG=$(CXX_COMPILER) $<

ECHO_PRECOMPILED_MSG1=$(C_COMPILER_NAME) $(C_COMPILER_FLAGS) /Yl$(TARGETNAME) /Yc$(?F) $(HEADERFILE) \
               $(HEADEROBJ) $(PRECOMPILED_FLAG) $(PRECOMPILED_SOURCEFILE) 

ECHO_PRECOMPILED_MSG2=$(C_COMPILER_NAME) $(C_COMPILER_FLAGS) /Yl$(TARGETNAME) /Yc$(?F) $(HEADERFILE) \
               $(HEADEROBJ) $(PRECOMPILED_FLAG)

{}.c{}.asm:
    $(C_COMPILER_NAME) @<< /Fa $(MAKEDIR)\$(<F)
$(C_COMPILER_FLAGS: =
)
$(C_COMPILER_FLAGS:##= )
<<NOKEEP

#
# Here is a workaround for a masm code generation bug having to do with
# broken interaction between the preprocessor and the code generator.
#
# The workaround is to generate a preprocessed file first, then assemble
# that.  When the assembler is fixed, delete the following !if-related
# statements and the stuff in the !else block.
#

{..\amd64\}.asm{}.pp:
    $(AMD64_ASSEMBLER) -Zs /EP $(MAKEDIR)\$< > $(MAKEDIR)\$@

{amd64\}.asm{}.pp:
    $(AMD64_ASSEMBLER) -Zs /EP $(MAKEDIR)\$< > $(MAKEDIR)\$@

!if 1

{..\amd64\}.asm{}.lst:
    $(AMD64_ASSEMBLER) /Fl$@ /Fo$(_OBJ_DIR)\amd64\$(@B).obj $(MAKEDIR)\$<

{amd64\}.asm{}.lst:
    $(AMD64_ASSEMBLER) /Fl$@ /Fo$(MAKEDIR)\$(_OBJ_DIR)\amd64\$(@B).obj $(MAKEDIR)\$<

{..\amd64\}.asm{$(_OBJ_DIR)\amd64\}.obj:
    $(AMD64_ASSEMBLER) -Fo$(MAKEDIR)\$@ $(MAKEDIR)\$<

{amd64\}.asm{$(_OBJ_DIR)\amd64\}.obj:
    $(AMD64_ASSEMBLER) -Fo$(MAKEDIR)\$@ $(MAKEDIR)\$<

{$(O)\}.asm{$(O)\}.obj:
    $(AMD64_ASSEMBLER) -Fo$(MAKEDIR)\$@ $(MAKEDIR)\$<

!else

{..\amd64\}.asm{}.lst:
    -$(AMD64_ASSEMBLER) -Zs /EP $(MAKEDIR)\$< > $(MAKEDIR)\$(@R).pp
    $(AMD64_ASSEMBLER) /Fl$@ /Fo$(_OBJ_DIR)\amd64\$(@B).obj $(MAKEDIR)\$(@R).pp
    del $(MAKEDIR)\$(@R).pp

{amd64\}.asm{}.lst:
    -$(AMD64_ASSEMBLER) -Zs /EP $(MAKEDIR)\$< > $(MAKEDIR)\$(@R).pp
    $(AMD64_ASSEMBLER) /Fl$@ /Fo$(MAKEDIR)\$(_OBJ_DIR)\amd64\$(@B).obj $(MAKEDIR)\$(@R).pp
    del $(MAKEDIR)\$(@R).pp

{..\amd64\}.asm{$(_OBJ_DIR)\amd64\}.obj:
    -$(AMD64_ASSEMBLER) -Zs /EP $(MAKEDIR)\$< > $(MAKEDIR)\$(@R).pp
    $(AMD64_ASSEMBLER) -Fo$(MAKEDIR)\$@ $(MAKEDIR)\$(@R).pp
    del $(MAKEDIR)\$(@R).pp

{amd64\}.asm{$(_OBJ_DIR)\amd64\}.obj:
    -$(AMD64_ASSEMBLER) -Zs /EP $(MAKEDIR)\$< > $(MAKEDIR)\$(@R).pp
    $(AMD64_ASSEMBLER) -Fo$(MAKEDIR)\$@ $(MAKEDIR)\$(@R).pp
    del $(MAKEDIR)\$(@R).pp

{$(O)\}.asm{$(O)\}.obj:
    -$(AMD64_ASSEMBLER) -Zs /EP $(MAKEDIR)\$< > $(MAKEDIR)\$(@R).pp
    $(AMD64_ASSEMBLER) -Fo$(MAKEDIR)\$@ $(MAKEDIR)\$(@R).pp
    del $(MAKEDIR)\$(@R).pp

!endif

!IFDEF _NTAMD64LIBS
_NTLIBS=$(_NTAMD64LIBS)
!ENDIF

COPYDST=$(AMD64COPYDST)
LIB_COPY=ntamd64cp.cmd

!IF "$(NTDEBUGTYPE)" == "windbg" || "$(NTDEBUGTYPE)" == "both"
NTTEST_LINK_OPTIONS=-entry:KiSystemStartup
!ELSE
NTTEST_LINK_OPTIONS=-base:0x80100000 -entry:KiSystemStartup
!ENDIF

LINKER_FLAGS=$(LINKER_FLAGS) /opt:nowin98

!ifndef USE_LOCAL_HOTPATCHBUFFER
LINKER_FLAGS=$(LINKER_FLAGS) $(SDK_LIB_PATH)\hotpatch.obj
!endif

LINKER_FLAGS=$(LINKER_FLAGS) /functionpadmin:6

DRIVER_ALIGNMENT=0x0080
KERNEL_ALIGNMENT=0x1000

