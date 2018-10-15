# (C) Copyright 1999-2003 Microsoft Corporation

package DISPLAY;

    use strict;                # Keep the code clean
    no strict 'refs';          # no strict references

    sub FALSE { return(0); }     # Utility subs
    sub TRUE  { return(1); }

    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my $Version  = "5.1.2250.0"; # Version of this file
    my $DEBUG    = 0;            # Set to 1 for debugging output

    my $Current_INF_File;       # Stores name of the current INF file.
    my $Current_HTM_File;       # Name of the output file to use.

    my @Device_Class_Options;   # Array to store device specific options.

    my %AllModels;              # Array of all models present in INF (Model) = (Type)
    my %DisplayErrorTable  = (6001 => ["Directive %s invalid in this section."],
                              6002 => ["%s takes a single numeric parameter."],
                              6003 => ["The only values %s allows are 0 and 1."],
                              6004 => ["Applet extensions should be installed to HKLM,".
                                       "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\Display."],
                            );
    my %DisplayWarningTable =();

#---------------------------------------------------#
# Routine for checking device class specific
# directives that occur in common sections verified
# by ChkINF.  Return 0 if directive is unknown or
# 1 if the directive is known (you should also verify
# it and write any appropriate warnings/errors.
#---------------------------------------------------#
sub DCDirective {
    return(0); # return 1 on success
}

#-------------------------------------------------------------------------------------------------#
# Main Entry Point to Device Specific Module. REQUIRED!                                           #
#-- InfCheck() -----------------------------------------------------------------------------------#
sub InfCheck {
    my($Item);
	my $DeviceID;
	my $Section;
    my $SkipUndecorated = 0;

    print(STDERR "\tInvoking Display::InfCheck...\n") if ($DEBUG);

    # Propagate a list of all models found in the INF
    %AllModels  = LearnModels();

    if ( $Current_INF_File->sectionDefined("GeneralConfigData") ) {
        # We should work with this && .NTALPHA if exists
        VerifyGCData("GeneralConfigData");
    }

    # Create an array for each model.
    # Array name is uc(Model) and first element is Model
    foreach $DeviceID (keys(%AllModels)) {
        $Section      = $AllModels{uc $DeviceID}{"DDINSTALL"};
        CheckModel($Section);
    }

#    foreach $DeviceID (keys %AllModels) {
#        $Section      = $AllModels{uc $DeviceID}{"DDINSTALL"};
#        # Check all existing NT/2000 sections
#        if ( $Current_INF_File->sectionDefined("$Section.NT") ) {
#            ProcessDeviceID($DeviceID,"NT",$AllModels{$DeviceID});
#            $SkipUndecorated++;
#        }
#        if ( $Current_INF_File->sectionDefined("$Section.NTX86") ) {
#            ProcessDeviceID($DeviceID,"NTX86",$AllModels{$DeviceID});
#            $SkipUndecorated++;
#        }
#        if ( $Current_INF_File->sectionDefined("$Section.NTIA64") ) {
#            ProcessDeviceID($DeviceID,"NTIA64",$AllModels{$DeviceID});
#            $SkipUndecorated++;
#        }
#        # Only check default if no NT/2000 section is defined
#        if ( $Current_INF_File->sectionDefined("$Section") and (! $SkipUndecorated) ){
#            ProcessDeviceID($DeviceID,"",$AllModels{$DeviceID});
#        }
#    }

}

#---------------------------------------------------#
# Verifies that the module loaded correctly.  Be sure
# to change the string to the name of your module.
#---------------------------------------------------#
sub Exists {
    print(STDERR "Module \"DISPLAY\" Loaded\n");
    return(1); # return 1 on success
}


#---------------------------------------------------#
# Is called when module is first included.
# Use it to set your per-inf variables.
#---------------------------------------------------#
sub InitGlobals {
    # First parameter is the INF object
    $Current_INF_File=$_[1];

    # Output file = htm subdir + ((INF Name - .inf) + .htm)
    $Current_HTM_File = $Current_INF_File->{inffile};
    $Current_HTM_File = substr($Current_HTM_File, rindex($Current_HTM_File,"\\")+1);
    $Current_HTM_File =~ s/\.inf$/\.htm/i;
    $Current_HTM_File = "htm\\" . $Current_HTM_File;

    # Second parameter is a list of Device Specific options.
    my($DC_Specific_Options) = $_[2];

    # $DC_Specific_Options is a string that contains all device
    #   class options delimited by &'s
    if (defined($DC_Specific_Options)) {
        @Device_Class_Options = split(/&/,$DC_Specific_Options);
    } else {
        @Device_Class_Options = ("NULL", "NULL");
    }

    # Array must be shifted since first element also had a & prepended
    shift(@Device_Class_Options);

    return(1); # return 1 on success
}


#---------------------------------------------------#
# Allows to add Device specific information to the
# top of the INF summary page.  The information here
# should be brief and link to detailed summaries
# below. (Which will be written using PrintDetials).
#---------------------------------------------------#
sub PrintHeader {
    return(1); # return 1 on success
}


#---------------------------------------------------#
# Allows addition of device specific results to the
# detail section on the INF summary.
#---------------------------------------------------#
sub PrintDetails {
    return(1); # return 1 on success
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
    my($Model) = $_[0];

    print(STDERR "\tInvoking Display::CheckModel...\n") if ($DEBUG);

    if ( $Current_INF_File->sectionDefined($Model.".SOFTWARESETTINGS") ) {
        # We should work with this && .NTALPHA if exists
        VerifySoftwareSettings($Model.".SOFTWARESETTINGS");
    }

    if ( $Current_INF_File->sectionDefined($Model.".OPENGLSOFTWARESETTINGS") ) {
        # We should work with this && .NTALPHA if exists
        VerifySoftwareSettings($Model,".OPENGLSOFTWARESETTINGS");
    }

    if ( $Current_INF_File->sectionDefined($Model.".GENERALCONFIGDATA") ) {
        # We should work with this && .NTALPHA if exists
        VerifyGCData($Model.".GENERALCONFIGDATA");
    }

    my @lines   = $Current_INF_File->getSection($Model);
	my $line;
	my $count = 0;
	my $Directive;
	my $Value;
	my $Section = $Model;
	my @RegLines;

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/); # Skip if it's a blank line
        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);

        if (uc $Directive eq "ADDREG") {
            foreach ( ChkInf::GetList($Value,$Current_INF_File->getIndex($Section,$count)) ) {
                push(@RegLines, RegLines($_)) if ( $Current_INF_File->sectionDefined($_) );
            }
        }
    }

	my @regvalues;
	foreach (@RegLines) {
		if ($$_[0] =~ /^HKLM\s*,\s*"?SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls\sFolder\\Device/i) {
			AddDisplayError(6004,$$_[1]);
		}
	}

    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# LearnModels - Returns an array of all models listed in the INF                                  #
#-- LearnModels() --------------------------------------------------------------------------------#
sub LearnModels {
    return(FALSE) if (! $Current_INF_File->sectionDefined("Manufacturer") );
    my $Section = "Manufacturer";
    my $line;
    my @lines   = $Current_INF_File->getSection("Manufacturer");
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

        if ( $Current_INF_File->sectionDefined($Value) ) {
            # Add the models from $INFSections{$Value} to our running list
            %Models = (%Models, ExpandModelSection($Value));
        }
        $count++;
    }

    return(%Models);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyGCData($line, $Model, $Extension) ------------------------------------------------------#
sub VerifyGCData {
    my $Section     = $_[0];
    my @lines       = $Current_INF_File->getSection($Section);;
    my $count       = 0;
    my $line;

    my($Directive, @Values, $Value);

    my(%SecToCheck) = ("MAXIMUMDEVICEMEMORYCONFIGURATION", TRUE,
                       "MAXIMUMNUMBEROFDEVICES"          , TRUE,
                       "KEEPEXISTINGDRIVERENABLED"       , TRUE,
                       "COPYFILES"						 , TRUE );


    my($CurrentLine, $Temp, @LineItems, $SizeOfArray);
	ChkInf::Mark("$Section");

    print(STDERR "\tBeginning VerifyGCData (@_)...\n") if ($DEBUG);
    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if it's a blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values             = ChkInf::GetList($Value,$Current_INF_File->getIndex($Section,$count));

        if (uc($Directive) eq "MAXIMUMDEVICEMEMORYCONFIGURATION") {
            if (defined($Values[0])) {
                if (defined($Values[1])) {
                    AddDisplayError(6002,$Current_INF_File->getIndex($Section,$count),$Directive);
                }
            } else {
                AddDisplayError(6002,$Current_INF_File->getIndex($Section,$count),$Directive);
            }
        } elsif (uc($Directive) eq "MAXIMUMNUMBEROFDEVICES") {
            if (defined($Values[0])) {
                if (defined($Values[1])) {
                    AddDisplayError(6002,$Current_INF_File->getIndex($Section,$count),$Directive);
                }
            } else {
                AddDisplayError(6002,$Current_INF_File->getIndex($Section,$count),$Directive);
            }
        } elsif (uc($Directive) eq "KEEPEXISTINGDRIVERENABLED") {
            if (defined($Values[0])) {
                if (defined($Values[1])) {
                    AddDisplayError(6002,$Current_INF_File->getIndex($Section,$count),$Directive);
                }
                if (($Values[0] != 1) and ($Values[0] != 0)) {
                    AddDisplayError(6003,$Current_INF_File->getIndex($Section,$count),$Directive);
                }
            } else {
                AddDisplayError(6002,$Current_INF_File->getIndex($Section,$count),$Directive);
            }
        } else {
            AddDisplayError(6001,$Current_INF_File->getIndex($Section,$count),$Directive)
            	unless (defined $SecToCheck{uc $Directive});
        }
        $count++;

    }

    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifySoftwareSettings($line, $Model, $Extension) --------------------------------------------#
sub VerifySoftwareSettings {
    my $Section     = $_[0];

    my @lines      = $Current_INF_File->getSection($Section);
    my $count      = 0;
    my $line;

    my($Directive, @Values, $Value);


    my(%SecToCheck) = ("ADDREG",    TRUE,
                       "COPYFILES", TRUE,
                       "NEEDS",     TRUE);

    my($CurrentLine, $Temp, @LineItems, $SizeOfArray);

	ChkInf::Mark("$Section");

    print(STDERR "\tBeginning VerifyModel (@_)...\n") if ($DEBUG);
    foreach $line (@lines) {

        $count++, next if ($line =~ /^\s*$/);# Skip if it's a blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values             = ChkInf::GetList($line,$Current_INF_File->getIndex($Section,$count));

        if (uc($Directive) eq "ADDREG") {
            # ADDREG is valid, so call the ChkInf ADDREG checking routine
            ChkInf::ADDREG($Section, $count, $Value);
        } elsif (uc($Directive) eq "DELREG") {
            # ADDREG is valid, so call the ChkInf ADDREG checking routine
            ChkInf::DELREG($Section, $count, $Value);
        } else {
            AddDisplayError(6001,$Current_INF_File->getIndex($Section,$count),$Directive)
            	unless (defined $SecToCheck{uc $Directive});
        }

        $count++;

    }
    return(TRUE);
}

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                   HELPER SUBROUTINES                                          |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################

#-------------------------------------------------------------------------------------------------#
# This sub adds a new error to the list of errors for the file.                                   #
#-- AddDisplayError($ErrorNum, $LineNum, @ErrorArgs) ---------------------------------------------#
sub AddDisplayError {
    my($ErrorNum) = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@ErrorArgs)= @_;

    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    $info_err = $DisplayErrorTable{"$ErrorNum"};

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
#-- AddDisplayWarning($WarnNum, $LineNum, @ErrorArgs) --------------------------------------------#
sub AddDisplayWarning {
    my($WarnNum)  = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@WarnArgs) = @_;

    $WarnArgs[0] = " " if (! defined($WarnArgs[0]));
    my($str, $this_str, $info_wrn, $format_wrn);

    $info_wrn = $DisplayWarningTable{"$WarnNum"};

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
    my @lines    =  $Current_INF_File->getSection($Section);
    my $count    = 0;

    my(%Models)  =  ();
    my($TempVar);

    my($Directive, @Values, $Value, $DevSection, $Provider, $DevDesc, $DevID);

    print(STDERR "\tInvoking Display::ExpandModelSection...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if it's a blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values             = ChkInf::GetList($Value,$Current_INF_File->getIndex($Section,$count));

        $Values[0] =~ s/^\s*//i; # Kill leading spaces
        $Values[0] =~ s/\s*$//i; # Kill trailing spaces

        if (defined $Values[0]) {
            $DevSection = $Values[0];
        } else {
            # ERROR
            next;
        }
    
        if (defined $Values[1]) {
            $DevID = $Values[1];
        } else {
            # ERROR
            next;
        }

        if (defined($Values[0])) {
            if (defined $Models{uc $DevID}) {
                # ERROR
            } else {
               $Models{uc $DevID}{"DDINSTALL"}   = $DevSection;
               $Models{uc $DevID}{"DESCRIPTION"} = $DevDesc;
               $Models{uc $DevID}{"PROVIDER"}    = $Provider;
            }
        }
        $count++;
    }

    return(%Models);
}

#-------------------------------------------------------------------------------------------------#
# RegLines - returns all reglines (w/ string tokens removed) from a section.                      #
#-- RegLines() -----------------------------------------------------------------------------------#
sub RegLines {
    my $line;
    my $Section  =  shift;
    my @lines    =  $Current_INF_File->getSection($Section);
    my $count    =  0;
    my @reglines;

    print(STDERR "\tInvoking Media::RegLines...\n") if ($DEBUG);

    foreach $line (@lines) {
        #Skip blank lines even though this should never be true.
        $count++, next if ($line =~ /^[\s\t]*$/);
        # Remove all string tokens
        $line = ChkInf::RemoveStringTokens($Current_INF_File->getIndex($Section,$count),$line);
        # Update our list with [$line, $linenum]
        push(@reglines, [$line,$Current_INF_File->getIndex($Section,$count)]);
        $count++;
    }
    return(@reglines);
}

return(1); # Module must return 1 to let chkinf know it loaded correctly.
