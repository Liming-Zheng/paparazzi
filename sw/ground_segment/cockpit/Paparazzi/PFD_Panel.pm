# The zone above the artificial horizon which gives informations on ap mode, GPS etc..
#=============================================================================
package Paparazzi::PFD_Panel;
use Subject;
@ISA = ("Subject");

use Tk; 
use Tk::Zinc;
use Data::Dumper;

use strict;
sub populate {
  my ($self, $args) = @_;
  
  $self->SUPER::populate($args);
  $self->configspec(-zinc    => [S_NEEDINIT, S_PASSIVE, S_RDONLY, S_OVRWRT, S_NOPRPG, undef],
		    -parent_grp  => [S_NEEDINIT, S_PASSIVE, S_RDONLY, S_OVRWRT, S_NOPRPG, undef],
		    -origin  => [S_NEEDINIT, S_PASSIVE, S_RDONLY, S_OVRWRT, S_NOPRPG, undef],
		    -width   => [S_NEEDINIT, S_PASSIVE, S_RDONLY, S_OVRWRT, S_NOPRPG, undef],
		    -height  => [S_NEEDINIT, S_PASSIVE, S_RDONLY, S_OVRWRT, S_NOPRPG, undef],
		    -gps_mode => [S_NOINIT,   S_METHOD,  S_RDWR,   S_OVRWRT, S_NOPRPG, 0.],
		    -ap_mode  => [S_NOINIT,   S_METHOD,  S_RDWR,   S_OVRWRT, S_NOPRPG, 0.],
		    -rc_mode  => [S_NOINIT,   S_METHOD,  S_RDWR,   S_OVRWRT, S_NOPRPG, 0.],
		    -ctrst_mode => [S_NOINIT,   S_METHOD,  S_RDWR,   S_OVRWRT, S_NOPRPG, 0.],
		    -ctrst_value => [S_NOINIT,   S_METHOD,  S_RDWR,   S_OVRWRT, S_NOPRPG, 0.],
		    -lls_mode => [S_NOINIT,   S_METHOD,  S_RDWR,   S_OVRWRT, S_NOPRPG, 0.],
		    -lls_value => [S_NOINIT,   S_METHOD,  S_RDWR,   S_OVRWRT, S_NOPRPG, 0.],
		    -if_mode => [S_NOINIT,   S_METHOD,  S_RDWR,   S_OVRWRT, S_NOPRPG, 0.],
#		    -pubevts => [S_NEEDINIT, S_PASSIVE, S_RDWR, S_APPEND, S_NOPRPG,[]]	   
		   );
}

sub completeinit {
  my $self = shift;
  $self->SUPER::completeinit();
  $self->{modes} = [ { name => 'rc',
		       str =>  ["lost","ok", "really lost", "not possible"],
		       color => ["orange", "green", "red", "red"]
		     },
		     { name => 'cal',
		       str =>  ["unkwn", "wait", "ok"],
		       color =>["red",  "orange", "green"]
		     },
		     { name => 'ap',
		       str =>  ["manual", "auto1", "auto2", "home"],
		       color =>["green",  "green", "green", "orange"]
		     },
		     { name => 'gps',
		       str => [ "No fix",
				"dead reckoning only",
				"2D-fix",
				"3D-fix",
				"GPS + dead reckoning combined"],
		       color => ["red", "red", "orange", "green", "orange"]
		     },
		     { name => 'lls',
		       str =>  ["OFF"  , "ON"],
		       color =>["orange", "green"]
		     },
		     { name => 'if',
		       str =>  ["none", "down", "up"],
		       color =>["green", "orange", "orange"]
		     }
		   ];
  $self->{modes_by_name} = {};
  foreach my $mode (@{$self->{modes}}) {
    $self->{modes_by_name}->{$mode->{name}} = $mode;
  }
  $self->build_gui();
  $self->configure('-pubevts' => 'CLICKED');
}


sub build_gui {
  my ($self) = @_;

  my $zinc = $self->get('-zinc');
  my $parent_grp = $self->get('-parent_grp');
  $self->{main_group} = $zinc->add('group', $parent_grp, -visible => 1);
  my @origin = $self->get('-origin');
  $zinc->coords($self->{main_group}, \@origin);


  my $modes = $self->{modes};

  my $nb_col = $#$modes+1;

#  print "nb_col $nb_col\n";

  my $w = $self->get('-width');
  my $h = $self->get('-height');
  my $i;
  for  ($i=1; $i<$nb_col; $i++) {
    my $x =  $i / $nb_col * $w;
    $zinc->add('curve', $self->{main_group},
	       [$x, 10, $x, $h +10],
	       -linewidth => 2,
	       -linecolor => 'white',
	       -filled => 0);
  }

  my $f = '-adobe-helvetica-bold-o-normal--16-240-100-100-p-182-iso8859-1';
  my $labelformat = "100x200 x80x20+20+10 x80x20^0>0 x80x20^0>1";
  my @tab_args = ('tabular',$self->{main_group}, 3,
		  -labelformat => $labelformat,);
  my @tab_style = (  -font => $f,
		      -color => 'green');
  my $x=-10;
  my $dx =  $self->get('-width') / $nb_col;
  foreach my $mode (@{$self->{modes}}) {
    $mode->{tabular} = $zinc->add(@tab_args, -position => [$x, -5] );
    $zinc->itemconfigure ($mode->{tabular}, 0, @tab_style, -text => uc $mode->{name});
    $zinc->bind($mode->{tabular},'<ButtonPress-1>'=> [\&onRectClicked, $self, $mode->{name}]);
    $x += $dx;
  }


  $self->{rect_group} = $zinc->add('group', $self->{main_group}, -visible => 1);
  my $rect = $zinc->add('rectangle',  $self->{main_group}, [0, 0, $w, $h],
			-visible => 0, 
			-linecolor => 'white',
			-filled => '1',
		       );
#  $zinc->bind($rect,'<ButtonPress-1>'=> [\&onRectClicked, $self, "coucou"]);

}



sub onRectClicked {
  my ($zinc, $self, $name) = @_;
  print "onRectClicked : $name\n";
  $self->notify('CLICKED', $name);
}



sub set_mode {
  my ($self, $name, $previous_val, $new_val) = @_;
  my $mode = $self->{modes_by_name}->{$name};
  if (defined $mode) {
    if (!defined $previous_val || $previous_val != $new_val) {
      my $zinc = $self->get('-zinc');
      $zinc->itemconfigure( $mode->{tabular}, 1,
			   -text => $mode->{str}[$new_val],
			   -color =>$mode->{color}[$new_val],
			  );
    }
  }
}

sub gps_mode() {
  my ($self, $previous_mode, $new_mode) = @_;
  $self->set_mode("gps", $previous_mode, $new_mode); 

}

sub ap_mode() {
  my ($self, $previous_mode, $new_mode) = @_;
  $self->set_mode("ap", $previous_mode, $new_mode);
}

sub rc_mode {
  my ($self, $previous_mode, $new_mode) = @_;
  $self->set_mode("rc", $previous_mode, $new_mode);
}

sub lls_mode {
  my ($self, $previous_mode, $new_mode) = @_;
  $self->set_mode("lls", $previous_mode, $new_mode);
}

sub if_mode {
  my ($self, $previous_mode, $new_mode) = @_;
  $self->set_mode("if", $previous_mode, $new_mode); 
}

sub lls_value {
  my ($self, $previous_val, $new_val) = @_;
  my $mode = $self->{modes_by_name}->{lls};
  if (defined $mode) {
    if (!defined $previous_val || $previous_val != $new_val) {
      my $zinc = $self->get('-zinc');
      my $str_val = sprintf ("%.4f", $new_val);
      $zinc->itemconfigure( $mode->{tabular}, 2,
			    -text => $str_val,
			    -color => "green",
			  );
    }
  }
}



sub ctrst_mode {
  my ($self, $previous_mode, $new_mode) = @_;
  $self->set_mode("ctrst", $previous_mode, $new_mode);
}

sub ctrst_value {
  my ($self, $previous_val, $new_val) = @_;
  my $mode = $self->{modes_by_name}->{ctrst};
  if (defined $mode) {
    if (!defined $previous_val || $previous_val != $new_val) {
      my $zinc = $self->get('-zinc');
      my $str_val = sprintf ("%.4f", $new_val);
      $zinc->itemconfigure( $mode->{tabular}, 2,
			    -text => $str_val,
			    -color => "green",
			  );
    }
  }
}


sub setLabelContent {
  my ($self, $item, $labelformat) = @_;

  my @fieldsSpec = split (/ / , $labelformat);
  shift @fieldsSpec;

  my $i=0;
  foreach my $fieldSpec (@fieldsSpec) {
    my ($posSpec) = $fieldSpec =~ /^.\d+.\d+(.*)/ ;
    print "$fieldSpec\t$i\t$posSpec\n";
    $self->{zinc}->itemconfigure ($item,$i,
				  -text => "$i: $posSpec",
				  -border => "contour",
				  -color => 'green',
				  -backcolor => 'white',
				  -filled => 1
				 );
    $i++;
  }
}

1;
