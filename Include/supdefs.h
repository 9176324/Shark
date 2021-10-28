/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original Code is blindtiger.
*
*/

#ifndef _SUPDEFS_H_
#define _SUPDEFS_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    /** Internal error - we're screwed if this happens. */
#define ERR_INTERNAL_ERROR (-32)

/*******************************************************************************
* Defined Constants And Macros *
*******************************************************************************/
/** The support service name. */
#define ServiceString L"VBoxDrv"
/** NT Device name. */
#define DeviceString L"\\Device\\VBoxDrv"
/** Win Symlink name. */
#define SymbolicString L"\\DosDevices\\VBoxDrv"
/** soft registry. */
#define RegistryString L"\\Registry\\Machine\\Software\\Oracle\\VirtualBox"

/*
* IOCtl numbers.
* We're using the Win32 type of numbers here, thus the macros below.
* The SUP_IOCTL_FLAG macro is used to separate requests from 32-bit
* and 64-bit processes.
*/
#ifndef _WIN64 
# define SUP_IOCTL_FLAG 0
#else 
# define SUP_IOCTL_FLAG 128
#endif // !_WIN64

/* Automatic buffering, size not encoded. */
# define SUP_CTL_CODE_SIZE(Function, Size) \
CTL_CODE(FILE_DEVICE_UNKNOWN, (Function) | SUP_IOCTL_FLAG, METHOD_BUFFERED, FILE_WRITE_ACCESS)
# define SUP_CTL_CODE_BIG(Function) \
CTL_CODE(FILE_DEVICE_UNKNOWN, (Function) | SUP_IOCTL_FLAG, METHOD_BUFFERED, FILE_WRITE_ACCESS)
# define SUP_CTL_CODE_FAST(Function) \
CTL_CODE(FILE_DEVICE_UNKNOWN, (Function) | SUP_IOCTL_FLAG, METHOD_NEITHER, FILE_WRITE_ACCESS)
# define SUP_CTL_CODE_NO_SIZE(uIOCtl) (uIOCtl)

/** Fast path IOCtl: VMMR0_DO_RAW_RUN */
#define SUP_IOCTL_FAST_DO_RAW_RUN SUP_CTL_CODE_FAST(64)
/** Fast path IOCtl: VMMR0_DO_HWACC_RUN */
#define SUP_IOCTL_FAST_DO_HWACC_RUN SUP_CTL_CODE_FAST(65)
/** Just a NOP call for profiling the latency of a fast ioctl call to VMMR0. */
#define SUP_IOCTL_FAST_DO_NOP SUP_CTL_CODE_FAST(66)

/*******************************************************************************
* Structures and Typedefs *
*******************************************************************************/
#ifndef _WIN64 
# pragma pack(4) /* paranoia. */
#else 
# pragma pack(8) /* paranoia. */
#endif // !_WIN64

/**
* The paging mode.
*/
    typedef enum _SUPPAGINGMODE {
        /** The usual invalid entry.
        * This is returned by SUPGetPagingMode() */
        SUPPAGINGMODE_INVALID = 0,
        /** Normal 32-bit paging, no global pages */
        SUPPAGINGMODE_32_BIT,
        /** Normal 32-bit paging with global pages. */
        SUPPAGINGMODE_32_BIT_GLOBAL,
        /** PAE mode, no global pages, no NX. */
        SUPPAGINGMODE_PAE,
        /** PAE mode with global pages. */
        SUPPAGINGMODE_PAE_GLOBAL,
        /** PAE mode with NX, no global pages. */
        SUPPAGINGMODE_PAE_NX,
        /** PAE mode with global pages and NX. */
        SUPPAGINGMODE_PAE_GLOBAL_NX,
        /** AMD64 mode, no global pages. */
        SUPPAGINGMODE_AMD64,
        /** AMD64 mode with global pages, no NX. */
        SUPPAGINGMODE_AMD64_GLOBAL,
        /** AMD64 mode with NX, no global pages. */
        SUPPAGINGMODE_AMD64_NX,
        /** AMD64 mode with global pages and NX. */
        SUPPAGINGMODE_AMD64_GLOBAL_NX
    } SUPPAGINGMODE;

    /**
    * Common In/Out header.
    */
    typedef struct _SUPREQHDR {
        /** Cookie. */
        u32 u32Cookie;
        /** Session cookie. */
        u32 u32SessionCookie;
        /** The size of the input. */
        u32 cbIn;
        /** The size of the output. */
        u32 cbOut;
        /** Flags. See SUPREQHDR_FLAGS_* for details and values. */
        u32 fFlags;
        /** The VBox status code of the operation, out direction only. */
        s32 rc;
    } SUPREQHDR, *PSUPREQHDR;

    /** @name SUPREQHDR::fFlags values
    * @{ */
    /** Masks out the magic value. */
#define SUPREQHDR_FLAGS_MAGIC_MASK u32c(0xff0000ff)
/** The generic mask. */
#define SUPREQHDR_FLAGS_GEN_MASK u32c(0x0000ff00)
/** The request specific mask. */
#define SUPREQHDR_FLAGS_REQ_MASK u32c(0x00ff0000)

/** There is extra input that needs copying on some platforms. */
#define SUPREQHDR_FLAGS_EXTRA_IN u32c(0x00000100)
/** There is extra output that needs copying on some platforms. */
#define SUPREQHDR_FLAGS_EXTRA_OUT u32c(0x00000200)

/** The magic value. */
#define SUPREQHDR_FLAGS_MAGIC u32c(0x42000042)
/** The default value. Use this when no special stuff is requested. */
#define SUPREQHDR_FLAGS_DEFAULT SUPREQHDR_FLAGS_MAGIC
/** @} */

/** @name SUP_IOCTL_COOKIE
* @{
*/
/** The request size. */
#define SUP_IOCTL_COOKIE_SIZE sizeof(SUPCOOKIE)
/** Negotiate cookie. */
#define SUP_IOCTL_COOKIE SUP_CTL_CODE_SIZE(1, SUP_IOCTL_COOKIE_SIZE)
/** The SUPREQHDR::cbIn value. */
#define SUP_IOCTL_COOKIE_SIZE_IN sizeof(SUPREQHDR) + RTL_FIELD_SIZE(SUPCOOKIE, u.In)
/** The SUPREQHDR::cbOut value. */
#define SUP_IOCTL_COOKIE_SIZE_OUT sizeof(SUPREQHDR) + RTL_FIELD_SIZE(SUPCOOKIE, u.Out)
/** SUPCOOKIE_IN magic word. */
#define SUPCOOKIE_MAGIC "The Magic Word!"
/** The initial cookie. */
#define SUPCOOKIE_INITIAL_COOKIE 0x69726f74 /* 'tori' */

/** Current interface version.
* The upper 16-bit is the major version, the the lower the minor version.
* When incompatible changes are made, the upper major number has to be changed. */
#define SUPDRVIOC_VERSION 0x00070002

/** SUP_IOCTL_COOKIE. */
    typedef struct _SUPCOOKIE {
        /** The header.
        * u32Cookie must be set to SUPCOOKIE_INITIAL_COOKIE.
        * u32SessionCookie should be set to some random value. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Magic word. */
                char szMagic[16];
                /** The requested interface version number. */
                u32 u32ReqVersion;
                /** The minimum interface version number. */
                u32 u32MinVersion;
            } In;

            struct {
                /** Cookie. */
                u32 u32Cookie;
                /** Session cookie. */
                u32 u32SessionCookie;
                /** Interface version for this session. */
                u32 u32SessionVersion;
                /** The actual interface version in the driver. */
                u32 u32DriverVersion;
                /** Number of functions available for the SUP_IOCTL_QUERY_FUNCS request. */
                u32 cFunctions;
                /** Session handle. */
                ptr pSession;
            } Out;
        } u;
    } SUPCOOKIE, *PSUPCOOKIE;
    /** @} */

    /** @name SUP_IOCTL_QUERY_FUNCS
    * Query SUPR0 functions.
    * @{
    */
#define SUP_IOCTL_QUERY_FUNCS_SIZE(cFuncs) FIELD_OFFSET(SUPQUERYFUNCS, u.Out.aFunctions[(cFuncs)])
#define SUP_IOCTL_QUERY_FUNCS(cFuncs) SUP_CTL_CODE_SIZE(2, SUP_IOCTL_QUERY_FUNCS_SIZE(cFuncs))
#define SUP_IOCTL_QUERY_FUNCS_SIZE_IN sizeof(SUPREQHDR)
#define SUP_IOCTL_QUERY_FUNCS_SIZE_OUT(cFuncs) SUP_IOCTL_QUERY_FUNCS_SIZE(cFuncs)

    /** A function. */
    typedef struct SUPFUNC {
        /** Name - mangled. */
        char szName[32];
        /** Address. */
        ptr pfn;
    } SUPFUNC, *PSUPFUNC;

    typedef struct SUPQUERYFUNCS {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Number of functions returned. */
                u32 cFunctions;
                /** Array of functions. */
                SUPFUNC aFunctions[1];
            } Out;
        } u;
    } SUPQUERYFUNCS, *PSUPQUERYFUNCS;
    /** @} */

    /** @name SUP_IOCTL_IDT_INSTALL
    * Install IDT patch for calling processor.
    * @{
    */
#define SUP_IOCTL_IDT_INSTALL_SIZE sizeof(SUPIDTINSTALL)
#define SUP_IOCTL_IDT_INSTALL SUP_CTL_CODE_SIZE(3, SUP_IOCTL_IDT_INSTALL_SIZE)
#define SUP_IOCTL_IDT_INSTALL_SIZE_IN sizeof(SUPREQHDR)
#define SUP_IOCTL_IDT_INSTALL_SIZE_OUT sizeof(SUPIDTINSTALL)

    typedef struct SUPIDTINSTALL {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** The IDT entry number. */
                u8 u8Idt;
            } Out;
        } u;
    } SUPIDTINSTALL, *PSUPIDTINSTALL;
    /** @} */

    /** @name SUP_IOCTL_IDT_REMOVE
    * Remove IDT patch for calling processor.
    * @{
    */
#define SUP_IOCTL_IDT_REMOVE_SIZE sizeof(SUPIDTREMOVE)
#define SUP_IOCTL_IDT_REMOVE SUP_CTL_CODE_SIZE(4, SUP_IOCTL_IDT_REMOVE_SIZE)
#define SUP_IOCTL_IDT_REMOVE_SIZE_IN sizeof(SUPIDTREMOVE)
#define SUP_IOCTL_IDT_REMOVE_SIZE_OUT sizeof(SUPIDTREMOVE)

    typedef struct SUPIDTREMOVE {
        /** The header. */
        SUPREQHDR Hdr;
    } SUPIDTREMOVE, *PSUPIDTREMOVE;
    /** @}*/

    /** @name SUP_IOCTL_LDR_OPEN
    * Open an image.
    * @{
    */
#define SUP_IOCTL_LDR_OPEN_SIZE sizeof(SUPLDROPEN)
#define SUP_IOCTL_LDR_OPEN SUP_CTL_CODE_SIZE(5, SUP_IOCTL_LDR_OPEN_SIZE)
#define SUP_IOCTL_LDR_OPEN_SIZE_IN sizeof(SUPLDROPEN)
#define SUP_IOCTL_LDR_OPEN_SIZE_OUT (sizeof(SUPREQHDR) + RTL_FIELD_SIZE(SUPLDROPEN, u.Out))

    typedef struct _SUPLDROPEN {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Size of the image we'll be loading. */
                u32 cbImage;
                /** Image name.
                * This is the NAME of the image, not the file name. It is used
                * to share code with other processes. (Max len is 32 chars!) */
                c szName[32];
            } In;

            struct {
                /** The base address of the image. */
                ptr pvImageBase;
                /** Indicate whether or not the image requires loading. */
                b fNeedsLoading;
            } Out;
        } u;
    } SUPLDROPEN, *PSUPLDROPEN;
    /** @} */

    /** @name SUP_IOCTL_LDR_LOAD
    * Upload the image bits.
    * @{
    */
#define SUP_IOCTL_LDR_LOAD SUP_CTL_CODE_BIG(6)
#define SUP_IOCTL_LDR_LOAD_SIZE(cbImage) FIELD_OFFSET(SUPLDRLOAD, u.In.achImage[cbImage])
#define SUP_IOCTL_LDR_LOAD_SIZE_IN(cbImage) FIELD_OFFSET(SUPLDRLOAD, u.In.achImage[cbImage])
#define SUP_IOCTL_LDR_LOAD_SIZE_OUT sizeof(SUPREQHDR)

    /**
    * Symbol table entry.
    */
    typedef struct _SUPLDRSYM {
        /** Offset into of the string table. */
        u32 offName;
        /** Offset of the symbol relative to the image load address. */
        u32 offSymbol;
    } SUPLDRSYM, *PSUPLDRSYM;

    /**
    * SUPLDRLOAD::u::In::EP type.
    */
    typedef enum _SUPLDRLOADEP {
        SUPLDRLOADEP_NOTHING = 0,
        SUPLDRLOADEP_VMMR0,
        SUPLDRLOADEP_32BIT_HACK = 0x7fffffff
    } SUPLDRLOADEP;

    typedef struct _SUPLDRLOAD {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** The address of module initialization function. Similar to _DLL_InitTerm(hmod, 0). */
                ptr pfnModuleInit;
                /** The address of module termination function. Similar to _DLL_InitTerm(hmod, 1). */
                ptr pfnModuleTerm;
                /** Special entry points. */

                union {
                    struct {
                        /** The module handle (i.e. address). */
                        ptr pvVMMR0;
                        /** Address of VMMR0EntryInt function. */
                        ptr pvVMMR0EntryInt;
                        /** Address of VMMR0EntryFast function. */
                        ptr pvVMMR0EntryFast;
                        /** Address of VMMR0EntryEx function. */
                        ptr pvVMMR0EntryEx;
                    } VMMR0;
                } EP;

                /** Address. */
                ptr pvImageBase;
                /** Entry point type. */
                SUPLDRLOADEP eEPType;
                /** The offset of the symbol table. */
                u32 offSymbols;
                /** The number of entries in the symbol table. */
                u32 cSymbols;
                /** The offset of the string table. */
                u32 offStrTab;
                /** Size of the string table. */
                u32 cbStrTab;
                /** Size of image (including string and symbol tables). */
                u32 cbImage;
                /** The image data. */
                c achImage[1];
            } In;
        } u;
    } SUPLDRLOAD, *PSUPLDRLOAD;
    /** @} */

    /** @name SUP_IOCTL_LDR_FREE
    * Free an image.
    * @{
    */
#define SUP_IOCTL_LDR_FREE_SIZE sizeof(SUPLDRFREE)
#define SUP_IOCTL_LDR_FREE SUP_CTL_CODE_SIZE(7, SUP_IOCTL_LDR_FREE_SIZE)
#define SUP_IOCTL_LDR_FREE_SIZE_IN sizeof(SUPLDRFREE)
#define SUP_IOCTL_LDR_FREE_SIZE_OUT sizeof(SUPREQHDR)

    typedef struct _SUPLDRFREE {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Address. */
                ptr pvImageBase;
            } In;
        } u;
    } SUPLDRFREE, *PSUPLDRFREE;
    /** @} */

    /** @name SUP_IOCTL_LDR_GET_SYMBOL
    * Get address of a symbol within an image.
    * @{
    */
#define SUP_IOCTL_LDR_GET_SYMBOL_SIZE sizeof(SUPLDRGETSYMBOL)
#define SUP_IOCTL_LDR_GET_SYMBOL SUP_CTL_CODE_SIZE(8, SUP_IOCTL_LDR_GET_SYMBOL_SIZE)
#define SUP_IOCTL_LDR_GET_SYMBOL_SIZE_IN sizeof(SUPLDRGETSYMBOL)
#define SUP_IOCTL_LDR_GET_SYMBOL_SIZE_OUT (sizeof(SUPREQHDR) + RTL_FIELD_SIZE(SUPLDRGETSYMBOL, u.Out))

    typedef struct _SUPLDRGETSYMBOL {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Address. */
                ptr pvImageBase;
                /** The symbol name. */
                c szSymbol[64];
            } In;

            struct {
                /** The symbol address. */
                ptr pvSymbol;
            } Out;
        } u;
    } SUPLDRGETSYMBOL, *PSUPLDRGETSYMBOL;
    /** @} */

    /** @name SUP_IOCTL_CALL_VMMR0
    * Call the R0 VMM Entry point.
    *
    * @todo Might have to convert this to a big request...
    * @{
    */
#define SUP_IOCTL_CALL_VMMR0_SIZE(cbReq) FIELD_OFFSET(SUPCALLVMMR0, abReqPkt[cbReq])
#define SUP_IOCTL_CALL_VMMR0(cbReq) SUP_CTL_CODE_SIZE(9, SUP_IOCTL_CALL_VMMR0_SIZE(cbReq))
#define SUP_IOCTL_CALL_VMMR0_SIZE_IN(cbReq) SUP_IOCTL_CALL_VMMR0_SIZE(cbReq)
#define SUP_IOCTL_CALL_VMMR0_SIZE_OUT(cbReq) SUP_IOCTL_CALL_VMMR0_SIZE(cbReq)

    typedef struct SUPCALLVMMR0 {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** The VM handle. */
                ptr pVMR0;
                /** Which operation to execute. */
                u32 uOperation;
#if R0_ARCH_BITS == 64
                /** Alignment. */
                u32 u32Reserved;
#endif
                /** Argument to use when no request packet is supplied. */
                u64 u64Arg;
            } In;
        } u;
        /** The VMMR0Entry request packet. */
        u8 abReqPkt[1];
    } SUPCALLVMMR0, *PSUPCALLVMMR0;
    /** @} */

    /** @name SUP_IOCTL_LOW_ALLOC
    * Allocate memory below 4GB (physically).
    * @{
    */
#define SUP_IOCTL_LOW_ALLOC SUP_CTL_CODE_BIG(10)
#define SUP_IOCTL_LOW_ALLOC_SIZE(cPages) ((u32)FIELD_OFFSET(SUPLOWALLOC, u.Out.aPages[cPages]))
#define SUP_IOCTL_LOW_ALLOC_SIZE_IN (sizeof(SUPREQHDR) + RTL_FIELD_SIZE(SUPLOWALLOC, u.In))
#define SUP_IOCTL_LOW_ALLOC_SIZE_OUT(cPages) SUP_IOCTL_LOW_ALLOC_SIZE(cPages)

    typedef struct SUPLOWALLOC {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Number of pages to allocate. */
                u32 cPages;
            } In;

            struct {
                /** The ring-3 address of the allocated memory. */
                ptr pvR3;
                /** The ring-0 address of the allocated memory. */
                ptr pvR0;
                /** Array of pages. */
                u64 aPages[1];
            } Out;
        } u;
    } SUPLOWALLOC, *PSUPLOWALLOC;
    /** @} */

    /** @name SUP_IOCTL_LOW_FREE
    * Free low memory.
    * @{
    */
#define SUP_IOCTL_LOW_FREE SUP_CTL_CODE_SIZE(11, SUP_IOCTL_LOW_FREE_SIZE)
#define SUP_IOCTL_LOW_FREE_SIZE sizeof(SUPLOWFREE)
#define SUP_IOCTL_LOW_FREE_SIZE_IN sizeof(SUPLOWFREE)
#define SUP_IOCTL_LOW_FREE_SIZE_OUT sizeof(SUPREQHDR)
    typedef struct SUPLOWFREE {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** The ring-3 address of the memory to free. */
                ptr pvR3;
            } In;
        } u;
    } SUPLOWFREE, *PSUPLOWFREE;
    /** @} */

    /** @name SUP_IOCTL_PAGE_ALLOC
    * Allocate memory and map into the user process.
    * The memory is of course locked.
    * @{
    */
#define SUP_IOCTL_PAGE_ALLOC SUP_CTL_CODE_BIG(12)
#define SUP_IOCTL_PAGE_ALLOC_SIZE(cPages) FIELD_OFFSET(SUPPAGEALLOC, u.Out.aPages[cPages])
#define SUP_IOCTL_PAGE_ALLOC_SIZE_IN (sizeof(SUPREQHDR) + RTL_FIELD_SIZE(SUPPAGEALLOC, u.In))
#define SUP_IOCTL_PAGE_ALLOC_SIZE_OUT(cPages) SUP_IOCTL_PAGE_ALLOC_SIZE(cPages)

    typedef struct SUPPAGEALLOC {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Number of pages to allocate */
                u32 cPages;
            } In;

            struct {
                /** Returned ring-3 address. */
                ptr pvR3;
                /** The physical addresses of the allocated pages. */
                u64 aPages[1];
            } Out;
        } u;
    } SUPPAGEALLOC, *PSUPPAGEALLOC;
    /** @} */

    /** @name SUP_IOCTL_PAGE_FREE
    * Free memory allocated with SUP_IOCTL_PAGE_ALLOC.
    * @{
    */
#define SUP_IOCTL_PAGE_FREE_SIZE_IN sizeof(SUPPAGEFREE)
#define SUP_IOCTL_PAGE_FREE SUP_CTL_CODE_SIZE(13, SUP_IOCTL_PAGE_FREE_SIZE_IN)
#define SUP_IOCTL_PAGE_FREE_SIZE sizeof(SUPPAGEFREE)
#define SUP_IOCTL_PAGE_FREE_SIZE_OUT sizeof(SUPREQHDR) 

    typedef struct SUPPAGEFREE {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Address of memory range to free. */
                ptr pvR3;
            } In;
        } u;
    } SUPPAGEFREE, *PSUPPAGEFREE;
    /** @} */

    /** @name SUP_IOCTL_PAGE_LOCK
    * Pin down physical pages.
    * @{
    */
#define SUP_IOCTL_PAGE_LOCK SUP_CTL_CODE_BIG(14) 
#define SUP_IOCTL_PAGE_LOCK_SIZE_IN (sizeof(SUPREQHDR) + RTL_FIELD_SIZE(SUPPAGELOCK, u.In))
#define SUP_IOCTL_PAGE_LOCK_SIZE_OUT(cPages) FIELD_OFFSET(SUPPAGELOCK, u.Out.aPages[cPages])
#define SUP_IOCTL_PAGE_LOCK_SIZE(cPages) \
           (__max((size_t)SUP_IOCTL_PAGE_LOCK_SIZE_IN, (size_t)SUP_IOCTL_PAGE_LOCK_SIZE_OUT(cPages)))

    typedef struct SUPPAGELOCK {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Start of page range. Must be PAGE aligned. */
                ptr pvR3;
                /** The range size given as a page count. */
                u32 cPages;
            } In;

            struct {
                /** Array of pages. */
                u64 aPages[1];
            } Out;
        } u;
    } SUPPAGELOCK, *PSUPPAGELOCK;
    /** @} */

    /** @name SUP_IOCTL_PAGE_UNLOCK
    * Unpin physical pages.
    * @{ */
#define SUP_IOCTL_PAGE_UNLOCK_SIZE sizeof(SUPPAGEUNLOCK)
#define SUP_IOCTL_PAGE_UNLOCK SUP_CTL_CODE_SIZE(15, SUP_IOCTL_PAGE_UNLOCK_SIZE)
#define SUP_IOCTL_PAGE_UNLOCK_SIZE_IN sizeof(SUPPAGEUNLOCK)
#define SUP_IOCTL_PAGE_UNLOCK_SIZE_OUT sizeof(SUPREQHDR)

    typedef struct SUPPAGEUNLOCK {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** Start of page range of a range previuosly pinned. */
                ptr pvR3;
            } In;
        } u;
    } SUPPAGEUNLOCK, *PSUPPAGEUNLOCK;
    /** @} */

    /** @name SUP_IOCTL_CONT_ALLOC
    * Allocate contious memory.
    * @{
    */
#define SUP_IOCTL_CONT_ALLOC_SIZE sizeof(SUPCONTALLOC)
#define SUP_IOCTL_CONT_ALLOC SUP_CTL_CODE_SIZE(16, SUP_IOCTL_CONT_ALLOC_SIZE)
#define SUP_IOCTL_CONT_ALLOC_SIZE_IN (sizeof(SUPREQHDR) + RTL_FIELD_SIZE(SUPCONTALLOC, u.In))
#define SUP_IOCTL_CONT_ALLOC_SIZE_OUT sizeof(SUPCONTALLOC)

    typedef struct SUPCONTALLOC {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** The allocation size given as a page count. */
                u32 cPages;
            } In;

            struct {
                /** The address of the ring-0 mapping of the allocated memory. */
                ptr pvR0;
                /** The address of the ring-3 mapping of the allocated memory. */
                ptr pvR3;
                /** The physical address of the allocation. */
                u64 HCPhys;
            } Out;
        } u;
    } SUPCONTALLOC, *PSUPCONTALLOC;
    /** @} */

    /** @name SUP_IOCTL_CONT_FREE Input.
    * @{
    */
    /** Free contious memory. */
#define SUP_IOCTL_CONT_FREE_SIZE sizeof(SUPCONTFREE)
#define SUP_IOCTL_CONT_FREE SUP_CTL_CODE_SIZE(17, SUP_IOCTL_CONT_FREE_SIZE)
#define SUP_IOCTL_CONT_FREE_SIZE_IN sizeof(SUPCONTFREE)
#define SUP_IOCTL_CONT_FREE_SIZE_OUT sizeof(SUPREQHDR)

    typedef struct SUPCONTFREE {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** The ring-3 address of the memory to free. */
                ptr pvR3;
            } In;
        } u;
    } SUPCONTFREE, *PSUPCONTFREE;
    /** @} */

    /** @name SUP_IOCTL_GET_PAGING_MODE
    * Get the host paging mode.
    * @{
    */
#define SUP_IOCTL_GET_PAGING_MODE_SIZE sizeof(SUPGETPAGINGMODE)
#define SUP_IOCTL_GET_PAGING_MODE SUP_CTL_CODE_SIZE(18, SUP_IOCTL_GET_PAGING_MODE_SIZE)
#define SUP_IOCTL_GET_PAGING_MODE_SIZE_IN sizeof(SUPREQHDR)
#define SUP_IOCTL_GET_PAGING_MODE_SIZE_OUT sizeof(SUPGETPAGINGMODE)

    typedef struct SUPGETPAGINGMODE {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** The paging mode. */
                SUPPAGINGMODE enmMode;
            } Out;
        } u;
    } SUPGETPAGINGMODE, *PSUPGETPAGINGMODE;
    /** @} */

    /** @name SUP_IOCTL_SET_VM_FOR_FAST
    * Set the VM handle for doing fast call ioctl calls.
    * @{
    */
#define SUP_IOCTL_SET_VM_FOR_FAST_SIZE sizeof(SUPSETVMFORFAST)
#define SUP_IOCTL_SET_VM_FOR_FAST SUP_CTL_CODE_SIZE(19, SUP_IOCTL_SET_VM_FOR_FAST_SIZE)
#define SUP_IOCTL_SET_VM_FOR_FAST_SIZE_IN sizeof(SUPSETVMFORFAST)
#define SUP_IOCTL_SET_VM_FOR_FAST_SIZE_OUT sizeof(SUPREQHDR)

    typedef struct SUPSETVMFORFAST {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** The ring-0 VM handle (pointer). */
                ptr pVMR0;
            } In;
        } u;
    } SUPSETVMFORFAST, *PSUPSETVMFORFAST;
    /** @} */

    /** @name SUP_IOCTL_GIP_MAP
    * Map the GIP into user space.
    * @{
    */
#define SUP_IOCTL_GIP_MAP_SIZE sizeof(SUPGIPMAP)
#define SUP_IOCTL_GIP_MAP SUP_CTL_CODE_SIZE(20, SUP_IOCTL_GIP_MAP_SIZE)
#define SUP_IOCTL_GIP_MAP_SIZE_IN sizeof(SUPREQHDR)
#define SUP_IOCTL_GIP_MAP_SIZE_OUT sizeof(SUPGIPMAP)

    typedef struct SUPGIPMAP {
        /** The header. */
        SUPREQHDR Hdr;

        union {
            struct {
                /** The physical address of the GIP. */
                u64 HCPhysGip;
                /** Pointer to the read-only usermode GIP mapping for this session. */
                ptr pGipR3;
                /** Pointer to the supervisor mode GIP mapping. */
                ptr pGipR0;
            } Out;
        } u;
    } SUPGIPMAP, *PSUPGIPMAP;
    /** @} */

    /** @name SUP_IOCTL_GIP_UNMAP
    * Unmap the GIP.
    * @{
    */
#define SUP_IOCTL_GIP_UNMAP_SIZE sizeof(SUPGIPUNMAP)
#define SUP_IOCTL_GIP_UNMAP SUP_CTL_CODE_SIZE(21, SUP_IOCTL_GIP_UNMAP_SIZE)
#define SUP_IOCTL_GIP_UNMAP_SIZE_IN sizeof(SUPGIPUNMAP)
#define SUP_IOCTL_GIP_UNMAP_SIZE_OUT sizeof(SUPGIPUNMAP)

    typedef struct SUPGIPUNMAP {
        /** The header. */
        SUPREQHDR Hdr;
    } SUPGIPUNMAP, *PSUPGIPUNMAP;
    /** @} */

    /**
    * Header which heading all memory blocks.
    */
    typedef struct _MEMHDR {
        /** Magic (RTMEMHDR_MAGIC). */
        u32 u32Magic;
        /** Block flags (RTMEMHDR_FLAG_*). */
        u32 fFlags;
        /** The actual size of the block. */
        u32 cb;
        /** The request allocation size. */
        u32 cbReq;
    } MEMHDR, *PMEMHDR;

    /**
    * Loaded image.
    */
    typedef struct _SUPDRVLDRIMAGE {
        /** Next in chain. */
        struct SUPDRVLDRIMAGE * volatile pNext;
        /** Pointer to the image. */
        ptr pvImage;
        /** Pointer to the optional module initialization callback. */
        ptr pfnModuleInit;
        /** Pointer to the optional module termination callback. */
        ptr pfnModuleTerm;
        /** Size of the image. */
        u32 cbImage;
        /** The offset of the symbol table. */
        u32 offSymbols;
        /** The number of entries in the symbol table. */
        u32 cSymbols;
        /** The offset of the string table. */
        u32 offStrTab;
        /** Size of the string table. */
        u32 cbStrTab;
        /** The ldr image state. (IOCtl code of last opration.) */
        u32 uState;
        /** Usage count. */
        u32 volatile cUsage;
        /** Image name. */
        char szName[32];
    } SUPDRVLDRIMAGE, *PSUPDRVLDRIMAGE;

#ifndef _WIN64               
# define MEM_FENCE_EXTRA 4 
#else                                 
# define MEM_FENCE_EXTRA 16  
#endif // !_WIN64

#pragma pack() /* paranoia */

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_SUPDEFS_H_
