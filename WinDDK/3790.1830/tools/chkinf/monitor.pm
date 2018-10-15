# Copyright 1999-2000 Microsoft Corporation

package MONITOR;          # The name of this module. For our purposes,
                          #  must be all in caps.

    use strict;           # Keep this code clean
    my $Version  = "5.1.2402.0"; # Version of this file
    my $Current_INF_File;

    sub TRUE  { return(1); }
    sub FALSE { return(0); }

1;

# Subroutine required to be defined
sub PrintHeader {        return(-1);}

# Subroutine required to be defined
sub PrintDetails {       return(-1);}

#-------------------------------------------------------------------------------------------------#
# Required subroutine! Used to verify a Directive as being device specific.                       #
#-- Exists ---------------------------------------------------------------------------------------#
sub DCDirective {
    my($Directive) = $_[1];
    return(FALSE); # Only return 1 if validating the line
}

#-------------------------------------------------------------------------------------------------#
# Required subroutine! Allows us to verify that the module loaded correctly.                      #
#-------------------------------------------------------------------------------------------------#
sub Exists {
    print(STDERR "Module \"MONITOR\" Loaded\n");
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# REQUIRED- Entry point for Class specific checking                                               #
#-------------------------------------------------------------------------------------------------#
sub InfCheck {
    ClassInstall32("ClassInstall32")
            if ( $Current_INF_File->sectionDefined("CLASSINSTALL32") );
    ClassInstall32("ClassInstall32.NT")
            if ( $Current_INF_File->sectionDefined("CLASSINSTALL32.NT") );
    ClassInstall32("ClassInstall32.NTX86")
            if ( $Current_INF_File->sectionDefined("CLASSINSTALL32.NTX86") );

    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# Required subroutine                                                                             #
#-------------------------------------------------------------------------------------------------#
sub InitGlobals {
    # Is called when module is first included.
    # Use it to set your per-inf variables.
    shift;
    $Current_INF_File = shift;
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# Verifies required regkeys in ClassInstall32 sections                                            #
#-------------------------------------------------------------------------------------------------#
sub ClassInstall32 {
    my $Section = shift;
    my @lines   = $Current_INF_File->getSection($Section);
    my $count   = 0;
    my $line;

    my($Directive, $Value, @Sections);

    foreach $line (@lines) {
        $count++, next if ($line =~ /^\s*$/);# Skip if it's a blank line

        ($Directive, $Value) = ChkInf::SplitLine($Section,$count,$line);

        if (uc($Directive) eq "ADDREG") {
            @Sections = ChkInf::GetList($Value,$Current_INF_File->getIndex($Section,$count));
            foreach (@Sections) {
                CheckAddReg($_);
            }
        }

      $count++;
    }
    return(TRUE);

}

#-------------------------------------------------------------------------------------------------#
# Helper routine for ClassInstall32()                                                             #
#-------------------------------------------------------------------------------------------------#
sub CheckAddReg {
    my $Section  = shift;
    my @lines   = $Current_INF_File->getSection($Section);
    my $count   = 0;
    my $line;

    my @NeededKeys = (0,0,0);
    my $SecStart   = $Current_INF_File->getIndex($Section) - 1;
    my @RegKey;
    my $REG;

    my $Pattern1 = quotemeta("HKR,,NoInstallClass,,1");
    my $Pattern2 = quotemeta("HKR,,Icon,,-1");
    my $Pattern3 = quotemeta("HKR,,,,%MonitorClassName%");
    my($Quote);

    foreach $line (@lines) {

        $count++, next if ($line =~ /^\s*$/);# Skip if it's a blank line

        $REG  = $line;

        $REG =~ s/\"//g;
        if ($REG =~ /^$Pattern1$/i) {
            $NeededKeys[0] = 1;
        } elsif ($REG =~ /^$Pattern2$/i) {
            $NeededKeys[1] = 1;
        } elsif ($REG =~ /^$Pattern3.*/i) {
            $NeededKeys[2] = 1;
        }
        $count++;
    }

    my($i);
    if (! $NeededKeys[0]) {
            ChkInf::AddDeviceSpecificError($SecStart,3001,"Missing a required regkey! (\"HKR,,NoInstallClass,,1\")");
    }
    if (! $NeededKeys[1]) {
            ChkInf::AddDeviceSpecificError($SecStart,3002,"Missing a required regkey! (\"HKR,,Icon,,-1\"");
    }
    if (! $NeededKeys[2]) {
            ChkInf::AddDeviceSpecificError($SecStart,3003,"Missing a required regkey! (\"HKR,,,,%MonitorClassName%\")");
    }

    return(TRUE);
}
