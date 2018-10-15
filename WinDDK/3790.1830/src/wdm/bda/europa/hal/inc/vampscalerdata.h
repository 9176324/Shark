//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   VampScalerData | This module contains data tables to support scaling
// computations<nl>
// @end
// Filename: VampScalerData.h
//
// Routines/Functions:
//
//  public:
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPDATA_H__INCLUDED_)
#define AFX_VAMPDATA_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampTypes.h"

//@globalv 
// Reducing the frame rate by setting the Scaler RegField FSkip
// FSkip[1:0]: Fields to be processed   (3..1)
// FSkip[7:2]: Fields to be skipped     (63..0)
// FLOAT fps = xx.yy 
// PalFrameRates[][0]: xx part
// PalFrameRates[][1]: yy part
// single field:
// PalFrameRates[][2]: fields to be processed
// PalFrameRates[][3]: fields to be skipped
// interlaced field:
// PalFrameRates[][4]: fields to be processed
// PalFrameRates[][5]: fields to be skipped

static WORD PalFrameRates[32][6] = {
     0, 781, 1, 62,  2, 62,   //  0
     0, 806, 1, 60,  2, 60,   //  1
     0, 833, 1, 58,  2, 58,   //  2
     0, 862, 1, 56,  2, 56,   //  3
     0, 892, 1, 54,  2, 54,   //  4
     0, 925, 1, 52,  2, 52,   //  5
     0, 961, 1, 50,  2, 50,   //  6
     1,   0, 1, 48,  2, 48,   //  7
     1,  41, 1, 46,  2, 46,   //  8
     1,  86, 1, 44,  2, 44,   //  9
     1, 136, 1, 42,  2, 42,   // 10
     1, 190, 1, 40,  2, 40,   // 11
     1, 250, 1, 38,  2, 38,   // 12
     1, 315, 1, 36,  2, 36,   // 13
     1, 388, 1, 34,  2, 34,   // 14
     1, 470, 1, 32,  2, 32,   // 15
     1, 562, 1, 30,  2, 30,   // 16
     1, 666, 1, 28,  2, 28,   // 17
     1, 785, 1, 26,  2, 26,   // 18
     1, 923, 1, 24,  2, 24,   // 19
     2,  83, 1, 22,  2, 22,   // 20
     2, 272, 1, 20,  2, 20,   // 21
     2, 500, 1, 18,  2, 18,   // 22
     2, 777, 1, 16,  2, 16,   // 23
     3, 125, 1, 14,  2, 14,   // 24
     3, 571, 1, 12,  2, 12,   // 25
     4, 166, 1, 10,  2, 10,   // 26
     5,   0, 1,  8,  2,  8,   // 27
     6, 250, 1,  6,  2,  6,   // 28
     8, 333, 1,  4,  2,  4,   // 29
    12, 500, 1,  2,  2,  2,   // 30
    25,   0, 1,  0,  2,  0 }; // 31

//@globalv 
// Find fast the correct FSkip parameters for a given fps
// eg: fps 20.0 ---> PalFindFrameRates[20][]
// --> next close is PalFrameRates[64][]
// --> fps 18.750, 3 fields processed, 1 field skipped
// eg: fps 2.57 ---> PalFindFrameRates[2][]
// --> next close is range PalFrameRates[52-55][]
// --> fps 2.500, 1 fields processed, 9 field skipped (PalFrameRates[54][])

static WORD PalFindFrameRates[26][2] = {
     0,  7,       //  0.xx
     7, 20,       //  1.xx
    20, 24,       //  2.xx
    24, 26,       //  3.xx
    26, 27,       //  4.xx
    27, 28,       //  5.xx
    28, 29,       //  6.xx
    29, 29,       //  7.xx
    29, 30,       //  8.xx
    30, 30,       //  9.xx
    30, 30,       // 10.xx
    30, 30,       // 11.xx
    30, 31,       // 12.xx
    31, 31,       // 13.xx
    31, 31,       // 14.xx
    31, 31,       // 15.xx
    31, 31,       // 16.xx
    31, 31,       // 17.xx
    31, 31,       // 18.xx
    31, 31,       // 19.xx
    31, 31,       // 20.xx
    31, 31,       // 21.xx   
    31, 31,       // 22.xx   
    31, 31,       // 23.xx   
    31, 31,       // 24.xx   
    31, 31  };    // 25.0


static WORD NtscFrameRates[32][6] = {
     0, 937, 1, 62,  2, 62,   //  0
     0, 967, 1, 60,  2, 60,   //  1
     1,   0, 1, 58,  2, 58,   //  2
     1,  34, 1, 56,  2, 56,   //  3
     1,  71, 1, 54,  2, 54,   //  4
     1, 111, 1, 52,  2, 52,   //  5
     1, 153, 1, 50,  2, 50,   //  6
     1, 200, 1, 48,  2, 48,   //  7
     1, 250, 1, 46,  2, 46,   //  8
     1, 304, 1, 44,  2, 44,   //  9
     1, 363, 1, 42,  2, 42,   // 10
     1, 428, 1, 40,  2, 40,   // 11
     1, 500, 1, 38,  2, 38,   // 12
     1, 578, 1, 36,  2, 36,   // 13
     1, 666, 1, 34,  2, 34,   // 14
     1, 764, 1, 32,  2, 32,   // 15
     1, 875, 1, 30,  2, 30,   // 16
     2,   0, 1, 28,  2, 28,   // 17
     2, 142, 1, 26,  2, 26,   // 18
     2, 307, 1, 24,  2, 24,   // 19
     2, 500, 1, 22,  2, 22,   // 20
     2, 727, 1, 20,  2, 20,   // 21
     3,   0, 1, 18,  2, 18,   // 22
     3, 333, 1, 16,  2, 16,   // 23
     3, 750, 1, 14,  2, 14,   // 24
     4, 285, 1, 12,  2, 12,   // 25
     5,   0, 1, 10,  2, 10,   // 26
     6,   0, 1,  8,  2,  8,   // 27
     7, 500, 1,  6,  2,  6,   // 28
    10,   0, 1,  4,  2,  4,   // 29
    15,   0, 1,  2,  2,  2,   // 30
    30,   0, 1,  0,  2,  0 }; // 31

static WORD NtscFindFrameRates[31][2] = {
     0,  2,       //  0.xx
     2, 17,       //  1.xx
    17, 22,       //  2.xx
    22, 25,       //  3.xx
    25, 26,       //  4.xx
    26, 27,       //  5.xx
    27, 28,       //  6.xx
    28, 29,       //  7.xx
    29, 29,       //  8.xx
    29, 29,       //  9.xx
    29, 30,       // 10.xx
    30, 30,       // 11.xx
    30, 30,       // 12.xx
    30, 30,       // 13.xx
    30, 30,       // 14.xx
    30, 31,       // 15.xx
    31, 31,       // 16.xx
    31, 31,       // 17.xx
    31, 31,       // 18.xx
    31, 31,       // 19.xx
    31, 31,       // 20.xx
    31, 31,       // 21.xx   
    31, 31,       // 22.xx   
    31, 31,       // 23.xx   
    31, 31,       // 24.xx   
    31, 31,       // 25.xx   
    31, 31,       // 26.xx   
    31, 31,       // 27.xx   
    31, 31,       // 28.xx   
    31, 31,       // 29.xx   
    31, 31  };    // 30.0


static const tScalingValues m_tScalingValues[] = {
    { 0, 0, 0, 0 },   //  0
    { 0, 0, 0, 0 },   //  1
    { 1, 1, 1, 0 },   //  2
    { 3, 0, 2, 2 },   //  3
    { 7, 0, 3, 2 },   //  4
    { 7, 0, 3, 2 },   //  5
    { 8, 1, 4, 3 },   //  6
    { 8, 1, 4, 3 },   //  7
    {15, 0, 4, 3 },   //  8
    {15, 0, 4, 3 },   //  9
    {16, 1, 5, 3 },   // 10
    {16, 1, 5, 3 },   // 11 --> Not Documented
    {16, 1, 5, 3 },   // 12 --> Not Documented
    {16, 1, 5, 3 },   // 13 
    {16, 1, 5, 3 },   // 14 --> Not Documented
    {16, 1, 5, 3 },   // 15
    {31, 0, 5, 3 },   // 16
    {32, 1, 6, 3 },   // 17 --> Not Documented
    {32, 1, 6, 3 },   // 18 --> Not Documented
    {32, 1, 6, 3 },   // 19  
    {32, 1, 6, 3 },   // 20 --> Not Documented
    {32, 1, 6, 3 },   // 21 --> Not Documented
    {32, 1, 6, 3 },   // 22 --> Not Documented
    {32, 1, 6, 3 },   // 23 --> Not Documented
    {32, 1, 6, 3 },   // 24 --> Not Documented
    {32, 1, 6, 3 },   // 25 --> Not Documented
    {32, 1, 6, 3 },   // 26 --> Not Documented
    {32, 1, 6, 3 },   // 27 --> Not Documented
    {32, 1, 6, 3 },   // 28 --> Not Documented
    {32, 1, 6, 3 },   // 29 --> Not Documented
    {32, 1, 6, 3 },   // 30 --> Not Documented
    {32, 1, 6, 3 },   // 31
    {63, 1, 7, 3 },   // 32
    {63, 1, 7, 3 },   // 33 --> Not Documented
    {63, 1, 7, 3 },   // 34 --> Not Documented
    {63, 1, 7, 3 },   // 35
    {63, 1, 7, 3 },   // 36
    {63, 1, 7, 3 },   // 37
    {63, 1, 7, 3 },   // 38
    {63, 1, 7, 3 },   // 39
    {63, 1, 7, 3 },   // 40
    {63, 1, 7, 3 },   // 41
    {63, 1, 7, 3 },   // 42
    {63, 1, 7, 3 },   // 43
    {63, 1, 7, 3 },   // 44
    {63, 1, 7, 3 },   // 45
    {63, 1, 7, 3 },   // 46
    {63, 1, 7, 3 },   // 47
    {63, 1, 7, 3 },   // 48
    {63, 1, 7, 3 },   // 49
    {63, 1, 7, 3 },   // 50
    {63, 1, 7, 3 },   // 51
    {63, 1, 7, 3 },   // 52
    {63, 1, 7, 3 },   // 53
    {63, 1, 7, 3 },   // 54
    {63, 1, 7, 3 },   // 55
    {63, 1, 7, 3 },   // 56
    {63, 1, 7, 3 },   // 57
    {63, 1, 7, 3 },   // 58
    {63, 1, 7, 3 },   // 59
    {63, 1, 7, 3 },   // 60
    {63, 1, 7, 3 },   // 61
    {63, 1, 7, 3 },   // 62
    {63, 1, 7, 3 }    // 63
};
 


#endif // !defined(AFX_VAMPDATA_H__INCLUDED_)
