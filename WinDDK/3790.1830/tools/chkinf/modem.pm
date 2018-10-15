# Copyright 1999-2000 Microsoft Corporation
########################################################################
#
# Package with Identifier Types (do not change them, work like constants)
#
# Usage: $IdType'UNIMODEM_ID
#
########################################################################

package IdType;

  $UNIMODEM_ID  = "U";
  $PCMCIA_ID    = "P";
  $SERENUM_ID   = "S";
  $ISAPNP_ID    = "I";
  $BIOS_ID      = "B";
  $STAR_ID      = "*";
  $MF_ID        = "M";
  $UNKNOWN_ID   = "K";


########################################################################
#
# Package with Install Types (do not change them, work like constants)
#
# Usage: $InstallType'PCMCIA_INST
#
########################################################################

package InstallType;

  $PCMCIA_INST  = "P";
  $UNKNOWN_INST = "K";


package MODEM;          # The name of this module. For our purposes,
                        #  must be all in caps.

    use strict;                  # Keep this code clean
    my($Version) = "5.1.2250.0"; # Version of this file
    my($DEBUG)   = 0;            # Set to 1 for debugging output
                    
    sub FALSE { return(0); } # Utility subs
    sub TRUE  { return(1); }

    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my($inffile);         # Holds name of INF currently being processed.
    my($CmdOptions);      # Holds the commandline options specific
                          # to this modem device
    my($FaxCheck);

    my(%AllModels);       # Array of all models present in INF (Model) = (Type)
    my(%InstList);      # list of Install sections

    my(%AllIDList);         # ID => array of id definitons
                            # [ModelName, LineNumber, Rank, $OriginalName]
    my(%InstallIDTypes);
    my(%InstallTypeList);
    my(%AddRegList);
    my(%ATSyntaxErr);
    my(%VoiceInstallList);
    my(%InstallProps);
    my(%Inst_HKR);
    my(@InstallResponses);

    my(%ErrSpeedReport);

    my($bAllWarnings) = 0;
    my($bDupWarnings);
    my($bSecondDwordSpeed);

    my($AdvisoryMode) = 0;

    my($TotalTime) = time();
    sub SetATError {$ATSyntaxErr{$_[0]} = $_[1];}

    my($MaxIDLen);
    my(%GlobalR0List) = (); # should be empty when starting a chkinf run

    my(%ModemErrorTable) = (1027 => ["Blank identifier."],
                            1035 => ["Identifier %s not defined."],
                            1051 => ["The quotes surrounding the IDs should be removed."],
                            3033 => ["Illegal characters in string %s."],
                            3034 => ["Rank0 and Rank1 can't be the same."],
                            3035 => ["PCMCIA\\ prefix required for PCMCIA-modem id."],
                            3036 => ["PCMCIA id required."],
                            3038 => ["Star (*) format ID should be used for ISAPNP modems."],
                            3040 => ["Star (*) format ID should be used for BIOS modems."],
                            3042 => ["Keyword ADDREG not defined."],
                            3043 => ["UNIMODEM id is not of the expected length."],
                            3045 => ["Section [%s.NORESDUP] expected but missing."],
                            3046 => ["\"HKR, %s\" already defined in this section. Keep last definition."],
                            3047 => ["Mandatory field <Key> is missing."],
                            3049 => ["Mandatory field [Value-entry-name] should not be blank."],
                            3050 => ["%s does not have any Answer."],
                            3051 => ["%s does not have any Hangup."],
                            3052 => ["%s does not have any Init."],
                            3053 => ["%s does not have any Reset."],
                            3054 => ["More than one Reset in model %s."],
                            3055 => ["%s does not have any Monitor."],
                            3056 => ["%s does not have any Settings,Prefix."],
                            3057 => ["%s does not have any Settings,DialPrefix."],
                            3058 => ["%s does not have any Settings,DialSuffix."],
                            3059 => ["%s does not have any Settings,Terminator."],
                            3060 => ["%s does not have any Responses."],
                            3061 => ["%s does not have any Properties."],
                            3062 => ["%s has more than one \"Properties\" entry."],
                            3063 => ["%s does not have any InactivityScale."],
                            3064 => ["%s\'s properties expect a key \"Settings, %s\" which is not defined."],
                            3065 => ["%s: Command %s, level #%d not found."],
                            3066 => ["%s: Command %s, levels #%d-#%d not found."],
                            3067 => ["AT Command should not start with spaces"],
                            3068 => ["AT-Command \"None\" should not end with \"<CR>\"."],
                            3069 => ["AT-Command \"None\" suspicious."],
                            3070 => ["AT-Command \"NoResponse\" should not end with \"<CR>\"."],
                            3071 => ["AT-Command \"NoResponse\" suspicious."],
                            3072 => ["AT-Command string should start with \"AT\"."],
                            3073 => ["AT-Command string should end with \"<CR>\"."],
                            3074 => ["<CELLULAR_O> key - can no longer be used."],
                            3075 => ["Expect <Key> to be a quoted string."],
                            3076 => ["<Key> can't be empty string."],
                            3077 => ["Expect response to be in binary."],
                            3078 => ["Properties should have 8 DWORDS."],
                            3079 => ["InactivityScale should have 1 DWORD."],
                            3080 => ["<Value> is expected to have only hex digits."],
                            3081 => ["Expect <Value> to be a quoted string."],
                            3082 => ["<Value> can't be empty string."],
                            3084 => ["First field should be one of the HKR, HKU, HKLM, HKCR or HKCU keywords."],
                            3086 => ["Section [%s] is empty. It should contain \"*PNP0500\"."],
                            3087 => ["String \"*PNP0500\" is missing for section [%s]."],
                            3088 => ["Wrong strings in section [%s]. Only \"*PNP0500\" is allowed."],
                            3089 => ["A verbosity response of either V1 or V0 must be given for Init Settings"],
                            3090 => ["Empty INF file"],
                            3091 => ["UNIMODEM id required for MF-modems."],
                            3092 => ["%s has more than one \"DCB\" entry."],
                            3093 => ["DCB structure should have 26 bytes."],
                            3094 => ["%s has more than one \"DeviceType\" entry."],
                            3095 => ["%s does not have any DeviceType."],
                            3096 => ["Too many bytes defined for DeviceType entry."],
                            3097 => ["DeviceType should be 01, 02 or 03."],
                            3098 => ["230K port declared for external device %s."],
                            3099 => ["VoiceProfile should have 4 bytes."],
                            3100 => ["%s: Monitor suspicious. Expect 1: \"ATS0=0<cr>\" and 2: \"None\"."],
                            3101 => ["SERENUM modem: no unimodem ID is permitted for the same parameterization."],
                            3102 => ["Identifier %s, only UNIMODEM ids are allowed in NORESDUP section."],
                            3104 => ["Padded responses preferred"],
                            3105 => ["%s, duplicate rank 0 Id in file(s) \"%s\"."],
                            3106 => ["Voice modem \"%s\" (%s), minimal voice keys not present (Enumerator, VoiceProfile)."],
                            3108 => ["PnP ID declared rank 1 or greater (probably rank 0). Use one PnP rank 0 ID per device."],
                            3109 => ["Responses should have 10 bytes."],
                            3114 => ["Wrong value for response state."],
                            3115 => ["Wrong value for negotiated option."],
                            3123 => ["Missing %d basic numeric responses: %s."],
                            3124 => ["\"V1\" found but verbose responses are missing."],
                            3125 => ["\"V\" or \"V0\" found but numeric responses are missing."],
                            3127 => ["Missing %d basic verbose responses: %s."],
                            3128 => ["%d modems have the same name \"%s\" under manufacturer \"%s\"."],
                            3129 => ["Additional response \"%s\" should be given when stripped responses are used."],
                            3130 => ["Caller ID enabled in voice profile but \"EnableCallerID\" key not found."],
                            3131 => ["Distinctive ring enabled in voice profile but \"EnableDistinctiveRing\" key not found."],
                            3132 => ["\"VariableTerminator\" key needed if Caller ID or Distinctive ring enabled."],
                            3133 => ["Generate Digit enabled in voice profile but \"GenerateDigit\" key not found."],
                            3134 => ["Handset enabled in voice profile but \"%s\" key not found."],
                            3136 => ["SpeakerPhone enabled in voice profile but \"%s\" key not found."],
                            3137 => ["SpeakerPhoneSpecs should have 16 bytes."],
                            3139 => ["Mute enabled in voice profile but \"%s\" key not found."],
                            3140 => ["Voice enabled in voice profile but \"%s\" key not found."],
                            3142 => ["Set Baud Rate bit enabled in voice profile but \"VoiceBaudRate\" key not found."],
                            3143 => ["Parameter Strings \"%s\" not allowed in \"%s\" section."],
                            3144 => ["\"%s\" model is missing \"%s\" AddReg section entry required for CallerID modems."],
                            3145 => ["AddService directive not present in \"%s\" section."],
                            3146 => ["ServiceType directive should be 1 for the Service Install section."],
                            3147 => ["StartType directive should be 0,1,2,3 or 4 for the Service Install section."],
                            3148 => ["ErrorControl directive should be 0,1,2 or 3 for the Service Install section."],
                            3149 => ["ServiceBinary directive should not be empty for the Service Install section."],
                            3150 => ["LoadOrderGroup directive should be equal to \"Base\" for the Service Install section."],
                            3151 => ["Service Install section not present for the AddService directive."],
                            3152 => ["\"%s\" [CopyFiles]  not correct as defined in DestinationDirs section."],
                            3153 => ["XformID for WaveDriver should be double byte entry and not dword."],
                            3200 => ["PortDriver entry should be wdmmdmld.vxd for driverbased modems and serial.vxd for controllered modems."],
                            3206 => ["Voice Modem is missing Enumerator Key as one of the Voice Addreg Entries."],
                            3207 => ["Connect response is not defined with valid Responses key."],
                            3208 => ["AT E1 should not be part of Init Command."],
                            3209 => ["%s: Init not robust. Expect \"%s\" also."],
                            );

    my(%ModemWarningTable)=(2011 => ["White spaces in identifier \"%s\"."],
                            4011 => ["Install Sections may be inconsistent."],
                            4013 => ["%s : only one PnP id type is recommended."],
                            4015 => ["%s: VoiceView may not be supported in future."],
                            4016 => ["%s: Answer suspicious. Expect \"ATA<cr>\"."],
                            4017 => ["%s: Hangup suspicious. Expect \"ATH<cr>\"."],
                            4018 => ["%s: Init not robust. Expect \"%s\" also."],
                            4019 => ["%s: Reset suspicious. Expect \"ATZ<cr>\" or \"AT&F..<cr>\"."],
                            4020 => ["%s: DCE speed exceeds maximum value (%d) given in Properties."],
                            4021 => ["%s: Settings, Prefix suspicious. Expect \"AT\"."],
                            4022 => ["%s: Settings, DialPrefix suspicious. Expect \"D\"."],
                            4023 => ["%s: Settings, DialSuffix suspicious. Expect \"\" or \";\"."],
                            4024 => ["%s: Settings, Terminator suspicious. Expect \"<cr>\"."],
                            4025 => ["%s: Same <Value> for at least two of SpeakerVolume_XXX Settings."],
                            4026 => ["%s: Same <Value> for at least two of SpeakerMode_XXX Settings."],
                            4027 => ["%s: Same <Value> for Compression_On/Off Settings."],
                            4028 => ["%s: Same <Value> for ErrorControl_On/Off/Forced Settings."],
                            4029 => ["%s: VoiceView strings discontinued."],
                            4030 => ["%s: Same <Value> for ErrorControl_Cellular/Cellular_Forced Settings."],
                            4031 => ["%s: Same <Value> for at least two of the FlowControl_XXX Settings."],
                            4032 => ["%s: Same <Value> for Modulation_XXX Settings."],
                            4033 => ["%s: Possible inversion of <Value> between Settings Modulation_CCITT/Bell."],
                            4034 => ["%s: Same <Value> for SpeedNegotiation_On/Off Settings."],
                            4035 => ["%s: Possible inversion of <Value> between Settings SpeedNegotiation_On/Off."],
                            4036 => ["%s: Same <Value> for Tone/Pulse Settings."],
                            4037 => ["%s: Possible inversion of <Value> between Settings Tone/Pulse."],
                            4038 => ["%s: Same <Value> for Blind_On/Off Settings."],
                            4039 => ["%s: Possible inversion of <Value> between Settings Blind_On/Off."],
                            4040 => ["Command \"%s\" should not terminate in AT<cr>."],
                            4041 => ["If Rockwell chipset, enable ForceBlindDial & SetBaudBeforeWave bits. Otherwise, ignore warning."],
                            4042 => ["SVD key(s) illegal."],
                            4043 => ["Inactivity timer value is not 0, 90 or 255."],
                            4044 => ["Setup timer value is not 0 or 255."],
                            4045 => ["Second dword used to report the speed. Usually first dword is used."],
                            4046 => ["\"Error control\" bit is set, may be OK, but usually means only compression."],
                            4047 => ["Too many numeric responses (verbose setting in Init)."],
                            4048 => ["Too many verbose responses (numeric setting in Init)."],
                            4049 => ["Responses matching pattern \"%s\" not found for speeds %s."],
                            4050 => ["Since padded responses preferred, this response is not needed"],
                            4051 => ["%s: DTE speed exceeds maximum value (%d) given in Properties."],
                            4052 => ["Separate sound mixer chip required."],
                            4053 => ["Obsolete key."],
                            4054 => ["The response suggests one of %s states, while first byte gives \"%s\"."],
                            4055 => ["PnP id \"%s\" requires ExcludeFromSelect. Vendors or INF floppy install users may ignore this message."],
                            4056 => ["Numeric response does not agree with the response state."],
                            4057 => ["The response contains no keyword for the state 0x%02x (%s)."],
                            4058 => ["The response contains no keyword for \"%s\" option (0x%02x)."],
                            4059 => ["The response suggests \"%s\", but the corresponding bit is not set."],
                            4060 => ["\"Negotiated options\" byte should not be used for \"%s\" state."],
                            4061 => ["Speeds should not be used for \"%s\" state."],
                            4062 => ["Speed value in response string is not %d."],
                            4063 => ["Speed %d reported but dword value is missing."],
                            4064 => ["Response \"%s\" duplicated in line %d."],
                            4065 => ["Key \"%s\" not needed, Caller ID is not enabled in voice profile."],
                            4066 => ["Key \"%s\" not needed, distinctive ring is not enabled in voice profile."],
                            4067 => ["Key \"%s\" not needed, generate digit is not enabled in voice profile."],
                            4068 => ["Key \"%s\" not needed, handset is not enabled in voice profile."],
                            4069 => ["Key \"%s\" not needed, speakerphone is not enabled in voice profile."],
                            4070 => ["Key \"%s\" not needed, mute is not enabled in voice profile."],
                            4071 => ["Key \"%s\" not needed, serial wave is not enabled in voice profile."],
                            4072 => ["Key \"%s\" not needed, voice is not enabled."],
                            4073 => ["Voice Profile doesn't have NT5 bit set."],
                            4074 => ["Voice bit (0x00000001) must be enabled."],
                            4103 => ["SERENUM modem and matched model %s: friendly names are the same."],
                            4107 => ["WIN9x Specific: Key value should be \"serwave.vxd\"."],
                            4110 => ["Duplicated Responses found in one or more responses. Use \"/DC ALLWARNINGS\" switch for details."],
                            4111 => ["Second Dword of Response used as speed in one or more responses. Use \"/DC ALLWARNINGS\" switch for details."],
                            4112 => ["UNIMODEM id required for PCMCIA-modems on Windows 2000 or Windows 98."],
                            4113 => ["UNIMODEM id required for SERENUM-modems on Windows 2000 or Windows 98."],
                            4114 => ["UNIMODEM id required for STAR-modems on Windows 2000 or Windows 98."],
                            4115 => ["Section [%s.POSDUP] expected but missing for Windows 2000 or Windows 98."],
                            4116 => ["PortDriver entry is ignored on NT-based versions of Windows."],
                        );


    my(@VoiceViewStrings) = (   "APPS_DESC",
                                "StartUp_DESC",
                                "SendTo_DESC",
                                "FileXfer_DESC",
                                "SendToPhone_DESC",
                                "StartXfer_DESC"
                            );

    my(%InstallExtensions) = (  "WIN"       => "WIN",
                        "NT"        => "NT",
                        "NTX86"     => "NTX86",
                        "NTMIPS"    => "NTMIPS",
                        "NTALPHA"   => "NTALPHA",
                        "NTIA64"   => "NTIA64",
                        "NTPPC"     => "NTPPC" );


    my(%NumRespMatching)    = ( 0 => 0x00, 1 => 0x02, 2 => 0x08, 3 => 0x04, 4 => 0x03,
                        5 => 0x02, 6 => 0x05, 7 => 0x06, 8 => 0x07 );

#   ResponseState   => [ [keyword, Flag] ]

# Matching Flags
# 0 (most of them) - if matching, property is set
# 1 (like in "COMPRESSION : NONE") - if mathcing, property shouldn't be set
# 2 (MNP 5, CLASS 5, V42BIS) - implies compression as well, informative,
#                               works only with compression, should not covere
#                               the others, low priority

    my($ForceNotSet) = 1;       # if matching, the response is no,
    my($DependentOnCompression)  = 2;

    my(%NegotiatedOptionComment) = (    0x01    => "Compression",
                0x02    => "Error control",
                0x08    => "Cellular protocol"
            );

    my(%NegotiatedOptionLookup)     = (
            # Compression
                    0x01    =>  [   ['\bADC\b', 0],
                                ['\bCLASS *5\b', 0],
                                ['\bCOMP(RESS(ED|ION))?\b', 0],
                                ['\bNONE\b', $ForceNotSet],
                                ['\bEC(L|LC|\-DC)\b', 0],
                                ['\bMNP *(LEVEL)? *5\b', 0],
                                ['\bLAPM-V\b', 0],
                                ['\bREL *(\-)? *5\b', 0],
                                ['\bV\.?42 *B(IS)?\b', 0],
                                ['\bVBIS\b', 0],
                                ['\bX\.?70 *(BTX|NL)\b', 0],
                                ['\bX\.?75\b', 0],
                            ],
            # Error control
                    0x02    =>  [   ['\bALT(ERNATE|\-EC)?\b', 0],
                                ['\bARQ\b', 0],
                                ['\bCLASS *[2-4]\b', 0],
                                ['\bCLASS *5\b', $DependentOnCompression],
                                ['\bEC(L|LC|\-DC)?\b', 0],
                                ['\bERROR *\-? *CONTROL\b', 0],
                                ['\bLAP[_\-]?[MB]\b', 0],
                                ['\bLAPM[_\-]?V\b', 0],
                                ['\bLAP[_\-]?M[_\-]?EC\b', 0],
                                ['\bMNP *(LEVEL)? *[1-4]\b', 0],
                                ['\bMNP(?! *5)', 0],
                                ['\bMNP *5\b', $DependentOnCompression],
                                ['\bPAD\b', 0],
                                ['\bREL *[1234]\b', 0],
                                ['\bREL(C|IABLE)?', 0],
                                ['\bRLP\b', 0],
                                ['\bPPP\b', 0],
                                ['\bSLIP\b', 0],
                                ['\bV( *|\.)?120\b', 0],
                                ['\bV\.?14\b', 0],
                                ['\bV\.?42(?!( *B(IS)?))', 0],
                                ['\bV\.?42 *B(IS)?\b', $DependentOnCompression],
                                ['\bX\.?70 *(BTX|NL)\b', 0],
                                ['\bX\.?(75|25)\b', 0],
                            ],
            # Cellular protocol
                    0x08    =>  [   ['\bCELL(ULAR)?\b', 0],
                                ['\bMNP10\b', 0],
                                ['\bETC\b', 0],
                                ['\bLAP[_\-]?M[_\-]?EC\b', 0],
                                ['\bALT\-EC\b', 0],
                            ],
                        );

    my(%ResponseStateComment) = (   0x00    => "OK",
                0x01    => "Negotiation Progress",
                0x02    => "Connect",
                0x03    => "Error",
                0x04    => "No Carrier",
                0x05    => "No Dialtone",
                0x06    => "Busy",
                0x07    => "No Answer",
                0x08    => "Ring",
                0x09    => "VoiceView: SSV",
                0x0A    => "VoiceView: SMD",
                0x0B    => "VoiceView: SFA",
                0x0C    => "VoiceView: SRA",
                0x0D    => "VoiceView: SRQ",
                0x0E    => "VoiceView: SRC",
                0x0F    => "VoiceView: STO",
                0x10    => "VoiceView: SVM",
                0x18    => "Single Ring",
                0x19    => "Double Ring",
                0x1a    => "Triple Ring",
                0x1b    => "Sierra DTMF",
                0x1c    => "Blacklisted",
                0x1d    => "Delayed",
                0x91    => "Ring duration",
                0x92    => "Ring break",
                0x93    => "Date",
                0x94    => "Time",
                0x95    => "Number",
                0x96    => "Name",
                0x97    => "Message",
                0x9e    => "Diag",
            );
    my(%ResponseStateLookup)        = (
                #OK
                    0x00    =>  [   ['^OK$', 0],
                                ['^\#?VCON$', 0],
                                ['^VOICE$', 0],
                            ],
                #Negotiation Progress
                    0x01    =>  [   ['^\#V(IL|UR)$', 0],
                                ['^<cr>$', 0],
                                ['^<lf>$', 0],
                                ['^<ff><cr>$', 0],
                                ['^<H00>$', 0],
                                ['^CARRIER\b', 0],
                                ['^CED$', 0],
                                ['^COMPRESSION\b', 0],
                                ['^\+MRR', 0],
                                ['^\+MCR', 0],
                                ['^\+ER\b', 0],
                                ['^\+DR\b', 0],
                                ['^MODULATION\b', 0],
                                ['^DIALING$', 0],
                                ['^ERRM *=', 0],
                                ['^RING(ING| *BACK)$', 0],
                                ['^RRING$', 0],
                                ['\bSPEED *SETUP\b', 0],
                                ['^OFF *HOOK$', 0],
                                ['^PROTOCOL(?!.*\bFAX\b)\b', 0],        # but no FAX after that
                                ['^AUTOSTREAM\b', 0],
                                ['^NOT *USED\b', 0],
                                ['^VOICE$', 0],
                                ['^\+ILRR', 0],
                            ],
                #Connect
                    0x02    =>  [   ['^CONNECT\b', 0],
                                ['^CLIENT *SERVER$', 0],
                                ['^\+MRR', 0],
                            ],
                #Error
                    0x03    =>  [   ['\+F4\b', 0],
                                ['(\+|\-|\b)FC( *ERROR)?\b', 0],
                                ['^\+FCON$', 0],
                                ['^ABOR(D|TED)$', 0],
                                ['\bBLACKLIST', 0],
                                ['^DATA$', 0],
                                ['\bDELAY(ED)?\b', 0],
                                ['^ERROR$', 0],
                                ['\bFAIL(ED)?\b', 0],
                                ['\bFAX\b', 0],
                                ['^LOCKED$', 0],
                                ['\bNO *USER\b', 0],
                                ['\bSTOP\b', 0],
                                ['\bTIMEOUT\b', 0],
                                ['\bUN *(\-)? *OBTAINABLE', 0],
                                ['^VOICE$', 0],
                                ['\bWAIT\b', 0],
                            ],
                # No Carrier
                    0x04    =>  [   ['^NO *CARRIER$', 0],
                                ['^DISCONNECT$', 0],
                            ],
                # No Dialtone
                    0x05    =>  [   ['^NO *DIAL *TONE$', 0],
                            ],
                # Busy
                    0x06    =>  [   ['^BUSY$', 0],
                            ],
                # No Answer
                    0x07    =>  [   ['^NO *ANSWER$', 0],
                            ],
                # Ring
                    0x08    =>  [   ['^RING(ING)?$', 0],
                                ['^<HFF>$', 0],
                                ['^DIAL *IN$', 0],
                                ['^(<H00>)?CLIENT$', 0],
                                ['^<H10>R$', 0],
                            ],
                # VoiceView
                    0x09    =>  [   ['\bSSV\b', 0],
                            ],
                    0x0A    =>  [   ['\bSMD\b', 0],
                            ],
                    0x0B    =>  [   ['\bSFA\b', 0],
                            ],
                    0x0C    =>  [   ['\bSRA\b', 0],
                            ],
                    0x0D    =>  [   ['\bSRQ\b', 0],
                            ],
                    0x0E    =>  [   ['\bSRC\b', 0],
                            ],
                    0x0F    =>  [   ['\bSTO\b', 0],
                            ],
                    0x10    =>  [   ['\bSVM\b', 0],
                            ],
                # Single Ring
                    0x18    =>  [   ['^RING *(A|1)$', 0],
                            ],
                # Double Ring
                    0x19    =>  [   ['^RING *(B|2)$', 0],
                            ],
                # Triple Ring
                    0x1a    =>  [   ['^RING *(C|3)$', 0],
                            ],
                # Sierra DTMF
                    0x1b    =>  [   ['^\#VD[\#\*0-9ABCDGT]$', 0],
                                ['^\#VI[RS]$', 0],
                                ['^\#VT[HO]$', 0],
                            ],
                # Blacklisted
                    0x1c    =>  [   ['\bBLACKLIST', 0],
                            ],

                # Delayed
                    0x1d    =>  [   ['\bDELAY(ED)?\b', 0],
                            ],

                # Voice: Ring duration
                    0x91    =>  [   ['^DRON *=$', 0],
                            ],
                # Voice: Ring break
                    0x92    =>  [   ['^DROF *=$', 0],
                            ],
                # Voice: Date
                    0x93    =>  [   ['^DATE *=$', 0],
                            ],
                # Voice: Time
                    0x94    =>  [   ['^TIME *=$', 0],
                            ],
                # Voice: Number
                    0x95    =>  [   ['^CALR *=$', 0],
                                ['^CID *=$', 0],
                                ['^NMBR *=$', 0],
                            ],
                # Voice: Name
                    0x96    =>  [   ['^NAME *=$', 0],
                            ],
                # Voice: Message
                    0x97    =>  [   ['^MESG *=$', 0],
                            ],
                # DIAG: Message
                    0x9e    =>  [   ['^DIAG$', 0],
                            ],
            );

    my(%ResponseSpeeds)     = ( 75  => 0, 240   => 0, 300   => 0, 600   => 0, 1200  => 0, 1275  => 0,
                2400    => 0, 2600  => 0, 4800  => 0, 5000  => 0, 7200  => 0, 7400  => 0, 9600  => 0,
                9800    => 0, 12000 => 0, 12200 => 0, 14000 => 0, 14400 => 0, 14600 => 0, 16000 => 0,
                16800   => 0, 17000 => 0, 19200 => 0, 19400 => 0, 21400 => 0, 21600 => 0, 21800 => 0,
                24000   => 0, 24200 => 0, 26400 => 0, 26600 => 0, 26800 => 0, 28800 => 0, 29000 => 0,
                31200   => 0, 31400 => 0, 32000 => 0, 33600 => 0,
                33800   => 0, 38400 => 0, 48000 => 0, 56000 => 0, 57600 => 0, 64000 => 0,
                76800   => 0, 115200    => 0, 230400    => 0, 112000    => 0, 128000    => 0
            );

    my(%CallerIDCheck);

    my(@MissingCallerIDKeys) = ("EnableCallerID", "CallerIDPrivate", "CallerIDOutside",
                        "VariableTerminator","DATE","TIME","NMBR","NAME","MESG");



########################################################################
#
# Package with Identifier Types (do not change them, work like constants)
#
# Usage: $IdType'UNIMODEM_ID
#
########################################################################

 my(%IdType) = (    "UNIMODEM_ID"   => "U", "PCMCIA_ID" => "P",
                    "SERENUM_ID"    => "S", "ISAPNP_ID" => "I",
                    "BIOS_ID"       => "B", "STAR_ID"   => "*",
                    "MF_ID"         => "M", "UNKNOWN_ID" => "K"
                );

########################################################################
#
# Package with Install Types (do not change them, work like constants)
#
# Usage: $InstallType'PCMCIA_INST
#
########################################################################

 my(%InstallType) = ( "PCMCIA_INST" => "P", "UNKNOWN_INST" => "K" );


                            1; # To tell require this file is okay.


###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                 INTERFACE SUBROUTINES                                         |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################
#-------------------------------------------------------------------------------------------------#
# Required subroutine! Used to verify a Directive as being device specific.                       #
#-- Exists ---------------------------------------------------------------------------------------#
sub DCDirective {
    my($Directive) = $_[1];

    return(0); # Only return 1 if validating the line
}


#-------------------------------------------------------------------------------------------------#
# Required subroutine! Allows us to verify that the module loaded correctly.                      #
#-- Exists ---------------------------------------------------------------------------------------#
sub Exists {
    print(STDERR "Module \"MODEM\" Loaded\n");
}

#-------------------------------------------------------------------------------------------------#
# Main Entry Point to Device Specific Module. REQUIRED!                                           #
#-- InfCheck() -----------------------------------------------------------------------------------#
sub InfCheck {

    my($Item);
    my($InstSection);
    my($key,$ModelInfo,$name,$ModelDescription);
    my($TotalElapsed,$StartTime,$EndTime,$ElapsedTime);

    print(STDERR "\tModem::InfCheck...\n") if ($DEBUG);

    # Propagate a list of all models found in the INF
    LearnModels();

    foreach $InstSection (keys %InstList)
    {
        if ( $inffile->sectionDefined($InstSection) )
        {   &InstProc($InstSection); }
        # default section used to add hardware registry keys
        if ( $inffile->sectionDefined($InstSection.'.HW') )
        {   &InstProc($InstSection.'.HW', 1);   }
    }

    &ModelIDTypes();

    &CheckInstSections;

    &CheckVoice;

    &CheckIds;

    #substitute manu and model names from Fstr into  ModelList.
    # modem specific

    my @lines = $inffile->getOrigSection("STRINGS");
    my $val;
    my $count = 0;
    my $linetxt;
    my $line;

    foreach $linetxt (@lines) {
        $line = $inffile->getIndex("STRINGS",$count);

        $count++, next if ($linetxt =~ /^\s*$/); # Skip if it's a blank line

        ($key,$val) = ChkInf::SplitLine("STRINGS",$count,$linetxt);
        if (defined ($val) ) {
            $val =~ /^\"(.*)\"$/;
            $val = $1 if defined ($1);
            if ($key !~ /Mfg/i)
            {
                AddModemError(3033,$line, $key)
                    if ($val =~ /[\^\/:\|"<>]|\.$/);
            }
            else
            {
                AddModemError(3033,$line, $key)
                    if ($val =~ /[\^\/:\|"<>]/);
            }
            
        }
        $count++;
    }


    if (($bDupWarnings == 1) && ($bAllWarnings == 0))
        { AddModemWarning(4110,$inffile->{totallines}+1);}
    if (($bSecondDwordSpeed == 1) && ($bAllWarnings == 0))
        { AddModemWarning(4111,$inffile->{totallines}+1);}

    &modem_section_usage;

}

#-------------------------------------------------------------------------------------------------#
# Early initiation of module. REQUIRED!                                                           #
#-- InitGlobals($inffile) ------------------------------------------------------------------------#
sub InitGlobals {
    # Is called when module is first included.
    # Use it to set your per-inf variables.
    $inffile = $_[1];
    $CmdOptions = $_[2];

    if (defined($CmdOptions))
    {
    #   print "\tChecking Modem infs with $CmdOptions commandline options...\n";
    }
    else
    { $CmdOptions = "WHQL"; }

    if ($CmdOptions =~ /ALLWARNINGS/i )
        { $bAllWarnings = 1;}

    if ($CmdOptions =~ /FAX/i )
        { $FaxCheck = 1;}
    else
        { $FaxCheck = 0;}

    if ( (uc $CmdOptions) =~ /\/A\s$/ )
    {
        $AdvisoryMode = 1;
    }
    $MaxIDLen  = 50;

    %InstallIDTypes=();    # This array has (ModelInstSec, IDsType) tuples.
                           # used for checking SERENUM and STAR modems

    %InstallTypeList=();       # This array has (ModelInstSec, InstallType) tuples.
    %AllIDList = ();

    %CallerIDCheck = ();

    %AllModels = ();

    %InstList = ();

    %AddRegList=();     #   InstallSection => [ RefHKRList, RefResponseStructs]
                #   RefHKRList is a hash indexed by first two keys in HKR line
                #   RefResponseStructs is a list of references to
                #           [ResponseStringIndex, LineNumber, RefResponseStruct]

    %ATSyntaxErr=();    # pairs of (line_no, err_no), used in CheckATString()


    %VoiceInstallList=();   # list of Install sections that has a minimum of keys to be considered
                # a valid Voice Install section
    %InstallProps   = ();   #   InstallSection =>
                #           [ [PropertiesStruct], [DCBStruct], [InitSettings], dwVoiceProfile ]

    %Inst_HKR = ();
    @InstallResponses = ();
    %ErrSpeedReport=();

    $bDupWarnings = 0 ;
    $bSecondDwordSpeed = 0;

    if ($FaxCheck == 1)
    {
        # Unfortunately, 'require' will fail if the Module can't be found in "PATH",
        #   so explicitly check to see if it exists (which requires path info), so
        #   search the each of the @INC paths for the module (ala DynaLoad package).
        #   This also allows for ChkINF to be invoked from an arbitrary 'current'
        #   directory.
        my $ModPath;
        foreach $ModPath (@INC) {
           if (-e "$ModPath\\faxreg.pm") {
              require "$ModPath\\FAXREG.pm";

              print "\tModule \"$ModPath\\FAXREG.pm\" Loaded\n";
              if(defined(&FAXREG::proc_faxreg_init))
              {
                  FAXREG::proc_faxreg_init();
              }
              last;
           }
        }
    }

    return(1);
}

#-------------------------------------------------------------------------------------------------#
# REQUIRED!                                                                                       #
#-- PrintDetails() -------------------------------------------------------------------------------#
sub PrintDetails {
    # Allows addition of device specific results to the detail section on the
    # INF summary.
    return(1);
}

#-------------------------------------------------------------------------------------------------#
# REQUIRED!                                                                                       #
#-- PrintHeader() --------------------------------------------------------------------------------#
sub PrintHeader {
    # Allows to add Device specific information to the top of the INF summary
    # Page.  The information here should be brief and link to detailed summaries
    # below. (Which will be writting using PrintDetials.
    return(1);
}

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                    WORK SUBROUTINES                                           |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################


#-------------------------------------------------------------------------------------------------#
# LearnModels - Returns an array of all models listed in the INF                                  #
#-- LearnModels() --------------------------------------------------------------------------------#
sub LearnModels {
    return() if (! $inffile->sectionDefined("Manufacturer") );
    my $Section = "Manufacturer";
    my $line;
    my @lines   = $inffile->getSection("Manufacturer");
    my $count   = 0;

    my($Directive, $Value);
    my(%Models);

    print(STDERR "\tInvoking Modem::LearnModels...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/); # Skip if it's a blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);

        # [Manufacturers] may not have both a directive & a value
        if ($Value =~ /^\s*$/) {
            $Value     = $Directive;
            $Directive = "";
        } else {
            $Value    =~  s/^\"(.*)\"/$1/;   # And Strip the Quotes!
        }

        if ( $inffile->sectionDefined($Value) ) {
            # Add the models from $INFSections{$Value} to our running list
            %Models = (%Models, ExpandModelSection($Value));
        }
        $count++;
    }

    return(%Models);
}

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                   HELPER SUBROUTINES                                          |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################

#-------------------------------------------------------------------------------------------------#
# This sub adds a new error to the list of errors for the file.                                   #
#-- AddModemError($ErrorNum, $LineNum, @ErrorArgs) -------------------------------------------------#
sub AddModemError {
    my($ErrorNum) = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@ErrorArgs)= @_;

    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    $info_err = $ModemErrorTable{"$ErrorNum"};

    if ( !defined($info_err) ) {
        $this_str = "Unknown error $ErrorNum.";
    } else {
        $format_err = $$info_err[0];
        $this_str = sprintf($format_err, @ErrorArgs);
    }
    ChkInf::AddDeviceSpecificError($LineNum, $ErrorNum, $this_str);
}

#-------------------------------------------------------------------------------------------------#
# This sub adds a new warning to the list of warnings for the file.                               #
#-- AddModemWarning($WarnNum, $LineNum, @ErrorArgs) ----------------------------------------------#
sub AddModemWarning {
    my($WarnNum)  = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@WarnArgs) = @_;

    $WarnArgs[0] = " " if (! defined($WarnArgs[0]));
    my($str, $this_str, $info_wrn, $format_wrn);

    $info_wrn = $ModemWarningTable{"$WarnNum"};

    if ( !defined($info_wrn) ) {
        $this_str = "Unknown warning $WarnNum.";
    } else {
        $format_wrn = $$info_wrn[0];
        $this_str = sprintf($format_wrn, @WarnArgs);
    }
    ChkInf::AddDeviceSpecificWarning($LineNum, $WarnNum, $this_str);
}

#-------------------------------------------------------------------------------------------------#
# Globals changed:
#   ModelList= hash table with $ModelName => ModelInfoReference
#           ModelInfoReference = [  $ModelDescription,
#                       $InstallSection,
#                       RefArrayWithRank_0_IDs,
#                       RefArrayWithRank_1_IDs,
#                       RefArrayWithRank_2_IDs,
#                       RefArrayWithLineNo,
#                       LastLineOfSection]
#           RefArrayWithRank_n_IDs = [array of rank n IDs found for the
#                           same model description], n =0,1,2
#
#-- ExpandModelSection($line, $Section) ----------------------------------------------------------#
sub ExpandModelSection {
    my $line;
    my $lcount    = 0;
    my $Section  =  shift;
    return unless ($inffile->sectionDefined($Section));
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;
    my %Models   =  ();
    my $TempVar;

    my($LHS, $RHS, $CLnum, $LLnum);
    my(%NewModelList, @id, $sz, $model, $ranks, $Type, $id_item, $ExcludeID, $ExcludeRef);
    my(%ModelErrorList, @ArrayRanks, $r, $ManuRef, $count, $ExcludeKey, $ExcludeVal);
    my($Directive, $Value);
    my($element,$inst,$i);

    %NewModelList = ();
    %ModelErrorList = ();

    foreach $linetxt (@lines) {
        $line=$inffile->getIndex($Section,$count);

        $lcount++, next if ($linetxt =~ /^\s*$/); # Skip if it's a blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$lcount,$linetxt);
        @id                 = ChkInf::GetList($Value,$inffile->getIndex($Section,$lcount));
        $sz = @id;

        $inst = shift(@id);
        $inst = uc $inst;
        $sz = @id;
        $model = $Directive;
        # check for the install section
        # possible to use extensions
        $InstList{$inst} = 1; # placeholder....

        {
            my($SectionsDefined, $Extension);
            $SectionsDefined = 0;

            if ($inffile->sectionDefined($inst))
            {   $SectionsDefined++; }

            foreach $Extension (keys %InstallExtensions)
            {
                if ( $inffile->sectionDefined($inst.".$Extension") )
                {
                    $SectionsDefined++;
                    $InstList{uc ($inst.".$Extension")} = 1;
                }
            }
        }

        $count = 0;
        foreach $ExcludeID (@id)
        {
            my $UCExcludeID = uc $ExcludeID;

            if ($ExcludeID !~ /^$/)
            {
                #   add to the list of all ids
                if (!defined($AllIDList{$UCExcludeID}))
                {
                    $AllIDList{$UCExcludeID} = [];
                }
                #   model, line, rank, original value
                push(@{$AllIDList{$UCExcludeID}}, [$model, $line, $count, $ExcludeID]);
            }
            $count++;   # this gives the rank of the ID
        }

        if (defined $NewModelList{uc $model})   # same modem with diff id.
        {
            my($ROldModel);
            #add the new rank0 & rank1id and proceed.

            $ROldModel = $NewModelList{uc $model}; # pointer to the model array

                # add new ranks
            for($r = 0; $r < 3; $r++)
            {
               push( @{$$ROldModel[2+$r]} , (defined($id[$r])) ? $id[$r] : "" );
            }

                #  Compare if same modem -> same Inst section.
            if ( ($inst ne $$ROldModel[1]) && ! defined($ModelErrorList{$$ROldModel[0]}) )
            {
                $ModelErrorList{$$ROldModel[0]} = 4011;
            }
                #  Add line number for ID definition.
            push(@{$$ROldModel[5]},  $line);
            $lcount++;
            next;
        }

            #       Encountered first time, add the Install section name.
        $NewModelList{uc $model} = [$Directive, $inst] ;

        for($r = 0; $r < 3; $r++)
        {
            push(@{$NewModelList{uc $model}}, [ (defined($id[$r])) ? $id[$r] : "" ]);
        }

            #       Add line number for ID definition.
        push(@{$NewModelList{uc $model}}, [$line]);


        $lcount++;
    }


    # report error for $sec
    foreach $model (keys %ModelErrorList)
    {
       AddModemWarning($ModelErrorList{$model}, $line-1, $model);
    }

    foreach $model (keys %NewModelList)
    {
        #       Add line number for Section.
        push(@{$NewModelList{uc $model}}, $line-1);
        # append the new model list to global model list.
        $AllModels{$model} =  $NewModelList{$model};
    }
    return(%NewModelList);
}
#-------------------------------------------------------------
#
# This proc processes the given Install section using the
# ModelKeys and the subs defined there.
#
# Globals changed:
#   $InstList   = hash table with $InstallSection => RefInstallInfo
#                           RefInstallInfo  = reference to a hash table with
#                                                   $InstallKeyword =>
#                                                   RefKeywordInfo
#                           RefKeywordInfo = [RefArrayWithKeywordValues,
#                           KeywordLine]
#                           RefArrayWithKeywordValues = List of values for the
#                           given keyword
#                                                       as found in INF
#
# Usage:
#   InstProc(expected_section_name, IsHWInstall)
#
#-------------------------------------------------------------
sub
InstProc
{
    my $Section     = shift;
    my $IsHWInstall = shift;
    my $count       = 0;
    my $line        = 0;
    my $linetxt;
    return unless ($inffile->sectionDefined($Section));
    my($sec_name, @sec_list);
    my($model, $InstKeywordsHash, $KeywordValuesArray);
    my(@lines)      = $inffile->getSection($Section);
    my($Directive, @Values, $Value);

    if (!defined($IsHWInstall))
    {   $IsHWInstall = 0; }         # used to mark and additional
                                    # default section used to add hardware
                                    # registry keys
                                    # we validate this section but nothing
                                    # more

    print(STDERR "\tInvoking InstProc (@_)...\n") if ($DEBUG);

    $InstKeywordsHash = {}; # reference to a new hash

    foreach $linetxt (@lines) {

        $count++, next if ($linetxt =~ /^\s*$/); # Skip if it's a blank line
        $line = $inffile->getIndex($Section, $count);

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$linetxt);
        @Values             = ChkInf::GetList(uc($Value),$inffile->getIndex($Section,$count));

    $Directive = uc $Directive;

        $KeywordValuesArray = ();
        ${$InstKeywordsHash}{$Directive} = [$KeywordValuesArray,    $line];


        @{$KeywordValuesArray} = @Values;
        ${$InstKeywordsHash}{$Directive} = [$KeywordValuesArray,    $line];

        $count++;
    }


    if ($IsHWInstall == 0)  #    is not a HW section
    {   $InstList{$Section} = $InstKeywordsHash;    }

}
#-------------------------------------------------------------
#
# Analyze rank ids for all models defined
#
# Globals changed:
#   ModelList
#
# Usage:
#   ModelIDTypes()
#
#-------------------------------------------------------------
sub
ModelIDTypes
{
my($ModelKey, $ModelInfo, $inst, $sz, $r);
my($id_item, @rank0, @rank1, @rank2, @lines, @rank, $Type);

print(STDERR "\tInvoking ModelIDTypes...\n") if ($DEBUG);

    while (($ModelKey, $ModelInfo) = each (%AllModels) )
    {   $inst   = $$ModelInfo[1];

        @rank0  = @{$$ModelInfo[2]};
        @rank1  = @{$$ModelInfo[3]};
        @rank2  = @{$$ModelInfo[4]};
        @lines  = @{$$ModelInfo[5]};

        $InstallTypeList{$inst} = $InstallType'UNKNOWN_INST; # placeholder....

        if (! defined ($InstallIDTypes{$inst}) )
        {   $InstallIDTypes{$inst} = "";
        }

        push(@{$ModelInfo}, "");

        # Upper case
        $sz = @rank0;       # all arrays should have the same length
        for($id_item = 0; $id_item < $sz; $id_item++)
        {   $rank0[$id_item] = uc ($rank0[$id_item]) ;
            $rank1[$id_item] = uc ($rank1[$id_item]) ;
            $rank2[$id_item] = uc ($rank2[$id_item]) ;

            @rank = ();
            push(@rank, $rank0[$id_item], $rank1[$id_item], $rank2[$id_item]);


            $Type = &GetRankType($inst, $lines[$id_item], @rank);


            $InstallIDTypes{$inst} .= $Type;

                $$ModelInfo[7] .= $Type;
        }
    }

}
#-------------------------------------------------------------
#
# Globals changed:
#
# Usage:
#   GetRankType($inst_section, $Cur_line_num, id_array);
#
# Returns:
#   Modem Type (PCMCIA, SERENUM, STAR, MF)
#-------------------------------------------------------------
sub
GetRankType
{
my($inst, $CLnum, @id ) = @_;
my($rank0, $rankNext, $sz, $Type, $TypeCur, $Count, $bError, $ExcludeRef);

        # Process Rank0 id
    $rank0 = $id[0];

    $Type = $IdType'UNKNOWN_ID;


    if ($rank0 !~ /^$/)
    {
        $Type = checkR0($rank0, $inst, $CLnum);


        AddGlobR0($rank0, $CLnum) if ($Type !~ /$IdType'UNKNOWN_ID/);

        AddModemError(3034, $CLnum)
                    if ($id[0] eq $id[1]);
    }

        # Process next ranks
    $Count = 1;
    $rankNext = $id[$Count];
    $bError = 0;
    while (1)
      { if (defined $rankNext)
          {
        $TypeCur = checkR0($rankNext, $inst, $CLnum);

        # establish the type as the first UNKNOWN

        if ($TypeCur ne $IdType'UNKNOWN_ID)
          {
            if ($Type ne $IdType'UNKNOWN_ID)
                # ambiguous rank type
            {
                AddModemError(3108, $CLnum)
                    if ($bError == 0);
                $bError = 1;
            }
             else
            { $Type = $TypeCur;
            }
          }
          }
        $Count++;
        last if $Count > 2; # At most three ranks allowed

        $rankNext = $id[$Count];
      }
    $Type;          # return value
}

#------------------------------------------------------------
# Checks validity of the R0/R1 id.
#   - check for correct length of Unimodem ID.
#   - returns a predefined constant corresponding to the type of ID
#   package $IdType
#       - UNIMODEM_ID
#       - PCMCIA_ID
#       - SERENUM_ID
#       - ISAPNP_ID     (not used anymore)
#       - STAR_ID
#       - BIOS_ID       (not used anymore)
#       - MF_ID
#       - UNKNOWN_ID
#
# Usage:
#       checkR0(ID, model, line_num)
#------------------------------------------------------------

sub checkR0
{
my($ID, $model, $Lnum) = @_;
my($Type);

    $Type = $IdType'UNKNOWN_ID;

    if ($ID =~ /^UNIMODEM(.*)/)         # id must be 8 chars
    {
        AddModemError(3043, $Lnum)
            if (length($1) != 8);
        $Type = $IdType'UNIMODEM_ID;
    }
    elsif ($ID =~ /^ISAPNP\\/)
    {
        AddModemError(3038, $Lnum);
    }
    elsif ($ID =~ /^SERENUM\\/)
    {
        $Type = $IdType'SERENUM_ID;
    }
    elsif ($ID =~ /^PCMCIA\\/)
    {
        $Type = $IdType'PCMCIA_ID;
    }
    elsif ($ID =~ /^BIOS\\/)
    {
        AddModemError(3040, $Lnum);
    }
    elsif ($ID =~ /^\*/)
    {
        $Type = $IdType'STAR_ID;
    }
    elsif ($ID =~ /^MF\\/)
    {
        $Type = $IdType'MF_ID;
    }

    $Type; # return value

}


#------------------------------------------------------------
# Puts the given R0 id in the list of global R0 ids.
#
# Clobals changed:
#       GlobalR0List
# Usage:
#       AddGlobR0(ID)
#------------------------------------------------------------
sub
AddGlobR0
{
my($ID, $CLnum) = @_;
my($val);

    print "$inffile: $ID\n" if ($DEBUG);
    if (defined ($val = $GlobalR0List{uc $ID}))
    {
        AddModemError(3105, $CLnum, $ID, $val);
        $GlobalR0List{uc $ID} = join(", ", $val, $inffile." (line $CLnum)");
    } else {
        $GlobalR0List{uc $ID} = $inffile." (line $CLnum)";
    }
}


#-------------------------------------------------------------
#
# Process the sections defined by ADDREG keyword for each install section.
#
# Usage:
#   CheckInstSections()
#
#-------------------------------------------------------------
sub
CheckInstSections
{
my($sec_name, @sec_list, $RefListKeywords, $RefSectList, $CLnum);
my($sec, $LLnum);
my($key, $value, $RefHashAddReg);

print(STDERR "\tInvoking CheckInstSections...\n") if ($DEBUG);

    while (($sec, $RefListKeywords) = each %InstList)
    {
        next if (! $inffile->sectionDefined($sec) );
        next if (! defined($LLnum = $inffile->getIndex($sec)) );


        if (!defined($RefSectList = ${$RefListKeywords}{'ADDREG'} ) )
        {
#           add_ref_err("Keyword ADDREG not defined.", $LLnum-1);
            AddModemError(3042, $LLnum-1);
            next;
        }

        @sec_list = @{$$RefSectList[0]};
        $CLnum = $$RefSectList[1];


        %Inst_HKR = ();
        @InstallResponses = ();

        foreach $sec_name (@sec_list)
        {
            next if ($sec_name =~ /^$/); # check for blank names.

            $sec_name = uc $sec_name;

            $sec_name =~ s/^[\s\t]*//; # remove blanks at ends.
            $sec_name =~ s/[\s\t]*$//;

            AddRegProc($sec_name);       # process the AddReg sections.

            # add new_HKR to the list of HKR for this model

            $RefHashAddReg = ${$AddRegList{$sec_name}}[0];
            while (($key, $value) = each(%{$RefHashAddReg}))
            {
                # The later entries can overwrite the former ones
                # automatically!

                $Inst_HKR{$key} = $value ;
            }
            if (defined(@{${$AddRegList{$sec_name}}[1]}))
            {
                push(@InstallResponses, @{${$AddRegList{$sec_name}}[1]});
            }

            # Flag Install section if PCMCIA
            if ($sec_name =~ /^PCMCIA$/)
            {   $InstallTypeList{$sec} = $InstallType'PCMCIA_INST; }
        }

        ProcInstall($CLnum, $sec);
    }


    #   if ($AdvisoryMode == 0)
    #   log only one error for speed if NOT in advisory mode
    {
        my($SpeedErrorKey, $RefSpeedError);

        while(($SpeedErrorKey, $RefSpeedError) = each %ErrSpeedReport)
        {
            my($count);
            $count = keys %{$$RefSpeedError[0]};
            if ($SpeedErrorKey =~ /^DCE/)
            {
                AddModemWarning(4020, $$RefSpeedError[3], "$count responses in $$RefSpeedError[1] models", $$RefSpeedError[2]);
                next;
            }
            if ($SpeedErrorKey =~ /^DTE/)
            {
                AddModemWarning(4051, $$RefSpeedError[3], "$count responses in $$RefSpeedError[1] models", $$RefSpeedError[2]);
                next;
            }
        }
    }
    print(STDERR "\tExiting CheckInstSections...\n") if ($DEBUG);
}

#-------------------------------------------------------------
#
# This sub processes the given section assuming it is an
# addreg section unless it is in the list AddREgList.
# Flags duplicate keys in the same section and finally
# adds the list of keys to AddRegList.
#
# Globals changed:
#       AddRegList
#
# Usage:
#       AddRegProc(section_name)
#
#-------------------------------------------------------------

sub
AddRegProc
{
    my($sec) = $_[0];
    return unless ($inffile->sectionDefined($sec));
    my($line,$key, $value, $UpperKey, %local_HKR, @local_Responses, $RefRespStruct);
    my(@lines)      = $inffile->getSection($sec);
    my $linetxt;
    my $count       = 0;
    $sec = uc $sec;

    return if (defined $AddRegList{$sec});      # already collected

    %local_HKR = ();
    @local_Responses = ();

    foreach $linetxt (@lines) {
        $line  =  $inffile->getIndex($sec, $count);
        $count++, next if ($linetxt =~ /^\s*$/); # Skip if it's a blank line

        ($key, $value) = &get_addreg_list($linetxt, $line);

        $line++,next if (!defined($key) || $key =~ /^$/);

        if ($key =~ /^Responses,/i)
        {
            $RefRespStruct  = parse_response($key, $value);
            if (defined($RefRespStruct))
            { push(@local_Responses, $RefRespStruct);   }
        }
        else
        {
        
            if ($key =~ /\"(.)*Connect(.)*\"/i) {AddModemError(3207,$line);}
        
            $UpperKey = uc $key;
            if ( defined($local_HKR{$UpperKey}) )
            {
    #           add_ref_err("\"HKR,$key\" already defined in this section.
    #           Keep last definition.", $CLnum);
                AddModemError(3046, $line, $key);
                $count++;
                next;
            }
            $local_HKR{$UpperKey} = $value;
        }
        $count++;
    }

    $AddRegList{$sec} = [\%local_HKR, \@local_Responses];
}

sub CheckHKey {
    my($Lnum, @list) = @_;

    # warning if not HKR, HKU, HKLM, HKCR, HKCU
    if ($list[0] !~ /^HKR$/i &&
        $list[0] !~ /^HKU$/i &&
        $list[0] !~ /^HKLM$/i &&
        $list[0] !~ /^HKCR$/i &&
        $list[0] !~ /^HKCU$/i ) {
        AddModemError(3084, $Lnum);
    }
}

#-------------------------------------------------------------
# this sub takes the given line and breaks it into a list of five
# fields. It checks the fields for existance and correct range.
#
# Returns: array of four elemnts - excluding the HKR key.
#
# Usage:
#       get_addreg_fields(line, line_num)
#
#-------------------------------------------------------------
sub
get_addreg_list
{
my($line, $Lnum) = @_;
my(@list,@tmp,$field);
my(@tmp2, $StrResp,$first, $second);

    @list = ();

# This is a fix if the quoted string for the response contains a ',' then the fields for response
# get messed up and so the quoted string is temporarily replaced with STRRESP and then copied back
# after splitting for comma

    if ($line =~ /Responses/i)
    {    
        @tmp2 = split('"', $line);
        $StrResp = $tmp2[1];
        $line = "";
        $line .= $tmp2[0];
        $line .="STRRESP";
        $line .= $tmp2[2];
    }

    @tmp = split(',', $line, 5);
    # remove whitespaces from all.
    foreach $field (@tmp)
    {
        $field =~ s/^[\s\t]*//;
        $field =~ s/[\s\t]*$//;
        if ($field =~ /STRRESP/) 
        { 
            $first .= "\"";
            $first .= $StrResp;
            $first .= "\"";
            $field = $first;
        }
        push(@list, $field);
    }

    &CheckHKey($Lnum, @list);

    if ($list[0] !~ /^HKR$/i)
    {   return () ; }

    # Field 2 Optional - check not required.
    # Field 3 Mandatory - check for existance.
    AddModemError(3047, $Lnum)
        if ($list[2] =~ /^$/);

    # Field 4 If not present - make it 0; if present ensure 0 or 1.

    $list[3] = '0'
        if ((! defined $list[3]) or ($list[3] =~ /^$/));



#   REMOVING THIS ERROR BECAUSE Format COULD be 0x10001
#   add_ref_err("Field &ltFormat&gt should be 0 or 1.", $Lnum)

    # Field 5 Mandatory - check for existance.
    if ((! defined $list[4]) or ($list[4] =~ /^$/))
    {
#       add_ref_err("Mandatory field &ltValue&gt is missing.", $Lnum);
        AddModemError(3049, $Lnum);
        $list[4] = "";
    }

    $list[3] = join(":", $Lnum, $list[3]);

    ( join(",", $list[1], $list[2]) , join(",", $list[3], $list[4]) ); #return value.
}


#-------------------------------------------------------------
# This sub processes the $HKR assuming it to be a whole list
# of HKRs of all sections put together.
#
# Globals changed:
#
# Usage:
#       ProcInstall(line_num, section)
#       Install_HKR has all keys and line_num points to the AddRegKey.
#-------------------------------------------------------------
sub
ProcInstall
{
my($Lnum, $model) = @_;
my(@ResponseStructs);
my($key,$val,@X);
my($VoiceProfile);

print(STDERR "\tInvoking ProcInstall($model)...\n") if ($DEBUG);


    $VoiceProfile   = $InstallProps{uc $model}->[3];

    # check for the AT-Command syntax

    # Process the different types of HKR strings

    # this print is just to tell the user I am processing ....
    # print ".";

    # first get rid of the keys that we do lesser processing with.
    # this will remove the most of HKR keys
    @ResponseStructs = @InstallResponses;

    &proc_skip($Lnum, $model);
    &proc_SVDKeys($Lnum, $model);
    &proc_voice($Lnum, $model);
    &proc_vv($Lnum, $model);

    &proc_set($Lnum, $model);
    &proc_propset($Lnum, $model);
    &proc_DeviceType($Lnum, $model);    # to be called before proc_DCB
    &proc_DCB($Lnum, $model);

    &proc_init($Lnum, $model);
    &proc_ans($Lnum, $model);
    &proc_hup($Lnum, $model);
    &proc_mon($Lnum, $model);
    &proc_reset($Lnum, $model);

    &proc_fax($Lnum, $model);

    &proc_resp($Lnum, $model, @ResponseStructs);    # to be called last

     # added checking for only caller id enabled modems since VariableTerminator key is present 
     # for modems that support distinctive ring and no caller id
    if ((defined($CallerIDCheck{uc $model}))&&(defined($VoiceProfile)))
    {
        if (($CallerIDCheck{uc $model} != 0x000001ff)&&(($VoiceProfile & 0x00000080) == 0))
        {
            my($index,$checkmissing);
            $index = 0;
            $checkmissing = $CallerIDCheck{uc $model};
            while ($index < 9)
            {
                if (($checkmissing & 0x1) == 0)
                {
                    AddModemError(3144,$Lnum,$model,$MissingCallerIDKeys[$index]);
                }
                $index++;
                $checkmissing = $checkmissing >> 1;
            }
        }
    }

    while(($key, $val) = each %Inst_HKR)
    {
        @X =  split(/:/,$val);
#       inst_err("Old key - cant be used any more.", $X[0])
        AddModemError(3074, $X[0]) if ($key =~ /CELLULAR_O/);
    }
print(STDERR "\tExiting ProcInstall($model)...\n") if ($DEBUG);
}


#-------------------------------------------------------------
#
# This sub makes dwords out of every 4 hex bytes in the list
# of bytes and returns the list of dwords in hex.
#
# Globals changed:
#
# Usage:
#    hex_dwords_list = get_dwords(length_in_bytes, list of bytes, lineno)
#
#-------------------------------------------------------------
sub
get_dwords
{
my($multiple,$L, @list) = @_;
my($cnt,$i,$byte);

    foreach (@list) # make them sane.
    {
      #strip all of their whitspaces and add 0s if needed.
        s/^[\s\t]*//;
        s/[\s\t]*$//;
        $_ = join("", 0, $_) if (length($_) < 2);
        $_ = join("", 0, $_) if (length($_) < 2);
        if ($_ !~ /^[a-fA-F0-9][a-fA-F0-9]$/)
        {
            AddModemError(3080, $L);
            $_ = '00';
        }
    }

    my(@dw) = ();
    $cnt = 0;
    $i = 0;
    $dw[$i] = "";
    foreach $byte (@list)
    {
        $dw[$i] = join("", $byte, $dw[$i]);
        $cnt++;
        if (!($cnt % $multiple))
        {
            $i++;
            $dw[$i]="";
        }
    }
    @dw;
}

sub
parse_response
{
my($StrResp, $BinResp) = @_;
my($QuoteStr, $QuoteIndex, $ReturnArray);
my(@tmp,@tmp2,$L,$T);
my($ActualResp);

    @tmp = split(',', $StrResp, 2);
    $QuoteStr = $tmp[1];

    @tmp = split(',', $BinResp, 2);
    @tmp2 = split(':', $tmp[0], 2);
    $L =  $tmp2[0];
    $T =  $tmp2[1];


    if($QuoteStr !~ /^\"/)
    {
#       inst_err("Expect &ltKey&gt to be a quoted string.", $L);
        AddModemError(3075, $L);
        return undef;
    } else {
        # get the quoted string.
        #$QuoteIndex = $1;
        #$ActualResp = ChkInf::QuotedString($QuoteIndex);
    $QuoteStr =~ s/^\"//;
    $QuoteStr =~ s/\"$//;
        $ActualResp = $QuoteStr;
        if($ActualResp =~ /^$/)
        {
#           inst_err("&ltKey&gt cant be empty string.", $L);
            AddModemError(3076, $L);
            return undef;
        }
        $tmp[1] =~ s/[\s\t]//g; # get rid of all white spaces for efficient comparison later!
    }
    if ($T !~ /^1$/)
    {
    
    print "Binary Resp : $BinResp \n";
    print "String Resp : $StrResp \n";
#       inst_err("Expect response to be in binary.", $L);
        AddModemError(3077, $L);
        return undef;
    }

    my(@dwords, @bytes, $count);
    @bytes = split(',', $tmp[1]);
    if ( ($count = @bytes) != 10)
    {
        AddModemError(3109, $L);
        return undef;
    }

    @dwords = get_dwords(1,$L, splice(@bytes, 0, 2));
    @dwords = (@dwords, get_dwords(4, $L, @bytes) );
    my(@ResponseStruct, $dw);
    @ResponseStruct = ();
    foreach $dw (@dwords)
    {
        $dw =~ s/^\s+//;
        $dw =~ s/\s+$//;
        if ($dw !~ /^$/)
        {
            push(@ResponseStruct, hex($dw));
        }
    }
    $ReturnArray = [$ActualResp , $L, \@ResponseStruct];

    AnalyseResponse($ReturnArray);      # adds flag if numeric response

    $ReturnArray;
}


#############################################################################
sub
AnalyseResponse
{
my($RefResponse) = @_;
my($QuoteIndex, $RespLine, @ResponseStruct);
my($OptionBits, $ResponseString, $NumericResponse, $MatchingState,$SearchResp, $KeywordFound);
my($GoodState);

    $QuoteIndex = $$RefResponse[0];
    #$ResponseString = ChkInf::QuotedString($QuoteIndex);
    $ResponseString = $$RefResponse[0];
    return if (!defined($ResponseString));

    $RespLine   = $$RefResponse[1];
    @ResponseStruct = @{$$RefResponse[2]};

    #   sanity checking
    # DTE speed reported, put up a warning
    if ($ResponseStruct[3] != 0)
    {
        if ($bAllWarnings == 1)
        {   AddModemWarning(4045, $RespLine);}
        else
        {   $bSecondDwordSpeed = 1; }
    }

    # First to bytes are OK ?
    #   Response state : 0x00 to 0x08 (data/fax)
    #                    0x09 to 0x10 (voice)
    #                    0x18 to 0x1b and 0x91 to 0x97
    #   Option Bits  : 0x01 | 0x02 | 0x08

    $OptionBits = ~(0x01 | 0x02 | 0x08);

    $GoodState = 1;
    if ( !(  $ResponseStruct[0] <= 0x10 ||
            ($ResponseStruct[0] >= 0x18 && $ResponseStruct[0] <= 0x1d) ||
            ($ResponseStruct[0] >= 0x91 && $ResponseStruct[0] <= 0x97) ||
            ($ResponseStruct[0] == 0x9e)    #For Diag Responses
          ))
    {
        AddModemError(3114, $RespLine);
        $GoodState = 0;
    }

    AddModemError(3115, $RespLine)  if ($ResponseStruct[1] & $OptionBits);

    if ($GoodState && $ResponseStruct[1] != 0 && $ResponseStruct[0] != 0x01 && $ResponseStruct[0] != 0x02)
    {
            # bad usage of negotiated options for the response state
        AddModemWarning(4060, $RespLine, $ResponseStateComment{$ResponseStruct[0]});
    }


    if ( $GoodState && ($ResponseStruct[2] != 0 || $ResponseStruct[3] != 0) &&
          $ResponseStruct[0] != 0x01 &&  $ResponseStruct[0] != 0x02)
    {
            # bad usage of speed for the response state
        AddModemWarning(4061, $RespLine, $ResponseStateComment{$ResponseStruct[0]});
    }

    #   verify if numeric response
    $NumericResponse = 0;
    if ($ResponseString =~ /^(\d+)/)
    {
        $NumericResponse    = 1;
        #   check for usual values
        if (defined($MatchingState = $NumRespMatching{int($1)}) && $MatchingState != $ResponseStruct[0])
        {   AddModemWarning(4056, $RespLine);   }
        $$RefResponse[3] = [$NumericResponse, int($1)] ;
    }
    else
        { $$RefResponse[3] = [$NumericResponse]; }
    return if ($NumericResponse == 1);

    # check verbose responses
        # remove spaces <cr>, <lf>, <ff>
    $SearchResp = $ResponseString;
    if ($SearchResp !~ /^<(cr|lf|ff)>$/i &&
        $SearchResp !~ /^<ff><cr>$/i )
    {
        $SearchResp =~ s/<cr>/ /gi;
        $SearchResp =~ s/<lf>/ /gi;
        $SearchResp =~ s/<ff>/ /gi;
    }
    $SearchResp =~ s/^\s+//;
    $SearchResp =~ s/\s+$//;

                # check if ReponseState is correct (try to find a keyword in
                # response string)
    $KeywordFound = 0;
    if (($GoodState) && ($AdvisoryMode == 1))
    {
        my($RefArray);
        foreach $RefArray (@{$ResponseStateLookup{$ResponseStruct[0]}})
        {
            if ($SearchResp =~ /$$RefArray[0]/i)
            {
                $KeywordFound++;
                last;
            }
        }
        if ($KeywordFound == 0)
        {
            AddModemWarning(4057, $RespLine, $ResponseStruct[0], $ResponseStateComment{$ResponseStruct[0]});
#           printf "Wrong State: 0x%02x %s\n", $ResponseStruct[0],
#           $SearchResp;
        }
    }

            # The reverse, interpret the response and suggest the true
            # response state
    if ($AdvisoryMode == 1)    # check this only if in advisory mode
    {
        my  @PossibleStates = ();
        my  ($IsInList, $PossCount, $StateKey, $RefLookup);

        $IsInList = 0;
        while (($StateKey, $RefLookup) = each %ResponseStateLookup)
        {
            $KeywordFound = 0;
            my($RefArray);
            foreach $RefArray (@{$RefLookup})
            {
                if ($SearchResp =~ /$$RefArray[0]/i)
                {
                    $KeywordFound++;
                    last;
                }
            }
            if ($KeywordFound != 0)
            {
                push(@PossibleStates, $StateKey);
                if ($StateKey == $ResponseStruct[0])
                {   $IsInList = 1;  }
            }
        }
        if ($IsInList == 0 && (($PossCount = @PossibleStates) != 0))
        {
            my $ListOfStates = "";
            my $StateFound = "";

            $ListOfStates   = '"'.$ResponseStateComment{$PossibleStates[0]}.'"';
            shift(@PossibleStates);
            foreach $StateFound (@PossibleStates)
            {
                $ListOfStates   = $ListOfStates.", ". '"'.$ResponseStateComment{$StateFound}.'"';
            }
            AddModemWarning(4054, $RespLine, $ListOfStates,$ResponseStateComment{$ResponseStruct[0]});
#           printf "Suggested state 0x%02x: 0x%02x %s\n", $StateKey,
#           $ResponseStruct[0], $SearchResp;
        }
    }


            # Negotiated options
    if (($AdvisoryMode == 1) &&($ResponseStruct[1] != 0 || $ResponseStruct[0] == 0x01 || $ResponseStruct[0] == 0x02))
    {
        my($NegotiatedKey);

        foreach $NegotiatedKey (keys %NegotiatedOptionLookup)
        {
            my($CheckCompression,$RefArray);
            $KeywordFound = 0;
            $CheckCompression = 1;      # 0 if found at least one keyword not dependent
                                        # 1 if only dependent on compression
            foreach $RefArray (@{$NegotiatedOptionLookup{$NegotiatedKey}})
            {
                if ($SearchResp =~ /$$RefArray[0]/i)
                {
                    if ($$RefArray[1] & $ForceNotSet)
                    {
                        $KeywordFound = 0;
                        last;
                    }
                            # see if state is dependent on compression
                            # settings
                    if ( ($$RefArray[1] & $DependentOnCompression) == 0 )
                    {   $CheckCompression = 0;  }

                    $KeywordFound++;
                }
            }

                # check if Negotiated Options are correct (try to find a
                # keyword in response string)
            if (($ResponseStruct[1] & $NegotiatedKey) && $KeywordFound == 0)
            {
                AddModemWarning(4058, $RespLine, $NegotiatedOptionComment{$NegotiatedKey}, $NegotiatedKey);
#               printf "Wrong Option (1): 0x%02x $SearchResp\n",
#               $NegotiatedKey;
            }

            # cross check
            if ($KeywordFound != 0)     # something found
            {
                        # see if option set
                if ($CheckCompression != 0)
                {
                    # option not set is OK
                    if ($ResponseStruct[1] & $NegotiatedKey)
                    {
                        if ($ResponseStruct[1] & 0x01)
                                    # if option is set, then compression
                                    # should be set
                        {               # put an warning
                            AddModemWarning(4046, $RespLine);
#                           printf "Dependence: 0x%02x $SearchResp\n",
#                           $NegotiatedKey;
                        }
                        else            # like keyword not found
                        {
                            AddModemWarning(4058, $RespLine, $NegotiatedOptionComment{$NegotiatedKey}, $NegotiatedKey);
#                           printf "Wrong Option (2): 0x%02x $SearchResp\n",
#                           $NegotiatedKey;
                        }
                    }
                }
                elsif (($ResponseStruct[1] & $NegotiatedKey) == 0)  # option not set, keyword present
                {   AddModemWarning(4059, $RespLine, $NegotiatedOptionComment{$NegotiatedKey});
#                   printf "Missing Option: 0x%02x $SearchResp\n",
#                   $NegotiatedKey;
                }
            }
        }
    }

    # test if speed given in response equals that given in dwords
    if (($AdvisoryMode == 1) && ($ResponseStruct[0] == 0x01 ||  $ResponseStruct[0] == 0x02))
    {
        my($i);
        foreach $i (2,3)
        {
            my($SpeedString);

            next if ($ResponseStruct[$i] == 0);

            # verify the speed given in dword equals that from response
            $SpeedString = sprintf "%d", $ResponseStruct[$i];
            my (@SpeedFormats);

            push(@SpeedFormats, $SpeedString);  # normal format %d
            $SpeedString =~ /(.*)(\d\d\d)$/;    # comma format %d,%d
            my $value_to_push;
            (defined $1) ? $value_to_push="$1," : $value_to_push = " ,";
            (defined $2) ? $value_to_push.="$2" : $value_to_push.= " ";
            push(@SpeedFormats, "$value_to_push");
            if ($ResponseStruct[$i] >= 64000)
            {
                $SpeedString = sprintf "%d", int($ResponseStruct[$i]/1000);
                push(@SpeedFormats, $SpeedString."K");  # K format %dK (like in 112K)
            }

            $KeywordFound = 0;
            foreach $SpeedString (@SpeedFormats)
            {
                if ($SearchResp =~ /$SpeedString/i)
                {
                    $KeywordFound = 1;
                    last;
                }
            }

            if ($KeywordFound == 0)
            {
                AddModemWarning(4062, $RespLine, $ResponseStruct[$i]);
#               printf "Wrong Speed: %d in $SearchResp\n",
#               $ResponseStruct[$i];
            }
        }

        # missing speed report (dwords are zero while some speed reported in
        # response)
        if ($ResponseStruct[2] == 0 && $ResponseStruct[3] == 0)
        {
            my($SpeedValue);

            $SpeedValue = 0;
            # try to find the speed in 3 formats
            # \d+ (normal), \d+,\d\d\d, \d+K
            if ($SearchResp =~ /(\d+)K/i)
            {   $SpeedValue = int($1) * 1000;   }
            elsif ($SearchResp =~ /(\d+),(\d\d\d)/)
            {   $SpeedValue = int($1.$2);   }
            elsif ($SearchResp =~ /(\d+)([TR]X|)\/(\d+)([TR]X|)/)
            {   $SpeedValue = (int($1) > int($3)) ? int($1) : int($3);  }
            elsif ($SearchResp =~ /(\d+)/)
            {   $SpeedValue = int($1);  }
            if ($SpeedValue != 0 && defined($ResponseSpeeds{$SpeedValue}))
            {
                AddModemWarning(4063, $RespLine, $SpeedValue);
#               printf "Missing Speed: %d in $SearchResp\n", $SpeedValue;
            }
        }
    }

    # find response pattern
    if ( ($AdvisoryMode == 1) && ($ResponseStruct[0] == 0x01 ||  $ResponseStruct[0] == 0x02) &&
         ($ResponseStruct[2] != 0 || $ResponseStruct[3] != 0)
        )
    {
        my($SpeedFormat);
        $SpeedFormat = '\d+(?:(?:,\d\d\d)|(?:K\b)|(?:TX\/\d+RX)|(?:\/\d+)|)';
                                    # without ?: brackets are numbered (and
                                    # usege below would be wrong)
        # find the speed and keep the rest
        if ( $ResponseString =~ /^(\D*)($SpeedFormat)(.*)$/i )
        {   my($beginPattern, $endPattern, $FinalPattern);
            $beginPattern = $1;
            $endPattern = $3;
            $FinalPattern = str2regex(uc $beginPattern)."($SpeedFormat)".str2regex(uc $endPattern);
            $$RefResponse[4] = [$FinalPattern, $beginPattern."<speed>".$endPattern, $ResponseStruct[0]];
        }
    }
}
# translate a string in a regular expression replacing special character and
# spaces
sub
str2regex
{
my($string)=@_;
my(@metachars,$specchar);

    @metachars = ('\/', '\^', '\.', '\$', '\|', '\(', '\)', '\[', '\]', '\*', '\ +', '\?', '\{', '\}', '\-', '\,', '\<', '\>');

    $string =~ s/\\/\\/g;       # \ with \\
    foreach $specchar (@metachars)
    {   $string =~ s/$specchar/$specchar/g;
    }

    $string =~ s/\s+/ \*/g;     # \ with \\

    return $string;
}


#################################################################
#
# Utility subs.
#
#################################################################
#-------------------------------------------------------------
# Checks the Answer key
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------


sub
proc_ans
{
my($Lnum, $model) = @_;
my($count,@list,$this_val);
my($L,@tmp,@tmp2,$field);

    @list = getPathKey("Answer,");
    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Answer.", $Lnum);
        AddModemError(3050, $Lnum, $model);
        return;
    }
    # check for the AT-Command syntax
    CheckATCommands($model, "Answer,", @list);

    $this_val = &join_strval(@list);

    foreach $field (@list)
    {
        @tmp = split(',', $field, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
    }
#   inst_wrn("$model: Answer suspicious. Expect \"ATA&ltcr&gt\".", $L)
    AddModemWarning(4016, $L, $model) if ($this_val !~ /ATA<CR>$/);
}



#-------------------------------------------------------------
#
# Checks the Hangup key
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------

sub
proc_hup
{
my($Lnum, $model) = @_;
my(@list,$count,$this_val);
my($L,@tmp,@tmp2,$field);

    @list = getPathKey("Hangup,");
    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Hangup.", $Lnum);
        AddModemError(3051, $Lnum, $model);
        return;
    }

    # check for the AT-Command syntax
    CheckATCommands($model, "Hangup,", @list);

    $this_val = &join_strval(@list);
    foreach $field (@list)
    {
        @tmp = split(',', $field, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
    }
#   inst_wrn("$model: Hangup suspicious. Expect \"ATH&ltcr&gt\".", $L)
    AddModemWarning(4017, $L, $model)
        if ($this_val !~ /^ATH.*<CR>$/);

}



#-------------------------------------------------------------
# Checks the Init key
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_init
{
my($Lnum, $model) = @_;
my($count,@list,$this_val);
my($L,@tmp,@tmp2,$field);

    @list = getPathKey("Init,");
    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Init.", $Lnum);
        AddModemError(3052, $Lnum, $model);
        return;
    }

    # check for the AT-Command syntax
    CheckATCommands($model, "Init,", @list);

    # now the list has no duplicates.  Just multiple levels.
    # join all levels and see if we have a good enough init.
    # make sure that all levels are in strings.

    $this_val = &join_strval(@list);

    if (!defined($this_val))
    {
    $this_val = "";
    }
    foreach $field (@list)
    {
        @tmp = split(',', $field, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
    }
    #ensure that this_val is a strong and robust init.
    my($str);
    $str = "";


    $str = join("", $str, "AT") if ($this_val !~ /^AT/);
    $str = join("", $str, "&F") if ($this_val !~ /&F/);
    $str = join("", $str, "E0") if ($this_val !~ /E(\d*)/ || $1 !~ /^0?$/);
    $str = join("", $str, "&D2") if ($this_val !~ /&D2/);
    $str = join("", $str, "&C1") if ($this_val !~ /&C1/);
    $str = join("", $str, "S0=0") if ($this_val !~ /S0=0/);
    if ($str !~ /^$/) # found errs
    {
#       inst_wrn("$model: Init not robust. Expect \"$str\" also.", $L);
        AddModemError(3209, $L, $model, $str);
    }

    if ($this_val =~ /E1/)
    {
        AddModemError(3208, $L);
    }

    if ($this_val !~ /V(\d*)/ || $1 !~ /^(0|1)?$/)
    {
        AddModemError(3089, $L);
    }

    if (!defined($InstallProps{uc $model}))
    {
        $InstallProps{uc $model} = [];
    }

    ${$InstallProps{uc $model}}[2] = [($this_val =~ /V(\d*)/ && $1 =~ /^0?$/) ? 0 : 1 ];    # verbose mode
}



#-------------------------------------------------------------
# Checks the Reset key
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_reset
{
my($Lnum, $model) = @_;
my(@list,$count,$this_val);
my($L,@tmp,@tmp2,$field);

    @list = getPathKey(",Reset");
    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Reset.", $Lnum);
        AddModemError(3053, $Lnum, $model);
        return;
    } elsif (($count = @list) > 1)
    {
#       add_ref_err("More than one Reset in model $model.", $Lnum);
        AddModemError(3054, $Lnum, $model);
        return;
    }

    $this_val = &join_strval(@list);
    foreach $field (@list)
    {
        @tmp = split(',', $field, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
    }
#   inst_wrn("$model: Reset suspicious. Expect \"ATZ&ltcr&gt\" or
#   \"AT&F..&ltcr&gt\".", $L)
    AddModemWarning(4019, $L, $model)
        unless (($this_val =~ /^ATZ.*<CR>$/) || ($this_val =~ /^AT&F.*<CR>$/));
}



#-------------------------------------------------------------
# Checks the Monitor key
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_mon
{
my($Lnum, $model) = @_;
my(@list,$count,$this_val);
my($L,@tmp,@tmp2,$field);

    @list = getPathKey("Monitor,");
    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Monitor.", $Lnum);
        AddModemError(3055, $Lnum, $model);
        return;
    }

    # check for the AT-Command syntax
    CheckATCommands($model, "Monitor,", @list);

    $this_val = &join_strval(@list);
    foreach $field (@list)
    {
        @tmp = split(',', $field, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
    }
#   inst_err("$model: Monitor suspicious. Expect 1: \"ATS0=0&ltcr&gt\" and 2:
#   \"None\".", $L)
#   AddModemError(3153, $Lnum, $model), return unless defined $this_val;
    if (defined($this_val))
    {
    AddModemError(3100, $L, $model)
        unless ($this_val =~ /^ATS0=0<CR>NONE$/);
    }
}

# process mandatory settings fields here.

#-------------------------------------------------------------
# Checks the madatory Settings, * keys.
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_set
{
my($Lnum, $model) = @_;
my($this_val,@list,$count);

    @list = getPathKey("Settings,Prefix",1);

    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Settings,Prefix.", $Lnum);
        AddModemError(3056, $Lnum, $model);
    }
    else
    {
    $this_val = &join_strval(@list);
    if (!defined($this_val))
    {
        $this_val = "";
    }
#   inst_wrn("$model: Settings, Prefix suspicious. Expect \"AT\".", $L)
    AddModemWarning(4021, $Lnum, $model)
        unless ($this_val =~ /AT/);
    }

    @list = getPathKey("Settings,DialPrefix",1);

    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Settings,DialPrefix.", $Lnum);
        AddModemError(3057, $Lnum, $model);
    }
    else
    {
    $this_val = &join_strval(@list);
    if (!defined($this_val))
    {
        $this_val = "";
    }
#   inst_wrn("$model: Settings, DialPrefix suspicious. Expect \"D\".", $L)
    AddModemWarning(4022, $Lnum, $model)
        unless ($this_val =~ /D/);
    }

    @list = getPathKey("Settings,DialSuffix",1);
    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Settings,DialSuffix.", $Lnum);
        AddModemError(3058, $Lnum, $model);
    }
    else
    {
        $this_val = &join_strval(@list);
#   inst_wrn("$model: Settings, DialSuffix suspicious. Expect \"\" or \";\".",
#   $L)
        if (!defined($this_val))
        { $this_val = ""; }
        AddModemWarning(4023, $Lnum, $model)
        unless (($this_val =~ /^$/) || ($this_val =~ /;/));
    }

    @list = getPathKey("Settings,Terminator",1);
    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Settings,Terminator.", $Lnum);
        AddModemError(3059, $Lnum, $model);
    }
    else
    {
    $this_val = &join_strval(@list);
    if (!defined($this_val))
    {
        $this_val = "";
    }
#   inst_wrn("$model: Settings, Terminator suspicious. Expect \"&ltcr&gt\".",
#   $L)
    AddModemWarning(4024, $Lnum, $model)
        unless ($this_val =~ /<CR>/);
    }
}
#-------------------------------------------------------------
#
# This sub looks at the modem's properties and decides the
# relevent Settings keys it must find and validate.
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_propset
{
my($Lnum, $model) = @_;
my(@tmp, @tmp2,@list,$count,$L, $T);
my($line,@bytes);

    @list = getPathKey(",Properties",1);
    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any Properties.", $Lnum);
        AddModemError(3061, $Lnum, $model);
        return;
    } elsif (($count = @list) > 1)
    {
#       add_ref_err("$model has more than one \"Properties\" entry.", $Lnum);
        AddModemError(3062, $Lnum, $model);
        return;
    }

    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
        $T =  $tmp2[1];
        return if ($T !~ /^1$/);    # dont know what to do with binaries!!!!
    }

    @bytes = split(',', $tmp[3]);
    if (($count = @bytes) != 32)
    {
#       inst_err("Properties should have 8 DWORDS.", $L);
        AddModemError(3078, $L);
        return;
    }
    my($PropLine, @dwords);
    @dwords = get_dwords(4,$L, @bytes);

    if (!defined($InstallProps{uc $model}))
    {
        $InstallProps{uc $model} = [];
    }

    my(@PropStruct, $dw);
    @PropStruct = ();
    foreach $dw (@dwords)
    {   push(@PropStruct, hex($dw));    }
    ${$InstallProps{uc $model}}[0] = \@PropStruct;

    $PropLine = $L;
    &proc_callsetup($dwords[1], $Lnum, $PropLine, $model);
    &proc_inact($dwords[2], $Lnum, $PropLine, $model);
    &proc_spvol($dwords[3], $Lnum, $model);
    &proc_spmode($dwords[4], $Lnum, $model);
    &proc_mdop($dwords[5], $Lnum, $model);
}



#-------------------------------------------------------------
# Get the valid keys out - dont know how to process them!!
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_skip
{
my($Lnum, $model) = @_;
my(@list);
my($count,@tmp,$L,$T,@tmp2);

    @list = getPathKey(",FriendlyDriver");
    @list = getPathKey(",DevLoader");
    @list = getPathKey(",ConfigDialog");
    @list = getPathKey(",EnumPropPages");
    @list = getPathKey(",PortSubClass");
    @list = getPathKey(",PortDriver");
    if (($count = @list) != 0)
    {
        @tmp = ();
        $L = 0;
        my($line);
        foreach $line (@list) 
        {
            @tmp = split(',', $line, 4);
            @tmp2 = split(':', $tmp[2], 2);
            $L =  $tmp2[0];
            $T =  $tmp2[1];
        }
        if (defined($tmp[3]))
        {
            if (($tmp[3] =~ /SERIAL\.VXD/i) or ($tmp[3] =~ /WDMMDMLD\.VXD/i))
            {
#               print "Using correct WDM loader:$tmp[3]\n";
            }
            else
            {
                AddModemWarning(4116,$L);
#               print "Using incorrect WDM loader:$tmp[3]\n";
            }
        }
    }
    @list = getPathKey(",Contention");
    @list = getPathKey("Override,");
}

#-------------------------------------------------------------
# Flagg errors for invalid SVD keys
#
#-------------------------------------------------------------
sub
proc_SVDKeys
{
my($Lnum, $model) = @_;
my(@list, $line, $count, @tmp, @tmp2);
my(@IllegalKeys, $Key, $L, $T);

    @IllegalKeys = ("SVDmode,", "MediaLinkCntrl,", "MediaLinkModulation,");

    foreach $Key  (@IllegalKeys)
    {
        @list = getPathKey($Key);

        if (($count = @list) != 0)
        {
            @tmp = ();
            $L = 0;
            foreach $line (@list)
            {
                @tmp = split(',', $line, 4);
                @tmp2 = split(':', $tmp[2], 2);
                $L =  $tmp2[0];
                $T =  $tmp2[1];
            }
            AddModemWarning(4042, $L);
        }
    }
}

#-------------------------------------------------------------
# Get the Fax keys out - dont know how to process them!!
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_fax
{
my($Lnum, $model) = @_;
my($line,@list,@tmp,@tmp2,$L);

    if((defined(&FAXREG::proc_faxreg))&&($FaxCheck == 1))
    {
        FAXREG::proc_faxreg($Lnum, $model);
    }

    @list = getPathKey(",Default",1);
    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
        AddModemWarning(4053, $L);
    }

    @list = getPathKey(",FClass",1);
    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
        AddModemWarning(4053, $L);
    }

    @list = getPathKey("Fax,");
    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
        AddModemWarning(4053, $L);
    }
}

#-------------------------------------------------------------
#
# Get the Voice keys
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_voice
{
my($Lnum, $model) = @_;
my($VoiceProfile, $count,@list);

    voice_minimal_keys($Lnum, $model);
    return if (!defined($InstallProps{uc $model}->[3]));

    $VoiceProfile   = $InstallProps{uc $model}->[3];

#   caller ID
    @list = getPathKey("EnableCallerID,");
    # check for the AT-Command syntax
    CheckATCommands($model, "EnableCallerID,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4065, $Lnum, "EnableCallerID")
            if (($VoiceProfile & 0x00000080) ||
                !defined($VoiceProfile));
        $CallerIDCheck{uc $model} = 1;
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3130, $Lnum) if (($VoiceProfile & 0x00000080) == 0);  }

    @list = getPathKey(",CallerIDPrivate");
    if (($count = @list) > 0)
    {   AddModemWarning(4065, $Lnum, "CallerIDPrivate")
            if (($VoiceProfile & 0x00000080) ||
                !defined($VoiceProfile));

        my(@tmp,$field);
        @tmp = ();
        foreach $field (@list)
        {
            @tmp = split(",",$field,4);
            #if($tmp[3] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
            #{
               # get the quoted string.
            #    $tmp[3] = ChkInf::QuotedString($1);
            #}
            if($tmp[3] =~ /^\"/)
            {
        $tmp[3] =~ s/^\"//;
        $tmp[3] =~ s/\"$//;
                # get the quoted string.
                #$tmp[3] = ChkInf::QuotedString($1);
            }
            if ($tmp[3] =~ /P/)
            {
                if (defined($CallerIDCheck{uc $model}))
                 { $CallerIDCheck{uc $model} =  $CallerIDCheck{uc $model} | 0x000000002;  }
                else
                { $CallerIDCheck{uc $model} = 0x000000002; }
            }
        }
    }

    @list = getPathKey(",CallerIDOutSide");
    if (($count = @list) > 0)
    {   AddModemWarning(4065, $Lnum, "CallerIDOutSide")
            if (($VoiceProfile & 0x00000080) ||
                !defined($VoiceProfile));

        my(@tmp,$field);
        @tmp = ();
        foreach $field (@list)
        {
            @tmp = split(",",$field,4);
            #if($tmp[3] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
            #{
                # get the quoted string.
            #    $tmp[3] = ChkInf::QuotedString($1);
            #}
            if($tmp[3] =~ /^\"/)
            {
                $tmp[3] =~ s/^\"//;
                $tmp[3] =~ s/\"$//;
                # get the quoted string.
                #$tmp[3] = ChkInf::QuotedString($1);
            }
            if ($tmp[3] =~ /O/)
            {
                if (defined($CallerIDCheck{uc $model}))
                 { $CallerIDCheck{uc $model} =  $CallerIDCheck{uc $model} | 0x000000004;  }
                else
                { $CallerIDCheck{uc $model} = 0x000000004; }
            }
        }
    }

#   distinctive rings
    @list = getPathKey("EnableDistinctiveRing,");
    # check for the AT-Command syntax
    CheckATCommands($model, "EnableDistinctiveRing,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4066, $Lnum, "EnableDistinctiveRing")
            if (($VoiceProfile & 0x00001000) ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3131, $Lnum) if (($VoiceProfile & 0x00001000) == 0);  }

    #   VariableTerminator used by either callerID or distinctive ring
    @list = getPathKey(",VariableTerminator");
    if (($count = @list) == 0 and defined($VoiceProfile))
    {   AddModemError(3132, $Lnum) if (($VoiceProfile & 0x00000080) == 0 || ($VoiceProfile & 0x00001000) == 0 );
    }
    if (($count = @list) > 0)
    {
        my(@tmp,$field);
        @tmp = ();
        foreach $field (@list)
        {
            @tmp = split(",",$field,4);
            #if($tmp[3] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
            #{
                # get the quoted string.
            #   $tmp[3] = ChkInf::QuotedString($1);
            #}
            if($tmp[3] =~ /^\"/)
            {
                $tmp[3] =~ s/^\"//;
                $tmp[3] =~ s/\"$//;
                # get the quoted string.
                #$tmp[3] = ChkInf::QuotedString($1);
            }
            if ($tmp[3] =~ /<cr><lf>/)
            {
                if (defined($CallerIDCheck{uc $model}))
                 { $CallerIDCheck{uc $model} =  $CallerIDCheck{uc $model} | 0x000000008;  }
                else
                { $CallerIDCheck{uc $model} = 0x000000008; }
            }
        }
    }

# generate digits
    @list = getPathKey("GenerateDigit,");
    # check for the AT-Command syntax
    CheckATCommands($model, "GenerateDigit,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4067, $Lnum, "GenerateDigit")
            if (($VoiceProfile & 0x00020000) ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3133, $Lnum) if (($VoiceProfile & 0x00020000) == 0);  }

# handset
    @list = getPathKey("OpenHandset,");
    # check for the AT-Command syntax
    CheckATCommands($model, "OpenHandset,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4068, $Lnum, "OpenHandset")
            if (($VoiceProfile & 0x00000002) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3134, $Lnum, "OpenHandset")
                if ($VoiceProfile & 0x00000002);
    }

    @list = getPathKey("CloseHandset,");
    # check for the AT-Command syntax
    CheckATCommands($model, "CloseHandset,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4068, $Lnum, "CloseHandset")
            if (($VoiceProfile & 0x00000002) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3134, $Lnum, "CloseHandset")
                if ($VoiceProfile & 0x00000002);
    }

    @list = getPathKey("HandsetSetRecordFormat,");
    # check for the AT-Command syntax
    CheckATCommands($model, "HandsetSetRecordFormat,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4068, $Lnum, "HandsetSetRecordFormat")
            if (($VoiceProfile & 0x00000002) == 0  ||
                ($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3134, $Lnum, "HandsetSetRecordFormat")
                if (($VoiceProfile & 0x00000002) &&
                    ($VoiceProfile & 0x00000020));
    }

    @list = getPathKey("HandsetSetPlayFormat,");
    # check for the AT-Command syntax
    CheckATCommands($model, "HandsetSetPlayFormat,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4068, $Lnum, "HandsetSetPlayFormat")
            if (($VoiceProfile & 0x00000002) == 0 ||
                ($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3134, $Lnum, "HandsetSetPlayFormat")
                if (($VoiceProfile & 0x00000002) &&
                    ($VoiceProfile & 0x00000020));
    }

# SpeakerPhone
    @list = getPathKey("SpeakerPhoneEnable,");
    # check for the AT-Command syntax
    CheckATCommands($model, "SpeakerPhoneEnable,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4069, $Lnum, "SpeakerPhoneEnable")
            if (($VoiceProfile & 0x00000004) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3136, $Lnum, "SpeakerPhoneEnable")
                if ($VoiceProfile & 0x00000004);
    }

    @list = getPathKey("SpeakerPhoneDisable,");
    # check for the AT-Command syntax
    CheckATCommands($model, "SpeakerPhoneDisable,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4069, $Lnum, "SpeakerPhoneDisable")
            if (($VoiceProfile & 0x00000004) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3136, $Lnum, "SpeakerPhoneDisable")
                if ($VoiceProfile & 0x00000004);
    }

    @list = getPathKey("SpeakerPhoneSetVolumeGain,");
    # check for the AT-Command syntax
    CheckATCommands($model, "SpeakerPhoneSetVolumeGain,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4069, $Lnum, "SpeakerPhoneSetVolumeGain")
            if (($VoiceProfile & 0x00000004) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3136, $Lnum, "SpeakerPhoneSetVolumeGain")
                if ($VoiceProfile & 0x00000004);
    }

    my($speakerspec) = proc_speaker_specs();
    if ($speakerspec == 0)
    {
        AddModemError(3136, $Lnum, "SpeakerPhoneSpecs")
                if ($VoiceProfile & 0x00000004);
    }

    @list = getPathKey("SpeakerPhoneMute,");
    # check for the AT-Command syntax
    CheckATCommands($model, "SpeakerPhoneMute,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4070, $Lnum, "SpeakerPhoneMute")
            if (($VoiceProfile & 0x00000004) &&
                ($VoiceProfile & 0x00400000) ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3139, $Lnum, "SpeakerPhoneMute")
                if (($VoiceProfile & 0x00000004) &&
                    ($VoiceProfile & 0x00400000) == 0) ;
    }

    @list = getPathKey("SpeakerPhoneUnMute,");
    # check for the AT-Command syntax
    CheckATCommands($model, "SpeakerPhoneUnMute,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4070, $Lnum, "SpeakerPhoneUnMute")
            if (($VoiceProfile & 0x00000004) &&
                ($VoiceProfile & 0x00400000) ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3139, $Lnum, "SpeakerPhoneUnMute")
                if (($VoiceProfile & 0x00000004) &&
                    ($VoiceProfile & 0x00400000) == 0) ;
    }

# Line record
    @list = getPathKey("LineSetRecordFormat,");
    # check for the AT-Command syntax
    CheckATCommands($model, "LineSetRecordFormat,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4071, $Lnum, "LineSetRecordFormat")
            if (($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "LineSetRecordFormat")
                if (($VoiceProfile & 0x00000020));
    }

    @list = getPathKey("LineSetPlayFormat,");
    # check for the AT-Command syntax
    CheckATCommands($model, "LineSetPlayFormat,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4071, $Lnum, "LineSetPlayFormat")
            if (($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "LineSetPlayFormat")
                if (($VoiceProfile & 0x00000020));
    }

    @list = getPathKey("StartPlay,");
    # check for the AT-Command syntax
    CheckATCommands($model, "StartPlay,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4071, $Lnum, "StartPlay")
            if (($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "StartPlay")
                if (($VoiceProfile & 0x00000020));
    }

    @list = getPathKey("StartRecord,");
    # check for the AT-Command syntax
    CheckATCommands($model, "StartRecord,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4071, $Lnum, "StartRecord")
            if (($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "StartRecord")
                if (($VoiceProfile & 0x00000020));
    }

    @list = getPathKey("StopPlay,");
    # check for the AT-Command syntax
    CheckATCommands($model, "StopPlay,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4071, $Lnum, "StopPlay")
            if (($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "StopPlay")
                if (($VoiceProfile & 0x00000020));
    }

    @list = getPathKey("StopRecord,");
    # check for the AT-Command syntax
    CheckATCommands($model, "StopRecord,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4071, $Lnum, "StopRecord")
            if (($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "StopRecord")
                if (($VoiceProfile & 0x00000020));
    }

    @list = getPathKey(",AbortPlay");
    &proc_voice_range2("AbortPlay",@list);
    if (($count = @list) > 0)
    {   AddModemWarning(4071, $Lnum, "AbortPlay")
            if (($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "AbortPlay")
                if (($VoiceProfile & 0x00000020));
    }

    @list = getPathKey(",TerminatePlay");
    &proc_voice_range2("TerminatePlay", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4071, $Lnum, "TerminatePlay")
            if (($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "TerminatePlay")
                if (($VoiceProfile & 0x00000020));
    }

    @list = getPathKey(",TerminateRecord");
    &proc_voice_range2("TerminateRecord",@list);
    if (($count = @list) > 0)
    {   AddModemWarning(4071, $Lnum, "TerminateRecord")
            if (($VoiceProfile & 0x00000020) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "TerminateRecord")
                if (($VoiceProfile & 0x00000020));
    }

# wavedriver
    if ($VoiceProfile & 0x02000000)
    {
        @list = getPathKey("WaveDriver,BaudRate");
        if (($count = @list) > 0)
        {   AddModemWarning(4071, $Lnum, "WaveDriver,BaudRate")
                if (($VoiceProfile & 0x00000020) == 0 ||
                    !defined($VoiceProfile));
        }
        elsif (defined($VoiceProfile))
        {   AddModemError(3140, $Lnum, "WaveDriver, BaudRate")
                    if (($VoiceProfile & 0x00000020));
        }

        @list = getPathKey("WaveDriver,WaveHardwareID");
        if (($count = @list) > 0)
        {   AddModemWarning(4071, $Lnum, "WaveDriver,WaveHardwareID")
                if (($VoiceProfile & 0x00000020) == 0 ||
                    !defined($VoiceProfile));
        }
        elsif (defined($VoiceProfile))
        {   AddModemError(3140, $Lnum, "WaveDriver, WaveHardwareID")
                    if (($VoiceProfile & 0x00000020));
        }

        @list = getPathKey("WaveDriver,XformID");
        my($entry,@break,@break1,$count1);
        if (($count1 = @list) > 0)
        {
            $entry = $list[0];
            @break = split(":",$entry);
            @break1 = split(",",$break[1]);
            if (($count = @break1) > 3)
            {
                #this is a two byte entry.
                AddModemError(3153, $Lnum);
            }
        }
        if (($count = @list) > 0)
        {   AddModemWarning(4071, $Lnum, "WaveDriver,XformID")
                if (($VoiceProfile & 0x00000020) == 0 ||
                    !defined($VoiceProfile));
        }
        elsif (defined($VoiceProfile))
        {   AddModemError(3140, $Lnum, "WaveDriver, XformID")
                    if (($VoiceProfile & 0x00000020));
        }

        @list = getPathKey("WaveDriver,XformModule");
        if (($count = @list) > 0)
        {   AddModemWarning(4071, $Lnum, "WaveDriver,XformModule")
                if (($VoiceProfile & 0x00000020) == 0 ||
                    !defined($VoiceProfile));
        }
        elsif (defined($VoiceProfile))
        {   AddModemError(3140, $Lnum, "WaveDriver, XformModule")
                    if (($VoiceProfile & 0x00000020));
        }
# baudrate
        @list = getPathKey(",VoiceBaudRate");
        if (($count = @list) == 0 && defined($VoiceProfile))
        {   AddModemError(3142, $Lnum)
                    if (($VoiceProfile & 0x00080000));
        }
    }

# voice commands
    @list = getPathKey("VoiceAnswer,");
    # check for the AT-Command syntax
    CheckATCommands($model, "VoiceAnswer,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4072, $Lnum, "VoiceAnswer")
            if (($VoiceProfile & 0x00000001) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "VoiceAnswer")
                if (($VoiceProfile & 0x00000001));
    }

    @list = getPathKey("VoiceToDataAnswer,");
    # check for the AT-Command syntax
    CheckATCommands($model, "VoiceToDataAnswer,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4072, $Lnum, "VoiceToDataAnswer")
            if (($VoiceProfile & 0x00000001) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "VoiceToDataAnswer")
                if (($VoiceProfile & 0x00000001));
    }

    @list = getPathKey("VoiceDialNumberSetup,");
    # check for the AT-Command syntax
    CheckATCommands($model, "VoiceDialNumberSetup,", @list);
    if (($count = @list) > 0)
    {   AddModemWarning(4072, $Lnum, "VoiceDialNumberSetup")
            if (($VoiceProfile & 0x00000001) == 0 ||
                !defined($VoiceProfile));
    }
    elsif (defined($VoiceProfile))
    {   AddModemError(3140, $Lnum, "VoiceDialNumberSetup")
                if (($VoiceProfile & 0x00000001));
    }

# others ignore
    @list = getPathKey(",ForwardDelay");
    @list = getPathKey(",HandsetCloseDelay");
    @list = getPathKey("Voice");
    @list = getPathKey(",Voice");

}

#-------------------------------------------------------------
#
# Check if Enumerator and VoiceProfile present
#   for voice modem ("Voice" in the friendly name ?)
#
#-------------------------------------------------------------
sub
voice_minimal_keys
{
my($Lnum, $model) = @_;
my(@list, $line, $count, $L, $T, @tmp, @tmp2, $IsValid, @KeyValues);
my($EnumPresent);

    $EnumPresent = 1;

    $IsValid = 1;

    @list = getPathKey(",Enumerator");
    $EnumPresent = 0 if (($count = @list) == 0);

    if ($EnumPresent !=0)
    {
        @tmp = ();
        foreach $line (@list)
        {
            @tmp = split(',', $line, 4);
            @tmp2 = split(':', $tmp[2], 2);
            $L =  $tmp2[0];
            $T =  $tmp2[1];
        }

        @KeyValues = split(',', $tmp[3]);
        if (defined($KeyValues[0]))
        {
            if ($KeyValues[0] !~ /^serwave.vxd$/i)
                 { AddModemWarning(4107, $L);}
        }

    }

    if (proc_voice_profile($model) == 0)
    {   $IsValid = 0;   }

    if ($IsValid == 1)
    {
        if ($EnumPresent == 0) {
            AddModemError(3206, $Lnum);
        }
        $VoiceInstallList{uc $model} = 1;   #placeholder
    }
}


#-------------------------------------------------------------
#
# Check for the VoiceProfile settings for Rockwell chips
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_voice_profile
{
my($model) = @_;
my(@bytes, $L, $T, @list, $line, $count, @tmp, @tmp2, $VoiceProfile);

    #   instalation properties: VoiceProfile
    undef($InstallProps{uc $model}->[3]);

    @list = getPathKey(",VoiceProfile", 1);
    return 0 if (($count = @list) == 0);

    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
        $T =  $tmp2[1];
        return 1 if ($T !~ /^1$/);
    }

    @bytes = split(',', $tmp[3]);
    if (($count = @bytes) != 4)
    {
#       inst_err("VoiceProfile should have one DWORD.", $L);
        AddModemError(3099, $L);
        return 1;
    }

    my(@dwords) = ();
    @dwords = get_dwords(4,$L, @bytes);

    if (defined($dwords[0]))
    {
        $VoiceProfile   = hex($dwords[0]);

        AddModemWarning(4073, $L)
            if (($VoiceProfile & 0x02000000) == 0);

        AddModemWarning(4074, $L)
            if (($VoiceProfile & 0x00000001) == 0);

        if ( !($VoiceProfile & 0x00080000) ||       # SetBaudBeforeWave
             !($VoiceProfile & 0x00000200)          # ForceBlindDial
            )
        {   AddModemWarning(4041, $L);  }
        if ($VoiceProfile & 0x00000100)         # Modem speaker volume can be changed with multimedia mixer.
        {   AddModemWarning(4052, $L);  }

        $InstallProps{uc $model}->[3] = $VoiceProfile;
    }

    return 1;
}

sub
proc_speaker_specs
{
my(@bytes,@list,$L, $T, $line, $count, @tmp, @tmp2);

    @list = getPathKey(",SpeakerPhoneSpecs", 1);
    $count = @list;
    if ($count == 0)
        { return 0 ; }

    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
        $T =  $tmp2[1];
        return 1 if ($T !~ /^1$/);
    }

    @bytes = split(',', $tmp[3]);
    if (($count = @bytes) != 16)
    {
#       inst_err("SpeakerPhoneSpecs should have 16 bytes.", $L);
        AddModemError(3137, $L);
    }

    return 1;
}



#-------------------------------------------------------------
#
# Get the Voice special keys AbortPlay, TerminatePlay, TerminateRecord
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_voice_range2
{
my($command,@list) = @_;
my($count,$this_val,@tmp,@tmp2,$line,$L);

    $this_val = &join_strval(@list);
    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
    }

    AddModemWarning(4040, $L, $command) if ($this_val =~ /AT<CR>$/i);
}

#-------------------------------------------------------------
#
# Get the VoiceView keys
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------

sub
proc_vv
{
my($Lnum, $model) = @_;
my(@list,$count);

    @list = getPathKey("MonitorVoiceViewOn,");
    # check for the AT-Command syntax
    CheckATCommands($model, "MonitorVoiceViewOn,", @list);

    @list = getPathKey("MonitorVoiceViewOff,");
    # check for the AT-Command syntax
    CheckATCommands($model, "MonitorVoiceViewOff,", @list);

    @list = getPathKey("MonitorVoice");
    @list = getPathKey(",RunOnce");
    @list = getPathKey(",VoiceView");
    if (($count = @list) > 0)
    {
#       add_ref_wrn("$model: VoiceView may not be supported in future.",
#       $Lnum);
        AddModemWarning(4015, $Lnum, $model);
        return;
    }
}

#-------------------------------------------------------------
#
# Get the Responses keys out - do minimal processing.
#
# Globals changed:
#
# Usage:
#
#-------------------------------------------------------------
sub
proc_resp
{
my($Lnum, $model, @CurrentResponses) = @_;
my(@Properties, $MaxDTE, $MaxDCE, $MaxSpeed);
my($RefResponse, @ResponseBinary, $ResponseString, $ResponseLine, $IsNumericResponse);
my($DCESpeedErrors,$DTESpeedErrors);
my(%MinimalNumericResponse, $NumericValue, $NumericRespErrors, %MinimalVerboseResponse);
my($CountMinimalNumeric, $CountMinimalVerbose);
my($CountNumericResponse, $CountVerboseResponse, @InitSettings);
my(%SectionPatterns, $ResponsePattern, %PatternSpeeds);
my(%GlobalResponseStrings, $StrippedResponses);
my(%HashDCESpeeds, %HashDTESpeeds);
my(@ExceptStrippedResponses);
my($count,$Message,$VerboseRespErrors);

    %MinimalNumericResponse = ( 0 => 0, 2 => 0, 3 => 0, 4 => 0,
                                6 => 0, 7 => 0, 8 => 0);
    %MinimalVerboseResponse = ( "OK" => "OK", "CONNECT" => "CONNECT", "RING" => "RING",
                                "NOCARRIER" => "NO CARRIER", "ERROR" => "ERROR",
                                "NODIALTONE" => "NO DIALTONE", "BUSY" => "BUSY",
                                "NOANSWER" => "NO ANSWER");
    @ExceptStrippedResponses    = (
                            '^(?i)NAME *= *$',      #required for CID
                            '^(?i)TIME *= *$',      #required for CID
                            '^(?i)CALR *= *$',
                            '^(?i)NMBR *= *$',      #required for CID
                            '^(?i)MESG *= *$',      #required for CID
                            '^(?i)DATE *= *$',      #required for CID
                            '^(?i)VCON *$',
                            '^(?i)RING *1$',
                            '^(?i)RING *2$',
                            '^(?i)RING *3$',
                            '^(?i)RING *A$',
                            '^(?i)RING *B$',
                            '^(?i)RING *C$',
                            '^(?i)<cr>$',
                            '^(?i)<lf>$',
                            '^(?i)<cr><lf>$',
                            );

    $CountMinimalNumeric = keys %MinimalNumericResponse;
    $CountMinimalVerbose = keys %MinimalVerboseResponse;

    %GlobalResponseStrings = ();

    if (($count = @CurrentResponses) < 1)
    {
#       add_ref_err("$model does not have any Responses.", $Lnum);
        AddModemError(3060, $Lnum, $model);
    }

    if (defined(@{${$InstallProps{uc $model}}[0]}))
    {   @Properties = @{${$InstallProps{uc $model}}[0]}; }
    else
    {
        return;
    }
    if (defined(@{${$InstallProps{uc $model}}[2]}))
    {   @InitSettings = @{${$InstallProps{uc $model}}[2]};}
    else
    {
        return;
    }
    $MaxDTE = $Properties[6];
    $MaxDCE = $Properties[7];
    $MaxSpeed = ($MaxDCE < $MaxDTE) ? $MaxDTE : $MaxDCE;
    $DCESpeedErrors = 0;
    $DTESpeedErrors = 0;
    %HashDCESpeeds = ();
    %HashDTESpeeds = ();
    $CountNumericResponse = 0;
    $CountVerboseResponse = 0;
    $StrippedResponses = 0;

    foreach $RefResponse (@CurrentResponses)
    {
        my($SearchString, $DuplicateLine, $DupRefStructResp);

        #$ResponseString = ChkInf::QuotedString($$RefResponse[0]);
        $ResponseString = $$RefResponse[0];
        @ResponseBinary = @{$$RefResponse[2]};
        $ResponseLine   = $$RefResponse[1];
        $IsNumericResponse = ${$$RefResponse[3]}[0];
        $NumericValue = ${$$RefResponse[3]}[1];
        $ResponsePattern = ${$$RefResponse[4]}[0];

        $SearchString = $ResponseString;
        $SearchString =~ s/\s$//;
        if(defined($GlobalResponseStrings{uc $SearchString}))
        {
            ($DuplicateLine, $DupRefStructResp) =
                @{$GlobalResponseStrings{uc $SearchString}};
            if ($DupRefStructResp->[0] == $ResponseBinary[0] and
                $DupRefStructResp->[1] == $ResponseBinary[1] and
                $DupRefStructResp->[2] == $ResponseBinary[2] and
                $DupRefStructResp->[3] == $ResponseBinary[3])
            {
                if ($bAllWarnings == 1)
                {   AddModemWarning(4064, $ResponseLine, $SearchString, $DuplicateLine);}
                else
                { $bDupWarnings = 1;}
            }
        }
        $GlobalResponseStrings{uc $SearchString} =
                        [$ResponseLine, [@ResponseBinary]];

                # assure minimal numeric responses
        if ($IsNumericResponse != 0)
        {
            $CountNumericResponse++;
            delete($MinimalNumericResponse{$NumericValue});
        }
        else
        {

            $CountVerboseResponse++;
            $SearchString = $ResponseString;

            if (($ResponseBinary[0] == 0x96)||($ResponseBinary[0] == 0x97)||
                ($ResponseBinary[0] == 0x95)||($ResponseBinary[0] == 0x94)||
                ($ResponseBinary[0] == 0x93))
            {
                my($checkbit,$shiftamount);
                $shiftamount = ($ResponseBinary[0] - 0x93) + 4;
                $checkbit = 1;
                $checkbit = $checkbit << $shiftamount;

                if (defined($CallerIDCheck{uc $model}))
                { $CallerIDCheck{uc $model} =  $CallerIDCheck{uc $model} | $checkbit; }
            }


            # check if stripped responses are used (only for verbose)
            if ($StrippedResponses == 0)
            {
                my ($Exception, $Excepted);

                $Excepted = 0;
                foreach $Exception (@ExceptStrippedResponses)
                {
                    if ($SearchString =~ /$Exception/ )
                    {
                        $Excepted = 1;
                        last;
                    }
                }

                if ($Excepted == 0 and
                    $SearchString !~ /^<cr><lf>/i and
                    $SearchString !~ /<cr><lf>$/i)
                {
#                    print "$SearchString causing error\n";
                    $StrippedResponses = 1;
                }
            }

            $SearchString =~ s/<cr>//gi;
            $SearchString =~ s/<lf>//gi;
            $SearchString =~ s/\s+//gi;

            delete($MinimalVerboseResponse{uc $SearchString});
        }

                # check speeds against properties
        if ($ResponseBinary[2] > $MaxSpeed)     # first dword could be either DTE or DCE
        {
            # log error if in advisory mode
            # add_ref_wrnX(4020, $ResponseLine, "Model $model", $MaxSpeed);
            $DCESpeedErrors++;
            $HashDCESpeeds{$ResponseLine} = 1;
        }
        if ($ResponseBinary[3] > $MaxDTE)
        {
            # log error if in advisory mode
            # add_ref_wrnX(4051, $ResponseLine, "Model $model", $MaxDTE);
            $DTESpeedErrors++;
            $HashDTESpeeds{$ResponseLine} = 1;
        }
    }

    # if ($AdvisoryMode == 0)
    # log only one error for speed if NOT in advisory mode
    {
        my($RefSpeedError);

        if ($DCESpeedErrors > 0)
        {
            if (!defined($ErrSpeedReport{"DCE".$MaxSpeed}))
            {
                $ErrSpeedReport{"DCE".$MaxSpeed} = [\%HashDCESpeeds, 1, $MaxSpeed, $Lnum];
            }
            else
            {
                my($k);
                $RefSpeedError = $ErrSpeedReport{"DCE".$MaxSpeed};
                foreach $k (keys %HashDCESpeeds)
                {
                    ${$$RefSpeedError[0]}{$k} = 1;
                }
                $$RefSpeedError[1]++;
            }
        }

        if ($DTESpeedErrors > 0)
        {
            if (!defined($ErrSpeedReport{"DTE".$MaxDTE}))
            {
                $ErrSpeedReport{"DTE".$MaxDTE} = [\%HashDTESpeeds, 1, $MaxDTE, $Lnum];
            }
            else
            {
                $RefSpeedError = $ErrSpeedReport{"DTE".$MaxDTE};
                my($k);
                foreach $k (keys %HashDTESpeeds)
                {
                    ${$$RefSpeedError[0]}{$k} = 1;
                }
                $$RefSpeedError[1]++;
            }
        }
    }

        # check for minimal numeric responses
    $NumericRespErrors = (keys %MinimalNumericResponse);
    $Message = join(', ', sort keys %MinimalNumericResponse);
    AddModemError(3123, $Lnum, $NumericRespErrors, $Message)
            if ($NumericRespErrors > 0);

        # check for minimal verbose responses
    $VerboseRespErrors = (keys %MinimalVerboseResponse);
    $Message = join(', ', sort values %MinimalVerboseResponse);
    AddModemError(3127, $Lnum, $VerboseRespErrors, $Message)
            if ($VerboseRespErrors > 0);

        # check if responses (numeric/verbose) agree with Init V0/V1
    if (defined($InitSettings[0]))
    {
        if ($InitSettings[0] == 1)  # verbose response
        {
            AddModemError(3124, $Lnum) if ($CountVerboseResponse <= $CountMinimalVerbose);
            AddModemWarning(4047, $Lnum) if ($CountVerboseResponse < $CountNumericResponse);
        }
        else
        {
            AddModemError(3125, $Lnum) if ($CountNumericResponse <= $CountMinimalNumeric);
            AddModemWarning(4048, $Lnum) if ($CountVerboseResponse > $CountNumericResponse);
        }
    }

        # padded responses prefered
    if ($StrippedResponses == 1)
    {
        AddModemError(3104, $Lnum);
    }

        # because responses preferred, <cr>, <lf> must not appear
    if (defined($GlobalResponseStrings{"<CR>"}))
    {
        AddModemWarning(4050, $GlobalResponseStrings{"<CR>"}->[0]);
    }
    elsif ($StrippedResponses == 1)
    {
        AddModemError(3129, $Lnum, "<cr>");
    }

    if (defined($GlobalResponseStrings{"<LF>"}))
    {
        AddModemWarning(4050, $GlobalResponseStrings{"<LF>"}->[0]);
    }
    elsif ($StrippedResponses == 1)
    {
        AddModemError(3129, $Lnum, "<lf>");
    }
}


#-------------------------------------------------------------
#
# This sub returns the list of HKRs that match the given Path
# and removes the entries found from the list of current HKR's
#
#
# Globals changed:
#   Inst_HKR in CheckInstSections
#
# Usage:
#   getPathKey(Path, MatchWholeWord)
#
#-------------------------------------------------------------
sub
getPathKey
{
my($match, $bWholeWord) = @_;
my($bKeyNum, $key, @SortedKeys, %list, @tmp,@ret);

    if (!defined($bWholeWord))
    {   $bWholeWord = 0; }

    %list = ();
    $match = uc $match;

    $bKeyNum = 1;
    foreach $key (keys %Inst_HKR)
    {
        if ( ($bWholeWord) ? ($key =~ /^$match$/) : ($key =~ /^$match/) )
        {
            @tmp = split(',', $key, 2);

            #if at least one command key is alpha, use string compare
            if ( $bKeyNum && ($tmp[1] !~ /^[0-9]*$/) )
            {   $bKeyNum = 0; }

            $list{$key} = $Inst_HKR{$key};

            delete $Inst_HKR{$key};
        }

    }
    @ret = ();
    if ($bKeyNum)
    {   @SortedKeys = sort StrIntCompare (keys %list); }
    else
    {   @SortedKeys = sort(keys %list); }

    foreach $key (@SortedKeys)
    {
        push(@ret, join(",", $key, $list{$key}));
    }

    @ret;
}

#-------------------------------------------------------------
#
# Procedure used to compare the keys in getPathKey
#
#-------------------------------------------------------------
sub
StrIntCompare
{
my(@AParts, @BParts, $StrPart);

  @AParts = split(",",$a);
  @BParts = split(",",$b);

  $StrPart = uc($AParts[0]) cmp uc($BParts[0]); #compare first parts as strings
  if ($StrPart == 0)
     { $AParts[1] <=> $BParts[1]; } #compare second parts as numbers
  else
     {$StrPart;}
}

#-------------------------------------------------------------
#
# This sub returns the last line of the list of HKRs
#
#-------------------------------------------------------------
sub
getKeyLastLine
{
    my (@HKRLines) = @_;
    my ($line, $L, @tmp);

    foreach $line (@HKRLines)
    {
        @tmp = split(',', $line, 4);
        @tmp = split(':', $tmp[2], 2);
        $L =  $tmp[0];
    }

    $L;
}
#-------------------------------------------------------------
#
# This sub retrives the <Value> as a string from the list and
# joins if multiple lines are passed. This can be used for,
# say, multilevel Inits where you pass the list of Inits (as
# recieved from a call to getPathKey("Init,") and this sub will
# return a single variable with the join of all the Inits.
# It also checks for empty strings and the %s in the string.
#
# Globals changed:
#
# Usage:
#   join_strval(list)
#
#-------------------------------------------------------------
sub
join_strval
{
my(@list) = @_;
my($count,$L,$line, $this_val, @tmp, @tmp2, $T, $cnt, $cnt2);

    $this_val ="";
    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
        $T =  $tmp2[1];
        return if ($T !~ /^0$/);    # dont know what to do with binaries!!!!
        if($tmp[3] !~ /^\"/)
        {
#           inst_err("Expect &ltValue&gt to be a quoted string.", $L);
            AddModemError(3081, $L);
            return;
        } else {

            $tmp[3] =~ s/^\"//;
            $tmp[3] =~ s/\"$//;
            #$tmp[3] = ChkInf::QuotedString($1);
            if($tmp[3] =~ /^$/)
            {
#               inst_err("&ltValue&gt cant be empty string.", $L) unless
#               ($tmp[1] =~ /DIALSUFFIX/);
                AddModemError(3082, $L) unless ($tmp[1] =~ /DIALSUFFIX/);
                return;
            }

            # check for %s.
            $_ = $tmp[3];
            $cnt = tr/%/%/;
            $cnt2 = s/%%/%%/g;

            if (($cnt % 2) || ($cnt != 2*$cnt2))
            {
#               inst_err("&ltValue&gt should have doubled %s.", $L);
#               AddModemError(3083, $L);
            }

            $tmp[3] =~ s/[\s\t]//g; # get rid of all white spaces for efficient comparison later!
        }

        $this_val = join("", $this_val, $tmp[3]);
    }
    $this_val = uc $this_val;

    $this_val;
}


#-------------------------------------------------------------
#   Checks if the command lines are in sequence. The input lines
#   are in the form <command>, <sequence_number>, <lineNo>:[0/1] ...
#
# Usage:
#       CheckSeqCommand(Inst_Section, array_command_lines)
#
#-------------------------------------------------------------

sub
CheckSeqCommand
{
my($InstSect, $command, @CommandLines) = @_;
my(@ArgLine,$size,@ErrReturn, $ErrCount, @aux, $NoExpect, $CommandIndex, $first, $last);

    $size = @CommandLines;
    $CommandIndex = 0;
    $NoExpect = 1;

    @ArgLine = split(",", $command);
    $command = $ArgLine[0];

    while ($CommandIndex < $size)
    {   @ArgLine = split(",", $CommandLines[$CommandIndex]);
        @ErrReturn = ();
        while ($ArgLine[1] > $NoExpect )
        {   push(@ErrReturn, $NoExpect);
            $NoExpect++;
        }
        if (($ErrCount = @ErrReturn) > 0)
        {   @aux = split(":", $ArgLine[2]);
            $first = $ErrReturn[0];
            $last = $ErrReturn[$ErrCount-1];
            if ($ErrCount == 1)
               {
#              add_ref_err("$InstSect: Command $command, level #$first not
#              found.", $aux[0]);
               AddModemError(3065, $aux[0], $InstSect, $command, $first);
               }
            else
               {
#              add_ref_err("$InstSect: Command $command, levels #$first-#$last
#              not found.", $aux[0]);
               AddModemError(3066, $aux[0], $InstSect, $command, $first, $last);
                }
        }
        $NoExpect++;
        $CommandIndex++;
    }
}


#-------------------------------------------------------------
#   Checks if AT commands end with <cr>
#
# Globals changed:
#   %ATSyntaxErr
#
# Usage:
#       CheckATString(HKR_lines)
#
#-------------------------------------------------------------
sub
CheckATString
{
my(@HKRLines)=@_;
my($command, @HKRArg, @aux, $lineNo, $CmdString, $NoErrs, $i);

    foreach $command (@HKRLines)
    {   @HKRArg = split(",", $command);
        @aux = split(":", $HKRArg[2]);
        $lineNo = $aux[0];

        # skip if line already processed
        next
            if ( defined($ATSyntaxErr{$lineNo}) );

        $NoErrs = 0;
        #if($HKRArg[3] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
        $CmdString = "";
        if($HKRArg[3] =~ /^\"/)
        {
        
            $CmdString = $HKRArg[3];
            for ($i = 4; $i < scalar(@HKRArg) ; $i++) {
                $CmdString = $CmdString.",";
                $CmdString = $CmdString.$HKRArg[$i];
            }
            $CmdString =~ s/^\"//;
            $CmdString =~ s/\"$//;

    
            $CmdString = uc $CmdString;

            if ($CmdString =~ /^[\s\t]+/)
            {
#               add_ref_err("AT Command should not start with spaces",
#               $lineNo);
                AddModemError(3067, $lineNo);
            }

            $CmdString =~ s/^[\s\t]*//;
            $CmdString =~ s/[\s\t]*$//;

            goto LineSyntax
                if ( ($CmdString =~ /^NONE$/) || ($CmdString =~ /^NORESPONSE$/) );

            if ( ($CmdString =~ /NONE/))
            {   if ($CmdString =~ /^NONE<CR>$/)
                {
#               add_ref_err("AT-Command \"None\" should not end with
#               \"&ltCR&gt\".", $lineNo);
                AddModemError(3068, $lineNo);
                }
                else
                {
#                   add_ref_err("AT-Command \"None\" suspicious.", $lineNo);
                    AddModemError(3069, $lineNo);
                }
                $NoErrs++;
                goto LineSyntax;
            }

            if ( ($CmdString =~ /NORESPONSE/))
            {   if ($CmdString =~ /^NORESPONSE<CR>$/)
                {
#                   add_ref_err("AT-Command \"NoResponse\" should not end with
#                   \"&ltCR&gt\".", $lineNo);
                    AddModemError(3070, $lineNo);
                }
                else
                {
#                   add_ref_err("AT-Command \"NoResponse\" suspicious.",
#                   $lineNo);
                    AddModemError(3071, $lineNo);
                }
                $NoErrs++;
                goto LineSyntax;
            }

            if ($CmdString !~ /^(AT|<H10>|<DLE>)/)
            {
#               add_ref_err("AT-Command string should start with \"AT\".",
#               $lineNo);
                AddModemError(3072, $lineNo);
                $NoErrs++;
            }
            if ($CmdString !~ /<CR>$/ && $CmdString =~ /^AT/)
            {
#               add_ref_err("AT-Command string should end with \"&ltCR&gt\".",
#               $lineNo);
                AddModemError(3073, $lineNo);
                $NoErrs++;
            }
        }
        LineSyntax: {$ATSyntaxErr{$lineNo} = $NoErrs;}
    }
}

#-------------------------------------------------------------
#
# This sub returns the values for each of the settings keys
# passed in SetList. It also flags error if any of the keys
# is not defined.
#
# Globals changed:
#
# Usage:
#   get_settings(SetList)
#
#-------------------------------------------------------------
sub
get_settings
{
my(@SetList) = @_;
my(@S);
my($set,$this_val,$count,@list);
my($model, $Lnum);

    $Lnum = shift(@SetList);
    $model = shift(@SetList);


    @S = ();

    foreach $set (@SetList)
    {
        @list = getPathKey("Settings,$set", 1);


        if (($count = @list) > 0)
        {
            $this_val = &join_strval(@list);
            if (defined($this_val))
                { $this_val =~ s/0//; } #0s are insignificant here!!
            @S = (@S, $this_val);
        }
        else
        {
#           add_ref_err("$model\'s properties expect a key \"Settings, $set\"
#           which is not defined.", $Lnum);
            AddModemError(3064, $Lnum, $model, $set);
        }
    }
    @S; # return value.
}
#-------------------------------------------------------------
#
# This sub validates the CallSetupFailTimer related keys.
#
# Globals changed:
#
# Usage:
#   proc_callsetup(hex_val)
#
#-------------------------------------------------------------
sub
proc_callsetup
{
my($val, $Lnum, $PropLine, $model) = @_;

    $val = hex($val);
    if ($val == 0)
    {
        return; # CallSetupFailTimer not supported.
    }

    AddModemWarning(4044, $PropLine) if ($val != 255);

    get_settings($Lnum, $model, "CallSetupFailTimer");
}

#-------------------------------------------------------------
#
# This sub validates the InactivityTimeout related keys.
#
# Globals changed:
#
# Usage:
#   proc_inact(hex_val)
#
#-------------------------------------------------------------
sub
proc_inact
{
my($val, $Lnum, $PropLine, $model) = @_;
my(@list,$count,$line,@bytes,@tmp,@tmp2,$L,$T);

    $val = hex($val);
    if ($val == 0)
    {
        return; # InactivityTimeout not supported.
    }

    AddModemWarning(4043, $PropLine) if ($val != 255 && $val != 90);

    get_settings($Lnum, $model,"InactivityTimeout");

    @list = getPathKey(",InactivityScale",1);
    if (($count = @list) < 1)
    {
#       add_ref_err("$model does not have any InactivityScale.", $Lnum);
        AddModemError(3063, $Lnum, $model);
        return;
    }
    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
        $T =  $tmp2[1];
        return if ($T !~ /^1$/);    # dont know what to do with binaries!!!!
    }

        #take the last one
    @bytes = split(',', $tmp[3]);
    if (($count = @bytes) != 4)
    {
#       inst_err("InactivityScale should have 1 DWORD.", $L);
        AddModemError(3079, $L);
        return;
    }
    get_dwords(4,$L, @bytes);
}
#-------------------------------------------------------------
#
# This sub checks for duplicates within the list passed.
#
# Globals changed:
#
# Usage:
#   check_dup(List)
#
#-------------------------------------------------------------
sub
check_dup
{
my(@list) = @_;
my($count,$i,$j);


    $count = @list;
    return(0) if ($count<2);
    $i=0;
    while ($i < $count - 2)
    {
        $j = $i+1;
        while ($j < $count - 1)
        {
            if ((defined($list[$i])) && (defined($list[$j])))
            {
                return(1) if ($list[$i] eq $list[$j]);
            }
            $j++;
        }
        $i++;

    }
    return(0);
}


#-------------------------------------------------------------
#
# This sub validates the Speaker Volume related keys.
#
# Globals changed:
#
# Usage:
#   proc_spvol(hex_val)
#
#-------------------------------------------------------------
sub
proc_spvol
{
my($val, $Lnum, $model) = @_;
my(@SetList,$count,@S);

    $val = hex($val);

    @SetList=();    #List of expected settings
    @SetList = (@SetList, "SpeakerVolume_Low") if ($val & 0x00000001);
    @SetList = (@SetList, "SpeakerVolume_Med") if ($val & 0x00000002);
    @SetList = (@SetList, "SpeakerVolume_High") if ($val & 0x00000004);


    if (($count = @SetList) > 0)
    {
        @S = get_settings($Lnum, $model,@SetList);

        #check if any two are same.
#       inst_wrn("$model: Same &ltValue&gt for at least two of
#       SpeakerVolume_XXX Settings.", $L)
        AddModemWarning(4025, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
    }

}

#-------------------------------------------------------------
#
# This sub validates the speaker modes related keys.
#
# Globals changed:
#
# Usage:
#   proc_spmode(hex_val)
#
#-------------------------------------------------------------
sub
proc_spmode
{
my($val, $Lnum, $model) = @_;
my(@SetList,$count,@S);

    $val = hex($val);
    @SetList=();    #List of expected settings
    @SetList = (@SetList, "SpeakerMode_Off") if ($val & 0x00000001);
    @SetList = (@SetList, "SpeakerMode_Dial") if ($val & 0x00000002);
    @SetList = (@SetList, "SpeakerMode_On") if ($val & 0x00000004);
    @SetList = (@SetList, "SpeakerMode_Setup") if ($val & 0x00000008);

    if (($count = @SetList) > 0)
    {
    @S = get_settings($Lnum, $model,@SetList);

    #check if any two are same.
#   inst_wrn("$model: Same &ltValue&gt for at least two of SpeakerMode_XXX
#   Settings.", $L)
    AddModemWarning(4026, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
    }
}

#-------------------------------------------------------------
#
# This sub validates the modem options related keys.
#
# Globals changed:
#
# Usage:
#   proc_mdop(hex_val)
#
#-------------------------------------------------------------
sub
proc_mdop
{
my($val, $Lnum, $model) = @_;
my(@SetList,$count,@S);

    $val = hex($val);

    # compression settings.
    @SetList=();
    @SetList = ("Compression_On", "Compression_Off") if ($val & 0x00000001);
    if (($count = @SetList) > 0)
    {
        @S = get_settings($Lnum, $model,@SetList);
#       inst_wrn("$model: Same &ltValue&gt for Compression_On/Off Settings.",
#       $L)
        AddModemWarning(4027, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
    }

    # err ctrl settings.
    @SetList=();
    @SetList = ("ErrorControl_On", "ErrorControl_Off")  if ($val & 0x00000002);
    @SetList = ("ErrorControl_Off", "ErrorControl_Forced")  if ($val & 0x00000004);
    @SetList = ("ErrorControl_On", "ErrorControl_Off", "ErrorControl_Forced") if (($val & 0x00000002)&&($val & 0x00000004));
    if (($count = @SetList) > 0)
    {
        @S = get_settings($Lnum, $model,@SetList);
#       inst_wrn("$model: Same &ltValue&gt for ErrorControl_On/Off/Forced
#       Settings.", $L)
        AddModemWarning(4028, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
    }

    # ****** Forced, Cell
    @SetList=();
    @SetList = ("ErrorControl_Cellular", "ErrorControl_Cellular_Forced")  if ($val & 0x00000008);
    if (($count = @SetList) > 0)
    {
        @S = get_settings($Lnum, $model,@SetList);
#       inst_wrn("$model: Same &ltValue&gt for
#       ErrorControl_Cellular/Cellular_Forced Settings.", $L)
        AddModemWarning(4030, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
    }

    # flow ctrl settings.
    @SetList=();
    @SetList = ("FlowControl_Off", "FlowControl_Hard") if ($val & 0x00000010);
    @SetList = ("FlowControl_Off", "FlowControl_Soft") if ($val & 0x00000020);
    @SetList = ("FlowControl_Off", "FlowControl_Soft", "FlowControl_Hard") if (($val & 0x00000010)&&($val & 0x00000020));
    if (($count = @SetList) > 0)
    {
        @S = get_settings($Lnum, $model,@SetList);
#       inst_wrn("$model: Same &ltValue&gt for at least two of FlowControl_XXX
#       Settings.", $L)
        AddModemWarning(4031, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
    }

    @SetList=();
    @SetList = ("Modulation_CCITT", "Modulation_Bell") if ($val & 0x00000040);
    @SetList = ("Modulation_CCITT_V23") if ($val & 0x00000400);
    @SetList = ("Modulation_CCITT", "Modulation_Bell", "Modulation_CCITT_V23") if (($val & 0x00000040)&&($val & 0x00000400));
    if (($count = @SetList) > 0)
    {
        @S = get_settings($Lnum, $model,@SetList);
#       inst_wrn("$model: Same &ltValue&gt for Modulation_XXX Settings.", $L)
        AddModemWarning(4032, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
#       inst_wrn("$model: Possible inversion of &ltValue&gt between Settings
#       Modulation_CCITT/Bell.", $L)
        if (defined($S[0]))
        {
            AddModemWarning(4033, $Lnum, $model)
                if((($count = @S) >1) && (($S[0] eq 'B1') && ($S[1] eq 'B0')));
        }
    }

    @SetList=();
    @SetList = ("SpeedNegotiation_On", "SpeedNegotiation_Off") if ($val & 0x00000080);
    if (($count = @SetList) > 0)
    {
        @S = get_settings($Lnum, $model,@SetList);
#       inst_wrn("$model: Same &ltValue&gt for SpeedNegotiation_On/Off
#       Settings.", $L)
        AddModemWarning(4034, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
#       inst_wrn("$model: Possible inversion of &ltValue&gt between Settings
#       SpeedNegotiation_On/Off.", $L)
        if (defined($S[0]))
        {
            AddModemWarning(4035, $Lnum, $model)
                if((($count = @S) >1) && (($S[0] eq 'N0') && ($S[1] eq 'N1')));
        }
    }

    @SetList=();
    @SetList = ("Pulse", "Tone") if ($val & 0x00000100);
    if (($count = @SetList) > 0)
    {
        @S = get_settings($Lnum, $model,@SetList);
#       inst_wrn("$model: Same &ltValue&gt for Tone/Pulse Settings.", $L)
        AddModemWarning(4036, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
#       inst_wrn("$model: Possible inversion of &ltValue&gt between Settings
#       Tone/Pulse.", $L)
        if (defined($S[0]))
        {
            AddModemWarning(4037, $Lnum, $model)
                if((($count = @S) >1) && (($S[0] eq 'T') && ($S[1] eq 'P')));
        }
    }

    @SetList=();
    @SetList = ("Blind_On", "Blind_Off") if ($val & 0x00000200);
    if (($count = @SetList) > 0)
    {
        @S = get_settings($Lnum, $model,@SetList);
#       inst_wrn("$model: Same &ltValue&gt for Blind_On/Off Settings.", $L)
        AddModemWarning(4038, $Lnum, $model)
                if((($count = @S) >1) && &check_dup(@S));
#       inst_wrn("$model: Possible inversion of &ltValue&gt between Settings
#       Blind_On/Off.", $L)
        if (defined($S[0]))
        {
            AddModemWarning(4039, $Lnum, $model)
                if((($count = @S) >1) && (($S[0] eq 'X4') && ($S[1] eq 'X3')));
        }
    }
}
#-------------------------------------------------------------
#
# This sub looks at the modem's DeviceType structure    '
#
#-------------------------------------------------------------
sub
proc_DeviceType
{
my($Lnum, $model) = @_;
my(@tmp, @tmp2, @list);
my($DeviceType, $RefAddregList);
my($count,$line,$L,$T,@bytes);

    $DeviceType = 0;
    @list = getPathKey(",DeviceType");
    if (($count = @list) > 1)
    {
        AddModemError(3094, $Lnum, $model);
    }
    elsif ($count == 0)
    {
        AddModemError(3095, $Lnum, $model);
    }
    else
    {
        foreach $line (@list)
        {
            @tmp = split(',', $line, 4);
            @tmp2 = split(':', $tmp[2], 2);
            $L =  $tmp2[0];
            $T =  $tmp2[1];
        }

        @bytes = split(',', $tmp[3]);
        if (($count = @bytes) > 1)
        {
            AddModemError(3096, $L);
        }
        if (defined($bytes[0]))
        {
            $bytes[0] =~ s/^[\s\t]//;
            $bytes[0] =~ s/[\s\t]$//;
            if ($bytes[0] !~ /^[a-fA-F0-9][a-fA-F0-9]$/ )
            {
                AddModemError(3080, $L);
            }
            $DeviceType = hex($bytes[0]);
            if ($DeviceType > 5)
            {
                AddModemError(3097, $L);
            }
        }
    }

    $RefAddregList      = ${$InstList{$model}}{'ADDREG'};
    if (defined($RefAddregList))
    {   push(@{$RefAddregList},$DeviceType);    }

}
#-------------------------------------------------------------
#
# This sub looks at the modem's DCB structure   '
#
#-------------------------------------------------------------
sub
proc_DCB
{
my($Lnum, $model) = @_;
my(@tmp, @tmp2, @list);
my($RefAddregList);
my($line,$count,$L,$T,@bytes);

    @list = getPathKey(",DCB",1);
    if (($count = @list) > 1)
    {
        AddModemError(3092, $Lnum, $model);
        return;
    }

    return if ($count == 0);

    foreach $line (@list)
    {
        @tmp = split(',', $line, 4);
        @tmp2 = split(':', $tmp[2], 2);
        $L =  $tmp2[0];
        $T =  $tmp2[1];
        return if ($T !~ /^1$/);
    }

    @bytes = split(',', $tmp[3]);
    if (($count = @bytes) != 26)
    {
        AddModemError(3093, $L);
    }

    my(@dwords) = ();
    @dwords = get_dwords(4,$L, splice(@bytes, 0, 12));
    @dwords = (@dwords, get_dwords(2,$L, splice(@bytes, 0, 6)) );
    @dwords = (@dwords, get_dwords(1,$L, @bytes));

    if (!defined($InstallProps{uc $model}))
    {
        $InstallProps{uc $model} = [];
    }

    my(@DCBStruct, $dw);
    @DCBStruct = ();
    foreach $dw (@dwords)
    {   push(@DCBStruct, hex($dw)); }
    ${$InstallProps{uc $model}}[1] = \@DCBStruct;

    $RefAddregList      = ${$InstList{$model}}{'ADDREG'};
    if (hex($dwords[1]) >= 230000 &&
        defined($$RefAddregList[2]) &&
        $$RefAddregList[2] == 1)
    {
        AddModemError(3098, $Lnum, $model);
    }


}

#------------------------------------------------------------
# Check if minimal install keys present
#   for voice modem ("Voice" in the friendly name ?)
# Use %VoiceInstallList filled up in &voice_minimal_keys()
#------------------------------------------------------------
sub
CheckVoice
{
my($CLnum, $ModemKey, $ModemRef, $InstallSection, $ModemName);

print(STDERR "\tInvoking CheckVoice...\n") if ($DEBUG);

    while (($ModemKey, $ModemRef) = each %AllModels)
    {
        $InstallSection = $$ModemRef[1];
        $CLnum  = $$ModemRef[6];
        $ModemName=$$ModemRef[0];
        next if (!defined($ModemName));
        next if ($ModemName !~ /VOICE/i);

#  Does not make sense to check for voice functionality when voice is present in modem's description string
#        if (! defined($VoiceInstallList{uc $InstallSection}) ) {
#            AddModemError(3106, $CLnum, $ModemName, $$ModemRef[0]);
#
#        }
    }
    print(STDERR "\tExiting CheckVoice...\n") if ($DEBUG);
}


#-------------------------------------------------------------
#
# Check modem ids consistency.
#       - If PCMCIA flag in the install section, one id is prefixed with
#       either
#       "PCMCIA\", "SERENUM\", "MF\" or "*"
#       and an UNIMODEM id must be present.
#       - If SERENUM, PCMCIA, STAR, MF type an UNIMODEM id must be present.
#       - Error if more than one type SERENUM, STAR, MF or PCMCIA.
#
#
# Globals changed:
#
# Usage:
#   CheckIds($section_name)
#
#-------------------------------------------------------------
sub 
CheckIds
{
my($InstSection, $Model, $ModelProp, @Properties, @IdLine, $IdIndex);
my($IsUNKNOWN, $IsPCMCIA, $IsSERENUM, $IsUNIMODEM, $IsSTAR, $IsMF, $SpecialIdCount);
my($bUnimodemRequired, $Type, $SerenumMatchModel);

#       iterate Install sections
print(STDERR "\tInvoking CheckIds...\n") if ($DEBUG);

    while (($Model, $ModelProp) = each(%AllModels))
    { 
        $Model = $$ModelProp[0];
        @IdLine = @{$$ModelProp[5]};
        $InstSection = $$ModelProp[1];
        $Type   = $$ModelProp[7];

        $IsUNKNOWN = index( $Type, $IdType'UNKNOWN_ID);
        $IsPCMCIA  = index( $Type, $IdType'PCMCIA_ID);
        $IsSERENUM = index( $Type, $IdType'SERENUM_ID);
        $IsMF      = index( $Type, $IdType'MF_ID);
        $IsUNIMODEM =index( $Type, $IdType'UNIMODEM_ID);
        $IsSTAR =index( $Type, $IdType'STAR_ID);

    
        # conflict types
        $SpecialIdCount = ($IsPCMCIA >= 0) + ($IsSERENUM >= 0) + ($IsSTAR >= 0) + ($IsMF >= 0);
    #   add_ref_wrn("$Model : only one PnP id type is recommended.",
    #   $$ModelProp[6])
        AddModemWarning(4013, $$ModelProp[6], $Model)
            if ($SpecialIdCount > 1);


        if ($IsSTAR >= 0)
        {
            if (! $inffile->sectionDefined("$InstSection.POSDUP") )
            {
                #   add_ref_err("Section [$InstSection.POSDUP] expected but
                #   missing.", $TotalLines)
                AddModemWarning(4115, $IdLine[$IsSTAR], $InstSection);
            }
        else
        {
        ChkInf::Mark(uc("$InstSection.POSDUP"));
        }
        }

        $bUnimodemRequired = 0;


        # PCMCIA is present in Install Section
        if ( index( $InstallTypeList{$InstSection},
                    $InstallType'PCMCIA_INST) >= 0  )
        {
            $IdIndex = $IsUNKNOWN;
            # Unknown ids without PCMCIA prefix
            while ($IdIndex >= 0)
             { 
                #   add_ref_err("PCMCIA\\ prefix required for PCMCIA-modem
                #   id.", $IdLine[$IdIndex] );
                AddModemError(3035, $IdLine[$IdIndex] );
                $IdIndex = index( $Type, $IdType'UNKNOWN_ID, $IdIndex+1);
             }
         
            # has a UNIMODEM id but no PCMCIA, SERENUM, UNKNOWN
            if ( $IsUNIMODEM >= 0 && 
                $IsSTAR < 0 &&
                $IsPCMCIA < 0 &&
                $IsSERENUM < 0 && 
                $IsMF < 0 && 
                $IsUNKNOWN < 0)
            {
                # a SERENUM or STAR modem can refer to this Install section
                # that has only
                # a UNIMODEM ID
                # still is not a SERENUM or STAR
                if ( index( $InstallIDTypes{$InstSection}, $IdType'SERENUM_ID)
< 0 && 
                     index( $InstallIDTypes{$InstSection}, $IdType'STAR_ID) < 0 )
                {
                    #   add_ref_err("$Model : PCMCIA id required.",
                    #   $$ModelProp[6]);
                    AddModemError(3036, $IdLine[$IsUNIMODEM]);
                }
            }
            # is unknown and no UNIMODEM id
            if ( $IsUNKNOWN >= 0 && 
                 $IsUNIMODEM < 0 && 
                 !$bUnimodemRequired )
            { 
                #  add_ref_err("$Model : UNIMODEM id required for
                #  PCMCIA-modems.", $$ModelProp[6]);
                AddModemWarning(4112, $IdLine[$IsUNKNOWN]);
                $bUnimodemRequired = 1;
            }
        }


        # special test for SERENUM and STAR modems
        if ($IsSERENUM >= 0)
        {
            if ($IsUNIMODEM >= 0)
            {
                #   "SERENUM modem: no unimodem ID is permitted for the same
                #   parameterization."
                AddModemError(3101, $IdLine[$IsUNIMODEM]);
            }
            if (! $inffile->sectionDefined("$InstSection.NORESDUP") )
            {
            # Disabled for Whistler
            #    #   add_ref_err("Section [$InstSection.NORESDUP] expected but
            #    #   missing.", $TotalLines)
            #    AddModemError(3045, $IdLine[$IsSERENUM], $InstSection);
            } else {
                my($TempVar1,$TempVar2);
                $SerenumMatchModel = &SerenumMatch($InstSection);

                # has a SERENUM prefix but no UNIMODEM id
                if (!defined($SerenumMatchModel)) { 
                    #add_ref_err("$Model : UNIMODEM id required for SERENUM-modems.", $$ModelProp[6]);
                    AddModemWarning(4113, $IdLine[$IsSERENUM]);
                } else {
                    if (((uc $SerenumMatchModel) cmp (uc $Model)) != 0) {   
                        AddModemWarning(4103, $IdLine[$IsSERENUM],$SerenumMatchModel);
                    }
                }
                ChkInf::Mark(uc("$InstSection.NORESDUP"));
            }
        }

        # search for UNIMODEM id in the modems installed in the section
        if ($IsUNIMODEM < 0 && $IsSTAR >= 0)
        {   $IsUNIMODEM = index( $InstallIDTypes{$InstSection}, $IdType'UNIMODEM_ID);
        }

        if (!$bUnimodemRequired && $IsUNIMODEM < 0)
        {
            # has a PCMCIA prefix but no UNIMODEM id
            if ($IsPCMCIA >= 0)
            { 
                # add_ref_err("$Model : UNIMODEM id required for PCMCIA-modems.",
                # $$ModelProp[6]);
                AddModemWarning(4112, $IdLine[$IsPCMCIA]);
            }

            # has a MF prefix but no UNIMODEM id
            if ($IsMF >= 0)
            { 
                #add_ref_err("$Model : UNIMODEM id required for MF-modems.", $$ModelProp[6]);
                #AddModemError(3091, $IdLine[$IsMF]); bug #515524 - Unimodem IDs no longer required in XP
            }

            # has a STAR prefix but no UNIMODEM id
            if ($IsSTAR >= 0)
            { 
                #add_ref_err("$Model : UNIMODEM id required for STAR-modems.", $$ModelProp[6]);
                AddModemWarning(4114, $IdLine[$IsSTAR]);
            }
        }
    }
print(STDERR "\tExiting CheckIds...\n") if ($DEBUG);
}

#------------------------------------------------------------
# This sub also checks for proper posdup and noresdup sections.
#
# Globals changed:
#
# Usage:
#       modem_section_usage()
#------------------------------------------------------------

sub
modem_section_usage {
    my $sec;
    foreach $sec (sort $inffile->getSections() ) {
        if ($sec =~ /(.+)\.NORESDUP/) {
            &checkNORESDUP($sec, 1);
            next;
        }
        if ($sec =~ /(.+)\.POSDUP/) {
            &checkPOSDUP($sec);
            next;
        }
    }
}

#-------------------------------------------------------------
#
# Checks [XXX.NORESDUP] section for SERENUM models.
#
# Usage:
#   checkNORESDUP(NoResDupSection)
#
#-------------------------------------------------------------
sub checkNORESDUP {
    my ($section, $FlagErrs) = @_;
    return unless ($inffile->sectionDefined($section));
    my ($line, @IDs, $Id);
    my (@UnimodemIDs);
    my @lines  =  $inffile->getSection($section);
    my $linetxt;

    @UnimodemIDs = ();
    if (!defined($FlagErrs)) {
        $FlagErrs = 1;
    }

    return if (! $inffile->sectionDefined($section) );
    $line=$inffile->getIndex($section) - 1;

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/); # Skip if it's a blank line
        # the section should contains only UNIMODEM ids
        @IDs = &get_list($linetxt,$inffile->getIndex($section,$line), $FlagErrs);
        foreach $Id (@IDs) {
            #if ($Id =~ /^__QUOTED_STR_\(([0-9]*)\)__$/) {
            if ($Id =~ /^\"/)
        {
        $Id =~ s/^\"//;
        $Id =~ s/\"$//;
                #$Id = ChkInf::QuotedString($1);
                AddModemError(1051, $inffile->getIndex($section,$line));
            }
            if ($Id =~ /$^/) {
                AddModemError(1027, $inffile->getIndex($section,$line)) if ($FlagErrs == 1);
                next;
            }
            if( $Id !~ /^UNIMODEM(.*)/i) {
                # "%s: Only UNIMODEM ids allowed in NORESDUP section."
                AddModemError(3102, $inffile->getIndex($section,$line), $Id) if ($FlagErrs == 1);
            } else {
                push(@UnimodemIDs, $Id) if (defined($AllIDList{uc $Id})); }
            if (!defined($AllIDList{uc $Id})) {
                AddModemError(1035,$inffile->getIndex($section,$line),$Id) if ($FlagErrs == 1);
            }
        }
        $line++;
    }
    @UnimodemIDs;
}
#------------------------------------------------------------
# this sub extracts a list of elements seperated by ',' from
# the given line. It strips leading and ending whitespaces.
# It flags error for whitespaces in elements.
#
# Usage:
#       get_list(line, line_num, check_empty)
#------------------------------------------------------------
sub get_list
{
my($line, $Lnum, $check_empty) = @_;
my(@list, $element);

    if (!defined($check_empty))
    {
        $check_empty    = 0;
    }
    @list = split(',', $line);
    foreach $element (@list)
    {
        $element =~ s/^[\s\t]*//; # remove blanks at ends.
        $element =~ s/[\s\t]*$//; # remove blanks at ends.

#       add_ref_wrn("White spaces in identifier \"$element\".", $Lnum)
        AddModemWarning(2011, $Lnum, $element)
            if ($element =~ /[\s\t]+/);
    }
    @list;  #return value.
}

#-------------------------------------------------------------
#
# Checks [XXX.POSDUP] section for STAR models.
#
# Usage:
#   checkPOSDUP(PosdupSection)
#
#-------------------------------------------------------------
sub checkPOSDUP {

    my $section = shift;
    my $count   = 0;
    my $present = 0;
    my $line    = 0;
    return unless ($inffile->sectionDefined($section));
    my @lines   = $inffile->getSection($section);
    my $linetxt;

    foreach $linetxt (@lines) {
        $count++, next if ($linetxt =~ /^\s*$/); # Skip if it's a blank line
        $present++ if ($linetxt =~ /^\*PNP0500$/i);
        $count++;
        $line++;
    }

    if ($count == 0) {
        AddModemError(3086, $inffile->getIndex($section)-1, $section);
        return;
    }

    if ($present == 0) {
        AddModemError(3087, $inffile->getIndex($section)-1, $section);
    }

    if ($line > 1) {
        AddModemError(3088, $inffile->getIndex($section)-1, $section);
    }
}

#-------------------------------------------------------------
#
# Find a model that a Serenum modem could match
#
#-------------------------------------------------------------
sub SerenumMatch
{
my($InstSection)=@_;
my(@UnimodemIds, $Id);
my($IdRef, $ModelRef);

    @UnimodemIds = ();

    @UnimodemIds = &checkNORESDUP("$InstSection.NORESDUP",0);

    foreach $Id (@UnimodemIds)
    {
        my $IdDefinition;

        next if (!defined($IdRef = $AllIDList{uc $Id}));
        foreach $IdDefinition (@{$IdRef})
        {
            next if (!defined($ModelRef = $AllModels{uc $IdDefinition->[0]}));
            if ( ((uc $$ModelRef[1]) cmp (uc $InstSection)) == 0)
            {   return $IdDefinition->[0]; }

        }
    }
    return undef;
}


#-------------------------------------------------------------
#
# Checks the ATCommands :
#   - correctness of the sequences for multiple commands
#   - AT..<cr> syntax
#
# Usage:
#   CheckATCommands(Install_sect, HKRLinesArray)
#
#-------------------------------------------------------------
sub
CheckATCommands
{   my($InstSect, $command, @CommandList) = @_;

    &CheckSeqCommand($InstSect, $command , @CommandList);
    &CheckATString(@CommandList);
}
#------------------------------------------------------------
# this sub extracts the LHS from a line.
# It ensures that the line has a '=' and that the left of '='
# is just one word. Else it generates an error.
# Replaces text if it is a quoted string.
#
# Usage:
#       getLHS(line, line_num)
#------------------------------------------------------------
sub getLHS
{
my($LHS, $this_num) = @_;

    $LHS =~ s/=.*$/=/;
    $LHS =~ s/^\s*//;
    $LHS =~ s/[\s\t]*=//;
    $LHS; #return value
}
