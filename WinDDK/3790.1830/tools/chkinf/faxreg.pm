# Copyright 1999-2000 Microsoft Corporation
######################################################################
#
#
######################################################################

package FAXREG;

my(%FaxErrorTable) = (
	6001 => ["Command string is not quoted string."],
	6002 => ["Missing command string."],
	6003 => ["Invalid value for command string.  Value should be \"19200\", \"38400\", \"57600\", \"115200\" or \"230400\"."],
	6004 => ["Invalid value for command string.  Value should be \"0\" or \"1\"."],
	6005 => ["Invalid value for command string.  Value should be \"1\", \"2\" or \"20\"."],
	6006 => ["Invalid value for command string.  Value should be \"14400\", \"12000\", \"9600\", \"7200\", \"4800\", \"2400\" or \"0\"."],
	6007 => ["\"HKR, Fax, EnableV17Send\" conflicts with \"HKR, Fax, HighestSendSpeed\"."],
	6008 => ["\"HKR, Fax, HighestSendSpeed\" conflicts with \"HKR, Fax, EnableV17Send\"."],
	6009 => ["\"HKR, Fax, HighestSendSpeed\" is slower than \"HKR, Fax, LowestSendSpeed\"."],
	6010 => ["\"HKR, Fax, LowestSendSpeed\" is faster than \"HKR, Fax, HighestSendSpeed\"."],
	6011 => ["%s does not have \"HKR, %s %s\"."],
	6012 => ["%s does not have \"HKR, %s\"."],
	6013 => ["%s does not have Fax."],
);

my(%FaxWarningTable) = (
    6501 => ["Not default value.  Default value is \"0\"."],
    6502 => ["%s does not have \"HKR, %s %s\"."],
    6503 => ["%s does not have \"HKR, Fax, %s\"."],
    6504 => ["%s does not have Fax Class %s Adaptive Answer."],
    6505 => ["Unknown or obsolete key."],
);


my(%fax_info) = ();

#-------------------------------------------------------------------------------------------------#
# This sub adds a new error to the list of errors for the file.
#
#-- AddModemError($ErrorNum, $LineNum, @ErrorArgs) -------------------------------------------------#
sub AddFaxError {
    my($ErrorNum) = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@ErrorArgs)= @_;

    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    $info_err = $FaxErrorTable{"$ErrorNum"};

    if ( !defined($info_err) ) {
        $this_str = "Unknown error $ErrorNum.";
    } else {
        $format_err = $$info_err[0];
        $this_str = sprintf($format_err, @ErrorArgs);
    }
    ChkInf::AddDeviceSpecificError($LineNum, $ErrorNum, $this_str);
}

#-------------------------------------------------------------------------------------------------#
# This sub adds a new warning to the list of warnings for the file.
#
#-- AddModemWarning($WarnNum, $LineNum, @ErrorArgs) ----------------------------------------------#
sub AddFaxWarning {
    my($WarnNum)  = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@WarnArgs) = @_;

    $WarnArgs[0] = " " if (! defined($WarnArgs[0]));
    my($str, $this_str, $info_wrn, $format_wrn);

    $info_wrn = $FaxWarningTable{"$WarnNum"};

    if ( !defined($info_wrn) ) {
        $this_str = "Unknown warning $WarnNum.";
    } else {
        $format_wrn = $$info_wrn[0];
        $this_str = sprintf($format_wrn, @WarnArgs);
    }
    ChkInf::AddDeviceSpecificWarning($LineNum, $WarnNum, $this_str);
}


######################################################################
#
# proc_faxreg_init()
#
# Initializes proc_faxreg()
#
######################################################################

sub
proc_faxreg_init
{
	%fax_info = ();
}

######################################################################
#
# IsATStringValid()
#
# Verifies the HKR key has a quoted string, the quoted string does not
# start with spaces, and the quoted string starts with "AT"
#
######################################################################

sub
IsATStringValid
{
my($HKRKey) = @_;
my($NumErrors);
my(@tmp,@tmp2,$L);

	@tmp = split(",", $HKRKey);
	@tmp2 = split(":", $tmp[2]);
# Get the line number
	$L = $tmp2[0];

# Skip if line already processed
#	if(defined($ATSyntaxErr{$L})) {
#		return;
#	}

	$NumErrors = 0;

# Get the quoted string
	if($tmp[3] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
	{
		$tmp[3] = ChkInf::QuotedString($1);

		if($tmp[3] =~ /^[\s\t]+/)
		{
#			Error: AT Command should not start with spaces.
			MODEM::AddModemWarning(3067, $L);
			$NumErrors++;
		}

		$tmp[3] =~ s/^[\s\t]*//;
		$tmp[3] =~ s/[\s\t]*$//;

		if($tmp[3] !~ /^AT/)
		{
#			Error: AT Command string should start with "AT".
			MODEM::AddModemWarning(3072, $L);
			$NumErrors++;
		}
	}
	else
	{
		AddFaxError(6001, $L);
	}

	MODEM::SetATError($L,$NumErrors);
	#$ATSyntaxErr{$L} = $NumErrors;
}

######################################################################
#
# proc_fax_adaptive()
#
# Verifies the "HKR, Fax\Class1\AdaptiveAnswer" and
# "HKR, Fax\Class2\AdaptiveAnswer" adaptive answer keys.
#
######################################################################

sub
proc_fax_adaptive
{
my($Lnum, $model, $FClass, @AdaptiveKeys) = @_;

# List of unknown keys
my(@UnknownKeys);

# Index to loop through the list
my($AdaptiveKey);

# These booleans indicate the relevant key was found
my($bModemResponseFaxDetect);
my($bModemResponseDataDetect);
my($bSerialSpeedFaxDetect);
my($bSerialSpeedDataDetect);
my($bHostCommandFaxDetect);
my($bHostCommandDataDetect);
my($bModemResponseFaxConnect);
my($bModemResponseDataConnect);

my(@tmp,@tmp2,$L);

# Initialize the list of unknown keys
	@UnknownKeys = ();

# Initialize the booleans
	$bModemResponseFaxDetect = 0;
	$bModemResponseDataDetect = 0;
	$bSerialSpeedFaxDetect = 0;
	$bSerialSpeedDataDetect = 0;
	$bHostCommandFaxDetect = 0;
	$bHostCommandDataDetect = 0;
	$bModemResponseFaxConnect = 0;
	$bModemResponseDataConnect = 0;

# Loop through each key in the list of Adaptive Answer Keys
	foreach $AdaptiveKey (@AdaptiveKeys)
	{
		@tmp = split(',', $AdaptiveKey, 4);
		@tmp2 = split(':', $tmp[2], 2);
# Get the line number
		$L = $tmp2[0];
# Get the quoted string
		if($tmp[3] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
		{
			$tmp[3] = ChkInf::QuotedString($1);

			if($tmp[1] =~ /^MODEMRESPONSEFAXDETECT$/)
			{
				$bModemResponseFaxDetect = 1;

				if($tmp[3] =~ /^$/)
				{
#					Error: "ModemResponseFaxDetect" missing command string.
					AddFaxError(6002, $L);
				}
			}
			elsif($tmp[1] =~ /^MODEMRESPONSEDATADETECT$/)
			{
				$bModemResponseDataDetect = 1;

				if($tmp[3] =~ /^$/)
				{
#					Error: "ModemResponseDataDetect" missing command string.
					AddFaxError(6002, $L);
				}
			}
			elsif($tmp[1] =~ /^SERIALSPEEDFAXDETECT$/)
			{
				$bSerialSpeedFaxDetect = 1;

				if(!(($tmp[3] =~ /^19200$/) || ($tmp[3] =~ /^38400$/) || ($tmp[3] =~ /^57600$/) || ($tmp[3] =~ /^115200$/) || ($tmp[3] =~ /^230400$/)))
				{
#					Error: "SerialSpeedFaxDetect" has invalid value for command string.  Value should be "19200", "38400", "57600", "115200" or "230400".
					AddFaxError(6003, $L);
				}
			}
			elsif($tmp[1] =~ /^SERIALSPEEDDATADETECT$/)
			{
				$bSerialSpeedDataDetect = 1;

				if(!(($tmp[3] =~ /^19200$/) || ($tmp[3] =~ /^38400$/) || ($tmp[3] =~ /^57600$/) || ($tmp[3] =~ /^115200$/) || ($tmp[3] =~ /^230400$/)))
				{
#					Error: "SerialSpeedDataDetect" has invalid value for command string.  Value should be "19200", "38400", "57600", "115200" or "230400".
					AddFaxError(6003, $L);
				}
			}
			elsif($tmp[1] =~ /^HOSTCOMMANDFAXDETECT$/)
			{
				$bHostCommandFaxDetect = 1;

				IsATStringValid($AdaptiveKey);
			}
			elsif($tmp[1] =~ /^HOSTCOMMANDDATADETECT$/)
			{
				$bHostCommandDataDetect = 1;

				IsATStringValid($AdaptiveKey);
			}
			elsif($tmp[1] =~ /^MODEMRESPONSEFAXCONNECT$/)
			{
				$bModemResponseFaxConnect = 1;

				if($tmp[3] =~ /^$/)
				{
#					Error: "ModemResponseFaxConnect" missing command string.
					AddFaxError(6002, $L);
				}
			}
			elsif($tmp[1] =~ /^MODEMRESPONSEDATACONNECT$/)
			{
				$bModemResponseDataConnect = 1;

				if($tmp[3] =~ /^$/)
				{
#					Error: "ModemResponseDataConnect" missing command string.
					AddFaxError(6002, $L);
				}
			}
			else
			{
				push(@UnknownKeys, $AdaptiveKey);
			}
		}
		else
		{
			push(@UnknownKeys, $AdaptiveKey);
		}
	}

	if(!($bModemResponseFaxDetect))
	{
#		Error: Model does not have ModemResponseFaxDetect.
		AddFaxError(6011, $Lnum, $model, $FClass, "ModemResponseFaxDetect");
	}

	if(!($bModemResponseDataDetect))
	{
#		Error: Model does not have ModemResponseDataDetect.
		AddFaxError(6011, $Lnum, $model, $FClass, "ModemResponseDataDetect");
	}

	if(!($bSerialSpeedFaxDetect))
	{
#		Warning: Model does not have SerialSpeedFaxDetect.
		AddFaxWarning(6502, $Lnum, $model, $FClass, "SerialSpeedFaxDetect");
	}

	if(!($bSerialSpeedDataDetect))
	{
#		Warning: Model does not have SerialSpeedDataDetect.
		AddFaxWarning(6502, $Lnum, $model, $FClass, "SerialSpeedDataDetect");
	}

	if(!($bHostCommandFaxDetect))
	{
#		Warning: Model does not have HostCommandFaxDetect.
		AddFaxWarning(6502, $Lnum, $model, $FClass, "HostCommandFaxDetect");
	}

	if(!($bHostCommandDataDetect))
	{
#		Warning: Model does not have HostCommandDataDetect.
		AddFaxWarning(6502, $Lnum, $model, $FClass, "HostCommandDataDetect");
	}

	if(!($bModemResponseFaxConnect))
	{
#		Error: Model does not have ModemResponseFaxConnect.
		AddFaxError(6011, $Lnum, $model, $FClass, "ModemResponseFaxConnect");
	}

	if(!($bModemResponseDataConnect))
	{
#		Error: Model does not have ModemResponseDataConnect.
		AddFaxError(6011, $Lnum, $model, $FClass, "ModemResponseDataConnect");
	}

	return @UnknownKeys;
}

######################################################################
#
# proc_fax_hierarchical()
#
# Verifies the "HKR, Fax\Class1\AdaptiveAnswer",
# "HKR, Fax\Class2\AdaptiveAnswer", "HKR, Fax\Class1",
# "HKR, Fax\Class2" and "HKR, Fax" hierarchical keys.
#
######################################################################

sub
proc_fax_hierarchical
{
my($Lnum, $model, $Hierarchical, @HierarchicalKeys) = @_;

# List of unknown keys
my(@UnknownKeys);

# Index to loop through the list
my($HierarchicalKey);

# These booleans indicate the relevant key was found
my($bExitCommand);
my($bPreAnswerCommand);
my($bPreDialCommand);
my($bResetCommand);
my($bSetupCommand);
my($bHardwareFlowControl);
my($bSerialSpeedInit);

my(@tmp,@tmp2,$L);

# Initialize the list of unknown keys
	@UnknownKeys = ();

# Initialize the booleans
	$bExitCommand = 0;
	$bPreAnswerCommand = 0;
	$bPreDialCommand = 0;
	$bResetCommand = 0;
	$bSetupCommand = 0;
	$bHardwareFlowControl = 0;
	$bSerialSpeedInit = 0;

# Loop through each key in the list of Hierarchical Keys
	foreach $HierarchicalKey (@HierarchicalKeys)
	{
		@tmp = split(',', $HierarchicalKey, 4);
		@tmp2 = split(':', $tmp[2], 2);
# Get the line number
		$L = $tmp2[0];
# Get the quoted string
		if($tmp[3] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
		{
			$tmp[3] = ChkInf::QuotedString($1);

			if($tmp[1] =~ /^EXITCOMMAND$/)
			{
				$bExitCommand = 1;

				if(!($tmp[3] =~ /^$/))
				{
					IsATStringValid($HierarchicalKey);
				}
			}
			elsif($tmp[1] =~ /^PREANSWERCOMMAND$/)
			{
				$bPreAnswerCommand = 1;

				if(!($tmp[3] =~ /^$/))
				{
					IsATStringValid($HierarchicalKey);
				}
			}
			elsif($tmp[1] =~ /^PREDIALCOMMAND$/)
			{
				$bPreDialCommand = 1;

				if(!($tmp[3] =~ /^$/))
				{
					IsATStringValid($HierarchicalKey);
				}
			}
			elsif($tmp[1] =~ /^RESETCOMMAND$/)
			{
				$bResetCommand = 1;

				if($tmp[3] =~ /^$/)
				{
#					Error: "ResetCommand" missing command string.
					AddFaxError(6002, $L);
				}
				else
				{
					IsATStringValid($HierarchicalKey);
				}
			}
			elsif($tmp[1] =~ /^SETUPCOMMAND$/)
			{
				$bSetupCommand = 1;

				if($tmp[3] =~ /^$/)
				{
#					Error: "SetupCommand" missing command string.
					AddFaxError(6002, $L);
				}
				else
				{
					IsATStringValid($HierarchicalKey);
				}
			}
			elsif($tmp[1] =~ /^HARDWAREFLOWCONTROL$/)
			{
				$bHardwareFlowControl = 1;

				if(!(($tmp[3] =~ /^0$/) || ($tmp[3] =~ /^1$/)))
				{
#					Error: "HardwareFlowControl" has invalid value for command string.  Value should be "0" or "1".
					AddFaxError(6004, $L);
				}
			}
			elsif($tmp[1] =~ /^SERIALSPEEDINIT$/)
			{
				$bSerialSpeedInit = 1;

				if(!(($tmp[3] =~ /^19200$/) || ($tmp[3] =~ /^38400$/) || ($tmp[3] =~ /^57600$/) || ($tmp[3] =~ /^115200$/) || ($tmp[3] =~ /^230400$/)))
				{
#					Error: "SerialSpeedInit" has invalid value for command string.  Value should be "19200", "38400", "57600", "115200" or "230400".
					AddFaxError(6003, $L);
				}
			}
			else
			{
				push(@UnknownKeys, $HierarchicalKey);
			}
		}
		else
		{
			push(@UnknownKeys, $HierarchicalKey);
		}
	}

	if(!($bExitCommand))
	{
#		Warning: Model does not have ExitCommand.
		AddFaxWarning(6502, $Lnum, $model, $Hierarchical, "ExitCommand");
	}

	if(!($bPreAnswerCommand))
	{
#		Warning: Model does not have PreAnswerCommand.
		AddFaxWarning(6502, $Lnum, $model, $Hierarchical, "PreAnswerCommand");
	}

	if(!($bPreDialCommand))
	{
#		Warning: Model does not have PreDialCommand.
		AddFaxWarning(6502, $Lnum, $model, $Hierarchical, "PreDialCommand");
	}

	if(!($bResetCommand))
	{
#		Warning: Model does not have ResetCommand.
		AddFaxWarning(6502, $Lnum, $model, $Hierarchical, "ResetCommand");
	}

	if(!($bSetupCommand))
	{
#		Warning: Model does not have SetupCommand.
		AddFaxWarning(6502, $Lnum, $model, $Hierarchical, "SetupCommand");
	}

	if(!($bHardwareFlowControl))
	{
#		Warning: Model does not have HardwareFlowControl.
		AddFaxWarning(6502, $Lnum, $model, $Hierarchical, "HardwareFlowControl");
	}

	if(!($bSerialSpeedInit))
	{
#		Warning: Model does not have SerialSpeedInit.
		AddFaxWarning(6502, $Lnum, $model, $Hierarchical, "SerialSpeedInit");
	}

	return @UnknownKeys;
}

######################################################################
#
# proc_fax_remaining()
#
# Verifies the "HKR, Fax" remaining keys.
#
######################################################################

sub
proc_fax_remaining
{
my($Lnum, $model, @RemainingKeys) = @_;

# List of unknown keys
my(@UnknownKeys);

# Index to loop through the list
my($RemainingKey);

# These booleans indicate the relevant key was found
my($bEnableV17Recv);
my($bEnableV17Send);
my($bFixModemClass);
my($bFixSerialSpeed);
my($bHighestSendSpeed);
my($bLowestSendSpeed);

# bV17Enabled indicates V.17 send is enabled
my($bV17Enabled);
# HighestSendSpeed is the highest send speed
my($HighestSendSpeed);
# LowestSendSpeed is the lowest send speed
my($LowestSendSpeed);

# Initialize the list of unknown keys
	@UnknownKeys = ();

# Initialize the booleans
	$bEnableV17Recv = 0;
	$bEnableV17Send = 0;
	$bFixModemClass = 0;
	$bFixSerialSpeed = 0;
	$bHighestSendSpeed = 0;
	$bLowestSendSpeed = 0;

# Initialize bV17Enabled
	$bV17Enabled = 0;
# Initialize HighestSendSpeed
	$HighestSendSpeed = 0;
# Initialize LowestSendSpeed
	$LowestSendSpeed = 0;

my(@tmp,@tmp2,$L);

# Loop through each key in the list of Remaining Keys
	foreach $RemainingKey (@RemainingKeys)
	{
		@tmp = split(',', $RemainingKey, 4);
		@tmp2 = split(':', $tmp[2], 2);
# Get the line number
		$L = $tmp2[0];
# Get the quoted string
		if($tmp[3] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
		{
			$tmp[3] = ChkInf::QuotedString($1);

			if($tmp[1] =~ /^ENABLEV17RECV$/)
			{
				$bEnableV17Recv = 1;

				if(!(($tmp[3] =~ /^0$/) || ($tmp[3] =~ /^1$/)))
				{
#					Error: "EnableV17Recv" has invalid value for command string.  Value should be "0" or "1".
					AddFaxError(6004, $L);
				}
			}
			elsif($tmp[1] =~ /^ENABLEV17SEND$/)
			{
				$bEnableV17Send = 1;

				if(!(($tmp[3] =~ /^0$/) || ($tmp[3] =~ /^1$/)))
				{
#					Error: "EnableV17Send" has invalid value for command string.  Value should be "0" or "1".
					AddFaxError(6004, $L);
				}
				elsif($tmp[3] =~ /^1$/) {
					$bV17Enabled = 1;
					if(($HighestSendSpeed) && ($HighestSendSpeed < 14400))
					{
#						Error: "HKR, Fax, EnableV17Send" conflicts with "HKR, Fax, HighestSendSpeed".
						AddFaxError(6007, $L);
					}
				}
				else
				{
					$bV17Enabled = 0;
				}
			}
			elsif($tmp[1] =~ /^FIXMODEMCLASS$/)
			{
				$bFixModemClass = 1;

				$fax_info{uc $model} = [$tmp[3]];

				if(!(($tmp[3] =~ /^1$/) || ($tmp[3] =~ /^2$/) || ($tmp[3] =~ /^20$/)))
				{
#					Error: "FixModemClass" has invalid value for command string.  Value should be "1", "2" or "20".
					AddFaxError(6005, $L);
				}
			}
			elsif($tmp[1] =~ /^FIXSERIALSPEED$/)
			{
				$bFixSerialSpeed = 1;

				if(!(($tmp[3] =~ /^19200$/) || ($tmp[3] =~ /^38400$/) || ($tmp[3] =~ /^57600$/) || ($tmp[3] =~ /^115200$/) || ($tmp[3] =~ /^230400$/)))
				{
#					Error: "FixSerialSpeed" has invalid value for command string.  Value should be "19200", "38400", "57600", "115200" or "230400".
					AddFaxError(6003, $L);
				}
			}
			elsif($tmp[1] =~ /^HIGHESTSENDSPEED$/)
			{
				$bHighestSendSpeed = 1;

				if(!(($tmp[3] =~ /^14400$/) || ($tmp[3] =~ /^12000$/) || ($tmp[3] =~ /^9600$/) || ($tmp[3] =~ /^7200$/) || ($tmp[3] =~ /^4800$/) || ($tmp[3] =~ /^2400$/) || ($tmp[3] =~ /^0$/)))
				{
#					Error: "HighestSendSpeed" has invalid value for command string.  Value should be "14400", "12000", "9600", "7200", "4800", "2400" or "0".
					AddFaxError(6006, $L);
				}

				if(!($tmp[3] =~ /^0$/))
				{
#					Warning: "HighestSendSpeed" is not default value.  Default value is "0".
					AddFaxWarning(6501, $L);
				}

				if(($tmp[3] =~ /^0$/) || ($tmp[3] =~ /^14400$/))
				{
					$HighestSendSpeed = 14400;
				}
				elsif($tmp[3] =~ /^12000$/)
				{
					$HighestSendSpeed = 12000;
				}
				elsif($tmp[3] =~ /^9600$/)
				{
					$HighestSendSpeed = 9600;
				}
				elsif($tmp[3] =~ /^7200$/)
				{
					$HighestSendSpeed = 7200;
				}
				elsif($tmp[3] =~ /^4800$/)
				{
					$HighestSendSpeed = 4800;
				}
				elsif($tmp[3] =~ /^2400$/)
				{
					$HighestSendSpeed = 2400;
				}
				if(($bV17Enabled) && ($HighestSendSpeed) && ($HighestSendSpeed < 14400))
				{
#					Error: "HKR, Fax, HighestSendSpeed" conflicts with "HKR, Fax, EnableV17Send".
					AddFaxError(6008, $L);
				}
				if(($HighestSendSpeed) && ($HighestSendSpeed < $LowestSendSpeed))
				{
#					Error: "HKR, Fax, HighestSendSpeed" is slower than "HKR, Fax, LowestSendSpeed".
					AddFaxError(6009, $L);
				}
			}
			elsif($tmp[1] =~ /^LOWESTSENDSPEED$/)
			{
				$bLowestSendSpeed = 1;

				if(!(($tmp[3] =~ /^14400$/) || ($tmp[3] =~ /^12000$/) || ($tmp[3] =~ /^9600$/) || ($tmp[3] =~ /^7200$/) || ($tmp[3] =~ /^4800$/) || ($tmp[3] =~ /^2400$/) || ($tmp[3] =~ /^0$/)))
				{
#					Error: "LowestSendSpeed" has invalid value for command string.  Value should be "14400", "12000", "9600", "7200", "4800", "2400" or "0".
					AddFaxError(6006, $L);
				}

				if(!($tmp[3] =~ /^0$/))
				{
#					Warning: "LowestSendSpeed" is not default value.  Default value is "0".
					AddFaxWarning(6501, $L);
				}

				if($tmp[3] =~ /^14400$/)
				{
					$LowestSendSpeed = 14400;
				}
				elsif($tmp[3] =~ /^12000$/)
				{
					$LowestSendSpeed = 12000;
				}
				elsif($tmp[3] =~ /^9600$/)
				{
					$LowestSendSpeed = 9600;
				}
				elsif($tmp[3] =~ /^7200$/)
				{
					$LowestSendSpeed = 7200;
				}
				elsif($tmp[3] =~ /^4800$/)
				{
					$LowestSendSpeed = 4800;
				}
				elsif($tmp[3] =~ /^2400$/)
				{
					$LowestSendSpeed = 2400;
				}
				if(($HighestSendSpeed) && ($LowestSendSpeed > $HighestSendSpeed))
				{
#					Error: "HKR, Fax, LowestSendSpeed" is faster than "HKR, Fax, HighestSendSpeed".
					AddFaxError(6010, $L);
				}
			}
			else
			{
				push(@UnknownKeys, $RemainingKey);
			}
		}
		elsif($tmp[1] =~ /^CL1FCS$/)
		{
		}
		elsif($tmp[1] =~ /^CL2DC2CHAR$/)
		{
		}
		elsif($tmp[1] =~ /^CL2LSEX$/)
		{
		}
		elsif($tmp[1] =~ /^CL2LSSR$/)
		{
		}
		elsif($tmp[1] =~ /^CL2RECVBOR$/)
		{
		}
		elsif($tmp[1] =~ /^CL2SENDBOR$/)
		{
		}
		elsif($tmp[1] =~ /^CL2SKIPCTRLQ$/)
		{
		}
		elsif($tmp[1] =~ /^CL2SWBOR$/)
		{
		}
		else
		{
			push(@UnknownKeys, $RemainingKey);
		}
	}

	if(!($bEnableV17Recv))
	{
#		Warning: Model does not have EnableV17Recv
		AddFaxWarning(6503, $Lnum, $model, "EnableV17Recv");
	}

	if(!($bEnableV17Send))
	{
#		Warning: Model does not have EnableV17Send
		AddFaxWarning(6503, $Lnum, $model, "EnableV17Send");
	}

	if(!($bFixModemClass))
	{
#		Warning: Model does not have FixModemClass
		AddFaxWarning(6503, $Lnum, $model, "FixModemClass");
	}

	if(!($bFixSerialSpeed))
	{
#		Warning: Model does not have FixSerialSpeed
		AddFaxWarning(6503, $Lnum, $model, "FixSerialSpeed");
	}

	if(!($bHighestSendSpeed))
	{
#		Warning: Model does not have HighestSendSpeed
		AddFaxWarning(6503, $Lnum, $model, "HighestSendSpeed");
	}

	if(!($bLowestSendSpeed))
	{
#		Warning: Model does not have LowestSendSpeed
		AddFaxWarning(6503, $Lnum, $model, "LowestSendSpeed");
	}

	return @UnknownKeys;
}

######################################################################
#
# proc_fax_unknown()
#
# Processes the "HKR, Fax" keys that are unknown or are missing quoted
# strings
#
######################################################################

sub
proc_fax_unknown
{
my(@UnknownKeys) = @_;

# Index to loop through the list
my($UnknownKey);
my(@tmp,@tmp2,$L);

# Loop through each key in the list of Unknown Keys
	foreach $UnknownKey (@UnknownKeys)
	{
		@tmp = split(',', $UnknownKey, 4);
		@tmp2 = split(':', $tmp[2], 2);
# Get the line number
		$L = $tmp2[0];

		if($tmp[3] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
		{
			AddFaxWarning(6505, $L);
		}
		else
		{
			AddFaxError(6001, $L);
		}
	}
}

######################################################################
#
# proc_faxreg()
#
# Verifies the "HKR, Fax" keys
#
######################################################################

sub
proc_faxreg
{
my($Lnum, $model) = @_;

# AdaptiveCommandKeys is the list of adaptive answer command keys
# AdaptiveKeys if the list of adaptive answer keys
my(@AdaptiveCommandKeys, @AdaptiveKeys);
# Index to loop through the list of adaptive answer command keys
my($AdaptiveCommandKey);
# HierarchicalKeys is the list of hierarchical keys
my(@HierarchicalKeys);
# RemainingKeys is the list of remaining keys
my(@RemainingKeys);
# UnknownKeys is the list of unknown keys
my(@UnknownKeys);
# NumKeys is the number of keys
my($NumKeys);

	@AdaptiveCommandKeys = MODEM::getPathKey("Fax\\\\Class1\\\\AdaptiveAnswer\\\\AnswerCommand,");
	@AdaptiveKeys = MODEM::getPathKey("Fax\\\\Class1\\\\AdaptiveAnswer,");
	if((($NumKeys = @AdaptiveKeys) < 1) && (($NumKeys = @AdaptiveCommandKeys) < 1))
	{
#		Warning: Model does not have Fax Class 1 Adaptive Answer.
		AddFaxWarning(6504, $Lnum, $model, "1");
	}
	elsif(($NumKeys = @AdaptiveCommandKeys) < 1)
	{
#		Error: Model does not have Fax\\Class1\\AdaptiveAnswer\\AnswerCommand
		AddFaxError(6012, $Lnum, $model, "Fax\\Class1\\AdaptiveAnswer\\AnswerCommand");
	}
	else {
		MODEM::CheckSeqCommand($model, "Fax\\Class1\\AdaptiveAnswer\\AnswerCommand", @AdaptiveCommandKeys);
		foreach $AdaptiveCommandKey (@AdaptiveCommandKeys)
		{
			IsATStringValid($AdaptiveCommandKey);
		}

		@HierarchicalKeys = proc_fax_adaptive($Lnum, $model, "Fax\\Class1\\AdaptiveAnswer,", @AdaptiveKeys);
		if(($NumKeys = @HierarchicalKeys) > 0) {
			@UnknownKeys = proc_fax_hierarchical($Lnum, $model, "Fax\\Class1\\AdaptiveAnswer,", @HierarchicalKeys);
			proc_fax_unknown(@UnknownKeys);
		}
	}

	@AdaptiveCommandKeys = MODEM::getPathKey("Fax\\\\Class2\\\\AdaptiveAnswer\\\\AnswerCommand,");
	@AdaptiveKeys = MODEM::getPathKey("Fax\\\\Class2\\\\AdaptiveAnswer,");
	if((($NumKeys = @AdaptiveKeys) < 1) && (($NumKeys = @AdaptiveCommandKeys) < 1))
	{
#		Warning: Model does not have Fax Class 2 Adaptive Answer.
		AddFaxWarning(6504, $Lnum, $model, "2");
	}
	elsif(($NumKeys = @AdaptiveCommandKeys) < 1)
	{
#		Error: Model does not have Fax\\Class2\\AdaptiveAnswer\\AnswerCommand
		AddFaxError(6012, $Lnum, $model, "Fax\\Class2\\AdaptiveAnswer\\AnswerCommand");
	}
	else {
		MODEM::CheckSeqCommand($model, "Fax\\Class2\\AdaptiveAnswer\\AnswerCommand", @AdaptiveCommandKeys);
		foreach $AdaptiveCommandKey (@AdaptiveCommandKeys)
		{
			IsATStringValid($AdaptiveCommandKey);
		}

		@HierarchicalKeys = proc_fax_adaptive($Lnum, $model, "Fax\\Class2\\AdaptiveAnswer,", @AdaptiveKeys);
		if(($NumKeys = @HierarchicalKeys) > 0) {
			@UnknownKeys = proc_fax_hierarchical($Lnum, $model, "Fax\\Class2\\AdaptiveAnswer,", @HierarchicalKeys);
			proc_fax_unknown(@UnknownKeys);
		}
	}

	@AdaptiveCommandKeys = MODEM::getPathKey("Fax\\\\Class2_0\\\\AdaptiveAnswer\\\\AnswerCommand,");
	@AdaptiveKeys = MODEM::getPathKey("Fax\\\\Class2_0\\\\AdaptiveAnswer,");
	if((($NumKeys = @AdaptiveKeys) < 1) && (($NumKeys = @AdaptiveCommandKeys) < 1))
	{
#		Warning: Model does not have Fax Class 2_0 Adaptive Answer.
		AddFaxWarning(6504, $Lnum, $model, "2_0");
	}
	elsif(($NumKeys = @AdaptiveCommandKeys) < 1)
	{
#		Error: Model does not have Fax\\Class2_0\\AdaptiveAnswer\\AnswerCommand
		AddFaxError(6012, $Lnum, $model, "Fax\\Class2_0\\AdaptiveAnswer\\AnswerCommand");
	}
	else {
		MODEM::CheckSeqCommand($model, "Fax\\Class2_0\\AdaptiveAnswer\\AnswerCommand", @AdaptiveCommandKeys);
		foreach $AdaptiveCommandKey (@AdaptiveCommandKeys)
		{
			IsATStringValid($AdaptiveCommandKey);
		}

		@HierarchicalKeys = proc_fax_adaptive($Lnum, $model, "Fax\\Class2_0\\AdaptiveAnswer,", @AdaptiveKeys);
		if(($NumKeys = @HierarchicalKeys) > 0) {
			@UnknownKeys = proc_fax_hierarchical($Lnum, $model, "Fax\\Class2_0\\AdaptiveAnswer,", @HierarchicalKeys);
			proc_fax_unknown(@UnknownKeys);
		}
	}

	@HierarchicalKeys = MODEM::getPathKey("Fax\\\\Class1,");
	if(($NumKeys = @HierarchicalKeys) > 0)
	{
		@UnknownKeys = proc_fax_hierarchical($Lnum, $model, "Fax\\Class1,", @HierarchicalKeys);
		proc_fax_unknown(@UnknownKeys);
	}

	@HierarchicalKeys = MODEM::getPathKey("Fax\\\\Class2,");
	if(($NumKeys = @HierarchicalKeys) > 0)
	{
		@UnknownKeys = proc_fax_hierarchical($Lnum, $model, "Fax\\Class2,", @HierarchicalKeys);
		proc_fax_unknown(@UnknownKeys);
	}

	@HierarchicalKeys = MODEM::getPathKey("Fax\\\\Class2_0,");
	if(($NumKeys = @HierarchicalKeys) > 0)
	{
		@UnknownKeys = proc_fax_hierarchical($Lnum, $model, "Fax\\Class2_0,", @HierarchicalKeys);
		proc_fax_unknown(@UnknownKeys);
	}

	@HierarchicalKeys = MODEM::getPathKey("Fax,");
	if(($NumKeys = @HierarchicalKeys) < 1)
	{
#		Error: Model does not have Fax
#		AddFaxError(6013, $Lnum, $model);
	}
	else
	{
		@RemainingKeys = proc_fax_hierarchical($Lnum, $model, "Fax,", @HierarchicalKeys);
		@UnknownKeys = proc_fax_remaining($Lnum, $model, @RemainingKeys);
		proc_fax_unknown(@UnknownKeys);
	}

	@UnknownKeys = MODEM::getPathKey("Fax");
	proc_fax_unknown(@UnknownKeys);
}

1;
