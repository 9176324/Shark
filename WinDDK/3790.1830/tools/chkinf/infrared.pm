# Copyright 1999-2000 Microsoft Corporation

package INFRARED;                # The name of this module. For our purposes,
                                 #   must be all in caps.

    use strict;                  # Keep this code clean
    no strict 'refs';            # no strict references

    my $Version = "5.1.2402.0";  # Version of this file
    my $DEBUG   = 0;             # Set to 1 for debugging output

    sub FALSE { return(0); } # Utility subs
    sub TRUE  { return(1); }

    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my @AddRegs;         # Array holds AddRegs for current model
    my $CurrentINF;      # Holds current INF object
    my $inffile;         # Holds name of INF currently being processed.

    my %AllAddRegs;      # Array holds all AddReg lines, index by INF Line Number
    my %AllModels;       # Array of all models present in INF (Model) = (Type)
    my %AddRegList;


    my(%NetCardErrorTable) = (
    3043 => ["Keyword Characteristics not defined in install section"],
    3044 => ["Keyword BusType not defined in install section"],
    3045 => ["Keyword Compatible not defined in Version section"],
    3046 => ["Keyword Compatible should be equal to 1 if Signature is \$CHICAGO\$"],
    4001 => ["BusType does not match the NetCard model."],
    4002 => ["Characteristics can have only one of NCF_VIRTUAL,NCF_PHYSICAL & NCF_WRAPPER."],
    4003 => ["NCF_NO_SERVICE cannot be used with NCF_VIRTUAL or NCF_PHYSICAL."],
    4005 => ["Invalid interface used in Addreg section"],
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
    );

    my(%NetCardWarningTable) = (
    5001 => ["PnP id \"%s\" requires ExcludeFromSelect. Vendors or INF floppy install users may ignore this message."],
    4014 => ["First field should be one of the HKR, HKU, HKLM, HKCR or HKCU keywords."],
    );

    my(%ValidInterfaces) = ('ETHERNET' => 1, 'ATM' => 1, 'TOKENRING' => 1, 'SERIAL' => 1, 'FDDI' => 1,
                    'BASEBAND' => 1, 'BROADBAND' => 1,'ARCNET' => 1, 'ISDN' => 1, 'NOLOWER' => 1,
                    'NDIS5' => 1,'NDISATM' => 1,'NETBIOS' => 1, 'IPX' => 1, 'TDI' => 1,
                    'STREAMS' => 1, 'WINSOCK' => 1, 'WINNET5' => 1, 'NOUPPER' =>1, 'NDISWAN' => 1);

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
                             "COMPATIBLE"       => TRUE);
    return(FALSE) unless (defined($ValidDirectives{$Directive}));
    return(TRUE);
}


#-------------------------------------------------------------------------------------------------#
# Required subroutine! Allows us to verify that the module loaded correctly.                      #
#-- Exists ---------------------------------------------------------------------------------------#
sub Exists {
    print(STDERR "Module \"INFRARED\" Loaded\n");
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
    foreach ( keys(%AllModels) ) {
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
    # First parameter is the INF object
    $CurrentINF=$_[1];
    %AddRegList = ();
    $inffile = $CurrentINF->{inffile};
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

    VerifyModel($Model.".NTX86")   if ( $CurrentINF->sectionDefined($Model.".NTX86",$Model) );
    VerifyModel($Model.".NTALPHA") if ( $CurrentINF->sectionDefined($Model.".NTALPHA",$Model) );
    VerifyModel($Model.".NT")      if ( $CurrentINF->sectionDefined($Model.".NT",$Model) );
    VerifyModel($Model)            if ( $CurrentINF->sectionDefined($Model,$Model) );

    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# LearnModels - Returns an array of all models listed in the INF                                  #
#-- LearnModels() --------------------------------------------------------------------------------#
sub LearnModels {
    return() if (! $CurrentINF->sectionDefined("Manufacturer") );
    my $Section = "Manufacturer";
    my $line;
    my @lines   = $CurrentINF->getSection("Manufacturer");
    my $count   = 0;

    my($Directive, $Value);
    my(%Models);

    print(STDERR "\tInvoking Infrared::ProcManu...\n") if ($DEBUG);

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

        if ( $CurrentINF->sectionDefined($Value) ) {
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

#-------------------------------------------------------------------------------------------------#
#-- ExpandModelSection($line, $Section) ----------------------------------------------------------#
sub ExpandModelSection {
    my $Section  = shift;
    my $line;
    my @lines    =  $CurrentINF->getSection($Section);
    my $count    = 0;

    my(%Models)  =  ();
    my($TempVar);

    my($Directive, @Values, $Value);

    print(STDERR "\tInvoking Infrared::ExpandModelSection...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if it's a blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values             = ChkInf::GetList($Value,$CurrentINF->getIndex($Section,$count));

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
    my @lines       = $CurrentINF->getSection($Section);
    my $count       = 0;
    my $line;
    my @AddReg;

    print(STDERR "\tBeginning GetRegLines (@_)...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);

        @AddReg = (@AddReg, $line);
        $AllAddRegs{ $CurrentINF->getIndex($Section,$count) } = 1;
        $count++;
    }
    return(@AddReg);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyModel($line, $Model, $Extension) -------------------------------------------------------#
sub VerifyModel {
    my $Section = shift;
    my @lines   = $CurrentINF->getSection($Section);
    my $count   = 0;
    my $line;
    my $model   = shift;

    my($Directive, @Values, $Value);
    my($characteristics,$bustype,$cpfiles,$lnum);
    my($Charac,$Btype) = (0,0);
    my(@AddRegSections,$addregsec);

    my(%NetDirectives) = (  'CHARACTERISTICS'   => " ",
                            'BUSTYPE'           => " ");

    my(%SecToCheck) = ("ADDREG",    TRUE,
                       "COPYFILES", TRUE,
                       "NEEDS",     TRUE);
    my($CurrentLine, $Temp, @LineItems, $SizeOfArray);
    @AddRegs     =  ();

    if (! $CurrentINF->sectionDefined($Section . ".SERVICES") ) {
        AddNetCardError(4206,$CurrentINF->getIndex($Section),$Section);
    }

    print(STDERR "\tBeginning VerifyModel (@_)...\n") if ($DEBUG);
    foreach $line (@lines) {

        $count++, next if ($line =~ /^\s*$/);

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values             = ChkInf::GetList($Value,$CurrentINF->getIndex($Section,$count));

        if (uc($Directive) eq "ADDREG") {
            foreach $Value (@Values) {
                if ( $CurrentINF->sectionDefined($Value) ) {
                    @AddRegs = (@AddRegs, GetRegLines($Value));
                    push(@AddRegSections,$Value);
                }
            }

        }
        #check nic's specific directives
        if (uc($Directive) eq "CHARACTERISTICS") {
            if (defined($Values[0])) {
                $characteristics = $Values[0];
                $characteristics =~ s/^0x//;
                $characteristics =~ s/^0X//;
                CheckCharacteristics($characteristics,$Section,$CurrentINF->getIndex($Section,$count));
                $Charac++;
            }
        } elsif (uc($Directive) eq "BUSTYPE") {
            if (defined($Values[0])) {
                $bustype = $Values[0];
                CheckBusType($bustype,$Section,$CurrentINF->getIndex($Section,$count) );
                $Btype++;
            }
        } elsif (uc($Directive) eq "COPYFILES") {
            foreach $cpfiles (@Values) {
                if ($cpfiles !~ /^@/ ) {
                    CheckCopyFiles($cpfiles) if ($CurrentINF->sectionDefined($cpfiles) );
                }
            }
        }

        $count++;
    }
    AddNetCardError(3043,$CurrentINF->getIndex($Section) ) unless ($Charac);

    if ($Btype == 0) {
        if (defined($characteristics) && ($characteristics =~ /4$/)) {# Bustype needed for physical adapter
            AddNetCardError(3044,$CurrentINF->getIndex($Section) );
        }
    }


    $characteristics = 0 unless (defined($characteristics));
    foreach $addregsec (@AddRegSections) {
        AddRegProc($addregsec,$characteristics);
    }

    CheckServiceName($Section.".SERVICES") if ( $CurrentINF->sectionDefined($Section.".SERVICES") );

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

    if ($x+$y+$z!=1) {
        AddNetCardError(4002,$Line_num);
    }

    if (($RHS&0x10)&&(($RHS&0x1)||($RHS&0x4)||($RHS&0x2))) {
        AddNetCardError(4003,$Line_num);
    }

    return(TRUE);
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
    foreach ( keys(%AllModels) ) {
        $Item = uc($_);
        $$Item[0] = $_;
        if ($Value eq $Item) {
            $HwId =  $AllModels{$_};
            if (!defined($HwId)) {
                next;
            }
            $HwId =~ s/^\s*//; # Remove leading whitespace
            $HwId =~ s/\s*$//; # Remove trailing whitespace

            if ($HwId =~ /ISAPNP/) {
                if ($RHS !~ /^14$/) {
                    AddNetCardError(4001,$Line_num);
                }
            } elsif ($HwId =~ /PCMCIA/) {
                if ($RHS !~ /^8$/) {
                    AddNetCardError(4001,$Line_num);
                }
            } elsif ($HwId =~ /PCI/) {
                if ($RHS !~ /^5$/) {
                    AddNetCardError(4001,$Line_num);
                }
            } elsif ($HwId =~ /EISA/) {
                if ($RHS !~ /^2$/) {
                    AddNetCardError(4001,$Line_num);
                }
            } elsif ($HwId =~ /^\*/) {
                if ($RHS !~ /^14$/) {
                    AddNetCardError(4001,$Line_num);
                }
            } elsif ($HwId =~ /USB/) {
                if ($RHS !~ /^15$/) {
                    AddNetCardError(4001,$Line_num);
                }
            } else {
                if ($RHS !~ /^1$/) {
                    AddNetCardError(4001,$Line_num);
                }
            }
        }
    }

    return(TRUE);
}
#-- CheckCopyFiles($line, $Section) ------------------------------------------------------#
sub CheckCopyFiles {
    my $Section   = shift;
    my @lines     = $CurrentINF->getSection($Section);
    my $count     = 0;
    my $line;


    my($Destination, $Source, $Temp, $Flags);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/); # Skip if blank line

        ($Destination, $Source, $Temp, $Flags) = ChkInf::GetList($line,$CurrentINF->getIndex($Section,$count));

        if (!(($Destination =~ /sys$/i) || ($Destination =~ /dll$/i) || ($Destination =~ /hlp$/i )))
        {
            AddNetCardError(4207,$CurrentINF->getIndex($Section,$count),$Destination);
        }

        $count++;
    }
    return(TRUE);
}
#-- AddRegProc($Section) ------------------------------------------------------#
sub AddRegProc {

    my $Section       = shift;
    my $ModelChar     = shift;
    my @lines         =  $CurrentINF->getSection($Section);
    my $count         = 0;
    my $ParamVerify   = hex($ModelChar)&0x80;
    my $ParamsDefined = 0;

    my $line;
    my(@LineItems,$CurrentLine);
    my($Temp,$SizeOfArray,$TempVar);


    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if blank line

        $CurrentLine = $line;

        @LineItems = (); # Clear the array
        @LineItems = ChkInf::GetList($CurrentLine,$CurrentINF->getIndex($Section,$count));

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
            AddNetCardError(4033,$CurrentINF->getIndex($Section,$count),"5");
            $count++;
            next;
        }

        if ($LineItems[1] =~ /^NDI/i) {
            if ($LineItems[2] =~ /SERVICE/i) {
                if ( defined($LineItems[4]) ) {
                    $AddRegList{'SERVICE'} = $LineItems[4];
                    $AddRegList{'SERVICE'} =~ s/\"//g;
                }
            }

            if ($LineItems[1] =~ /INTERFACES/i) {
                my(@tmp3,$cnt,$i,$str); #set of interfaces can be defined..
                $str = $LineItems[4];
                @tmp3 = split(',',$str);
                $cnt = $#tmp3;
                $cnt=0 unless(defined($cnt));

                for ($i=0; $i < $cnt; $i++) {
                    AddNetCardError(4005,$CurrentINF->getIndex($Section,$count) ) if (!defined($ValidInterfaces{uc $tmp3[$i]}));
                }
            }


            if ($LineItems[1] =~ /PARAMS/i) {
                my(@tmp3,$val);
                @tmp3 = split(/\\/,$LineItems[1]);
                $ParamsDefined = 1;
                $val = hex($ModelChar)&0x80;
                AddNetCardError(4007,$CurrentINF->getIndex($Section,$count) ) if (defined($ReservedWords{uc $tmp3[2]}));

                if ($LineItems[1] !~ /ENUM/i) {
                    AddNetCardError(4008,$CurrentINF->getIndex($Section,$count)) if (!defined($ParamKeys{uc $LineItems[2]}));
                }
            }

            if ($LineItems[1] =~ /ENUM/i) {
                # No verification yet
            }
        }
        $count++;
    }

    if ($ParamsDefined == 1) {
        AddNetCardError(4009,$CurrentINF->getIndex($Section) ) if ($ParamVerify == 0);
    }
    return(TRUE);
}
#-----------------------CheckServiceName($Section)-----------------------
sub CheckServiceName {
    my $Section  =  shift;
    my @lines    = $CurrentINF->getSection($Section);
    my $count    = 0;
    my $line;

    my($Directive,@Values, $Value);
    my($ucAddReg,$ucAddService);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values             = ChkInf::GetList($Value,$CurrentINF->getIndex($Section,$count));

        $ucAddReg =     uc $AddRegList{'SERVICE'};
        $ucAddService = uc $Values[0];

        if ( (!defined($Values[0]) ) or
             (!defined($AddRegList{'SERVICE'}) ) or
             ($ucAddService ne $ucAddReg) ) {
            AddNetCardError(4011,$CurrentINF->getIndex($Section,$count) );
        }

        $count++;
    }
    return(TRUE);
}
#-----------------------CheckCompatibleKey--------------------------------
sub CheckCompatibleKey {
    my $Section = "Version";
    my @lines   = $CurrentINF->getSection($Section);
    my $count   = 0;
    my $CompPresent = FALSE;

    my ($line, $Directive,$Value);
    my ($Signature,$CompVal,$CompLine);

    $CompLine = $count;

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);  # Skip if blank line

        ($Directive, $Value) = ChkInf::SplitLine($Section,$count,$line);

        if ($Directive =~ /SIGNATURE/i) {
            $Signature = $Value;

        } elsif ($Directive =~ /COMPATIBLE/i) {
            $CompVal = $Value;
            $CompPresent = 1;
            $CompLine = $line;
        }

        $count++
    }

    if ($Signature =~ /CHICAGO/i) {
        if ( $CompPresent == 1) {
            if ($CompVal != 1) {
                AddNetCardError(3046,$CurrentINF->getIndex($Section,$CompLine) );
            }
        } else {
            AddNetCardError(3045,$CurrentINF->getIndex($Section,$CompLine) );
        }
    }
    return(TRUE);
}



