NAME = ffdrv1
EXT  = dll

IS_32 = 1
USEDDK32 = 1

GOALS = $(PBIN)\$(NAME).$(EXT)

LIBS    = kernel32.lib user32.lib uuid.lib dxguid.lib

OBJS    = main.obj clsfact.obj effdrv.obj effhw.obj

!if "$(DEBUG)" == "internal" #[
COPT =-DDEBUG #-Zi -FAs 
AOPT =-DDEBUG
LOPT =-debug:full -debugtype:cv
ROPT =-DDEBUG
!else if "$(DEBUG)" == "debug" #][
COPT =-DRDEBUG #-Zi -FAs 
AOPT =-DRDEBUG
LOPT =-debug:full -debugtype:cv
ROPT =-DRDEBUG
!else #][
COPT =
AOPT =
LOPT =-debug:none -incremental:no
ROPT =
!endif #]
DEF = $(NAME).def
RES = $(NAME).res 

CFLAGS  =-Fc -Oxw -QIfdiv- -YX -Gz $(COPT) $(INCLUDES) -DWIN95 -D_X86_ -Zl
!if "$(DEBUG)"!="$(DEBUG)"
CFLAGS  =$(CFLAGS) -DHID_SUPPORT
!endif

LFLAGS  =$(LOPT)
RCFLAGS =$(ROPT) $(INCLUDES)
AFLAGS	=$(AOPT) -Zp1 -Fl
!include $(MANROOT)\proj.mk

############################################################################
### Dependencies

MKFILE  =..\default.mk
CINCS   =\
        ..\effdrv.h     \

!IFNDEF ARCH
ARCH=x86
!ENDIF

$(PLIB)\$(NAME).lib: $(NAME).lib $(OBJLIB)
        copy $(@F) $@ >nul
        lib @<<
/OUT:$@
/NOLOGO
$@
$(OBJLIB)
<<

main.obj:       $(MKFILE) $(CINCS) ..\$(*B).c
clsfact.obj:    $(MKFILE) $(CINCS) ..\$(*B).c
effdrv.obj:     $(MKFILE) $(CINCS) ..\$(*B).c

###########################################################################

$(NAME).lbw :  ..\$(NAME).lbc
	wlib -n $(NAME).lbw @..\$(NAME).lbc
	
$(NAME).lib $(NAME).$(EXT): \
	$(OBJS) $(NAME).res ..\$(NAME).def ..\default.mk
        $(LINK) @<<
$(LFLAGS)
-nologo
-out:$(NAME).$(EXT)
-map:$(NAME).map
-dll
-base:0x60000000
-machine:i386
-subsystem:windows,4.0
-entry:DllEntryPoint 
-implib:$(NAME).lib
-def:..\$(NAME).def
-warn:2
$(LIBS)
$(NAME).res 
$(OBJS) 
<<
	mapsym -nologo $(NAME).map >nul
