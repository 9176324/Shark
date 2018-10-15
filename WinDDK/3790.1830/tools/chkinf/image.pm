# Copyright 1999-2002 Microsoft Corporation

# The name of this module. For our purposes it must be all in caps.
package IMAGE;

    use strict;                 # Keep the code clean

    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my $Version = "5.2.3656.0"; # Version of this file
    my $DEBUG   = 0;            # Set to 1 for debugging output
    my $CurrentINF;             # Stores name of the current INF file.
    my @Device_Class_Options;   # Array to store device specific options.

    my %AllModels;              # Array of all models present in INF.
    my %ModelLines;             # Array of all directive lines for the current model.

#---------------------------------------------------#
# Routine for checking device class specific
# directives that occur in common sections verified
# by ChkINF.  Return 0 if directive is unknown or
# 1 if the directive is known (you should also verify
# it and write any appropriate warnings/errors.
#---------------------------------------------------#
sub DCDirective 
{
    my ($Module, $Directive) = @_;
    
    # Image specific directives
    my %ValidDirectives = 
    ( 
        "CAPABILITIES"     => 1,
        "CONNECTION"       => 1,
        "DESCRIPTION"      => 1,
        "DEVICEDATA"       => 1,
        "DEVICESUBTYPE"    => 1,
        "DEVICETYPE"       => 1,
        "EVENTS"           => 1,
        "ICMPROFILES"      => 1,
        "PORTSELECT"       => 1,
        "PROPERTYPAGES"    => 1,
        "SUBCLASS"         => 1,
        "UNINSTALLSECTION" => 1,
        "USDCLASS"         => 1,
        "VENDORSETUP"      => 1,
        "WIASECTION"       => 1,
    );

    # if defined, return true, else false    
    return defined($ValidDirectives{uc $Directive}); 
}

#---------------------------------------------------#
# Verifies that the module loaded correctly.  Be sure
# to change the string to the name of your module.
#---------------------------------------------------#
sub Exists 
{
    print(STDERR "Module \"IMAGE\" Loaded\n");
    
    return 1; # return 1 on success
}

#---------------------------------------------------#
# Is called when module is first included.
# Use it to set your per-inf variables.
#---------------------------------------------------#
sub InitGlobals 
{
    print(STDERR "\tInvoking Image::InitGlobals...\n") if ($DEBUG);
    
    # First parameter is the name of the current INF.
    $CurrentINF = $_[1];

    # Second parameter is a list of Device Specific options.
    my($DC_Specific_Options) = $_[2];

    # $DC_Specific_Options is a string that contains all device
    #   class options delimited by &'s
    if (defined($DC_Specific_Options)) 
    {
        @Device_Class_Options = split(/&/, $DC_Specific_Options);
    } 
    else 
    {
        @Device_Class_Options = ("NULL","NULL");
    }

    # Array must be shifted since first element also had a & prepended
    shift(@Device_Class_Options);

    return 1; # return 1 on success
}

#---------------------------------------------------#
# The module's main checking routine.
# This is your entry point for verifying an INF
#---------------------------------------------------#
sub InfCheck 
{
    print(STDERR "\tInvoking Image::InfCheck...\n") if ($DEBUG);
    
    %AllModels = LearnModels();

    foreach my $Model (keys(%AllModels))
    {        
        %ModelLines = LearnModelLines($Model);
        
        VerifyInstallSection();
        
        VerifyAddRegistrySection();
        
        VerifyDeviceDataSection();
        
        VerifyEventsSection();
        
        VerifyCopyFilesSection();
    }
    
    return 1; # return 1 on success
}

#---------------------------------------------------#
# Allows to add Device specific information to the
# top of the INF summary page.  The information here
# should be brief and link to detailed summaries
# below. (Which will be written using PrintDetails).
#---------------------------------------------------#
sub PrintHeader 
{
    return 1; # return 1 on success
}


#---------------------------------------------------#
# Allows addition of device specific results to the
# detail section on the INF summary.
#---------------------------------------------------#
sub PrintDetails 
{
    return 1; # return 1 on success
}

#-------------------------------------------------------------------------------------------------#
#-- AddError($LineNum, $ErrorNum, @ErrorArgs) ----------------------------------------------------#
sub AddError
{
    my ($LineNum, $ErrorNum, @ErrorArgs) = @_;

    my %ErrorTable = 
    (
        2001 => ["The value for %s should be %s."],
        2002 => ["Section [%s] should be defined."],
        2003 => ["The value for %s should be in the format \"%s\"."],
        2004 => ["The [%s] section for %s should have a %s line."],
        2005 => ["%s must have a corresponding %s line in the [%s] section."],
        2006 => ["For the device installation to succeed, section [%s] should be defined."],
        2007 => ["%s will not work as expected."],
    );
    
    my $info_err = $ErrorTable{$ErrorNum};

    my $this_str;

    if (!defined($info_err)) 
    {
        $this_str = "Unknown error $ErrorNum.";
    } 
    else 
    {
        $this_str = sprintf($$info_err[0], @ErrorArgs);
    }
    
    ChkInf::AddDeviceSpecificError($LineNum, $ErrorNum, $this_str);

    print(STDERR "$LineNum: (E $ErrorNum) $this_str\n") if ($DEBUG);
}

#-------------------------------------------------------------------------------------------------#
#-- AddWarning($LineNum, $ErrorNum, @ErrorArgs) --------------------------------------------------#
sub AddWarning 
{
    my ($LineNum, $WarnNum, @WarnArgs) = @_;

    my %WarningTable = 
    (
        3001 => ["%s may be included for backward compatibility, but it has no effect on Windows 2000/XP."],
        3002 => ["The value for %s should be %s."],
        3003 => ["The value for %s should be a file name with %s extension."],
        3004 => ["The [%s] section for %s should have a %s line."],
        3005 => ["Do not copy file %s directly, use %s in the [%s] section."],
        3006 => ["The value for %s specifies an unknown flag 0x%08X."],
        3007 => ["The value for %s should specify the STI_GENCAP_WIA flag if this is a WIA driver."],
    );
    
    my $info_wrn = $WarningTable{$WarnNum};

    my $this_str;

    if (!defined($info_wrn)) 
    {
        $this_str = "Unknown warning $WarnNum.";
    } 
    else 
    {
        $this_str = sprintf($$info_wrn[0], @WarnArgs);
    }
    
    ChkInf::AddDeviceSpecificWarning($LineNum, $WarnNum, $this_str);

    print(STDERR "$LineNum: (W $WarnNum) $this_str\n") if ($DEBUG);
}

#-------------------------------------------------------------------------------------------------#
# is_numeric - Returns true if the value is numeric                                               #
#-- is_numeric($Value) ---------------------------------------------------------------------------#
sub is_numeric
{
    my ($Value) = @_;

    return $Value =~ /^\d+$/;
}

#-------------------------------------------------------------------------------------------------#
# is_guid - Returns true if the value is a GUID                                                   #
#-- is_guid($Value) ------------------------------------------------------------------------------#
sub is_guid
{
    my ($Value) = @_;
    
    return $Value =~ /^\{[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}\}$/;
}

#-------------------------------------------------------------------------------------------------#
# is_defined - Returns an array of refences to the parsed lines if a matching directive           #
#   with an (optionally) matching value is found in the specified section                         #
#-- is_defined($SectionName, $DirectivePattern, $ValuePattern) -----------------------------------#
sub is_defined
{
    my ($SectionName, $DirectivePattern, $ValuePattern) = @_;
    
    my @Lines; 
    
    foreach my $Line (@{$ModelLines{$SectionName}})
    {
        my ($LineNum, $Directive, @Values) = @$Line;
                
        if ($Directive =~ /^$DirectivePattern$/)
        {
            if (!defined($ValuePattern) || grep(/^$ValuePattern$/, @Values))
            {
                push(@Lines, $Line);
            }
        }
    }

    return @Lines;
}

#-------------------------------------------------------------------------------------------------#
# LearnModels - Returns an array of all models listed in the INF                                  #
#-- LearnModels() --------------------------------------------------------------------------------#
sub LearnModels 
{
    print(STDERR "\tInvoking Image::LearnModels...\n") if ($DEBUG);
    
    my %Models;
    
    my @Lines = GetSectionLines(0, "MANUFACTURER");
    
    foreach my $Line (@Lines) 
    {
        my ($LineNum, $Directive, $SectionName, @Decorations) = @$Line;
        
        if (@Decorations == 0)
        {
            %Models = (%Models, ExpandModelSection($LineNum, "$SectionName"));
        }
        else
        {
            foreach my $Decoration (@Decorations)
            {
                %Models = (%Models, ExpandModelSection($LineNum, "$SectionName.$Decoration"));
            }
        }
    }
    
    return %Models;
}

#-------------------------------------------------------------------------------------------------#
# ExpandModelSection - Returns an array of install sections for the specified manufacturer models #
#-- ExpandModelSection($LineNum, $SectionName) ---------------------------------------------------#
sub ExpandModelSection 
{
    my ($LineNum, $SectionName) = @_;
    
    print(STDERR "\tInvoking Image::ExpandModelSection($SectionName)...\n") if ($DEBUG);
    
    my %PnPIDs;

    my %Models;
    
    my @Decorations = (".NTx86", ".NTIA64", ".AMD64", ".NT");
    
    my @Lines = GetSectionLines($LineNum, $SectionName);
    
    foreach my $Line (@Lines)
    {
        my ($LineNum, $Directive, @Values) = @$Line;

        # If multiple models have the same Rank 1 PnPID, include only the first one

        if (!defined($PnPIDs{"$Values[1]"}))
        { 
            $PnPIDs{"$Values[1]"} = 1;
        
            # Try to find the NT specific sections first
        
            my $DecoratedSectionFound = 0;
        
            foreach my $Decoration (@Decorations)
            { 
                if ($CurrentINF->sectionDefined("$Values[0]$Decoration")) 
                {
                    $Models{"$Values[0]$Decoration"} = $Values[1];
                
                    $DecoratedSectionFound = 1;
                }
            }
        
            # Check the undecorated section only if the NT sections are not present
        
            if (!$DecoratedSectionFound && $CurrentINF->sectionDefined("$Values[0]"))
            {
                $Models{"$Values[0]"} = $Values[1];
            }
        }
    }

    return %Models;
}

#-------------------------------------------------------------------------------------------------#
# LearnModelLines - Returns an array of refences to the parsed section lines                      #
#-- LearnModelLines($DDInstallSectionName) -------------------------------------------------------#
sub LearnModelLines
{
    my ($DDInstallSectionName) = @_;
    
    print(STDERR "\tInvoking Image::LearnModelLines($DDInstallSectionName)...\n") if ($DEBUG);
    
    my %ModelLines;
    
    if ($CurrentINF->sectionDefined($DDInstallSectionName)) 
    {
        push(@{$ModelLines{DDINSTALL}}, GetSectionLines(0, $DDInstallSectionName));
        
        foreach my $Line (@{$ModelLines{DDINSTALL}}) 
        {
            my ($LineNum, $Directive, @Values) = @$Line;
            
            if ($Directive eq "ADDREG")
            {
                push(@{$ModelLines{ADDREG}}, GetSectionLines($LineNum, @Values));
            }
            elsif ($Directive eq "COPYFILES")
            {
                push(@{$ModelLines{COPYFILES}}, GetSectionLines($LineNum, @Values));
            }
            elsif ($Directive eq "DEVICEDATA")
            {
                push(@{$ModelLines{DEVICEDATA}}, GetSectionLines($LineNum, @Values));
            }
            elsif ($Directive eq "EVENTS")
            {
                push(@{$ModelLines{EVENTS}}, GetSectionLines($LineNum, @Values));
            }
            elsif ($Directive eq "UNINSTALLSECTION")
            {
                push(@{$ModelLines{UNINSTALLSECTION}}, GetSectionLines($LineNum, @Values));
            }
            elsif ($Directive eq "WIASECTION")
            {
                push(@{$ModelLines{DDINSTALL}}, GetSectionLines($LineNum, @Values));
            }
        }
    }
    
    if (!$CurrentINF->sectionDefined("$DDInstallSectionName.SERVICES")) 
    {
        my $DDInstallSectionLineNum = ${@{$ModelLines{DDINSTALL}}}[0][0]-1;
        
        # 2006 => ["For the device installation to succeed, section [%s] should be defined."],
        AddError($DDInstallSectionLineNum, 2006, "$DDInstallSectionName.SERVICES");
    }
    else
    {
        push(@{$ModelLines{SERVICES}}, GetSectionLines(0, "$DDInstallSectionName.SERVICES"));
    }
    
    if ($CurrentINF->sectionDefined("$DDInstallSectionName.COINSTALLERS"))
    {
        push(@{$ModelLines{COINSTALLERS}}, GetSectionLines(0, "$DDInstallSectionName.COINSTALLERS"));
        
        foreach my $Line (@{$ModelLines{COINSTALLERS}}) 
        {
            my ($LineNum, $Directive, @Values) = @$Line;
            
            if ($Directive eq "ADDREG")
            {
                push(@{$ModelLines{ADDREG}}, GetSectionLines($LineNum, @Values));
            }
            elsif ($Directive eq "COPYFILES")
            {
                push(@{$ModelLines{COPYFILES}}, GetSectionLines($LineNum, @Values));
            }
        }
    }
    
    foreach my $Line (@{$ModelLines{UNINSTALLSECTION}}) 
    {
        my ($LineNum, $Directive, @Values) = @$Line;
        
        if ($Directive eq "DELFILES")
        {
            push(@{$ModelLines{DELFILES}}, GetSectionLines($LineNum, @Values));
        }
        elsif ($Directive eq "DELREG")
        {
            push(@{$ModelLines{DELREG}}, GetSectionLines($LineNum, @Values));
        }
    }
    
    return %ModelLines;
}

#-------------------------------------------------------------------------------------------------#
# GetSectionLines - Returns an array of refences to the parsed lines in the specified section(s)  #
#-- GetSectionLines($SectionListLineNum, @SectionListValues) -------------------------------------#
sub GetSectionLines
{
    my ($SectionListLineNum, @SectionListValues) = @_;
    
    print(STDERR "\tInvoking Image::GetSectionLines(@SectionListValues)...\n") if ($DEBUG);
    
    my @Lines;

    foreach my $SectionName (@SectionListValues)
    {
        if ($SectionName =~ /^@/)
        {
            # If the name begins with @, then this is actually a file name in CopyFiles
            
            push(@Lines, [$SectionListLineNum, "", substr($SectionName, 1)]);
        }
        elsif (!$CurrentINF->sectionDefined($SectionName)) 
        {   
            # 2002 => ["Section [%s] not defined."]
            AddError($SectionListLineNum, 2002, $SectionName);
        }
        else
        {
            # Mark the section as processed
            
            ChkInf::Mark($SectionName);
            
            # Read all the lines into and array and process one by one
            
            my @Section = $CurrentINF->getSection($SectionName);

            for my $Offset (0 .. $#Section) 
            {
                if ($Section[$Offset] !~ /^\s*$/) # Skip blank lines
                {
                    my $LineNum = $CurrentINF->getIndex($SectionName, $Offset);
                    
                    my ($Directive, @Values);
                    
                    if ($Section[$Offset] =~ /=/) 
                    {
                        # If the line is in the form "Directive=Value1,Value2,...", 
                        # split it into a $Directive string and an @Values array
                        
                        ($Directive, my $ValueList) = ChkInf::SplitLine($SectionName, $Offset, $Section[$Offset]);

                        @Values = ChkInf::GetList($ValueList, $LineNum, 1);
                    }
                    else
                    {
                        # If the line is in the form "Value1, Value2, ...",
                        # split it into an @Values array
                        
                        @Values = ChkInf::GetList($Section[$Offset], $LineNum, 1);
                    }
                    
                    # Convert the directive to uppercase
                    
                    $Directive = uc($Directive); 

                    foreach my $Value (@Values)
                    {
                        $Value =~ s/\"//g; # Strip quotes
                        
                        if ($Value =~ /^0[xX][a-fA-F0-9]+$/) 
                        {
                            # If the value is hexadecimal, convert it to decimal
                            
                            $Value = hex($Value); 
                        }
                        else
                        {
                            # Convert the value to uppercase
                            
                            $Value = uc($Value); 
                        }
                    }
                    
                    # If the line is in an included INF, use the referring line's number
                    
                    if ($LineNum == -1)
                    {
                        $LineNum = $SectionListLineNum;
                    }
                    
                    push(@Lines, [$LineNum, $Directive, @Values]);
                }
            }
        }
    }
    
    return @Lines;
}

#-------------------------------------------------------------------------------------------------#
# VerifyInstallSection - Verifies the DDINSTALL sections                                          #
#-- VerifyInstallSection() -----------------------------------------------------------------------#
sub VerifyInstallSection
{
    print(STDERR "\tInvoking Image::VerifyInstallSection...\n") if ($DEBUG);

    foreach my $Line (@{$ModelLines{DDINSTALL}}) 
    {
        my ($LineNum, $Directive, @Values) = @$Line;
        
        if ($Directive eq "CAPABILITIES") 
        {
            if (@Values != 1 || !is_numeric($Values[0]))
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "numeric");
            }
            else
            {
                if ($Values[0] & ~0x3F)
                {
                    # 3006 => ["The value of %s contains an unknown flag 0x%08X."]
                    AddWarning($LineNum, 3006, $Directive, $Values[0] & ~0x3F);
                }
                
                if ($Values[0] & 0x10)
                {
                    if (!is_defined("DDINSTALL", "DEVICEDATA"))
                    {
                        # 3004 => ["The [%s] section for %s should have a %s line."]
                        AddWarning($LineNum, 3004, "INSTALL", "WIA drivers", "DEVICEDATA");
                    }
                }
                else
                {
                    # 3007 => ["The value for %s should specify the WIA flag if this is a WIA driver."]
                    AddWarning($LineNum, 3007, $Directive);
                }
            }
        } 
        elsif ($Directive eq "CONNECTION") 
        {
            if (@Values != 1 || !grep(/^\Q$Values[0]\E$/, ("SERIAL", "PARALLEL")))
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "SERIAL or PARALLEL");
            }
        } 
        elsif ($Directive eq "DESCRIPTION") 
        {
            if (@Values != 1)
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "a string");
            }
        } 
        elsif ($Directive eq "DEVICESUBTYPE") 
        {   
            if (@Values != 1 || !is_numeric($Values[0]) || $Values[0] > 0xFFFF)
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "16-bit numeric");
            }
        }
        elsif ($Directive eq "DEVICETYPE") 
        {   
            if (@Values != 1 || !grep(/^\Q$Values[0]\E$/, (1, 2, 3)))
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "1, 2 or 3");
            }
        } 
        elsif ($Directive eq "ICMPROFILES") 
        {   
            foreach my $Value (@Values)
            {
                if ($Value !~ /.+\.ICM$/)
                {
                    # 3003 => ["The value for %s should be a file name with %s extension."]
                    AddWarning($LineNum, 3003, $Directive, ".ICM");
                }
            }
        } 
        elsif ($Directive eq "PORTSELECT") 
        {   
            if (@Values != 1 || !grep(/^\Q$Values[0]\E$/, ("NO", "MESSAGE1")))
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "NO or MESSAGE1");
            }
        } 
        elsif ($Directive eq "PROPERTYPAGES") 
        {   
            if ($Values[0] eq "ESTP2CPL.DLL" && $Values[1] eq "ENUMSTIPROPPAGES")
            {
                # 2007 => ["%s will not work as expected."]
                AddError($LineNum, 2007, "PROPERTYPAGES=ESTP2CPL.DLL,ENUMSTIPROPPAGES");
            }
            else
            {
                # 3001 => ["%s may be included it for backward compatibility, but it has no effect on Windows 2000/XP."],
                AddWarning($LineNum, 3001, $Directive);
            }
        }
        elsif ($Directive eq "SUBCLASS")
        {   
            if (@Values != 1)
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "a string");
            }
            elsif ($Values[0] eq "STILLIMAGE" && !is_defined("DDINSTALL", "DEVICETYPE"))
            {
                # 2005 => ["%s must have a corresponding %s line in the [%s] section."]
                AddError($LineNum, 2005, "SUBCLASS=STILLIMAGE", "DEVICETYPE", "INSTALL");
            }
        } 
        elsif ($Directive eq "UNINSTALLSECTION") 
        {
            # 3001 => ["%s may be included it for backward compatibility, but it has no effect on Windows 2000/XP."],
            AddWarning($LineNum, 3001, $Directive);
        }
        elsif ($Directive eq "USDCLASS") 
        {
            VerifyUSDClassLine($LineNum, $Directive, @Values);
        } 
        elsif ($Directive eq "VENDORSETUP") 
        {   
            # 3001 => ["%s may be included it for backward compatibility, but it has no effect on Windows 2000/XP."],
            AddWarning($LineNum, 3001, $Directive);
        }
        elsif ($Directive eq "WIASECTION")
        {
            if (!is_defined("ADDREG", ".*", "STI_CI.DLL\s*,\s*COINSTALLERENTRY"))
            {
                # 2005 => ["%s must have a corresponding %s line in the [%s] section."]
                AddError($LineNum, 2005, "WIASECTION", "HKR,,COINSTALLERS32,0x00010000,\"STI_CI.DLL,COINSTALLERENTRY\"", "COINSTALLERS.ADDREG");
            }
        }
    }
    
    if (!is_defined("DDINSTALL", "SUBCLASS", "STILLIMAGE"))
    {
        my $DDInstallSectionLineNum = ${@{$ModelLines{DDINSTALL}}}[0][0]-1;
        
        # 3004 => ["The [%s] section for %s should have a %s line."]
        AddWarning($DDInstallSectionLineNum, 3004, "INSTALL", "WIA/STI drivers", "SUBCLASS=STILLIMAGE");
    }
    
    return 1;
}

#-------------------------------------------------------------------------------------------------#
# VerifyAddRegistrySection - Verifies the ADDREG sections                                         #
#-- VerifyAddRegistrySection() -------------------------------------------------------------------#
sub VerifyAddRegistrySection
{
    print(STDERR "\tInvoking Image::VerifyAddRegistrySection...\n") if ($DEBUG);
    
    foreach my $Line (@{$ModelLines{ADDREG}}) 
    {
        my ($LineNum, $Directive, $RegRoot, $SubKey, $Name, $Flags, @Values) = @$Line;
        
        if ($RegRoot eq "HKR") 
        {
            if ($Name eq "CREATEFILENAME") 
            {
                if (@Values != 1 )
                {
                    # 2001 => ["The value for %s should be %s."]
                    AddError($LineNum, 2001, $Directive, "a string");
                }
            }
            elsif ($Name eq "HARDWARECONFIG") 
            {
                if ($Flags != 1 || @Values != 1 || !grep(/^\Q$Values[0]\E$/, (1, 2, 4, 8, 16)))
                {
                    # 2001 => ["The value for %s should be %s."]
                    AddError($LineNum, 2001, $Directive, "1, 2, 4, 8 or 16");
                }
            }
            elsif ($Name eq "USDCLASS") 
            {
                VerifyUSDClassLine($LineNum, $Directive, @Values);
            }
        }
    }

    return 1;
}

#-------------------------------------------------------------------------------------------------#
# VerifyDeviceDataSection - Verifies the DEVICEDATA sections                                      #
#-- VerifyDeviceDataSection() --------------------------------------------------------------------#
sub VerifyDeviceDataSection
{
    print(STDERR "\tInvoking Image::VerifyDeviceDataSection...\n") if ($DEBUG);
    
    foreach my $Line (@{$ModelLines{DEVICEDATA}}) 
    {
        my ($LineNum, $Directive, @Values) = @$Line;
        
        if ($Directive eq "ICMPROFILE") 
        {
            if (@Values != 1 || $Values[0] !~ /.+\.ICM$/)
            {
                # 3003 => ["The value for %s should be a file name with %s extension."]
                AddWarning($LineNum, 3003, $Directive, ".ICM");
            }
        } 
        elsif ($Directive eq "ISISDRIVERNAME") 
        {
            if (@Values != 1 || $Values[0] !~ /.+\.PWX$/)
            {
                # 3003 => ["The value for %s should be a file name with %s extension."]
                AddWarning($LineNum, 3003, $Directive, ".PWX");
            }
        }
        elsif ($Directive eq "MICRODRIVER") 
        {
            if (@Values != 1 || $Values[0] !~ /.+\.DLL$/)
            {
                # 3003 => ["The value for %s should be a file name with %s extension."]
                AddWarning($LineNum, 3003, $Directive, ".DLL");
            }
        }
        elsif ($Directive eq "POLLTIMEOUT") 
        {
            if (@Values != 2 || !is_numeric($Values[0]) || $Values[1] != 1)
            {
                # 2003 => ["The value for %s should be in the format %s."]
                AddError($LineNum, 2003, $Directive, "numeric, 1");
            }
        }
        elsif ($Directive eq "SERVER") 
        {
            if (@Values != 1 || $Values[0] ne "LOCAL")
            {
                # 3002 => ["The value for %s should be %s."]
                AddWarning($LineNum, 3002, $Directive, "LOCAL");
            }
        }
        elsif ($Directive eq "TWAINDS") 
        {
            if (@Values != 1)
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "the TWAIN data source name");
            }
        }
        elsif ($Directive eq "UI CLASS ID") 
        {
            if (@Values != 1 || !is_guid($Values[0]))
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "a GUID");
            }
        }
        elsif ($Directive eq "UI DLL") 
        {
            # 3001 => ["%s may be included it for backward compatibility, but it has no effect on Windows 2000/XP."],
            AddWarning($LineNum, 3001, $Directive);
        }
        elsif ($Directive eq "PREFERREDMEDIASUBTYPE") 
        {
            if (@Values != 1 || !is_guid($Values[0]))
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "a GUID");
            }
        }
        elsif ($Directive eq "PREFERREDVIDEOWIDTH") 
        {
            if (@Values != 2 || !is_numeric($Values[0]) || $Values[1] != 1)
            {
                # 2003 => ["The value for %s should be in the format %s."]
                AddError($LineNum, 2003, $Directive, "numeric, 1");
            }
        }
        elsif ($Directive eq "PREFERREDVIDEOHEIGHT") 
        {
            if (@Values != 2 || !is_numeric($Values[0]) || $Values[1] != 1)
            {
                # 2003 => ["The value for %s should be in the format %s."]
                AddError($LineNum, 2003, $Directive, "numeric, 1");
            }
        }
        elsif ($Directive eq "PREFERREDVIDEOFRAMERATE") 
        {
            if (@Values != 2 || !is_numeric($Values[0]) || $Values[1] != 1)
            {
                # 2003 => ["The value for %s should be in the format %s."]
                AddError($LineNum, 2003, $Directive, "numeric, 1");
            }
        }
        elsif ($Directive eq "EVENTCODE") 
        {
            foreach my $Value (@Values) 
            {   
                if (($Value & 0xF000) != 0xC000)
                {
                    # 2001 => ["The value for %s should be %s."]
                    AddError($LineNum, 2001, $Directive, "a list of 16-bit numeric values with highest 4 bits equal to 1100");
                }
            }
        }
        elsif ($Directive =~ /^EVENTCODE[A-F0-9]{4}$/) 
        {
            if (@Values != 1 || !is_guid($Values[0]))
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "a GUID");
            }
        }
        elsif ($Directive eq "PROPCODE") 
        {
            foreach my $Value (@Values) 
            {   
                if (($Value & 0xF000) != 0xD000)
                {
                    # 2001 => ["The value for %s should be %s."]
                    AddError($LineNum, 2001, $Directive, "a list of 16-bit numeric values with highest 4 bits equal to 1101");
                }
            }
        }
        elsif ($Directive =~ /^PROPCODE[A-F0-9]{4}$/) 
        {
            if (@Values != 2 || !is_numeric($Values[0]) || $Values[0] < 0x9802 || $Values[0] > 0x11802)
            {
                # 2003 => ["The value for %s should be in the format %s."]
                AddError($LineNum, 2003, $Directive, "WIA property code (between 0x9802 and 0x11802), description");
            }
        }
        elsif ($Directive eq "VENDOREXTID") 
        {
            if (@Values != 1 || !is_numeric($Values[0]))
            {
                # 2001 => ["The value for %s should be %s."]
                AddError($LineNum, 2001, $Directive, "numeric");
            }
        }
    }

    return 1;
}

#-------------------------------------------------------------------------------------------------#
# VerifyEventsSection - Verifies the EVENTS sections                                              #
#-- VerifyEventsSection() ------------------------------------------------------------------------#
sub VerifyEventsSection
{
    print(STDERR "\tInvoking Image::VerifyEventsSection...\n") if ($DEBUG);
    
    foreach my $Line (@{$ModelLines{EVENTS}}) 
    {
        my ($LineNum, $Directive, @Values) = @$Line;
        
        if (@Values != 3 || !is_guid($Values[1]))
        {
            # 2003 => ["The value for %s should be in the format %s."]
            AddError($LineNum, 2003, $Directive, "event name, event GUID, handler name");
        }
    }

    return 1;
}

#-------------------------------------------------------------------------------------------------#
# VerifyEventsSection - Verifies the COPYFILES sections                                           #
#-- VerifyCopyFilesSection() ---------------------------------------------------------------------#
sub VerifyCopyFilesSection
{
    print(STDERR "\tInvoking Image::VerifyCopyFilesSection...\n") if ($DEBUG);
    
    foreach my $Line (@{$ModelLines{COPYFILES}}) 
    {
        my ($LineNum, $Directive, @Values) = @$Line;
        
        if ($Values[0] eq "PTPUSB.DLL" || $Values[0] eq "PTPUSD.DLL") 
        {
            if (!is_defined("DDINSTALL", "INCLUDE", "STI\.INF") || 
                !is_defined("DDINSTALL", "NEEDS", "STI\.PTPUSBSECTION"))
            {
                # 3005 => ["Do not copy file %s directly, use %s in the [%s] section."]
                AddWarning($LineNum, 3005, $Values[0], "INCLUDE=STI.INF & NEEDS=STI.PTPUSBSECTION", "INSTALL");
            }
        }
        elsif ($Values[0] eq "SCSISCAN.SYS")
        {
            if (!is_defined("DDINSTALL", "INCLUDE", "STI\.INF") || 
                !is_defined("DDINSTALL", "NEEDS", "STI\.SCSISECTION$|^STI\.SBP2SECTION"))
            {
                # 3005 => ["Do not copy file %s directly, use %s in the [%s] section."]
                AddWarning($LineNum, 3005, $Values[0], "INCLUDE=STI.INF & NEEDS=STI.SCSISECTION or STI.SBP2SECTION", "INSTALL");
            }
            
            if (!is_defined("SERVICES", "INCLUDE", "STI\.INF") || 
                !is_defined("SERVICES", "NEEDS", "STI\.SCSISECTION\.SERVICES$|^STI\.SBP2SECTION\.SERVICES"))
            {
                # 3005 => ["Do not copy file %s directly, use %s in the [%s] section."]
                AddWarning($LineNum, 3005, $Values[0], "INCLUDE=STI.INF & NEEDS=STI.SCSISECTION.SERVICES or STI.SBP2SECTION.SERVICES", "INSTALL.SERVICES");
            }                
        }
        elsif ($Values[0] eq "SERSCAN.SYS")
        {
            if (!is_defined("DDINSTALL", "INCLUDE", "STI\.INF") || 
                !is_defined("DDINSTALL", "NEEDS", "STI\.SERIALSECTION"))
            {
                # 3005 => ["Do not copy file %s directly, use %s in the [%s] section."]
                AddWarning($LineNum, 3005, $Values[0], "INCLUDE=STI.INF & NEEDS=STI.SERIALSECTION", "INSTALL");
            }
            
            if (!is_defined("SERVICES", "INCLUDE", "STI\.INF") || 
                !is_defined("SERVICES", "NEEDS", "STI\.SERIALSECTION\.SERVICES"))
            {
                # 3005 => ["Do not copy file %s directly, use %s in the [%s] section."]
                AddWarning($LineNum, 3005, $Values[0], "INCLUDE=STI.INF & NEEDS=STI.SERIALSECTION.SERVICES", "SERVICES");
            }                
        }
        elsif ($Values[0] eq "USBSCAN.SYS")
        {
            if (!is_defined("DDINSTALL", "INCLUDE", "STI\.INF") || 
                !is_defined("DDINSTALL", "NEEDS", "STI\.USBSECTION$|^STI\.PTPUSBSECTION"))
            {
                # 3005 => ["Do not copy file %s directly, use %s in the [%s] section."]
                AddWarning($LineNum, 3005, $Values[0], "INCLUDE=STI.INF & NEEDS=STI.USBSECTION or STI.PTPUSBSECTION", "INSTALL");
            }
            
            if (!is_defined("SERVICES", "INCLUDE", "STI\.INF") || 
                !is_defined("SERVICES", "NEEDS", "STI\.USBSECTION\.SERVICES"))
            {
                # 3005 => ["Do not copy file %s directly, use %s in the [%s] section."]
                AddWarning($LineNum, 3005, $Values[0], "INCLUDE=STI.INF & NEEDS=STI.USBSECTION.SERVICES", "SERVICES");
            }                
        }
        elsif ($Values[0] eq "WIAFBDRV.DLL")
        {
            if (!is_defined("DDINSTALL", "INCLUDE", "STI\.INF") || 
                !is_defined("DDINSTALL", "NEEDS", "STI\.MICRODRIVERSECTION"))
            {
                # 3005 => ["Do not copy file %s directly, use %s in the [%s] section."]
                AddWarning($LineNum, 3005, $Values[0], "INCLUDE=STI.INF & NEEDS=STI.MICRODRIVERSECTION", "INSTALL");
            }
        }
    }
}

#-------------------------------------------------------------------------------------------------#
# VerifyUSDClassLine - Verifies the USDCLASS line                                                 #
#-- VerifyUSDClassLine() -------------------------------------------------------------------------#
sub VerifyUSDClassLine
{
    my ($LineNum, $Directive, @Values) = @_;
    
    if (@Values != 1 || !is_guid($Values[0]))
    {
        # 2001 => ["The value for %s should be %s."]
        AddError($LineNum, 2001, $Directive, "a GUID");
    }
    elsif ($Values[0] eq "{BB6CF8E2-1511-40BD-91BA-80D43C53064E}")
    {
        # Scanner micro driver
    
        if (!is_defined("DDINSTALL", "DEVICETYPE", "1"))
        {
            # 2004 => ["The [%s] section for %s should have a %s line."],
            AddError($LineNum, 2004, "INSTALL", "scanner micro driver", "DEVICETYPE=1");
        }
        
        if (!is_defined("DEVICEDATA", "MICRODRIVER"))
        {
            # 2004 => ["The [%s] section for %s should have a %s line."],
            AddError($LineNum, 2004, "DEVICEDATA", "scanner micro driver", "MICRODRIVER");
        }
    }
    elsif ($Values[0] eq "{B5EE90B0-D5C5-11D2-82D5-00C04F8EC183}")
    {
        # PTP class driver
    
        if (!is_defined("DDINSTALL", "DEVICETYPE", "2"))
        {
            # 2004 => ["The [%s] section for %s should have a %s line."],
            AddError($LineNum, 2004, "INSTALL", "PTP class driver", "DEVICETYPE=1");
        }
    
        if (!is_defined("DEVICEDATA", "VENDOREXTID"))
        {
            # 2004 => ["The [%s] section for %s should have a %s line."],
            AddError($LineNum, 2004, "DEVICEDATA", "PTP class driver", "VENDOREXTID");
        }
        
        foreach my $EventCode (is_defined("DEVICEDATA", "EVENTCODE"))
        {
            my ($LineNum, $Directive, @Values) = @{$EventCode};
            
            foreach my $Code (@Values)
            {
                my $XXXX = sprintf("%04X", $Code);
                
                if (!is_defined("DEVICEDATA", "EVENTCODE$XXXX"))
                {   
                    # 2005 => ["%s must have a corresponding %s line in the [%s] section."]
                    AddError($LineNum, 2005, "EVENTCODE=0x$XXXX", "EVENTCODE$XXXX", "DEVICEDATA");
                }
            }
        }

        foreach my $EventCodeXXXX (is_defined("DEVICEDATA", "EVENTCODE[A-F0-9]{4}"))
        {
            my ($LineNum, $Directive, @Values) = @{$EventCodeXXXX};
            
            my $XXXX = ($Directive =~ /^EVENTCODE([A-F0-9]{4})$/, $1);
            
            if (!is_defined("DEVICEDATA", "EVENTCODE", hex($XXXX)))
            {
                # 2005 => ["%s must have a corresponding %s line in the [%s] section."]
                AddError($LineNum, 2005, $Directive, "EVENTCODE=0x$XXXX", "DEVICEDATA");
            }
            
            if (!is_defined("EVENTS", "$Directive"))
            {   
                # 2005 => ["%s must have a corresponding %s line in the [%s] section."]
                AddError($LineNum, 2005, $Directive, $Directive, "EVENTS");
            }
        }
        
        foreach my $PropCode (is_defined("DEVICEDATA", "PROPCODE"))
        {
            my ($LineNum, $Directive, @Values) = @{$PropCode};
            
            foreach my $Code (@Values)
            {
                my $XXXX = sprintf("%04X", $Code);
                
                if (!is_defined("DEVICEDATA", "PROPCODE$XXXX"))
                {   
                    # 2005 => ["%s must have a corresponding %s line in the [%s] section."]
                    AddError($LineNum, 2005, "PROPCODE=0x$XXXX", "PROPCODE$XXXX", "DEVICEDATA");
                }
            }
        }
        
        foreach my $PropCodeXXXX (is_defined("DEVICEDATA", "PROPCODE[A-F0-9]{4}"))
        {
            my ($LineNum, $Directive, @Values) = @{$PropCodeXXXX};
            
            my $XXXX = ($Directive =~ /^PROPCODE([A-F0-9]{4})$/, $1);
            
            if (!is_defined("DEVICEDATA", "PROPCODE", hex($XXXX)))
            {
                # 2005 => ["%s must have a corresponding %s line in the [%s] section."]
                AddError($LineNum, 2005, $Directive, "PROPCODE=0x$XXXX", "DEVICEDATA");
            }
        }
    }
    elsif ($Values[0] eq "{0527D1D0-88C2-11D2-82C7-00C04F8EC183}")
    {    
        # WIA Video driver
    
        if (!is_defined("DDINSTALL", "DEVICETYPE", "3"))
        {
            # 2004 => ["The [%s] section for %s should have a %s line."],
            AddError($LineNum, 2004, "INSTALL", "WIA Video driver", "DEVICETYPE=3");
        }
    }
}

return 1; # Module must return 1 to let chkinf know it loaded correctly.
