#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: sedin.pl,v 1.12 2025/04/12 18:02:46 cvsuser Exp $
# sed in processing tool, processing embedded @PERL@ @PYTHON@ etc
#
# Copyright Adam Young 2017 - 2025
#
# This file is part of the Midnight Commander.
#
# The Midnight Commander is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# The Midnight Commander is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# ==end==
#

use strict;
use POSIX qw(strftime asctime);
use File::Basename;

sub usage {
        my ($msg) = shift;
        print "sedin: ${msg}\n\n" if ($msg);
        print "Usage: sedin.pl [-uvi] in out\n";
        exit 1;
}

my $mode    = "script.in";
my $outenc  = "";
my $version = "0.0.1";
my $iswin64 = undef;
my $infile  = 0;
my $verbose = 0;

sub scriptin();
sub script();
sub manin();

while (defined $ARGV[0] && ($ARGV[0] =~ /^--?[a-z]/)) {

        my $arg = $ARGV[0];

        if ($arg eq '-u' || $arg eq '--unix') {
                $outenc = ":raw :bytes";        # unix mode

        } elsif ($arg eq '-i' || $arg eq '--in') {
                ++$infile;

        } elsif ($arg eq '--script.in') {
                $mode = "script.in";

        } elsif ($arg eq '--script') {
                $mode = "script"

        } elsif ($arg eq '--man') {
                $mode = "man";

        } elsif ($arg =~ /^--version=(.*)$/) {
                $version = $1;

        } elsif ($arg =~ /^--iswin64=(.*)$/) {
                $iswin64 = $1;

        } elsif ($arg eq '-v' || $arg eq '--verbose') {
                ++$verbose;

        } else {
                usage("unknown option <$arg>");
        }
        shift @ARGV;
}

if (scalar @ARGV < 2) {                         # in/out
        usage("missing input/output files");
} elsif (scalar @ARGV > 2) {
        usage("unexpected arguments $ARGV[2] ...");
}

my $in  = $ARGV[0];
my $out = $ARGV[1];

if ($in !~ /\.in$/ && ($mode ne "script")) {
        print "sedin: input file <$in> missing '.in' extension\n";
        exit 1;
}

$out =~ s/\.in$//           # remove .in extension
        if ($infile);

printf "converting ${in} to ${out} ...\n";

if (!open (IN, "<", $in)) {                     # input
        print "sedin: cannot input open file <${in}> : $!\n";
        exit 1;
}

if (!open (OUT, ">${outenc}", $out)) {          # output
        print "sedin: cannot output file <${out}> : $!\n";
        exit 1;
}

if ($mode eq "script.in") {
        scriptin();
} elsif ($mode eq "script") {
        script();
} else {
        manin();
}

# Process script tokens
#
#       @EXTHELPERSDIR@     [runtime]
#
#       @AWK@               busybox awk
#       @GREP@              busybox grep
#       @HAVE_ZIPINFO@      0
#       @MANDOC@            mandoc
#       @MAN_FLAGS@         ""
#       @PERL@              perl
#       @PYTHON@            python
#       @RUBY@              ruby
#       @UNZIP@             busybox unzip
#       @ZIP@               busybox zip
#

sub
scriptin()
{
        my $filename = basename($out);
        my $busybox = '$(MC_BUSYBOX)';
        my $line;

        $line = <IN>;
        if ($line) {                # see: win32_utl.c
                my $extra = '';

                # line 1, #! @PERL@ || @PYTHON@
                if ($line !~ s/(\@PERL\@)/\/usr\/bin\/perl/ &&
                                $line !~ s/(\@PYTHON\@)/\/usr\/bin\/python/) {

                        if ($line =~ /\/bin\/sh/) {
                                $busybox = '';  # shell
                                $extra = <<'END'
if [ ! -z "${MC_DIRECTORY}" ]; then
    export PATH="${MC_DIRECTORY}:${PATH}"
fi
END
                        } else {
                                while ($line =~ /(\@[A-Za-z_]+\@)/g) {
                                        printf "${in} (1): warning $1\n";
                                }
                        }

                } else {
                        if ($1 eq '@PERL@') {   # perl
                                $busybox = '${ENV{MC_BUSYBOX}}';
                        } else {                # python
                                $busybox = 'os.environ.get(\'MC_BUSYBOX\')';
                        }
                }
                chomp $line;
                print OUT "${line}\n${extra}";
        }

        $busybox .= ' '                         # trailing space
                if ($busybox);

        while ($line = <IN>) {
                # other lines
                if ($verbose) {
                        pos($line) = 1;
                        while ($line =~ /(\@[A-Za-z_]+\@)/g) {
                                printf "${in} ($.): $1\n";
                        }
                }

                $line =~ s/\@PERL_FOR_BUILD\@/\/usr\/bin\/perl/g;
                $line =~ s/\@PERL\@/perl/g;
                $line =~ s/\@PYTHON\@/python/g;
                $line =~ s/\@RUBY\@/ruby/g;

                $line =~ s/\@HAVE_ZIPINFO\@/0/g;
                $line =~ s/\@AWK\@/${busybox}awk/g;
                $line =~ s/\@GREP\@/${busybox}grep/g;
                $line =~ s/\@SED\@/${busybox}sed/g;
                $line =~ s/\@UNZIP\@/${busybox}unzip/g;
                $line =~ s/\@ZIP\@/${busybox}zip/g;

                $line =~ s/\QMC_XDG_OPEN="xdg-open"\E/MC_XDG_OPEN="mcstart"/g;

                if ($filename eq 'text.sh' || $filename eq 'mc.menu' || $filename eq 'mcedit.menu') {
                        # local mandoc
                        $line =~ s/\@MANDOC\@//g;
                        $line =~ s/\@MAN_FLAGS\@//g;
                        $line =~ s/man -P cat/mcmandoc -T utf8/g;
                        $line =~ s/nroff[ ]+/mcmandoc -T utf8 /g;
                        $line =~ s/roff[ ]+/mcmandoc -T utf8 /g;
                }

                $line =~ s/\@PACKAGE\@/mcwin32/g;

                if ($line =~ /(\@[A-Za-z_]+\@)/) {
                        my $var = $1;
                        if ($var ne '@EXTHELPERSDIR@') {
                                printf "WARNING ${in} ($.): unknown variable (${var})\n";
                        }
                }

                chomp $line;
                print OUT "${line}\n";
        }

        close(IN);
        close(OUT);
}

# Process simple script
#
#       Variable            Run-time
#       MC_XDG_OPEN         start
#

sub
script()
{
        my $filename = basename($out);
        my $line;

        $line = <IN>;
        if ($line) {
                my $extra = '';
                if ($line =~ /\/bin\/sh/) {
                        $extra = <<'END'
if [ ! -z "${MC_DIRECTORY}" ]; then
    export PATH="${MC_DIRECTORY}:${PATH}"
fi
END
                }
                chomp $line;
                print OUT "${line}\n${extra}";
        }

        while ($line = <IN>) {
                # other lines
                if ($verbose) {
                        pos($line) = 1;
                        while ($line =~ /(\@[A-Za-z_]+\@)/g) {
                                printf "${in} ($.): $1\n";
                        }
                }

                $line =~ s/\QMC_XDG_OPEN="xdg-open"\E/MC_XDG_OPEN="mcstart"/g;

                if ($line =~ /(\@[A-Za-z_]+\@)/) {
                        my $var = $1;
                        if ($var ne '@EXTHELPERSDIR@') {
                                printf "WARNING ${in} ($.): unknown variable (${var})\n";
                        }
                }

                chomp $line;
                print OUT "${line}\n";
        }

        close(IN);
        close(OUT);
}

# Process man-page tokens
#

sub
manin()
{
        my $date = strftime('%B %Y', localtime);
        my $line;

        while ($line = <IN>) {
                if ($verbose) {
                        while ($line =~ /(\@[A-Za-z_]+\@)/g) {
                                printf "${in} ($.): $1\n";
                        }
                }

                $line =~ s/\%DATE_OF_MAN_PAGE\%/$date/g;
                $line =~ s/\%MAN_VERSION\%/$version/g;

#               $line =~ s/\Q%sysconfdir%\E/@sysconfdir@/g;
#               $line =~ s/\Q%libexecdir%\E/@libexecdir@/g;
#               $line =~ s/\Q%pkglibexecdir%\E/$(libexecdir)/@PACKAGE@/g;
#               $line =~ s/\Q%pkgdatadir%/$(datadir)\E/@PACKAGE@/g;

                chomp $line;
                print OUT "${line}\n";
        }

        close(IN);
        close(OUT);
}

exit 0;
