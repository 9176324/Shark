# (C) Copyright 1999-2003 Microsoft Corporation

# The name of this module. For our purposes it must be all in caps.
package DISKDRIVE;

    use strict;                # Keep the code clean
    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my($Version) = "5.1.2402.0"; # Version of this file
    my($Current_INF_File);     # Stores name of the current INF file.
    my($Current_HTM_File);     # Name of the output file to use.
    my(@Device_Class_Options); # Array to store device specific options.


#-------------------------------------------------------------------------------------------------#
# This sub adds a new error to the list of errors for the file.
#-- AddDDError($ErrorNum, $LineNum, @ErrorArgs) --------------------------------------------------#
sub AddDDError {

    my %DDErrorTable = (
        8901 => ["The value for %s should not have bit 2 (0x4) set."],
    );

    my $ErrorNum  = shift;
    my $LineNum   = shift;
    my @ErrorArgs = @_;

    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    $info_err = $DDErrorTable{$ErrorNum};

    if ( !defined($info_err) ) {
        $this_str = "Unknown error $ErrorNum.";
    } else {
        $format_err = $$info_err[0];
        $this_str = sprintf($format_err, @ErrorArgs);
    }
    ChkInf::AddDeviceSpecificError($LineNum, $ErrorNum, $this_str);
}

#---------------------------------------------------#
# Checks for DiskCIPrivateData in the DDInstall
# sections.
#---------------------------------------------------#
sub CheckForDiskCIPrivateData {
    my $Section = shift;
    my $line;
    my @lines   = $Current_INF_File->getSection("$Section");
    my $count   = 0;

    my($Directive, $Value);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/); # Skip if it's a blank line

        ($Directive,$Value) = ChkInf::SplitLine($Section,$count,$line);

        if (uc $Directive eq "DISKCIPRIVATEDATA") {
            if ( ($Value & 0x4) == 0x4) {
                AddDDError(8901, $Current_INF_File->getIndex($Section, $count), $Directive)
                    unless (ChkInf::GetGlobalValue("MS_INTERNAL"));
            }
        }

        $count++;
    }

    return(1);
}
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
    my %ValidDirectives  = ( "DISKCIPRIVATEDATA" => 1);
    return(0) unless (defined $ValidDirectives{$Directive});
    return(1);
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

    foreach $DeviceID (keys %AllDevices) {
        $Section      = $AllDevices{uc $DeviceID}{"DDINSTALL"};

        if ( $Current_INF_File->sectionDefined("${Section}.NT") ) {
            CheckForDiskCIPrivateData("${Section}.NT");
        }
        if ( $Current_INF_File->sectionDefined("${Section}.NTX86") ) {
            CheckForDiskCIPrivateData("${Section}.NTX86");
        }
        if ( $Current_INF_File->sectionDefined("${Section}.NTIA64") ) {
            CheckForDiskCIPrivateData("${Section}.NTIA64");
        }
        if ( $Current_INF_File->sectionDefined("${Section}.AMD64") ) {
            CheckForDiskCIPrivateData("${Section}.AMD64");
        }
        if ( $Current_INF_File->sectionDefined("$Section") ){
            CheckForDiskCIPrivateData("$Section");
        }
    }

    return(1); # return 1 on success

}

#---------------------------------------------------#
# Verifies that the module loaded correctly.  Be sure
# to change the string to the name of your module.
#---------------------------------------------------#
sub Exists {
    print(STDERR "Module \"DISKDRIVE\" Loaded\n");
    return(1); # return 1 on success
}

#-------------------------------------------------------------------------------------------------#
#-- ExpandModelSection($line, $Section) ----------------------------------------------------------#
sub ExpandModelSection {
    my $Section  = shift;
    my $Provider = shift;
    my $line;
    my @lines    =  $Current_INF_File->getSection($Section);
    my $count    = 0;

    my %Models   =  ();
    my $TempVar;

    my ($DevDesc, @Values, $Value);
    my ($DevSection, $DevID);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if it's a blank line

        ($DevDesc,$Value) = ChkInf::SplitLine($Section,$count,$line);
        @Values           = ChkInf::GetList($Value,$Current_INF_File->getIndex($Section,$count));

        if (defined $Values[0]) {
            $DevSection = $Values[0];
        }

        if (defined $Values[1]) {
            $DevID = $Values[1];
        }

        if (defined($Values[0])) {
            if (! defined $Models{uc $DevID}) {
               $Models{uc $DevID}{"DDINSTALL"}   = $DevSection;
               $Models{uc $DevID}{"DESCRIPTION"} = $DevDesc;
               $Models{uc $DevID}{"PROVIDER"}    = $Provider;
            }
        }
        $count++;
    }

    return(%Models);
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


#-------------------------------------------------------------------------------------------------#
# LearnModels - Returns an array of all models listed in the INF                                  #
#-- LearnModels() --------------------------------------------------------------------------------#
sub LearnModels {
    return() if (! $Current_INF_File->sectionDefined("Manufacturer") );
    my $Section = "Manufacturer";
    my $line;
    my @lines   = $Current_INF_File->getSection("Manufacturer");
    my $count   = 0;

    my($Provider, $DevSection);
    my(%Models);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/); # Skip if it's a blank line

        ($Provider,$DevSection) = ChkInf::SplitLine($Section,$count,$line);

        # [Manufacturers] may not have both a directive & a value
        if ($DevSection =~ /^\s*$/) {
            $DevSection = $Provider;
        } else {
            $DevSection    =~  s/^\"(.*)\"/$1/;   # And Strip the Quotes!
        }

        if ( $Current_INF_File->sectionDefined($DevSection) ) {
            # Add the models from $INFSections{$DevSection} to our running list
            %Models = (%Models, ExpandModelSection($DevSection,$Provider));
        }
        $count++;
    }

    return(%Models);
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

return(1); # Module must return 1 to let chkinf know it loaded correctly.
