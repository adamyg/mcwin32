#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: sedin.pl,v 1.4 2020/05/02 22:33:19 cvsuser Exp $
# sed in processing tool, processing embedded @PERL@ @PYTHON@ etc
#
# Copyright Adam Young 2017 - 2020
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

sub usage {
        my ($msg) = shift;
        print "sedin: ${msg}\n\n" if ($msg);
        print "Usage: sedin.pl [-uvi] in out\n";
        exit 1;
}

my $outenc  = "";
my $infile  = 0;
my $verbose = 0;

while (defined $ARGV[0] && ($ARGV[0] =~ /^--?[a-z]+$/)) {
        if ($ARGV[0] eq '-u' || $ARGV[0] eq '--unix') {
                $outenc = ":raw :bytes";        # unix mode

        } elsif ($ARGV[0] eq '-i' || $ARGV[0] eq '--in') {
                ++$infile;

        } elsif ($ARGV[0] eq '-v' || $ARGV[0] eq '--verbose') {
                ++$verbose;

        } else {
                usage("unknown option <$ARGV[0]>");
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

if ($in !~ /\.in$/) {
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


# Process tokens
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

my $busybox = '$(MC_BUSYBOX)';
my $line;

$line = <IN>;
if ($line) {                # see: win32_utl.c
        # line 1, #! @PERL@ || @PYTHON@
        if ($line !~ s/(\@PERL\@)/\/usr\/bin\/perl/ &&
                        $line !~ s/(\@PYTHON\@)/\/usr\/bin\/python/) {
                while ($line =~ /(\@[A-Za-z_]+\@)/g) {
                        printf "${in} (1): warning $1\n";
                }

        } else {
                if ($1 eq '@PERL@') {
                    $busybox = '${ENV{MC_BUSYBOX}}';
                } else {
                    $busybox = 'os.environ.get(\'MC_BUSYBOX\')';
                }
        }
        chomp $line;
        print OUT "${line}\n";
}

while ($line = <IN>) {
        # other lines
        if ($verbose) {
                pos($line) = 1;
                while ($line =~ /(\@[A-Za-z_]+\@)/g) {
                    printf "${in} ($.): $1\n";
                }
        }

        $line =~ s/\@AWK\@/${busybox} awk/g;
        $line =~ s/\@GREP\@/${busybox} grep/g;
        $line =~ s/\@HAVE_ZIPINFO\@/0/g;
        $line =~ s/\@MANDOC\@/mandoc/g;
        $line =~ s/\@MAN_FLAGS\@//g;
        $line =~ s/\@PERL\@/perl/g;
        $line =~ s/\@PYTHON\@/python/g;
        $line =~ s/\@RUBY\@/ruby/g;
        $line =~ s/\@UNZIP\@/${busybox} unzip/g;
        $line =~ s/\@ZIP\@/${busybox} zip/g;

        if ($line =~ /(\@[A-Za-z_]+\@)/) {
                    printf "WARNING ${in} ($.): unknown variable\n";
        }

        chomp $line;
        print OUT "${line}\n";
}

close(IN);
close(OUT);

exit 0;

