/*++

Copyright (c) 1990-2003  Microsoft Corporation
All Rights Reserved

Abstract:

    Routines to facilitate printing of raw jobs.

--*/
#include "local.h"

#include <wchar.h>


BYTE abyFF[1] = { 0xc };


/*++
*******************************************************************
    P r i n t R a w J o b

    Routine Description:
        Prints out a job with RAW data type.

    Arguments:
        pData           => Print Processor data structure
        pPrinterName    => name of printer to print on

    Return Value:
        TRUE  if successful
        FALSE if failed - GetLastError will return reason
*******************************************************************
--*/

BOOL
PrintRawJob(
    IN PPRINTPROCESSORDATA pData,
    IN LPWSTR pPrinterName,
    IN UINT uDataType)

{
    DOC_INFO_1  DocInfo;
    DWORD       Copies;
    DWORD       NoRead, NoWritten;
    DWORD       i;
    BOOL        rc;
    HANDLE      hPrinter;
    BYTE        *ReadBuffer = NULL;
    BOOL        bRet        = FALSE;
    BOOL        bStartDoc   = FALSE;

    DocInfo.pDocName    = pData->pDocument;     /* Document name */
    DocInfo.pOutputFile = pData->pOutputFile;   /* Output file */
    DocInfo.pDatatype   = pData->pDatatype;     /* Document data type */

    /** Let the printer know we are starting a new document **/

    if (!StartDocPrinter(pData->hPrinter, 1, (LPBYTE)&DocInfo)) {
        goto Done;
    }

    bStartDoc = TRUE;


    /** Allocate the read buffer, dynamically allocated to conserve stack space **/

    ReadBuffer = AllocSplMem(READ_BUFFER_SIZE);

    if (!ReadBuffer) {
        goto Done;
    }

    /** Print the data pData->Copies times **/

    Copies = pData->Copies;

    while (Copies--) {

        /**
            Open the printer.  If it fails, return.  This also sets up the
            pointer for the ReadPrinter calls.
        **/

        if (!OpenPrinter(pPrinterName, &hPrinter, NULL)) {   
            goto Done;
        }

        /**
            Loop, getting data and sending it to the printer.  This also
            takes care of pausing and cancelling print jobs by checking
            the processor's status flags while printing.
        **/

        while ((rc = ReadPrinter(hPrinter, ReadBuffer, READ_BUFFER_SIZE, &NoRead)) &&
               NoRead) {


            /** If the print processor is paused, wait for it to be resumed **/

            if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {
                WaitForSingleObject(pData->semPaused, INFINITE);
            }

            /** If the job has been aborted, don't write anymore **/

            if (pData->fsStatus & PRINTPROCESSOR_ABORTED) {
                break;
            }

            /** Write the data to the printer **/

            WritePrinter(pData->hPrinter, ReadBuffer, NoRead, &NoWritten);
        }


        /**
            Close the printer - we open/close the printer for each
            copy so the data pointer will rewind.
        **/

        ClosePrinter(hPrinter);

    } /* While copies to print */

    bRet = TRUE;

Done:    
    
    /** Close the buffer we allocated **/

    if (ReadBuffer) {
        FreeSplMem(ReadBuffer);
    }
    
    /** Let the printer know that we are done printing **/

    if (bStartDoc) {
        EndDocPrinter(pData->hPrinter);
    }
    
    return bRet;
}

