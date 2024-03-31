#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: mklicense.pl,v 1.1 2024/03/15 13:49:24 cvsuser Exp $
#
# Copyright (c) 2020 - 2024, Adam Young.
# All rights reserved.
#
# The applications are free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 3.
#
# Redistributions of source code must retain the above copyright
# notice, and must be distributed with the license document above.
#
# Redistributions in binary form must reproduce the above copyright
# notice, and must include the license document above in
# the documentation and/or other materials provided with the
# distribution.
#
# The applications are distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# ==end==
#

use strict;
use warnings 'all';

my $in  = shift @ARGV;
my $out = shift @ARGV;
my $lbl = 'license';

$lbl = shift @ARGV
        if (scalar @ARGV);

die "usage: mklicense <in> <out> [label=license]\n\n"
        if (!$in or !$out);

open (INPUT, $in) or
        die "can't open <${in}>: $!";

open (OUTPUT, '>', $out) or
        die "can't create <${out}>: $!";

print OUTPUT
        "/* File created from ${in} via mklicense.pl */\n".
        "const char *${lbl}[] = {\n";

my $lines = 0;
while (<INPUT>) {
        chomp;
        print OUTPUT ",\n"
                if ($lines++);
        s/\"/\\\"/g;
        print OUTPUT "\t\"$_\"";
        last if (/END OF TERMS AND CONDITIONS/);
        }

print OUTPUT
        "\n};\n".
        "/*end*/\n";

close(INPUT) or
        die "can't close <${in}>: $!";

close(OUTPUT) or
        die "can't close <${out}>: $!";

#end

