# Copyright 1999-2000 Microsoft Corporation

package NET;                     # The name of this module. For our purposes,
                                 #   must be all in caps.

    use strict;                  # Keep this code clean
    no strict 'refs';            # we use pointers to functions
    my($Version) = "5.1.2402.0"; # Version of this file
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


    my(%NetCardErrorTable) = (
    3043 => ["Keyword Characteristics not defined in install section"],
    3044 => ["Keyword BusType not defined in install section"],
    3045 => ["Keyword Compatible not defined in Version section"],
    3046 => ["Keyword Compatible should be equal to 1 if Signature is \$CHICAGO\$"],
    4001 => ["BusType does not match the NetCard model."],
    4002 => ["Characteristics can have only one of NCF_VIRTUAL,NCF_PHYSICAL & NCF_WRAPPER."],
    4003 => ["NCF_NO_SERVICE cannot be used with NCF_VIRTUAL or NCF_PHYSICAL."],
    4005 => ["Invalid interface \"%s\" used in Addreg section"],
    4006 => ["Lower interface cannot have an install section"],
    4007 => ["Reserved Word used as Parameter Key"],
    4008 => ["Invalid Value-Name for Parameter Key"],
    4009 => ["NCF_HAS_UI should be one of the characteristics for advanced parameters"],
    4011 => ["The AddService ServiceName parameter must match the NDI\\Service entry in the Registry."],
    4030 => ["Section [%s] is empty. It should contain \"*PNP8132\"."],
    4031 => ["String \"*PNP8132\" is missing for section [%s]."],
    4032 => ["Wrong strings in section [%s]. Only \"*PNP8132\" is allowed."],
    4033 => ["Incorrect number of parameters, expected %s."],

    4100 => ["Invalid Priority Value for ConfigPriority"],
    4101 => ["Invalid Config Type for ConfigPriority"],
    4102 => ["Invalid IRQ value for IRQconfig"],
    4103 => ["The hexadecimal integer value exceeds range"],
    4104 => ["IRQConfig not assigned according to DDK specification"],
    4105 => ["IOConfig not assigned according to DDK specification"],
    4106 => ["Invalid DMA value for DMAconfig"],
    4107 => ["DMAConfig not assigned according to DDK specification"],
    4108 => ["MEMConfig not assigned according to DDK specification"],

    4200 => ["Invalid value for ServiceType in Services Install Section"],
    4201 => ["Invalid value for StartType in Services Install Section"],
    4202 => ["Invalid value for ErrorControl in Services Install Section"],
    4203 => ["ServiceBinary value not defined in Services Install Section"],
    4204 => ["Invalid value for LoadOrderGroup in Services Install Section"],
    4205 => ["Invalid value for StartName in Services Install Section"],
    4206 => ["Services section not defined for \"%s\" model"],
    4207 => ["\"%s\" file not allowed in CopyFiles section"],
    4208 => ["Period required as prefix for MillenniumPreferred Directive in Version Section"],
    );

    my(%NetCardWarningTable) = (
    5001 => ["PnP id \"%s\" requires ExcludeFromSelect. Vendors or INF floppy install users may ignore this message."],
    4014 => ["First field should be one of the HKR, HKU, HKLM, HKCR or HKCU keywords."],
    5002 => ["Usage of LogConfigOverride Sections should be avoided for NetCards"],
    );

    my(%ValidInterfaces) = ('ETHERNET' => 1, 'ATM' => 1, 'TOKENRING' => 1, 'SERIAL' => 1, 'FDDI' => 1,
                    'BASEBAND' => 1, 'BROADBAND' => 1,'ARCNET' => 1, 'ISDN' => 1, 'NOLOWER' => 1,
                    'NDIS3' => 1,'NDIS4' => 1,'NDIS5' => 1,'NDISATM' => 1,'NETBIOS' => 1, 'IPX' => 1, 'TDI' => 1,
                    'STREAMS' => 1, 'WINSOCK' => 1, 'WINNET5' => 1, 'NOUPPER' =>1, 'NDISWAN' => 1, 'NDIS1394' => 1);

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
#-- DCDirective ----------------------------------------------------------------------------------#
sub DCDirective {
    my($Directive) = $_[1];

    my(%ValidDirectives) = ( "CHARACTERISTICS"  => TRUE,
                             "ADAPTERMASK"      => TRUE,
                             "EISACOMPRESSEDID" => TRUE,
                             "BUSTYPE"          => TRUE,
                             "MILLENNIUMPREFERRED"          => TRUE,
                             "MAXINSTANCE"      => TRUE,
                             "COMPATIBLE"       => TRUE);
    return(FALSE) unless (defined($ValidDirectives{uc $Directive}));
    return(TRUE);
}


#-------------------------------------------------------------------------------------------------#
# Required subroutine! Allows us to verify that the module loaded correctly.                      #
#-- Exists ---------------------------------------------------------------------------------------#
sub Exists {
    print(STDERR "Module \"NET\" Loaded\n");
    return(TRUE);
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
        $$Item[0] = $_;
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
    my $Model  = $_[0];
    my($MillSuffix) = CheckVersionForME("VERSION");


    if ($MillSuffix !~ /^$/) {
        VerifyModel($Model.$MillSuffix)   if ( $inffile->sectionDefined($Model.$MillSuffix) );
    }
    

    VerifyModel($Model.".NTX86")   if ( $inffile->sectionDefined($Model.".NTX86") );
    VerifyModel($Model.".NT.COINSTALLERS")   if ( $inffile->sectionDefined($Model.".NT.COINSTALLERS") );
    VerifyModel($Model.".NTALPHA") if ( $inffile->sectionDefined($Model.".NTALPHA") );
    VerifyModel($Model.".NT")      if ( $inffile->sectionDefined($Model.".NT") );
    VerifyModel($Model)            if ( $inffile->sectionDefined($Model) && !
                                        ( $inffile->sectionDefined("${Model}.NT") ||
                                          $inffile->sectionDefined("${Model}.NTX86") ||
                                          $inffile->sectionDefined("${Model}.NTIA64")));


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
    my @lines       = $inffile->getSection($Section);
    my $count       = 0;
    my $line;
    my @AddReg;

    print(STDERR "\tBeginning GetRegLines (@_)...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);

        @AddReg = (@AddReg, $line);
        $AllAddRegs{ $inffile->getIndex($Section,$count) } = 1;
        $count++;
    }
    return(@AddReg);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyModel($line, $Model, $Extension) -------------------------------------------------------#
sub VerifyModel {
    my $Section     = $_[0];
    my $Model       = $_[1];

    my @lines       = $inffile->getSection($Section);
    my $line;
    my $count       = 0;

    my($Directive, @Values, $Value);
    my($characteristics,$bustype,$cpfiles,$lnum);
    my($Charac,$Btype) = (0,0);
    my(@AddRegSections,$addregsec);
    my($charlnum);
    my($MillSuffix) = CheckVersionForME("VERSION");

 
    $charlnum = 0;

    my(%NetDirectives) = (  'CHARACTERISTICS'   => " ",
                            'BUSTYPE'           => " ");

    my(%SecToCheck) = ("ADDREG",    TRUE,
                       "COPYFILES", TRUE,
                       "NEEDS",     TRUE);
    my($CurrentLine, $Temp, @LineItems, $SizeOfArray);
    @AddRegs     =  ();


    if ( $Section !~ /COINSTALLERS/)
    {
	if (($MillSuffix !~ /^$/) && ($Section != /$MillSuffix$/))
	{

            if ( ! $inffile->sectionDefined($Section . ".SERVICES") ) {
                AddNetCardError(4206,$inffile->getIndex($Section),$Section);
            }
	}	
    }

    print(STDERR "\tBeginning VerifyModel (@_)...\n") if ($DEBUG);
    foreach $line (@lines) {

        $count++, next if ($line =~ /^\s*$/);# Skip if blank line


        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values             = ChkInf::GetList($Value,$inffile->getIndex($Section,$count));

        if (uc($Directive) eq "ADDREG") {

            foreach $Value (@Values) {
                $Value =~ s/^\s*//;
                $Value =~ s/\s*$//;
                if ( $inffile->sectionDefined($Value) ) {
                    @AddRegs = (@AddRegs, GetRegLines($Value));
                    push(@AddRegSections,$Value);
                }
            }

        }
        #check nic's specific directives
        if (uc($Directive) eq "CHARACTERISTICS") {
            if (defined($Values[0]))
            {
                $characteristics = $Values[0];
                $characteristics =~ s/^\s*//;
                $characteristics =~ s/^0x//;
                $characteristics =~ s/^0X//;
                $characteristics =~ s/\s*$//;
                CheckCharacteristics($characteristics,$Section,$inffile->getIndex($Section,$count));
                $Charac++;
                $charlnum = $inffile->getIndex($Section,$count);
            }
        }
        if (uc($Directive) eq "BUSTYPE") {
            if (defined($Values[0])) {
                $bustype = $Values[0];
                $bustype =~ s/^\s*//;
                $bustype =~ s/\s*$//;
                CheckBusType($bustype,$Section,$inffile->getIndex($Section,$count));
                $Btype++;
            }
        }
        if (uc($Directive) eq "COPYFILES") {  }

        $count++;
    }
    if ( $Section !~ /COINSTALLERS/)
    {
		if ($MillSuffix =~ /^$/)
        { AddNetCardError(3043,$inffile->getIndex($Section)) unless ($Charac); }
    
    if ($Btype == 0)
    {
        if (defined($characteristics) && ($characteristics =~ /4$/)) # Bustype needed for physical adapter
        {
            AddNetCardError(3044,$inffile->getIndex($Section));
        }
    }
    }


    $characteristics = 0 unless (defined($characteristics));
    foreach $addregsec (@AddRegSections) {
        AddRegProc($addregsec,$characteristics,$charlnum);
    }

    CheckServiceName($Section.".SERVICES") if ( $inffile->sectionDefined($Section.".Services") );

    if ( $inffile->sectionDefined($Section.".LogConfigOverride") )
    {
        AddNetCardWarning(5002,$inffile->getIndex($Section.".LogConfigOverride"));
    }



    @AddRegs = ();
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

    $x=$RHS&0x1;
    $y=($RHS&0x2)>>1;
    $z=($RHS&0x4)>>2;

    if ($x+$y+$z!=1)
    {
        AddNetCardError(4002,$Line_num);
    }
    if (($RHS&0x10)&&(($RHS&0x1)||($RHS&0x4)||($RHS&0x2)))
    {
        AddNetCardError(4003,$Line_num);
    }

}

#-------------------------------------------------------------
#
#   Checks if the install section has valid bustype
#
# Usage:
#   CheckBusType(bus_type,expected_section_name,line_num)
#
#-------------------------------------------------------------
sub CheckBusType {
    my($RHS,$sec, $Line_num) = @_;
    my($Item);
    my($Section,$Extn,$HwId);
    my(@Values,$Value);

    $sec = uc $sec;
    $Value = $sec;
    $Value =~ s/\.NT$//;
    $Value =~ s/\.NTX86$//;
    $Value =~ s/\.NTALPHA$//;
    foreach (keys(%AllModels)) {
        $Item = uc($_);
        $$Item[0] = $_;
        if ($Value eq $Item) {
            $HwId =  $AllModels{$_};
            if (!defined($HwId)) {
                # This error is flagged by Chkinf.pm
                next;
            }
            if ($HwId =~ /ISAPNP/)
            {
                if ($RHS !~ /^14$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
            elsif (($HwId =~ /PCMCIA/)||($HwId =~ /MF/))
            {
                if ($RHS !~ /^8$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
            elsif ($HwId =~ /USB/)
            {
                if ($RHS !~ /^15$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
            elsif ($HwId =~ /PCI/)
            {
                if ($RHS !~ /^5$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
            elsif (($HwId =~ /EISA/)||($HwId =~ /^\*/))
            {
                if ($RHS !~ /^2$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
            else
            {
                if ($RHS !~ /^1$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
        }
        # Check the model
    }

}
#-- CheckCopyFiles($line, $Section) ------------------------------------------------------#
sub CheckCopyFiles {
    my $Section  = shift;
    my @lines    = $inffile->getSection($Section);
    my $count    = 0;
    my $line;
    my($Destination, $Source, $Temp, $Flags);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/); # Skip if blank line

        ($Destination, $Source, $Temp, $Flags) = split(/,/,$line);

        $Destination =~ s/^\s*//;
        $Destination =~ s/\s*$//;

        $count++;
    }
    return(TRUE);
}
#-- AddRegProc($Section) ------------------------------------------------------#
sub AddRegProc {

    my($Section,$ModelChar,$CharLine)    =  @_ ;
    my($line);
    my(@lines)   =  $inffile->getSection($Section);
    my $count    =  0;



    my(@LineItems,$CurrentLine);
    my($Temp,$SizeOfArray,$TempVar);
    my($ParamsDefined) = 0;
    my($ParamVerify) = hex($ModelChar)&0x80;

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if blank line

        $CurrentLine = $line;

        @LineItems = (); # Clear the array
        @LineItems = ChkInf::GetList($CurrentLine,$inffile->getIndex($Section,$count));

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
        if (! defined($LineItems[4])) {
            AddNetCardError(4033,$inffile->getIndex($Section,$count),"5");
            $count++;
            next;
        }

        if ($LineItems[1] =~ /^NDI/i)
        {
        if ($LineItems[2] =~ /SERVICE/i)
        {
            if ( defined $LineItems[4])
            {
                my($ucAdd) = uc $LineItems[4];
                while ($ucAdd =~ /^\"/)
                {
                    $ucAdd =~ s/^\"//;
                    $ucAdd =~ s/\"$//;
                }
                $AddRegList{'SERVICE'} = $ucAdd;
            }
        }
            if ($LineItems[1] =~ /INTERFACES/i)
            {

                my(@tmp3,$cnt,$i,$str); #set of interfaces can be defined..
                $str = uc $LineItems[4];

                @tmp3 = split(',',$str);

                $cnt = $#tmp3 + 1;

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
                    AddNetCardError(4005,$inffile->getIndex($Section,$count), $tmp3[$i])
                        if (!defined($ValidInterfaces{uc $tmp3[$i]}));
                }
            }

            if ($LineItems[1] =~ /PARAMS/i)
            {
                my(@tmp3,$val);
                @tmp3 = split(/\\/,$LineItems[1]);
                $ParamsDefined = 1;
                $val = hex($ModelChar)&0x80;
                AddNetCardError(4007,$inffile->getIndex($Section,$count))
                    if (defined($ReservedWords{uc $tmp3[2]}));
                if ($LineItems[1] !~ /ENUM/i)
                {
                    AddNetCardError(4008,$inffile->getIndex($Section,$count))
                    if (!defined($ParamKeys{uc $LineItems[2]}));
                }
            }
        }   #ndi-if ends.

        $count++;
    }
    if ($ParamsDefined == 1)
    {
#        AddNetCardError(4009,$inffile->getIndex($Section)) if ($ParamVerify == 0);
        AddNetCardError(4009,$CharLine) if ($ParamVerify == 0);
    }
    return(TRUE);
}
#-----------------------CheckServiceName($Section)-----------------------
sub CheckServiceName {
    my $Section  = shift;
    my $line;
    my @lines    =  $inffile->getSection($Section);
    my $count    = 0;
    my $checkval; # if checkval is non-zero, then we check the associated service name

    my($Directive,$Value,@Values);
    my($ucAddReg,$ucAddService);


    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values             = ChkInf::GetList($Value,$inffile->getIndex($Section,$count));

        $ucAddReg = $AddRegList{'SERVICE'};
        $ucAddReg = uc($ucAddReg); # Ugly statement because some versions of Perl complained! =-(
        $ucAddService = uc $Values[0];
		$ucAddReg     =~ s/"//g;
		$ucAddService =~ s/"//g;
		
		$checkval = hex($Values[1]);
		
		if ($checkval != 0)
		{
            if ((!defined($Values[0])) or (!defined($AddRegList{'SERVICE'})) or ($ucAddService ne $ucAddReg)) {
                AddNetCardError(4011,$inffile->getIndex($Section,$count) );
            }
        }

        $count++;
    }
    return(TRUE);
}
#-----------------------CheckCompatibleKey--------------------------------
sub CheckCompatibleKey {
    my $Section  = "Version";
    my $line;
    my @lines    =  $inffile->getSection($Section);
    my $count    =  0;
    my($Directive,$Value);
    my($Signature,$CompVal,$CompPresent,$CompLine);

    $CompPresent = 0;

    $CompLine = $count;
    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if blank line

        ($Directive, $Value) = ChkInf::SplitLine($Section,$count,$line);

        if ($Directive =~ /SIGNATURE/i) {
            $Signature = $Value;
        }
        if ($Directive =~ /COMPATIBLE/i) {
            $CompVal = $Value;
            $CompPresent = 1;
            $CompLine = $line;
        }

        $count++
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



