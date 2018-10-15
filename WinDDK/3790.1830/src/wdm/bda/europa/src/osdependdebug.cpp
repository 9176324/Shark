//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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

#include "34AVStrm.h"
#include "OSDepend.h"

#if DBG //remove code in the release build

//
// Description:
//  Global debug object.
//
COSDependDebug* g_pDebugObject = 0;
//
// Description:
//  Global debug file handle.
//
int g_hDebFileHandle;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor, stores a registry access object and calls
//  OnDebugLevelChange() which gets all debuglevels from the registry.
//
//////////////////////////////////////////////////////////////////////////////
COSDependDebug::COSDependDebug
(
    COSDependRegistry* pRegistryAccess //registry access object
)
{
    //store registry access object
    m_pRegistryAccess = pRegistryAccess;
    //store registry access object
        g_hDebFileHandle = NULL;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: No debug file available"));
    //get all debuglevels from the registry
    OnDebugLevelChange();
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor, does nothing special.
//
//////////////////////////////////////////////////////////////////////////////
COSDependDebug::~COSDependDebug()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Prints a sourcefile name and line of calling into a debug string prolog.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void COSDependDebug::Debug_Print_Prolog
(
    PCHAR pszFileName,  //Pointer to a string that includes the name of
                        //the sourcefile (Length must be <= MAX_PATH)
    int nLine           //line of calling
)
{
    //print sourcefile name and line of calling into the debug string prolog
    sprintf(
        m_pszDebugProlog,
        "SAA7134: File-%s, Line-%d\n",
        pszFileName,
        nLine);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Sends Debug Message (and prolog) with a variable argument list to the
//  Softice Debugger.
//
// Remarks:
//  Current implementation is not sufficent to support debug level handling.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void COSDependDebug::Debug_Print
(
    PCHAR pszDebugString,   //Pointer to Null terminated String (contains
                            //debug message in a format that is similar to
                            //ANSI printf)
    ...                     //variable argument list
)
{
    //declare variable to be an argument list pointer
    va_list ap;
    //set pointer to the beginning of the argument list.
    va_start(ap, pszDebugString);

    //print the variable arguments into the debug string
    vsprintf(m_pszDebugString, pszDebugString, ap);
    //print the debug prolog
    //_DbgPrintF(DEBUGLVL_BLAB,(m_pszDebugProlog));
//  int nLength = strlen(m_pszDebugProlog);
//  i_write(g_hDebFileHandle, m_pszDebugProlog, nLength);
    //print the debug message
    //_DbgPrintF(DEBUGLVL_BLAB,(m_pszDebugString));
//  nLength = strlen(m_pszDebugString);
//  i_write(g_hDebFileHandle, m_pszDebugString, nLength);

    //reset the argument pointer
    va_end(ap);
}


void COSDependDebug::Debug_Print
(
    BYTE ucDebugLevel,      //Debug Level that has to be lower or equal to the
                            //current class debug level to print out the debug
                            //message
    PCHAR pszDebugString,   //Pointer to Null terminated String (contains debug
                            //message in a format that is similar to ANSI
                            //printf)
    ...                     //variable argument list
)
{
//    if(ucDebugLevel <= m_ucDebugLevel[m_dwClassID])
//    {
//        declare variable to be an argument list pointer
//        va_list ap;
//        set pointer to the beginning of the argument list.
//        va_start(ap, pszDebugString);
//
//        print the variable arguments into the debug string
//        vsprintf(m_pszDebugString, pszDebugString, ap);
//        print the debug prolog
//        _DbgPrintF(DEBUGLVL_BLAB,(m_pszDebugProlog));
//      print the debug message
//        _DbgPrintF(DEBUGLVL_BLAB,(m_pszDebugString));
//        reset the argument pointer
//        va_end(ap);
//    }
//  else
    if(ucDebugLevel == 0)
    {
        //declare variable to be an argument list pointer
        va_list ap;
        //set pointer to the beginning of the argument list.
        va_start(ap, pszDebugString);

        //print the variable arguments into the debug string
        vsprintf(m_pszDebugString, pszDebugString, ap);
        //
        _DbgPrintF(DEBUGLVL_ERROR,(m_pszDebugString));
        //reset the argument pointer
        va_end(ap);

    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Set a Breakpoint if the debug level of this breakpoint is lower than or
//  equal to the global debug break level,
//
//
// Return Value:
//  void
//
//////////////////////////////////////////////////////////////////////////////
void COSDependDebug::Debug_Break
(
    BYTE ucDebugBreakLevel //The debug break level
)
{
    //check if the debug level of this breakpoint is lower than or equal to
    //      the global debug break level
    if(ucDebugBreakLevel <= m_ucDebugLevel[DEBUGBREAKLEVEL])
    {
        //set breakpoint
        _DbgPrintF(DEBUGLVL_ERROR,("Automatic debug break HAL"));
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Actualizes debug levels for all registered class IDs, it is not allowed to
//  call this function during interrupt processing.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void COSDependDebug::OnDebugLevelChange()
{
//    CHAR pszValueName[10];  //10 is the maximum size of integer numbers converted
//                            //to a string
//    DWORD dwTargetBuffer;
//    //get actual debug levels from the registry
//    for(int i = 0;i<=LAST_DEBUG_CLASS_ID;i++)
//    {
//      //Debug_Printf_Service("i = %d, dwTargetBuffer = %d \n", i, dwTargetBuffer);
//        //convert the number into a registry value name
//        _itoa(i, pszValueName, 10);//10 is the basis for the converted number
//        //read the registry debug levels into the dwTargetBuffer variable
//        m_pRegistryAccess->ReadRegistry
//            (pszValueName,
//             &dwTargetBuffer,
//             "DebugLevels",
//             0);
//        //store the registry debug level value in the member variable for this
//        //      class
//        m_ucDebugLevel[i] = (UCHAR)dwTargetBuffer;
//    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is called every time a debug message is sent from a HAL
//  class and stores the corresponding class ID in a member variable that is
//  accessed from the debug print functions.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void COSDependDebug::SetCurrentClassID
(
    DWORD dwClassID //The class ID to set.
)
{
    //actualize debug class ID for access from the Debug_Print() function
//    if(dwClassID <= LAST_DEBUG_CLASS_ID)
//    {
//        m_dwClassID = dwClassID;
//    }
}

#endif // DBG
