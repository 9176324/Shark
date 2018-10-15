# (C) Copyright 1999-2003 Microsoft Corporation

# The name of this module. For our purposes it must be all in caps.
package MEDIA;

    use strict;                # Keep the code clean
    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my $Version  = "5.1.2402.0"; # Version of this file
    my $inffile;                 # Stores name of the current INF file.
    my $DEBUG    = 0;            # True for debug spew
    my %SectionsChecked;         # Hash for every section looked at
    #
    # Required, but currently unused routines
    #
    sub PrintHeader  { return(1); }
    sub PrintDetails { return(1); }

return(1); # Module must return 1 to let chkinf know it loaded correctly.
#---------------------------------------------------#
# Routine for checking device class specific
# directives that occur in common sections verified
# by ChkINF.  Return 0 if directive is unknown or
# 1 if the directive is known (you should also verify
# it and write any appropriate warnings/errors.
#---------------------------------------------------#
sub DCDirective {
    shift;
    my $Directive        = shift;
    my %ValidDirectives  = ( "KNOWNREGENTRIES" => 1,
                             "KNOWNFILES"      => 1);
    return(0) unless (defined $ValidDirectives{$Directive});
    return(1);
}

#---------------------------------------------------#
# Verifies that the module loaded correctly.  Be sure
# to change the string to the name of your module.
#---------------------------------------------------#
sub Exists {
    print(STDERR "Module \"MEDIA\" Loaded\n");
    return(1); # return 1 on success
}

#---------------------------------------------------#
# The module's main checking routine.
# This is your entry point for verifying an INF
#---------------------------------------------------#
sub InfCheck {
    my %AllDevices = LearnModels();
    #
    # AllDevices{DEVICE_ID}{PROVIDER}
    #                      {DESCRIPTION}
    #                      {DDINSTALL}
    #
    my $DeviceID;
    my $Section;
    my $SkipUndecorated = 0;

    foreach $DeviceID (keys %AllDevices) {
        $Section      = $AllDevices{uc $DeviceID}{"DDINSTALL"};
        # Check all existing NT/2000 sections
        if ( $inffile->sectionDefined("$Section.NT") ) {
            ProcessDeviceID($DeviceID,"NT",$AllDevices{$DeviceID});
            $SkipUndecorated++;
        }
        if ( $inffile->sectionDefined("$Section.NTX86") ) {
            ProcessDeviceID($DeviceID,"NTX86",$AllDevices{$DeviceID});
            $SkipUndecorated++;
        }
        if ( $inffile->sectionDefined("$Section.NTIA64") ) {
            ProcessDeviceID($DeviceID,"NTIA64",$AllDevices{$DeviceID});
            $SkipUndecorated++;
        }
        # Only check default if no NT/2000 section is defined
        if ( $inffile->sectionDefined("$Section") and (! $SkipUndecorated) ){
            ProcessDeviceID($DeviceID,"",$AllDevices{$DeviceID});
        }
    }

    return(1); # return 1 on success
}

#---------------------------------------------------#
# Is called when module is first included.
# Use it to set your per-inf variables.
#---------------------------------------------------#
sub InitGlobals {
    shift;
    # First parameter is the INF object
    $inffile=shift;
    # Media.pm does not accept DeviceClassOptions, so ignore
    # the rest of the parameters.

    # Reset globals
    %SectionsChecked=();
    return(1); # return 1 on success
}

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                 CLASS SPECIFIC SUBROUTINES                                    |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################

#-------------------------------------------------------------------------------------------------#
# This sub adds a new error to the list of errors for the file.                                   #
#-- AddMediaError($ErrorNum, $LineNum, @ErrorArgs) -----------------------------------------------#
sub AddMediaError {

	my %MediaErrorTable = (
    	8001 => ["GUID %s requires the non-localizable reference string \"%s\"."],
	    8002 => ["All audio port device install sections must contain 'NEEDS=%s'."],
	    8003 => ["All audio port device install sections must contain 'INCLUDE=%s'."],
    );

    my $ErrorNum  = shift;
    my $LineNum   = shift;
    my @ErrorArgs = @_;

    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    $info_err = $MediaErrorTable{$ErrorNum};

    if ( !defined($info_err) ) {
        $this_str = "Unknown error $ErrorNum.";
    } else {
        $format_err = $$info_err[0];
        $this_str = sprintf($format_err, @ErrorArgs);
    }
    ChkInf::AddDeviceSpecificError($LineNum, $ErrorNum, $this_str);
}

#-------------------------------------------------------------------------------------------------#
#-- ExpandModelSection($line, $Section) ----------------------------------------------------------#
sub ExpandModelSection {
    my $Section  = shift;
    my $Provider = shift;
    my $line;
    my @lines    =  $inffile->getSection($Section);
    my $count    = 0;

    my %Models   =  ();
    my $TempVar;

    my ($DevDesc, @Values, $Value);
    my ($DevSection, $DevID);

    print(STDERR "\tInvoking Media::ExpandModelSection...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if it's a blank line

        ($DevDesc,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values           = ChkInf::GetList($Value,$inffile->getIndex($Section,$count));

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
# Interfaces - returns all addinterfaces (w/ string tokens removed) from a section.               #
#-- Interfaces() ---------------------------------------------------------------------------------#
sub Interfaces {
    my $line;
    my $Section  =  shift;
    my @lines    =  $inffile->getSection($Section);
    my $count    =  0;
    my @interfaces;
    my ($Directive,$Value);

    print(STDERR "\tInvoking Media::Interfaces...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^[\s\t]*$/);
        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);

        if (uc $Directive eq "ADDINTERFACE") {
            # Update our list with [$value, $linenum]
            push(@interfaces, [$Value,$inffile->getIndex($Section,$count)]);
        }

        $count++;
    }

    return(@interfaces);
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

    my($Provider, $DevSection);
    my(%Models);

    print(STDERR "\tInvoking Media::LearnModels...\n") if ($DEBUG);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/); # Skip if it's a blank line

        ($Provider,$DevSection) = ChkInf::SplitLine($Section,$count,$line);

        # [Manufacturers] may not have both a directive & a value
        if ($DevSection =~ /^\s*$/) {
            $DevSection = $Provider;
        } else {
            $DevSection    =~  s/^\"(.*)\"/$1/;   # And Strip the Quotes!
        }

        if ( $inffile->sectionDefined($DevSection) ) {
            # Add the models from $INFSections{$DevSection} to our running list
            %Models = (%Models, ExpandModelSection($DevSection,$Provider));
        }
        $count++;
    }

    return(%Models);
}

#-------------------------------------------------------------------------------------------------#
# ProcessDeviceID - Validate a Device Installation                                                #
#-- ProcessDeviceID() ----------------------------------------------------------------------------#
sub ProcessDeviceID {
    my $DeviceID     = shift;
    my $Extension    = shift;
    # Get the passed hash
    my %Device       = %{$_[0]}; shift;

    my $Section      = $Device{"DDINSTALL"};
    if (defined $Extension) {
        $Section     = "$Section.$Extension";
    }

    return() if (defined $SectionsChecked{uc $Section});
    return() if (! $inffile->sectionDefined($Section) );

    print(STDERR "\tInvoking Media::ProcessDeviceID...\n") if ($DEBUG);

    # Define so section isn't checked again.
    $SectionsChecked{uc $Section} = 1;

    my $line;
    my @lines   = $inffile->getSection($Section);
    my $count   = 0;
    my @Include;
    my @Needs;
    my @RegLines;
    my @Interfaces;
    my $Interface;

    my ($Directive,$Value);
    my ($GUID,$RefStr,$Sec,$Flags);
    my $IsAudio = 0;

    my %KnownGUIDS = (
        "6994AD04-93EF-11D0-A3CC-00A0C9223196" => "WAVE",      # AUDIO
        "65E8773E-8F56-11D0-A3B9-00A0C9223196" => "WAVE",      # RENDER
        "65E8773D-8F56-11D0-A3B9-00A0C9223196" => "WAVE",      # CAPTURE
        "DDA54A40-1E4C-11D1-A050-405705C10000" => "TOPOLOGY",  # TOPOLOGY
		"DFF220F3-F70F-11D0-B917-00A0C9223196" => "DMUSIC",    # SYNTHESIZER
        );

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/); # Skip if it's a blank line
        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);

        if      (uc $Directive eq "INCLUDE") {
            push(@Include,( ChkInf::GetList($Value,$inffile->getIndex($Section,$count)) ));
        } elsif (uc $Directive eq "NEEDS") {
            push(@Needs,( ChkInf::GetList($Value,$inffile->getIndex($Section,$count)) ));
        } elsif (uc $Directive eq "ADDREG") {
            foreach ( ChkInf::GetList($Value,$inffile->getIndex($Section,$count)) ) {
                push(@RegLines, RegLines($_)) if ( $inffile->sectionDefined($_) );
            }
        }
    }

    if ( $inffile->sectionDefined("$Section.INTERFACES") ) {
        push(@Interfaces,Interfaces("$Section.INTERFACES"));
    }

    foreach $Interface (@Interfaces) {
        $count = $$Interface[1];
        $line  = $$Interface[0];

        ($GUID,$RefStr,$Sec,$Flags) = ChkInf::GetList($line,$count);
        $GUID   =~ s/{(.*)}/$1/;
        $GUID   =~ s/\"(.*)\"/$1/;
        $GUID   =  uc $GUID;
        $RefStr =~ s/\"(.*)\"/$1/;
        $RefStr =  uc $RefStr;

        if ($GUID eq "6994AD04-93EF-11D0-A3CC-00A0C9223196") {
            $IsAudio++;
        }

		# The Audio Team no longer believes this is the case.
        #if (defined $KnownGUIDS{$GUID} ) {
        #    AddMediaError(8001,$count, $GUID, $KnownGUIDS{$GUID})
        #        unless ( $KnownGUIDS{$GUID} eq $RefStr );
        #}
    }

    if ($IsAudio) {
		unless ( grep( /KS.Registration/i, @Needs) ) {
			AddMediaError(8002,$inffile->getIndex($Section) - 1,"KS.Registration");
        }
        unless ( grep( /WDMAUDIO.Registration/i, @Needs) ) {
			AddMediaError(8002,$inffile->getIndex($Section) - 1,"WDMAUDIO.Registration");
        }
		unless ( grep( /ks.inf/i, @Include) ) {
			AddMediaError(8003,$inffile->getIndex($Section) - 1,"ks.inf");
        }
        unless ( grep( /wdmaudio.inf/i, @Include) ) {
			AddMediaError(8002,$inffile->getIndex($Section) - 1,"wdmaudio.inf");
        }
    }

	return 1;
}

#-------------------------------------------------------------------------------------------------#
# RegLines - returns all reglines (w/ string tokens removed) from a section.                      #
#-- RegLines() -----------------------------------------------------------------------------------#
sub RegLines {
    my $line;
    my $Section  =  shift;
    my @lines    =  $inffile->getSection($Section);
    my $count    =  0;
    my @reglines;

    print(STDERR "\tInvoking Media::RegLines...\n") if ($DEBUG);

    foreach $line (@lines) {
        #Skip blank lines even though this should never be true.
        $count++, next if ($line =~ /^[\s\t]*$/);
        # Remove all string tokens
        $line = ChkInf::RemoveStringTokens($inffile->getIndex($Section,$count),$line);
        # Update our list with [$line, $linenum]
        push(@reglines, [$line,$inffile->getIndex($Section,$count)]);
        $count++;
    }
    return(@reglines);
}
