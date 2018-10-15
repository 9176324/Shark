#include "stdio.h"
#include "stdlib.h"
#include <string.h>

typedef unsigned long ULONG;
typedef long                 LONG;
typedef int                 BOOL;
typedef unsigned char       BYTE;



#define  cMAX_LOSpurHarmonic   5
#define WRITE             "w"


FILE *gpFILE_SourceFile;


ULONG  gaVCO_FREQS[5][2] =
{
    { 1890000, 1790000 },    //  VCO 1 lower limit
    { 1720000, 1617000 },    //  VCO 2 lower limit
    { 1530000, 1454000 },    //  VCO 3 lower limit
    { 1370000, 1291000 },    //  VCO 4 lower limit
    { 0,       0       }     //  VCO 5 lower limit
};

typedef struct  TunerVars
{
    ULONG     ulDesiredLO1;   // kHz!
    ULONG     ulLO1;
    ULONG     ulLO1Freq;      // kHz!
    ULONG     ulDesiredLO2;   // kHz
    ULONG     ulLO2;
    ULONG     ulNUM;
        
} TUNERVARS, *PTUNERVARS;

typedef struct  MixerParam
{
    ULONG     ulTunerID;            // Table index
    ULONG     ulTunerMode;          // Radio, TV
    ULONG     ulTunerStandard;      // PAL_X , SECAM_Y, NTSC_Z
    ULONG     ulFrequency;          // actual wanted frequency
    ULONG     ulLastFrequency;      // last good know frequency
    ULONG     ulRefFreq;            // reference frequency == 5,25 MHz
    ULONG     ulIF1;                // input intermediate frequency == 1090 MHz
    ULONG     ulIF2;                // output intermediate frequency ( depends on FM/TV and country)
    ULONG     ulTuningGranularity;  // output TuningGranularity; ( depends on FM/TV and country)

} MIXERPARAM, *PMIXERPARAM;

MIXERPARAM  mParams;
TUNERVARS mTuning;

unsigned char gucString[50];

void WriteByte(BYTE x, BYTE y)  
{	
	sprintf(gucString, "%d,",y);
	printf("Address: 0x%X Value: 0x%X  %d\n", x, y, y);
	fputs(gucString, gpFILE_SourceFile);
}


BOOL SpurCheck(void)
{
    LONG  nLO1       = 1;
    LONG  lBandWidth = 0;


    lBandWidth   = 6000;

    //
    //  Start out with fLO1 - 2fLO2 - fIF2
    //  Add LO1 harmonics until MAX_LOSpurHarmonic is reached.
    do
    {
        LONG  nLO2 = -nLO1;
        LONG  lSpur = nLO1 * (mTuning.ulLO1Freq - mTuning.ulDesiredLO2);

        //  Subtract LO2 harmonics until fSpur is below IF2 band pass
        do
        {
            --nLO2;
            lSpur -= mTuning.ulDesiredLO2;

            if (abs(abs(lSpur) - mParams.ulIF2) < (lBandWidth / 2))
            {
                printf( "Leaving IsSpurCheck() with  true.\n");
                return 1;
            }
			printf("In SpurCheck\n");
        }
        while ( ( lSpur > nLO2 - (LONG)mTuning.ulDesiredLO2 - (lBandWidth / 2) ) && 
                ( nLO2  > - cMAX_LOSpurHarmonic) );
    }
    while ( ++nLO1 < cMAX_LOSpurHarmonic );

    return 0;
}

BYTE  SelectVCO( ULONG  fInFreq, unsigned int fAdjustVCO )
{
    BYTE  bSel;
    BYTE  bCol = (fAdjustVCO == 0) ? 1 : 0;

    for (bSel=0; bSel<5; bSel++)
    {
        if ( fInFreq > gaVCO_FREQS[bSel][bCol] )
            break;
    }

    //DebugOut((0, "EmTyV::SelectVCO returns %d \n", bSel ));
    return bSel;
}


int  SetFrequency ( ULONG  ulFrequency )
{
    int  Status = 0;
    BYTE      bVCO   = 0;
	long  lLO1Adjust;

	ULONG  ulLockCheck = 0;
    BOOL   fStatus     = 0;
    BYTE   bLock       = 0;
    BYTE   bData       = 0;
    //
    // turn input freqency to kHz
    //
    if ( ulFrequency )
        ulFrequency /= 1000;

    if ( 0 == mParams.ulRefFreq )
    {
        printf("EmTyV::SetFrequency got RefFreq == 0\n" );
        return  1; //Status;
    }

    if ( (38000 <= ulFrequency) && (863250 >= ulFrequency) )
        mParams.ulFrequency = ulFrequency;
	else
    {
        printf("EmTyV::SetFrequency got invalid Freq %d in Mode %d\n", ulFrequency, mParams.ulTunerMode );
        return  1; //Status;
    }

    // TunerAccessViaTDA( TRUE );
    //
    // ok, frequency is valid...  calculate LO1
    //
    mTuning.ulDesiredLO1 = mParams.ulFrequency + mParams.ulIF1 ;         // kHz!
    mTuning.ulLO1        = (mTuning.ulDesiredLO1 + mParams.ulRefFreq/2) / mParams.ulRefFreq;
    mTuning.ulLO1Freq    = (mTuning.ulLO1) * mParams.ulRefFreq;          // kHz!

    printf( "ulDesiredLO1 (Input): %d\nulLO1: %d\nLo1Freq: %d\n", mTuning.ulDesiredLO1, mTuning.ulLO1,  mTuning.ulLO1Freq);
    //
    // calculate LO2 frequency
    //
	printf("Before Spur Check\n");

    mTuning.ulDesiredLO2 = mTuning.ulLO1Freq - mParams.ulFrequency - mParams.ulIF2;   // kHz!
    //
    // spur check
    //

    for ( lLO1Adjust= 1; (lLO1Adjust < 4) && SpurCheck(); lLO1Adjust++)
    {
        mTuning.ulLO1       += (mTuning.ulDesiredLO1 > mTuning.ulLO1Freq) ? lLO1Adjust : -lLO1Adjust;
        mTuning.ulLO1Freq    = (mTuning.ulLO1) * mParams.ulRefFreq;                        // kHz!
        mTuning.ulDesiredLO2 =  mTuning.ulLO1Freq - mParams.ulFrequency - mParams.ulIF2;   // kHz!
        //DebugOut((0, "EmTyV::SetFrequency  LO1Freq %d  ulLO1 %d  LO2Freq %d\n", mTuning.ulLO1Freq, mTuning.ulLO1,  mTuning.ulDesiredLO2));
    }

	printf( "ulDesiredLO1 (Input): %d\nulLO1: %d\nLo1Freq: %d\n", mTuning.ulDesiredLO1, mTuning.ulLO1,  mTuning.ulLO1Freq);
	printf("After Spur Check\n");
    //
    // select vco range
    //
    bVCO = SelectVCO( mTuning.ulLO1Freq, 0 );
    
    mTuning.ulLO2        = mTuning.ulDesiredLO2 / mParams.ulRefFreq;
    mTuning.ulNUM        = ((mTuning.ulDesiredLO2 % mParams.ulRefFreq) * 3780)/mParams.ulRefFreq;
    //
    // calculate to stepping...?
    //
    printf( "EmTyV::SetFrequency DLO2 %d  Num %d If2 = %d\n", mTuning.ulDesiredLO2, mTuning.ulNUM, mParams.ulIF2);
    if ( 62500 == mParams.ulTuningGranularity )
        mTuning.ulNUM = 45*(( mTuning.ulNUM + 45/2 )/45);   // TV
    else// if ( 50000 == mParams.ulTuningGranularity )
        mTuning.ulNUM = 36*(( mTuning.ulNUM + 36/2 )/36);   // TV
    //else
    //    mTuning.ulNUM = 9*(( mTuning.ulNUM + 9/2 )/9);      // FM
    //DebugOut((0, "EmTyV::SetFrequency DLO2 %d  Num recalculated %d\n", mTuning.ulDesiredLO2, mTuning.ulNUM));
    //
    // write registers
    //
    WriteByte( 0x00, (BYTE)((mTuning.ulLO1>>3) -1 ) ) ;
    WriteByte( 0x01, (BYTE)((bVCO <<4) | (mTuning.ulLO1&0x7)) );
    WriteByte( 0x02, 0x86);
    WriteByte( 0x05, (BYTE)((mTuning.ulLO2<<5) | ((mTuning.ulLO2>>3) -1 )) );
    WriteByte( 0x06, (BYTE) ((mParams.ulFrequency < 400000) ? 0xE4 : 0xF4) );
    WriteByte( 0x07, 0x0F );
    WriteByte( 0x0B, (BYTE)(mTuning.ulNUM & 0xFF) );
    WriteByte( 0x0C, (BYTE)(0x80 | ((mTuning.ulNUM>>8) & 0x0F) ) );
	
	// LO1 Bit 7 - 3
	// VCO Bit 7 - 3 | LO1 Bit 2 - 0        VCO 0 - 5

    
/*
    do
    {
        fStatus = AdjustVCO();
        //
        //  If we still haven't locked, we need to try raising the SRO
        //  loop gain control voltage in case it's fallen too low.
        //
        ReadByte( 0x0E, &bLock);
        
        if ( fStatus  && ( 0x06 != (bLock & 0x06)) )
        {
            ReadByte( 0x07, &bData);
            bData |= 0x80;         //  Turn charge pump on
            fStatus = WriteByte( 0x07, bData);

            //DebugOut((0, "Waiting 10 msec before turning off charge pumps...\n"));
            mpDevice->tuner.Wait(10000);

            bData  &= 0x0F;        //  Turn charge pump back off
            fStatus = WriteByte( 0x07, bData);
        }
    }
    while ( fStatus  && ( 0x06 != (bLock & 0x06)) && (++ulLockCheck < 2));

    WriteByte( 0x02, 0x20 );
    TunerAccessViaTDA( FALSE );*/
    return  1;
}



void main (void)
{
	long lValue ;
	long lStart = 50000000;
	long lEnd =  800000000;
	long lStep =    250000;

    mParams.ulFrequency         = 1;
    mParams.ulLastFrequency     = 1;
    mParams.ulRefFreq           = 5250;
    mParams.ulTunerID           = 1;
    mParams.ulIF1               = 1090000;
    mParams.ulIF2               = 38900L;
    mParams.ulTunerMode         = 0;
    mParams.ulTuningGranularity = 62500;
    mTuning.ulDesiredLO1        = 1;
    mTuning.ulLO1               = 1;
    mTuning.ulLO1Freq           = 1;
    mTuning.ulDesiredLO2        = 1;
    mTuning.ulLO2               = 1;
    mTuning.ulNUM               = 1;
	
	gpFILE_SourceFile = fopen("c:\\test.csv", WRITE);

	sprintf(gucString, "0x00, 0x01, 0x02, 0x05,0x06, 0x07, 0x0b, 0x0c\n");
	fputs(gucString, gpFILE_SourceFile);

	lValue = lStart;
	do
	{
		printf("Frequency [MHz]: " );
		//scanf("%d", &value);
		lValue += lStep;		
		printf("Set %d Hz\n", lValue);
		sprintf(gucString, "%d,", lValue);
		fputs(gucString, gpFILE_SourceFile);
		SetFrequency ( lValue );
		sprintf(gucString, "\n");
		fputs(gucString, gpFILE_SourceFile);
	}
	while (lValue < lEnd);
	
	fclose(gpFILE_SourceFile);

}
