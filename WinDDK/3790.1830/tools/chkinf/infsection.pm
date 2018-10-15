# Modulels depended upon
use strict;

#---------------------------------------------------------------
package InfSection; {

################################################################
# InfSection: per section data for InfFile.pm
#
# INFData = inf_file
#           section_name
#           offset_in_file,
#           lines_in_section,
#           [  line[0]
#               " "
#               " "
#              line[n]
#           ]
################################################################

# standard routines
sub FALSE        {return(0);}      # For BOOLEAN testing
sub TRUE         {return(1);}      # For BOOLEAN testing

#---------------------------------------------------------------
# Information functions - retrieve data from object
#---------------------------------------------------------------

    #---------------------------------------------------------------
    # returns scalar
    sub count {
        my $self = shift;
        return($self->{count});
    }

    #---------------------------------------------------------------
    # returns scalar
    sub fileOffset {
        my $self = shift;
        return($self->{offset});
    }

    #---------------------------------------------------------------
    # accepts scalar
    # returns scalar
    sub get_line_index {
        my $self = shift;
        my $num  = shift;

        $num = 0 if (( !defined($num) ) || ( $num !~ /^\d*$/ ));
        return($self->{offset} + $num);
    }

    #---------------------------------------------------------------
    # returns scalar
    sub infFile {
        my $self = shift;
        return($self->{inf});
    }

    #---------------------------------------------------------------
    # returns array
    sub lines {
        my $self = shift;
        return(@{$self->{lines}});
    }

    #---------------------------------------------------------------
    # returns scalar
    sub sectionName {
        my $self = shift;
        return($self->{name});
    }

#---------------------------------------------------------------
# Action functions - does something to/with object
#---------------------------------------------------------------

    #-----------------------------------------------------------
    # requires scalar, scalar, scalar, array
    # returns blessed reference or FALSE
    sub new {
        my $proto = shift;
        my $class = ref($proto) || $proto;
        my $self  = {};
        bless($self, $class);

        if ( @_ < 3) {
            warn("Too few parameters for InfSection->new().\n");
            return($self);
        }

        # Allow a negative index for included files
        if ( $_[2] !~ /^-?\d*$/ ) {
            warn("InfSection->_initialize() expected parameter 3 to be numeric.\n");
            return($self);
        }

        $self->_initialize(@_)||return(FALSE);

        return $self;
    }

    #---------------------------------------------------------------
    # accepts optional filehandle reference
    # returns TRUE
    sub print {
        my $self   = shift;
        my $handle = shift;

        $handle = \*STDOUT if (( !defined ($handle) ) || ( ref($handle) ne "GLOB" ));

        print($handle "\tinffile: $self->{'inf'}\n");
        print($handle "\tsection: $self->{'name'}\n");
        print($handle "\toffset:  $self->{'offset'}\n");
        print($handle "\tcount:   $self->{'count'}\n");
        foreach (@{$self->{'lines'}}) {
            print($handle "\n"),next unless defined($_);
            print($handle "\t\t",$_,"\n");
        }
        return(TRUE);
    }

#---------------------------------------------------------------
# Support functions - for internal object use
#---------------------------------------------------------------

    #-----------------------------------------------------------
    # No good reason to call this directly unless for some odd
    #   reason you need to re-point an existing InfSection object
    #   to a different InfSection.
    #-----------------------------------------------------------
    # requires scalar scalar scalar array
    # returns TRUE or FALSE
    sub _initialize {
        my $self = shift;

        if ( @_ < 3) {
            warn("Too few parameters for InfSection->_initialize().\n");
            return(FALSE);
        }

        # Allow a negative index for included files
        if ( $_[2] !~ /^-?\d*$/ ) {
            warn("InfSection->_initialize() expected parameter 3 to be numeric.\n");
            return(FALSE);
        }

        # Initialize our data
        $self->{"inf"}    = shift;
        $self->{"name"}   = shift;
        $self->{"offset"} = shift;
        $self->{"count"}  = @_;
        $self->{"lines"}  = [@_];

        return(TRUE);
    }

    #---------------------------------------------------------------
    # perl defined routine - invoked when object is destroyed
    # (does nothing - add code if necessary)
    sub DESTROY {
        my $self = shift;
    }

}
1;
