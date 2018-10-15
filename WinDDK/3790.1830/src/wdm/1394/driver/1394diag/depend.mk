$(OBJDIR)\1394api.obj $(OBJDIR)\1394api.lst: ..\1394api.c \
	..\..\..\..\..\dev\ntddk\inc\alpharef.h \
	..\..\..\..\..\dev\ntddk\inc\basetsd.h \
	..\..\..\..\..\dev\ntddk\inc\bugcodes.h \
	..\..\..\..\..\dev\ntddk\inc\ntdef.h \
	..\..\..\..\..\dev\ntddk\inc\ntiologc.h \
	..\..\..\..\..\dev\ntddk\inc\ntstatus.h \
	..\..\..\..\..\dev\tools\c32\inc\alphaops.h \
	..\..\..\..\..\dev\tools\c32\inc\ctype.h \
	..\..\..\..\..\dev\tools\c32\inc\excpt.h \
	..\..\..\..\..\dev\tools\c32\inc\poppack.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack1.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack4.h \
	..\..\..\..\..\dev\tools\c32\inc\string.h \
	..\..\..\..\..\wdm\1394\tools\1394api\1394api.h \
	..\..\..\..\..\wdm\ddk\inc\1394.h ..\..\..\..\..\wdm\ddk\inc\wdm.h \
	..\1394diag.h ..\debug.h ..\pch.h
.PRECIOUS: $(OBJDIR)\1394api.lst

$(OBJDIR)\1394diag.obj $(OBJDIR)\1394diag.lst: ..\1394diag.c \
	..\..\..\..\..\dev\ntddk\inc\alpharef.h \
	..\..\..\..\..\dev\ntddk\inc\basetsd.h \
	..\..\..\..\..\dev\ntddk\inc\bugcodes.h \
	..\..\..\..\..\dev\ntddk\inc\ntdef.h \
	..\..\..\..\..\dev\ntddk\inc\ntiologc.h \
	..\..\..\..\..\dev\ntddk\inc\ntstatus.h \
	..\..\..\..\..\dev\tools\c32\inc\alphaops.h \
	..\..\..\..\..\dev\tools\c32\inc\ctype.h \
	..\..\..\..\..\dev\tools\c32\inc\excpt.h \
	..\..\..\..\..\dev\tools\c32\inc\poppack.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack1.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack4.h \
	..\..\..\..\..\dev\tools\c32\inc\string.h \
	..\..\..\..\..\wdm\1394\tools\1394api\1394api.h \
	..\..\..\..\..\wdm\ddk\inc\1394.h ..\..\..\..\..\wdm\ddk\inc\wdm.h \
	..\1394diag.h ..\debug.h ..\pch.h
.PRECIOUS: $(OBJDIR)\1394diag.lst

$(OBJDIR)\asyncapi.obj $(OBJDIR)\asyncapi.lst: ..\asyncapi.c \
	..\..\..\..\..\dev\ntddk\inc\alpharef.h \
	..\..\..\..\..\dev\ntddk\inc\basetsd.h \
	..\..\..\..\..\dev\ntddk\inc\bugcodes.h \
	..\..\..\..\..\dev\ntddk\inc\ntdef.h \
	..\..\..\..\..\dev\ntddk\inc\ntiologc.h \
	..\..\..\..\..\dev\ntddk\inc\ntstatus.h \
	..\..\..\..\..\dev\tools\c32\inc\alphaops.h \
	..\..\..\..\..\dev\tools\c32\inc\ctype.h \
	..\..\..\..\..\dev\tools\c32\inc\excpt.h \
	..\..\..\..\..\dev\tools\c32\inc\poppack.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack1.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack4.h \
	..\..\..\..\..\dev\tools\c32\inc\string.h \
	..\..\..\..\..\wdm\1394\tools\1394api\1394api.h \
	..\..\..\..\..\wdm\ddk\inc\1394.h ..\..\..\..\..\wdm\ddk\inc\wdm.h \
	..\1394diag.h ..\debug.h ..\pch.h
.PRECIOUS: $(OBJDIR)\asyncapi.lst

$(OBJDIR)\ioctl.obj $(OBJDIR)\ioctl.lst: ..\ioctl.c \
	..\..\..\..\..\dev\ntddk\inc\alpharef.h \
	..\..\..\..\..\dev\ntddk\inc\basetsd.h \
	..\..\..\..\..\dev\ntddk\inc\bugcodes.h \
	..\..\..\..\..\dev\ntddk\inc\ntdef.h \
	..\..\..\..\..\dev\ntddk\inc\ntiologc.h \
	..\..\..\..\..\dev\ntddk\inc\ntstatus.h \
	..\..\..\..\..\dev\tools\c32\inc\alphaops.h \
	..\..\..\..\..\dev\tools\c32\inc\ctype.h \
	..\..\..\..\..\dev\tools\c32\inc\excpt.h \
	..\..\..\..\..\dev\tools\c32\inc\poppack.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack1.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack4.h \
	..\..\..\..\..\dev\tools\c32\inc\string.h \
	..\..\..\..\..\wdm\1394\tools\1394api\1394api.h \
	..\..\..\..\..\wdm\ddk\inc\1394.h ..\..\..\..\..\wdm\ddk\inc\wdm.h \
	..\1394diag.h ..\debug.h ..\pch.h
.PRECIOUS: $(OBJDIR)\ioctl.lst

$(OBJDIR)\isochapi.obj $(OBJDIR)\isochapi.lst: ..\isochapi.c \
	..\..\..\..\..\dev\ntddk\inc\alpharef.h \
	..\..\..\..\..\dev\ntddk\inc\basetsd.h \
	..\..\..\..\..\dev\ntddk\inc\bugcodes.h \
	..\..\..\..\..\dev\ntddk\inc\ntdef.h \
	..\..\..\..\..\dev\ntddk\inc\ntiologc.h \
	..\..\..\..\..\dev\ntddk\inc\ntstatus.h \
	..\..\..\..\..\dev\tools\c32\inc\alphaops.h \
	..\..\..\..\..\dev\tools\c32\inc\ctype.h \
	..\..\..\..\..\dev\tools\c32\inc\excpt.h \
	..\..\..\..\..\dev\tools\c32\inc\poppack.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack1.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack4.h \
	..\..\..\..\..\dev\tools\c32\inc\string.h \
	..\..\..\..\..\wdm\1394\tools\1394api\1394api.h \
	..\..\..\..\..\wdm\ddk\inc\1394.h ..\..\..\..\..\wdm\ddk\inc\wdm.h \
	..\1394diag.h ..\debug.h ..\pch.h
.PRECIOUS: $(OBJDIR)\isochapi.lst

$(OBJDIR)\pnp.obj $(OBJDIR)\pnp.lst: ..\pnp.c \
	..\..\..\..\..\dev\ntddk\inc\alpharef.h \
	..\..\..\..\..\dev\ntddk\inc\basetsd.h \
	..\..\..\..\..\dev\ntddk\inc\bugcodes.h \
	..\..\..\..\..\dev\ntddk\inc\ntdef.h \
	..\..\..\..\..\dev\ntddk\inc\ntiologc.h \
	..\..\..\..\..\dev\ntddk\inc\ntstatus.h \
	..\..\..\..\..\dev\tools\c32\inc\alphaops.h \
	..\..\..\..\..\dev\tools\c32\inc\ctype.h \
	..\..\..\..\..\dev\tools\c32\inc\excpt.h \
	..\..\..\..\..\dev\tools\c32\inc\poppack.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack1.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack4.h \
	..\..\..\..\..\dev\tools\c32\inc\string.h \
	..\..\..\..\..\wdm\1394\tools\1394api\1394api.h \
	..\..\..\..\..\wdm\ddk\inc\1394.h ..\..\..\..\..\wdm\ddk\inc\wdm.h \
	..\1394diag.h ..\debug.h ..\pch.h
.PRECIOUS: $(OBJDIR)\pnp.lst

$(OBJDIR)\power.obj $(OBJDIR)\power.lst: ..\power.c \
	..\..\..\..\..\dev\ntddk\inc\alpharef.h \
	..\..\..\..\..\dev\ntddk\inc\basetsd.h \
	..\..\..\..\..\dev\ntddk\inc\bugcodes.h \
	..\..\..\..\..\dev\ntddk\inc\ntdef.h \
	..\..\..\..\..\dev\ntddk\inc\ntiologc.h \
	..\..\..\..\..\dev\ntddk\inc\ntstatus.h \
	..\..\..\..\..\dev\tools\c32\inc\alphaops.h \
	..\..\..\..\..\dev\tools\c32\inc\ctype.h \
	..\..\..\..\..\dev\tools\c32\inc\excpt.h \
	..\..\..\..\..\dev\tools\c32\inc\poppack.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack1.h \
	..\..\..\..\..\dev\tools\c32\inc\pshpack4.h \
	..\..\..\..\..\dev\tools\c32\inc\string.h \
	..\..\..\..\..\wdm\1394\tools\1394api\1394api.h \
	..\..\..\..\..\wdm\ddk\inc\1394.h ..\..\..\..\..\wdm\ddk\inc\wdm.h \
	..\1394diag.h ..\debug.h ..\pch.h
.PRECIOUS: $(OBJDIR)\power.lst


