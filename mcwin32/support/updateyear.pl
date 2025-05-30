#!/usr/bin/perl
# $Id: updateyear.pl,v 1.3 2025/02/16 11:12:45 cvsuser Exp $
# -*- mode: perl; tabs: 8; indent-width: 4; -*-
# Update the copyright year within the specified files
#

use strict;
use warnings;

use Cwd 'realpath', 'getcwd', 'abs_path';
use Getopt::Long;
use File::Copy;                                 # copy()
use File::Basename;
use POSIX 'asctime';

my $CWD         = getcwd();                     # current working directory
my $ROOT        = abs_path(dirname($0));        # script path

sub Main();
sub Usage;

my $o_dryrun    = 0;
my $o_backup    = 1;
my $o_glob      = 0;

Main();
exit 0;

#   Function:           Main
#       main
#
#   Parameters:
#       ARGV - Argument vector.
#
#   Returns:
#       nothing
#
sub
Main()
{
    my $o_help = 0;

    my $ret
        = GetOptions(
                'dryrun'    => \$o_dryrun,
                'run'       => sub {$o_dryrun = 0},
                'backup',   => \$o_backup,
                'nobackup', => sub {$o_backup = 0},
                'glob'      => \$o_glob,
                'help'      => \$o_help
                );

    Usage() if (!$ret || $o_help);
    Usage("expected <file ...>") if (scalar @ARGV < 1);

    foreach (@ARGV) {
        if ($o_glob) {                          # internal glob
            my @files = glob("$_");
            print "glob: @files\n";
            foreach (@files) {
                Process($_);
            }
        } else {
            Process($_);                        # for-each
        }
    }
}


#   Function:           Usage
#       command line usage.
#
#   Parameters:
#       [msg] - Optional message.
#
#   Returns:
#       nothing
#
sub
Usage                   # ([message])
{
    my ($message) = @_;

    print "\nupdateyear: $message\n\n"
        if ($message ne "");

    print <<EOU;
    print Usage: perl updateyear.pl [options] <source>

Options
    --help                  Help.

    --dryrun                Dryrun mode, report yet dont modify any source.
    --run                   Execute.

    --[no]backup            Backup original source prior to any modifications (default=yes).

    --glob                  Perform interal glob on specified names.

EOU
    exit(42);
}


#   Function:           Process
#       Process the specified source image 'file'.
#
#   Parameters:
#       file - Source file.
#
#   Returns:
#       nothing
#
sub
Process($)              # (file)
{
    my ($file) = @_;

    my $ext = ($file =~ /\.([^.])$/ ? uc($1) : "");

    my ($lines, $result)
            = load($file, $ext);

    if ($result >= 0) {
        backup($file) if ($o_backup);

        if ($o_dryrun) {
            print "dryrun: update [$result] <$$lines[$result]>\n";
        } else {
            open(OUT, ">$file") or
                die "cannot recreate '$file' : $!";
            foreach (@$lines) {
                print OUT $_;
            }
            close(OUT);
        }
    }
}


sub
load($$)                # (file)
{
    my ($file) = @_;
    my $result = -1;
    my @lines;

    open(IN, "<$file") or
        die "can't open '$file' : $!";
    while (<IN>) {
        chomp(); chomp();
        if ($result < 0) {
            if (/Copyright \(C\) 20/i) {
                s/-[\t\ ]*2023/- 2025/;
                s/-[\t\ ]*2024/- 2025/;
                $result = scalar @lines;

            } elsif (/Copyright \(C\) 19/i) {
                s/-[\t\ ]*2023/- 2025/;
                s/-[\t\ ]*2024/- 2025/;
                $result = scalar @lines;
            }
        }
        push @lines, $_."\n";
    }
    close(IN);

    return (\@lines, $result);
}


sub
backup($)               # (file)
{
    my ($file) = @_;

    if ($o_dryrun) {
        print "dryrun: rename($file, $file.bak)\n";
    } else {
        rename ($file, "${file}.bak") or
            die "can't backup '$file' : $!";
    }
}


sub
trimnl($)
{
    $_ = shift;
    s/\n//g;
    return $_;
}

#end

