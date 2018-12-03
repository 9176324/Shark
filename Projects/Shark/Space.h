/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
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
* The Initial Developer of the Original e is blindtiger.
*
*/

#ifndef _SPACE_H_
#define _SPACE_H_

#include "..\..\WRK\base\ntos\mm\mi.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef struct _MM_AVL_NODE {
        union {
            LONG_PTR Balance : 2;
            struct _MM_AVL_NODE * Parent;
        };

        struct _MM_AVL_NODE * LeftChild;
        struct _MM_AVL_NODE * RightChild;
    }MM_AVL_NODE, *PMM_AVL_NODE;

    typedef struct _RTL_BALANCED_NODE {
        union {
            struct _RTL_BALANCED_NODE * Children[2];

            struct {
                struct _RTL_BALANCED_NODE * LeftChild;
                struct _RTL_BALANCED_NODE * RightChild;
            };
        };
        union {
            struct {
                UINT8 Red : 1;
                UINT8 Balance : 2;
            };

            struct _RTL_BALANCED_NODE * Parent;
        };
    }RTL_BALANCED_NODE, *PRTL_BALANCED_NODE;

    typedef struct _VAD_NODE {
        union {
            struct _MM_AVL_NODE AvlRoot;
            struct _RTL_BALANCED_NODE BalancedRoot;
        };

        union {
            struct {
                ULONG_PTR StartingVpn;
                ULONG_PTR EndingVpn;
            }Legacy;

            struct {
                ULONG StartingVpn;
                ULONG EndingVpn;
                CHAR StartingVpnHigh;
                CHAR EndingVpnHigh;
            };
        };
    }VAD_NODE, *PVAD_NODE;

    typedef struct _VAD_LOCAL_NODE {
        RTL_BALANCED_NODE BalancedRoot;
        ULONG_PTR BeginAddress;
        ULONG_PTR EndAddress;
    } VAD_LOCAL_NODE, *PVAD_LOCAL_NODE;

#ifndef _WIN64
    typedef struct _MMPTE_EX_HIGHLOW {
        ULONG32 LowPart;
        ULONG32 HighPart;
    }MMPTE_EX_HIGHLOW, *PMMPTE_EX_HIGHLOW;

#define _HARDWARE_PTE_WORKING_SET_BITS  11

    typedef struct _HARDWARE_PTE_EX {
        union {
            struct {
                ULONG64 Valid : 1;
                ULONG64 Write : 1;
                ULONG64 Owner : 1;
                ULONG64 WriteThrough : 1;
                ULONG64 CacheDisable : 1;
                ULONG64 Accessed : 1;
                ULONG64 Dirty : 1;
                ULONG64 LargePage : 1;
                ULONG64 Global : 1;
                ULONG64 CopyOnWrite : 1;
                ULONG64 Prototype : 1;
                ULONG64 Reserved0 : 1;
                ULONG64 PageFrameNumber : 26;
                ULONG64 reserved1 : 25 - (_HARDWARE_PTE_WORKING_SET_BITS + 1);
                ULONGLONG SoftwareWsIndex : _HARDWARE_PTE_WORKING_SET_BITS;
                ULONG64 NoExecute : 1;
            };

            struct {
                ULONG32 LowPart;
                ULONG32 HighPart;
            };
        };
    }HARDWARE_PTE_EX, *PHARDWARE_PTE_EX;

    typedef struct _MMPTE_EX_HARDWARE {
        ULONG64 Valid : 1;
        ULONG64 Dirty1 : 1;
        ULONG64 Owner : 1;
        ULONG64 WriteThrough : 1;
        ULONG64 CacheDisable : 1;
        ULONG64 Accessed : 1;
        ULONG64 Dirty : 1;
        ULONG64 LargePage : 1;
        ULONG64 Global : 1;
        ULONG64 CopyOnWrite : 1;
        ULONG64 Unused : 1;
        ULONG64 Write : 1;
        ULONG64 PageFrameNumber : 26;
        ULONG64 reserved1 : 25 - (_HARDWARE_PTE_WORKING_SET_BITS + 1);
        ULONGLONG SoftwareWsIndex : _HARDWARE_PTE_WORKING_SET_BITS;
        ULONG64 NoExecute : 1;
    }MMPTE_EX_HARDWARE, *PMMPTE_EX_HARDWARE;

    typedef struct _MMPTE_EX_PROTOTYPE {
        ULONG64 Valid : 1;
        ULONG64 Unused0 : 7;
        ULONG64 ReadOnly : 1;
        ULONG64 Unused1 : 1;
        ULONG64 Prototype : 1;
        ULONG64 Protection : 5;
        ULONG64 Unused : 16;
        ULONG64 ProtoAddress : 32;
    }MMPTE_EX_PROTOTYPE, *PMMPTE_EX_PROTOTYPE;

    typedef struct _MMPTE_EX_SOFTWARE {
        ULONG64 Valid : 1;
        ULONG64 PageFileLow : 4;
        ULONG64 Protection : 5;
        ULONG64 Prototype : 1;
        ULONG64 Transition : 1;
        ULONG64 InStore : 1;
        ULONG64 Unused1 : 19;
        ULONG64 PageFileHigh : 32;
    }MMPTE_EX_SOFTWARE, *PMMPTE_EX_SOFTWARE;

    typedef struct _MMPTE_EX_TIMESTAMP {
        ULONG64 MustBeZero : 1;
        ULONG64 PageFileLow : 4;
        ULONG64 Protection : 5;
        ULONG64 Prototype : 1;
        ULONG64 Transition : 1;
        ULONG64 Unused : 20;
        ULONG64 GlobalTimeStamp : 32;
    }MMPTE_EX_TIMESTAMP, *PMMPTE_EX_TIMESTAMP;

    typedef struct _MMPTE_EX_TRANSITION {
        ULONG64 Valid : 1;
        ULONG64 Write : 1;
        ULONG64 Owner : 1;
        ULONG64 WriteThrough : 1;
        ULONG64 CacheDisable : 1;
        ULONG64 Protection : 5;
        ULONG64 Prototype : 1;
        ULONG64 Transition : 1;
        ULONG64 PageFrameNumber : 26;
        ULONG64 Unused : 26;
    }MMPTE_EX_TRANSITION, *PMMPTE_EX_TRANSITION;

    typedef struct _MMPTE_EX_SUBSECTION {
        ULONG64 Valid : 1;
        ULONG64 Unused0 : 4;
        ULONG64 Protection : 5;
        ULONG64 Prototype : 1;
        ULONG64 Unused1 : 21;
        ULONG64 SubsectionAddress : 32;
    }MMPTE_EX_SUBSECTION, *PMMPTE_EX_SUBSECTION;

    typedef struct _MMPTE_EX_LIST {
        ULONG64 Valid : 1;
        ULONG64 OneEntry : 1;
        ULONG64 filler0 : 8;
        ULONG64 Prototype : 1;
        ULONG64 filler1 : 21;
        ULONG64 NextEntry : 32;
    }MMPTE_EX_LIST, *PMMPTE_EX_LIST;

    typedef struct _MMPTE_EX {
        union {
            ULONG64 Long;
            ULONG64 VolatileLong;

            struct _MMPTE_EX_HIGHLOW HighLow;
            struct _HARDWARE_PTE_EX Flush;
            struct _MMPTE_EX_HARDWARE Hard;
            struct _MMPTE_EX_PROTOTYPE Proto;
            struct _MMPTE_EX_SOFTWARE Soft;
            struct _MMPTE_EX_TIMESTAMP TimeStamp;
            struct _MMPTE_EX_TRANSITION Trans;
            struct _MMPTE_EX_SUBSECTION Subsect;
            struct _MMPTE_EX_LIST List;
        }u;
    }MMPTE_EX, *PMMPTE_EX;

    C_ASSERT(sizeof(MMPTE_EX) == 2 * sizeof(ULONG_PTR));
#endif // !_WIN64

#pragma pack(push, 1)
    typedef struct _MMPFNENTRY_7600 {
        struct {
            UCHAR PageLocation : 3;
            UCHAR WriteInProgress : 1;
            UCHAR Modified : 1;
            UCHAR ReadInProgress : 1;
            UCHAR CacheAttribute : 2;
        };

        struct {
            UCHAR Priority : 3;
            UCHAR Rom : 1;
            UCHAR InPageError : 1;
            UCHAR KernelStack : 1;
            UCHAR RemovalRequested : 1;
            UCHAR ParityError : 1;
        };
    }MMPFNENTRY_7600, *PMMPFNENTRY_7600;

    C_ASSERT(sizeof(MMPFNENTRY_7600) == 2);

    typedef struct _PFN_7600 {
        union {
            ULONG_PTR Flink;
            ULONG WsIndex;
            PKEVENT Event;
            PVOID Next;
            PVOID VolatileNext;
            PKTHREAD KernelStackOwner;
            SINGLE_LIST_ENTRY NextStackPfn;
        }u1;

        union {
            ULONG_PTR Blink;
            PMMPTE ImageProtoPte;
            ULONG_PTR ShareCount;
        }u2;

        union {
            PMMPTE PteAddress;
            PVOID VolatilePteAddress;
            LONG Lock;
            ULONG_PTR PteLong;
        };

        union {
            struct {
                USHORT ReferenceCount;
                MMPFNENTRY_7600 e1;
            };

            struct {
                union {
                    USHORT ReferenceCount;
                    SHORT VolatileReferenceCount;
                };

                USHORT ShortFlags;
            }e2;
        }u3;

#ifdef _WIN64
        USHORT UsedPageTableEntries;
        UCHAR VaType;
        UCHAR ViewCount;
#endif // _WIN64

        union {
            MMPTE OriginalPte;
            LONG AweReferenceCount;
        };

        union {
            struct {
#ifndef _WIN64 
                ULONG_PTR PteFrame : 25;
                ULONG_PTR PfnImageVerified : 1;
                ULONG_PTR AweAllocation : 1;
                ULONG_PTR PrototypePte : 1;
                ULONG_PTR PageColor : 4;
#else
                ULONG_PTR PteFrame : 52;
                ULONG_PTR Unused : 3;
                ULONG_PTR PfnImageVerified : 1;
                ULONG_PTR AweAllocation : 1;
                ULONG_PTR PrototypePte : 1;
                ULONG_PTR PageColor : 6;
#endif // !_WIN64
            };
        }u4;
    }PFN_7600, *PPFN_7600;

    C_ASSERT(FIELD_OFFSET(PFN_7600, u2) == sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_7600, PteAddress) == 2 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_7600, u3) == 3 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_7600, OriginalPte) == 4 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_7600, u4) == 5 * sizeof(ULONG_PTR));
    C_ASSERT(RTL_FIELD_SIZE(PFN_7600, u4) == sizeof(ULONG_PTR));

#ifndef _WIN64
    typedef struct _PFN_7600_EX {
        union {
            ULONG_PTR Flink;
            ULONG WsIndex;
            PKEVENT Event;
            PVOID Next;
            PVOID VolatileNext;
            PKTHREAD KernelStackOwner;
            SINGLE_LIST_ENTRY NextStackPfn;
        }u1;

        union {
            ULONG_PTR Blink;
            PMMPTE ImageProtoPte;
            ULONG_PTR ShareCount;
        }u2;

        union {
            PMMPTE PteAddress;
            PVOID VolatilePteAddress;
            LONG Lock;
            ULONG_PTR PteLong;
        };

        union {
            struct {
                USHORT ReferenceCount;
                MMPFNENTRY_7600 e1;
            };

            struct {
                union {
                    USHORT ReferenceCount;
                    SHORT VolatileReferenceCount;
                };

                USHORT ShortFlags;
            }e2;
        }u3;

        union {
            MMPTE_EX OriginalPte;
            LONG AweReferenceCount;
        };

        union {
            struct {
                ULONG_PTR PteFrame : 25;
                ULONG_PTR PfnImageVerified : 1;
                ULONG_PTR AweAllocation : 1;
                ULONG_PTR PrototypePte : 1;
                ULONG_PTR PageColor : 4;
            };
        }u4;
    }PFN_7600_EX, *PPFN_7600_EX;

    C_ASSERT(FIELD_OFFSET(PFN_7600_EX, u2) == sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_7600_EX, PteAddress) == 2 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_7600_EX, u3) == 3 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_7600_EX, OriginalPte) == 4 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_7600_EX, u4) == 6 * sizeof(ULONG_PTR));
    C_ASSERT(RTL_FIELD_SIZE(PFN_7600_EX, u4) == sizeof(ULONG_PTR));
#endif // !_WIN64

    typedef struct _MMPFNENTRY_9200 {
        struct {
            UCHAR PageLocation : 3;
            UCHAR WriteInProgress : 1;
            UCHAR Modified : 1;
            UCHAR ReadInProgress : 1;
            UCHAR CacheAttribute : 2;
        };

        struct {
            UCHAR Priority : 3;
            UCHAR OnProtectedStandby : 1;
            UCHAR InPageError : 1;
            UCHAR Spare : 1;
            UCHAR RemovalRequested : 1;
            UCHAR ParityError : 1;
        };
    }MMPFNENTRY_9200, *PMMPFNENTRY_9200;

    C_ASSERT(sizeof(MMPFNENTRY_9200) == 2);

    typedef struct _PFN_9200 {
        union {
#ifndef _WIN64
            ULONG_PTR Flink;
#else
            struct {
                ULONG_PTR Flink : 36;
                ULONG_PTR NodeFlinkHigh : 28;
            };
#endif // !_WIN64

            ULONG WsIndex;
            PKEVENT Event;
            PVOID Next;
            PVOID VolatileNext;
            PKTHREAD KernelStackOwner;
            SINGLE_LIST_ENTRY NextStackPfn;
        }u1;

        union {
            struct {
#ifndef _WIN64 
                ULONG_PTR Blink : 25;
                ULONG_PTR TbFlushStamp : 4;
                ULONG_PTR SpareBlink : 3;
#else
                ULONG_PTR Blink : 36;
                ULONG_PTR NodeBlinkHigh : 20;
                ULONG_PTR TbFlushStamp : 4;
                ULONG_PTR SpareBlink : 4;
#endif // !_WIN64
            };

            PMMPTE ImageProtoPte;
            ULONG_PTR ShareCount;
        }u2;

        union {
            PMMPTE PteAddress;
            PVOID VolatilePteAddress;
            LONG Lock;
            ULONG_PTR PteLong;
        };

        union {
            struct {
                USHORT ReferenceCount;
                MMPFNENTRY_9200 e1;
            };

            struct {
                union {
                    USHORT ReferenceCount;
                    SHORT VolatileReferenceCount;
                };

                union {
                    USHORT ShortFlags;
                    USHORT VolatileShortFlags;
                };
            }e2;
        }u3;

#ifndef _WIN64
        MMPTE_EX OriginalPte;
#else
        USHORT NodeBlinkLow;

        struct {
            UCHAR Unused : 4;
            UCHAR VaType : 4;
        };

        union {
            UCHAR ViewCount;
            UCHAR NodeFlinkLow;
        };

        MMPTE OriginalPte;
#endif // !_WIN64

        union {
            struct {
#ifndef _WIN64 
                ULONG_PTR PteFrame : 25;
                ULONG_PTR PageIdentity : 2;
                ULONG_PTR PrototypePte : 1;
                ULONG_PTR PageColor : 4;
#else
                ULONG_PTR PteFrame : 36;
                ULONG_PTR Channel : 2;
                ULONG_PTR Unused : 16;
                ULONG_PTR PfnExists : 1;
                ULONG_PTR PageIdentity : 2;
                ULONG_PTR PrototypePte : 1;
                ULONG_PTR PageColor : 6;
#endif // !_WIN64
            };

            ULONG_PTR EntireField;
        }u4;
    }PFN_9200, *PPFN_9200;

#ifndef _WIN64
    C_ASSERT(FIELD_OFFSET(PFN_9200, u2) == sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9200, u3) == 3 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9200, OriginalPte) == 4 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9200, u4) == 6 * sizeof(ULONG_PTR));
    C_ASSERT(RTL_FIELD_SIZE(PFN_9200, u4) == sizeof(ULONG_PTR));
#else
    C_ASSERT(FIELD_OFFSET(PFN_9200, u2) == sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9200, u3) == 3 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9200, OriginalPte) == 4 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9200, u4) == 5 * sizeof(ULONG_PTR));
    C_ASSERT(RTL_FIELD_SIZE(PFN_9200, u4) == sizeof(ULONG_PTR));
#endif // !_WIN64

    typedef MMPFNENTRY_9200 MMPFNENTRY_9600;
    typedef PMMPFNENTRY_9200 PMMPFNENTRY_9600;

    C_ASSERT(sizeof(MMPFNENTRY_9600) == 2);

    typedef struct _PFN_9600 {
        union {
#ifndef _WIN64
            ULONG_PTR Flink;
#else
            struct {
                ULONG_PTR Flink : 36;
                ULONG_PTR NodeFlinkHigh : 28;
            };
#endif // !_WIN64

            ULONG WsIndex;
            PKEVENT Event;
            PVOID Next;
            PVOID VolatileNext;
            PKTHREAD KernelStackOwner;
            SINGLE_LIST_ENTRY NextStackPfn;
        }u1;

        union {
            struct {
#ifndef _WIN64 
                ULONG_PTR Blink : 24;
                ULONG_PTR TbFlushStamp : 4;
                ULONG_PTR SpareBlink : 3;
#else
                ULONG_PTR Blink : 36;
                ULONG_PTR NodeBlinkHigh : 20;
                ULONG_PTR TbFlushStamp : 4;
                ULONG_PTR SpareBlink : 4;
#endif // !_WIN64
            };

            PMMPTE ImageProtoPte;
            ULONG_PTR ShareCount;
        }u2;

        union {
            PMMPTE PteAddress;
            PVOID VolatilePteAddress;
            LONG Lock;
            ULONG_PTR PteLong;
        };

        union {
            struct {
                USHORT ReferenceCount;
                MMPFNENTRY_9600 e1;
            };

            struct {
                union {
                    USHORT ReferenceCount;
                    SHORT VolatileReferenceCount;
                };

                union {
                    USHORT ShortFlags;
                    USHORT VolatileShortFlags;
                };
            }e2;
        }u3;

#ifndef _WIN64
        MMPTE_EX OriginalPte;
#else
        USHORT NodeBlinkLow;

        struct {
            UCHAR Unused : 4;
            UCHAR VaType : 4;
        };

        union {
            UCHAR ViewCount;
            UCHAR NodeFlinkLow;
        };

        MMPTE OriginalPte;
#endif // !_WIN64

        union {
            struct {
#ifndef _WIN64 
                ULONG_PTR PteFrame : 24;
                ULONG_PTR PageIdentity : 3;
                ULONG_PTR PrototypePte : 1;
                ULONG_PTR PageColor : 4;
#else
                ULONG_PTR PteFrame : 36;
                ULONG_PTR Channel : 2;
                ULONG_PTR Unused1 : 1;
                ULONG_PTR Unused2 : 1;
                ULONG_PTR Unused3 : 13;
                ULONG_PTR PfnExists : 1;
                ULONG_PTR PageIdentity : 3;
                ULONG_PTR PrototypePte : 1;
                ULONG_PTR PageColor : 6;
#endif // !_WIN64
            };

            ULONG_PTR EntireField;
        }u4;
    }PFN_9600, *PPFN_9600;

#ifndef _WIN64
    C_ASSERT(FIELD_OFFSET(PFN_9600, u2) == sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9600, u3) == 3 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9600, OriginalPte) == 4 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9600, u4) == 6 * sizeof(ULONG_PTR));
    C_ASSERT(RTL_FIELD_SIZE(PFN_9600, u4) == sizeof(ULONG_PTR));
#else
    C_ASSERT(FIELD_OFFSET(PFN_9600, u2) == sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9600, u3) == 3 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9600, OriginalPte) == 4 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_9600, u4) == 5 * sizeof(ULONG_PTR));
    C_ASSERT(RTL_FIELD_SIZE(PFN_9600, u4) == sizeof(ULONG_PTR));
#endif // !_WIN64

    typedef struct _MMPFNENTRY_10240 {
        struct {
            UCHAR PageLocation : 3;
            UCHAR WriteInProgress : 1;
            UCHAR Modified : 1;
            UCHAR ReadInProgress : 1;
            UCHAR CacheAttribute : 2;
        };

        struct {
            UCHAR Priority : 3;
            UCHAR OnProtectedStandby : 1;
            UCHAR InPageError : 1;
            UCHAR SystemChargedPage : 1;
            UCHAR RemovalRequested : 1;
            UCHAR ParityError : 1;
        };
    }MMPFNENTRY_10240, *PMMPFNENTRY_10240;

    C_ASSERT(sizeof(MMPFNENTRY_10240) == 2);

    typedef struct _MIPFNBLINK_10240 {
        union {
#ifndef _WIN64 
            union {
                struct {
                    ULONG_PTR Blink : 24;
                    ULONG_PTR TbFlushStamp : 4;
                    ULONG_PTR Unused : 1;
                    ULONG_PTR PageBlinkDeleteBit : 1;
                    ULONG_PTR PageBlinkLockBit : 1;
                };

                struct {
                    ULONG_PTR ShareCount : 30;
                    ULONG_PTR PageShareCountDeleteBit : 1;
                    ULONG_PTR PageShareCountLockBit : 1;
                };
            };
#else
            struct {
                ULONG_PTR Blink : 36;
                ULONG_PTR NodeBlinkHigh : 20;
                ULONG_PTR TbFlushStamp : 4;
                ULONG_PTR Unused : 2;
                ULONG_PTR PageBlinkDeleteBit : 1;
                ULONG_PTR PageBlinkLockBit : 1;
            };

            struct {
                ULONG_PTR ShareCount : 62;
                ULONG_PTR PageShareCountDeleteBit : 1;
                ULONG_PTR PageShareCountLockBit : 1;
            };
#endif // !_WIN64 

            ULONG_PTR EntireField;
            LONG_PTR Lock;

            struct {
#ifndef _WIN64
                ULONG_PTR LockNotUsed : 30;
#else
                ULONG_PTR LockNotUsed : 62;
#endif // !_WIN64
                ULONG_PTR DeleteBit : 1;
                ULONG_PTR LockBit : 1;
            };
        };
    }MIPFNBLINK_10240, *PMIPFNBLINK_10240;

    C_ASSERT(sizeof(MIPFNBLINK_10240) == sizeof(ULONG_PTR));

    typedef struct _PFN_10240 {
        union {
            LIST_ENTRY ListEntry;
            RTL_BALANCED_NODE TreeNode;

            struct {
                union {
#ifndef _WIN64
                    ULONG_PTR Flink;
#else
                    struct {
                        ULONG_PTR Flink : 36;
                        ULONG_PTR NodeFlinkHigh : 28;
                    };
#endif // !_WIN64

                    ULONG_PTR WsIndex;
                    PKEVENT Event;
                    PVOID Next;
                    PVOID VolatileNext;
                    PKTHREAD KernelStackOwner;
                    SINGLE_LIST_ENTRY NextStackPfn;
                }u1;

                union {
                    PMMPTE PteAddress;
                    PVOID VolatilePteAddress;
                    ULONG_PTR PteLong;
                };

#ifndef _WIN64
                MMPTE_EX OriginalPte;
#else
                MMPTE OriginalPte;
#endif // !_WIN64
            };
        };

        MIPFNBLINK_10240 u2;

        union {
            struct {
                USHORT ReferenceCount;
                MMPFNENTRY_10240 e1;
            };

            struct {
                USHORT ReferenceCount;

                union {
                    USHORT ShortFlags;
                    USHORT VolatileShortFlags;
                };
            }e2;
        }u3;

#ifdef _WIN64
        USHORT NodeBlinkLow;

        struct {
            UCHAR Unused : 4;
            UCHAR VaType : 4;
        };

        union {
            UCHAR ViewCount;
            UCHAR NodeFlinkLow;
        };
#endif // _WIN64

        union {
            struct {
#ifndef _WIN64
                ULONG_PTR PteFrame : 24;
                ULONG_PTR PageIdentity : 3;
                ULONG_PTR PrototypePte : 1;
                ULONG_PTR PageColor : 4;
#else
                ULONG_PTR PteFrame : 36;
                ULONG_PTR Channel : 2;
                ULONG_PTR Unused1 : 1;
                ULONG_PTR Unused2 : 1;
                ULONG_PTR Partition : 10;
                ULONG_PTR Spare : 2;
                ULONG_PTR FileOnly : 1;
                ULONG_PTR PfnExists : 1;
                ULONG_PTR PageIdentity : 3;
                ULONG_PTR PrototypePte : 1;
                ULONG_PTR PageColor : 6;
#endif // !_WIN64
            };

            ULONG_PTR EntireField;
        }u4;
    }PFN_10240, *PPFN_10240;

#ifndef _WIN64
    C_ASSERT(FIELD_OFFSET(PFN_10240, PteAddress) == sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_10240, OriginalPte) == 2 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_10240, u2) == 4 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_10240, u3) == 5 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_10240, u4) == 6 * sizeof(ULONG_PTR));
#else
    C_ASSERT(FIELD_OFFSET(PFN_10240, OriginalPte) == 2 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_10240, u2) == 3 * sizeof(ULONG_PTR));
    C_ASSERT(FIELD_OFFSET(PFN_10240, u3) == 4 * sizeof(ULONG_PTR));
    C_ASSERT(RTL_FIELD_SIZE(PFN_10240, u3) == sizeof(ULONG));
    C_ASSERT(FIELD_OFFSET(PFN_10240, NodeBlinkLow) == FIELD_OFFSET(PFN_10240, u3) + 4);
    C_ASSERT(FIELD_OFFSET(PFN_10240, ViewCount) == FIELD_OFFSET(PFN_10240, u3) + 7);
    C_ASSERT(FIELD_OFFSET(PFN_10240, u4) == 5 * sizeof(ULONG_PTR));
#endif // !_WIN64

    typedef union _PFN {
        SINGLE_LIST_ENTRY SingleListEntry;
        LIST_ENTRY ListEntry;

        PFN_7600 Pfn7600;
#ifndef _WIN64
        PFN_7600_EX Pfn7600Ex;
#endif // !_WIN64
        PFN_9200 Pfn9200;
        PFN_9600 Pfn9600;
        PFN_10240 Pfn10240;
    }PFN, *PPFN;

#pragma pack(pop)

#ifndef _WIN64
#define EMPTY_PFN_LIST 0xFFFFFFUI32
#else
#define EMPTY_PFN_LIST 0xFFFFFFFFFUI64
#endif // !_WIN64

    typedef struct _PROTOTYPE_PFN {
        ULONG_PTR Flink;
        ULONG_PTR Blink;
        PMMPTE PointerPte;
        ULONG Protection;
        UCHAR PageLocation;
        UCHAR Modified;
        UCHAR CacheAttribute;
        ULONG_PTR ShareCount;
        USHORT ReferenceCount;
        LONG_PTR PteFrame;
    }PROTOTYPE_PFN, *PPROTOTYPE_PFN;

#define PDE_MAPS_LARGE_PAGE(PPDE) ((PPDE)->u.Hard.LargePage == 1)
#define MAKE_PDE_MAP_LARGE_PAGE(PPDE) ((PPDE)->u.Hard.LargePage = 1)
#define GET_PAGE_FRAME_FROM_PTE(PPTE) ((PPTE)->u.Hard.PageFrameNumber)
#define GET_PAGE_FRAME_FROM_TRANSITION_PTE(PPTE) ((PPTE)->u.Trans.PageFrameNumber)
#define GET_PROTECTION_FROM_SOFT_PTE(PPTE) ((PPTE)->u.Soft.Protection)
#define GET_PROTECTION_FROM_TRANSITION_PTE(PPTE) ((PPTE)->u.Trans.Protection)

#define SET_PTE_DIRTY(PPTE) (PPTE)->u.Long |= HARDWARE_PTE_DIRTY_MASK
#define SET_ACCESSED_IN_PTE(PPTE, ACCESSED) (PPTE)->u.Hard.Accessed = ACCESSED

    typedef struct DECLSPEC_ALIGN(0x40) _PFNLIST {
        PFN_NUMBER Total;
        MMLISTS ListName;
        PFN_NUMBER Flink;
        PFN_NUMBER Blink;
        ULONG_PTR Lock;
    } PFNLIST, *PPFNLIST;

    C_ASSERT(sizeof(PFNLIST) == 0x40);

#define MAXIMUM_PFNSLIST_COUNT 0x2
#define MAXIMUM_PFNSLIST_DEPTH 0x3

#ifndef _WIN64
    typedef struct _PFNSLIST_HEADER {
        PSINGLE_LIST_ENTRY Next;
        USHORT Depth;
        USHORT Sequence;
    } PFNSLIST_HEADER, *PPFNSLIST_HEADER;

    C_ASSERT(sizeof(PFNSLIST_HEADER) == 8);
#else
    typedef struct _PFNSLIST_HEADER {
        struct {
            ULONG64 Depth : 16;
            ULONG64 Sequence : 48;
        };

        PSINGLE_LIST_ENTRY Next;
    }PFNSLIST_HEADER, *PPFNSLIST_HEADER;

    C_ASSERT(sizeof(PFNSLIST_HEADER) == 0x10);
#endif // !_WIN64

#ifndef _WIN64     
    typedef struct _PARTITION_PAGE_LISTS {
        PPFNLIST FreePagesByColor[2];
        PPFNSLIST_HEADER FreePageSlist[2];
        PFNLIST ZeroedPageListHead;
        PFNLIST FreePageListHead;
        PFNLIST StandbyPageListHead;
    }PARTITION_PAGE_LISTS, *PPARTITION_PAGE_LISTS;

    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, FreePagesByColor) == 0);
    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, FreePageSlist) == 8);
    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, ZeroedPageListHead) == 0x40);
    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, FreePageListHead) == 0x80);
    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, StandbyPageListHead) == 0xc0);
#else     
    typedef struct _PARTITION_PAGE_LISTS {
        PPFNLIST FreePagesByColor[2];
        PPFNSLIST_HEADER FreePageSlist[2];
        PFNLIST ZeroedPageListHead;
        PFNLIST FreePageListHead;
        PFNLIST StandbyPageListHead;
    }PARTITION_PAGE_LISTS, *PPARTITION_PAGE_LISTS;

    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, FreePagesByColor) == 0);
    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, FreePageSlist) == 0x10);
    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, ZeroedPageListHead) == 0x40);
    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, FreePageListHead) == 0x80);
    C_ASSERT(FIELD_OFFSET(PARTITION_PAGE_LISTS, StandbyPageListHead) == 0xc0);
#endif // !_WIN64

#ifndef _WIN64
    C_ASSERT(sizeof(PFN_7600) == 0x18);
    C_ASSERT(sizeof(PFN_7600_EX) == 0x1c);
    C_ASSERT(sizeof(PFN_9200) == 0x1c);
    C_ASSERT(sizeof(PFN_9600) == 0x1c);
    C_ASSERT(sizeof(PFN_10240) == 0x1c);
    C_ASSERT(sizeof(PFN) == 0x1c);

#define GetPxeAddress(va) (NULL);
#define GetPpeAddress(va) (NULL);
#define GetVirtualAddressMappedByPxe(Pxe) (NULL);
#define GetVirtualAddressMappedByPpe(Ppe) (NULL);

    BOOLEAN
        NTAPI
        IsPAEEnable(
            VOID
        );
#else
    C_ASSERT(sizeof(PFN_7600) == 0x30);
    C_ASSERT(sizeof(PFN_9200) == 0x30);
    C_ASSERT(sizeof(PFN_9600) == 0x30);
    C_ASSERT(sizeof(PFN_10240) == 0x30);
    C_ASSERT(sizeof(PFN) == 0x30);

    PMMPTE
        NTAPI
        GetPxeAddress(
            __in PVOID VirtualAddress
        );

    PMMPTE
        NTAPI
        GetPpeAddress(
            __in PVOID VirtualAddress
        );

    PVOID
        NTAPI
        GetVirtualAddressMappedByPxe(
            __in PMMPTE Pxe
        );

    PVOID
        NTAPI
        GetVirtualAddressMappedByPpe(
            __in PMMPTE Ppe
        );
#endif // !_WIN64

#ifndef _WIN64
    NTKERNELAPI
        PSLIST_ENTRY
        FASTCALL
        InterlockedPopEntrySList(
            __inout PSLIST_HEADER ListHead
        );

    NTKERNELAPI
        PSLIST_ENTRY
        FASTCALL
        InterlockedPushEntrySList(
            __inout PSLIST_HEADER ListHead,
            __inout PSLIST_ENTRY ListEntry
        );

    NTKERNELAPI
        PSLIST_ENTRY
        FASTCALL
        ExInterlockedFlushSList(
            __inout PSLIST_HEADER ListHead
        );
#else
    NTKERNELAPI
        PSLIST_ENTRY
        NTAPI
        ExpInterlockedPopEntrySList(
            __inout PSLIST_HEADER ListHead
        );

    NTKERNELAPI
        PSLIST_ENTRY
        NTAPI
        ExpInterlockedPushEntrySList(
            __inout PSLIST_HEADER ListHead,
            __inout PSLIST_ENTRY ListEntry
        );

    NTKERNELAPI
        PSLIST_ENTRY
        NTAPI
        ExpInterlockedFlushSList(
            __inout PSLIST_HEADER ListHead
        );
#endif // !_WIN64

    VOID
        NTAPI
        InitializeSystemSpace(
            __in PVOID Parameter
        );

    PMMPTE
        NTAPI
        GetPdeAddress(
            __in PVOID VirtualAddress
        );

    PMMPTE
        NTAPI
        GetPteAddress(
            __in PVOID VirtualAddress
        );

    PVOID
        NTAPI
        GetVirtualAddressMappedByPde(
            __in PMMPTE Pde
        );

    PVOID
        NTAPI
        GetVirtualAddressMappedByPte(
            __in PMMPTE Pte
        );

    VOID
        NTAPI
        SetPageProtection(
            __in_bcount(NumberOfBytes) PVOID VirtualAddress,
            __in SIZE_T NumberOfBytes,
            __in ULONG ProtectionMask
        );

    PVOID
        NTAPI
        AllocateIndependentPages(
            __in SIZE_T NumberOfBytes
        );

    VOID
        NTAPI
        FreeIndependentPages(
            __in PVOID VirtualAddress,
            __in SIZE_T NumberOfBytes
        );

    VOID
        NTAPI
        _FlushSingleTb(
            __in PVOID VirtualAddress
        );

    VOID
        NTAPI
        FlushSingleTb(
            __in PVOID VirtualAddress
        );

    FORCEINLINE
        VOID
        NTAPI
        _FlushMultipleTb(
            __in PVOID VirtualAddress,
            __in SIZE_T RegionSize
        )
    {
        PFN_NUMBER NumberOfPages = 0;
        PFN_NUMBER PageFrameIndex = 0;

        NumberOfPages = BYTES_TO_PAGES(RegionSize);

        for (PageFrameIndex = 0;
            PageFrameIndex < NumberOfPages;
            PageFrameIndex++) {
            _FlushSingleTb((PCHAR)VirtualAddress + PAGE_SIZE * PageFrameIndex);
        }
    }

    VOID
        NTAPI
        FlushMultipleTb(
            __in PVOID VirtualAddress,
            __in SIZE_T RegionSize,
            __in BOOLEAN AllProcesors
        );

    VOID
        NTAPI
        LocalVpn(
            __in PVAD_NODE VadNode,
            __out PVAD_LOCAL_NODE VadLocalNode
        );

    PVAD_NODE
        NTAPI
        GetVadRootProcess(
            __in PEPROCESS Process
        );

    PVAD_NODE
        FASTCALL
        GetNextNode(
            __in PVAD_NODE Node
        );

    extern PPFN PfnDatabase;
    extern PPFNLIST * FreePagesByColor;
    extern PPFNSLIST_HEADER FreePageSlist[2];
    extern ULONG MaximumColor;
    extern PFNSLIST_HEADER FreeSlist;
    extern ULONG64 ProtectToPteMask[32];

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_SPACE_H_
