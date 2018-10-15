#include "agp.h"

//
// Define the location of the GART aperture control registers
//

//
// The GART registers on the 440 live in the host-PCI bridge.
// This is unfortunate, since the AGP driver attaches to the PCI-PCI (AGP)
// bridge. So we have to get to the host-PCI bridge config space
// and this is only possible because we KNOW this is bus 0, slot 0.
//
#define AGP_440_GART_BUS_ID     0
#define AGP_440_GART_SLOT_ID    0

#define AGP_440LX_IDENTIFIER    0x71808086
#define AGP_440LX2_IDENTIFIER   0x71828086
#define AGP_440BX_IDENTIFIER    0x71908086
#define AGP_815_IDENTIFIER      0x11308086

#define APBASE_OFFSET  0x10             // Aperture Base Address
#define APSIZE_OFFSET  0xB4             // Aperture Size Register
#define PACCFG_OFFSET  0x50             // PAC Configuration Register
#define AGPCTRL_OFFSET 0xB0             // AGP Control Register
#define ATTBASE_OFFSET 0xB8             // Aperture Translation Table Base

#define READ_SYNC_ENABLE 0x2000

//
// Conversions from APSIZE encoding to MB
//
// 0x3F (b 11 1111) =   4MB
// 0x3E (b 11 1110) =   8MB
// 0x3C (b 11 1100) =  16MB
// 0x38 (b 11 1000) =  32MB
// 0x30 (b 11 0000) =  64MB
// 0x20 (b 10 0000) = 128MB
// 0x00 (b 00 0000) = 256MB

#define AP_SIZE_4MB     0x3F
#define AP_SIZE_8MB     0x3E
#define AP_SIZE_16MB    0x3C
#define AP_SIZE_32MB    0x38
#define AP_SIZE_64MB    0x30
#define AP_SIZE_128MB   0x20
#define AP_SIZE_256MB   0x00

#define AP_SIZE_COUNT 7
#define AP_MIN_SIZE (4 * 1024 * 1024)
#define AP_MAX_SIZE (256 * 1024 * 1024)

#define AP_815_SIZE_COUNT 2
#define AP_815_MAX_SIZE (64 * 1024 * 1024)

//
// Define the GART table entry.
//
typedef struct _GART_ENTRY_HW {
    ULONG Valid     :  1;
    ULONG Reserved  : 11;
    ULONG Page      : 20;
} GART_ENTRY_HW, *PGART_ENTRY_HW;

#define PAGE_LOW_MASK 0xFFFFF

//
// GART Entry states are defined so that all software-only states
// have the Valid bit clear.
//
#define GART_ENTRY_VALID        1           //  001
#define GART_ENTRY_FREE         (0 | AgpV_PTE.Hard.Valid)  //  000 or 001

#define GART_ENTRY_WC           2           //  010
#define GART_ENTRY_UC           4           //  100

#define GART_ENTRY_RESERVED_WC  (GART_ENTRY_WC | GART_ENTRY_FREE)
#define GART_ENTRY_RESERVED_UC  (GART_ENTRY_UC | GART_ENTRY_FREE)

#define GART_ENTRY_VALID_WC     (GART_ENTRY_VALID)
#define GART_ENTRY_VALID_UC     (GART_ENTRY_VALID)

#define GART_ENTRY_GUARD  (GART_ENTRY_WC | GART_ENTRY_UC | GART_ENTRY_FREE)

#define VERIFIER_ENABLED (AgpV_PTE.Hard.Valid)

#define VERIFIER_PFN(GartPte) \
    ((GartPte)->Hard.Page == AgpV_PTE.Hard.Page)

//
// When the verifier is enabled, we also need to make sure that the
// PFN is equal to our verifier PFN, otherwise this entry is either
// a non-coherent mapped, or guard entry
//
#define GartEntryFree(GartPte) \
    (((GartPte)->Soft.State == GART_ENTRY_FREE) && \
     (!(VERIFIER_ENABLED && !VERIFIER_PFN(GartPte))))

//
// When the verifier is enabled, we need to make sure the PFN
// is equal to the verifier PFN, otherwise this is either a
// coherent-mapped, or guard page entry
//
#define GartEntryReserved(GartPte) \
    ((((GartPte)->Soft.State == GART_ENTRY_RESERVED_UC) || \
      ((GartPte)->Soft.State == GART_ENTRY_RESERVED_WC)) && \
     (!(VERIFIER_ENABLED && !VERIFIER_PFN(GartPte))))

//
// If the verifier is enabled, then in addition to match on
// requested state type, PFN must match the verifier PFN
//
#define GartEntryReservedAs(GartPte, Type) \
    (((GartPte)->Soft.State == (Type)) && \
     (!(VERIFIER_ENABLED && !VERIFIER_PFN(GartPte))))

//
// If the verifier is enabled, then an entry is mapped if it has
// one of the valid soft state types, i.e., not guard, and the PFN
// is not the verifier PFN
//
#define GartEntryMapped(GartPte) \
    ((((GartPte)->Soft.State == GART_ENTRY_VALID_WC) || \
      ((GartPte)->Soft.State == GART_ENTRY_VALID_UC)) && \
      (!(VERIFIER_ENABLED && VERIFIER_PFN(GartPte))))

typedef struct _GART_ENTRY_SW {
    ULONG State     : 3;
    ULONG Reserved  : 29;
} GART_ENTRY_SW, *PGART_ENTRY_SW;

typedef struct _GART_PTE {
    union {
        GART_ENTRY_HW Hard;
        ULONG      AsUlong;
        GART_ENTRY_SW Soft;
    };
} GART_PTE, *PGART_PTE;

//
// Define the layout of the hardware registers
//
typedef struct _AGPCTRL {
    ULONG Reserved1     : 7;
    ULONG GTLB_Enable   : 1;
    ULONG Reserved2     : 24;
} AGPCTRL, *PAGPCTRL;

typedef struct _PACCFG {
    USHORT Reserved1    : 9;
    USHORT GlobalEnable : 1;
    USHORT PCIEnable    : 1;
    USHORT Reserved2    : 5;
} PACCFG, *PPACCFG;


//
// Define the 440-specific extension
//
typedef struct _AGP440_EXTENSION {
    BOOLEAN             GlobalEnable;
    BOOLEAN             PCIEnable;
    PHYSICAL_ADDRESS    ApertureStart;
    ULONG               ApertureLength;
    PGART_PTE           Gart;
    ULONG               GartLength;
    PHYSICAL_ADDRESS    GartPhysical;
    ULONGLONG           SpecialTarget;
} AGP440_EXTENSION, *PAGP440_EXTENSION;

//
// Verifier definitions
//
extern GART_PTE AgpV_PTE;
extern ULONG AgpV_Flags;
extern ULONG AgpV_GartCachePteOffset;

NTSTATUS
AgpV_PlatformInit(
    IN PVOID AgpContext,
    IN PVOID VerifierPage,
    IN OUT PULONG VerifierFlags
    );

VOID
AgpV_PlatformWorker(
    IN PVOID AgpContext
    );

VOID
AgpV_PlatformStop(
    IN PVOID AgpContext
    );


