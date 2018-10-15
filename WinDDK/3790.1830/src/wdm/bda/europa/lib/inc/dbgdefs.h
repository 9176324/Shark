//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantibility or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// DbgDefs.h
//
// Macro definitions for per-class debug levels
//
//////////////////////////////////////////////////////////////////////////////

// WDM
#define C34PORTVIDEO             1
#define C34VIDEOSOURCE           2
#define CAP7134                  3
#define CAP7134ADAPTER           4
#define CAPTURE                  5
#define CAUDIOOUT                6
#define CBUILTVIDEOFORMAT        7
#define CCLIPTABLE               8
#define CDPC                     9
#define CKSANALOGAUDIOFORMAT     10
#define CKSANALOGIN              11
#define CKSANALOGOUT             12
#define CKSANALOGVIDEOFORMAT     13
#define CKSAUDIOFORMAT           14
#define CKSDATASTREAM            15
#define CKSDECODERIN             16
#define CKSDRIVER                17
#define CKSFORMAT                18
#define CKSIOCTL                 19
#define CKSSOURCE                20
#define CKSSRBLIST               21
#define CKSSTREAM                22
#define CKSSTREAMADAPTER         23
#define CKSSTREAMDRIVER          24
#define CKSTIMEOUT               25
#define CKSTRANSPORTIN           26
#define CKSVIDEOFORMAT           27
#define CKSVIDEOOUT              28
#define COSDEPEND                29
#define CREGKEY                  30
#define CROSSBAR                 31
#define CSAMPLERATECLOCK         32
#define CTRANSPORTFORMAT         33
#define CTRANSPORTOUT            34
#define CVBICLOCK                35
#define CVBIFORMAT               36
#define CVBIOUT                  37
#define CVIDEOCLOCK              38
#define CVIDEOCONTROLPROPERTIES  39
#define CVIDEOPINFORMAT          40
#define CVIDEOPREVIEW            41
#define CWDMLAYERMGR             42
#define CWDMOBJECTFACTORY        43
#define CWDMSYNCCALL             44
#define TVAUDIO                  45

// HAL
#define CVAMPBASECORESTREAM      101
#define CVAMPBUFFER              102
#define CVAMPCLIPPER             103
#define CVAMPCORESTREAM          104
#define CVAMPDECODER             105
#define CVAMPDECODERIRQ          106
#define CVAMPDEVICERESOURCES     107
#define CVAMPDMACHANNEL          108
#define CVAMPFIFOALLOC           109
#define CVAMPFIFOIRQ             110
#define CVAMPGPIOCTRL            111
#define CVAMPI2C                 112
#define CVAMPIO                  113
#define CVAMPIRQ                 114
#define CVAMPPANICMANAGER        115
#define CVAMPSCALER              116
#define CVAMPSTREAMMANAGER       117
#define CVAMPTRANSPORTSTREAM     118
#define CVAMPTRAPLOSTIRQRA1      119
#define CVAMPTRAPLOSTIRQRA2      120
#define CVAMPTRAPLOSTIRQRA3      121
#define CVAMPTRAPLOSTIRQRA4      122
#define CVAMPVBASICSTREAM        123
#define CVAMPVBISTREAM           124
#define CVAMPVIDEOSTREAM         125
#define CVAMPEVENTHANDLER        126

// Miscellaneous
#define DEBUGBREAKLEVEL          127
#define DEBUGLINENO              128
#define DEBUGIRQL                129

// Emulation
#define EMULATION_DECODER        200
#define EMULATION_DMA            201              
#define EMULATION_GPIO           202               
#define EMULATION_SC2DMA         203             
#define EMULATION_SCALER         204        
#define EMULATION_SUPPORT        205    
#define ENABLE_ALL               206

#define LAST_DEBUG_CLASS_ID      206
