/*++

Copyright (c) Microsoft 1998, All Rights Reserved

Module Name: 

    resource.h

Abstract

    Resource defines.

Author:

    Peter Binder (pbinder) 4/22/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/22/98  pbinder   birth
4/22/98  pbinder   taken for 1394test
5/04/98  pbinder   taken for win1394
--*/

#define IDC_STATIC                              -1

#define IDI_APP_ICON                            10

#define IDM_EXIT                                100
#define IDM_ABOUT                               101
#define IDM_SAVE_BUFFER                         102
#define IDM_CLEAR_BUFFER                        103

#define IDM_ENABLE_DIAGNOSTIC_MODE              150
#define IDM_DISABLE_DIAGNOSTIC_MODE             151
#define IDM_SELECT_TEST_DEVICE                  152
#define IDM_SELECT_VIRTUAL_TEST_DEVICE          153
#define IDM_ADD_VIRTUAL_DRIVER                  154
#define IDM_REMOVE_VIRTUAL_DRIVER               155

#define IDM_BUS_RESET                           200
#define IDM_GET_GENERATION_COUNT                201
#define IDM_GET_LOCAL_HOST_INFORMATION          202
#define IDM_GET_ADDRESS_FROM_DEVICE_OBJECT      203
#define IDM_CONTROL                             204
#define IDM_GET_MAX_SPEED_BETWEEN_DEVICES       205
#define IDM_GET_CONFIGURATION_INFORMATION       206
#define IDM_SET_DEVICE_XMIT_PROPERTIES          207
#define IDM_SEND_PHY_CONFIG_PACKET              208
#define IDM_BUS_RESET_NOTIFICATION              209
#define IDM_SET_LOCAL_HOST_PROPERTIES           210

#define IDM_ALLOCATE_ADDRESS_RANGE              250
#define IDM_FREE_ADDRESS_RANGE                  251
#define IDM_ASYNC_READ                          252
#define IDM_ASYNC_WRITE                         253
#define IDM_ASYNC_LOCK                          254
#define IDM_ASYNC_START_LOOPBACK                255
#define IDM_ASYNC_STOP_LOOPBACK                 256
#define IDM_ASYNC_STREAM                        257
#define IDM_ASYNC_STREAM_START_LOOPBACK         258
#define IDM_ASYNC_STREAM_STOP_LOOPBACK          259

#define IDM_ISOCH_ALLOCATE_BANDWIDTH            300
#define IDM_ISOCH_ALLOCATE_CHANNEL              301
#define IDM_ISOCH_ALLOCATE_RESOURCES            302
#define IDM_ISOCH_ATTACH_BUFFERS                303
#define IDM_ISOCH_DETACH_BUFFERS                304
#define IDM_ISOCH_FREE_BANDWIDTH                305
#define IDM_ISOCH_FREE_CHANNEL                  306
#define IDM_ISOCH_FREE_RESOURCES                307
#define IDM_ISOCH_LISTEN                        308
#define IDM_ISOCH_QUERY_CURRENT_CYCLE_TIME      309
#define IDM_ISOCH_QUERY_RESOURCES               310
#define IDM_ISOCH_SET_CHANNEL_BANDWIDTH         311
#define IDM_ISOCH_STOP                          312
#define IDM_ISOCH_TALK                          313
#define IDM_ISOCH_START_LOOPBACK                314
#define IDM_ISOCH_STOP_LOOPBACK                 315

#define IDC_1394_DEVICES                        500

//
//  1394 Commands
//

// bus reset
#define IDC_BUS_RESET_FORCE_ROOT                1000

// get local host information
#define IDC_GET_HOST_UNIQUE_ID                  1050
#define IDC_GET_HOST_HOST_CAPABILITIES          1051
#define IDC_GET_HOST_POWER_SUPPLIED             1052
#define IDC_GET_HOST_PHYS_ADDR_ROUTINE          1053
#define IDC_GET_HOST_CONFIG_ROM                 1054
#define IDC_GET_SPEED_MAP                       1055
#define IDC_GET_TOPOLOGY_MAP                    1056

// get 1394 address from device object
#define IDC_GET_ADDR_USE_LOCAL_NODE             1100

// get max speed between devices
#define IDC_GET_MAX_USE_LOCAL_NODE              1150

// set device xmit properties
#define IDC_SET_DEVICE_XMIT_100MBPS             1200
#define IDC_SET_DEVICE_XMIT_200MBPS             1201
#define IDC_SET_DEVICE_XMIT_400MBPS             1202
#define IDC_SET_DEVICE_XMIT_1600MBPS            1203
#define IDC_SET_DEVICE_XMIT_FASTEST             1204

// send phy config packet
#define IDC_SEND_PHY_PHYS_ID                    1250
#define IDC_SEND_PHY_PACKET_ID                  1251
#define IDC_SEND_PHY_GAP_COUNT                  1252
#define IDC_SEND_PHY_SET_GAP_COUNT              1253
#define IDC_SEND_PHY_FORCE_ROOT                 1254
#define IDC_SEND_PHY_RESERVED1                  1255
#define IDC_SEND_PHY_RESERVED2                  1256
#define IDC_SEND_PHY_INVERSE                    1257

// bus reset notification
#define IDC_BUS_RESET_NOTIFY_REGISTER           1300
#define IDC_BUS_RESET_NOTIFY_DEREGISTER         1301

// set local host properties
#define IDC_SET_LOCAL_HOST_LEVEL                1370

#define IDC_SET_LOCAL_HOST_LEVEL_GAP_COUNT      1350
#define IDC_SET_LOCAL_HOST_LEVEL_CROM           1351
#define IDC_SET_LOCAL_HOST_GAP_COUNT            1352
#define IDC_SET_LOCAL_HOST_CROM_ADD             1353
#define IDC_SET_LOCAL_HOST_CROM_REMOVE          1354
#define IDC_SET_LOCAL_HOST_CROM_HCROMDATA       1355
#define IDC_SET_LOCAL_HOST_CROM_NLENGTH         1356
#define IDC_SET_LOCAL_HOST_CROM_BUFFER          1357

//
//  Async Commands
//

// allocate address range
#define IDC_ASYNC_ALLOC_LENGTH                  2000
#define IDC_ASYNC_ALLOC_MAX_SEGMENT_SIZE        2001
#define IDC_ASYNC_ALLOC_OFFSET_HIGH             2002
#define IDC_ASYNC_ALLOC_OFFSET_LOW              2003
#define IDC_ASYNC_ALLOC_USE_MDL                 2004
#define IDC_ASYNC_ALLOC_USE_FIFO                2005
#define IDC_ASYNC_ALLOC_USE_NONE                2006
#define IDC_ASYNC_ALLOC_USE_BIG_ENDIAN          2007
#define IDC_ASYNC_ALLOC_ACCESS_READ             2008
#define IDC_ASYNC_ALLOC_ACCESS_WRITE            2009
#define IDC_ASYNC_ALLOC_ACCESS_LOCK             2010
#define IDC_ASYNC_ALLOC_ACCESS_BROADCAST        2011
#define IDC_ASYNC_ALLOC_NOTIFY_READ             2012
#define IDC_ASYNC_ALLOC_NOTIFY_WRITE            2013
#define IDC_ASYNC_ALLOC_NOTIFY_LOCK             2014

// free address range
#define IDC_ASYNC_FREE_ADDRESS_HANDLE           2050

// async read
#define IDC_ASYNC_READ_USE_BUS_NODE_NUMBER      2100
#define IDC_ASYNC_READ_BUS_NUMBER               2101
#define IDC_ASYNC_READ_NODE_NUMBER              2102
#define IDC_ASYNC_READ_OFFSET_HIGH              2103
#define IDC_ASYNC_READ_OFFSET_LOW               2104
#define IDC_ASYNC_READ_FLAG_NON_INCREMENT       2105
#define IDC_ASYNC_READ_BYTES_TO_READ            2106
#define IDC_ASYNC_READ_BLOCK_SIZE               2107
#define IDC_ASYNC_READ_GET_GENERATION           2108
#define IDC_ASYNC_READ_GENERATION_COUNT         2109

// async write
#define IDC_ASYNC_WRITE_USE_BUS_NODE_NUMBER     2150
#define IDC_ASYNC_WRITE_BUS_NUMBER              2151
#define IDC_ASYNC_WRITE_NODE_NUMBER             2152
#define IDC_ASYNC_WRITE_OFFSET_HIGH             2153
#define IDC_ASYNC_WRITE_OFFSET_LOW              2154
#define IDC_ASYNC_WRITE_FLAG_NON_INCRMENT       2155
#define IDC_ASYNC_WRITE_FLAG_NO_STATUS          2156
#define IDC_ASYNC_WRITE_BYTES_TO_WRITE          2157
#define IDC_ASYNC_WRITE_BLOCK_SIZE              2158
#define IDC_ASYNC_WRITE_GET_GENERATION          2159
#define IDC_ASYNC_WRITE_GENERATION_COUNT        2160

// async lock
#define IDC_ASYNC_LOCK_USE_BUS_NODE_NUMBER      2200
#define IDC_ASYNC_LOCK_BUS_NUMBER               2201
#define IDC_ASYNC_LOCK_NODE_NUMBER              2202
#define IDC_ASYNC_LOCK_OFFSET_HIGH              2203
#define IDC_ASYNC_LOCK_OFFSET_LOW               2204
#define IDC_ASYNC_LOCK_32BIT                    2205
#define IDC_ASYNC_LOCK_64BIT                    2206
#define IDC_ASYNC_LOCK_ARGUMENT1                2207
#define IDC_ASYNC_LOCK_ARGUMENT2                2208
#define IDC_ASYNC_LOCK_DATA1                    2209
#define IDC_ASYNC_LOCK_DATA2                    2210
#define IDC_ASYNC_LOCK_GET_GENERATION           2211
#define IDC_ASYNC_LOCK_GENERATION_COUNT         2212
#define IDC_ASYNC_LOCK_MASK_SWAP                2213
#define IDC_ASYNC_LOCK_COMPARE_SWAP             2214
#define IDC_ASYNC_LOCK_LITTLE_ADD               2215
#define IDC_ASYNC_LOCK_FETCH_ADD                2216
#define IDC_ASYNC_LOCK_BOUNDED_ADD              2217
#define IDC_ASYNC_LOCK_WRAP_ADD                 2218

// async stream
#define IDC_ASYNC_STREAM_BYTES_TO_STREAM        2250
#define IDC_ASYNC_STREAM_CHANNEL                2251
#define IDC_ASYNC_STREAM_TAG                    2252
#define IDC_ASYNC_STREAM_SYNCH                  2253
#define IDC_ASYNC_STREAM_100MBPS                2254
#define IDC_ASYNC_STREAM_200MBPS                2255
#define IDC_ASYNC_STREAM_400MBPS                2256
#define IDC_ASYNC_STREAM_1600MBPS               2257
#define IDC_ASYNC_STREAM_FASTEST                2258

// async loopback
#define IDC_ASYNC_LOOP_READ                     2300
#define IDC_ASYNC_LOOP_WRITE                    2301
#define IDC_ASYNC_LOOP_LOCK                     2302
#define IDC_ASYNC_LOOP_RANDOM_SIZE              2303
#define IDC_ASYNC_LOOP_OFFSET_HIGH              2304
#define IDC_ASYNC_LOOP_OFFSET_LOW               2305
#define IDC_ASYNC_LOOP_MAX_BYTES                2306
#define IDC_ASYNC_LOOP_ITERATIONS               2307

// async stream loopback
#define IDC_ASYNC_STREAM_LOOP_CHANNEL           2350
#define IDC_ASYNC_STREAM_LOOP_BYTES_TO_STREAM   2351
#define IDC_ASYNC_STREAM_LOOP_SYNCH             2352
#define IDC_ASYNC_STREAM_LOOP_TAG               2353
#define IDC_ASYNC_STREAM_LOOP_ITERATIONS        2354
#define IDC_ASYNC_STREAM_LOOP_100MBPS           2355
#define IDC_ASYNC_STREAM_LOOP_200MBPS           2356
#define IDC_ASYNC_STREAM_LOOP_400MBPS           2357
#define IDC_ASYNC_STREAM_LOOP_1600MBPS          2358
#define IDC_ASYNC_STREAM_LOOP_FASTEST           2359
#define IDC_ASYNC_STREAM_LOOP_FLAGS             2360

//
//  Isoch Commands
//

// isoch allocate bandwidth
#define IDC_ALLOC_BW_BYTES_PER_FRAME            3000
#define IDC_ALLOC_BW_100MBPS                    3001
#define IDC_ALLOC_BW_200MBPS                    3002
#define IDC_ALLOC_BW_400MBPS                    3003
#define IDC_ALLOC_BW_1600MBPS                   3004
#define IDC_ALLOC_BW_FASTEST                    3005

// isoch allocate channel
#define IDC_ALLOC_CHAN_REQUESTED_CHANNEL        3050

// isoch allocate resources
#define IDC_ISOCH_ALLOC_RES_CHANNEL             3100
#define IDC_ISOCH_ALLOC_RES_BYTES_PER_FRAME     3101
#define IDC_ISOCH_ALLOC_RES_NUM_OF_BUFFERS      3102
#define IDC_ISOCH_ALLOC_RES_MAX_BUFFER_SIZE     3103
#define IDC_ISOCH_ALLOC_RES_QUADLETS_TO_STRIP   3104
#define IDC_ISOCH_ALLOC_RES_100MBPS             3105
#define IDC_ISOCH_ALLOC_RES_200MBPS             3106
#define IDC_ISOCH_ALLOC_RES_400MBPS             3107
#define IDC_ISOCH_ALLOC_RES_1600MBPS            3108
#define IDC_ISOCH_ALLOC_RES_FASTEST             3109
#define IDC_ISOCH_ALLOC_RES_USED_IN_LISTEN      3110
#define IDC_ISOCH_ALLOC_RES_USED_IN_TALK        3111
#define IDC_ISOCH_ALLOC_RES_BUFFERS_CIRCULAR    3112
#define IDC_ISOCH_ALLOC_RES_STRIP_QUADLETS      3113
#define IDC_ISOCH_ALLOC_RES_SYNC_ON_TIME        3114
#define IDC_ISOCH_ALLOC_RES_USE_PACKET_BASED    3115

// isoch attach buffers
#define IDC_ISOCH_ATTACH_RESOURCE               3150
#define IDC_ISOCH_ATTACH_NUM_DESCRIPTORS        3151
#define IDC_ISOCH_ATTACH_LENGTH                 3152
#define IDC_ISOCH_ATTACH_TAG_VALUE              3153
#define IDC_ISOCH_ATTACH_CYCLE_OFFSET           3154
#define IDC_ISOCH_ATTACH_SECOND_COUNT           3155
#define IDC_ISOCH_ATTACH_BYTES_PER_FRAME        3156
#define IDC_ISOCH_ATTACH_SYNCH_VALUE            3157
#define IDC_ISOCH_ATTACH_CYCLE_COUNT            3158
#define IDC_ISOCH_ATTACH_SYNCH_ON_SY            3159
#define IDC_ISOCH_ATTACH_SYNCH_ON_TAG           3160
#define IDC_ISOCH_ATTACH_SYNCH_ON_TIME          3161
#define IDC_ISOCH_ATTACH_USE_SY_TAG_IN_FIRST    3162
#define IDC_ISOCH_ATTACH_TIME_STAMP_COMPLETE    3163
#define IDC_ISOCH_ATTACH_PRI_TIME_DELIVERY      3164
#define IDC_ISOCH_ATTACH_USE_CALLBACK           3165

// isoch detach buffers
#define IDC_ISOCH_DETACH_RESOURCE               3200
#define IDC_ISOCH_DETACH_NUM_DESCRIPTORS        3201
#define IDC_ISOCH_DETACH_ISOCH_DESCRIPTOR       3202

// isoch free bandwidth
#define IDC_ISOCH_FREE_BW_HANDLE                3250

// isoch free channel
#define IDC_ISOCH_FREE_CHAN_CHANNEL             3300

// isoch free resources
#define IDC_FREE_RES_RESOURCE_HANDLE            3350

// isoch listen
#define IDC_ISOCH_LISTEN_RESOURCE_HANDLE        3400
#define IDC_ISOCH_LISTEN_CYCLE_OFFSET           3401
#define IDC_ISOCH_LISTEN_CYCLE_COUNT            3402
#define IDC_ISOCH_LISTEN_SECOND_COUNT           3403

// isoch query resources
#define IDC_ISOCH_QUERY_RES_100MBPS             3450
#define IDC_ISOCH_QUERY_RES_200MBPS             3451
#define IDC_ISOCH_QUERY_RES_400MBPS             3452
#define IDC_ISOCH_QUERY_RES_1600MBPS            3453
#define IDC_ISOCH_QUERY_RES_FASTEST             3454

// isoch set channel bandwidth
#define IDC_ISOCH_SET_CHAN_BW_RESOURCE          3500
#define IDC_ISOCH_SET_CHAN_BW_BYTES_PER_FRAME   3501

// isoch stop
#define IDC_ISOCH_STOP_HANDLE                   3550

// isoch talk
#define IDC_ISOCH_TALK_RESOURCE_HANDLE          3600
#define IDC_ISOCH_TALK_CYCLE_OFFSET             3601
#define IDC_ISOCH_TALK_CYCLE_COUNT              3602
#define IDC_ISOCH_TALK_SECOND_COUNT             3603

// isoch loopback
#define IDC_ISOCH_LOOP_LISTEN                   3650
#define IDC_ISOCH_LOOP_TALK                     3651
#define IDC_ISOCH_LOOP_CIRCULAR                 3652
#define IDC_ISOCH_LOOP_STRIP_QUADS              3653
#define IDC_ISOCH_LOOP_SYNC_TIME                3654
#define IDC_ISOCH_LOOP_PACKET_BASED             3655
#define IDC_ISOCH_LOOP_RESOURCE                 3656
#define IDC_ISOCH_LOOP_NUM_DESCRIPTORS          3657
#define IDC_ISOCH_LOOP_LENGTH                   3658
#define IDC_ISOCH_LOOP_TAG_VALUE                3659
#define IDC_ISOCH_LOOP_CYCLE_OFFSET             3660
#define IDC_ISOCH_LOOP_SECOND_COUNT             3661
#define IDC_ISOCH_LOOP_BYTES_PER_FRAME          3662
#define IDC_ISOCH_LOOP_SYNCH_VALUE              3663
#define IDC_ISOCH_LOOP_CYCLE_COUNT              3664
#define IDC_ISOCH_LOOP_SYNCH_ON_SY              3665
#define IDC_ISOCH_LOOP_SYNCH_ON_TAG             3666
#define IDC_ISOCH_LOOP_SYNCH_ON_TIME            3667
#define IDC_ISOCH_LOOP_USE_SY_TAG_IN_FIRST      3668
#define IDC_ISOCH_LOOP_TIME_STAMP_COMPLETE      3669
#define IDC_ISOCH_LOOP_PRI_TIME_DELIVERY        3670
#define IDC_ISOCH_LOOP_USE_CALLBACK             3671


