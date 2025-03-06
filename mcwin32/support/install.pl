#!/usr/bin/perl -w

BEGIN {
    my ($var) = $ENV{ "PERLINC" };

    if ( defined($var) && -d $var ) {           # import PERLINC
        my ($quoted_var) = quotemeta( $var );
        push (@INC, $var)
            if ( ! grep /^$quoted_var$/, @INC );

    } elsif ( $^O eq "MSWin32" ) {              # ActivePerl (defaults)
        if ( ! grep /\/perl\/lib/, @INC ) {
            push (@INC, "c:/perl/lib")  if ( -d "c:/perl/lib" );
            push (@INC, "/perl/lib")    if ( -d "/perl/lib" );
        } 
    }         
}

use strict;

my $VERSION;
$VERSION = sprintf "%d.%02d", q$Revision: 1.2 $ =~ /(\d+)/g;

# install.pl - Perl replacement for install(1)

# See the POD documentation a the end of this file
# or run `perl install.pl --man'
# for more information.

use Getopt::Long qw(:config bundling_override);
use Pod::Usage;

use File::Copy;
use File::Basename;
use File::Compare;
use File::Spec::Functions ':ALL';

sub makedirs;
sub install;
sub message;
sub logaction;
sub preaction;
sub postaction;
sub setmode;
sub setgroup;
sub setowner;

my %opt = (
           help =>0,
           man => 0,
           version => 0,
           directory => 0,
           backup => 0,
           B => '.old',
           force => 0,
           compare => 0,
           preserve => 0,
           mode => undef,
           owner => undef,
           group => undef,
           verbose => 0,
           log => undef,
           pre => [],
           post => [],
);

GetOptions (\%opt,
            'help|?',
	    'man',
	    'version',
            'directory|d',
	    'backup|b',
	    'B=s',
            'force|f',
            'compare|C',
            'copy_old|c',
	    'preserve|p',
	    'mode|m=s',
	    'owner|o=s',
	    'group|g=s',
	    'verbose|v',
            'log=s',
	    'pre=s',
	    'post=s',
	   ) or pod2usage(1);
pod2usage(1) if $opt{help};
pod2usage(-exitstatus => 0, -verbose => 2) if $opt{man};
print "$VERSION\n" and exit(0) if $opt{version};


my $log;
if (defined $opt{log}) {
    no strict 'refs';
    open($log, ">> $opt{log}") or die "can't open $opt{log}: $!";
};

if (defined $opt{mode}){
  my %likely = (
		777 => 1, 776 => 1, 775 => 1, 774 => 1, 766 => 1,
		755 => 1, 744 => 1, 700 => 1,
		666 => 1, 665 => 1, 664 => 1, 655 => 1, 644 => 1,
		600 => 1,
		555 => 1, 554 => 1, 544 => 1, 500 => 1,
		444 => 1, 400 => 1,
	       );

  warn "$opt{mode} seems an unlikely mode"
    unless $likely{sprintf "%d", $opt{mode}};
};

if ($opt{directory}) {
    pod2usage("$0: not enough arguments") unless @ARGV > 0;

    makedirs(@ARGV);
} elsif (@ARGV == 2 && ! -d $ARGV[1]){
    install $ARGV[0], $ARGV[1];
} else {
    pod2usage("$0: not enough arguments") unless @ARGV > 1;

    my $dest = pop @ARGV;

    pod2usage "$0: `$dest' is not a directory" unless -d $dest;

    for (@ARGV){
      my $base = basename($_);
      my $target = catfile($dest, $base);

      install $_, $target;
    };
};

sub makedirs{
    # print "makedirs called with: @_\n";
    for (@_) {
        if(-d $_){
	  postaction($_);
	  next;
	};

        #print "trying $_\n";
        if (-d dirname($_)) {
            mkdir $_ or die "can't mkdir $_: $!";
            message $_;
	    logaction "mkdir $_";
	    postaction $_;
        }else {
            makedirs(dirname($_), $_);
        };
    };
};

sub install{
  my $source = shift;
  my $target = shift;
  my $base = basename($source);
  my $old;

#  die "$source is not a file" unless -f $source;

  preaction($source);

  if ($opt{compare} and compare($source, $target) == 0){
      logaction "$source and $target are the same";
  } else {
      if ($opt{backup} && -f $target){
          my $ext = $opt{B};
          if ($ext eq 'numbered'){
              $ext = 1;
              while (-f "$target.$ext"){$ext++};
              $ext = ".$ext";
          };
          $old = "$target$ext";

          move $target => $old or die "can't move $target to $old: $!";
          message $old;
          logaction "$target => $old";
      };

if ($^O eq "MSWin32" ) {
} else {
      my ($dev, $ino) = (stat $source)[0,1];
      if (-f $target && $dev == (stat $target)[0] && $ino == (stat _)[1]){
          die "$source and $target are the same file";
      };
}

      my $copied = copy $source, $target;

      unless ($copied){
          if ($opt{force} && -f $target){
              unlink $target or die "can't unlink $target:$!";
              logaction "unlink $target";
              $copied = copy $source, $target;
          };
          if ($opt{backup}) {
              move $old => $target or die "can't move $old to $target: $!";
              message "restoring $target from $old";
              logaction "$old => $target";
          }
      };
      die "can't copy $source to $target: $!" unless $copied;

      message $target;
      logaction "$source -> $target";
  };

  # Code cribbed from
  # <http://www.perl.com/language/ppt/src/cp/cp.schumacks>
  if ($opt{preserve}){
    my($mode, $uid, $gid, $atime, $mtime) = (stat $source)[2,4,5,8,9];
    utime $atime, $mtime, $target or die "can't utime $target: $!";
    my $oldmode = (07777 & $mode);
    chmod $oldmode, $target or die "can't chmod $target: $!";
    chown $uid, $gid, $target
      or warn "can't chown $target to $mode: $!";
  };

  postaction($target);
};

sub preaction{
  my $source = shift;
  for my $pre (@{$opt{pre}}){
    system "$pre $source" and die "--pre command `$pre' failed: $!";
    logaction "$pre $source";
  };
};

sub postaction{
  my $target = shift;
  setmode($opt{mode}, $target) if (defined $opt{mode});
  setowner($opt{owner}, $target) if (defined $opt{owner});
  setgroup($opt{group}, $target) if (defined $opt{group});

  for my $post (@{$opt{post}}){
    system "$post $target" and die "--post command `$post' failed: $!";
    logaction "$post $target";
  };
};

sub setmode{
  my $mode = shift;
  $mode = oct($mode);


  for (@_){
    chmod $mode, $_ or die "can't chmod $_ to $mode: $!";
    logaction "chmod $mode, $_";
  };

};

sub setowner{
  my $owner = shift;
  my $uid;

  if ($owner =~ /^\d+$/) {
      ($uid, $owner) = ($owner, getpwuid $owner);
      die "can't getpwuid $uid: $!" unless defined $owner;
  } else {
      $uid = getpwnam $owner;
      die "can't getpwnam $owner: $!" unless defined $uid;
  }

  for (@_){
    chown $uid, -1, $_ or die "can't chown $_ to $owner: $!";
    logaction "chown $uid, -1, $_";
  };
};

sub setgroup{
  my $group = shift;
  my $gid;

  if ($group =~ /^\d+$/) {
      ($gid, $group) = ($group, getgrgid $group);
      die "can't getgrnam $gid: $!" unless defined $group;
  }else {
      $gid = getgrnam $group;
      die "can't getgrnam $group: $!" unless defined $gid;
  }

  for (@_){
    chown -1, $gid, $_ or die "can't chown $_ to $group: $!";
    logaction "chown -1, $gid, $_";
  };

};

sub message{
    print @_, "\n" if $opt{verbose};
}

sub logaction{
    no strict 'refs';
    print $log scalar gmtime, ': ', @_, "\n" if defined $opt{log};
};

__END__

=head1 NAME

install.pl - Perl replacement for install(1)

=head1 SYNOPSIS

=over

=item B<install.pl> [I<options>] I<source> I<target>

=item B<install.pl> [I<options>] I<source>... I<directory>

=item B<install.pl> B<-d> [I<options>] I<directory>...

=back

=head1 DESCRIPTION

Installs the I<source> file to the I<target> filename, each I<source>
file to the destination I<directory>, or (with B<-d>) creates each
I<directory>.  Like install(1), install.pl includes several options
primarily intended for use in makefiles.

=head2 Why use yet another install program?

Features, portability and flexibility.

An install program is really just a glorified cp(1) command with a few
convenience options.  If your install program is missing a feature
that you need, you're better off writing a program or script that
does what you want.  An install program should be ``All things to all
people''.

Once you have an install program that does exactly what you want,
you'll probably want to take it with you to new machines and operating
systems.  Many implementations of install are portable in the sense of
working on a wide variety of machines.  A perl implementation of
install has a subtle advantage of being quickly portable.  No need to
build and install your install program--just copy install.pl to a
machine with a working installation of perl.

Even the most complete install program is missing a feature important
to someone.  Obviously a perl script has a flexibility advantage.  But
install.pl also has pre- and post-installation hooks to provide
flexibility without the need to change any code.  In fact several of
the options provided by traditional install programs can by
implemented through these hooks.  (For efficiency reasons, most are
not, however.)

=head2 The three forms

Traditionally, install programs have three forms: the single file
form, the multiple file form, and the directory form.  If there are
only two arguments and the second is not a directory name, install.pl
assumes the single file form.  The I<source> file is copied to the
I<target> filename.  Use this argument if you want to specify a
different filename then the original.  For instance, the original
might have its version be part of the filename, but the installed copy
should exclude the version portion of the filename.

If the final argument is a directory name, install.pl copies each
listed I<source> file to that I<directory>.  This form is usual for
installing many or if I<target> file will have the same name as the
I<source>.  All actions (such as changing the permissions mode) are
taken on each target file.

The B<-d> option causes install.pl to assume the final, directory
form.  Each argument is created (including all components) if it does
not already exist.  Also all actions are taken on each listed
I<directory>.  Some install programs, such as this one, also perform
each action on the missing components that are created.

=head1 OPTIONS

=over

=item B<-d>, B<--directory>

Create each I<directory>.  It is functionally the same as C<mkdir -p>.

=item B<-b>, B<--backup>

Make a backup of each existing target file.

=item B<-B> I<suffix>

Specify the backup I<suffix>.  The default is C<.old>.  Usually
I<suffix> is a literal value, but the special value C<numbered> causes
the suffix to be a number.  For instance, the first time a file is
backed up, its suffix is C<.1> and the second time it's C<.2> and so
on.

=item B<-f>, B<--force>

Attempt to force each file to be copied even if the target permissions
mode wouldn't normally allow it to be changed (i.e., 0555 and
stricter).  In general, it is better to use the backup option instead
since it allows the original file to be restored.

=item B<-C>, B<--compare>

If a I<target> exists and is the same as the I<source> file, the
actual copy is skipped.  If the B<--backup> option is specified, the
backup is skipped as well.  Other actions, such as changing the mode,
are performed as usual.

=item B<-c>,

Copy the file.  This is actually the default. The -c option is 
only included for backwards compatibility.

=item B<-p>, B<--preserve>

Preserve the file attributes of the I<source> files.  Other options
may ultimately change some target file attributes.

=item B<-m> I<mode>, B<--mode> I<mode>

Set the absolute permission I<mode> of the target files and
directories.  By default the mode is not explicitly set.

=item B<-o> I<owner>, B<--owner> I<owner>

Set the I<owner> of target files and directories.

=item B<-g> I<group>, B<--group> I<group>

Set the I<group> of target files and directories.

=item B<--pre> I<command>

Specify a I<command> to be executed on each I<source> file before it
is installed.  The name of the I<source> file will be provided as an
argument to each I<command>.  This option may be repeated to perform
multiple pre-installation I<command>s.

=item B<--post> I<command>

Specify a I<command> to be executed on each I<target> file or
I<directory> after it is installed.  The name of the I<target> file or
I<directory> will be provided as an argument to each I<command>.  This
option may be repeated to perform multiple post-installation
I<command>s.

=item B<--log> I<logfile>

Append a record of actions to a I<logfile>.  The special value C<->
sends the output to STDOUT.

=item B<-?>, B<--help>

Prints the B<SYNOPSIS> and B<OPTIONS> sections.

=item B<--man>

Prints the install.pl(1) manual.

=item B<--version>

Prints the current version number of install.pl and exits.

=back

=head1 EXAMPLES

The simplest case copies one or more files into the target
I<directory>.  This example copies src/program1 and src/program2 into
the bin directory:

  install.pl src/program1 src/program2 bin

Creating a backup of the original targets makes it easier to recover
from mistakes (such as installing a buggy program).  On some systems
it can also protect running processes that are using the original
files from crashing or losing data.  (The reason is that the file
descriptor continues to point to the original even when it is
renamed.) This example moves the original targets to bin/program1~ and
bin/program2~:

  install.pl -b -B \~ src/program1 src/program2 bin

Be sure to protect the tilde from its special meaning in shell
commands!

Suppose you wanted to display the source and target file names as
programs are installed.  You could use the B<-v> option or you
can use the B<--pre> and B<--post> options:

  $ install.pl --pre printf src/program1 src/program2 bin \
    --post 'printf " -> %s\n"'
  src/program1 -> bin/program1
  src/program2 -> bin/program2

Note that options may be placed anywhere in the command line and that
multiple pre- and post-installation commands are allowed.  For
instance, this command strips the target files and prints a file
listing:

  install.pl --post strip --post 'ls -l' src/program1 src/program2 bin

A makefile install target generally looks something like:

  INSTALL := ./install.pl -m 775 -b -B .bak

  prefix := /usr/local

  bindir := $(prefix)/bin

  install: $(SCRIPTS) $(PROGRAMS)
	  $(INSTALL) $^ $(bindir)

When `make install' is executed, each script and program will be
copied to /usr/local/bin.  If the target already exists, it will be
backed up to a file ending in `.bak'.  The mode will be set to be user
and group writable, readable and executable, and world readable and
executable.

=head1 TODO

=over

=item *

Add an option to install into a tar file.

=item *

Add an option or create a new script to un-install based on an install
log file.

=item *

Add an option to create a link instead of copying.

=item *

Make the B<-m> option accept several forms of input.
(Such as allowing symbolic modes.)

=item *

Test on a wider variety of operating systems.  Please write to the
author if you'd like to help.

=item *

Make the B<--pre> and B<--post> options accept the file name anywhere
within the command, rather then only at the end.  The xargs(1) B<-i>
option is the model.

=item *

Implement some sort of locking mechanism, most likely using flock, to
allow two or mor install programs to work on the same files.

=back

=head1 BUGS

=over

=item *

install.pl is B<not> yet all things to all people.

=item *

install.pl has too many options which may interact in unexpected ways,
open security holes and confuse people who don't know the syntax.

=item *

The B<-f> option is probably unneeded.

=back

=head1 NOTES

See R. Pike and B.W. Kernighan, Program Design in the UNIX Environment
(Derived from the talk by Rob Pike, ``Cat -v considered harmful'')
I<http://www.cs.bell-labs.com/cm/cs/doc/84/kp.ps.gz> for reasons why
programs like install are at odds with the UNIX Style.  For instance,
the install target in the above example could be written like this:

  install: $(SCRIPTS) $(PROGRAMS)
          -ls $^ | xargs -i mv $(bindir)/`basename {}` \
                               $(bindir)/`basename {}`.bak
          cp $^ $(bindir)
          ls $^ | xargs -i chmod 755 $(bindir)/`basename {}`

The C<-> before the first command is needed since there isn't always a
target file.  But there are other reasons the backup might fail.  So
really there should be something like C<if [ -f $(bindir)/`basename
{}`]> added.  If the copy fails, the backup really should be restored.
Pretty soon you'd have written a shell version of install.pl.

The underlying problem in this case is that UNIX doesn't provide a
reliable way to make backed-up copies.  GNU cp(1) provides a backup
feature, but there are many other programs that probably should make
backed-up copies.  Ideally, the file system should automatically
create backups whenever a file is modified.  I believe this is a
feature of VMS, for instance.

An install program is often deployed in hostile environments.  The
B<-d> option is important on systems where C<mkdir -p> isn't
available.  Some systems don't even have a cp(1) command.  When you're
touring Europe, it's ok to rely on restaurants and markets for food,
but you better bring rations and water if you're trekking through the
desert.

On the other hand, install.pl doesn't go out of its way to make life
harder for people using it in the comforts of a UNIX environment.
Consider this rather tortured output from GNU install(1):

  $ install -vd man
  install: creating directory `man'

  $ install -v install.pl.1 man
  `install.pl.1' -> `man/install.pl.1'

  $ install -vb install.pl.1 man
  `install.pl.1' -> `man/install.pl.1' (backup: `man/install.pl.1~')

Imagine trying to use this in a pipeline!  The far more civilized
output:

  $ install.pl -vd man
  man

  $ install.pl -v install.pl.1 man
  man/install.pl.1

  $ install.pl -vb install.pl.1 man
  man/install.pl.1.old
  man/install.pl.1

=head1 HISTORY

The install utility appeared in 4.2BSD.  Originally it moved files to
their destination.  By at least 4.3BSD Reno install had a B<-c> option
to copy files instead.  (See I<http://www.freebsd.org/cgi/man.cgi> for
a wide collection of Unix man pages.)  Modern install programs no
longer move by default, but the B<-c> option has been retained for
legacy scripts and makefiles.  This version does not support the
vestigial B<-c> at all.

Every install program that I am aware of has a B<-s> option to strip
the binary of debugging information after it has been installed.
install.pl does not offer that option because I believe there is
little reason to strip binaries.  Stripping should be a strictly
private activity.  (If you I<must> strip you binaries, use the
C<--post strip> option.)

=head1 SEE ALSO

install(1), perl(1), cp(1), mv(1), chmod(1), chown(1), mkdir(1),
stat(2), File::Copy, File::Compare, File::Basename, File::Spec,
File::Spec::Functions

=head1 AUTHOR

Jon Ericson I<jericson@cpan.org>

=head1 COPYRIGHT

  Copyright 2004 by Jon Ericson.

  This program is free software; you can redistribute it and/or modify
  it under the same terms as Perl.

=begin CPAN

=head1 README

A Perl replacement for install(1).

=head1 SCRIPT CATEGORIES

UNIX/System_administration
VersionControl/CVS
Win32/Utilities

=end CPAN

=cut

#  LocalWords:  LocalWords Getopt makedirs GetOptions ARGV Kernighan bindir
#  LocalWords:  basename xargs printf chmod mkdir chown exitstatus dest sprintf
#  LocalWords:  atime mtime utime oldmode dirname logfile logaction preaction
#  LocalWords:  postaction setmode setgroup setowner elsif getpwnam getgrnam
#  LocalWords:  VersionControl TODO UIDs GIDs undef getpwuid getgrgid gmtime
#  LocalWords:  catfile STDOUT
