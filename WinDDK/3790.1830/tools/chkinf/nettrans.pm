# Copyright 1999 Microsoft Corporation

package NETTRANS;                # The name of this module. For our purposes,
                                 #   must be all in caps.

    use strict;                  # Keep this code clean
    my($Version) = "5.0.2402.0"; # Version of this file
    my($DEBUG)        = 0;       # Set to 1 for debugging output

    sub FALSE { return(0); } # Utility subs
    sub TRUE  { return(1); }

    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my(@InfLines);        # Array to hold the INF lines
    my(@AddRegs);         # Array holds AddRegs for current model

    my($inffile);         # Holds name of INF currently being processed.

    my(%AllAddRegs);      # Array holds all AddReg lines, index by INF Line Number
    my(%AllModels);       # Array of all models present in INF (Model) = (Type)

    my(%AddRegList);
    my(%AddServiceList);
    my($NDIKeyPresent);

    my %InstList;
    my %InstallExtensions;
    my %AllIDList;

    my($WinsockService,%WinsockProviderID);


    my(%NetCardErrorTable) = (
    3043 => ["Keyword Characteristics not defined in install section"],
    3045 => ["Keyword Compatible not defined in Version section"],
    3046 => ["Keyword Compatible should be equal to 1 if Signature is \$CHICAGO\$"],
    4003 => ["NCF_NO_SERVICE cannot be used with NCF_VIRTUAL or NCF_PHYSICAL."],
    4004 => ["Characteristics cannot have any of NCF_VIRTUAL,NCF_PHYSICAL,NCF_SOFTWARE_ENUMERATED,NCF_MULTIPORT_INSTANCED_ADAPTER or NCF_FILTER."],
    4005 => ["Invalid interface \"%s\" used in Addreg section"],
    4006 => ["Lower interface cannot have an install section"],
    4009 => ["NDI key should be created by Addreg section"],
    4010 => ["CoServices or ExcludeSetupStartServices key of Add-Registry Section should be of type REG_MULTI_SZ"],
    4011 => ["The AddService ServiceName parameter must match the NDI\\Service entry in the Registry."],
    4033 => ["Incorrect number of parameters, expected %s."],
    4100 => ["CoServices Key should be present if a Service is defined in Addreg Section"],
    4101 => ["\"%s\" Service found in ExcludeSetupStartServices not present in CoServices Key"],
    4102 => ["Format for CLSID GUID key in AddReg section is wrong"],
    4103 => ["Winsock Install Section not present"],
    4104 => ["AddSock Directive not present in Winsock Section"],
    4105 => ["TransportService value is not defined in Install Service Section"],
    4106 => ["Format for ProviderID GUID key in Winsock Install section is wrong"],
    4107 => ["Invalid Namespace \"%s\" used in Winsock Install section"],
    4108 => ["Winsock Remove Install Section not present"],
    4109 => ["DelSock Directive not present in Winsock Remove Section"],
    4110 => ["TransportService not matching for Winsock Install and Winsock Remove Section"],
    4111 => ["ProviderID not matching for Winsock Install and Winsock Remove Section"],
    4112 => ["TransportService Directive not present in Winsock Install Section"],
    4113 => ["HelperDllName Directive not present in Winsock Install Section"],
    4114 => ["MaxSockAddrLength Directive not present in Winsock Install Section"],
    4115 => ["MinSockAddrLength Directive not present in Winsock Install Section"],
    4116 => ["DelService value is not defined in Install Service Section"],
    4117 => ["DelService Directive not present in Services Remove Section"],
    4120 => ["Both Upper and Lower Interfaces should be present in AddReg Section"],
    4206 => ["Services section not defined for \"%s\" model"],
    4207 => ["\"%s\" file not allowed in CopyFiles section"],
    4208 => ["The values are incorrectly specified in RegisterDLL section"],
    4209 => ["PrintMonitor Install Section not present"],
    4210 => ["AddMonitor Directive not present in PrintMonitor Section"],
    4211 => ["PrintMonitorName Directive not present in PrintMonitor Install Section"],
    4212 => ["PrintMonitorDLL Directive not present in PrintMonitor Install Section"],
    4213 => ["PrintMonitor Remove Install Section not present"],
    4214 => ["DelMonitor Directive not present in PrintMonitor Remove Section"],
    4215 => ["Period required as prefix for MillenniumPreferred Directive in Version Section"],
    );

    my(%NetCardWarningTable) = (
    5001 => ["PnP id \"%s\" requires ExcludeFromSelect. Vendors or INF floppy install users may ignore this message."],
    4014 => ["First field should be one of the HKR, HKU, HKLM, HKCR or HKCU keywords."],
    );

    my(%ValidUpperInterfaces) = ('NDIS5' => 1,'NDISATM' => 1,'NETBIOS' => 1, 'IPX' => 1, 'TDI' => 1,'NDIS5_ATALK' => 1,
                     'NDISCOWAN' => 1,'NDISATM' => 1,'NDIS5_DLC' => 1, 'NDIS5_IP' => 1, 'NDIS5_IPX' => 1,
                     'NDIS5_STREAMS' => 1, 'WINSOCK' => 1, 'NDIS5_NBF' => 1, 'NOUPPER' =>1, 'NDISWAN' => 1);

    my(%ValidLowerInterfaces) = ('ETHERNET' => 1, 'ATM' => 1, 'TOKENRING' => 1, 'SERIAL' => 1, 'FDDI' => 1,
                    'BASEBAND' => 1, 'BROADBAND' => 1,'ARCNET' => 1, 'ISDN' => 1, 'NOLOWER' => 1,
                    'NDIS5' => 1,'NDISATM' => 1, 'LOCALTALK' => 1,'WAN' => 1, 'NDIS1394' => 1);

    my(%ValidNameSpaces) = ('0' => 1, '1' => 1, '2' => 1, '3' => 1, '10' => 1,
                    '11' => 1, '12' => 1,'13' => 1, '14' => 1, '20' => 1,
                    '30' => 1,'31' => 1, '32' => 1,'40' => 1, '41' => 1,'50' => 1);

    my(%LoadOrderGroup) = (
    'SYSTEM BUS EXTENDER' => 1, 'SCSI MINIPORT' => 1,'PORT' => 1, 'PRIMARY DISK' => 1,
    'SCSI CLASS' => 1, 'SCSI CDROM CLASS' => 1, 'FILTER' => 1, 'BOOT FILE SYSTEM' => 1,
    'BASE' => 1, 'POINTER PORT' => 1, 'KEYBOARD PORT' => 1, 'POINTER CLASS' => 1,
    'KEYBOARD CLASS' => 1, 'VIDEO INIT' => 1, 'VIDEO' => 1, 'VIDEO SAVE' => 1,
    'FILE SYSTEM' => 1, 'EVENT LOG' => 1, 'STREAMS DRIVERS' => 1, 'PNP_TDI' => 1,
    'NDIS' => 1, 'TDI' => 1, 'NETBIOSGROUP' => 1, 'SPOOLERGROUP' => 1, 'NETDDEGROUP' => 1,
    'PARALLEL ARBITRATOR' => 1, 'EXTENDED BASE' => 1, 'REMOTEVALIDATION' => 1,
    'PCI CONFIGURATION' => 1, );

    my(%ReservedWords) = (
    'CHARACTERISTICS' => 1, 'COMPONENTID' => 1, 'DESCRIPTION' => 1, 'DRIVERDESC' => 1,
    'INFPATH' => 1, 'INFSECTION' => 1, 'INFSECTIONEXT' => 1, 'MANUFACTURER' => 1,
    'NETCFGINSTANCEID' => 1, 'PROVIDER' => 1, 'PROVIDERNAME' => 1,);

    my(%ParamKeys) = ( 'PARAMDESC' => 1, 'DEFAULT' =>1, 'TYPE' => 1, 'MIN' => 1, 'MAX' => 1,
                'STEP' => 1, 'BASE' => 1,'OPTIONAL' => 1, 'LIMITTEXT' => 1, 'UPPERCASE' => 1, 'FLAG' => 1,);

#Assuming unsigned 64-bit value and unsigned 32-bit value

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

    my(%ValidDirectives) = ( "CHARACTERISTICS"  => TRUE,
                             "ADAPTERMASK"      => TRUE,
                             "EISACOMPRESSEDID" => TRUE,
                             "REGISTERDLLS" => TRUE,
                             "MAXINSTANCE"      => TRUE,
                             "COMPATIBLE"       => TRUE);
    return(FALSE) unless (defined($ValidDirectives{$Directive}));
    return(TRUE);
}


#-------------------------------------------------------------------------------------------------#
# Required subroutine! Allows us to verify that the module loaded correctly.                      #
#-- Exists ---------------------------------------------------------------------------------------#
sub Exists {
    print(STDERR "Module \"NETTRANS\" Loaded\n");
}

#-------------------------------------------------------------------------------------------------#
# Main Entry Point to Device Specific Module. REQUIRED!                                           #
#-- InfCheck() -----------------------------------------------------------------------------------#
sub InfCheck {
    my($Item);

    CheckCompatibleKey();

    # Propagate a list of all models found in the INF
    %AllModels  = LearnModels();

    # Create an array for each model.
    # Array name is uc(Model) and first element is Model
    foreach (keys(%AllModels)) {
        $Item = uc($_);
        no strict; # Indirect ref's not valid when using strict
        $$Item[0] = $_;
        use strict;
        # Check the model
        CheckModel($AllModels{$_}[1]);
    }
}

#-------------------------------------------------------------------------------------------------#
# Early initiation of module. REQUIRED!                                                           #
#-- InitGlobals($inffile) ------------------------------------------------------------------------#
sub InitGlobals {
    # Is called when module is first included.
    # Use it to set your per-inf variables.
    %AddRegList = ();
    %AddServiceList = ();

    # Is called when module is first included.
    # Use it to set your per-inf variables.
    $inffile = $_[1];

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
# CheckModel - Verifies an individual model                                                       #
#-- CheckModel($Model)  --------------------------------------------------------------------------#
sub CheckModel($Model) {
    print ("\tInvoking CheckModel(@_)\n") if ($DEBUG);

    my $Model  = shift;;
    my($MillSuffix) = CheckVersionForME("VERSION");
    
    if ($MillSuffix !~ /^$/) {
        VerifyModel($inffile->getIndex($Model.$MillSuffix), $Model, $MillSuffix)   if ( $inffile->sectionDefined($Model.$MillSuffix) );
    }

    if ( $inffile->sectionDefined($Model.".NTX86") || $inffile->sectionDefined($Model.".NTALPHA") ) {
        # We should work with this && .NTALPHA if exists
        VerifyModel( $inffile->getIndex($Model.".NTX86"),   $Model, ".NTX86");
    } elsif ( $inffile->sectionDefined($Model.".NT") ) {
        VerifyModel( $inffile->getIndex($Model.".NT"),   $Model, ".NT")
        # We should work with this model!
    } elsif ( $inffile->sectionDefined($Model) ) {
        # Our last choice section since dual OS INF use this for 9x
        VerifyModel( $inffile->getIndex($Model),   $Model)
    } else {
        # Model Must not exist
        # We trust that ChkInf.pm has already flagged all missing sections, so move on.
        return(FALSE);
    }
    print ("\tExiting ChkeckModel(@_)\n") if ($DEBUG);
    return(TRUE);
}

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
#-- AddNetCardError($ErrorNum, $LineNum, @ErrorArgs) -------------------------------------------------#
sub AddNetCardError {
    my($ErrorNum) = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@ErrorArgs)= @_;

    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    $info_err = $NetCardErrorTable{"$ErrorNum"};

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
#-- AddNetCardWarning($WarnNum, $LineNum, @ErrorArgs) ----------------------------------------------#
sub AddNetCardWarning {
    my($WarnNum)  = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@WarnArgs) = @_;

    $WarnArgs[0] = " " if (! defined($WarnArgs[0]));
    my($str, $this_str, $info_wrn, $format_wrn);

    $info_wrn = $NetCardWarningTable{"$WarnNum"};

    if ( !defined($info_wrn) ) {
        $this_str = "Unknown warning $WarnNum.";
    } else {
        $format_wrn = $$info_wrn[0];
        $this_str = sprintf($format_wrn, @WarnArgs);
    }
    ChkInf::AddDeviceSpecificWarning($LineNum, $WarnNum, $this_str);
}
#-------------------------------------------------------------------------------------------------
#-- CheckVersionForME($Section)
sub CheckVersionForME {
    my($Section)  = shift;
    my(@lines)    = $inffile->getSection($Section);
    my($SecStart) = $inffile->getIndex($Section,0)-1;
    my($DONE)     = FALSE;
    my($line);
    my($count)    = 0;
    my($Directive, $Value);
    my($ChicagoINF) = 0;
    my($Compatible) = 0;
    my($MillenniumSuffix) = "";

    foreach $line (@lines) {
        $count++, next if (! defined $line);

        ($Directive, $Value) =  ChkInf::SplitLine($Section,$count,$line);


        if (uc($Directive) eq "SIGNATURE") {
            $Value =~ s/\"//g;
            if (uc($Value) eq "\$CHICAGO\$") 
            {
                $ChicagoINF = 1;
            }
        }

        if (uc($Directive) eq "COMPATIBLE") {
                $Value =~ s/^\s*//; # Chop leading space
                $Value =~ s/\s*$//; # Chop tailing space
                if ($Value =~ /^1$/) {
                    $Compatible = 1;
                }
        }

        if (uc($Directive) eq "MILLENNIUMPREFERRED") {
            if (($ChicagoINF == 1) && ($Compatible == 1)) {
                $Value =~ s/^\s*//; # Chop leading space
                $Value =~ s/\s*$//; # Chop tailing space
                if ($Value =~ /^\./) {
                    $MillenniumSuffix = $Value;
                }
                else {
                    AddNetCardError(4208,$inffile->getIndex($Section,$count));
                    $MillenniumSuffix = "";
                }
            }
        }

        $count++;
    }

    return($MillenniumSuffix);
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

        {   my($SectionsDefined, $Extension);
            $SectionsDefined = 0;

            if ($inffile->sectionDefined($inst)) {
                $SectionsDefined++;
            }

            foreach $Extension (keys %InstallExtensions) {
                if ( $inffile->sectionDefined($inst.".$Extension") ) {
                    $SectionsDefined++;
                    $InstList{uc ($inst.".$Extension")} = 1;
                }
            }
        }

        $count = 0;
        foreach $ExcludeID (@id) {
            my $UCExcludeID = uc $ExcludeID;

            if ($ExcludeID !~ /^$/) {
                #   add to the list of all ids
                if (!defined($AllIDList{$UCExcludeID})) {
                    $AllIDList{$UCExcludeID} = [];
                }
                #   model, line, rank, original value
                push(@{$AllIDList{$UCExcludeID}}, [$model, $line, $count, $ExcludeID]);
            }
            $count++;   # this gives the rank of the ID
        }

        if (defined $NewModelList{uc $model}) {  # same modem with diff id.
            my($ROldModel);
            #add the new rank0 & rank1id and proceed.

            $ROldModel = $NewModelList{uc $model}; # pointer to the model array

                # add new ranks
            for($r = 0; $r < 3; $r++) {
               push( @{$$ROldModel[2+$r]} , (defined($id[$r])) ? $id[$r] : "" );
            }

                #  Compare if same modem -> same Inst section.
            if ( (uc $inst ne uc $$ROldModel[1]) && ! defined($ModelErrorList{$$ROldModel[0]}) ) {
                $ModelErrorList{$$ROldModel[0]} = 4011;
            }

            #  Add line number for ID definition.
            push(@{$$ROldModel[5]},  $line);
            $lcount++;
            next;
        }

        # Encountered first time, add the Install section name.
        $NewModelList{uc $model} = [$Directive, $inst] ;

        for($r = 0; $r < 3; $r++) {
            push(@{$NewModelList{uc $model}}, [ (defined($id[$r])) ? $id[$r] : "" ]);
        }

        # Add line number for ID definition.
        push(@{$NewModelList{uc $model}}, [$line]);

        $lcount++;
    }


    # report error for $sec
    foreach $model (keys %ModelErrorList) {
       AddModemWarning($ModelErrorList{$model}, $line-1, $model);
    }

    foreach $model (keys %NewModelList)  {
        #       Add line number for Section.
        push(@{$NewModelList{uc $model}}, $line-1);
    }
    return(%NewModelList);
}

#-------------------------------------------------------------------------------------------------#
#-- GetRegLines($Section) ------------------------------------------------------------------------#
sub GetRegLines {
    my($line)       = 0;
    my($Section)    = $_[0];

    $Section =~ s/\s*$//; # Chop trailing spaces
    $Section =~ s/^\s*//; # Chop starting spaces

    my($DONE)       = FALSE;
    my(@lines)      = $inffile->getSection($Section);
    my(@AddReg);
    my $linetxt;

    print(STDERR "\tBeginning GetRegLines (@_)...\n") if ($DEBUG);

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if it's a blank line

        @AddReg = (@AddReg, $linetxt);
        $AllAddRegs{$inffile->getIndex($Section, $line)} = 1;
        $line++;
    }
    print(STDERR "\tExiting GetRegLines ...\n") if ($DEBUG);
    return(@AddReg);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyModel($line, $Model, $Extension) -------------------------------------------------------#
sub VerifyModel {
    my($line)       = 0;
    shift;
    my $Model       = shift;
    my $Extension   = shift;
       $Extension="" unless defined $Extension;
    my $Section     = $Model.$Extension;
    $Section =~ s/\s*$//; # Chop trailing spaces!

    my(@lines)      = $inffile->getSection($Section);
    my($Directive, @Values, $Value);
    my($characteristics,$cpfiles,$lnum);
    my($Charac)     = FALSE;
    my(@AddRegSections,$addregsec);
    my $linetxt;
        
    %AddRegList = ();
    %AddServiceList = ();
    
    $WinsockService = "";
    %WinsockProviderID = (); 
    $NDIKeyPresent = 0;

    my(%NetDirectives) = (  'CHARACTERISTICS'   => " " );

    my(%SecToCheck) = ("ADDREG",    TRUE,
                       "COPYFILES", TRUE,
                       "NEEDS",     TRUE);
    my($CurrentLine, $Temp, @LineItems, $SizeOfArray);
    @AddRegs     =  ();

        print(STDERR "\tBeginning VerifyModel (@_)...\n") if ($DEBUG);
        foreach $linetxt (@lines) {

            $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

            ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);
            @Values             = ChkInf::GetList($Value,$inffile->getIndex($Section,$line));
            $Directive = uc $Directive;

            if ($Directive eq "ADDREG") {

                foreach $Value (@Values) {
                    $Value =~ s/^\s*//;
                    $Value =~ s/\s*$//;
                    if ( $inffile->sectionDefined($Value) ) {
                        @AddRegs = (@AddRegs, GetRegLines($Value));
                        push(@AddRegSections,$Value);
                    } else {
                        # This error is flagged by ChkInf.pm
                        #print(STDERR "\t\tCould not find section \"$Value\"\n");
                    }
                }

            }
            #check nic's specific directives
            if ($Directive eq "CHARACTERISTICS") {
                if (defined($Values[0]))
                {
                    $characteristics = $Values[0];
                    $characteristics =~ s/^\s*//;
                    $characteristics =~ s/^0x//;
                    $characteristics =~ s/^0X//;
                    $characteristics =~ s/\s*$//;
                    CheckCharacteristics($characteristics,$Section,$line);
                    $Charac=TRUE;
                }
            }
            if ($Directive eq "COPYFILES") {
                if (defined($Values[0]))
                {
                    $cpfiles = $Values[0];
                    $cpfiles =~ s/^\s*//;
                    $cpfiles =~ s/\s*$//;
                    if ($cpfiles !~ /^@/ )
                    {
                        CheckCopyFiles($inffile->getIndex($cpfiles)-1,$cpfiles);
                    }
                }
                else
                {
                    #Chkinf should flag an error if Value is missing
                }
            }
            if ($Directive eq "REGISTERDLLS") 
            {
                foreach $Value (@Values) 
                {
                    $Value =~ s/^\s*//;
                    $Value =~ s/\s*$//;
                    if ( $inffile->sectionDefined($Value) ) {
                        CheckRegDLLSection($Value);
                    } 
                }

            }

            $line++;
        }
        AddNetCardError(3043,$inffile->getIndex($Section)-1) unless ($Charac);


        $characteristics = 0 unless (defined($characteristics));
        foreach $addregsec (@AddRegSections) {
            AddRegProc($addregsec,$characteristics);
        }

        CheckServiceName($Section.".SERVICES") if ( $inffile->sectionDefined("${Section}.SERVICES") );
        CheckServiceRemoveSection($Section.".REMOVE.SERVICES") if ( $inffile->sectionDefined("${Section}.REMOVE.SERVICES") );
        
        CheckWinsockSection($Section.".WINSOCK") if ( $inffile->sectionDefined("${Section}.WINSOCK") );

        CheckWinsockRemoveSection($Section.".REMOVE.WINSOCK") if ( $inffile->sectionDefined("${Section}.REMOVE.WINSOCK") );

        CheckRemoveInstSection($Section.".REMOVE") if ( $inffile->sectionDefined("${Section}.REMOVE") );

        CheckPrintMonSection($Section.".PRINTMONITOR") if ( $inffile->sectionDefined("${Section}.PRINTMONITOR") );

        CheckPrintMonRemoveSection($Section.".REMOVE.PRINTMONITOR") if ( $inffile->sectionDefined("${Section}.REMOVE.PRINTMONITOR") );

        # We now have one massive array called @AddRegs that contins all the AddReg lines for
        # the modem we're currently looking at.

        #### Accumulate data about this modem
        #
        # Net Team will need to work on this routine!
        #
    
    if ($NDIKeyPresent == 0) {
        AddNetCardError(4009,$inffile->getIndex($Section)-1);
    }

    @AddRegs = ();
    @AddRegSections = ();

    print(STDERR "\tExiting VerifyModel ...\n") if ($DEBUG);
    return(TRUE);
}
#-------------------------------------------------------------
#
#   Checks if the given characteristics are correct
#
# Usage:
#   CheckCharacteristics(expected_section_name,line_num)
#
#-------------------------------------------------------------

sub
CheckCharacteristics
{
my($char,$sec, $Line_num) = @_;
my($RHS);
my($x,$y,$z);

    $RHS= hex($char);

    if (($RHS&0x10)&&(($RHS&0x1)||($RHS&0x4)||($RHS&0x2)))
    {
        AddNetCardError(4003,$Line_num);
    }
    if (($RHS&0x1)||($RHS&0x2)||($RHS&0x4)||($RHS&0x40)||($RHS&0x400))
    {
        AddNetCardError(4004,$Line_num);
    }

}

#-- CheckCopyFiles($line, $Section) ------------------------------------------------------#
sub CheckCopyFiles {
    my($line,$Section)    =  @_ ;
    $line = 0;
    my $linetxt;
    my @lines   = $inffile->getSection($Section);

    my($Destination, $Source, $Temp, $Flags);

    print(STDERR "\tInvoking CheckCopyFiles(@_)...\n") if ($DEBUG);
    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/); # Skip if blank line

        ($Destination, $Source, $Temp, $Flags) = split(/,/,$linetxt);

        $Destination =~ s/^\s*//;
        $Destination =~ s/\s*$//;

        if (!(($Destination =~ /sys$/i) || ($Destination =~ /dll$/i) || ($Destination =~ /hlp$/i )))
        {
            AddNetCardError(4207,$inffile->getIndex($Section, $line),$Destination);
        }

            $line++;
    }
    return(TRUE);
}
#-- ($Section) ------------------------------------------------------#
sub AddRegProc {

    my($Section,$ModelChar)    =  @_ ;

    my $line    = 0;
    my $linetxt;
    my @lines   = $inffile->getSection($Section);

    my(@LineItems,$CurrentLine);
    my($Temp,$SizeOfArray,$TempVar);
    my($UpperPresent) = 0;
    my($LowerPresent) = 0;
    my(@CoServices,@ExServices);
    my($ServicePresent) = 0;
    my($CoServicesPresent) = 0;
    my($NDIAddreg)= 0;
    
    @CoServices = ();
    @ExServices = ();

    print(STDERR "\tInvoking AddRegProc(@_)...\n") if ($DEBUG);

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        $CurrentLine = $linetxt;

        @LineItems = (); # Clear the array
        @LineItems = ChkInf::GetList($CurrentLine, $inffile->getIndex($Section, $line),0);
        $SizeOfArray = $#LineItems;
        for ($Temp=0; $Temp <= $SizeOfArray; $Temp++) {
            if (defined($LineItems[$Temp])) {
                $LineItems[$Temp] =~ s/^\s*//; # Remove leading whitespace
                $LineItems[$Temp] =~ s/\s*$//; # Remove trailing whitespace
                $LineItems[$Temp] = uc($LineItems[$Temp]);
            } else {
                $LineItems[$Temp] = " ";
            }
        }

        # Now that we have a clean, standardized line, verify it
        if (defined($LineItems[3]))
        {
            if (! defined($LineItems[4])) 
            {
                my($keytype,$temp);
                $keytype = $LineItems[3];

                if (!(($keytype =~ /00000010/) || ($keytype =~ /0x10/i)))
                {
                    
                    AddNetCardError(4033,$inffile->getIndex($Section,$line),"5");
                    $line++;
                    next;
                }
            }
        }
        for ($Temp=1; $Temp <= 2; $Temp++) 
        {
            if (defined($LineItems[$Temp])) 
            {
                    $LineItems[$Temp] = uc($LineItems[$Temp]);
            }
        }

        if ($LineItems[1] =~ /^NDI/i)
        {
            $NDIKeyPresent = 1;
            $NDIAddreg = 1;
            if ($LineItems[2] =~ /^SERVICE$/i)
            {
                $ServicePresent = 1;
                if ( defined $LineItems[4])
                {
                    my($ucAdd) = uc $LineItems[4];
                    while ($ucAdd =~ /^\"/)
                    {
                        $ucAdd =~ s/^\"//;
                        $ucAdd =~ s/\"$//;
                    }
                    $AddRegList{$ucAdd} = 1;
                }
            }

            if ($LineItems[1] =~ /INTERFACES/i)
            {
                my(@tmp3,$cnt,$i,$str); #set of interfaces can be defined..

                if ( defined $LineItems[4] )
                {
                    $str = $LineItems[4];;
                    $str  =~ s/\"//g;
                    @tmp3 = split(',',$str);
                    $cnt = $#tmp3+1;
                }

                if (!(defined($LineItems[4]))) { $cnt=0; } 
                for ($i=0; $i < $cnt; $i++)
                {
	                while ($tmp3[$i] =~ /^\"/)
                    {
                        $tmp3[$i] =~ s/^\"//;
                    }
	                while ($tmp3[$i] =~ /\"$/)
                    {
                        $tmp3[$i] =~ s/\"$//;
                    }
                    if ($LineItems[2] =~ /UPPERRANGE/i)
                    {
                        $UpperPresent = 1;
                        AddNetCardError(4005,$inffile->getIndex($Section,$line),$tmp3[$i])
                            if (! defined($ValidUpperInterfaces{uc $tmp3[$i]}));
                    }
                    if ($LineItems[2] =~ /LOWERRANGE/i)
                    {
                        $LowerPresent = 1;
                        AddNetCardError(4005,$inffile->getIndex($Section,$line),$tmp3[$i])
                            if (! defined($ValidLowerInterfaces{uc $tmp3[$i]}));
                    }
                }
            }
            if (($LineItems[2] =~ /COSERVICES/i)||($LineItems[2] =~ /EXCLUDESETUPSTARTSERVICES/i))
            {
                my($TempVar);
                if ($LineItems[3] =~ /%(.*?)%/) 
                {
                    $TempVar = quotemeta($1);
                }
                else
                {
                    $TempVar = $LineItems[3];
                }
                $TempVar =~ s/^\s*//; # Kill leading spaces
                $TempVar =~ s/\s*$//; # Kill trailing spaces
                if (($TempVar !~ /0x00010000/i) && ($TempVar !~ /0x10000/i))
                {
                    AddNetCardError(4010,$inffile->getIndex($Section,$line));
                }
            }

            if ($LineItems[2] =~ /COSERVICES/i)
            {
                # Make a list of all the services found in coservices
                # Also keep a flag if this entry is present 
                $CoServicesPresent = 1;
                my($cnt,$i,$coserv);
                $cnt = $#LineItems + 1;
                for ($i = 4; $i < $cnt; $i++)
                {
                    $coserv = $LineItems[$i];
                    push(@CoServices,$coserv);
                }       
            }
            if ($LineItems[2] =~ /EXCLUDESETUPSTARTSERVICES/i)
            {
                # Make a list of all the services found in excludesetupstartservices
                # Later check against the services found in coservices.
                my($cnt,$i,$exser);
                $cnt = $#LineItems + 1;
                for ($i = 4; $i < $cnt; $i++)
                {
                    $exser = $LineItems[$i];
                    push(@ExServices,$exser);
                }       
            }
            if ($LineItems[2] =~ /HELPTEXT/i)
            {
            }
            if ($LineItems[2] =~ /CLSID/i)
            {
                my($classid);
                if ( defined $LineItems[4] )
                {
                    $classid = $LineItems[4];
                }
                else
                {
                    $classid = "NULL";
                }
                if ($classid !~ /^\{[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}\}$/)
                {
                    AddNetCardError(4102,$inffile->getIndex($Section,$line));
                }
                
            }

        }   #ndi-if ends.
        else
        {
            if ($LineItems[1] =~ /^$/ )     #Defaults section...
            {
                #Also flag an error if the value-name doesnt satisfy "some" criterion..
                # This criteria is unknown for DynamicCRTMax, DynamicCRTIncrement,
                # DynamicCRTInterval, EventMessageFile, TypesSupported and TextModeFlags.
            }
        }

        $line++;
    }
    if ((($UpperPresent == 0)||($LowerPresent == 0))&&($NDIAddreg == 1))
    {
        AddNetCardError(4120,$inffile->getIndex($Section)-1);
    }
    if ($ServicePresent == 1)
    {
        if ($CoServicesPresent == 0)
        {
#           AddNetCardError(4100,$SecRef{$Section});
        }
        else
        {
            # Check if the exservices are present in coservices list
            my($exser);
            foreach $exser (@ExServices)
            {
                $exser =~ s/^\s*//; # Kill leading spaces
                $exser =~ s/\s*$//; # Kill trailing spaces
                my($present) = 0;
                my($coserv);
                foreach $coserv (@CoServices)
                {
                    $coserv =~ s/^\s*//; # Kill leading spaces
                    $coserv =~ s/\s*$//; # Kill trailing spaces
                    if ($exser eq $coserv)
                    {
                        $present = 1;
                    }
                }
                if ($present == 0)
                {
                    AddNetCardError(4101,$inffile->getIndex($Section)-1,$exser);
                }
            }
        }
    }
    print(STDERR "\tExiting AddRegProc(@_)...\n") if ($DEBUG);
    return(TRUE);
}
#-----------------------CheckServiceName($Section)-----------------------
sub CheckServiceName {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value, @Values);
    my($ucAddReg,$ucAddService);
    
    ChkInf::Mark(uc($Section));

    print(STDERR "\tInvoking CheckServiceName(@_)...\n") if ($DEBUG);

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);
        @Values    = ChkInf::GetList($Value,$inffile->getIndex($Section,$line));

        if ($Directive =~ /ADDSERVICE/i) {

            $ucAddService = uc $Values[0];
    
            if (defined($Values[0])) {
                $AddServiceList{$ucAddService} = $inffile->getIndex($Section,$line);
            }
        }

        $line++;
    }

    my($addregser,$addserv);
    my($servpresent) = 0;    
    foreach $addserv (keys %AddServiceList) {
        foreach $addregser (keys %AddRegList) {
            if (uc $addserv eq uc $addregser) {
                $servpresent = 1;
            }
        }
    }

    if ($servpresent == 0) {
       AddNetCardError(4011,$inffile->getIndex($Section) -1);
    }
   print(STDERR "\tExiting CheckServiceName(@_)...\n") if ($DEBUG);
    return(TRUE);
}

#-----------------------CheckCompatibleKey--------------------------------
sub CheckCompatibleKey {
    my $Section;
    my $line     =  0; 
    my @lines    =  $inffile->getSection("VERSION");;

    my($Directive,$Value);
    my($Signature,$CompVal,$CompPresent,$CompLine);
    my $count    = 0;

    $CompPresent = 0;

    print(STDERR "\tInvoking CheckCompatibleKey(@_)...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine("VERSION",$count,$line);

        if ($Directive =~ /SIGNATURE/i) {
            $Signature = $Value;
        }
        if ($Directive =~ /COMPATIBLE/i) {
            $CompVal = $Value;
            $CompPresent = 1;
            $CompLine = $inffile->getIndex("VERSION",$count);
        }

        $count++
    }

    if ($Signature =~ /CHICAGO/i) {
        if ($CompPresent == 1) {
            if ($CompVal != 1) {
                AddNetCardError(3046,$CompLine);
            }
        } else {
            AddNetCardError(3045,$CompLine);
        }
    }
    return(TRUE);
}
#-----------------------CheckWinsockSection($Section)-----------------------
sub CheckWinsockSection {
    print(STDERR "\tInvoking CheckWinsockSection(@_)...\n") if ($DEBUG);
    my $Section  =  shift;
    my $line     =  0; 
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my($AddSockPresent) = 0;
    
    ChkInf::Mark(uc($Section));

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);

        if ($Directive =~ /ADDSOCK/i) {
            $AddSockPresent = 1;
            if ( $inffile->sectionDefined($Value) ) {
                CheckWinsockInstall(uc $Value);
            } else {
                AddNetCardError(4103,$inffile->getIndex($Section,$line));
            }
        }
            
        $line++;
    }
    
    if ($AddSockPresent == 0) {
        AddNetCardError(4104,$inffile->getIndex($Section) - 1);
    }
    print(STDERR "\tExiting CheckWinsockSection(@_)...\n") if ($DEBUG);
    return(TRUE);
}
#-----------------------CheckWinsockInstall($Section)-----------------------
sub CheckWinsockInstall {
    print(STDERR "\tInvoking CheckWinsockInstall(@_)...\n") if ($DEBUG);
    my $Section  =  shift;
    my $line     =  0; 
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my($AddTransportService);
    my($ServiceKeyPresent) = 0;
    my($HelperDLLPresent) = 0;
    my($MinSizePresent) = 0;
    my($MaxSizePresent) = 0;
    my($ProviderIDPresent) = 0;

    ChkInf::Mark(uc($Section));

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);

        if ($Directive =~ /TRANSPORTSERVICE/i)  {
            $ServiceKeyPresent = 1;
            if (!(defined($AddRegList{uc $Value}))) {
                AddNetCardError(4105,$inffile->getIndex($Section,$line));
            } else {
                $WinsockService = uc $Value;
                $AddTransportService = uc $Value;
            }
        }

        if ($Directive =~ /HELPERDLLNAME/i) {
            $HelperDLLPresent = 1;
        }

        if ($Directive =~ /MAXSOCKADDRLENGTH/i) {
            $MaxSizePresent = 1;
        }

        if ($Directive =~ /MINSOCKADDRLENGTH/i) {
            $MinSizePresent = 1;
        }
        if ($Directive =~ /PROVIDERID/i) {
            $ProviderIDPresent = 1;
            my($classid);
            if ( defined $Value ) {
                $classid = $Value;
            } else {
                $classid = "NULL";
            }
            if ($classid !~ /^\{[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}\}$/) {
                AddNetCardError(4106,$inffile->getIndex($Section,$line));
            }
            $WinsockProviderID{uc $classid} = $AddTransportService;
        }

        if ($Directive =~ /DISPLAYSTRING/i) {
        }
        if ($Directive =~ /SUPPORTEDNAMESPACE/i)
        {
            AddNetCardError(4107,$inffile->getIndex($Section,$line),$Value) if (!defined($ValidNameSpaces{uc $Value}));
        }
        $line++;
    }
    if ($ServiceKeyPresent == 0) {
        AddNetCardError(4112,$inffile->getIndex($Section) - 1);
    }
    if ($HelperDLLPresent == 0) {
        AddNetCardError(4113,$inffile->getIndex($Section) - 1);
    }
    if ($MaxSizePresent == 0) {
        AddNetCardError(4114,$inffile->getIndex($Section) - 1);
    }
    if ($MinSizePresent == 0) {
        AddNetCardError(4115,$inffile->getIndex($Section) - 1);
    }
    if ($ProviderIDPresent == 0) {
        $WinsockProviderID{"NOPROVIDERID"} = $AddTransportService;
    }       
    return(TRUE);
}

#-----------------------CheckWinsockRemoveSection($Section)-----------------------
sub CheckWinsockRemoveSection {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my($DelSockPresent) = 0;
    
    ChkInf::Mark(uc($Section));
    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);

        if ($Directive =~ /DELSOCK/i) {
            $DelSockPresent = 1;
            if ( $inffile->sectionDefined($Value) ) {
                CheckWinsockRemoveInstall(uc $Value);
            } else {
                AddNetCardError(4108,$inffile->getIndex($Section,$line));
            }
        }
            
        $line++;
    }
    
    if ($DelSockPresent == 0) {
        AddNetCardError(4109,$inffile->getIndex($Section, - 1) );
    }
    return(TRUE);
}
#-----------------------CheckWinsockRemoveInstall($Section)-----------------------
sub CheckWinsockRemoveInstall {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my($DelServiceValue);
    
    ChkInf::Mark(uc($Section));
    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);

        if ($Directive =~ /TRANSPORTSERVICE/i) {
            $Value = uc $Value;
            $DelServiceValue = $Value;
            my($servpresent,$id);
            $servpresent = 0;
            foreach $id (keys %WinsockProviderID) {
                if (uc $WinsockProviderID{$id} eq uc $Value) {
                    $servpresent = 1;
                }
            }
            if ($servpresent == 0) {
                AddNetCardError(4110,$inffile->getIndex($Section,$line));
            }
        }

        if ($Directive =~ /PROVIDERID/i) {
            my($classid,$servval);
            if ( defined $Value )
            {
                $classid = $Value;
            }
            else
            {
                $classid = "NULL";
            }
            $classid = uc $classid;
            $servval = uc $WinsockProviderID{$classid};
            if ($servval ne $DelServiceValue)
            {
                AddNetCardError(4111,$inffile->getIndex($Section,$line));
            }
        }

        $line++;
    }
    return(TRUE);
}

#-----------------------CheckServiceRemoveSection($Section)-----------------------
sub CheckServiceRemoveSection {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my($DelServicePresent) = 0;
    my(@Values,@Sections);
    
    ChkInf::Mark(uc($Section));
    print(STDERR "\tInvoking CheckServiceRemoveSection(@_)...\n") if ($DEBUG);
    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);

        if ($Directive =~ /DELSERVICE/i) {
            $DelServicePresent = 1;
            if (!(defined($AddServiceList{uc $Value}))) {
                AddNetCardError(4116,$inffile->getIndex($Section,$line))
            }
        }
        if ($Directive =~ /DELREG/i) {
            my($addregsec);
            @Values     = split(/,/, $Value);
            foreach $Value (@Values) {
                $Value =~ s/^\s*//;
                $Value =~ s/\s*$//;
                if ( $inffile->sectionDefined($Value) ) {
                    push(@Sections,$Value);
                } 
            }
            foreach $addregsec (@Sections) {
                AddRegProc($addregsec,0);
                ChkInf::Mark(uc($addregsec));
            }
        }
             
        $line++;
    }

    if ($DelServicePresent == 0) {
        AddNetCardError(4117,$inffile->getIndex($Section) - 1);
    }
    print(STDERR "\tExiting CheckServiceRemoveSection(@_)...\n") if ($DEBUG);
    return(TRUE);
}
#-----------------------CheckRemoveInstSection($Section)-----------------------
sub CheckRemoveInstSection {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my(@Values,@Sections);
    
    ChkInf::Mark(uc($Section));

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);

        if ($Directive =~ /DELREG/i) {
            my($addregsec);
            @Values     = split(/,/,$Value);
            foreach $Value (@Values) {
                $Value =~ s/^\s*//;
                $Value =~ s/\s*$//;
                if ( $inffile->sectionDefined($Value) ) {
                    push(@Sections,$Value);
                } 
            }
            foreach $addregsec (@Sections) {
                AddRegProc($addregsec,0);
                ChkInf::Mark(uc($addregsec));
            }
        }
        if ($Directive eq "UNREGISTERDLLS") 
        {
            @Values     = split(/,/, $Value);
            foreach $Value (@Values) {
                $Value =~ s/^\s*//;
                $Value =~ s/\s*$//;
                if ( $inffile->sectionDefined($Value) ) {
                    CheckRegDLLSection($Value);
                } 
            }
        
        }
            
        $line++;
    }
    return(TRUE);
}
#-----------------------CheckRegDLLSection($Section)-----------------------
sub CheckRegDLLSection {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my(@Values,@Sections);
    
    ChkInf::Mark(uc($Section));

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        @Values     = split(/,/, $linetxt);
        foreach $Value (@Values) {
            $Value =~ s/^\s*//;
            $Value =~ s/\s*$//;
        }
        
        if (($Values[0] =~ /^$/)||($Values[2] =~ /^$/)||($Values[3] =~ /^$/)) {
            AddNetCardError(4208,$inffile->getIndex($Section,$line));
        }
            
        $line++;
    }
    return(TRUE);
}
#-----------------------CheckPrintMonSection($Section)-----------------------
sub CheckPrintMonSection {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my($AddMonPresent) = 0;
    
    ChkInf::Mark(uc($Section));

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);

        if ($Directive =~ /ADDMONITOR/i) {
            $AddMonPresent = 1;
            if ( $inffile->sectionDefined( $Value) ) {
                CheckPrintMonInstall(uc $Value);
            } else {
                AddNetCardError(4209,$inffile->getIndex($Section,$line));
            }
        }
            
        $line++;
    }
    
    if ($AddMonPresent == 0) {
        AddNetCardError(4210,$inffile->getIndex($Section) - 1);
    }
    return(TRUE);
}
#-----------------------CheckPrintMonInstall($Section)-----------------------
sub CheckPrintMonInstall {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);

    my($PrintMonNamePresent) = 0;
    my($PrintMonDLLPresent) = 0;

    ChkInf::Mark(uc($Section));

    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);

        if ($Directive =~ /PRINTMONITORNAME/i) {
            $PrintMonNamePresent = 1;
        }

        if ($Directive =~ /PRINTMONITORDLL/i) {
            $PrintMonDLLPresent = 1;
        }

        $line++;
    }
    if ($PrintMonNamePresent == 0) {
        AddNetCardError(4211,$inffile->getIndex($Section)-1);
    }
    if ($PrintMonDLLPresent == 0) {
        AddNetCardError(4212,$inffile->getIndex($Section)-1);
    }
    return(TRUE);
}

#-----------------------CheckPrintMonRemoveSection($Section)-----------------------
sub CheckPrintMonRemoveSection {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my($DelMonPresent) = 0;
    
    ChkInf::Mark(uc($Section));
    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);

        if ($Directive =~ /DELMONITOR/i) {
            $DelMonPresent = 1;
            if ( $inffile->sectionDefined( $Value) ) {
                CheckPrintMonRemoveInstall(uc $Value);
            } else {
                AddNetCardError(4213,$inffile->getIndex($Section, $line));
            }
        }
            
        $line++;
    }
    
    if ($DelMonPresent == 0) {
        AddNetCardError(4214,$inffile->getIndex($Section)-1);
    }
    return(TRUE);
}
#-----------------------CheckPrintMonRemoveInstall($Section)-----------------------
sub CheckPrintMonRemoveInstall {
    my($Section) =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $linetxt;

    my($Directive,$Value);
    my($DelServiceValue);
    
    ChkInf::Mark(uc($Section));
    foreach $linetxt (@lines) {
        $line++, next if ($linetxt =~ /^\s*$/);# Skip if blank line
        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$linetxt);
        $line++;
    }
    return(TRUE);
}
