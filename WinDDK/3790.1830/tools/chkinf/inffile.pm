# Modules depended upon
use strict;
use InfSection;

#---------------------------------------------------------------
package InfFile; {

my(%cache);
my($UTF8) = ".\\utf8"; # Subdirectory for utf8 temp files

################################################################
#
# InfSection: Keeps track of all sections with a given name
#             across multiple inf's
#
# $INFSection = inf_file1 -> @InfData
#                   "           "
#               inf_filen -> @InfData
################################################################

# standard routines
sub FALSE        {return(0);}      # For BOOLEAN testing
sub TRUE         {return(1);}      # For BOOLEAN testing

#---------------------------------------------------------------
# Information functions - retrieve data from object
#---------------------------------------------------------------

    #---------------------------------------------------------------
    # requires scalar
    # returns scalar (-1 on error)
    sub count {
        my $self    = shift;
        my $sec     = shift;

        if ( !defined($sec) ) {
            warn("Missing required parameter \$section in call to InfFile->count\n");
            return(-1);
        } elsif ( !defined($self->{uc($sec)}) ) {
            warn("Non-existant section passed to InfFile->count\n");
            return(-1);
        } else {
            return($#{$self->{uc($sec)}});
        }
    }

    #---------------------------------------------------------------
    # returns scalar
    sub filename {
        my $self = shift;
        return($self->{inffile});
    }

    #---------------------------------------------------------------
    # returns scalar
    sub datafilename {
        my $self = shift;
        return($self->{datafile});
    }

    #---------------------------------------------------------------
    # requires scalar
    # accepts scalar scalar
    # returns scalar (-1 on error)
    sub getLine {
        my $self = shift;
        my $sec  = shift;
        my $line = shift;

        my($datum, $i);
        my($prev) = 0;

        if ( !defined($sec) ) {
            warn("Call to InfFile->getLine missing required parameter \$section.\n");
            return(-1);
        }

        if ( !defined($self->{uc($sec)}) ) {
            warn("InfFile->getLine() called with invalid section name: $sec.\n");
            return(-1);
        }

        $line = 0 if (( !defined($line) ) || ( $line !~ /^\d*$/ ));

        for ($i=0;$i<=$#{$self->{uc($sec)}};$i++) {

            if ( ( $prev+($self->{uc($sec)}->[$i]->count()) ) > $line ) {

                # In the correct subsection
                my @lines   = $self->{uc($sec)}->[$i]->lines();

                return( $lines[$line-$prev] );

            } else {
                $prev += $self->{uc($sec)}->[$i]->count();

            }
        }
        # The only way to get here is if $line is past end of last section
        return(undef);

    }

    #---------------------------------------------------------------
    # returns array
    sub getSections {
        my $self = shift;
        my $key;
        my @sections;

        foreach $key (sort keys %$self) {
            next if ($key eq "inffile");
            next if ($key eq "datafile");
            next if ($key eq "error_cnt");
            next if ($key eq "errors");
            next if ($key eq "inc_infs");
            next if ($key eq "_INCLUDED_FILES_");
            next if ($key eq "_INCLUDED_SECTIONS_");
            next if ($key eq "num_quotes");
            next if ($key eq "totallines");
            next if ($key eq "quotes");
            push(@sections, $key);
        }

        return(@sections);
    }

    #---------------------------------------------------------------
    # requires scalar scalar scalar
    # returns scalar or array depending on call (-1 on error)
    sub instanceData {
        my $self    = shift;
        my $sec     = shift;
        my $idx     = shift;
        my $item    = shift;

        if ( !defined($item) ) {
            warn("Too few parameters for InfFile->instanceData()\n");
            return(-1);
        }

        if ( !defined($sec) ) {
            warn("Missing required parameter \$section in call to InfFile->instanceData\n");
            return(-1);
        } elsif ( !defined($self->{uc($sec)}) ) {
            warn("Non-existant section passed to InfFile->instanceData\n");
            return(-1);
        }

        if ( (!defined($idx)) or ($idx !~ /^\d*/) ) {
            warn("Bad or missing value for \$index in call to InfFile->instanceData\n");
            return(-1);
        }

        SWITCH:{
            return($self->{uc($sec)}->[$idx]->count)       if (lc($item) eq "count");
            return($self->{uc($sec)}->[$idx]->fileOffset)  if (lc($item) eq "fileoffset");
            return($self->{uc($sec)}->[$idx]->infFile)     if (lc($item) eq "inffile");
            return($self->{uc($sec)}->[$idx]->lines)       if (lc($item) eq "lines");
            return($self->{uc($sec)}->[$idx]->sectionName) if (lc($item) eq "sectionname");
            warn("Invalid call to InfFile->instanceData: unknown datum.\n");
            return(-1);
        }
    }

    #---------------------------------------------------------------
    # requires scalar
    # accepts scalar scalar
    # returns scalar (-1 on error)
    sub getIndex {
        my $self = shift;
        my $sec  = shift;
        my $line = shift;

        my($datum, $i);
        my($prev) = 0;

        if ( !defined($sec) ) {
            warn("Call to InfFile->getIndex missing required parameter \$section.\n");
            return(-1);
        }

        if ( !defined($self->{uc($sec)}) ) {
            warn("InfFile->getIndex() called with invalid section name: $sec.\n");
            return(-1);
        }

        $line = 0 if (( !defined($line) ) || ( $line !~ /^\d*$/ ));

        for ($i=0;$i<=$#{$self->{uc($sec)}};$i++) {
            next if (uc($self->{uc($sec)}->[$i]->{inf}) ne
                     uc($self->{inffile}));

            if (($prev+($self->{uc($sec)}->[$i]->count())) > $line) {
                return($self->{uc($sec)}->[$i]->get_line_index($line-$prev));
            } else {
                $prev += $self->{uc($sec)}->[$i]->count();

            }
        }
        # The only way to get here is if $line is past end of last section
        return(-1);

    }


    #---------------------------------------------------------------
    # returns array
    sub lines {
        my $self = shift;

        my $datum;
        my $i;
        my(@lines)=();

        foreach $datum (sort keys %$self) {

            for ($i=0;$i<=$#{$self->{$datum}};$i++) {
                @lines=(@lines,$self->{$datum}->[$i]->lines());
            }
        }
        return(@lines);
    }

    #---------------------------------------------------------------
    # requires scalar
    # return TRUE or FALSE
    sub sectionDefined {
        my $self   = shift;
        my $sec    = shift;

        if ( !defined($sec) ) {
            warn("Call to InfFile->sectionDefined missing required parameter \$section.\n");
            return(FALSE);
        }
        if ( $self->{uc($sec)} ) {
            return(TRUE);
        } else {
            return(FALSE);
        }
    }

#---------------------------------------------------------------
# Action functions - does something to/with object
#---------------------------------------------------------------

    #---------------------------------------------------------------
    # returns TRUE on success, FALSE on error
    sub addSection {
        my $self   = shift;
        my $inf    = shift;
        my $sec    = shift;
        my $offset = shift;
        my @lines  = @_;

        if ($self->{inffile} ne uc $inf) {
                # Section is from an included INF, add it to the end of
                # the array
                # Let InfSection verify it's own parameters
                $self->{uc($sec)}->[++$#{$self->{uc($sec)}}] =
                    InfSection->new($inf, $sec, $offset, @lines)
                    ||return(FALSE);
                } else {
                # Section is from the original INF, add it to the start
                # of the array.
                unshift(@{$self->{uc($sec)}}, InfSection->new($inf, $sec, $offset, @lines));
                }

        return(TRUE);
    }

    #---------------------------------------------------------------
    # returns TRUE on success, FALSE on error
    sub addFile {

        my $self   = shift;
        my $inf    = shift;
        my $sec;
        my $str;

        # Must pass an inf name
        return(FALSE) unless ( defined($inf) );

        # Don't add the same inf twice

        return(TRUE) if (grep( /^\Q$inf\E$/i , @{$self->{_INCLUDED_FILES_}} ) );
        push(@{$self->{_INCLUDED_FILES_}}, uc($inf));

        # Cache un-cached infs
        $cache{uc($inf)} = InfFile->new(uc($inf))
            unless ( defined($cache{uc($inf)}) );

        # For each section in the new inf
        foreach $sec ($cache{uc($inf)}->getSections()) {

                # For simplicity, don't combine these sections
                next if (($sec =~ /^manufacturer$/i)
                     or  ($sec =~ /^version$/i)
                     or  ($sec =~ /^classinstall$/i)
                     or  ($sec =~ /^classinstall32$/i)
                     or  ($sec =~ /^classinstall32.nt/i)
                     );

                # Add it to the old inf
                $self->addSection($inf, $sec, -1, $cache{uc($inf)}->getSection($sec))
                    ||return(FALSE);

                $str = sprintf("%-30s from %s", "[$sec]",$inf);
                push(@{$self->{_INCLUDED_SECTIONS_}},$str);
        }

        # success!

        return(TRUE);
    }

    #---------------------------------------------------------------
    # requires scalar
    # returns array ( -1 on errors )
    sub getOrigSection {
        my $self   = shift;
        my $sec    = shift;
        my @lines  = ();
        my $i;

        if ( !defined($sec) ) {
            warn("Call to InfFile->getOrigSection() missing required parameter \$section.\n");
            return(-1);
        } elsif ( !defined($self->{uc($sec)}->[0]) ) {
            warn("InfFile->getOrigSection() called with invalid section name: $sec.\n");
            return(-1);
        }

        for ($i=0;$i<=($#{$self->{uc($sec)}}); $i++) {
            @lines=(@lines,$self->{uc($sec)}->[$i]->lines())
                unless ( uc($self->{uc($sec)}->[$i]->{inf}) ne
                         uc($self->{inffile}) );
        }

        return(@lines);
    }

    #---------------------------------------------------------------
    # requires scalar
    # return TRUE or FALSE
    sub OrigSectionDefined {
        my $self   = shift;
        my $sec    = shift;

        if ( !defined($sec) ) {
            warn("Call to InfFile->OrigDectionDefined missing required parameter \$section.\n");
            return(FALSE);
        }

        return (defined($self->{uc($sec)}->[0]));
    }

    #---------------------------------------------------------------
    # requires scalar
    # returns array ( -1 on errors )
    sub getSection {
        my $self   = shift;
        my $sec    = shift;
        my @lines  = ();
        my $i;
        my (@sec, $section, @values);
        my @AddedSections=($sec);


        if ( !defined($sec) ) {
            warn("Call to InfFile->getSection() missing required parameter \$section.\n");
            return(-1);
        } elsif ( !defined($self->{uc($sec)}->[0]) ) {
            warn("InfFile->getSection() called with invalid section name: $sec.\n");
            return(-1);
        }

        for ($i=0;$i<=($#{$self->{uc($sec)}}); $i++) {
            @lines=(@lines,$self->{uc($sec)}->[$i]->lines());
        }

        # See if any NEEDed sections should be appended.
        if (grep(/^[\s\t]*NEEDS[\s\t]*=/i,@lines)) {
            # Quick and dirty check passed, now do it right.
            foreach $i (@lines) {
                if ($i =~ /^NEEDS[\s\t]*=[\s\t]*(.*)[\s\t]*/i) {
                    push(@sec, $1);
                }
            }
        }

        foreach $section (@sec) {
            if ($section =~ /,/) {
                @values = split(/,/,$section);
            } else {
                $values[0] = $section;
            }

            foreach $i (@values) {
                $i=~s/^[\s\t]*//;
                $i=~s/[\s\t]*$//;
                if (  (grep(/^$section$/,@AddedSections)) ) {
                    next;
                }
                push(@AddedSections, $section);

                foreach (@{$self->{_INCLUDED_FILES_}}) {
                    next unless defined $cache{uc($_)};
                    if ( $cache{uc($_)}->sectionDefined($i) ) {
                        push(@{$self->{_INCLUDED_SECTIONS_}}, "$_,$section");
                        push(@lines, $cache{uc($_)}->getSection(uc($i)));
                    }
                }
            }
        }

        return(@lines);
    }

    #---------------------------------------------------------------
    # accepts handle
    # returns TRUE or FALSE
    sub print {
        my $self   = shift;
        my $handle = shift;

        $handle = \*STDOUT if ( !defined ($handle) );

        my $datum;
        my $i;

        print($handle "\t::: Section data for $self->{inffile} :::\n");
        foreach $datum (sort keys %$self) {
            next if ($datum eq "inffile");
            next if ($datum eq "datafile");
            next if ($datum eq "error_cnt");
            next if ($datum eq "errors");

            print($handle "Section: $datum\n");

            for ($i=0;$i<=$#{$self->{$datum}};$i++) {
                $self->{$datum}->[$i]->print($handle)||return(FALSE);
                print($handle "\n");
            }
        }
        print($handle "***DOH! I haven't added error table printing yet!!***\n");
        print($handle '-'x25,"\n\tTotal errors: $self->{error_cnt}\n",'-'x25,"\n");

        return(TRUE);
    }

    #---------------------------------------------------------------
    # requires scalar
    # returns blessed reference or FALSE
    sub new {
        my $proto = shift;
        my $class = ref($proto) || $proto;
        my $self  = {};

        bless($self, $class);

        if ( @_ < 0) {
            warn("Too few parameters for InfFile->new().\n");
            return($self);
        }

        $self->_initialize(@_)||return(FALSE);
        #$self->print()||die("Error printing!");
        return($self);
    }

    #---------------------------------------------------------------
    # no args
    # returns FALSE on failure
    sub CleanUpTempFiles {

        if (-e $UTF8)
        {
            my $FilesDeleted = unlink(<$UTF8\\*.inf>);
    
            # print(STDERR "Deleted $FilesDeleted files from temp directory \"$UTF8\": $!\n");
    
            if (rmdir($UTF8)) {
                return TRUE;
            }
            else {
                print(STDERR "Unable to delete temp directory \"$UTF8\": $!\n");
                return FALSE;
            }
        }
    }


#---------------------------------------------------------------
# Support functions - for internal object use
#---------------------------------------------------------------

    #-----------------------------------------------------------
    # No good reason to call this directly unless for some odd
    #   reason you need to re-point an existing InfSection object
    #   to a different InfSection.
    #-----------------------------------------------------------
    # requires scalar
    sub _initialize {
        my($self)         = shift;
        my($inf)          = shift;
        my $infdir        = substr($inf,0,(rindex($inf,"\\")));

        # INF must exist
        if ( (!defined($inf)) || (! -e $inf) ) {
            warn("InfFile->_initialize called with bad or non-existant inf\n");
            return(FALSE);
        }

        my (# Track the line we're working with
            $prev_line, $line,      $CLnum,
            # Track the current section
            $sec_start, $sec_end,   $csec,
            # Track output
            $TotalLines, @INFlines,
            # Used to find and remove comments
            $posquote1, $posquote2, $poscomma,  $bRemoveComment,
            # Odds-and-ends that need done
            %child,     %needs,      @values,   $section, $file
        );

        # Convert Unicode INF's to UTF8 - handles primary INF and include/needs scenarios
        #
        my $DataFile = ConvertIfUnicodeINF($inf);  # returns $inf if not UNICODE, else the name of the converted temp file

        local *INFILE;

        # Open the file the object is to be created from
        open (INFILE, "<$DataFile")    || die "Can't open $inf (actually: $DataFile) for input: $!\n";

        # Line before first can't contain anything
        $prev_line = "";

        $inf = uc($inf); # so we don't have to do everywhere it is used (but must be after DataFile name is created)

        # Initialize object data
        $self->{inffile}    = $inf;
        $self->{datafile}   = $DataFile;
        $self->{error_cnt}  = 0;
        $self->{errors}     = {};     # no errors before creation
        $self->{totallines} = 0; # remember how many lines came from the original INF
        $self->{inc_infs}   = {};
        $self->{_INCLUDED_FILES_} = [$inf];
        $self->{_INCLUDED_SECTIONS_} = [];

        # Set starting values
        $sec_start=-1;
        $sec_end  = 0;
        $CLnum=0;

        # The real workhorse
        foreach (<INFILE>) {
            $self->{totallines}++;
            # increment the current line number
            $CLnum++;

            # remove trailing carriage returns
            chop if /\n$/;

            # Concat with any data left from previous line
            if ($_ !~ /^\s*\[.*\]\s*$/) {
                $line = join("", $prev_line, $_);
                $prev_line = "";
            } else {
                $line = $_;
                $INFlines[$CLnum-1] = $prev_line unless ($prev_line eq "");
                $prev_line = "";
            }

            #process for comments.
            $posquote1  = index($line, '"');
            $posquote2  = ($posquote1 >= 0) ? index($line, '"', $posquote1+1) : -1;
            $poscomma   = index($line, ';');
            $bRemoveComment = 0;

            # Find first unquoted ; and remove from it to EOL
            while ($poscomma >= 0) {
                if ($poscomma < $posquote1 || $posquote1 < 0) {  # ';' before '"' => is comment
                    $bRemoveComment = 1;
                    last;
                }

                if ($posquote2 >= 0 && $posquote1 >= 0) {
                    # ';' between '"' => string first
                    if ($poscomma < $posquote2) {
                        $poscomma = index($line, ';', $posquote2 + 1);
                    }
                    # find next string
                    $posquote1  = index($line, '"', $posquote2 + 1);
                    $posquote2  = ($posquote1 >= 0) ? index($line, '"', $posquote1+1) : -1;
                    next;
                }
                # ';' after '"' => sting not terminated yet, string first
                last;
            }

            # remove the comment from the line
            if ($bRemoveComment == 1) {
                $line = substr($line, 0, $poscomma);
            }

            $line =~ s/^[\s\t]*//; #strip leading white spaces.
            $line =~ s/[\s\t]*$//; #strip ending white spaces and eol.

            # if line ends in \ it a multiline statement save current and go to next
            if ($line =~ /(.)*\\$/) {
                $line =~ s/\\$//;
                $prev_line = $line;
                # Consider current line blank
                $line = " ";
            }

            # Make sure line is defined and recorded before doing the rest
            $line = " " if (! defined $line);
            $INFlines[$CLnum] = $line;

            # Create new InfSection if necessary
            if ($line =~ /^\[(.*)\]$/) {
                if ($sec_start > -1) {
                    $sec_end = $CLnum - 1;

                    $self->addSection($inf, $csec, $sec_start, @INFlines[$sec_start..$sec_end]);

                    $sec_start = $CLnum+1;
                    $csec=$1;
                } else {
                    # This must be the first section
                    $sec_start = $CLnum+1;
                    $csec=$1;
                }

            # Track all sections to be imported from other INFs
            } elsif ($line =~ /^[\s\t]*NEEDS[\s\t]*=.*/i) {
                # Create a list of all sections NEEDed for later parsing
                $line =~ s/^[\s\t]*NEEDS.*=//i;
                if ($line =~ /,/) {
                    @values = split(/,/,$line);
                } else {
                    $values[0] = $line;
                }
                foreach (@values) {
                    s/^[\s\t]*//;
                    s/[\s\t]*$//;
                    $needs{$_} = [$CLnum,$csec];
                }
                @values=();

            # If we find an INCLUDE line, track the needed infs locally, but keep the actual
            #   inf objects in the persistent cache
            } elsif ($line =~ /^[\s\t]*INCLUDE[\s\t]*=.*/i) {
                # Create a new InfFile for parsing for NEEDS
                $line =~ s/^[\s\t]*INCLUDE.*=//i;
                $line =~ s/^[\s\t]*//;
                $line =~ s/[\s\t]*$//;
                if ($line =~ /,/) {
                    @values = split(/,/,$line);
                } else {
                    $values[0] = $line;
                }

                foreach (@values) {
                    s/^[\s\t]*//;
                    s/[\s\t]*$//;
                    next if (uc($_) eq uc($inf));
                    # INCLUDEd inf order = current dir, parent inf dir, %windir%\inf
                    if ( -e "$_") {
                        $cache{uc($_)} = InfFile->new(uc($_)) unless ( defined($cache{uc($_)}) );
                        $child{uc($_)}     = 1;
                        $self->addFile($_);
                    } elsif (-e "$infdir\\$_") {
                        $cache{uc("$infdir\\$_")} = InfFile->new("$infdir\\$_")
                            unless ( defined($cache{uc("$infdir\\$_")}) );
                        $child{uc($_)}     = 1;
                        $self->addFile("$infdir\\$_");
                    } elsif (-e "$ENV{WINDIR}\\inf\\$_") {
                        $cache{uc("$ENV{WINDIR}\\inf\\$_")} = InfFile->new("$ENV{WINDIR}\\inf\\$_")
                            unless ( defined($cache{uc("$ENV{WINDIR}\\inf\\$_")}) );
                        $child{uc($_)}     = 1;
                        $self->addFile("$ENV{WINDIR}\\inf\\$_");
                    } else {
#                        warn("\tNot reading $_: file not found.\n");
                    }

                }
                @values=();

            }
        } # end foreach $line

        # Handle the last section
        $self->addSection($inf, $csec, $sec_start, @INFlines[$sec_start..($self->{totallines})]);
        $TotalLines = $.;
        close(INFILE);

        #$FileSize = -s INFILE;
        #$self->AddErrorRef(1034, $TotalLines) if ($FileSize > 62*1024);

       return(TRUE);
    } # END INITIALIZE

    #-------------------------------------------------------------------------------------------------#
    # Convert INF from UNICODE to UTF8 (if necessary).  Returns filename containing converted data,
    #   or the original filename if no conversion necessary.
    #-- ConvertIfUnicodeINF($InfFile) ----------------------------------------------------------------#
    sub ConvertIfUnicodeINF
    {
        my($InfFile)   = $_[0];

        my $BomCheck;
        local *INFILE;

        # Open the file the object is to be created from
        unless (open (INFILE, "<$InfFile"))
        {
            print(STDERR "ERROR: Cannot open '$InfFile': $!\n");
            return $InfFile;
        }
        binmode INFILE;

        my $DataFile = $InfFile;

        #
        # reading 3 chars is fine here as a valid INF file must be >> 3 chars
        #
        if (defined(read(INFILE, $BomCheck, 3)) and length($BomCheck)==3)
        {
            my @data = unpack("CCC", $BomCheck);

            #
            # Todo - Unicode INF's without BOM
            # however Unicode INF's should always have BOM
            #

            if (($data[0] == 0xff and $data[1] == 0xfe) or
                ($data[0] == 0xfe and $data[1] == 0xff) or
                ($data[0] == 0xef and $data[1] == 0xbb and $data[2] == 0xbf))
            {

                # need 'utf8' work file
                mkdir($UTF8, 0777);

                $DataFile = main::FlattenINFFileName($InfFile, "$UTF8\\");

                unless(    (($data[0] == 0xff) and CopyUnicodeToUtf8(      $InfFile, $DataFile, 2))
                        or (($data[0] == 0xfe) and CopyUnicodeBigEndToUtf8($InfFile, $DataFile, 2))
                        or (($data[0] == 0xef) and CopyUtf8ToUtf8(         $InfFile, $DataFile, 3)))
                {
                    print(STDERR "ERROR: Cannot create UTF8 '$DataFile' from '$InfFile': $!\n");
                    if ($DataFile)
                    {
                        unlink($DataFile);
                    }
                    $DataFile = $InfFile;
                }
            }
        }

        close(INFILE);

        return $DataFile;
    }

    #-------------------------------------------------------------------------------------------------#
    # Helper functions to deal with Unicode INF's
    #-- CopyUnicodeToUtf8($InfFile,$Utf8File,$Skip) --------------------------------------------------#
    sub CopyUnicodeToUtf8 {
        my($InfFile)  = $_[0];
        my($Utf8File) = $_[1];
        my($Skip)     = $_[2];

        unless(open(Hin,"<$InfFile")) {
            return 0;
        }

        unless(open(Hout,">$Utf8File")) {
            close(Hin);
            return 0;
        }
        binmode(Hin);
        binmode(Hout);
        seek Hin,$Skip,0;

        my($chunk);
        my($len);

        # DO NOT write UTF8 header bytes!!!!

        while(defined($len = read(Hin,$chunk,1024)) and $len>=2) {
            #
            # extract into unicode words
            #
            my(@unichunk) = unpack("S*",$chunk);

            my($chr);

            foreach $chr (@unichunk) {
                if($chr<0x80) {
                    print Hout pack("C",$chr);
                } elsif($chr<0x800) {
                    print Hout pack("CC",(($chr>>6)&0x1F)|0xC0,($chr&0x3F)|0x80);
                } else {
                    print Hout pack("CCC",(($chr>>12)&0x0F)|0xE0,(($chr>>6)&0x3F)|0x80,($chr&0x3F)|0x80);
                }
            }
        }
        close(Hin);
        close(Hout);
        return 1;

    } # CopyUnicodeToUtf8()

    #-------------------------------------------------------------------------------------------------#
    # Helper functions to deal with Unicode INF's
    #-- CopyUnicodeBigEndToUtf8($InfFile,$Utf8File,$Skip) --------------------------------------------#
    sub CopyUnicodeBigEndToUtf8 {
        my($InfFile)     =   $_[0];
        my($Utf8File)    =   $_[1];
        my($Skip)        =   $_[2];

        unless(open(Hin,"<$InfFile")) {
            return 0;
        }

        unless(open(Hout,">$Utf8File")) {
            close(Hin);
            return 0;
        }
        binmode(Hin);
        binmode(Hout);
        seek Hin,$Skip,0;

        my($chunk);
        my($len);

        # DO NOT write UTF8 header bytes!!!!

        while(defined($len = read(Hin,$chunk,1024)) and $len>=2) {
            #
            # extract into unicode words
            #
            my(@unichunk) = unpack("S*",$chunk);

            my($swpchr);
            my($chr);

            foreach $swpchr (@unichunk) {
                $chr = (($swpchr>>8)&0xff)|(($swpchr&0xff)<<8);
                if($chr<0x80) {
                    print Hout pack("C",$chr);
                } elsif($chr<0x800) {
                    print Hout pack("CC",(($chr>>6)&0x1F)|0xC0,($chr&0x3F)|0x80);
                } else {
                    print Hout pack("CCC",(($chr>>12)&0x0F)|0xE0,(($chr>>6)&0x3F)|0x80,($chr&0x3F)|0x80);
                }
            }
        }
        close(Hin);
        close(Hout);
        return 1;

    } # CopyUnicodeBigEndToUtf8()

    #-------------------------------------------------------------------------------------------------#
    # Helper functions to deal with Unicode INF's
    #-- CopyUtf8ToUtf8($InfFile,$Utf8File,$Skip) -----------------------------------------------------#
    sub CopyUtf8ToUtf8 {
        my($InfFile)     =   $_[0];
        my($Utf8File)    =   $_[1];
        my($Skip)        =   $_[2];

        unless(open(Hin,"<$InfFile")) {
            return 0;
        }

        unless(open(Hout,">$Utf8File")) {
            close(Hin);
            return 0;
        }
        binmode(Hin);
        binmode(Hout);
        seek Hin,$Skip,0;

        my($chunk);
        my($len);

        while(defined($len = read(Hin,$chunk,1024)) and $len>=2) {
            print Hout $chunk;
        }
        close(Hin);
        close(Hout);

        print(STDERR "UTF8 files not supported by SetupAPI at this time\n");
        return 1;

    } # CopyUtf8ToUtf8()

    #---------------------------------------------------------------
    # perl defined routine - invoked when object is destroyed
    #sub DESTROY {
    #    #my $self = shift;
    #    #print("<-- $self->{inffile} leaving scope.\n");
    #}

}
1; # Keep perl happy

__END__
