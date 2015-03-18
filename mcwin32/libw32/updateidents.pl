#!/usr/bin/perl
# $Id: updateidents.pl,v 1.2 2012/08/03 02:26:42 ayoung Exp $
# -*- mode: perl; tabs: 8; indent-width: 4; -*-
# Update the identifiers and copyright notice within the specified files
# Note, script assumes the consumption of C/C++ based source usage
# on other source shall have undefined results.
#
# Copyright (c) 2012 - 2015, Adam Young.
# All Rights Reserved
#
#

use strict;
use warnings;

use Cwd 'realpath', 'getcwd';
use Getopt::Long;
use File::Copy;                                 # copy()
use File::Basename;
use POSIX 'asctime';

my $CWD                     = getcwd();

sub Main();
sub Export($$);
sub Usage;

my $o_prefix    = 'LIBW32_';
my $o_notice    = 'notice.txt';
my $o_dryrun    = 0;
my $o_backup    = 1;
my $o_rcsid     = 0;
my $o_rcsold    = 0;
my $o_glob      = 0;

Main();
exit 0;

#   Function:           Main
#       Makelib command line usage.
#
#   Parameters:
#       ARGV -              Argument vector.
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
                'rcsid'     => \$o_rcsid,
                'norcsid'   => sub {$o_rcsid  = 0},
                'rcsold'    => \$o_rcsold,
                'nonotice'  => sub {$o_notice = ''},
                'notice=s'  => \$o_notice,
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
#       Makelib command line usage.
#
#   Parameters:
#       [msg] -             Optional message.
#
#   Returns:
#       nothing
#
sub
Usage                   # ([message])
{
    my ($message) = @_;

    print "\nupdateidents: $message\n\n"
        if ($message ne "");

    print <<EOU;
    print Usage: perl updateidents.pl [options] <source>

Options
    --help                  Help.

    --dryrun                Dryrun mode, report yet dont modify any source.
    --run                   Execute.


    --[no]rcsid             Process and update RCSID labels.
    --rcsold                Build old style RCSID labels.

    --[no]backup            Backup original source prior to any modifications.

    --notice=<file>         Copyright/usage notice to be applied.
    --nonotice

    --glob                  Perform interal glob on specified names.

EOU
    exit(42);
}


#   Function:           Process
#       Process the specified source image 'file'.
#
#   Parameters:
#       file -              Source file.
#
#   Returns:
#       nothing
#
sub
remove($$)
{
    my ($lines, $idx) = @_;

    if ($$lines[ $idx ]) {
        print "- $$lines[ $idx ]" if ($o_dryrun);
        $$lines[ $idx ] = "";
    }
}


sub
replace($$$)
{
    my ($lines, $idx, $txt) = @_;

    print "! $$lines[ $idx ]" if ($o_dryrun);
    $$lines[ $idx ] = $txt;
}


sub
insert($$)
{
    my ($lines, $txt) = @_;

    splice @$lines, 0, 0, $txt;
    if ($o_dryrun) {
        my @parts = split(/\n/, $txt);
        foreach (@parts) {
            print "+ $_\n";
        }
    }
}


sub
Process($)              # (file)
{
    my ($file) = @_;

    my $ext = ($file =~ /\.([^.])$/ ? uc($1) : "");

    my $ident = uc($file);
    $ident =~ s/\./_/;

    my $define = "${o_prefix}${ident}_INCLUDED";

    my ($lines, $results)
            = load($file, $ident, $ext);

    backup($file) if ($o_backup);

    my ($header, $modeline, $notice, $leading, $footer) =
            ("", "", "", "", "");

    if ($o_notice) {
        print "importing <$o_notice>\n";
        open(IN, "<${o_notice}") or
            die "cannot open '${o_notice}' : $!";
        if (defined($notice = <IN>)) {
            $notice =~ /^(\s*[\*\-\/\+\#]+)/;   # allow '*', '-', '-', '+' and '#'
            $leading = $1;
            while (<IN>) {
                chomp();
                next if (!$_ && $leading);
                $notice .= "$_\n";
            }
            $notice .= ($leading ? $leading : ' ')."==end==\n"
                if ($notice !~ /==end==/);
        }
        close(IN);
    }

    if ('C' eq $ext || 'CPP' eq $ext) {
        #
        #   C/C++ source
        #

        if ($o_rcsid) {
            if ($o_rcsold) {
                $header =
                    "#ifndef lint\n".
                    "static const char RCSId_".lc($ident).'[] = "$' . 'Id$";' . "\n".
                    "#endif\n\n";
            } else {
                $header =
                    "#include <edidentifier.h>\n".
                    '__CIDENT_RCSID(' .lc($o_prefix.$ident) . ',"$' . 'Id:'.$$results{ID}.'$")' . "\n".
                    "\n";
            }
        }

        $modeline =
            "/* -*- mode: c; indent-width: 4; -*- */\n";

        if ($o_rcsid) {
            my $RCSID = $$results{RCS};

            if ($RCSID >= 0) {
                my $prev = $$lines[ $RCSID-1 ];
                my $next = $$lines[ $RCSID+1 ];

                remove($lines, $RCSID);
                remove($lines, $RCSID-1)
                    if ($prev =~ /^[\t ]*$/ || $prev =~ /^#if/ || $prev =~ /edidentifier/);
                if ($next =~ /^[\t ]*$/ || $next =~ /^#endif/) {
                    remove($lines, $RCSID+1);
                    remove($lines, $RCSID+2)
                        if ($$lines[ $RCSID+2 ] =~ /^[\t ]*$/);
                }
            }
        }

    } elsif ('H' eq $ext || 'HPP' eq $ext) {
        #
        #   Header/includes
        #
        $header =
            "#ifndef ${define}\n".
            "#define ${define}\n";

        if ($o_rcsid) {                         # OpenEdit default
            $header .=
                "#include <edidentifier.h>\n".
                '__CIDENT_RCSID(' .lc($o_prefix.$ident) . ',"$' . 'Id:'.$$results{ID}.'$")' . "\n".
                "__CPRAGMA_ONCE\n".
                "\n" if (! $o_rcsold);
        }

        $footer =
            "#endif /*${define}*/\n";

        $modeline =
            "/* -*- mode: c; indent-width: 4; -*- */\n";

        my $IFCOND   = $$results{IFCOND};
        my $IFDEFINE = $$results{IFDEFINE};
        my $IFEND    = $$results{IFEND};

        if ($IFEND > 0) {                       # remove existing
            remove($lines, $IFCOND) if ($IFCOND >= 0);
            remove($lines, $IFDEFINE) if ($IFDEFINE >= 0);

            if ($o_rcsid) {
                my $RCSID = $$results{RCS};
                
                if ($RCSID >= 0) {
                    my $prev = $$lines[ $RCSID - 1 ];
                    my $next = $$lines[ $RCSID + 1 ];

                    remove($lines, $RCSID);
                    remove($lines, $RCSID-1)
                        if ($prev =~ /^[\t ]*$/ || $prev =~ /^#if/ || $prev =~ /edidentifier/);
                    if ($next =~ /^[\t ]*$/ || $next =~ /^#endif/ || $next =~ /ONCE/i) {
                        remove($lines, $RCSID + 1);
                        remove($lines, $RCSID + 2)
                            if ($$lines[ $RCSID + 2 ] =~ /^[\t ]*$/);
                    }
                }
            }

            remove($lines, $IFEND)
                if ($IFEND >= 0);
        }
    }

    my $adjust = 0;

    if ($modeline) {
        my $MODELINE = $$results{MODELINE};

        if ($MODELINE < 0) {                    # add
            if ($o_rcsid) {
                $header .= $modeline;
            } else {
                if ($modeline) {
                    insert($lines, $header);
                    ++$adjust;
                }
            }
        } else {                                # update (if contained within stand-alone comment)
            my $modeline_tx = $$lines[ $MODELINE ];

            if ($modeline_tx =~ /^\/\*.*\*\/\s*$/) {
                replace($lines, $MODELINE, $modeline);
            }
        }
    }

    if ($header) {
        insert($lines, $header);
        ++$adjust;
    }

    if ($o_notice) {
        my $NOTESTART = $$results{NOTESTART};
        my $NOTEEND = $$results{NOTEEND};

        # apply new notice
        $NOTESTART += $adjust if ($NOTESTART > 0);
        $NOTEEND += $adjust if ($NOTEEND > 0);

        if (0 == $$results{NOTEEXT}) {
            #
            #   Either no or pre-existing internal.
            #
            if ($NOTEEND > 0 && $NOTESTART > 0) {
                while ($NOTESTART < $NOTEEND) { # pre-existing
                    remove($lines, $NOTESTART);
                    ++$NOTESTART;
                }
                replace($lines, $NOTEEND, $notice);

            } else {
                my $original = $$lines[$NOTEEND];

                if ($original =~ /==end==/) {   # explicit ==end== marker
                    replace($lines, $NOTEEND, $notice);

                } else {                        # new notice
                    $original .= "$leading\n"
                        if ($leading && ($original ne "$leading\n"));
                    replace($lines, $NOTEEND, $original . $notice);
                }
            }
        } elsif ($NOTESTART > 0) {
            #
            #   External notice, retain
            #
            #       [notice]
            #                   break
            #       [orginal]
            #
            my $original = $$lines[$NOTESTART];

            $notice .= "$leading\n"
                if ($leading && ($original ne "$leading\n"));
            replace($lines, $NOTESTART, $notice . $original);
        }

        # remove leading blank lines
    #   foreach (@$lines) {
    #       next if (/\/\*.*\*\/\s*$/);         # full line comment /* ... */
    #       last if (/\/\*/ || /\/\//);         # open comment /* ... or // ...
    #       if (/^\s*$/) {
    #           $_ = '';
    #   }
    }

    if ($footer) {
        push @$lines, $footer;
        print "+ $footer" if ($o_dryrun);
    }

    if ($o_dryrun) {
        print "dryrun;\n";
        foreach (@$lines) {
            print "$_";
        }
        print "----\n";
    } else {
        open(OUT, ">$file") or
            die "cannot recreate '$file' : $!";
        foreach (@$lines) {
            print OUT $_;
        }
        close(OUT);
    }
}


#   Function:           load
#       Process the specified source image 'file'.
#
#   Parameters:
#       file -              Source file.
#       ident -             File ident string
#       ext =               Extension.
#
#   Returns:
#       (\@lines, \%results)
#
sub
trimnl($)
{
    $_ = shift;
    s/\n//g;
    return $_;
}

sub
load($)                 # (file, ident, ext)
{
    my ($file, $ident, $ext) = @_;
    my ($ID, $IFCOND, $IFDEFINE, $RCSID, $IFEND, $MODELINE) =
            ("", 0, 0, 0, 0, 0);

    my ($NOTESTART, $NOTEEND, $NOTEEXT) =
            (0, 0, 0);

    my $blank = 0;
    my $state = 0;
    my @lines;

    open(IN, "<$file") or
        die "can't open '$file' : $!";

    print "Processing '$file' (ident:${ident}, ext:${ext})\n";
    while (<IN>) {
        chomp(); chomp();

        if (/^\s*$/ || /^\s[-*]\s$/) {          # blank, should be leading!!
            $blank = $.
                if ($blank <= 0);
            push @lines, $_."\n";
            next;
        }

        if (/^\s*#ifndef / ||
                /^\s*#if !defined /) {
            #
            #   #ifndef
            #   #if !defined
            #
            if (0 == $state) {
                if (/\Q${ident}\E/i) {
                    #
                    #
                    $IFCOND = $.;
                    $state = 2;                 # condition

                } elsif (/INCLUDE/i || /_${ext}/i) {
                    #
                    #
                    #
                    printf "indent: warning, conditional block misnamed";
                    $IFCOND = $.;
                    $state = 2;

                } else {
                    $state = 101;               # not-found
                }
            }

        } elsif (/^\s*#define/) {
            #
            #   #define
            #
            if (2 == $state) {
                if (/\Q${ident}\E/i || /INCLUDE/i || /_${ext}/i) {
                    $IFDEFINE = $.;
                    $state = 3;                 # found
                }
            } elsif (3 != $state) {
                $state = 102;                   # not-found
            }

        } elsif (/\$(Id|Header)/ &&
                    ((/static/ && /char.*=.*".*"/) || /RCSID/)) {
            #
            #   static [const] char =
            #       "$Id"
            #       "$Header"
            #
            #   IDENT_RCSID(xxx, "$Id")
            #
            $RCSID = $.                         # RCSID identifier
                if (! $RCSID);

        } elsif (/\$(Id|Header): (.*) \$/) {

            $ID = " $2 ";

        } elsif (/-\*-.*mode:/ || /-\*-.*indent-width:/) {

            $MODELINE = $.                      # mode line
                if (! $MODELINE);

        } elsif (/Uni.*California/i ||          # The Regents of the University of California.
                    /\Q1. Redistributions of source code must retain the above copyright\E\s*$/) {

            print "BSD style license encountered, shall preserve\n";
            $NOTEEXT = 1                        # ... BSD style, preserve
                if ($NOTESTART && !$NOTEEND);

        } elsif (/copyright/i ||                # Copyright or ==notice==
                    /==notice==/i) {

            $NOTESTART = ($blank ? $blank : $.)
                if (! $NOTESTART);

        } elsif (/==end==/i) {                  # ==end== marker

            $NOTEEND = $.
                if (! $NOTEEND);

        } elsif (/^\s*\/\*.*\*\/\s*$/) {        # full line comment /* ... */

        } elsif (/\*\//) {                      # */ comment block

            $NOTEEND = $. - 1
                if ($NOTEEND <= 0);

        } elsif (/^\s*#include/ ||              # block end
                    /^\s*#if defined/ || /^\s*#ifdef/) {

            if ($. > 1 && !/#include.*identifier/) {
                if ($NOTEEND <= 0) {            # missing header, build
                    printf("warning: no header block, creating at line $.\n");
                    push @lines, "/*\n";
                    push @lines, " * ==end==\n";
                    push @lines, " */\n";
                    push @lines, "\n";
                    $NOTEEND = $.+1;
                    $. += 4;
                }

                if (3 != $state) {
                    $state = 103;               # not-found, first "statement"
                }
            }
        }

        $blank = 0;
        push @lines, $_."\n";
    }

    if (3 == $state) {
        for (my $i = (scalar(@lines) -1); $i >= $NOTEEND; --$i) {
            my $line = $lines[$i];

            if ($line =~ /^\s*$/) {
                $lines[$i] = "";                # blank, trim

            } elsif ($line =~ /^\s*#endif/) {
                $IFEND = $i + 1;
                $state = 1;                     # closure
                last;

            } else {
                last;
            }
        }
    }

    close(IN);

    printf("Parse complete\n");
    printf(" state:    =${state}\n");
    printf("  ID       =<$ID>\n") if ($ID);
    printf("  #ifdef   =[%4d] <%s>\n", $IFCOND,    trimnl($lines[$IFCOND - 1]))    if ($IFCOND     > 0);
    printf("  #define  =[%4d] <%s>\n", $IFDEFINE,  trimnl($lines[$IFDEFINE - 1]))  if ($IFDEFINE   > 0);
    printf("  rcsid    =[%4d] <%s>\n", $RCSID,     trimnl($lines[$RCSID - 1]))     if ($RCSID      > 0);
    printf("  modeline =[%4d] <%s>\n", $MODELINE,  trimnl($lines[$MODELINE - 1]))  if ($MODELINE   > 0);
    printf("  #endif   =[%4d] <%s>\n", $IFEND,     trimnl($lines[$IFEND - 1]))     if ($IFEND      > 0);
    printf("Copyright\n");
    printf("  start    =[%4d] <%s>\n", $NOTESTART, trimnl($lines[$NOTESTART - 1])) if ($NOTESTART  > 0);
    printf("  end      =[%4d] <%s>\n", $NOTEEND,   trimnl($lines[$NOTEEND - 1]))   if ($NOTEEND    > 0);
    printf("  ext      =$NOTEEXT\n");

    my %results = (                             # export, convert lineno's to index's
            ID         => $ID,
            IFCOND     => $IFCOND - 1,
            IFEND      => $IFEND - 1,
            IFDEFINE   => $IFDEFINE - 1,
            RCS        => $RCSID - 1,
            MODELINE   => $MODELINE - 1,
            NOTESTART  => $NOTESTART - 1,
            NOTEEND    => $NOTEEND - 1,
            NOTEEXT    => $NOTEEXT
            );

    return (\@lines, \%results);
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

#end








