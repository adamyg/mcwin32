#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: libtool_win32.pl,v 1.20 2022/06/09 16:37:09 cvsuser Exp $
# libtool emulation for WIN32 builds.
#
#   **Warning**
#
#       Functionality is limited to the current GRIEF/MC/WINRSH build requirements.
#
#   Example usage:
#
#       $(D_LIB)/%.la:      $(D_OBJ)/%.lo
#               $(LIBTOOL) --mode=link $(CC) $(LDFLAGS) -rpath $(D_LIB) -bindir $(D_BIN) -o $@ $(D_OBJ)/$<
#
#       $(D_LIB)/%.lo:      %.c
#               $(LIBTOOL) --mode=compile $(CC) $(CFLAGS) -o $(D_OBJ)/$@ -c $<
#
#       $(D_LIB)/%.lo:      %.cpp
#               $(LIBTOOL) --mode=compile $(CXX) $(CXXFLAGS) -o $(D_OBJ)/$@ -c $<
#
# Copyright Adam Young 2012-2022
# All rights reserved.
#
# This file is part of the Midnight Commander.
#
# The applications are free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
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

use Getopt::Long;
use File::Basename;
use File::Copy;
use Cwd;

my $x_libdir = ".libs/";
my $o_help;
my $o_config;
my $o_dryrun;
my $o_features;
my $o_mode;
my $o_tag;
my $o_preserve_dup_deps;
my $o_quiet = 1;
my $o_silent = 0;
my $o_verbose = 0;
my $o_keeptmp = 0;
my $o_debug;
my $o_version;
my $o_extra = '';

sub Compile();
sub Link();
sub true_object($);
sub true_library($;$);
sub unix2dos($);
sub dos2unix($);
sub Help;
sub Usage;
sub Usage_Link();
sub Usage_Compile();
sub System;
sub __SystemReturnCode($);
sub Label;
sub Debug;
sub Verbose;
sub Warning;
sub Error;

exit &Main();


#   libtool ...
#
sub
Main
{
    Help() if (!scalar @ARGV);

    Getopt::Long::Configure("require_order");

    my $ret =
        GetOptions(
            'h|help'            => \$o_help,
            'config'            => \$o_config,
            'n|dry-run'         => \$o_dryrun,
            'features'          => \$o_features,
            'mode=s'            => \$o_mode,
                'finish'        => sub {$o_mode='finish';},
            'tag=s'             => \$o_tag,
            'preserve-dup-deps' => \$o_preserve_dup_deps,
            'quiet'             => \$o_quiet,
            'no-quiet'          => sub {$o_quiet=0;},
            'silent'            => \$o_silent,
            'no-silent'         => sub {$o_silent=0;},
            'v|verbose'         => \$o_verbose,
            'keeptmp'           => \$o_keeptmp,
            'no-verbose'        => sub {$o_verbose=0;},
            'debug'             => \$o_debug,
            'version'           => \$o_version);

    Help() if (!$ret);
    if ($o_help && $o_mode) {
        if ('compile' eq $o_mode) {
            Usage_Compile();

        } elsif ('link' eq $o_mode) {
            Usage_Link();
        }
    }
    Usage() if ($o_help);
    Usage("missing arguments") if (!scalar @ARGV);

    if ('compile' eq $o_mode) {
        # Compile a source file into a libtool object.
        Compile();

    } elsif ('link' eq $o_mode) {
        # Create a library or an executable.
        Link();

    } elsif ('install' eq $o_mode) {
        # Install libraries or executables.

    } elsif ('finish' eq $o_mode) {
        # Complete the installation of libtool libraries on the system.

    } elsif ('execute' eq $o_mode) {
        # Automatically set the library path so that another program can use
        # uninstalled libtool-generated programs or libraries.

    } elsif ('uninstall' eq $o_mode) {
        # Delete installed libraries or executables.

    } elsif ('clean' eq $o_mode) {
        # Delete uninstalled libraries or executables.
        Clean();

    } else {
        Usage("unknown mode <$o_mode>");
    }
    return 0;
}


#   Function:       Compile
#       Compile a library object.
#
sub
Compile()
{
    my $cc = shift @ARGV;
    my $object;
    my $source;
    my $compile;

    my @DEFINES;
    my @INCLUDES;
    my @STUFF;

    while (scalar @ARGV > 1) {
        $_ = shift @ARGV;

        if (/^\@(.*)$/) {                       # @cmdfile
            OpenCommandFile($1);
            next;
        }

        if (/^-o$/) {                           # -o <object>
            my $val = shift @ARGV;
            Error("compile: missing output")
                if (! $val);
            Error("compile: multiple objects specified <$object> and <$val>")
                if ($object);
            $object = $val;

        } elsif (/^[-\/]Fo[=]?(.*)/) {          # -Fo[=]<object>
            my $val = ($1 ? $1 : shift @ARGV);
            Error("compile: multiple objects specified <$object> and <$val>")
                if ($object);
            $object = $val;

        } elsif (/^[-\/]Fd[=]?(.*)/) {          # -Fd[=]<pdb>
            #consume

        } elsif (/^-i[=]?(.*)$/) {              # -i= <path>
            push @INCLUDES, ($1 ? $1 : shift @ARGV);

        } elsif (/^[-\/]I[=]?(.+)$/) {          # -I[=]<path>
            push @INCLUDES, $1;

        } elsif (/^-I$/) {                      # -I <path>
            push @INCLUDES, shift @ARGV;

        } elsif (/^-d(.*)$/) {                  # -d <define[=value]>
            if ('wcl386' eq $cc && (/^-d[0-9]/ || /^-db$/)) {
                push @STUFF, $_;
            } else {
                push @DEFINES, ($1 ? $1 : shift @ARGV);
            }

        } elsif (/^[-\/]D[=]?(.+)/) {           # -D[=]<define[=value]>
            push @DEFINES, $1;

        } elsif (/^[-\/]c$/) {                  # -c
            $compile = 1;

        } elsif (/^-prefer-pic$/) {
            # Libtool will try to build only PIC objects.

        } elsif (/^-prefer-non-pic$/) {
            # Libtool will try to build only non-PIC objects.

        } elsif (/^-shared$/) {
            # Even if Libtool was configured with --enable-static, the object file Libtool builds will
            # not be suitable for static linking. Libtool will signal an error if it was configured
            # with --disable-shared, or if the host does not support shared libraries.

        } elsif (/^-static$/) {
            # Even if libtool was configured with --disable-static, the object file Libtool builds will
            # be suitable for static linking.

        } elsif (/^-Wc,(.*)$/) {
            # Pass a flag directly to the compiler. With -Wc,, multiple flags may be separated by
            # commas, whereas -Xcompiler passes through commas unchanged.
            if ('owcc' eq $cc || 'gcc' eq $cc) {
                push @STUFF, $_;
            } else {
                Error("compile: unexpected option <$_>");
            }

        } elsif (/^-Xcompiler$/) {
            # Pass a flag directly to the compiler. With -Wc,, multiple flags may be separated by
            # commas, whereas -Xcompiler passes through commas unchanged.
            push @STUFF, shift @ARGV;

        } else {
            push @STUFF, $_;
        }
    }
    $source = shift @ARGV;

    Error("compile: unsupported compiler <$cc>")
        if (!('cl' eq $cc || 'wcl386' eq $cc || 'owcc' eq $cc || 'gcc' eq $cc || 'g++' eq $cc));
    Error("compile: unable to determine object")
        if (!$object);
    Error("compile: object file suffix not <.lo>")
        if ($object !~ /.lo$/);
    Error("compile: missing source file")
        if (!$source || $source =~ /^-/);       # missing or an option.
    Error("compile: -c options missing")
        if (!$compile);

    my $true_path = unix2dos(dirname($object))."\\${x_libdir}";
    my $true_object = ${true_path}.basename($object, 'lo')."obj";

    Verbose "cc:       ${cc}";
    Verbose "extra:    @STUFF";
    Verbose "defines:";
        foreach(@DEFINES) { Verbose "\t$_"; }
    Verbose "includes:";
        foreach(@INCLUDES) { Verbose "\t$_"; }
    Verbose "object:   ${object}";
    Verbose "  true:   ${true_object}";
    Verbose "source:   ${source}";

    my $cmd = '';

    if ('cl' eq $cc) {
        $cmd  = "$cc @STUFF /DDLL=1";
        foreach (@DEFINES) { $cmd .= " /D$_"; }
        foreach (@INCLUDES) { $cmd .= " /I$_"; }
        $cmd .= " /Fo$true_object";
        $cmd .= " /Fd".${true_path}.'\\';       # VCx0.pdb
        $cmd .= " /c $source";

    } elsif ('wcl386' eq $cc) {     # OpenWatcom, legacy interface.
        # http://www.openwatcom.org/index.php/Writing_DLLs
        $cmd  = "$cc @STUFF -dDLL=1";
        $cmd .= " -bd";                         # DLL builds
        foreach (@DEFINES) { $cmd .= " -d$_"; }
        foreach (@INCLUDES) { $cmd .= " -I=\"$_\""; }
        $cmd .= " -Fo=\"".unix2dos($true_object)."\"";
        $cmd .= " -c ".unix2dos($source);

    } elsif ('owcc' eq $cc) {       # OpenWatcom, posix driver.
        $cmd  = "$cc @STUFF -DDLL=1";
        $cmd .= " -shared";                     # DLL builds
        foreach (@DEFINES) { $cmd .= " -D$_"; }
        foreach (@INCLUDES) { $cmd .= " -I \"$_\""; }
        $cmd .= " -o \"".unix2dos($true_object)."\"";
        $cmd .= " -c ".unix2dos($source);

    } elsif ('gcc' eq $cc || 'g++' eq $cc) {
        $cmd  = "$cc @STUFF -D DLL=1 -shared";
        foreach (@DEFINES) { $cmd .= " -D $_"; }
        foreach (@INCLUDES) { $cmd .= " -I \"$_\""; }
        $cmd .= " -o \"$true_object\"";
        $cmd .= " -c $source";
    }

    my $ret = 0;
    mkdir $true_path;
    unlink $object;

    Verbose "cc:       $cmd";
    exit($ret)
        if (0 != ($ret = System($cmd)));

    # generate lo artifact
    open(LO, ">", $object) or
        die "cannot create <".$object."> : $!\n";
    print LO "#libtool win32 generated, do not modify\n";
    print LO "mode=dll\n";
    print LO "cc=$cc\n";
    print LO "cmd=$cmd\n";
    print LO "source=$source\n";
    print LO "object=$object\n";
    print LO "true_object=".dos2unix($true_object)."\n";
    close(LO);
    return 0;
}


#   Function:       Link
#       Link a library object.
#
sub
Link()
{
    my $cc = shift @ARGV;

    my ($output, $dlbase, $rpath, $bindir, $module, $mapfile);
    my $version_number = '';
    my $wc_fastcall = -1;                       # fastcall (Watcom) convention.
    my $wc_debugger = '';
    my $linktype = 'dll';
    my $cl_ltcg = 0;
    my $cl_debug = undef;
    my $gcc_debugger = 0;
    my @OBJECTS;
    my @RESOURCES;
    my @EXPORTS;
    my $DESCRIPTION;
    my @LIBRARIES;
    my @LIBPATHS;
    my @STUFF;

    $cc = 'gcc' if ('g++' eq $cc);              # alises
    while (scalar @ARGV) {
        $_ = shift @ARGV;

        if (/^\@(.*)$/) {                       # @cmdfile
            OpenCommandFile($1);
            next;
        }

        if (/^-o$/) {                           # -o <output>
            my $val = shift @ARGV;
            Error("link: missing output")
                if (! $val);
            Error("link: multiple outputs specified <$output> and <$val>")
                if ($output);
            if ($val =~ /\.exe$/i) {
                $linktype = 'exe';
            } elsif ($val =~ /\.(la|dll)$/i) {
                $linktype = 'dll';
            } else {
                Error("link: unexpected output type <${val}>");
            }
            $output = $val;

        } elsif (/^[-\/]Fo[=]?(.*)/) {          # -Fo[=]<output>
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: multiple outputs specified <$output> and <$val>")
                if ($output);
            $output = $val;

        } elsif (/^[-\/]Fe[=]?(.*)/) {          # -Fe[=]<output>
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: multiple outputs specified <$output> and <$val>")
                if ($output);
            $linktype = 'exe';
            $output = $val;

        } elsif (('wcl386' eq $cc || 'owcc' eq $cc) and /^[-\/]Fm[=]?(.*)/i) {
                                                # -Fm[=]<output>
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: multiple mapfile specified <$mapfile> and <$val>")
                if ($mapfile);
            $mapfile = $val;

        } elsif (/^[-\/]Map[:=]?(.*)/i) {       # -Map[:=]<output>
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: multiple mapfile specified <$mapfile> and <$val>")
                if ($mapfile);
            $mapfile = $val;

        } elsif (/^(.*)\.lo$/ || /^(.*)\.o$/ || /^(.*)\.obj$/i) {
            push @OBJECTS, $_;

        } elsif (/^(.*)\.res$/) {
            push @RESOURCES, $_;

        } elsif (/^(.*)\.rc$/) {
            Error("link: $_ not supported\n");

        } elsif (/^(.*)\.a$/ || /^(.*)\.lib$/i) {
            my $libname = $_;
            $libname =~ s/^lib//                # libxxxx[.a] => xxxx[.a]
                if ('gcc' eq $cc);
            push @LIBRARIES, $libname;

        } elsif (/^(.*)\.la$/) {
            push @LIBRARIES, $_;

        } elsif (/^-l(.*)$/) {
            my $libname = $1;
            if ('gcc' ne $cc) {
                if ($libname !~ /^lib/) {       # xxxx => libxxxx.lib
                    if ($libname !~ /\.lib$/) {
                        $libname = "lib${libname}.lib";
                            #XXX: maybe need to resolve
                    }
                }
            }
            push @LIBRARIES, $libname;

        } elsif (/^[-\/]LIBPATH[:]?\s*(.+)/) {  # -LIBPATH[:]<path>
            push @LIBPATHS, $1;

        } elsif (/^-L(.*)/) {
            push @LIBPATHS, $1;

        } elsif (/^-all-static$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-avoid-version$/) {
            #ignore

        } elsif (/^-bindir[=]?(.*)$/) {
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: $_ $val, not a valid directory : $!")
                if (! -d $val);
            $bindir = $val;

        } elsif (/^-dlbase[=]?(.*)/) {          # -dlbase[=]<output> (extension)
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: multiple dll base names specified <$dlbase> and <$val>")
                if ($dlbase);
            $dlbase = $val;     # dll base name, version is appended.

        } elsif (/^-dlopen(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-dlpreopen(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-export-dynamic$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-export-fastcall$/) {        # extension
            if ($wc_fastcall eq -1 or $wc_fastcall eq 1) {
                $wc_fastcall = 1;
            } else {
                Error("link: -export-fastcall and compiler switches are incompatible\n");
            }

        } elsif (/^-export-symbols[=]?(.*)$/) {
            my $val = ($1 ? $1 : shift @ARGV);
            Error("link: $_ $val, not a valid symbol file : $!")
                if (! -f $val);

            if ($val =~ /\.def$/i) {            # <name.def>
                ParseDefFile($val, \@EXPORTS, \$DESCRIPTION);
            } else {                            # <name.sym>
                ParseSymFile($val, \@EXPORTS);
            }

        } elsif (/^-export-symbols-regex(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-module$/) {
            $module=1;

        } elsif (/^-no-fast-install$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-no-install$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-no-undefined$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-objectlist(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-precious-files-regex(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-release(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^[-]+rpath[=]?(.*)$/) {
            my $val = ($1 ? $1 : shift @ARGV);
            if (! -d $val) {
                Warning("link: $_ $val, not a valid directory -- ignored");
            } else {
                $rpath = $val;
            }

        } elsif (/^-RTC[1csu]+$/ && ('cl' eq $cc)) {
            push @STUFF, $_;

        } elsif (/^-R(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-shared$/) {
            #default

        } elsif (/^-shrext(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-static$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-static-libtool-libs$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-version-info(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-version-number[=]?(.*)$/) {
            my $val = ($1 ? $1 : shift @ARGV);
            $version_number = $val;

        } elsif (/^-weak(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-Wc,(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-Xcompiler(.*)$/) {
            Error("link: $_ not supported\n");

        } elsif (/^-Wl,(.*)$/) {
            if ('owcc' eq $cc) {
                my $wl = $1;
                if ($wl =~ /^LIBPATH[:= ]?\s*(.+)/) {
                    push @LIBPATHS, $1;         # -LIBPATH[:= ]<path>
                    next;
                }
            }
            Error("link: $_ not supported\n");

        } elsif (/^-Xlinker(.*)$/) {
            my $opt = ($1 ? $1 : shift @ARGV);

            if ($opt =~ /^-Map=(.+)$/) {
                my $val = $1;
                Error("link: multiple mapfile specified <$mapfile> and <$val>")
                    if ($mapfile);
                $mapfile = $val;
            } else {
                Error("link: -Xlinker ${opt} opt supported\n");
            }

        } elsif (/^-XCClinker(.*)$/) {
            Error("link: $_ not supported\n");

        } else {
            # toolchain specific
            if ('cl' eq $cc) {

                # Optimisation
                if (/^[-\/]GL$/i) {             # /GL (Whole Program Optimization)
                    $cl_ltcg = 1;               # imply /LTCG
                    next;

                } elsif (/^[-\/]LTCG$/i) {
                    $cl_ltcg = 1;               # explicit /LTCG
                    next;

                # Debugger
                } elsif (/^[-\/]Z([7iI])$/) {   # /Z7, /Zi /ZI, enable debug
                    $cl_debug = "/DEBUG"
                        if (!defined $cl_debug);
                    $cl_ltcg = 0;

                } elsif (/^[-\/]DEBUG$/ || /^[-\/]DEBUG:(.+_)$/) {
                    $cl_debug = $_;             # explicit /DEBUG:NONE, /DEBUG:FULL and /DEBUG:FASTLINK
                    $cl_debug =~ s/^-/\//;
                    $cl_ltcg = 0;
                    next;
                }

            } elsif ('wcl386' eq $cc) {         # process
                # Calling convention:
                #   [3-6]r      register calling convention
                #   [3-6]s      stack based calling convention
                #
                #   ecc         set default calling convention to __cdecl
                #   ecd         set default calling convention to __stdcall
                #   ecf         set default calling convention to __fastcall
                #   ecp         set default calling convention to __pascal
                #   ecr         set default calling convention to __fortran
                #   ecs         set default calling convention to __syscall
                #   ecw         set default calling convention to __watcall (default)
                #
                # Warning:
                #   The OpenWatcom run-time library utilise either register or stack based (by default register) calls,
                #   hence use of the alternative convention via a '-ecx' option may create major compatiblity issues.
                #
                #   For example atexit() and qsort() shall assume register/stack yet as the default is applied the
                #   function argument, the caller can siliently pass an alternatively coded function; that shall be
                #   fatal at run-time.
                #
                if (/^-[3456]r$/ or /^-ecw$/) {
                    if ($wc_fastcall eq -1) {
                        $wc_fastcall = 1;       # auto-selection
                    } elsif ($wc_fastcall eq 0) {
                        Error("link: -export-fastcall and compiler switches are incompatible\n");
                    }

                } elsif (/^-[3456]s$/ or /^-ec[cdfprs]$/) {
                    if ($wc_fastcall eq 1) {
                        Error("link: -export-fastcall and compiler switches are incompatible\n");
                    }

                # Target
                } elsif (/^-bt=(.*)$/) {
                    my $target = uc($1);
                    Error("link: <$_> unexpected target\n")
                        if ($target ne 'NT');

                # Debugger support:
                #   -h[wcd]     Watcom,Codeview,Dwarf
                #
                } elsif (/^-h([wcd])$/) {
                    $wc_debugger = $1;
                }

            } elsif ('owcc' eq $cc) {
                # Call convention
                if (/^mregparm=1$/) {
                    if ($wc_fastcall eq -1) {
                        $wc_fastcall = 1;       # auto-selection
                    } elsif ($wc_fastcall eq 0) {
                        Error("link: -export-fastcall and compiler switches are incompatible\n");
                    }

                } elsif (/^mregparm=0$/) {
                    if ($wc_fastcall eq 1) {
                        Error("link: -export-fastcall and compiler switches are incompatible\n");
                    }

                # Target
                } elsif (/^-b$/) {
                    my $target = uc(shift @ARGV);
                    Error("link: <$_> unexpected target\n")
                        if ($target ne 'NT');

                } elsif (/^-m(console|windows)$/) {
                    my $target = $1;
                    Error("link: <$_> unexpected target\n")
                        if ($target ne 'console');

                # Debugger
                } elsif (/^-g([wcd])$/) {       # -g[wcd] Watcom,Codeview,Dwarf
                    $wc_debugger = $1;
                }

            } elsif ('gcc' eq $cc) {            # process
                if (/^-g$/) {                   # -g
                    $gcc_debugger = 1;
                }
            }

            push @STUFF, $_;
        }
    }

    Error("link: unsupported compiler <$cc>")
        if (!('cl' eq $cc || 'wcl386' eq $cc || 'owcc' eq $cc || 'gcc' eq $cc));
    Error("link: unable to determine output")
        if (!$output);
    if ($linktype eq 'dll') {
      Error("link: output file suffix not <.la>")
          if ($output !~ /.la$/);
    }

    Verbose "cc:       $cc";
    Verbose "extra:    @STUFF";
    Verbose "output:   $output";
    Verbose "objects:";
        foreach(@OBJECTS) {
            Verbose "\t$_";
        }
    Verbose "libraries:";
        foreach(@LIBRARIES) { Verbose "\t$_"; }

    my ($dll_version, $dll_major, $dll_minor) = ('', 0, 0);
    if ($version_number) {
        if ($version_number =~ /^\s*(\d+)\s*$/) {
            # <major>
            $dll_version = "$1.0";              # <name>major.0
            $version_number = ".$1";

        } elsif ($version_number =~ /^\s*(\d+):(\d+)$/) {
            # <major>:<minor>
            $dll_version = "$1.$2";             # major.minor
            $version_number = ".$1.$2";         # <name>.<major.dll

        } elsif ($version_number =~ /^\s*(\d+):(\d+):(\d+)$/) {
            # <major>:<minor>:<revision>
            $dll_version = "$1.$2";             # major.minor
            $version_number = ".$1.$2.$3";      # <name>.<major.<revision>.dll

        } elsif ($version_number =~ /^\s*(\d+)\s*\.\s*(\d+)$/) {
            # <major>.<minor>
            $dll_version = "$1.$2";             # major.minor
            $version_number = ".$1.$2";         # <name>.<major.<minor>.dll

        } elsif ($version_number =~ /^\s*(\d+)\s*\.\s*(\d+)\.\s*(\d+)$/) {
            # <major>.<minor>.<revision>
            $dll_version = "$1.$2";             # major.minor
            $version_number = ".$1.$2.$3";      # <name>.<major.<revision>.dll

        } else {
            Error("link: invalid -version_number <$version_number>\n");
        }
    }

    my ($cmdfile, $deffile, $cmd) = ("__libtool__.lnk", "__libtool__.def");
    my $basedir  = unix2dos(dirname($output));
    my $basename = unix2dos(basename($output, '.la'));
    my $basepath = $basedir.'\\'.$basename;
    my $dllbasename = $dlbase ? unix2dos($dlbase) : $basename;
    my $dllbasepath = $basedir.'\\'.$dllbasename;

    my $dllname  = "${dllbasename}${version_number}.dll";
    my $dllpath  = "${dllbasepath}${version_number}.dll";
    my $pdbpath  = "${dllbasepath}${version_number}.pdb";
    my $mappath  = ($mapfile ? $mapfile :
                        ($linktype eq 'dll' ? "${dllbasepath}${version_number}.map" : "${basepath}.map"));
    my $manifestpath = "${dllbasepath}${version_number}.dll.manifest";
    my $libpath  = "${basepath}.lib";           # import library
    my $exppath  = "${basepath}.exp";           # export library
        # Notes:
        #   Export (.exp) files contain information about exported functions and data items. When LIB creates an import library, it also creates an .exp file.
        #   You use the .exp file when you link a program that both exports to and imports from another program, either directly or indirectly.
        #   If you link with an .exp file, LINK does not produce an import library, because it assumes that LIB already created one.
        #   For details about .exp files and import libraries, see "Working with Import Libraries and Export Files" (MSDN).
        #
    my $sympath = undef;

    if ($linktype eq 'dll') {
        my @BAD_OBJECTS = ();

        for my $obj (@OBJECTS) {                # usage warning
            push @BAD_OBJECTS, $obj
                if ($obj !~ /\.lo$/i);
        }

        if (scalar @BAD_OBJECTS) {
print "*** Warning: Linking the shared library ${output} against the\n";
print "*** non-libtool objects @BAD_OBJECTS is not portable!\n";
        }

    } elsif ($linktype eq 'exe') {
        my @BAD_OBJECTS = ();

        for my $obj (@OBJECTS) {                # usage warning
            push @BAD_OBJECTS, $obj
                if ($obj =~ /\.lo$/i);
        }

        if (scalar @BAD_OBJECTS) {
print "*** Warning: Linking the executable ${output} against the\n";
print "*** non-libtool objects @BAD_OBJECTS is not portable!\n";
        }

        $output .= '.exe'
            if ($output !~ /\.exe$/i);
    }

    if ('cl' eq $cc) {
        #
        #   MSVC/Watcom
        #
        if (defined $ENV{'VCToolsInstallDir'}) { # 2010 plus
            my $toolbase = $ENV{'VCToolsInstallDir'};
            $cmd = "\"${toolbase}\\bin\\Hostx86\\x86\\link\" \@$cmdfile";

        } elsif (defined $ENV{'VCINSTALLDIR'}) { # 2008
            my $toolbase = $ENV{'VCINSTALLDIR'};
            $cmd = "\"${toolbase}\\bin\\link\" \@$cmdfile";

        } else {                                 # default
            $cmd = "link \@$cmdfile";
        }

        open(CMD, ">${cmdfile}") or
            die "cannot create <${$cmdfile}>: $!\n";

        if ($linktype eq 'dll') {
            print CMD "/DLL\n";
            print CMD "/SUBSYSTEM:WINDOWS\n";
            print CMD "/ENTRY:_DllMainCRTStartup\@12"."\n";
            print CMD "/OUT:${dllpath}\n";
            print CMD "/IMPLIB:${libpath}\n";
        } else {
            print CMD "/SUBSYSTEM:CONSOLE\n";
            print CMD "/OUT:${output}\n";

        }
        print CMD "/MANIFEST\n";
        print CMD "/NXCOMPAT\n";
        print CMD "/DYNAMICBASE\n";
        print CMD "/MAP:${mappath}\n";
        print CMD "/MAPINFO:EXPORTS\n";
        print CMD "/VERSION:${dll_version}\n"
            if ($dll_version);
        print CMD "/NOLOGO\n"
            if ($o_quiet || $o_silent);
        print CMD "/OPT:REF\n";
        if (defined $cl_debug) {
            print CMD ($cl_debug ? $cl_debug : "/DEBUG") . "\n";
            print CMD "/ASSEMBLYDEBUG\n";
        }
        print CMD "/INCREMENTAL:NO\n";          # after /DEBUG
        print CMD "/LTCG\n"
            if ($cl_ltcg);
      foreach(@OBJECTS) {
        print CMD true_object($_)."\n";
      }
      foreach(@RESOURCES) {
        print CMD true_object($_)."\n";
      }
      if ($linktype eq 'dll') {
        foreach(@EXPORTS) {
          print CMD "/EXPORT:".MSVCExportDef($_)."\n";
        }
      }
      foreach(@LIBPATHS) {
        print CMD "/LIBPATH:$_\n";
      }
      foreach(@LIBRARIES) {
        print CMD unix2dos(true_library($_))."\n";
      }
        close(CMD);

    } elsif ('wcl386' eq $cc || 'owcc' eq $cc) {
        #
        #   OpenWatcom
        #
        $cmd = "wlink \@$cmdfile";

        open(CMD, ">${cmdfile}") or
            die "cannot create <${$cmdfile}>: $!\n";

        if ($linktype eq 'dll') {
            print CMD "system   nt_dll initinstance terminstance\n";
                #
                #   nt_dll: Windows NT Dynamic Link Libraries.
                #
                #   When a dynamic link library uses the Watcom C/C++ run-time libraries, an automatic data
                #   segment is created each time a new process accesses the dynamic link library.  For this reason,
                #   initialization code must be executed when a process accesses the dynamic link library for the
                #   first time.  To achieve this, "INITINSTANCE" must be specified in the "SYSTEM" directive.
                #   Similarly, "TERMINSTANCE" must be specified so that the termination code is executed when
                #   a process has completed its access to the dynamic link library.  If the Watcom C/C++ run-time
                #   libraries are not used, these options are not required.
                #
            print CMD "name     ${dllpath}\n";
            print CMD "option   implib=${libpath}\n";
            print CMD "option   checksum\n";   # create an MS-CRC32 checksum for image.
        } else {
            print CMD "system   nt\n";
                #
                #   nt: Windows NT Character-Mode Executable.
                #   nt_win: Windows NT Windowed Executable.
                #
            print CMD "name     ${output}\n";
        }

     #  print CMD "option   modname='${modulepath}'\n";
     #  print CMD "option   copyright=''\n";
     #  print CMD "option   description=${DESCRIPTION}\n"
     #      if ($DESCRIPTION);

        print CMD "option   version=${dll_version}\n"
            if ($dll_version);
        print CMD "option   map=${mappath}\n";
        print CMD "option   static\n";          # export statics within the map file.
        print CMD "option   artificial\n";      # export internal symbols.
        print CMD "sort\n";                     # sort symbol by address.

        print CMD "option   quiet\n"
            if ($o_quiet || $o_silent);

      if (! $wc_debugger) {                     # options are exclusive; plus before objects.
        $sympath = "${dllbasepath}${version_number}.sym";
        print CMD "option   symfile=${sympath}\n";
            #XXX: consider default when -d1, allowing use within a production environment
      } elsif ($wc_debugger eq 'w') {
        print CMD "debug    watcom all\n";      # 'watcom all' is fatal at times!
      } elsif ($wc_debugger eq 'c') {
        print CMD "debug    codeview\n";
        print CMD "option   cvpack\n";
      } elsif ($wc_debugger eq 'd') {
        print CMD "debug    dwarf\n";
      }
      foreach(@OBJECTS) {
        print CMD "file     ".unix2dos(true_object($_))."\n";
      }
      foreach(@RESOURCES) {
        print CMD "option   resource=".unix2dos(true_object($_))."\n";
      }
      foreach(@EXPORTS) {
        print CMD "export   ".WatcomExportDef($_, $wc_fastcall)."\n";
      }
      foreach(@LIBPATHS) {
        my $u2d = unix2dos($_);
        if ($u2d =~ / /) {
            print CMD "libpath  '${u2d}'\n";
        } else {
            print CMD "libpath  ${u2d}\n";
        }
      }
      foreach(@LIBRARIES) {
        print CMD "library  ".unix2dos(true_library($_))."\n";
      }
        close(CMD);

    } elsif ('gcc' eq $cc) {
        #
        #   MinGW
        #
        $cmd = "g++ \@${cmdfile}";

        $libpath = "${basepath}.a";
        $pdbpath = undef;
        $exppath = undef;
        $manifestpath = undef;

        open(CMD, ">${cmdfile}") or
            die "cannot create <${cmdfile}>: $!\n";
        if ($linktype eq 'dll') {
            print CMD "-o ".dos2unix($dllpath)."\n";
            print CMD "-shared\n";
            print CMD "-Wl,--subsystem,windows\n";
            print CMD "-Wl,--out-implib,".dos2unix("${basepath}.a")."\n";
        } else {
            print CMD "-o ".dos2unix($output)."\n";
            print CMD "-Wl,--subsystem,console\n";
        }
        print CMD "-g\n"
            if ($gcc_debugger);

      foreach(@OBJECTS) {
        print CMD dos2unix(true_object($_))."\n";
      }
        print "warning: resources ignored @RESOURCES\n"
            if (scalar @RESOURCES);
        print CMD "-Xlinker -Map=".dos2unix($mappath)."\n";
      foreach(@LIBPATHS) {
        print CMD "-L ".dos2unix($_)."\n";
      }
      foreach(@LIBRARIES) {
        my $d2u = dos2unix(true_library($_, 1));
        if ($d2u =~ /\//) {                     # path ?
            print CMD "${d2u}\n";
        } else {
            $d2u =~ s/\.lib$//;                 # xxxx.lib  => xxxx
            $d2u =~ s/\.a$//;                   # xxxx.a    => xxxx
            print CMD "-l${d2u}\n";
        }
      }

      if (scalar @EXPORTS) {
        print CMD dos2unix($deffile)."\n";
        open(DEF, ">${deffile}") or
            die "cannot create <${deffile}>: $!\n";
        print DEF "LIBRARY \"${dllname}\"\n";
        print DEF "EXPORTS\n";
        foreach(@EXPORTS) {
          if (/^(.+)=(.+)$/) {
             my ($name, $alias) = ($1, $2);
             $alias =~ s/^_//
                if ($alias =~ /\@/);            # remove leading if <xxxx@##>
             print DEF "'$name'='$alias'\n";    # quote internal name
          } else {
             s/^_// if (/\@/);                  # remove leading if <xxxx@##>
             print DEF "$_\n";
          }
        }
        close(DEF);
      }

        close(CMD);
    }

    my $ret = 0;
    unlink $output;
    if (0 != ($ret = System($cmd))) {
        if ('cl' eq $cc) {
            print "#\n".
            "# LINK: fatal error LNK1123: failure during conversion to COFF; file invalid or corrupt\n".
            "#  can be result of an incorrect version of cvtres.exe due to dual VC10/VC2012 installations,\n".
            "#  rename 'C:/Program Files (x86)/Microsoft Visual Studio 10/VC/Bin/cvtres.exe' => cvtres_org.exe\n".
            "#\n";
#           $o_verbose = 0;
#           System("which cvtres");
        }
        exit ($ret);
    }

    # generate la artifact
    if ($linktype eq 'dll') {
        open(LA, ">${output}") or
            die "cannot create <$output> : $!\n";
        print LA "#libtool win32 generated, do not modify\n";
        print LA "mode=link\n";
        print LA "cc=$cc\n";
        print LA "lib=${libpath}\n";
        print LA "exp=${exppath}\n" if ($exppath);
        print LA "map=${mappath}\n" if ($mappath);
        print LA "sym=${sympath}\n" if ($sympath);
        print LA "dll=${dllpath}\n";
        print LA "pdb=${pdbpath}\n" if ($pdbpath);
        print LA "manifest=${manifestpath}\n" if ($manifestpath);
        print LA "[objects]\n";
        foreach(@OBJECTS) {
            print LA true_object($_)."\n";
        }
        print LA "[libraries]\n";
        foreach(@LIBRARIES) {
            print LA "$_\n";
        }
        close(LA);
    }

    if ('gcc' eq $cc && "${basepath}.a" ne $libpath) {
        copy("${basepath}.a", $libpath) or
            die "link: unable to copy <${basepath}.a> to <${libpath}> : $!\n";
    }

    if ($bindir) {
        print "installing ${dllname} to ${bindir}\n";
        copy($dllpath, "${bindir}/${dllname}") or
            die "link: unable to copy <$dllname> to <${bindir}/${dllname}> : $!\n";
    }

    unlink $cmdfile, $deffile                   # remove temporary
        if (! $o_keeptmp);

    return 0;
}


sub
OpenCommandFile($)
{
    my ($file) = @_;
    my @NARGV;

    open(CMD, "<", $file) or
        die "cannot open command line <".$file."> : $!\n";
    while (<CMD>) {
        s/\s*([\n\r]+|$)//;
        s/^\s+//;
        next if (!$_ || /^[;\#]/);              # blank or comment
        push @NARGV, $_;
    }
    close(CMD);

    (scalar @NARGV) or
        die "empty command line <$file>\n";

    unshift(@ARGV, @NARGV);                     # insert results
}


#   Function: ParseDefFile
#       Parse a definition file.
#
#   Syntax:
#       NAME [application][BASE=address]
#       LIBRARY [library][BASE=address]
#       DESCRIPTION "text"
#       VERSION major[.minor]
#       /HEAP:reserve[,commit]
#       EXPORTS
#           entryname[=internalname] [@ordinal [NONAME]] [PRIVATE] [DATA]
#           ...
#       SECTIONS
#           definitions
#       STACKSIZE reserve[,commit]
#       STUB:filename
#
sub
ParseDefFile($$$)
{
    my ($file, $EXPORTSRef, $DESCRIPTIONRef) = @_;
    my $mode = 0;

    open(DEF, "<", $file) or
        die "cannot open <".$file."> : $!\n";
    while (<DEF>) {
        s/\s*([\n\r]+|$)//;
        s/^\s+//;
        next if (!$_ || /^;/);                  # blank or comment

        if (/^EXPORTS(.*)/) {
            $mode = 1;
            next if (!$1);
            $_ = $1;

        } elsif (/^NAME\s+(.*)/) {              # same as /OUT
            Warning("link: $file ($.), 'NAME $1'option ignored\n");
            next;

        } elsif (/^DESCRIPTION\s+(.*)$/) {
            $$DESCRIPTIONRef = $1;
            next;

        } elsif (/^LIBRARY\s+(.*)/) {
            Warning("link: $file ($.), 'LIBRARY $1' option ignored\n");
            next;

        } elsif (/^VERSION\s+.*$/) {            # same as /VERSION
            Error("link: $file ($.), VERSION option not supported\n");

        } elsif (/^\/HEAP:(.*)/) {
            Error("link: $file ($.), HEAP option not supported\n");

        } elsif (/^SECTIONS/) {
            Error("link: $file ($.), SECTIONS option not supported\n");

        } elsif (/^STACKSIZE\s+.*$/) {
            Error("link: $file ($.), STACKSIZE option not supported\n");

        } elsif (/^STUB:+.*$/) {
            Error("link: $file ($.), STUB option not supported\n");
        }

        if (1 == $mode) {                       # EXPORTS
            s/\s+/ /g;
            push @$EXPORTSRef, $_;
        } else {
            Error("link: $file ($.), unexpected text <$_>\n");
        }
    }
    close(DEF);
}


#   Function: MSVCExportDef
#       Generate a MSVC EXPORTY definition from a DEF definition.
#
#   Conversion:
#       entryname[=internalname]@ordinal [DATA] [PRIVATE]
#           --> /EXPORT:entryname[,@ordinal][,NONAME][,DATA]
#
sub
MSVCExportDef($)
{
    my ($def) = @_;
    my @parts = split(/ /, $def);
    my $name = $parts[0];
    my $ordinal = '';
    my $ret = "";

    if ($name =~ /\@(.+)$/) {                   # optional ordinal
        $ordinal = ",@"."$1";
        $name =~ s/\s*\@.+$//;
    }

    $ret  = $name;
    $ret .= ",${parts[1]}"
        if (scalar @parts >= 2);
    $ret .= ",${parts[2]}"
        if (scalar @parts >= 3);

    return $ret;
}


#   Function: WatcomExportDef
#       Generate a Watcom EXPORT definition from a DEF definition.
#
#   Conversion:
#       entryname[=internalname]@ordinal [DATA] [PRIVATE]
#           --> entryname[.ordinal]=internalname [PRIVATE]
#
sub
WatcomExportDef($$)
{
    my ($def, $fastcall) = @_;
    my @parts = split(/ /, $def);
    my $name = $parts[0];
    my $ordinal = '';
    my $ret = "";

    if ($name =~ /\@(.+)$/) {                   # optional ordinal
        $ordinal = ".$1";
        $name =~ s/\s*\@.+$//;
    }

    if ((scalar @parts >= 2 && $parts[1] eq 'DATA') ||
        (scalar @parts >= 3 && $parts[2] eq 'DATA')) {
        Warning("link: export ($def), Ordinal ignored on DATA element\n")
            if ($ordinal);
        if ($name =~ /^(.+)=(.+)$/) {
            $ret = "'$1='$2'";                  # quote internal name.
        } else {
            $ret = "${name}=_${name}";
        }

    } else {
        if ($name =~ /^(.+)=(.+)$/) {           # quote internal name.
            $ret = "'$1${ordinal}'='$2'";

        } elsif ($fastcall) {                   # fastcall.
            $ret = "${name}${ordinal}=${name}_";

        } else {                                # cdecl.
            $ret = "${name}${ordinal}=_${name}";
        }
    }

    if ((scalar @parts >= 2 && $parts[1] eq 'PRIVATE') ||
        (scalar @parts >= 3 && $parts[2] eq 'PRIVATE')) {
        $ret .= " PRIVATE";                     # otherwise RESIDENT
    }

    return $ret;
}



sub
ParseSymFile($$)
{
    my ($file, $EXPORTSRef) = @_;
    my $mode = 0;

    open(SYM, "<", $file) or
        die "cannot open symbol file <".$file."> : $!\n";
    while (<SYM>) {
        s/\s*([\n\r]+|$)//;
        s/^\s+//;
        next if (!$_ || /^[;#]/);               # blank or comment
        push @$EXPORTSRef, $_;
    }
    close(SYM);
}


#   Function:       Clean
#       Cleanup library object.
#
sub
Clean()
{
    my $rm = shift @ARGV;

    my @OBJECTS;
    my @LIBRARIES;

    while (scalar @ARGV) {
        $_ = shift @ARGV;

        if (/^\@(.*)$/) {                       # @cmdfile
            OpenCommandFile($1);
            next;
        }

        if (/\.lo$/ || /\.o$/ || /\.obj$/ || /\.res$/) {
            push @OBJECTS, $_;

        } elsif (/\.la$/ || /\.a$/ || /\.lib$/) {
            push @LIBRARIES, $_;
        }
    }

    Error("clean: unsupported remove command <$rm>")
        if (!('rm' eq $rm || 'rm.exe' eq $rm));

    foreach(@LIBRARIES) {
        my $lib = $_;
        if ($lib =~ /\.la$/ && -f $lib) {       # library artifact
            open(LA, "<", $lib) or
                die "cannot open library artifact <".$lib."> : $!\n";
            while (<LA>) {
                s/\s*([\n\r]+|$)//;
                if (/^(lib|dll|map|sym|pdb|exp|manifest)=(.*)$/) {
                    Verbose "rm: $2";
                    unlink($2);
                }
            }
            close(LA);
        }
        Verbose "rm: ${lib}";
        unlink($lib);
    }

    my %dirs = ();
    foreach(@OBJECTS) {
        my $obj = $_;
        if ($obj =~ /\.lo$/ && -f $obj) {       # object artifact
            my $true_object = true_object($obj);
            my $extra = $true_object;
            $dirs{dirname($true_object)}++;
            Verbose "rm: ${true_object}";
            unlink($true_object);
            unlink($extra)
                if ($extra =~ s/\.obj$/\.mbr/); # TODO, toolchain specific
        }
        Verbose "rm: ${obj}";
        unlink($obj);
    }

    foreach(keys %dirs) {
        rmdir($_);
    }
    return 0;
}


#   Function:       true_object
#       Retrieve the true object name for the specified 'lo' image.
#
sub
true_object($)          #(lo)
{
    my ($lo) = @_;
    my $true_object;

    return $lo
        if ($lo !~ /.lo$/);
    open(LO, "<", $lo) or
        die "cannot open object image <".$lo."> : $!\n";
    while (<LO>) {
        s/\s*([\n\r]+|$)//;
        next if (!$_ || /^\s#/);
        if (/^true_object=(.*)$/) {
            $true_object = $1;
            last;
        }
    }
    close(LO);
    die "internal: lo truename missing <$lo>" if (!$true_object);
    return $true_object;
}


#   Function:       true_library
#       Retrieve the true object name for the specified 'la' image.
#
sub
true_library($;$)       #(la [,striplib])
{
    my ($la,$striplib) = @_;
    my $true_library;

    return $la
        if ($la !~ /.la$/);
    open(LA, "<", $la) or
        die "cannot open library image <".$la."> : $!\n";
    while (<LA>) {
        s/\s*([\n\r]+|$)//;
        next if (!$_ || /^\s#/);
        if (/^lib=(.*)$/) {
            $true_library = $1;
            last;
        }
    }
    close(LA);
    die "internal: la truename missing <$la>" if (!$true_library);
    $true_library =~ s/^lib//                   # libxxxx.a => xxxx.a
        if (defined $striplib && $striplib);
    return $true_library;
}


#   Function:       unix2dos
#       Forward slash conversion.
#
sub
unix2dos($)             #(name)
{
    my $name = shift;
    $name =~ s/\//\\/g;
    return $name;
}


#   Function:       dos2unix
#       Forward slash conversion.
#
sub
dos2unix($)             #(name)
{
    my $name = shift;
    $name =~ s/\\/\//g;
    return $name;
}


#   Function:       Usage
#       libtool command line usage
#
sub
Help {
    print STDERR "@_\n\n"
        if (@_);
    print STDERR << "EOF";
Minimal windows libtool emulation (version: 0.41)

usage:  libtool.pl [options] ...

    --help for help

EOF
    exit(1);
}


sub
Usage {
    print STDERR "@_\n\n"
        if (@_);
    print STDERR << "EOF";
Minimal win32 libtool emulation (version: 0.41)

usage:  libtool.pl [options] ...

Options:

   --config
       Display libtool configuration variables and exit.

   --debug
       Trace of shell script execution to standard output.

   -n, --dry-run
       List commands only, do not execute.

   --features
       Display basic configuration options.

   --finish
       Same as --mode=finish.

   -h, --help
       Display a help message and exit. If --mode=mode is specified, then
       detailed help for mode is displayed.

   --mode=mode
       Use mode as the operation mode, mode must be set to one of the following:

       compile
           Compile a source file into a libtool object.

       execute
           Automatically set the library path so that another program can use
           uninstalled libtool-generated programs or libraries.

       link
           Create a library or an executable.

       install
           Install libraries or executables.

       finish
           Complete the installation of libtool libraries on the system.

       uninstall
           Delete installed libraries or executables.

       clean
           Delete uninstalled libraries or executables.

   --tag=tag
       Use configuration variables from tag tag (see Tags).

   --preserve-dup-deps
       Do not remove duplicate dependencies in libraries.

   --quiet, --silent
       Do not print out any progress or informational messages.

   -v, --verbose
       Print out progress and informational messages (enabled by default), as
       well as additional messages not ordinary seen by default.

   --keeptmp
       Keep temporary working files.

   --no-quiet, --no-silent
       Print out the progress and informational messages that are seen by default.

   --no-verbose
       Do not print out any additional informational messages beyond those
       ordinarily seen by default.

   --version
       Print libtool version information and exit.

EOF
   exit(1);
}


sub
Usage_Compile() {
   print STDERR << "EOF";

For compile mode, mode-args is a compiler command to be used in creating a "standard" object
file. These arguments should begin with the name of the C compiler, and contain the -c compiler
flag so that only an object file is created.

Libtool determines the name of the output file by removing the directory component from the
source file name, then substituting the source code suffix (e.g. '.c' for C source code) with the
library object suffix, '.lo'.

If shared libraries are being built, any necessary PIC generation flags are substituted into the
compilation command.

The following components of mode-args are treated specially:

   -o
       Note that the -o option is now fully supported. It is emulated on the platforms that dont
       support it (by locking and moving the objects), so it is really easy to use libtool, just
       with minor modifications to your Makefiles.

       Typing for example libtool

           --mode=compile gcc -c foo/x.c -o foo/x.lo

       will do what you expect.

       Note, however, that, if the compiler does not support -c and -o, it is impossible to
       compile foo/x.c without overwriting an existing ./x.o. Therefore, if you do have a source
       file ./x.c, make sure you introduce dependencies in your Makefile to make sure ./x.o (or
       ./x.lo) is re-created after any sub-directorys x.lo:

           x.o x.lo:   foo/x.lo bar/x.lo

       This will also ensure that make wont try to use a temporarily corrupted x.o to create a
       program or library. It may cause needless recompilation on platforms that support -c and
       -o together, but its the only way to make it safe for those that dont.

   -no-suppress
       If both PIC and non-PIC objects are being built, libtool will normally suppress the
       compiler output for the PIC object compilation to save showing very similar, if not
       identical duplicate output for each object. If the -no-suppress option is given in
       compile mode, libtool will show the compiler output for both objects.

   -prefer-pic
       Libtool will try to build only PIC objects.

   -prefer-non-pic
       Libtool will try to build only non-PIC objects.

   -shared
       Even if Libtool was configured with --enable-static, the object file Libtool builds will
       not be suitable for static linking. Libtool will signal an error if it was configured
       with --disable-shared, or if the host does not support shared libraries.

   -static
       Even if libtool was configured with --disable-static, the object file Libtool builds will
       be suitable for static linking.

   -Wc,flag
   -Xcompiler flag
       Pass a flag directly to the compiler. With -Wc,, multiple flags may be separated by
       commas, whereas -Xcompiler passes through commas unchanged.

EOF
}


sub
Usage_Link() {
   print STDERR << "EOF";

Link mode links together object files (including library objects) to form another library or to
create an executable program.

mode-args consist of a command using the C compiler to create an output file (with the -o flag)
from several object files.

The following components of mode-args are treated specially:

-all-static
   If output-file is a program, then do not link it against any shared libraries at all. If
   output-file is a library, then only create a static library. In general, this flag cannot be
   used together with 'disable-static' (see LT_INIT).

-avoid-version
   Tries to avoid versioning (see Versioning) for libraries and modules, i.e. no version
   information is stored and no symbolic links are created. If the platform requires versioning,
   this option has no effect.

-bindir BINDIR
   Pass the absolute name of the directory for installing executable programs (see Directory
   Variables). libtool may use this value to install shared libraries there on systems that do
   not provide for any library hardcoding and use the directory of a program and the PATH
   variable as library search path. This is typically used for DLLs on Windows or other systems
   using the PE (Portable Executable) format. On other systems, -bindir is ignored. The default
   value used is libdir/../bin for libraries installed to libdir. You should not use -bindir for
   modules.

-dlopen FILE
   Same as -dlpreopen file, if native dlopening is not supported on the host platform (see
   Dlopened modules) or if the program is linked with -static, -static-libtool-libs, or
   -all-static. Otherwise, no effect. If file is self Libtool will make sure that the program
   can dlopen itself, either by enabling -export-dynamic or by falling back to -dlpreopen self.

-dlpreopen FILE
   Link file into the output program, and add its symbols to the list of preloaded symbols (see
   Dlpreopening). If file is self, the symbols of the program itself will be added to preloaded
   symbol lists. If file is force Libtool will make sure that a preloaded symbol list is always
   defined, regardless of whether its empty or not.

-export-dynamic
   Allow symbols from output-file to be resolved with dlsym (see Dlopened modules).

-export-symbols symfile
   Tells the linker to export only the symbols listed in symfile. The symbol file should end in
   .sym and must contain the name of one symbol per line. This option has no effect on some
   platforms. By default all symbols are exported.

-export-symbols-regex regex
   Same as -export-symbols, except that only symbols matching the regular expression regex are
   exported. By default all symbols are exported.

-Llibdir
   Search libdir for required libraries that have already been installed.

-lname
   output-file requires the installed library libname. This option is required even when
   output-file is not an executable.

-module
   Creates a library that can be dlopened (see Dlopened modules). This option doesnt work for
   programs. Module names dont need to be prefixed with 'lib'. In order to prevent name clashes,
   however, libname and name must not be used at the same time in your package.

-no-fast-install
   Disable fast-install mode for the executable output-file. Useful if the program wont be
   necessarily installed.

-no-install
   Link an executable output-file that cant be installed and therefore doesnt need a wrapper
   script on systems that allow hardcoding of library paths. Useful if the program is only used
   in the build tree, e.g., for testing or generating other files.

-no-undefined
   Declare that output-file does not depend on any libraries other than the ones listed on the
   command line, i.e., after linking, it will not have unresolved symbols. Some platforms
   require all symbols in shared libraries to be resolved at library creation (see Inter-library
   dependencies), and using this parameter allows libtool to assume that this will not happen.

-o output-file
   Create output-file from the specified objects and libraries.

-objectlist FILE
   Use a list of object files found in file to specify objects.

-precious-files-regex regex
   Prevents removal of files from the temporary output directory whose names match this regular
   expression. You might specify "\.bbg?$" to keep those files created with gcc -ftest-coverage
   for example.

-release release
   Specify that the library was generated by release release of your package, so that users can
   easily tell which versions are newer than others. Be warned that no two releases of your
   package will be binary compatible if you use this flag. If you want binary compatibility, use
   the -version-info flag instead (see Versioning).

-rpath libdir
   If output-file is a library, it will eventually be installed in libdir. If output-file is a
   program, add libdir to the run-time path of the program. On platforms that dont support
   hardcoding library paths into executables and only search PATH for shared libraries, such as
   when output-file is a Windows (or other PE platform) DLL, the .la control file will be
   installed in libdir, but see -bindir above for the eventual destination of the .dll or other
   library file itself.

-R libdir
   If output-file is a program, add libdir to its run-time path. If output-file is a library,
   add -Rlibdir to its dependency_libs, so that, whenever the library is linked into a program,
   libdir will be added to its run-time path.

-shared
   If output-file is a program, then link it against any uninstalled shared libtool libraries
   (this is the default behavior). If output-file is a library, then only create a shared
   library. In the later case, libtool will signal an error if it was configured with
   --disable-shared, or if the host does not support shared libraries.

-shrext suffix
   If output-file is a libtool library, replace the systems standard file name extension for
   shared libraries with suffix (most systems use .so here). This option is helpful in certain
   cases where an application requires that shared libraries (typically modules) have an
   extension other than the default one. Please note you must supply the full file name
   extension including any leading dot.

-static
   If output-file is a program, then do not link it against any uninstalled shared libtool
   libraries. If output-file is a library, then only create a static library.

-static-libtool-libs
   If output-file is a program, then do not link it against any shared libtool libraries. If
   output-file is a library, then only create a static library.

-version-info current[:revision[:age]]
   If output-file is a libtool library, use interface version information current, revision, and
   age to build it (see Versioning). Do not use this flag to specify package release
   information, rather see the -release flag.

-version-number major[:minor[:revision]]
   If output-file is a libtool library, compute interface version information so that the
   resulting library uses the specified major, minor and revision numbers. This is designed to
   permit libtool to be used with existing projects where identical version numbers are already
   used across operating systems. New projects should use the -version-info flag instead.

-weak libname
   if output-file is a libtool library, declare that it provides a weak libname interface. This
   is a hint to libtool that there is no need to append libname to the list of dependency
   libraries of output-file, because linking against output-file already supplies the same
   interface (see Linking with dlopened modules).

-Wc,flag
-Xcompiler flag
   Pass a linker-specific flag directly to the compiler. With -Wc,, multiple flags may be
   separated by commas, whereas -Xcompiler passes through commas unchanged.

-Wl,flag
-Xlinker flag
   Pass a linker-specific flag directly to the linker.

-XCClinker flag
   Pass a link-specific flag to the compiler driver (CC) during linking. If the output-file ends
   in .la, then a libtool library is created, which must be built only from library objects (.lo
   files). The -rpath option is required. In the current implementation, libtool libraries may
   not depend on other uninstalled libtool libraries (see Inter-library dependencies).

If the output-file ends in .a, then a standard library is created using ar and possibly ranlib.

If output-file ends in .o or .lo, then a reloadable object file is created from the input files
(generally using 'ld -r'). This method is often called partial linking.

Otherwise, an executable program is created.

EOF
}


#   Function:       System
#       Execute a system command
#
#   Parameters:
#       cmd -               Command.
#
#   Returns:
#       return-code (-1 = exec error, -2 = core, application return code).
#
sub
System                  #(cmd)
{
    my ($cmd) = @_;

    Verbose "libtool: $cmd\n" if (!$o_silent);
    my $ret = system($cmd);
    $ret = __SystemReturnCode($ret);
    print "libtool: result=$ret\n" if (!$o_silent);
    return $ret;
}


#   Function:       __SystemReturnCode
#       Decode the return code from a system() call
#
#   Parameters:
#       rcode -             Return code.
#
#   Returns:
#       return-code
#
sub
__SystemReturnCode($)   #(retcode)
{
    my $rcode = 0;
    my $rc = shift;

    if ($rc == -1) {
        $rcode = -1;                            # task exec error
    } elsif ($rc & 127) {
        $rcode = -2;                            # cored
    } elsif ($rc) {
        $rcode = $rc >> 8;                      # application return code
    }
    return $rcode;
}


sub
Label {
    return "libtool @_";
}


sub
Debug {
    if ($o_verbose || $o_debug)  {
        print Label("(D) ") . sprintf( shift, @_ ) . "\n";
    }
}


sub
Verbose {
    if ($o_verbose)  {
        print Label("(V) ") . sprintf( shift, @_ ) . "\n";
    }
}


sub
Warning {
    print Label("(W) ") . sprintf( shift, @_ ) . "\n";
}


sub
Error {
    print Label("(E) ") . sprintf( shift, @_ ) . "\n";
    print @_;
    exit(3);
}

#end

