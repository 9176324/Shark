# Microsoft Developer Studio Generated NMAKE File, Based on CPLSVR1.DSP
!IF "$(CFG)" == ""
CFG=CPLSVR1 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to CPLSVR1 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "CPLSVR1 - Win32 Release" && "$(CFG)" !=\
 "CPLSVR1 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CPLSVR1.MAK" CFG="CPLSVR1 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CPLSVR1 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "CPLSVR1 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "CPLSVR1 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\CPLSVR1.dll"

!ELSE 

ALL : "$(OUTDIR)\CPLSVR1.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Button.obj"
	-@erase "$(INTDIR)\Cplsvr1.obj"
	-@erase "$(INTDIR)\Cplsvr1.res"
	-@erase "$(INTDIR)\Dicputil.obj"
	-@erase "$(INTDIR)\Pages.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\CPLSVR1.dll"
	-@erase "$(OUTDIR)\CPLSVR1.exp"
	-@erase "$(OUTDIR)\CPLSVR1.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /G3 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\CPLSVR1.pch" /YX /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Cplsvr1.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\CPLSVR1.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)\CPLSVR1.pdb" /machine:I386\
 /def:"Cplsvr1.def" /out:"$(OUTDIR)\CPLSVR1.dll"\
 /implib:"$(OUTDIR)\CPLSVR1.lib" 
DEF_FILE= \
	"Cplsvr1.def"
LINK32_OBJS= \
	"$(INTDIR)\Button.obj" \
	"$(INTDIR)\Cplsvr1.obj" \
	"$(INTDIR)\Cplsvr1.res" \
	"$(INTDIR)\Dicputil.obj" \
	"$(INTDIR)\Pages.obj"

"$(OUTDIR)\CPLSVR1.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "CPLSVR1 - Win32 Debug"

OUTDIR=.\DEBUG
INTDIR=.\DEBUG

!IF "$(RECURSE)" == "0" 

ALL : ".\CPLSVR1.dll"

!ELSE 

ALL : ".\CPLSVR1.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Button.obj"
	-@erase "$(INTDIR)\Cplsvr1.obj"
	-@erase "$(INTDIR)\Cplsvr1.res"
	-@erase "$(INTDIR)\Dicputil.obj"
	-@erase "$(INTDIR)\Pages.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\CPLSVR1.exp"
	-@erase "$(OUTDIR)\CPLSVR1.lib"
	-@erase "$(OUTDIR)\CPLSVR1.pdb"
	-@erase ".\CPLSVR1.dll"
	-@erase ".\CPLSVR1.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\CPLSVR1.pch" /YX /FD /c 
CPP_OBJS="$(OUTDIR)"
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Cplsvr1.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\CPLSVR1.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib dinput.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)\CPLSVR1.pdb" /debug /machine:I386\
 /def:"Cplsvr1.def" /out:"CPLSVR1.dll"\
 /implib:"$(OUTDIR)\CPLSVR1.lib" /pdbtype:sept 
DEF_FILE= \
	"Cplsvr1.def"
LINK32_OBJS= \
	"$(INTDIR)\Button.obj" \
	"$(INTDIR)\Cplsvr1.obj" \
	"$(INTDIR)\Cplsvr1.res" \
	"$(INTDIR)\Dicputil.obj" \
	"$(INTDIR)\Pages.obj"

".\CPLSVR1.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "CPLSVR1 - Win32 Release" || "$(CFG)" ==\
 "CPLSVR1 - Win32 Debug"
SOURCE=Button.cpp
DEP_CPP_BUTTO=\
	{$(INCLUDE)}"Cplsvr1.h"\
	{$(INCLUDE)}"dicpl.h"\
	{$(INCLUDE)}"dinput.h"\
	{$(INCLUDE)}"dinputd.h"\
	

"$(INTDIR)\Button.obj" : $(SOURCE) $(DEP_CPP_BUTTO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=Cplsvr1.cpp
DEP_CPP_CPLSV=\
	{$(INCLUDE)}"Cplsvr1.h"\
	{$(INCLUDE)}"Pages.h"\
	{$(INCLUDE)}"dicpl.h"\
	{$(INCLUDE)}"dinput.h"\
	{$(INCLUDE)}"dinputd.h"\
	

"$(INTDIR)\Cplsvr1.obj" : $(SOURCE) $(DEP_CPP_CPLSV) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=Cplsvr1.rc
DEP_RSC_CPLSVR=\
	"BtnDwn.ico"\
	"BtnUp.ico"\
	"config.ico"\
	"winflag.ico"\
	

!IF  "$(CFG)" == "CPLSVR1 - Win32 Release"


"$(INTDIR)\Cplsvr1.res" : $(SOURCE) $(DEP_RSC_CPLSVR) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\Cplsvr1.res" \
 /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "CPLSVR1 - Win32 Debug"


"$(INTDIR)\Cplsvr1.res" : $(SOURCE) $(DEP_RSC_CPLSVR) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\Cplsvr1.res" \
 /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=Dicputil.cpp
DEP_CPP_DICPU=\
	{$(INCLUDE)}"Cplsvr1.h"\
	{$(INCLUDE)}"Dicputil.h"\
	{$(INCLUDE)}"dicpl.h"\
	{$(INCLUDE)}"dinput.h"\
	{$(INCLUDE)}"dinputd.h"\
	

"$(INTDIR)\Dicputil.obj" : $(SOURCE) $(DEP_CPP_DICPU) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=Pages.cpp
DEP_CPP_PAGES=\
	{$(INCLUDE)}"Cplsvr1.h"\
	{$(INCLUDE)}"Dicputil.h"\
	{$(INCLUDE)}"Pages.h"\
	{$(INCLUDE)}"dicpl.h"\
	{$(INCLUDE)}"dinput.h"\
	{$(INCLUDE)}"dinputd.h"\
	

"$(INTDIR)\Pages.obj" : $(SOURCE) $(DEP_CPP_PAGES) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

