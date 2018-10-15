/////////////////////////////////////////////////////////////////////////////
// Copyright © Microsoft Corporation. All rights reserved.
// Microsoft ® PREfast desktop correctness checker.
//


/////////////////////////////////////////////////////////////////////////////
// Conditional Script Execution
//
// The @enable_update and @enable_feedback parameters have been moved.
// See the top of the wsc/pftExecEnv.wsc file for more information.
//


/////////////////////////////////////////////////////////////////////////////
// Global Variables

// Script error context
var g_arrContext;            // A stack of what the script is currently doing

// Global helper objects
var g_objShell;              // The Windows Script Host WshShell object
var g_objProcEnv;            // The process environment variables collection
var g_objFSO;                // The File System object
var g_objPREfast;

// Registry key paths
var g_strRegPathSAVE;

// Input strings
var g_strSavedSwitches;      // The user's saved switches from the registry
var g_strCmdLine;            // The full command line

// Script verbosity
var g_nVerbosity = 1;        // Indicates which messages to output

// Script environment
var g_strPREfastCL;          // Full pathname of prefastcl shim
var g_strPREfastUtil;        // Full pathname of configuration cmdline interface
var g_strPREfastHELPFILE;    // Full pathname of product help user interface
var g_strPREfastXSLFlatten;  // Full pathnames of the stylesheets for FLATTEN
var g_strPREfastXSLFlattenSeq;

// Switches
var g_dictValidSwitches;     // The values in effect of all switches, no matter where the value originated
var g_dictSpecdSwitches;     // The switches specified on the command line
var g_dictSavedSwitches;     // The switches saved to the registry
var g_dictValueSwitches;     // The switches that take a value
var g_dictMutexSwitches;     // The mutually exclusive switches

// Filters
var g_objPresets = null;             // The set of all presets
var g_objFilter = null;             // The selected filter preset;

/////////////////////////////////////////////////////////////////////////////
//
var nExitCode;
try
{
    // Initialize the stack of script context
    g_arrContext = new Array();

    // Call the script's main function
    nExitCode = main(WScript.Arguments.Count(), WScript.Arguments);
}
catch (e)
{
    // Report the exception
    var strError;
    if ("string" == typeof(e))
    {
        strError = e;
    }
    else
    {
        var strErrNum = "0000000" + (e.number + 0x100000000).toString(16);
        strErrNum = "0x" + strErrNum.substring(strErrNum.length - 8);
        strError = "Error " + strErrNum + " occurred while " + ContextTop() +
                   ":\n\t" + e.description;
    }
    Echo(0, strError, true, true);

    // Set the exit code of the script
    nExitCode = e.number;
}

// Flush the console output
Echo(0, "", false, false);

// Exit the script
WScript.Quit(nExitCode);


/////////////////////////////////////////////////////////////////////////////
//
function main(argc, args)
{
    // Check Scripting Version
    CheckJScriptVersion( 5, 5 );  // Must be at least version 5.5

    // Create global helper objects
    ContextSet("creating script helper objects");
    g_objFSO     = GuardedCreateObject("Scripting.FileSystemObject");
    g_objShell   = GuardedCreateObject("WScript.Shell");
    g_objProcEnv = g_objShell.Environment("Process");

    g_objPREfast = GuardedCreateObject("PREfast.ExecEnvironment");

    // Initialize the environment variables
    InitEnvironment();

    var arrArgs = new Array;

    // Copy the arguments from the slightly odd collection that they're in into an
    // array that we can manipulate more easily
    for ( var i = 0; i < argc; i++ )
        arrArgs[i] = args(i);

    // Parse the command line
    return ParseCommandLine(arrArgs);
}

/////////////////////////////////////////////////////////////////////////////
//
function InitEnvironment()
{
    ContextSet("initializing environment variables");


    // Other registry paths
    g_strRegPathSAVE = g_objPREfast.RegistryKey + "SavedSwitches\\";

    // The locations of specific PREfast scripts
    g_strPREfastCL            = g_objPREfast.ScriptsDirectory + "\\interceptcl\\cl.exe";
    g_strPREfastUtil          = '"' + g_objPREfast.ScriptsDirectory + '\\pftutil.exe"';
    g_strPREfastXSLFlatten    = g_objPREfast.ScriptsDirectory + "\\flatten.xsl";
    g_strPREfastXSLFlattenSeq = g_objPREfast.ScriptsDirectory + "\\flattenSeq.xsl";

    // The locations of specific PREfast documents
    g_strPREfastHELPFILE = g_objPREfast.DocumentationDirectory + "\\CmdLine.html";

    // Set the default MAXPATHS, if not specified by an environment variable
    if (!g_objProcEnv("_DFA_MAX_PATHS_").length)
        g_objProcEnv("_DFA_MAX_PATHS_") = 256;

    // Set the default MAXTIME, if not specified by an environment variable
    if (!g_objProcEnv("_PREFAST_MAX_TIME_").length)
        g_objProcEnv("_PREFAST_MAX_TIME_") = 5000;

    // Set the default INCREASEHEAP, if not specified by an environment variable
    if (!g_objProcEnv("PREFAST_INCREASE_HEAP").length)
        g_objProcEnv("PREFAST_INCREASE_HEAP") = 0;

    // Set the default STACKHOGTHRESHOLD, if not specified by an environment variable
    if (!g_objProcEnv("_PRECEDENCE_STACK_THRESHOLD_").length)
        g_objProcEnv("_PRECEDENCE_STACK_THRESHOLD_") = 1024;

    // Set the default EnableCritSState, if not specified by an environment variable
    if (!g_objProcEnv("_PRECEDENCE_FLAG_INITCRITS_").length)
        g_objProcEnv("_PRECEDENCE_FLAG_INITCRITS_") = 0;

    // Set the default NewFailure, if not specified by an environment variable
    if (!g_objProcEnv("_DFA_NEW_FAILURE_").length)
        g_objProcEnv("_DFA_NEW_FAILURE_") = "never";

    // Set the default regular cl, if not specified by an environment variable
    if (!g_objProcEnv("PREFAST_USER_CL_EXE").length)
        g_objProcEnv("PREFAST_USER_CL_EXE") = "default";

    // Set the default exclude list, if not specified by an environment variable
    if (!g_objProcEnv("PREFAST_EXCLUDE_LIST").length)
        g_objProcEnv("PREFAST_EXCLUDE_LIST") = "none;";
}


/////////////////////////////////////////////////////////////////////////////
//
function ParseCommandLine(arrArgs)
{
    var result;

    // Parse the command line
    ContextSet("parsing the command line");

    // We only need this any more for the feedback mechanism
    g_strCmdLine = ConcatenateArgs( arrArgs );
    
    InitSwitches();

    // Check for the UNSAVE command before parsing any switches.
    // Otherwise, it is impossible to recover from bad saved switches
    if ( arrArgs[0] && "UNSAVE" == arrArgs[0].toUpperCase() )
    {
        DisplayLogo();
        return DoUnSave();
    }

    // Parse any Switches that may have been passed
    result = ParseSwitches( arrArgs );
    if (result)
        return result;

    // Display the logo; it will be suppressed if VERBOSE has been set to 0
    DisplayLogo();

    // The switches have been shifted off, so the command is next.
    var strCommand = arrArgs.shift();
    if ( undefined == strCommand )
        return Syntax();

    // Process every other prefast command
    switch ( strCommand.toUpperCase() )
    {
    case "HELP":
        return DoHelp();
        break;

    case "CONFIG":
        return DoConfig();
        break;

    case "UNSAVE":
        return DoUnSave();
        break;

    case "VERSION":
        return DoVersion();
        break;

    case "UPDATE":
        return DoUpdate(false);
        break;

        // Undocumented command
    case "PREFAST_SWITCHTO_DEBUG":
        return DoSwitchToDebug();
        break;

        // Undocumented command
    case "PREFAST_SWITCHTO_RELEASE":
        return DoSwitchToRelease();
        break;

    case "RESET":
        var strLog = arrArgs.length > 0 ? arrArgs[0] : g_dictValidSwitches("LOG");
        return DoReset( strLog );
        break;

    case "LIST":
        var strLog = arrArgs.length > 0 ? arrArgs[0] : g_dictValidSwitches("LOG");
        return DoList( strLog );
        break;

    case "VIEW":
        var strLog = arrArgs.length > 0 ? arrArgs[0] : g_dictValidSwitches("LOG");
        return DoView( strLog );
        break;


    case "COUNT":
        var strLog = arrArgs.length > 0 ? arrArgs[0] : g_dictValidSwitches("LOG");
        return DoCount( strLog, false );
        break;

    case "COUNTF":
        var strLog = arrArgs.length > 0 ? arrArgs[0] : g_dictValidSwitches("LOG");
        return DoCount( strLog, true );
        break;

    case "FLATTEN":
        var strFile1 = (arrArgs.length > 0) ? arrArgs[0] : "";
        var strFile2 = (arrArgs.length > 1) ? arrArgs[1] : "";
        return DoFlatten(strFile1, strFile2);
        break;

    case "UNFLATTEN":
        var strFile1 = (arrArgs.length > 0) ? arrArgs[0] : "";
        var strFile2 = (arrArgs.length > 1) ? arrArgs[1] : "";
        return DoUnflatten(strFile1, strFile2);
        break;

    case "REMOVEDUPS":
        return DoRemoveDups(arrArgs);
        break;

    case "FILTER":
        var strFile1 = (arrArgs.length > 0) ? arrArgs[0] : "";
        var strFile2 = (arrArgs.length > 1) ? arrArgs[1] : "";
        return DoFilter( strFile1, strFile2 );
        break;

    case "MODMAN":
        return DoModMan( ConcatenateArgs( arrArgs ) );
        break;

    case "WRITEEXITFUNCTIONS":
        var strFile1 = (arrArgs.length > 0) ? arrArgs[0] : "";
        var strFile2 = (arrArgs.length > 1) ? arrArgs[1] : "";
        return DoWriteExitFunctions( strFile1, strFile2 );
        break;

    case "READEXITFUNCTIONS":
        var strFile1 = (arrArgs.length > 0) ? arrArgs[0] : "";
        var strFile2 = (arrArgs.length > 1) ? arrArgs[1] : "";
        return DoReadExitFunctions( strFile1, strFile2 );
        break;

    case "INSTALL":
        return DoInstall( arrArgs );

    case "SAVE":
        // Do not save the /N, /LOG, or switches
        g_dictValidSwitches.Remove("N");
        g_dictValidSwitches.Remove("LOG");
        return DoSave();
        break;

    case "EXEC":
        // We don't want the word 'EXEC', we want the build command
        strCommand = arrArgs.shift();
        // FALL THROUGH INTENTIONALLY
    default:
        return DoExec( strCommand, ConcatenateArgs( arrArgs, "\n" ), g_dictValidSwitches);
        break;
    }

    // We shouldn't ever get to this point, but if we do...
    return Syntax();
}


/////////////////////////////////////////////////////////////////////////////
//
function ConcatenateArgs( arrArgs, delim )
{
    ContextPush("generating the full command-line string");
    var strCmdLine = "";
    if ( !delim )
        delim = " ";
    for (var i = 0; i < arrArgs.length; ++i)
        strCmdLine += ((i > 0) ? delim : "") + arrArgs[i];
    ContextPop();
    return strCmdLine;
}

/////////////////////////////////////////////////////////////////////////////
//
function InitSwitches()
{
    // Define the valid switches and their default values
    g_dictValidSwitches = GuardedCreateObject("Scripting.Dictionary");
    g_dictValidSwitches.CompareMode = 1;
    g_dictValidSwitches("N"         ) = false;
    g_dictValidSwitches("LOG"       ) = g_objProcEnv("PREFASTLOG");
    g_dictValidSwitches("CONFIGFILE") = g_objProcEnv("PREFASTCONFIG");
    g_dictValidSwitches("MAXPATHS"  ) = g_objProcEnv("_DFA_MAX_PATHS_");
    g_dictValidSwitches("MAXTIME"   ) = g_objProcEnv("_PREFAST_MAX_TIME_");
    g_dictValidSwitches("INCREASEHEAP"   ) = g_objProcEnv("PREFAST_INCREASE_HEAP");
    g_dictValidSwitches("STACKHOGTHRESHOLD") = g_objProcEnv("_PRECEDENCE_STACK_THRESHOLD_");
    g_dictValidSwitches("ENABLECRITSWARNING") = g_objProcEnv("_PRECEDENCE_FLAG_INITCRITS_");
    g_dictValidSwitches("NEW_FAILURE") = g_objProcEnv("_DFA_NEW_FAILURE_");
    g_dictValidSwitches("CL_EXE"  ) = g_objProcEnv("PREFAST_USER_CL_EXE");
    g_dictValidSwitches("EXCLUDE"  ) = g_objProcEnv("PREFAST_EXCLUDE_LIST");
    g_dictValidSwitches("FILTERSFILE" ) = g_objProcEnv("PREFAST_FILTERS");
    g_dictValidSwitches("FILTERPRESET"    ) = g_objProcEnv("PREFAST_FILTER_PRESET");
    g_dictValidSwitches("MODELFILES" ) = g_objProcEnv("PREFASTMODEL");
    g_dictValidSwitches("COVERAGEFILE") = g_objProcEnv("PREFASTCOVERAGEFILE");
    g_dictValidSwitches("VERBOSE"   ) = 1;
    g_dictValidSwitches("RESET"     ) = true;
    g_dictValidSwitches("NORESET"   ) = false;
    g_dictValidSwitches("LIST"      ) = false;
    g_dictValidSwitches("NOLIST"    ) = true;
    g_dictValidSwitches("VIEW"      ) = false;
    g_dictValidSwitches("NOVIEW"    ) = true;
    g_dictValidSwitches("FEEDBACK"  ) = true;
    g_dictValidSwitches("NOFEEDBACK") = false;
    g_dictValidSwitches("UPDATE"    ) = true;
    g_dictValidSwitches("NOUPDATE"  ) = false;
    g_dictValidSwitches("ONEPASS"   ) = false;
    g_dictValidSwitches("TWOPASS"   ) = true;
    g_dictValidSwitches("NOREMOVEDUPS") = false;
    g_dictValidSwitches("REMOVEDUPS") = true;
    g_dictValidSwitches("HELP"      ) = false;
    g_dictValidSwitches("?"         ) = false;
    g_dictValidSwitches("NOFILTER"  ) = true;
    g_dictValidSwitches("FILTER"    ) = false;
    g_dictValidSwitches("WSPMIN"    ) = false;
    g_dictValidSwitches("MACRO"     ) = false;

    // Define the switches that must take values
    g_dictValueSwitches = GuardedCreateObject("Scripting.Dictionary");
    g_dictValueSwitches.CompareMode = 1;
    g_dictValueSwitches("LOG"     ) = ValidateNotEmpty;
    g_dictValueSwitches("CONFIGFILE"  ) = ValidateConfigFile;
    g_dictValueSwitches("MAXPATHS") = ValidateNotEmpty;
    g_dictValueSwitches("MAXTIME" ) = ValidateNotEmpty;
    g_dictValueSwitches("INCREASEHEAP" ) = ValidateNotEmpty;
    g_dictValueSwitches("VERBOSE" ) = ValidateVerbose;
    g_dictValueSwitches("STACKHOGTHRESHOLD") = ValidateNotEmpty;
    g_dictValueSwitches("ENABLECRITSWARNING") = ValidateNotEmpty;
    g_dictValueSwitches("NEW_FAILURE") = ValidateNewFailure;
    g_dictValueSwitches("CL_EXE") = ValidateNotEmpty;
    g_dictValueSwitches("EXCLUDE") = ValidateNotEmpty;
    g_dictValueSwitches("FILTERPRESET") = ValidateFilterPreset;
    g_dictValueSwitches("FILTERSFILE") = ValidateFiltersFile;
    g_dictValueSwitches("MODELFILES") = ValidateModelFiles;
    g_dictValueSwitches("COVERAGEFILE") = ValidateCoverageFile;
    // Doesn't actually take a value, but needs a validation function
    g_dictValueSwitches("WSPMIN") = ValidateWspmin;

    // Define the switches that are mutually exclusive
    g_dictMutexSwitches = GuardedCreateObject("Scripting.Dictionary");
    g_dictMutexSwitches.CompareMode = 1;
    g_dictMutexSwitches("RESET"     ) = "NORESET";
    g_dictMutexSwitches("NORESET"   ) = "RESET";
    g_dictMutexSwitches("LIST"      ) = "NOLIST";
    g_dictMutexSwitches("NOLIST"    ) = "LIST";
    g_dictMutexSwitches("VIEW"      ) = "NOVIEW";
    g_dictMutexSwitches("NOVIEW"    ) = "VIEW";
    g_dictMutexSwitches("FEEDBACK"  ) = "NOFEEDBACK";
    g_dictMutexSwitches("NOFEEDBACK") = "FEEDBACK";
    g_dictMutexSwitches("UPDATE"    ) = "NOUPDATE";
    g_dictMutexSwitches("NOUPDATE"  ) = "UPDATE";
    g_dictMutexSwitches("ONEPASS"   ) = "TWOPASS";
    g_dictMutexSwitches("TWOPASS"   ) = "ONEPASS";
    g_dictMutexSwitches("REMOVEDUPS" ) = "NOREMOVEDUPS";
    g_dictMutexSwitches("NOREMOVEDUPS") = "REMOVEDUPS";
    g_dictMutexSwitches("FILTER"    ) = "NOFILTER";
    g_dictMutexSwitches("NOFILTER"  ) = "FILTER";
}

/////////////////////////////////////////////////////////////////////////////
//
function ParseSwitches(arrArgs)
{
    ContextPush("parsing switches");

    // Create the collection of saved switches
    g_dictSavedSwitches = GuardedCreateObject("Scripting.Dictionary");
    g_dictSavedSwitches.CompareMode = 1; // Text

    // Create the collection of specified switches
    g_dictSpecdSwitches = GuardedCreateObject("Scripting.Dictionary");
    g_dictSpecdSwitches.CompareMode = 1; // Text

    ContextPush("parsing the saved switches");
    
    var strSwitch;
    var arrSwitches = new VBArray(g_dictValidSwitches.Keys()).toArray();
    for ( var i in arrSwitches )
    {
        strSwitch = arrSwitches[i];
        try
        {
            strValue = g_objShell.RegRead(g_strRegPathSAVE + strSwitch + "\\");
        }
        catch (e)
        {
            // Ignore the switch if it can't be read
            continue;
        }
        nResult = ParseSwitch( strSwitch, strValue, g_dictSavedSwitches, false );
        if ( nResult )
            throw "One or more saved switches are invalid.\n\n" +
            "Enter PREFAST UNSAVE to resolve the problem.";
    }
    ContextPop();
    
    ContextPush("parsing the specified switches");
    var strArg;
    while ( (strSwitch = arrArgs.shift()) != undefined )
    {
        var strFlag = strSwitch.charAt(0);
        if ('-' == strFlag || '/' == strFlag)
        {
            var arrArg = strSwitch.substring(1).split('=', 2);
            nResult = ParseSwitch( arrArg[0], arrArg[1], g_dictSpecdSwitches, true );
            if ( nResult )
                return nResult;
        }
        else
        {
            arrArgs.unshift( strSwitch );
            break;
        }
    }
    ContextPop();

    ContextPop();
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// Description: Parses an individual switch.
//
// Parameters:
//     strSwitch - The name of a switch, without the - or /
//     strValue  - The value of the switch
//     fReportViolations - When false, the function will not report syntax
// violations, but will not ignore them either. Instead, it will silently
// return a non-zero error code.
//      dictSpecdSwitches - The function saves all valid switch names
// into the provided dictionary
//
//     g_dictValidSwitches - A Dictionary object containing the valid switch
// string mapped to its current value. In this dictionary, the switch Strings
// must not be prefixed with a / or -. The CompareMode of the dictionary must
// have been previously set to 1 (Text). The switch and value, if valid,
// will be added to this dictionary.
//
// Return Value: If the parsing succeeds as specified, zero will be returned
// Otherwise, non-zero wil be returned.
//
function ParseSwitch(strSwitch, strValue, dictSpecdSwitches, fReportViolations)
{
    // Validate the specified Dictionary objects
    if (1 != g_dictValidSwitches.CompareMode)
        throw "The CompareMode of the dictValidSwitches argument to function\n" +
        "ParseSwitchArray must have been set to 1 (Text).";
    if (1 != g_dictSpecdSwitches.CompareMode)
        throw "The CompareMode of the dictSpecdSwitches argument to function\n" +
        "ParseSwitchArray must have been set to 1 (Text).";

    // Validate that this is a valid switch
    if (!g_dictValidSwitches.Exists(strSwitch))
        return SyntaxViolated("Invalid switch specified: " + strSwitch, !fReportViolations);

    // Process differently based on if the switch takes a value or not
    if (g_dictValueSwitches.Exists(strSwitch))
    {
        var fnValidate = g_dictValueSwitches(strSwitch);
        if (fnValidate)
        {
            var result = fnValidate(strSwitch, strValue, fReportViolations);
            if (result)
                return result;
        }

        dictSpecdSwitches(strSwitch) = strSwitch;
        g_dictValidSwitches(strSwitch) = strValue;
    }
    else
    {
        // Validate that a value was not specified with this switch
        if (strValue && "" != strValue)
            return SyntaxViolated("Switch does not take a value: " + strSwitch, !fReportViolations);

        // Validate that a conflicting switch has not already been specified
        if (g_dictMutexSwitches.Exists(strSwitch))
        {
            var strConflict = g_dictMutexSwitches(strSwitch);
            if (dictSpecdSwitches.Exists(strConflict))
                return SyntaxViolated("Conflicting switches specified: /" + strSwitch + " and /" + dictSpecdSwitches(strConflict), !fReportViolations);
            g_dictValidSwitches(strConflict) = false;
        }

        dictSpecdSwitches(strSwitch) = strSwitch;
        g_dictValidSwitches(strSwitch) = true;
    }

    // Indicate success
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// Validates switches that must specify a value.
//
function ValidateNotEmpty(strSwitch, strValue, fReportViolations)
{
    // A value must be specified with this switch
    if (!strValue || "" == strValue)
        return SyntaxViolated("Switch was specified without a value: " + strSwitch, !fReportViolations);
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// Validates the /VERBOSE= switch
//
function ValidateVerbose(strSwitch, strValue, fReportViolations)
{
    // A value must be specified with this switch
    var result = ValidateNotEmpty(strSwitch, strValue, fReportViolations)
                 if (result)
                     return result;

                 // Only certain values are allowed
                 var nValue = new Number(strValue);
    switch (nValue.toString())
    {
    case '0':
    case '1':
    case '2':
    case '3':
        g_nVerbosity = nValue;
        return 0;

    default:
        return SyntaxViolated("Invalid value '" + strValue + "' specified for switch '" + strSwitch + "'.", !fReportViolations);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Validates the /FILTERPRESET= switch
//
// Ensures that the value is an existing filter preset name in the registry
// or one of the default set.
function ValidateFilterPreset( strSwitch, strValue, fReportViolations )
{
    LoadPresets();
    g_objFilter = g_objPresets(strValue);

    if ( g_objFilter == null )
        return SyntaxViolated("Switch " + strSwitch + " must specify an existing filter preset name.\n"
                              + "Filter preset names are case sensitive.", !fReportViolations );

    g_objProcEnv("PREFAST_FILTER_PRESET") = strValue;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Validates the /FILTERSFILE= switch
//
// Currently, sets the PREFAST_FILTERS env var without checking if the file
// exists or not.
function ValidateFiltersFile( strSwitch, strValue, fReportViolations )
{
    g_objProcEnv("PREFAST_FILTERS") = strValue;

    // If g_objPresets already exists, we need to delete and recreate it so the
    // new setting of PREFAST_FILTERS will take effect
    if ( g_objPresets )
    {
        g_objPresets = null;
        LoadPresets();
        g_objFilter = g_objPresets.selectedPreset;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Validates the /WSPMIN switch
//
// Sets the value of FilterPreset to WSPMIN
//
function ValidateWspmin( strSwitch, strValue, fReportViolations )
{
    return ValidateFilterPreset( strSwitch, "wspmin", fReportViolations );
}

/////////////////////////////////////////////////////////////////////////////
// Validates the /COVERAGEFILE= switch
//
function ValidateCoverageFile( strSwitch, strValue, fReportViolations )
{
    if ( undefined == strValue || 0 == strValue.length )
        return SyntaxViolated("Switch was specified without a value: " + strSwitch, !fReportViolations);

    try
    {
        g_objProcEnv("PREFASTCOVERAGEFILE") = ValidateXMLPath(strValue, "coverage file", 0);
    }
    catch (e)
    {
        var err = "Invalid value '" + strValue + "' specified for " + strSwitch + ":\n";
        if ( "string" == typeof( e ) )
            err += e;   
        return SyntaxViolated(err, !fReportViolations);
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////
//
function ValidateModelFiles( strSwitch, strModelFiles, fReportViolations )
{
    if ( undefined == strModelFiles )
        strModelFiles = "";

    ContextPush("validating the model files: \"" + strModelFiles + "\"");

    var arrModelFiles = strModelFiles.split(";");
    var strNewModelFiles = "";
    var strModelsDirectory = g_objPREfast.ModelsDirectory;

    for ( var i in arrModelFiles )
    {
        strModelFile = arrModelFiles[i];
        if ( !strModelFile || strModelFile.length == 0 )
            continue;
        
        strNewModelFiles += g_objFSO.GetAbsolutePathName( strModelFile );
        strNewModelFiles += ";";
    }

    strNewModelFiles += g_objFSO.BuildPath( g_objPREfast.ModelsDirectory, "PfxModel.xml" );

    g_objProcEnv("PREFASTMODEL") = strNewModelFiles;
    
    Echo( 2, "PREFASTMODEL is " + strNewModelFiles );

    ContextPop();
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
function ValidateConfigFile( strSwitch, strConfigFile, fReportViolations )
{
    ContextPush("validating the config file: \"" + strConfigFile + "\"");

    // Just validate that the directory exists.  It is legal to pass in 
    // a non-existent file name for PREFAST CONFIG to create, but if the
    // directory doesn't exist we need to signal an error here.

    var strDirectory;

    strDirectory = g_objFSO.GetParentFolderName( strConfigFile );
    if ( !g_objFSO.FolderExists( strDirectory ) )
        return SyntaxViolated( strSwitch + " must specify a file in an existing directory.\n"
                              + "Directory '" + strDirectory + "' does not exist.\n" );

       
    ContextPop();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Validates the /NEW_FAILURE= switch
//
function ValidateNewFailure(strSwitch, strValue, fReportViolations)
{
    // A value must be specified with this switch
    var result = ValidateNotEmpty(strSwitch, strValue, fReportViolations);
    if (result)
        return result;

    // Only certain values are allowed
    switch (strValue.toUpperCase())
    {
    case 'NEVER':
    case 'NULL':
    case 'THROW':
        return 0;

    default:
        return SyntaxViolated("Invalid value '" + strValue + "' specified with switch '" + strSwitch + "'.", !fReportViolations);
    }
}

/////////////////////////////////////////////////////////////////////////////
//
function DoHelp()
{
    // Launch the help file
    ContextSet("launching the PREfast help file");
    g_objShell.Run('"' + g_strPREfastHELPFILE + '"', 1, false);

    // Indicate success
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoConfig()
{
    // Set the PREFASTCONFIG environment variable
    ValidateConfigFilePath(g_dictValidSwitches("CONFIGFILE"));

    // Launch the Configuration User Interface
    ContextSet("launching the PREfast Configuration user interface");
    var strFullCmdLine = g_strPREfastUtil + " config";

    g_objShell.Run( strFullCmdLine, 1 );

    // Indicate success
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoVersion()
{
    // Delegate to DoUpdate
    return DoUpdate(true);
}


/////////////////////////////////////////////////////////////////////////////
//
function DoUpdate(fCheckOnly)
{
    ContextSet("checking for updated PREfast components");

    // Get the installed and latest versions of PREfast
    var nVerbosityPrev = g_nVerbosity;
    g_nVerbosity = 2;
    var strInstalled = g_objPREfast.InstalledBuildNumber;
    var strLatest = g_objPREfast.LatestBuildNumber;
    g_nVerbosity = nVerbosityPrev;

    // Exit if either version could not be determined
    if ("????" == strInstalled || "????" == strLatest)
        return 3;

    // Convert each build number string to a number
    var nInstalled = new Number(strInstalled);
    var nLatest = new Number(strLatest);

    // Check for updated PREfast components
    var fNeedsUpdate = nLatest > nInstalled;
    if (!fNeedsUpdate)
    {
        ReportUpdateNotNeeded(strInstalled, strLatest);
        return 0;
    }

    // Report the needed updates or actually do it
    if (fCheckOnly)
    {
        // Report that updates are needed
        ReportUpdateNeeded(strInstalled, strLatest);

        // Indicate success
        return 0;
    }

    // Create a temporary file name for the update.bat file
    ContextSet("creating temporary file for update.bat");
    var strTempPath = g_objFSO.GetSpecialFolder(2).Path;
    if (strTempPath.charAt(strTempPath.length - 1) != "\\")
        strTempPath += "\\";
    var strDestFile = strTempPath + g_objFSO.GetTempName() + ".bat";

    // Download the update.bat file
    ContextSet("downloading latest update.bat file");
    var strUNC = g_objPREfast.UpdateUNCBase + "update.bat";
    g_objPREfast.DownloadUNCToTextFile(strUNC, strDestFile, true);

    // Restore Release files
    DoSwitchToRelease(true);

    // Begin update process
    ContextSet("launching Windows installer to update PREfast components");
    Echo(1, "Launching Windows Installer to update PREfast components...", true, true);
    g_objShell.Run("%COMSPEC% /c " + '"' + strDestFile + '"');

    // Indicate success
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function ReportUpdateNeeded(strInstalled, strLatest)
{
    var str = Decorate(
                      "An update to PREfast is available:\n" +
                      GetReportVersionString(strInstalled, strLatest), false, true);
    str +=
    "\nEnter PREFAST UPDATE to get the latest version.\n";
    Echo(1, str, true, false);
}


/////////////////////////////////////////////////////////////////////////////
//
function ReportUpdateNotNeeded(strInstalled, strLatest)
{
    var strMsg = g_objPREfast.IsUpdateEnabled ?
                 "The installed PREfast components are up-to-date:\n" : "";

    var str = Decorate(strMsg +
                       GetReportVersionString(strInstalled, strLatest), false, true);
    Echo(1, str, true, false);
}


/////////////////////////////////////////////////////////////////////////////
//
function GetReportVersionString(strInstalled, strLatest)
{
    if (g_objPREfast.IsUpdateEnabled)
    {
        return "    Installed version: " + g_objPREfast.ComposeVersionString() + "\n" +
        "       Latest version: " + g_objPREfast.ComposeVersionString(strLatest);
    }
    else
    {
        return "Installed PREfast version: " + g_objPREfast.ComposeVersionString();
    }
}


/////////////////////////////////////////////////////////////////////////////
//
function DoSwitchToDebug(fQuiet)
{
    // Ensure that the feature was installed
    if (!g_objFSO.FileExists(g_objPREfast.ModulesDirectory + "\\Debug\\Driver.dll"))
        return ReportDebugNotInstalled();

    try
    {
        // Copy all PDB's, DLL's from the Debug directory to the parent directory
        g_objFSO.CopyFile(g_objPREfast.ModulesDirectory + "\\Debug\\*.PDB", g_objPREfast.ModulesDirectory, true);
        g_objFSO.CopyFile(g_objPREfast.ModulesDirectory + "\\Debug\\*.DLL", g_objPREfast.ModulesDirectory, true);
    }
    catch (e)
    {
    }

    // Indicate success
    if (!fQuiet)
        Echo(1, "Debug versions of the PREfast modules have been copied.", true, true);
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoSwitchToRelease(fQuiet)
{
    // Ensure that the feature was installed
    if (!g_objFSO.FileExists(g_objPREfast.ModulesDirectory + "\\Release\\Driver.dll"))
        return ReportDebugNotInstalled();

    try
    {
        // Delete any PDB's from the parent directory
        g_objFSO.DeleteFile(g_objPREfast.ModulesDirectory + "\\*.PDB", true);
        // Copy all DLL's from the Debug directory to the parent directory
        g_objFSO.CopyFile(g_objPREfast.ModulesDirectory + "\\Release\\*.DLL", g_objPREfast.ModulesDirectory, true);

    }
    catch (e)
    {
    }

    // Indicate success
    if (!fQuiet)
        Echo(1, "Release versions of the PREfast modules have been restored.", true, true);
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function ReportDebugNotInstalled()
{
    Echo(0,
         "The 'Debug Files' feature was not installed when PREfast was installed.\n" +
         "Use Add/Remove Programs to add this feature to the PREfast installation.",
         true, true);
    return 1;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoReset(strLog, fQuiet)
{
    // Set the PREFASTLOG environment variable
    strLog = ValidateDefectLogPath(strLog);

    // Reset the defect log file
    ContextSet("reseting the PREfast Defect Log file: \"" + strLog + "\"");
    g_objFSO.DeleteFile(strLog);

    // Report what we just did
    Echo(fQuiet ? 2 : 1,
         "The PREfast Defect Log file was reset:\n\t\"" + strLog + "\"",
         true, true);

    // Indicate success
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoView(strLog)
{
    // Set the PREFASTLOG environment variable
    strLog = ValidateDefectLogPath(strLog, true);

    // Launch the Defect Log User Interface
    ContextSet("launching the PREfast Defect Log user interface");
    g_objPREfast.LaunchHTA("DefectUI\\DefectUI.hta", false);

    // Indicate success
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoList(strLog)
{
    // Launch the Defect Log text formatter
    ContextSet("transforming the PREfast Defect Log to text");
    Echo(1, "Contents of defect log: " + strLog, true, true);

    // Flush the console echo
    Echo(1, "", false, false);

    var objDefectSet = LoadDefects( strLog, true );

    // Loop through all of the defects
    ContextSet("looping through all defects in function DoList");
    var defects = objDefectSet.defectNodesFiltered;
    var cDefects = defects.length;
    for (var i = 0; i < cDefects; ++i)
    {
        // Get the next defect
        var defect = defects(i);

        // Extract the XML data and format the strings
        var strDesc     = defect.selectSingleNode("DESCRIPTION").text;
        var strWarning  = defect.selectSingleNode("DEFECTCODE").text;
        var strFile     = defect.selectSingleNode("SFA/FILEPATH").text +
                          defect.selectSingleNode("SFA/FILENAME").text + " (" +
                          defect.selectSingleNode("SFA/LINE").text + ")";
        var strFunction = defect.selectSingleNode("FUNCTION").text;
        var strFuncLine = defect.selectSingleNode("FUNCLINE").text;

        // Output the information about the defect
        WScript.Echo(strFile + ": warning " + strWarning + ": " + strDesc);
        WScript.Echo("\tFUNCTION: " + strFunction + " (" + strFuncLine + ")");
        var strPath = "";
        var sfas = defect.selectNodes("PATH/SFA");
        var cSfas = sfas.length;
        for (var iPath = 0; iPath < cSfas; ++iPath)
            strPath += sfas(iPath).selectSingleNode("LINE").text + " ";
        WScript.Echo("\tPATH: " + strPath);
        WScript.Echo("");
    }

    // Display a summary
    var cDefectsListed = objDefectSet.defectNodesFiltered.length;
    var strDefectsListed = ((1 == cDefectsListed) ? " Defect " : " Defects ") + "Listed\n";
    if ("(all defects)" == g_objFilter.name)
    {
        var str = cDefectsListed + strDefectsListed
                  str += "No filter in effect";
        Echo(1, str, true, true);
    }
    else
    {
        var str = objDefectSet.defectNodesFiltered.length + " of ";
        str += cDefectsListed + strDefectsListed;
        str += "Filter in effect: " + g_objFilter.name;
        Echo(1, str, true, true);
    }

    var str = "You can change the filter and sort options from the user interface.\n";
    str += "Enter PREFAST VIEW to display the defect log user interface.";
    Echo(1, str, true);

    // Indicate success
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoCount(strLog, fFiltered)
{
    ContextSet("counting the defects in the PREfast Defect Log");
    if (fFiltered)
        Echo(1, "Number of filtered defects in defect log: " + strLog, true, true);
    else
        Echo(1, "Number of unfiltered defects in defect log: " + strLog, true, true);

    var objDefectSet = LoadDefects( strLog, fFiltered );

    // Loop through all of the defects
    ContextSet("counting all defects in function DoCount");
    var defects = fFiltered ? objDefectSet.defectNodesFiltered : objDefectSet.defectNodesMaster;
    var cDefects = defects.length;

    // Display the number of defects
    Echo(1, cDefects.toString(), false, false);
    Echo(1, "", false, false);

    // Return the number of defects
    return cDefects;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoFlatten(strFileIn, strFileOut)
{
    // Verify that the filenames were specified
    if (!strFileIn || !strFileOut || !strFileIn.length || !strFileOut.length)
        return SyntaxViolated("Not enough parameters specified for FLATTEN command.");

    //
    // Transform the specified defect log to a flat file
    //

    // Open the output file
    ContextSet("opening the output file: " + strFileOut);
    var fileOut = g_objFSO.CreateTextFile(strFileOut, true);

    FlattenToFile( false, strFileIn, fileOut );
    fileOut.Close();
    delete fileOut;

    // Flush the console echo
    Echo(0, "", false, false);

    // Indicate success
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
// Remove duplicates from a flat file
//
function RemoveDupsFromFlatFile( strFile )
{
    var nRet = 0;
    var strTempFile1 = GetTempFileName();
    var strTempFile2 = GetTempFileName();

    // Get the scripts directory to locate sortFile.exe
    var strSortExe = g_objPREfast.ScriptsDirectory + "\\sortFile.exe";

    // Verify that sortFile.exe exists in the scripts directory
    if (!g_objFSO.FileExists(strSortExe))
        throw "sortFile.exe does not exist in " + g_objPREfast.ScriptsDirectory;

    // Sort the input flat file for uniquness
    ContextSet("sorting the temporary flat file at column 10: " + strFile);
    var strSort1 = strSortExe + "\n/+10\n/UNIQ\n" + strFile + "\n" + strTempFile1;
    nRet = RunProcess( strSort1, "\n" );
    if ( nRet != 0 )
    {
        g_objFSO.DeleteFile( strTempFile1 );
        Echo( 0, "Could not perform REMOVEDUPS, " + strSortExe + " returned '" + nRet + "'.\n" );
        return nRet;
    }

    // Sort the first temporary flat file for sequence
    ContextSet("sorting the temporary flat file on original sequence: " + strTempFile1);
    var strSort2 = strSortExe + "\n" + strTempFile1 + "\n" + strTempFile2;
    nRet = RunProcess( strSort2, "\n" );
    if ( nRet != 0 )
    {
        g_objFSO.DeleteFile( strTempFile1 );
        g_objFSO.DeleteFile( strTempFile2 );
        Echo( 0, "Could not perform REMOVEDUPS, " + strSortExe + " returned '" + nRet + "'.\n" );
        return nRet;
    }

    // Delete the first temporary file
    g_objFSO.DeleteFile(strTempFile1, true);

    // Re-open the input file as the output.
    ContextSet("opening the output file: " + strFile);
    var tsFileOut = g_objFSO.CreateTextFile(strFile, true);

    // Read the first temporary file, writing columns 10+ to output file
    ContextSet("writing unique lines from temporary file to output file");
    var iDefect = 0;
    var tsIn = g_objFSO.OpenTextFile(strTempFile2, 1, true); // ForReading = 1
    while (!tsIn.AtEndOfStream)
    {
        var strLine = tsIn.ReadLine();
        var strLineUniq = ++iDefect + "\t" + strLine.substr(9);
        tsFileOut.WriteLine(strLineUniq);
    }
    tsIn.Close();
    delete tsIn;

    // Delete the second temporary file
    g_objFSO.DeleteFile(strTempFile2, true);

    // Close the output text stream
    tsFileOut.Close();
    delete tsFileOut;

    // Indicate success
    return 0;
}

var g_nCoverageSeq;

/////////////////////////////////////////////////////////////////////////////
//
// Helper function for DoFlatten, DoRemoveDups, and DoMerge
// Flattens file strFileIn and appends the results to fileOut,
// which must already be open.
// The fRemoveDups flag governs which XSL stylesheet is used.
// Also appends coverage messages to objCoverage if it is not null
//
function FlattenToFile( fRemoveDups, strFileIn, fileOut, objCoverage )
{
    // Open the specified defect log
    ContextSet("opening the specified defect log file: " + strFileIn);
    var doc = GuardedCreateObject("Microsoft.XMLDOM");
    if (!doc.load(strFileIn))
    {
        var parseError = doc.parseError;
        var strErr = "Error loading " + strFileIn + "\n";
        strErr += parseError.reason   + "\n"
                  strErr += parseError.srcText;
        Echo(0, strErr);
        return 2;
    }

    if ( fRemoveDups )
        strFileStyleSheet = g_strPREfastXSLFlattenSeq;
    else
        strFileStyleSheet = g_strPREfastXSLFlatten;

    // Write coverage info, if any
    if ( objCoverage != null )
    {
        var objCoverageNodes = doc.selectNodes("/DEFECTS/DEFECT[(DEFECTCODE=98101) or (DEFECTCODE=98102)]");
        var objNode;
        for ( objCoverageNodes.reset(), objNode = objCoverageNodes.nextNode();
              objNode != null;
              objNode = objCoverageNodes.nextNode()
            )
        {
            objNode.setAttribute("_seq", ++g_nCoverageSeq );
            objCoverage.documentElement.appendChild( objNode );
        }
    }

    // Open the flatten transformation
    ContextSet("opening the flatten transformation style sheet: " + strFileStyleSheet);
    var docSheet = GuardedCreateObject("Microsoft.XMLDOM");
    if (!docSheet.load(strFileStyleSheet))
    {
        var parseError = doc.parseError;
        var strErr = "Error loading " + strFileStyleSheet + "\n";
        strErr += parseError.reason   + "\n"
                  strErr += parseError.srcText;
        Echo(0, strErr);
        return 2;
    }

    // Transform the defect log
    ContextSet("transforming the defect log to a flat file");
    var strOut = doc.transformNode(docSheet);

    // Write the transformed text to the output file
    ContextSet("writing the transformed text to the output file");
    fileOut.Write(strOut);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
function DoUnflatten(strFileIn, strFileOut)
{
    // Verify that the filenames were specified
    if (!strFileIn || !strFileOut || !strFileIn.length || !strFileOut.length)
        return SyntaxViolated("Not enough parameters specified for the UNFLATTEN command.");

    // Open the specified input file
    ContextSet("opening the specified flat file: " + strFileIn);
    var ts = g_objFSO.OpenTextFile(strFileIn, 1); // ForReading = 1

    // Create the output XML DOM
    ContextSet("creating an XMLDOM instance");
    var doc = GuardedCreateObject("microsoft.xmldom");
    doc.appendChild(doc.createProcessingInstruction("xml", 'version="1.0" encoding="UTF-8"'));
    var defects = doc.appendChild(doc.createElement("DEFECTS"));

    // Loop through each line of the input file
    ContextSet("looping through the input file");
    while (!ts.AtEndOfStream)
    {
        var strLine = ts.ReadLine();
        var arr = strLine.split("\t");

        var defect = defects.appendChild(doc.createElement("DEFECT"));
        defect.setAttribute("_seq", arr[0]);
        var sfa = defect.appendChild(doc.createElement("SFA"));
        sfa.appendChild(doc.createElement("LINE"       )).text = arr[ 1];
        sfa.appendChild(doc.createElement("COLUMN"     )).text = arr[ 2];
        sfa.appendChild(doc.createElement("FILENAME"   )).text = arr[ 3];
        sfa.appendChild(doc.createElement("FILEPATH"   )).text = arr[ 4];
        defect.appendChild(doc.createElement("DEFECTCODE" )).text = arr[ 5];
        defect.appendChild(doc.createElement("DESCRIPTION")).text = arr[ 6];
        defect.appendChild(doc.createElement("RANK"       )).text = arr[ 7];
        defect.appendChild(doc.createElement("MODULE"     )).text = arr[ 8];
        defect.appendChild(doc.createElement("RUNID"      )).text = arr[ 9];
        defect.appendChild(doc.createElement("FUNCTION"   )).text = arr[10];
        defect.appendChild(doc.createElement("FUNCLINE"   )).text = arr[11];
        var elemPath = defect.appendChild(doc.createElement("PATH"       ));
        if (arr[12] && arr[12].length)
        {
            var arrPath = arr[12].split(";");
            for (var iPath = 0; iPath < arrPath.length; ++iPath)
            {
                var strSFA = arrPath[iPath];
                if (strSFA && strSFA.length)
                {
                    var arrSFA = strSFA.split(",");
                    var elemSFA = elemPath.appendChild(doc.createElement("SFA"));
                    elemSFA.appendChild(doc.createElement("LINE"    )).text = arrSFA[0];
                    elemSFA.appendChild(doc.createElement("COLUMN"  )).text = arrSFA[1];
                    elemSFA.appendChild(doc.createElement("FILENAME")).text = arrSFA[2];
                    elemSFA.appendChild(doc.createElement("FILEPATH")).text = arrSFA[3];
                }
            }
        }
    }

    // Close the input text stream
    ts.Close();
    delete ts;

    // Delete the specified output file
    ContextSet("deleting the specified output file: " + strFileOut);
    if (g_objFSO.FileExists(strFileOut))
        g_objFSO.DeleteFile(strFileOut);

    // Save the new XML document to the specified output file name
    ContextSet("saving the XMLDOM instance to the specified output file: " + strFileOut);
    doc.save(strFileOut);

    // Indicate success
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoRemoveDups( arrFiles )
{
    ContextSet("Removing duplicate defects");

    // If there are no arguments, remove dups from the default log file
    if ( undefined == arrFiles || 0 == arrFiles.length )
    {
        arrFiles = new Array;
        arrFiles[0] = g_dictValidSwitches("LOG");
    }

    var nInputs = (arrFiles.length == 1) ? 1 : (arrFiles.length - 1);

    // Create a temporary file
    var strFileFlat = GetTempFileName();
    var fileFlat = g_objFSO.CreateTextFile( strFileFlat, true );

    var objCoverage = GetCoverageFile();
    g_nCoverageSeq = 0;
    // This is also where we write to the coverage file, if any
    // Flatten the input log(s) to the temporary file
    for ( var i = 0; i < nInputs; i++ )
    {
        var nResult;

        nResult = FlattenToFile(true, arrFiles[i], fileFlat, objCoverage);
        if (nResult)
            return nResult;
    }
    if ( objCoverage != null )
    {
        objCoverage.save( g_objProcEnv("PREFASTCOVERAGEFILE") );
    }

    fileFlat.Close();
    delete fileFlat;

    // Remove the duplicated from the temporary file
    nResult = RemoveDupsFromFlatFile( strFileFlat );
    if ( nResult )
        return nResult;

    // Unflatten the flat file to the specified output file
    nResult = DoUnflatten(strFileFlat, arrFiles[arrFiles.length - 1]);

    // Delete the temporary file
    ContextSet("deleting the temporary file: " + strFileFlat);
    g_objFSO.DeleteFile(strFileFlat, true);

    // Return the last result
    ContextPop();
    return nResult;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoModMan(strCmdLine)
{
    // Set the PREFASTCONFIG environment variable
    ValidateConfigFilePath(g_dictValidSwitches("CONFIGFILE"));

    // Launch the Configuration command line interface
    ContextSet("launching the PREfast Configuration command line interface");

    var strFullCmdLine = g_strPREfastUtil + " modman " + strCmdLine;

    var nRet = RunProcess( strFullCmdLine );
    if ( nRet != 0 )
    {
        Echo( 0, "ERROR: " + strFullCmdLine + " returned bad status '" + nRet + "'.\n" );
    }
    return nRet;
}

/////////////////////////////////////////////////////////////////////////////
//
// DoWriteExitFunctions
//
// Creates an xml model file at strXMLFile containing exit models
// for each function (one per line) in strTextFile.  This file
// can be added to the /MODELFILES argument.
//
function DoWriteExitFunctions( strTextFile, strXMLFile )
{
    // Verify that the filenames were specified
    if (!strTextFile || !strXMLFile || !strTextFile.length || !strXMLFile.length)
        return SyntaxViolated("Not enough parameters specified for the ExitFuncsFromText command.");
    
	// Create a helper object
	var objFSO = new ActiveXObject("Scripting.FileSystemObject");

	try
	{
		// Attempt to open the text file
		var streamInput = objFSO.OpenTextFile(strTextFile, 1, 0); // 1=ForReading, 0=ASCII
    }
    catch (e)
    {
        Echo( 0, "ERROR: could not read file '" + strTextFile + "'." );
        return 4;
    }

    try
    {
        // Attempt to open the XML file for writing
        var streamOutput = objFSO.CreateTextFile( strXMLFile, true, false ); // Allow overwriting, use ASCII (not unicode)
        streamOutput.WriteLine( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" );
        streamOutput.WriteLine( "<Models>" );
    } 
    catch (e)
    {
        Echo( 0, "ERROR: could not open file '" + strXMLFile + "' for writing." );
        return 4;
    }

	while (!streamInput.AtEndOfStream)
    {
    	var strLine = streamInput.ReadLine();
        var arrFuncs = strLine.split(/\s/);
        for ( var i in arrFuncs )
        {
            if ( arrFuncs[i].length == 0 )
                break;
            streamOutput.WriteLine( "  <Function name=\"" + arrFuncs[i] + "\">" );
            streamOutput.WriteLine( "    <FunctionProperties>"                  );
            streamOutput.WriteLine( "      <Terminates value=\"1\"/>"           );
            streamOutput.WriteLine( "    </FunctionProperties>"                 );
            streamOutput.WriteLine( "  </Function>"                             ); 
        }
    }

    streamOutput.WriteLine("</Models>");
    streamOutput.Close();
    streamInput.Close();

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
// DoReadExitFunctions
//
// Creates a text file at strTextFile containing the names of
// exit functions in strXMLFile, one function name per line
// Only exit functions are written; any other models in strXMLFile
// are ignored.
//
function DoReadExitFunctions( strXMLFile, strTextFile )
{
    // Verify that the filenames were specified
    if (!strTextFile || !strXMLFile || !strTextFile.length || !strXMLFile.length)
        return SyntaxViolated("Not enough parameters specified for the ExitFuncsFromText command.");
    
	// Create a helper object
	var objFSO = new ActiveXObject("Scripting.FileSystemObject");

    try
    {
        // Attempt to open the text file for writing
        var streamOutput = objFSO.CreateTextFile( strTextFile, true, false ); // Allow overwriting, use ASCII (not unicode)
    } 
    catch (e)
    {
        Echo( 0, "ERROR: could not open file '" + strTextFile + "' for writing." );
        return 4;
    }

    var docModels = GuardedCreateObject("Microsoft.XMLDOM");
    if (!docModels.load(strXMLFile))
    {
        var parseError = docModels.parseError;
        var strErr = "Error loading " + strXMLFile + "\n";
        strErr += parseError.reason   + "\n"
                  strErr += parseError.srcText;
        Echo(0, strErr);
        return 2;
    }

    objModels = docModels.selectNodes("Models/Function");
    for ( var i = 0; i < objModels.length; i++ )
    {
        objModel = objModels(i);
        objTerminates = objModel.selectSingleNode( "FunctionProperties/Terminates[@value=1]" );
        if ( objTerminates )
            streamOutput.writeLine( objModel.getAttribute("name") );
    }

    streamOutput.Close();
}


/////////////////////////////////////////////////////////////////////////////
//
function DoInstall( arrModules )
{
    var nRet = 0;
    ContextPush("Installing the specified modules");

    var strResult;
    var dictOldModules = GuardedCreateObject("Scripting.Dictionary");
    var strModule;
    var bFailure = false;

    // Read the existing modules.txt file
    var strModulesDirectory = g_objFSO.GetAbsolutePathName( g_objPREfast.ModulesDirectory );
    var strModulesFile = g_objFSO.BuildPath( strModulesDirectory, "modules.txt" );
    var streamModules = g_objFSO.OpenTextFile( strModulesFile, 1, true );  // 1 is for reading
    
    while ( !streamModules.AtEndOfStream )
    {
        strModule = streamModules.ReadLine();

        dictOldModules( strModule ) = 1;
    }

    // Close the file and reopen it for appending the names of new modules
    streamModules.Close();
    streamModules = g_objFSO.OpenTextFile( strModulesFile, 8, true ); // 2 is for appending

    for ( var i in arrModules )
    {
        strModule = arrModules[i];
        strResult = "";

        var strBaseName = g_objFSO.GetBaseName( strModule );
        if ( dictOldModules( strBaseName ) != 1 )
            streamModules.WriteLine( strBaseName );
        var strFolder = g_objFSO.GetParentFolderName( g_objFSO.GetAbsolutePathName( strModule ) );
        if ( strFolder != strModulesDirectory )
        {
            var strDestination = g_objFSO.BuildPath( strModulesDirectory, strBaseName );
            g_objFSO.CopyFile( strModule, strDestination, true );
        }
    }

    if ( bFailure )
        return 5;
    else
    {
        var strDefectDefs = g_objFSO.BuildPath( g_objPREfast.ScriptsDirectory, "DefectUI\\DefectDefs.xml" );
        if ( g_objFSO.FileExists( strDefectDefs ) )
        {
            // ensure that it is deleted, even if it is read-only
            g_objFSO.DeleteFile( strDefectDefs, true );
        }
        var strFullCmdLine = g_strPREfastUtil + " concatenate " + strDefectDefs;

        nRet = RunProcess( strFullCmdLine );
        if ( nRet != 0 )
        {
            Echo( 0, "ERROR: " + strFullCmdLine + " returned bad status '" + nRet + "'.\n" );
        }
    }
    ContextPop();

    return nRet;
}

/////////////////////////////////////////////////////////////////////////////
//
function DoSave()
{
    ContextSet("saving the specified command-line switches");

    var str = "The following switch defaults are in effect:\n";
    if (g_dictSpecdSwitches.Count)
        str += "(switches just saved are indicated with '*')\n";
    if (g_dictSavedSwitches.Count)
        str += "(switches saved earlier are indicated with '+')\n";
    Echo(1, str, true, false);

    for (var it = new Enumerator(g_dictValidSwitches); !it.atEnd(); it.moveNext())
    {
        var strSwitch = it.item();
        var strValue = g_dictValidSwitches(strSwitch);
        var strFormatted = "";
        var strOpposite = g_dictMutexSwitches( strSwitch );
        var boolNew = ( g_dictSpecdSwitches( strSwitch ) != undefined );
        var boolSaved = ( g_dictSavedSwitches( strSwitch ) != undefined );

        if ( strOpposite != undefined )
        {
            strFormatted = "/" + strSwitch;
            if ( boolNew )
            {
                g_objShell.RegWrite( g_strRegPathSAVE + strSwitch + "\\", "" );
                try
                {
                    g_objShell.RegDelete( g_strRegPathSAVE + strOpposite + "\\");
                }
                catch (e)
                {
                }
            }
        }
        else
        {
            strFormatted = "/" + strSwitch + "=" + strValue;
            if ( boolNew )
                g_objShell.RegWrite( g_strRegPathSAVE + "\\" + strSwitch + "\\", strValue );
        }

        // Print all the switch values except the false half of mutex switches
        if ( strOpposite == undefined || strValue == true )
        {
            var strPrepend = "  ";
            if ( boolNew )
                strPrepend = "* ";
            else if ( boolSaved )
                strPrepend = "+ ";
            strFormatted = strPrepend + strFormatted;
            Echo(1, strFormatted, false, false);
        }
    }

    Echo(1, "\n", true, false);

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
function DoUnSave()
{
    // Clear the default switches from the registry
    ContextSet("resetting default switches");

    var strSwitch;
    var arrSwitches = new VBArray(g_dictValidSwitches.Keys()).toArray();
    for ( var i in arrSwitches )
    {
        strSwitch = arrSwitches[i];
        try
        {
            strValue = g_objShell.RegDelete(g_strRegPathSAVE + strSwitch + "\\");
        }
        catch (e)
        {
        }
    }

    // Display a confirmation message
    Echo(1, "Switch settings have been restored to product defaults.", true, true);

    // Indicate success
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// Filter strFromFile according to the selected filter preset and
// output to strToFileName.
//
function DoFilter( strFromFile, strToFile )
{
    ContextSet("Filtering defect log file");
    // Verify that the filenames were specified
    if (!strFromFile || !strToFile.length)
        return SyntaxViolated("Not enough parameters specified for FILTER command.");

    var objDefectSet = LoadDefects( strFromFile, true );
    return objDefectSet.save( strToFile, true );
}

/////////////////////////////////////////////////////////////////////////////
//
function DoExec(strCommandName, strCommandArgs, dictSwitches)
{
    var buildresult = 0;

    // Validate that a command was specified
    if (!strCommandName || !strCommandName.length)
        return SyntaxViolated("No build command specified.");

    // Set the PREFASTCONFIG environment variable
    ValidateConfigFilePath(dictSwitches("CONFIGFILE"));

    // Set the PREFASTLOG environment variable
    var strLog = ValidateDefectLogPath(dictSwitches("LOG"));

    // Set the _DFA_MAX_PATHS_ environment variable
    g_objProcEnv("_DFA_MAX_PATHS_") = dictSwitches("MAXPATHS");

    // Set the _PREFAST_MAX_TIME_ environment variable
    g_objProcEnv("_PREFAST_MAX_TIME_") = dictSwitches("MAXTIME");

    // Set the PREFAST_INCREASE_HEAP environment variable
    g_objProcEnv("PREFAST_INCREASE_HEAP") = dictSwitches("INCREASEHEAP");

    // Set the _PRECEDENCE_STACK_THRESHOLD_ environment variable
    g_objProcEnv("_PRECEDENCE_STACK_THRESHOLD_") = dictSwitches("STACKHOGTHRESHOLD");

    // Set the _PRECEDENCE_flag_initcrits_ environment variable
    g_objProcEnv("_PRECEDENCE_FLAG_INITCRITS_") = dictSwitches("ENABLECRITSWARNING");

    // Set the _DFA_NEW_FAILURE_ environment variable
    g_objProcEnv("_DFA_NEW_FAILURE_") = dictSwitches("NEW_FAILURE");

    // Set the PREFAST_USER_CL_EXE environment variable
    g_objProcEnv("PREFAST_USER_CL_EXE") = dictSwitches("CL_EXE");

    // Set the PREFAST_EXCLUDE_LIST environment variable
    g_objProcEnv("PREFAST_EXCLUDE_LIST") = dictSwitches("EXCLUDE");

    // Set the PREFAST_MACRO environment variable
    if (dictSwitches("MACRO"))
        g_objProcEnv("PREFAST_MACRO") = "1";

    // Set the PREFAST_ONEPASS environment variable, if specified
    if (dictSwitches("ONEPASS"))
        g_objProcEnv("PREFAST_ONEPASS") = "1";

    // Reset the defect log, if specified
    if (dictSwitches("RESET"))
    {
        if (!dictSwitches("N"))
        {
            var result = DoReset(strLog, true);
            if (result)
                return result;
        }
        else
        {
            Echo(1, "<Reset the defect log>");
        }
    }

    // Make our parser the default by changing the PATH and BUILD_PATH environment variables
    g_objProcEnv("PREFAST_ADD_PATH")  = g_objPREfast.ScriptsDirectory + "\\interceptcl;";
    g_objProcEnv("PATH")              = g_objProcEnv("PREFAST_ADD_PATH") + g_objProcEnv("PATH");

    if (g_objProcEnv("BUILD_PATH") != "")
    {
        g_objProcEnv("BUILD_PATH")     = g_objProcEnv("PREFAST_ADD_PATH") + g_objProcEnv("BUILD_PATH");
    }

    // Get the filesize of the defect log prior to the launch of the specified command
    var cDefectsBefore = GetDefectCount(strLog);
    var strCommand = strCommandName + "\n" + strCommandArgs;

    // Flush the console echo
    Echo(0, "", false, false);

    // Launch the specified command
    ContextSet("launching the specified command");
    if (!dictSwitches("N"))
    {
        buildresult = RunProcess( strCommand, "\n" );
        if (buildresult != 0)
        {
            Echo( 1, "Error: The build command '" + strCommandName + "' returned status '" + buildresult + "'.\n");
        }
    }
    else
    {
        Echo(1, strCommand);
    }

    // Filter the log file, if specified
    ContextSet("Filtering the log file");
    if (dictSwitches("FILTER"))
    {
        if (!dictSwitches("N"))
        {
            if (g_objFSO.FileExists(strLog))
            {
                Echo(1, "Filtering the defect log...", true, false);
                DoFilter(strLog, strLog);
            }
        }
        else
        {
            Echo(1, "<filter the log file>");
        }
    }

    // Remove duplicate defects, if specified
    ContextSet("launching the REMOVEDUPS command");
    if (dictSwitches("REMOVEDUPS"))
    {
        if (!dictSwitches("N"))
        {
            if (g_objFSO.FileExists(strLog))
            {
                Echo(1, "Removing duplicate defects from the log...", true, false);
                DoRemoveDups();
            }
        }
        else
        {
            Echo(1, "<Remove duplicate defects>");
        }
    }

    // Get the filesize of the defect log following the launch of the specified command
    var cDefectsAfter = GetDefectCount(strLog);

    // Kick-off the feedback upload script
    if (g_objPREfast.IsFeedbackEnabled)
    {
        ContextSet("kicking-off the feedback upload script");
        if (dictSwitches("FEEDBACK"))
        {
            if (!dictSwitches("N"))
            {
                try
                {
                    // Pass some more usage information to the feedback script
                    g_objProcEnv("PREFAST_strCmdLine")       = g_strCmdLine;
                    g_objProcEnv("PREFAST_strSavedSwitches") = g_strSavedSwitches;
                    g_objProcEnv("PREFAST_build")            = g_objPREfast.InstalledBuildNumber;
                    g_objProcEnv("PREFAST_cwd")              = g_objFSO.GetAbsolutePathName(".");

                    // Asynchronously launch the feedback uploader script
                    var strCmdLine = "wscript.exe //NOLOGO //B //T:60 \"" +
                                     g_objPREfast.ScriptsDirectory + "\\fbup.js\" " + strLog;
                    g_objShell.Run(strCmdLine, 0, false);
                }
                catch (e)
                {
                    // We'll just stay quiet if this failed since the feedback
                    // mechanism is too much trouble if it gets 'noisy'.
                    if (g_objPREfast.IsDevelopmentEnvironment)
                        throw e; // Unless we're in the development environment!
                }
            }
        }
    }

    // List the defects, if specified
    ContextSet("listing the defect log as specified");
    if (dictSwitches("LIST"))
    {
        if (!dictSwitches("N"))
        {
            var result = DoList(strLog);
            if (result)
                return result;
        }
        else
        {
            Echo(1, "<List the defect log as text>");
        }
    }

    // View the defects, if specified
    ContextSet("viewing the defect log as specified");
    if (dictSwitches("VIEW"))
    {
        if (!dictSwitches("N"))
        {
            var result = DoView(strLog);
            if (result)
                return result;
        }
        else
        {
            Echo(1, "<View the defect log using the UI>");
        }
    }

    // Check for product updates, if specified
    ContextSet("checking for product updates as specified");
    var fNeedsUpdate;
    var strInstalled;
    var strLatest;
    if (dictSwitches("UPDATE"))
    {
        if (!dictSwitches("N"))
        {
            // Get the installed and latest versions of PREfast
            strInstalled = g_objPREfast.InstalledBuildNumber;
            strLatest = g_objPREfast.LatestBuildNumber;

            // Process only if both versions could be determined
            if ("????" != strInstalled && "????" != strLatest)
            {
                // Convert each build number string to a number
                var nInstalled = new Number(strInstalled);
                var nLatest = new Number(strLatest);

                // Check for updated PREfast components
                fNeedsUpdate = nLatest > nInstalled;
            }
        }
        else
        {
            Echo(1, "<Check for PREfast updates>");
        }
    }

    // Report if new defects were detected
    ContextSet("reporting the number of defects detected");
    if (!dictSwitches("N"))
    {
        if (cDefectsAfter > cDefectsBefore)
        {
            var str = Decorate(
                              "PREfast reported " + (cDefectsAfter - cDefectsBefore) +
                              " defects during execution of the command.", false, true) + "\n";
            if (!dictSwitches("LIST"))
                str += "Enter PREFAST LIST to list the defect log as text within the console.\n";
            str += "Enter PREFAST VIEW to display the defect log user interface.";
            Echo(1, str, true, false);
        }
        else if (cDefectsAfter == cDefectsBefore)
        {
            Echo(1, "No defects were detected during execution of the command.", true, true);
        }

        // Indicate if an update exists
        if (fNeedsUpdate)
        {
            // Report that updates are needed
            ReportUpdateNeeded(strInstalled, strLatest);
        }
    }

    // Indicate success
    return buildresult;
}



/////////////////////////////////////////////////////////////////////////////
//
function LoadDefects( strLog, fFiltered )
{
    // Set the PREFASTLOG environment variable
    strLog = ValidateDefectLogPath(strLog, true);

    // Open the defect log
    ContextSet("opening the specified defect log file: " + strLog);
    var objDefectSet = GuardedCreateObject("PREfast.DefectSet");
    try
    {
        objDefectSet.load(strLog);
    }
    catch (e)
    {
        var strErr = "Error loading " + strLog + "\n";
        strErr += e.description;

        throw strErr;
    }

    if ( fFiltered )
    {
        if ( g_objFilter == null )
        {
            LoadPresets();
            g_objFilter = g_objPresets.selectedPreset;
        }
        objDefectSet.filter = g_objFilter;
    }

    return objDefectSet;
}

/////////////////////////////////////////////////////////////////////////////
//
function LoadPresets()
{
    if ( g_objPresets == null )
        g_objPresets = GuardedCreateObject("PREfast.DefectFilterPresets");

    return g_objPresets;
}

/////////////////////////////////////////////////////////////////////////////
//
function ValidateXMLPath(strFilename, strFileType, fReadOnly)
{
    ContextPush("validating the specified file: \"" + strFilename + "\"");

    // Append an ".xml" extension if the filename has no extension
    var iPeriod = strFilename.lastIndexOf('.');
    var iBSlash = strFilename.lastIndexOf('\\');
    if (-1 == iPeriod || iBSlash > iPeriod)
        strFilename += ".xml";

    // Get the absolute path name of the specified file
    strFilename = g_objFSO.GetAbsolutePathName(strFilename);
    ContextPop();
    ContextPush("validating the specified file: \"" + strFilename + "\"");

    // Currently, it doesn't matter if the file exists if we only read from it
    if (!fReadOnly)
    {
        try
        {
            // Ensure that we can write to the file by attempting to rename it
            if (g_objFSO.FileExists(strFilename))
            {
                // Attempt to rename the file
                var objFile = g_objFSO.GetFile(strFilename);
                var strName = objFile.Name;
                var strTemp = g_objFSO.GetTempName();
                objFile.Name = strTemp;
                objFile.Name = strName;
            }
            else
            {
                // Attempt to create the file
                g_objFSO.CreateTextFile(strFilename).Close();
            }
        }
        catch (e)
        {
            var str =
            "The specified " + strFileType + " could not be written to:\n" +
            "\t" + strFilename + "\n\n" +
            "Please check that it is spelled correctly, that the directory exists,\n" +
            "and that you have write permissions to the directory/file.";
            throw str;
        }
    }
    ContextPop();

    return strFilename;
}

/////////////////////////////////////////////////////////////////////////////
//
function ValidateDefectLogPath(strDefectLog, fReadOnly)
{
    ContextPush("validating the specified defect logfile: \"" + strDefectLog + "\"");

    strDefectLog = ValidateXMLPath( strDefectLog, "defect log file", fReadOnly );

    // Set the %PREFASTLOG% environment variable
    g_objProcEnv("PREFASTLOG") = strDefectLog;

    ContextPop();

    // Return the absolute string
    return strDefectLog;
}

//////////////////////////////////////////////////////////////////////////////
//
function ValidateConfigFilePath( strConfigFile )
{
    ContextPush("validating the specified config file: \"" + strConfigFile + "\"");

    strConfigFile = ValidateXMLPath( strConfigFile, "configuration file", true );

    // Set the PREFASTCONFIG environment variable
    g_objProcEnv("PREFASTCONFIG") = strConfigFile;

    ContextPop();

    //return the absolute string
    return strConfigFile;
}

/////////////////////////////////////////////////////////////////////////////
//
function GetDefectCount(strFile)
{
    ContextPush("creating 'microsoft.xmldom' component");
    var objXMLDoc = GuardedCreateObject("microsoft.xmldom");
    ContextPop();
    try
    {
        objXMLDoc.async = false;
        objXMLDoc.validateOnParse = false;
        if (!objXMLDoc.load(strFile))
            return 0;
        return objXMLDoc.selectNodes("DEFECTS/DEFECT").length;
    }
    catch (e)
    {
        return 0;
    }
}


/////////////////////////////////////////////////////////////////////////////
//
function GetCoverageFile()
{
    ContextSet("Creating the Coverage File XML object");
    var strCoverageFile = g_objProcEnv("PREFASTCOVERAGEFILE");
    var objCoverageFile = null;
    if ( !strCoverageFile )
    {
        return null;
    }

    if ( g_objFSO.FileExists( strCoverageFile ) )
    {
        g_objFSO.DeleteFile( strCoverageFile );
    }
    
    objCoverageFile = GuardedCreateObject("Microsoft.XMLDOM");
    objCoverageFile.appendChild(
                objCoverageFile.createProcessingInstruction("xml", 'version="1.0" encoding="UTF-8"'));
    objCoverageFile.appendChild( objCoverageFile.createElement("DEFECTS"));

    return objCoverageFile;
}
            


/////////////////////////////////////////////////////////////////////////////
//
function GetTempFileName()
{
    // TemporaryFolder is special folder #2
    var strTempFolder = g_objFSO.GetSpecialFolder(2).Path + "\\";
    return strTempFolder + g_objFSO.GetTempName();
}

var g_dictPREfastObjects = null;

/////////////////////////////////////////////////////////////////////////////
//
function GuardedCreatePREfastObject(strProgID)
{
    try
    {
        if ( null == g_dictPREfastObjects )
        {
            g_dictPREfastObjects = GuardedCreateObject( "Scripting.Dictionary" );
            g_dictPREfastObjects("PREfast.ExecEnvironment") = "pftExecEnv.wsc";
            g_dictPREfastObjects("PREfast.DefectSet") = "pftDefectSet.wsc";
            g_dictPREfastObjects("PREfast.DefectFilterPresets") = "pftDefectFilterPresets.wsc";
            g_dictPREfastObjects("PREfast.SortKeys") = "pftSortKeys.wsc";
            g_dictPREfastObjects("PREfast.DefectFilter") = "pftDefectFilter.wsc";
            g_dictPREfastObjects("PREfast.DefectRegExpDefs") = "pftDefectRegExpDefs.wsc";
            g_dictPREfastObjects("PREfast.DefectDefs") = "pftDefectDefs.wsc";
        }

        var strFileName = g_dictPREfastObjects(strProgID);
        if ( null == strFileName )
            throw "Unknown PREfast component " + strProgID + "\n";

        var strPath = g_objProcEnv("PREFAST_ROOT");

    	var fDevEnv = false;
	    if (g_objProcEnv("PREFAST_DEVENV"))
		    if (0 != (new Number(g_objProcEnv("PREFAST_DEVENV"))))
			    fDevEnv = true;

        if ( fDevEnv )
            strPath = g_objFSO.BuildPath( strPath, "bin\\scripts" );
        else
            strPath = g_objFSO.BuildPath( strPath, "scripts" );
        strPath = g_objFSO.BuildPath( strPath, strFileName );
        if ( !g_objFSO.FileExists( strPath ) )
            throw "Required PREfast file '" + strPath + "' does not exist\n";

        var strArg = "script:" + strPath;
        var objNew = GetObject( strArg );

        if ( "undefined" != typeof( objNew.FinalConstruct ) )
            objNew.FinalConstruct( GuardedCreatePREfastObject, g_objPREfast );

        return objNew;
    }
    catch ( e )
    {
        Echo( 0, "while creating " + strProgID );
        throw e;
    }
}

/////////////////////////////////////////////////////////////////////////////
//
function GuardedCreateObject(strProgID)
{
    ContextPush("creating " + strProgID + " component");
    var obj;

    try
    {
        // Create the object and return it
        if ("prefast." == strProgID.substr(0, 8).toLowerCase())
            obj = GuardedCreatePREfastObject(strProgID);
        else
            obj = new ActiveXObject(strProgID);
    }
    catch (e)
    {
        // Attempt to display a more informative message about the failure
        var strProduct;
        switch (strProgID.toLowerCase())
        {
        case "microsoft.xmldom":
            strProduct = "Microsoft XML 2.0 or greater";
            break;

        case "scripting.dictionary":
        case "scripting.filesystemobject":
            strProduct = "Microsoft Script Runtime 5.0 or greater";
            break;

        case "wscript.shell":
            strProduct = "Microsoft Windows Script Host 2.0 or greater";
            break;

        default:
            break;
        }

        // Echo the required product name, if known
        if (strProduct && strProduct.length)
            Echo(0, strProduct + " must be properly installed to use PREfast.", true, true);

        // Rethrow the exception
        throw e;
    }

    ContextPop();
    return obj;
}

/////////////////////////////////////////////////////////////////////////////
//
function QuoteArgs( strArgs, strSeparator )
{
    var bSlash;
    var bSpace;
    var strArg;

    if ( !strSeparator || 0 == strSeparator.length )
        return strArgs;

    var arrArgs = strArgs.split( strSeparator );
    for ( var i in arrArgs )
    {
        bSpace = false;
        bSlash = false;
        strArg = arrArgs[i];
        for ( var j = 0; j < strArg.length; j++ )
        {
            var c = strArg.charCodeAt( j );
            if ( c >= 0x09 && c <= 0x0D )
                bSpace = true;
            if ( c == 0x20 )
                bSpace = true;

            if ( bSlash )
                bSlash = false;
            else
            {
                if ( "\\" == strArg.charAt(j) )
                    bSlash = true;
            }
        }
        if ( bSpace )
        {
            if ( bSlash )
                arrArgs[i] = "\"" + strArg + "\\\"";
            else
                arrArgs[i] = "\"" + strArg + "\"";
        }
    }

    return arrArgs.join( " " );
}

var g_fReportedBroken = false;

/////////////////////////////////////////////////////////////////////////////
//
function RunProcess( strArgs, strSeparator )
{

    var strQuotedArgs = QuoteArgs( strArgs, strSeparator );

    if ( "undefined" == typeof( g_objShell.Exec ) )
    {
	if ( !g_fReportedBroken )
        {
            WScript.Echo("Upgrade to IE 6.0 or greater to run your build in the current window");
            g_fReportedBroken = true;
        }
        var strRunCommand = g_objProcEnv("COMSPEC") + " /c " + strQuotedArgs;
        return g_objShell.Run( strRunCommand, 1, true );
    }

    var strTempFolder = g_objFSO.GetSpecialFolder(2).Path;
    var strOutFile = g_objFSO.BuildPath( strTempFolder, g_objFSO.GetTempName() );    
    var strErrFile = g_objFSO.BuildPath( strTempFolder, g_objFSO.GetTempName() );
    // var strOutFile = g_objFSO.GetTempName();
    // var strErrFile = g_objFSO.GetTempName();
    var strCmd = g_objProcEnv("COMSPEC") + " /c " + strQuotedArgs + " > " + strOutFile + " 2> " + strErrFile;
    var status;
    var streamOut;
    var streamErr;
    var bReady;
	var oExec = g_objShell.Exec( strCmd );
	do		
	{
		status = oExec.status;		
		
		if ( undefined == streamOut )
		{
			try
			{
				streamOut = g_objFSO.OpenTextFile( strOutFile, 1, false );
			}
			catch (e)
			{
			}
		}
		if ( undefined == streamErr )
		{
			try
			{
				streamErr = g_objFSO.OpenTextFile( strErrFile, 1, false );
			}
			catch (e) {}
		}
		do
		{
			bReady = false;
			if ( undefined != streamOut && !streamOut.AtEndOfStream )
			{
                try
                {
				    WScript.StdOut.WriteLine( streamOut.ReadLine() );
                }
                catch (e) {} // We can't write to NUL, apparently
				bReady = true;
			}
			if ( undefined != streamErr && !streamErr.AtEndOfStream )
			{
                try
                {
				    WScript.StdErr.WriteLine( streamErr.ReadLine() );
                }
                catch (e) {} // We can't write to NUL
				bReady = true;
			}
		}
		while ( bReady );
		WScript.Sleep(50);
	} while ( 0 == status );

	if ( undefined != streamOut )	
		streamOut.Close();
	if ( undefined != streamErr )
		streamErr.Close();
	
	try
	{
		g_objFSO.DeleteFile( strOutFile );	
	}
	catch (e) {}
	try
	{
		g_objFSO.DeleteFile( strErrFile );
	}
	catch (e) {}

    return oExec.ExitCode;
}
/////////////////////////////////////////////////////////////////////////////
//
function ContextSet(strContext)
{
    g_arrContext[0] = strContext;
    g_arrContext.length = 1;
}


/////////////////////////////////////////////////////////////////////////////
//
function ContextPush(strContext)
{
    g_arrContext[g_arrContext.length] = strContext;
}


/////////////////////////////////////////////////////////////////////////////
//
function ContextPop()
{
    g_arrContext.length = g_arrContext.length - 1;
}


/////////////////////////////////////////////////////////////////////////////
//
function ContextTop()
{
    return g_arrContext.length ? g_arrContext[g_arrContext.length - 1] : null;
}


/////////////////////////////////////////////////////////////////////////////
//
var g_strHorizontalBar;
function GetHorizontalBar(fAppendLF)
{
    if (!g_strHorizontalBar)
        g_strHorizontalBar =
        "---------------------------------------" +
        "--------------------------------------";
    return fAppendLF ? g_strHorizontalBar + "\n" : g_strHorizontalBar;
}


/////////////////////////////////////////////////////////////////////////////
//
var g_fLastEchoNeedsBottomBar;
function Decorate(str, fTopBar, fBottomBar)
{
    // Modify string with horizontal bars, if specified
    if (fTopBar)
        str = GetHorizontalBar(str.length) + str;
    if (fBottomBar)
        str += "\n" + GetHorizontalBar(false);
    return str;
}


/////////////////////////////////////////////////////////////////////////////
//
function Echo(nDetailLevel, str, fTopBar, fBottomBar)
{
    // Do not display if detail level is higher than current verbosity level
    if (nDetailLevel > g_nVerbosity)
        return;

    // If the last echo needed a bottom bar, set fTopBar
    if (g_fLastEchoNeedsBottomBar)
        fTopBar = true;

    // Save the fBottomBar for next echo
    g_fLastEchoNeedsBottomBar = fBottomBar;

    // Decorate the text as specified
    str = Decorate(str, fTopBar, false);

    // Do not display if string is empty
    if (!str.length)
        return;

    // Normal processing for verbosity levels 0-2
    if (g_nVerbosity < 3)
    {
        WScript.Echo(str);
        return;
    }

    // Special processing for highest verbosity level
    var arr = str.split("\n");
    for (var i = 0; i < arr.length; ++i)
        arr[i] = nDetailLevel + ": " + arr[i];
    WScript.Echo(arr.join("\n"));
}


/////////////////////////////////////////////////////////////////////////////
//
function DisplayLogo()
{
    Echo(1,
         "Microsoft (R) PREfast Version " + g_objPREfast.ComposeVersionString() + " for 80x86.\n" +
         "Copyright (C) Microsoft Corporation. All rights reserved.",
         true, true);
}


/////////////////////////////////////////////////////////////////////////////
//
function SyntaxViolated(strDetail, fSuppress)
{
    if (!fSuppress)
    {
        strDetail =
        Decorate(strDetail, false, true) +
        "\n\n" +
        "Enter PREFAST /?   for quick syntax help.\n" +
        "Enter PREFAST HELP for product documentation.\n";
        Echo(0, strDetail, true, false);
    }
    return 2;
}

/////////////////////////////////////////////////////////////////////////////
//
function CheckJScriptVersion( nMajorNeeded, nMinorNeeded )
{
    var nMajor;
    var nMinor;
    var strHave;
    try
    {
        nMajor = ScriptEngineMajorVersion();
        nMinor = ScriptEngineMinorVersion();

        if ( nMajor > nMajorNeeded || ( nMajor == nMajorNeeded && nMinor >= nMinorNeeded ) )
        {
            // Version is ok
            return 0;
        }
        strHave = "JScript version " + nMajor + "." + nMinor;
    }
    catch ( e )
    {
        // ScriptEngineMajorVersion isn't even implemented until JScript version 5,
        // so we may come here on very old platforms.
        strHave = "A JScript version older than 5.0";

    }
    strError = "ERROR: PREfast requires at least JScript version " + nMajorNeeded + "." + nMinorNeeded + ".\n";
    strError += strHave + " is installed.\n";
    strError += "Upgrading to a current version of Internet Explorer will fix the problem.\n";
    Echo( 0, strError );
    WScript.Quit( 10 );
}


/////////////////////////////////////////////////////////////////////////////
//
function Syntax()
{
    //---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8
    var str =                                                                          //
        "The syntax of this command is:\n\n" +                                         //
                                                                                       //
        "    PREFAST /?\n" +                                                           //
        "    PREFAST /HELP\n\n" +                                                      //
                                                                                       //
        "    PREFAST HELP\n\n" +                                                       //
                                                                                       //
        "    PREFAST [/LOG=logfile] RESET\n\n" +                                       //
                                                                                       //
        "    PREFAST [/LOG=logfile]\n" +                                               //
        "            [/FILTERPRESET=filter preset name]\n" +                           //
        "            LIST\n\n" +                                                       //
                                                                                       //
        "    PREFAST [/LOG=logfile]\n" +                                               //
        "            [/FILTERPRESET=filter preset name]\n" +                           //
        "            VIEW\n\n" +                                                       //
                                                                                       //                                                                                       
        "    PREFAST [/LOG=logfile]\n" +                                               //
        "            [/LIST]\n" +                                                      //
        "            [/VIEW]\n" +                                                      //
        "            [/RESET]\n" +                                                     //
        "            [/STACKHOGTHRESHOLD=nnnn]\n" +                                    //
        "            [/FILTERPRESET=filter preset name]\n" +                           //
        "            build_command [arguments]\n";                                     //
                                                                                       //
        //---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8

    Echo(0, str);
    return 1;
}
