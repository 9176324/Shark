# Copyright 1999 Microsoft Corporation

package NETSERVICE;                # The name of this module. For our purposes,
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
    
    my($WinsockService,%WinsockProviderID);

    my(%NetCardErrorTable) = (
    3043 => ["Keyword Characteristics not defined in install section"],
    3045 => ["Keyword Compatible not defined in Version section"],
    3046 => ["Keyword Compatible should be equal to 1 if Signature is \$CHICAGO\$"],
    4003 => ["NCF_NO_SERVICE cannot be used with NCF_VIRTUAL or NCF_PHYSICAL."],
    4004 => ["Characteristics cannot have any of NCF_VIRTUAL,NCF_PHYSICAL,NCF_SOFTWARE_ENUMERATED,".
             "NCF_MULTIPORT_INSTANCED_ADAPTER or NCF_FILTER."],
    4005 => ["Invalid interface \"%s\" used in Addreg section"],
    4006 => ["Lower interface cannot have an install section"],
    4009 => ["NDI key should be created by Addreg section"],
    4010 => ["CoServices or ExcludeSetupStartServices key of Add-Registry Section should be of type REG_MULTI_SZ"],
    4011 => ["The AddService ServiceName parameter must match the NDI\\Service entry in the Registry."],
    4033 => ["Incorrect number of parameters, expected %s."],
    4100 => ["CoServices Key should be present if a Service is defined in Addreg Section"],
    4101 => ["\"%s\" Service found in ExcludeSetupStartServices not present in CoServices Key"],
    4102 => ["Format for CLSID GUID key in AddReg section is wrong"],
    4116 => ["DelService value is not defined in Install Service Section"],
    4117 => ["DelService Directive not present in Services Remove Section"],
    4120 => ["Both Upper and Lower Interfaces should be present in AddReg Section"],
    4206 => ["Services section not defined for \"%s\" model"],
    4207 => ["\"%s\" file not allowed in CopyFiles section"],
    4208 => ["The values are incorrectly specified in RegisterDLL section"],
    4209 => ["Period required as prefix for MillenniumPreferred Directive in Version Section"],
    );

    my(%NetCardWarningTable) = (
    5001 => ["PnP id \"%s\" requires ExcludeFromSelect. Vendors or INF floppy install users may ignore this message."],
    4014 => ["First field should be one of the HKR, HKU, HKLM, HKCR or HKCU keywords."],
    );
    my(%ValidLowerInterfaces) = ('NDIS5' => 1,'NDISATM' => 1,'NETBIOS' => 1, 'IPX' => 1, 'TDI' => 1,'NDIS5_ATALK' => 1,
                     'NDISCOWAN' => 1,'NDISATM' => 1,'NDIS5_DLC' => 1, 'NDIS5_IP' => 1, 'NDIS5_IPX' => 1,
                     'NDIS5_STREAMS' => 1, 'WINSOCK' => 1, 'NDIS5_NBF' => 1, 'NOLOWER' =>1, 'NDISWAN' => 1);

    my(%ValidUpperInterfaces) = ('NOUPPER' => 1,);

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
    print(STDERR "Module \"NETSERVICE\" Loaded\n");
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
        CheckModel($Item);
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
    shift;
    $inffile = shift;
    return(1);
}

# REQUIRED!                                                                                       #
sub PrintDetails {return(1); }
sub PrintHeader { return(1); }

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                    WORK SUBROUTINES                                           |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################

#-------------------------------------------------------------------------------------------------#
# CheckModel - Verifies an individual model                                                       #
#-- CheckModel($Model)  --------------------------------------------------------------------------#
sub CheckModel($Model) {
    my($Model) = $_[0];
    my($MillSuffix) = CheckVersionForME("VERSION");
    
    if ($MillSuffix !~ /^$/) {
        VerifyModel($Model,$MillSuffix)   if ( $inffile->sectionDefined($Model.$MillSuffix) );
    }
    

    if ( $inffile->sectionDefined($Model.".NTX86") ) {
        VerifyModel( $Model, ".NTX86" );
    } elsif ( $inffile->sectionDefined($Model.".NT") ) {
        VerifyModel( $Model, ".NT" );
    } elsif ( $inffile->sectionDefined($Model) ) {
        VerifyModel( $Model,"" );
    }
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

    print(STDERR "\tInvoking Display::ProcManu...\n") if ($DEBUG);

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

#-------------------------------------------------------------------------------------------------#
#-- ExpandModelSection($line, $Section) ----------------------------------------------------------#
sub ExpandModelSection {
    my $Section  = shift;
    my $line;
    my @lines    =  $inffile->getSection($Section);
    my $count    = 0;

    my(%Models)  =  ();
    my($TempVar);

    my($Directive, @Values, $Value);

    print(STDERR "\tInvoking Net::ExpandModelSection...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if it's a blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values             = ChkInf::GetList($Value,$inffile->getIndex($Section,$count));

        if (defined($Values[0])) {
            $Models{$Values[0]} = $Values[1];
        }
        $count++;
    }

    return(%Models);
}

#-------------------------------------------------------------------------------------------------#
#-- GetRegLines($Section) ------------------------------------------------------------------------#
sub GetRegLines {
    my $Section     = shift;
    my $line        = 0;
    my @lines       = $inffile->getSection($Section);
    my $curline;

    my(@AddReg);

    print(STDERR "\tBeginning GetRegLines (@_)...\n") if ($DEBUG);

    foreach $curline (@lines) {
        $line++, next if ($curline =~ /^\s*$/);# Skip if it's a blank line
        @AddReg = (@AddReg, $curline);
        $AllAddRegs{$line} = 1;
        $line++;
    }
    print(STDERR "\tExiting GetRegLines ...\n") if ($DEBUG);
    return(@AddReg);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyModel($line, $Model, $Extension) -------------------------------------------------------#
sub VerifyModel {
    print(STDERR "\tBeginning VerifyModel (@_)...\n") if ($DEBUG);

    my $Model      =  shift;
    my $Extension  =  shift;
    my $Section    =  $Model.$Extension;
       $Section    =~ s/\s*$//;
    my $curline;
    my @lines      =  $inffile->getSection($Section);
    my $line       =  0;
    my $Charac     =  FALSE;

    my(@AddRegSections,$addregsec);
    my($Directive, @Values, $Value);
    my($characteristics,$cpfiles,$lnum);
    
    %AddRegList = ();
    %AddServiceList = ();
    
    $WinsockService = "";
    %WinsockProviderID = (); 
    $NDIKeyPresent = FALSE;

    my(%NetDirectives) = (  'CHARACTERISTICS'   => " " );

    my(%SecToCheck) = ("ADDREG",    TRUE,
                       "COPYFILES", TRUE,
                       "NEEDS",     TRUE);
    my($CurrentLine, $Temp, @LineItems, $SizeOfArray);
    @AddRegs     =  ();

    foreach $curline (@lines) {
        $line++, next if ($curline =~ /^\s*$/);# Skip if blank line
    
        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$curline);
        @Values             = ChkInf::GetList($Value,$inffile->getIndex($Section,$line));

        if (uc $Directive eq "ADDREG") {
            foreach $Value (@Values) {
                if ( $inffile->sectionDefined($Value) ) {
                    @AddRegs = (@AddRegs, GetRegLines($Value));
                    push(@AddRegSections,$Value);
                }
            }

        }
        #check nic's specific directives
        if (uc $Directive eq "CHARACTERISTICS") {
            if (defined($Values[0])) {
                $characteristics = $Values[0];
                $characteristics =~ s/^\s*//;
                $characteristics =~ s/^0x//;
                $characteristics =~ s/^0X//;
                $characteristics =~ s/\s*$//;
                CheckCharacteristics($characteristics,$Section,$line);
                $Charac=TRUE;
            }
        }
        if (uc $Directive eq "COPYFILES") {
            if (defined($Values[0])) {
                $cpfiles = $Values[0];
                $cpfiles =~ s/^\s*//;
                $cpfiles =~ s/\s*$//;
                if ($cpfiles !~ /^@/ )
                {
                    CheckCopyFiles($inffile->getIndex($cpfiles)-1,$cpfiles);
                }
            }
        }
        if (uc $Directive eq "REGISTERDLLS") {
            foreach $Value (@Values) {
                CheckRegDLLSection($Value) if ( $inffile->sectionDefined($Value) );
            }
        }

        $line++;
    }
    AddNetCardError(3043,$inffile->getIndex($Section)-1) unless ($Charac);

    $characteristics = 0 unless (defined($characteristics));

    foreach $addregsec (@AddRegSections) {
        AddRegProc($addregsec,$characteristics);
    }

    CheckServiceName($Section.".SERVICES")                  if ( $inffile->sectionDefined($Section.".SERVICES") );
    CheckServiceRemoveSection($Section.".REMOVE.SERVICES")  if ( $inffile->sectionDefined($Section.".REMOVE.SERVICES") );
    CheckRemoveInstSection($Section.".REMOVE")              if ( $inffile->sectionDefined($Section.".REMOVE") );
   
    if (! $NDIKeyPresent) {
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
sub CheckCharacteristics {
my($char,$sec, $Line_num) = @_;
my($RHS);
my($x,$y,$z);

    $RHS= hex($char);

    if (($RHS&0x10)&&(($RHS&0x1)||($RHS&0x4)||($RHS&0x2))) {
        AddNetCardError(4003,$Line_num);
    }
    if (($RHS&0x1)||($RHS&0x2)||($RHS&0x4)||($RHS&0x40)) {
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
    return(TRUE);}

#-- ($Section) ------------------------------------------------------#
sub AddRegProc {
    my $Section     = shift;
    my $ModelChar   = shift;
    my $line        = 0;
    my @lines       = $inffile->getSection($Section);
    my $curline;

    my(@LineItems,$CurrentLine);
    my($Temp,$SizeOfArray,$TempVar);
    my($ParamsDefined) = 0;
    my($ParamVerify) = hex($ModelChar)&0x80;
    my($UpperPresent) = 0;
    my($LowerPresent) = 0;
    my(@CoServices,@ExServices);
    my($ServicePresent) = 0;
    my($CoServicesPresent) = 0;
    my($NDIAddreg)= 0;
    
    @CoServices = ();
    @ExServices = ();

    foreach $curline (@lines) {
        $line++, next if ($curline =~ /^\s*$/);# Skip if blank line

        $CurrentLine = $curline;

        @LineItems = (); # Clear the array
        @LineItems = ChkInf::GetList($CurrentLine,$inffile->getIndex($Section,$line));

        $SizeOfArray = $#LineItems;
        for ($Temp=0; $Temp <= $SizeOfArray; $Temp++) {
            if (defined($LineItems[$Temp])) {
                $LineItems[$Temp] = uc($LineItems[$Temp]);
            } else {
                $LineItems[$Temp] = " ";
            }
        }

        # Now that we have a clean, standardized line, verify it
        if (defined($LineItems[3])) {
            if (! defined($LineItems[4])) {
                my($keytype,$temp);
                $keytype = $LineItems[3];
                
                if (!(($keytype =~ /00000010/) || ($keytype =~ /0x10/i))) {
                    AddNetCardError(4033,$inffile->getIndex($Section,$line),"5");
                    $line++;
                    next;
                }
            }
        }

        if ($LineItems[1] =~ /^NDI/i) {
            $NDIKeyPresent = TRUE;
            $NDIAddreg = 1;
            if ($LineItems[2] =~ /^SERVICE$/i) {
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

            if ($LineItems[1] =~ /INTERFACES/i) {
                my(@tmp3,$cnt,$i,$str); #set of interfaces can be defined..
                $str  = $LineItems[4];
                @tmp3 = split(',',$str);
                $cnt  = $#tmp3+1;

                if (!(defined($LineItems[4]))) { $cnt=0; } 
                for ($i=0; $i < $cnt; $i++) {
#                    $tmp3[$i] =~ s/\"//g;
	                while ($tmp3[$i] =~ /^\"/)
                    {
                        $tmp3[$i] =~ s/^\"//;
                    }
	                while ($tmp3[$i] =~ /\"$/)
                    {
                        $tmp3[$i] =~ s/\"$//;
                    }
                    if ($LineItems[2] =~ /UPPERRANGE/i) {
                        $UpperPresent = 1;
                        AddNetCardError(4005,$inffile->getIndex($Section,$line),$tmp3[$i])
                            if (!defined($ValidUpperInterfaces{uc $tmp3[$i]}));
                    }
                    if ($LineItems[2] =~ /LOWERRANGE/i) {
                        $LowerPresent = 1;
                        AddNetCardError(4005,$inffile->getIndex($Section,$line),$tmp3[$i])
                            if (!defined($ValidLowerInterfaces{uc $tmp3[$i]}));
                    }
                }
            }

            if (($LineItems[2] =~ /COSERVICES/i)||($LineItems[2] =~ /EXCLUDESETUPSTARTSERVICES/i)) {
                my($TempVar);
                $TempVar = $LineItems[3];
                if (($TempVar !~ /0x00010000/i) && ($TempVar !~ /0x10000/i)) {
                    AddNetCardError(4010,$inffile->getIndex($Section,$line));
                }
            }

            if ($LineItems[2] =~ /COSERVICES/i) {
                # Make a list of all the services found in coservices
                # Also keep a flag if this entry is present 
                $CoServicesPresent = 1;
                my($cnt,$i,$coserv);
                $cnt = $#LineItems + 1;
                for ($i = 4; $i < $cnt; $i++) {
                    $coserv = $LineItems[$i];
                    push(@CoServices,$coserv);
                }       
            }
            if ($LineItems[2] =~ /EXCLUDESETUPSTARTSERVICES/i) {
                # Make a list of all the services found in excludesetupstartservices
                # Later check against the services found in coservices.
                my($cnt,$i,$exser);
                $cnt = $#LineItems + 1;
                for ($i = 4; $i < $cnt; $i++) {
                    $exser = $LineItems[$i];
                    push(@ExServices,$exser);
                }       
            }

            if ($LineItems[2] =~ /HELPTEXT/i) { }

            if ($LineItems[2] =~ /CLSID/i)    {
                my($classid);
                if ( defined $LineItems[4] ) {
                    $classid = $LineItems[4];
                    $classid =~ s/\"//g;
                } else {
                    $classid = "NULL";
                }
                if ($classid !~ /^\{[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}\}$/) {
                    AddNetCardError(4102,$inffile->getIndex($Section,$line) );
                }
            }

        } #ndi-if ends.
        $line++;
    }
    if ((($UpperPresent == 0)||($LowerPresent == 0))&&($NDIAddreg == 1)) {
        AddNetCardError(4120,$inffile->getIndex($Section)-1);
    }
    if ($ServicePresent == 1) {
        if ($CoServicesPresent == 0) {
            # may want to eventually add checks here
        } else {
            # Check if the exservices are present in coservices list
            my($exser);
            foreach $exser (@ExServices) {
                my($present) = 0;
                my($coserv);
                foreach $coserv (@CoServices) {
                    if (uc $exser eq uc $coserv) {
                        $present = 1;
                    }
                }
                if ($present == 0) {
                    AddNetCardError(4101,$inffile->getIndex($Section)-1,$exser);
                }
            }
        }
    }
    return(TRUE);
}

#-----------------------CheckServiceName($Section)-----------------------
sub CheckServiceName {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $curline;

    my ($Directive,@Values,$Value);
    my ($ucAddReg,$ucAddService);

    foreach $curline (@lines) {
        $line++, next if ($curline =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$curline);
        @Values    = ChkInf::GetList($Value,$inffile->getIndex($Section,$line));

        if ($Directive =~ /ADDSERVICE/i) {
            $ucAddService = uc $Values[0];
            if (defined($Values[0])) {
                $AddServiceList{$ucAddService} = 1;
            }
        }

        $line++;
    }

    my($addregser,$addserv);
    my($servpresent) = 0;    
    foreach $addserv (keys %AddServiceList) {
        foreach $addregser (keys %AddRegList) {
            if ($addserv eq $addregser) {
                $servpresent = 1;
            }
        }
    }
    if ($servpresent == 0) {
       AddNetCardError(4011,$inffile->getIndex($Section));
    }
    return(TRUE);
}

#-----------------------CheckCompatibleKey--------------------------------
sub CheckCompatibleKey {
    my $Section  =  "Version";
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $curline;

    my ($Directive,$Value);
    my ($Signature,$CompVal,$CompPresent,$CompLine);

    $CompPresent = 0;
    $CompLine = $line;
    foreach $curline (@lines) {
        $line++, next if ($curline =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$curline);

        if ($Directive =~ /SIGNATURE/i) {
            $Signature = $Value;
        }
        if ($Directive =~ /COMPATIBLE/i) {
            $CompVal = $Value;
            $CompPresent = 1;
            $CompLine = $line;
        }
        $line++
    }

    if ($Signature =~ /CHICAGO/i) {
        if ($CompPresent == 1) {
            if ($CompVal != 1) {
                AddNetCardError(3046,$inffile->getIndex($Section,$CompLine));
            }
        } else {
            AddNetCardError(3045,$inffile->getIndex($Section,$CompLine));
        }
    }
    return(TRUE);
}

#-----------------------CheckServiceRemoveSection($Section)-----------------------
sub CheckServiceRemoveSection {
    my $Section =  shift;
    my $line    =  0;
    my @lines   =  $inffile->getSection($Section);
    my $curline;

    my $DelServicePresent = 0;
    my ($Directive,$Value,@Values);
    
    ChkInf::Mark(uc $Section);
    foreach $curline (@lines) {
        $line++, next if ($curline =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$curline);
        @Values    = ChkInf::GetList($Value,$inffile->getIndex($Section,$line));

        if ($Directive =~ /DELSERVICE/i) {
            $DelServicePresent = 1;
            if (!(defined($AddServiceList{uc $Value}))) {
                AddNetCardError(4116,$inffile->getIndex($Section,$line));
            }
        }
        if ($Directive =~ /DELREG/i) {
            foreach $Value (@Values) {
                if ( $inffile->sectionDefined($Value) ) {
                    AddRegProc($Value,0);
                    ChkInf::Mark(uc $Value);
                } 
            }
        }
        $line++;
    }

    if ($DelServicePresent == 0) {
        AddNetCardError(4117,$inffile->getIndex($Section)-1);
    }
    return(TRUE);
}

#-----------------------CheckRemoveInstSection($Section)-----------------------
sub CheckRemoveInstSection {
    my $Section  =  shift;
    my $line     =  0;
    my @lines    =  $inffile->getSection($Section);
    my $curline;

    my($Directive,$Value,@Values);
    
    ChkInf::Mark(uc($Section));

    foreach $curline (@lines) {
        $line++, next if ($curline =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$line,$curline);
        @Values    = ChkInf::GetList($Value,$inffile->getIndex($Section,$line));

        if ($Directive =~ /DELREG/i) {
            my($addregsec);
            foreach $Value (@Values) {
                if ( $inffile->sectionDefined($Value) ) {
                    AddRegProc($Value,0);
                    ChkInf::Mark(uc $addregsec);
                } 
            }
        } elsif (uc $Directive eq "UNREGISTERDLLS") {
            foreach $Value (@Values) {
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
    my $Section =  shift;
    my $line    =  0;
    my @lines   =  $inffile->getSection($Section);
    my $curline;
    my @Values;
    
    ChkInf::Mark(uc($Section));
    foreach $curline (@lines) {
        $line++, next if ($curline =~ /^\s*$/);# Skip if blank line
        @Values     = split(/,/, $curline);
        if (($Values[0] =~ /^$/)||($Values[2] =~ /^$/)||($Values[3] =~ /^$/)) {
            AddNetCardError(4208,$inffile->getIndex($Section,$line) );
        }
        $line++;
    }
    return(TRUE);
}
