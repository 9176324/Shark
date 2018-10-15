$(O)\TapeWithGS.lib : $(O)\tape.lib makefile.inc
    link -lib -def:<< -out:$@ @<<
NAME NTOSKRNL

EXPORTS
    KeBugCheckEx
!if !$(AMD64)
    KeTickCount		CONSTANT
!endif
<<NOKEEP
$(LIBRARIAN_FLAGS: =
)
$(DDK_LIB_PATH)\ntoskrnl.lib
<<NOKEEP
    link -lib -out:$@ $@ $(O)\tape.lib


