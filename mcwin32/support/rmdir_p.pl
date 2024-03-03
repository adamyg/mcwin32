#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: rmdir_p.pl,v 1.1 2020/10/17 18:35:26 cvsuser Exp $
#
# "rmdir -p" emulation.
#
use strict;
use warnings 'all';

sub rmdir_p {
    my $dir = shift;
    my $child = shift;

    $dir =~ s|/*\Z(?!\n)||s;
    return if (-d $dir);

    unless (rmdir($dir)) {
        return if ($child);
        die "Cannot remove directory $dir: $!\n";
    }

    if ($dir =~ m|[^/]/|s) {
        my $parent = $dir;
        $parent =~ s|[^/]*\Z(?!\n)||s;
        rmdir_p($parent, 1);
    }

    print "removed directory `$dir'\n";
}

my $arg;

foreach $arg (@ARGV) {
    $arg =~ tr|\\|/|;
    rmdir_p($arg, 0);
}

#end

