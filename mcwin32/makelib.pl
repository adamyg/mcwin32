#!/usr/bin/perl
# $Id: makelib.pl,v 1.42 2025/03/06 17:16:28 cvsuser Exp $
# Makefile generation under WIN32 (MSVC/WATCOMC/MINGW) and DJGPP.
# -*- perl; tabs: 8; indent-width: 4; -*-
# Automake emulation for non-unix environments.
#
#
# Copyright (c) 1998 - 2025, Adam Young.
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
# MSYS/MINGW install
#
#   Download and install: msys2-x86_64-YYYYMMDD.exe, then
#
#       pacman -S base-devel
#
#       pacman -S mingw-w64-x86_64-gcc          [64-bit]
#    or pacman -S mingw-w64-x86_64-toolchain
#
#       pacman -S mingw-w64-i686-gcc            [32-bit]
#

use strict;
use warnings;

BEGIN {
    my $var = $ENV{"PERLINC"};

    if (defined($var) && -d $var) {             # import PERLINC
        my ($quoted_var) = quotemeta($var);
        push (@INC, $var)
            if (! grep /^$quoted_var$/, @INC);
    }
}

use Cwd 'realpath', 'getcwd';
use Getopt::Long;
use File::Spec;
use File::Copy;                                 # copy()
use File::Basename;
use File::Which;
use POSIX 'asctime';
use Data::Dumper;
use Text::ParseWords;

my $CWD                     = getcwd();
my $BINPATH                 = '';
my $PERLPATH                = '';
my $MFCDIR                  = undef;
my $BUSYBOX                 = undef;
my $WGET                    = undef;
my $INNO                    = undef;
my $BISON                   = undef;
my $FLEX                    = undef;
my $LIBTOOL                 = '';
my $PROGRAMFILES            = ProgramFiles();

my $x_libw32                = 'libw32';

my %x_environment   = (
        'dj'            => {    # DJGPP
            TOOLCHAIN       => 'dj',
            TOOLCHAINEXT    => '.dj',
            TOOLCHAINNAME   => 'DJGPP',
            CC              => 'gcc',
            CXX             => 'g++',
            AR              => 'ar',
            DEFS            => '-DHAVE_CONFIG_H',
            CFLAGS          => '-fno-strength-reduce -I$(ROOT)/djgpp',
            CDEBUG          => '-g',
            CWARN           => '-W -Wall -Wshadow -Wmissing-prototypes',
            },

        'mingw'         => {    # MingW32 or MingW64 (default os)
            TOOLCHAIN       => 'mingw',
            TOOLCHAINEXT    => '.mingw',
            TOOLCHAINNAME   => 'mingw-legacy',
            CC              => 'gcc',
            CXX             => 'g++',
            VSWITCH         => '--version',
            VPATTERN        => 'gcc \([^)]*\) ([0-9\.]+)',
            OSWITCH         => '',
            LSWITCH         => '-l',
            XSWITCH         => '-o',
            AR              => 'ar',
            RC              => 'windres -DGCC_WINDRES',
            DEFS            => '-DHAVE_CONFIG_H',
            CINCLUDE        => '',
            CFLAGS          => '@CCVER@ -fno-strength-reduce',
            CCVER           => '-std=gnu11',
            CXXFLAGS        => '@CXXVER@ -fno-strength-reduce',
            CXXVER          => '-std=c++11',
            CDEBUG          => '-g',
            CWARN           => '-W -Wall -Wshadow -Wmissing-prototypes',
            CXXWARN         => '-W -Wall -Wshadow',
            LDFLAGS         => '',
            LDDEBUG         => '-g',
            LDRELEASE       => '',
            LDMAPFILE       => '-Xlinker -Map=$(MAPFILE)',
            EXTRALIBS       => '-lshlwapi -lpsapi -lole32 -luuid -lgdi32 '.
                                    '-luserenv -lnetapi32 -ladvapi32 -lshell32 -lmpr -lWs2_32',
            LIBMALLOC       => '-ldlmalloc',
            },

        'mingw32'       => {    # MingW64 (32-bit mode)
            TOOLCHAIN       => 'mingw32',
            TOOLCHAINEXT    => '.mingw32',
            TOOLCHAINNAME   => 'MingW32',
            CC              => 'gcc',
            CXX             => 'g++',
            VSWITCH         => '--version',
            VPATTERN        => 'gcc \([^)]*\) ([0-9\.]+)',
            OSWITCH         => '',
            LSWITCH         => '-l',
            XSWITCH         => '-o',
            AR              => 'ar',
            RC              => 'windres -DGCC_WINDRES',
            DEFS            => '-DHAVE_CONFIG_H',
            CINCLUDE        => '',
            CFLAGS          => '-m32 @CCVER@ -fno-strength-reduce',
            CCVER           => '-std=gnu11',
            CXXFLAGS        => '-m32 @CXXVER@ -fno-strength-reduce',
            CXXVER          => '-std=c++11',
            CDEBUG          => '-g',
            CWARN           => '-W -Wall -Wshadow -Wmissing-prototypes',
            CXXWARN         => '-W -Wall -Wshadow',
            LDFLAGS         => '',
            LDDEBUG         => '-g',
            LDRELEASE       => '',
            LDMAPFILE       => '-Xlinker -Map=$(MAPFILE)',
            EXTRALIBS       => '-lshlwapi -lpsapi -lole32 -luuid -lgdi32 '.
                                    '-luserenv -lnetapi32 -ladvapi32 -lshell32 -lmpr -lWs2_32',
            LIBMALLOC       => '-ldlmalloc',
            },

        'mingw64'       => {    # MingW64 (64-bit mode)
            ISWIN64         => 'yes',
            TOOLCHAIN       => 'mingw64',
            TOOLCHAINEXT    => '.mingw64',
            TOOLCHAINNAME   => 'MingW64',
            CC              => 'gcc',
            CXX             => 'g++',
            VSWITCH         => '--version',
            VPATTERN        => 'gcc \([^)]*\) ([0-9\.]+)',
            OSWITCH         => '',
            LSWITCH         => '-l',
            XSWITCH         => '-o',
            AR              => 'ar',
            RC              => 'windres -DGCC_WINDRES',
            DEFS            => '-DHAVE_CONFIG_H',
            CINCLUDE        => '',
            CFLAGS          => '-m64 @CCVER@ -fno-strength-reduce',
            CCVER           => '-std=gnu11',
            CXXFLAGS        => '-m64 @CXXVER@ -fno-strength-reduce',
            CXXVER          => '-std=c++11',
            CDEBUG          => '-g',
            CWARN           => '-W -Wall -Wshadow -Wmissing-prototypes',
            CXXWARN         => '-W -Wall -Wshadow',
            LDFLAGS         => '',
            LDDEBUG         => '-g',
            LDRELEASE       => '',
            LDMAPFILE       => '-Xlinker -Map=$(MAPFILE)',
            EXTRALIBS       => '-lshlwapi -lpsapi -lole32 -luuid -lgdi32 '.
                                    '-luserenv -lnetapi32 -ladvapi32 -lshell32 -lmpr -lWs2_32',
            LIBMALLOC       => '-ldlmalloc',
                #
                #    libgcc_s_dw2-1.dll [x86]
                # or libgcc_s_seh-1.dll [x64]
                #    libstdc++6.dll
                #    libwinpthread-1.dll
                #
                # Alternatively:
                #   -static-libgcc
                #   -static-libstdc++
                #
            },

        'vc1200'        => {    # Visual Studio 7
            TOOLCHAIN       => 'vs70',
            TOOLCHAINEXT    => '.vs70',
            TOOLCHAINNAME   => 'Visual Studio 7',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '',
            RTLIBRARY       => '-MDd',
            CFLAGS          => '-nologo -Yd -GZ @RTLIBRARY@',
            CXXFLAGS        => '-nologo -Yd -GZ @RTLIBRARY@',
            CDEBUG          => '-Zi -Od',
            CRELEASE        => '-O2 -GL -Gy -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo -GZ @RTLIBRARY@',
            LDDEBUG         => '-Zi',
            LDRELEASE       => '',
            LDMAPFILE       => '-MAP:$(MAPFILE)'
            },

        'vc1400'        => {    # 2005, Visual Studio 8
            TOOLCHAIN       => 'vs80',
            TOOLCHAINEXT    => '.vs80',
            TOOLCHAINNAME   => 'Visual Studio 2005',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '',
            RTLIBRARY       => '-MDd',
            CFLAGS          => '-nologo @RTLIBRARY@',
            CXXFLAGS        => '-nologo @RTLIBRARY@ -EHsc',
            CDEBUG          => '-Zi -RTC1 -Od',
            CRELEASE        => '-O2 -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo @RTLIBRARY@',
            LDDEBUG         => '-Zi -RTC1',
            LDMAPFILE       => '-MAP:$(MAPFILE)',

            MFCDIR          => '',
            MFCCFLAGS       => '-nologo @RTLIBRARY@',
            MFCCXXFLAGS     => '-nologo @RTLIBRARY@ -EHsc',
            MFCCOPT         => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCXXOPT       => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCINCLUDE     => '',
            MFCLIBS         => ''
            },

        'vc1500'        => {   # 2008, Visual Studio 9
            TOOLCHAIN       => 'vs90',
            TOOLCHAINEXT    => '.vs90',
            TOOLCHAINNAME   => 'Visual Studio 2008',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            RC              => 'rc',            # no, /nologo option
            CINCLUDE        => '',
            RTLIBRARY       => '-MDd',
            CFLAGS          => '-nologo @RTLIBRARY@ -Dinline=__inline',
            CXXFLAGS        => '-nologo @RTLIBRARY@ -EHsc',
            CDEBUG          => '-Zi -RTC1 -Od',
            CRELEASE        => '-O2 -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo @RTLIBRARY@',
            LDDEBUG         => '-Zi -RTC1',
            LDMAPFILE       => '-MAP:$(MAPFILE)'
                        # -Fm:  if positioned before /link
                        # -MAP: if positioned afer /link
            },

        'vc1600'        => {    # 2010, Visual Studio 10
            TOOLCHAIN       => 'vs100',
            TOOLCHAINEXT    => '.vs100',
            TOOLCHAINNAME   => 'Visual Studio 2010',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '',
            RTLIBRARY       => '-MDd',
            CFLAGS          => '-nologo @RTLIBRARY@',
            CXXFLAGS        => '-nologo @RTLIBRARY@ -EHsc',
            CDEBUG          => '-Zi -RTC1 -Od',
            CRELEASE        => '-O2 -GL -Gy -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo @RTLIBRARY@',
            LDDEBUG         => '-Zi -RTC1',
            LDRELEASE       => '-GL',
            LDMAPFILE       => '-MAP:$(MAPFILE)',
                        # -Fm:  if positioned before /link
                        # -MAP: if positioned afer /link

            # 7600.16385.1: Windows Driver Kit Version 7.1.0
            #   ==> http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=11800
            MFCDIR          => '/tools/WinDDK/7600.16385.1',
            MFCCFLAGS       => '-nologo @RTLIBRARY@',
            MFCCXXFLAGS     => '-nologo @RTLIBRARY@ -EHsc',
            MFCCOPT         => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCXXOPT       => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
            MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            },

       'vc1800'        => {    # 2013, Visual Studio 18
            TOOLCHAIN       => 'vs120',
            TOOLCHAINEXT    => '.vs120',
            TOOLCHAINNAME   => 'Visual Studio 2013',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '',
            RTLIBRARY       => '-MDd',
            CFLAGS          => '-nologo @RTLIBRARY@ -fp:precise',
            CXXFLAGS        => '-nologo @RTLIBRARY@ -EHsc -fp:precise',
            CDEBUG          => '-Zi -RTC1 -Od',
            CRELEASE        => '-O2 -GL -Gy -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo @RTLIBRARY@',
            LDDEBUG         => '-Zi -RTC1',
            LDRELEASE       => '-GL',
            LDMAPFILE       => '-MAP:$(MAPFILE)',
                        # -Fm:  if positioned before /link
                        # -MAP: if positioned afer /link

            # 7600.16385.1: Windows Driver Kit Version 7.1.0
            #   ==> http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=11800
            MFCDIR          => '/tools/WinDDK/7600.16385.1',
            MFCCFLAGS       => '-nologo @RTLIBRARY@',
            MFCCXXFLAGS     => '-nologo @RTLIBRARY@ -EHsc',
            MFCCOPT         => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCXXOPT       => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
            MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            },

       'vc1900'        => {    # 2015, Visual Studio 19
            TOOLCHAIN       => 'vs140',
            TOOLCHAINEXT    => '.vs140',
            TOOLCHAINNAME   => 'Visual Studio 2015',
            CC              => 'cl',
            COMPILERPATHS   => '%VS140COMNTOOLS%/../../VC/bin|%VCINSTALLDIR%/bin',
            COMPILERPATH    => '',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '',
            RTLIBRARY       => '-MDd',
            CFLAGS          => '-nologo @RTLIBRARY@ -fp:precise',
            CXXFLAGS        => '-nologo @RTLIBRARY@ -EHsc -fp:precise',
            CDEBUG          => '-Zi -RTC1 -Od',
            CRELEASE        => '-O2 -GL -Gy -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo @RTLIBRARY@',
            LDDEBUG         => '-Zi -RTC1',
            LDRELEASE       => '-GL',
            LDMAPFILE       => '-MAP:$(MAPFILE)',
                        # -Fm:  if positioned before /link
                        # -MAP: if positioned afer /link

            # 7600.16385.1: Windows Driver Kit Version 7.1.0
            #   ==> http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=11800
            MFCDIR          => '/tools/WinDDK/7600.16385.1',
            MFCCFLAGS       => '-nologo @RTLIBRARY@',
            MFCCXXFLAGS     => '-nologo @RTLIBRARY@ -EHsc',
            MFCCOPT         => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCXXOPT       => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
            MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            },

       # See: VsDevCmd.bat
       #  The VsDevCmd.bat file is a new file delivered with Visual Studio 2017.
       #  Visual Studio 2015 and earlier versions used VSVARS32.bat for the same purpose.
       #  This file was stored in \Program Files\Microsoft Visual Studio\[Version]\Common7\Tools
       #                       or \Program Files (x86)\Microsoft Visual Studio\Version\Common7\Tools.
       #
       #  pushd "C:/Program Files (x86)/Microsoft Visual Studio/2017/Enterprise/Common7/Tools"
       #  call  VsDevCmd.bat
       #  call  VsDevCmd.bat -test
       #  popd
       #
       'vc1910'        => {    # 2017, Visual Studio 19.10 -- 19.1x
            TOOLCHAIN       => 'vs150',
            TOOLCHAINEXT    => '.vs150',
            TOOLCHAINNAME   => 'Visual Studio 2017',
            CC              => 'cl',
            COMPILERPATHS   => '%VS150COMNTOOLS%/../../VC/bin|%VCToolsInstallDir%/bin/Hostx86/x86',
            COMPILERPATH    => '',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '',
            RTLIBRARY       => '-MDd',
            CFLAGS          => '-nologo @RTLIBRARY@ -fp:precise',
            CXXFLAGS        => '-nologo @RTLIBRARY@ -EHsc -fp:precise',
            CDEBUG          => '-Zi -RTC1 -Od',
            CRELEASE        => '-O2 -GL -Gy -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo @RTLIBRARY@',
            LDDEBUG         => '-Zi -RTC1',
            LDRELEASE       => '-GL',
            LDMAPFILE       => '-MAP:$(MAPFILE)',
                        # -Fm:  if positioned before /link
                        # -MAP: if positioned afer /link

            MFCDIR          => '',
            MFCCFLAGS       => '-nologo @RTLIBRARY@',
            MFCCXXFLAGS     => '-nologo @RTLIBRARY@ -EHsc',
            MFCCOPT         => '-Zc:wchar_t- -Zc:forScope',
            MFCCXXOPT       => '-Zc:wchar_t- -Zc:forScope',
          # MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
          # MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            MFCCINCLUDE     => '',
            MFCLIBS         => ''
            },

       'vc1920'        => {     # 2019, Visual Studio 19.2x
            TOOLCHAIN       => 'vs160',
            TOOLCHAINEXT    => '.vs160',
            TOOLCHAINNAME   => 'Visual Studio 2019',
            CC              => 'cl',
            COMPILERPATHS   => '%VS160COMNTOOLS%/../../VC/bin|%VCToolsInstallDir%/bin/Hostx86/x86',
            COMPILERPATH    => '',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            RC              => 'rc -nologo',    # -nologo option, not /nologo
            CINCLUDE        => '',
            RTLIBRARY       => '-MDd',
            CFLAGS          => '-nologo @RTLIBRARY@ -fp:precise',
            CXXFLAGS        => '-nologo @RTLIBRARY@ -EHsc -fp:precise -Zc:offsetof-',
            CDEBUG          => '-Zi -RTC1 -Od',
            CRELEASE        => '-O2 -GL -Gy -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo @RTLIBRARY@',
            LDDEBUG         => '-Zi -RTC1',
            LDRELEASE       => '-GL',
            LDMAPFILE       => '-MAP:$(MAPFILE)',
                        # -Fm:  if positioned before /link
                        # -MAP: if positioned afer /link

            MFCDIR          => '',
            MFCCFLAGS       => '-nologo @RTLIBRARY@',
            MFCCXXFLAGS     => '-nologo @RTLIBRARY@ -EHsc',
            MFCCOPT         => '-Zc:wchar_t- -Zc:forScope',
            MFCCXXOPT       => '-Zc:wchar_t- -Zc:forScope',
          # MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
          # MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            MFCCINCLUDE     => '',
            MFCLIBS         => ''
            },

       'vc1930'        => {     # 2022, Visual Studio 19.3x
            TOOLCHAIN       => 'vs170',
            TOOLCHAINEXT    => '.vs170',
            TOOLCHAINNAME   => 'Visual Studio 2022',
            CC              => 'cl',
            COMPILERPATHS   => '%VS170COMNTOOLS%/../../VC/bin|%VCToolsInstallDir%/bin/Hostx86/x86',
            COMPILERPATH    => '',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '',
            RTLIBRARY       => '-MDd',
            CFLAGS          => '-nologo @RTLIBRARY@ -fp:precise',
            CXXFLAGS        => '-nologo @RTLIBRARY@ -EHsc -fp:precise -Zc:offsetof-',
            CDEBUG          => '-Zi -RTC1 -Od',
            CRELEASE        => '-O2 -GL -Gy -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo @RTLIBRARY@',
            LDDEBUG         => '-Zi -RTC1',
            LDRELEASE       => '-GL',
            LDMAPFILE       => '-MAP:$(MAPFILE)',
                        # -Fm:  if positioned before /link
                        # -MAP: if positioned afer /link

            MFCDIR          => '',
            MFCCFLAGS       => '-nologo @RTLIBRARY@',
            MFCCXXFLAGS     => '-nologo @RTLIBRARY@ -EHsc',
            MFCCOPT         => '-Zc:wchar_t- -Zc:forScope',
            MFCCXXOPT       => '-Zc:wchar_t- -Zc:forScope',
          # MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
          # MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            MFCCINCLUDE     => '',
            MFCLIBS         => ''
            },

        'wc1300'        => {    # Watcom 11
            TOOLCHAIN       => 'wc11',
            TOOLCHAINEXT    => '.wc11',
            TOOLCHAINNAME   => 'Watcom 11',
            CC              => 'wcl386',
            COMPILERPATH    => '%WATCOM%/binnt',
            VSWITCH         => '-c',
            VPATTERN        => '(Watcom .*? Version [0-9\.]+)',
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '',
            RTLIBRARY       => '-MTd',
            CFLAGS          => '-nologo -showwopts -passwopts:"-q -d2" -Yd @RTLIBRARY@',
            CXXFLAGS        => '-nologo -showwopts -passwopts:"-q -d2" -Yd @RTLIBRARY@',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-nologo -passwopts:"-q -d2" @RTLIBRARY@',
            LDMAPFILE       => '-MAP:$(MAPFILE)'
            },

        'owc1900'       => {    # Open Watcom 1.9
            TOOLCHAIN       => 'owc19',
            TOOLCHAINEXT    => '.owc19',
            TOOLCHAINNAME   => 'Open-Watcom 1.9',
            CC              => 'wcl386',
            COMPILERPATH    => '%WATCOM%/binnt',
            VSWITCH         => '-c',
            VPATTERN        => '(Open Watcom .*? Version [0-9\.]+)',
            ISWITCH         => '-i=',
            OSWITCH         => '-fo=',
            LSWITCH         => '',
            XSWITCH         => '-fe=',
            AR              => 'lib',
            RC              => 'rc -nologo',    # -nologo option, not /nologo
            CINCLUDE        => '',

                # -q        Operate quietly.
                # -6r       Register calling conventions.
                #               warning: dont not mix module calling conventions.
                # -j        Signed character (default unsigned).
                # -ei       Force enum base type to use at least an int;
                #               otherwise smallest data-type to fit data-range.
                # -ri       Return chars/shorts as ints; removed
                # -d2       Full symbolic debugging information
                #   -d2i        C++ only; d2 and debug inlines.
                # -hc       Generate Codeview debugging information.
                #   or -hw  Generate Watcomc debugging information.
                #   or -hd  Dwarf debugging information (perferred format).
                # -db       Generate browsing information (.mbr).
                # -o..      Optimization(s)
                #   f           -> generate traceable stack frames as needed
                #   f+          -> always generate traceable stack frames
                #   i           -> expand intrinsic functions inline
                #   r           -> reorder instructions for best pipeline usage
                #   u           -> all functions must have unique addresses
                #   x           -> max optimizations
                #
                # -zlf      Add default library information to objects.
                # -aa       Allow non-constant initialisation expressions.
                # -bt=nt    Build target Windows NT (or greater).
                # -bm       Multi-threaded environment.
                # -br       Build with dll run-time library.
                # -sg       Enable stack guard management (for functions with >= 4K of local variables).
                # -zc       The zc option causes the code generator to place literal strings and const items in the code segment.
                #
                # -cc++     Treat source files as C++ code.
                # -xs       Exception handling: balanced (C++).
                # -xr       Enable RTTI (C++).
                #
                # Others:
                # -za       Disable extensions, helps to ensure modules match the ANSI C programming language specification; NO_EXT_KEYS is predefined.
                # -za99     Enable C99 ANSI compliance (1).
                # -ecc      __cdecl (2).
                #
                # see:      wcpp386 -?
                #
                #   (1) Use with caution, beta undocumented feature and not 100% stable.
                #   (2) Avoid changing the call convention from #r/#s, otherwise runtime library issues.
                #
                # stdbool:  za99 mode _Bool use within C modules causes crashes, remap
                #
            CFLAGS          => '-q -6r -j -ei -db -zlf -bt=nt -bm -br -za99 -aa -sg -D_Bool=char',
            CXXFLAGS        => '-q -6r -j -ei -db -zlf -bt=nt -bm -br -cc++ -xs -xr',
            CDEBUG          => '-d2 -hd -of+ ',
            CXXDEBUG        => '-d2 -hd -od',   #d2/d3 under hw generates invalid symbols
            CRELEASE        => '-ox -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-q -6r -db -bt=nt -bm -br',
            LDDEBUG         => '-d2 -hd',
            LDRELEASE       => '',
            LDMAPFILE       => '-fm=$(MAPFILE)',

            # 7600.16385.1: Windows Driver Kit Version 7.1.0
            #   ==> http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=11800
            MFCDIR          => '/tools/WinDDK/7600.16385.1',
            MFCCFLAGS       => '-q -j -ei -6r -d2  -hd -db -ofr -zlf -bt=nt -bm -br -aa',
            MFCCXXFLAGS     => '-q -j -ei -6r -d2i     -db -ofr -zlf -bt=nt -bm -br -xs -xr -cc++',
            MFCCOPT         => '',
            MFCCXXOPT       => '',
            MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
            MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            },

        'owc1900_posix' => {    # Open Watcom 1.9 (using owcc)
            TOOLCHAIN       => 'owc19',
            TOOLCHAINEXT    => '.owc19',
            TOOLCHAINNAME   => 'Open-Watcom 1.9',
            CC              => 'owcc',
            COMPILERPATH    => '%WATCOM%/binnt',
            VSWITCH         => '-v',            # version
            VPATTERN        => '(Open Watcom .*? Version [0-9\.]+)',
            OSWITCH         => '-o',
            LSWITCH         => '-l',
            XSWITCH         => '-o',
            AR              => 'lib',
            RC              => 'rc -nologo',    # -nologo option, not /nologo
            CINCLUDE        => '',

                # -zq                           Operate quietly.
                # -mregparm=1                   Register calling conventions; warning: dont not mix module calling conventions.
                # -mtune=686                    Target CPU optimisation.
                # -fsigned-char                 Signed character (default unsigned).
                # -fno-short-enum               Force enum base type to use at least an int;
                #                               otherwise smallest data-type to fit data-range.
                # -g2                           Full symbolic debugging information.
                # -gwatcom                      Generate Codeview debugging information.
                # -fbrowser                     Generate browsing information (.mbr).
                # -O#                           Optimization(s).
                # -Wc,-aa                       Allow non-constant initialisation expressions (-fnonconst-initializers, owc 2.0)
                # -bt=NT                        Build target Windows NT (or greater).
                # -mthreads                     Multi-threaded environment.
                # -mrtdll                       Build with dll run-time library.
                # -fgrow-stack                  Enable stack guard management (for functions with >= 4K of local variables).
                # -zc                           The zc option causes the code generator to place literal strings and const items in the code segment.
                # -xc++                         Treat source files as C++ code.
                # -feh                          Exception handling.
                # -frtti                        Enable RTTI (C++).
                #
            CFLAGS          => '-zq -mconsole -mrtdll -mthreads -mregparm=1 -mtune=686 -fbrowser -fsigned-char -fno-short-enum -Wc,-aa -fgrow-stack',
            CXXFLAGS        => '-zq -mconsole -mrtdll -mthreads -mregparm=1 -mtune=686 -fbrowser -fsigned-char -fno-short-enum -xc++ -feh -frtti',
            CDEBUG          => '-gw -g2 -O0',
            CXXDEBUG        => '-gw -g2 -O0',
            CRELEASE        => '-O2 -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-zq -b NT -mconsole -mrtdll -mthreads -mregparm=1 -mtune=686 -fbrowser',
            LDDEBUG         => '-gw -g2',
            LDRELEASE       => '',
            LDMAPFILE       => '-fm=$(MAPFILE)',

            MFCDIR          => '/tools/WinDDK/7600.16385.1',
            MFCCOPT         => '-q -j -ei -6r -d2  -hd -db -ofr -zlf -bt=nt -bm -br -aa',               #TODO
            MFCCXXOPT       => '-q -j -ei -6r -d2i     -db -ofr -zlf -bt=nt -bm -br -xs -xr -cc++',     #TODO
            MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
            MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            },

        'owc2000'       => {    # Open Watcom 2.0
            TOOLCHAIN       => 'owc20',
            TOOLCHAINEXT    => '.owc20',
            TOOLCHAINNAME   => 'Open-Watcom 2.0',
            CC              => 'wcl386',
            COMPILERPATH    => '%WATCOM%/binnt',
            VSWITCH         => '-c',
            VPATTERN        => '(Open Watcom .*? Version [0-9\.]+)',
            ISWITCH         => '-i=',
            OSWITCH         => '-fo=',
            LSWITCH         => '',
            XSWITCH         => '-fe=',
            AR              => 'lib',
            RC              => 'rc -nologo',
            CINCLUDE        => '',

                # -q        Operate quietly.
                # -6r       Register calling conventions.
                #               warning: dont not mix module calling conventions.
                # -j        Signed character (default unsigned).
                # -ei       Force enum base type to use at least an int;
                #               otherwise smallest data-type to fit data-range.
                # -ri       Return chars/shorts as ints; removed
                # -d2       Full symbolic debugging information
                #   -d2i        C++ only; d2 and debug inlines.
                # -hc       Generate Codeview debugging information.
                #   or -hw  Generate Watcomc debugging information.
                #   or -hd  Dwarf debugging information.
                # -db       Generate browsing information (.mbr).
                # -o..      Optimization(s)
                #   f           -> generate traceable stack frames as needed
                #   f+          -> always generate traceable stack frames
                #   i           -> expand intrinsic functions inline
                #   r           -> reorder instructions for best pipeline usage
                #   u           -> all functions must have unique addresses
                #
                # -zlf      Add default library information to objects.
                # -aa       Allow non-constant initialisation expressions.
                # -bt=nt    Build target Windows NT (or greater).
                # -bm       Multi-threaded environment.
                # -br       Build with dll run-time library.
                # -sg       Enable stack guard management (for functions with >= 4K of local variables).
                # -zc       The zc option causes the code generator to place literal strings and const items in the code segment.
                #
                # -cc++     Treat source files as C++ code.
                # -xs       Exception handling: balanced (C++).
                # -xr       Enable RTTI (C++).
                #
                # Others:
                # -za       Disable extensions, helps to ensure modules match the ANSI C programming language specification; NO_EXT_KEYS is predefined.
                # -za99     Enable C99 ANSI compliance (1).
                # -ecc      __cdecl (2).
                #
                # see:      wcpp386 -?
                #
                #   (1) Use with caution, beta undocumented feature and not 100% stable.
                #   (2) Avoid changing the call convention from #r/#s, otherwise runtime library issues.
                #
            CFLAGS          => '-q -6r -j -ei -db -zlf -bt=nt -bm -br -za99 -aa -sg',
            CXXFLAGS        => '-q -6r -j -ei -db -zlf -bt=nt -bm -br -cc++ -xs -xr',
            CDEBUG          => '-d2 -hd -of+',
        ##  CXXDEBUG        => '-d2i -hd -od',
            CXXDEBUG        => '-d2 -hd -od',
            CRELEASE        => '-ox -DNDEBUG',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDFLAGS         => '-q -6r -db -bt=nt -bm -br',
            LDDEBUG         => '-d2 -hd',
            LDRELEASE       => '',
            LDMAPFILE       => '-fm=$(MAPFILE)',

            # not-supported
            MFCDIR          => '/tools/WinDDK/7600.16385.1',
            MFCCOPT         => '-q -j -ei -6r -d2  -hd -db -ofr -zlf -bt=nt -bm -br -aa',
        ##  MFCCXXOPT       => '-q -j -ei -6r -d2i     -db -ofr -zlf -bt=nt -bm -br -xs -xr -cc++',
            MFCCXXOPT       => '-q -j -ei -6r -d2      -db -ofr -zlf -bt=nt -bm -br -xs -xr -cc++',
            MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
            MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            }
        );

my %win_entries     = (
        SET_MAKE            => 'MAKEFLAGS=',
        RM                  => '@BINPATH@rm.exe',
        MV                  => '@BINPATH@mv.exe',
        CP                  => '@BINPATH@cp.exe',
        TAR                 => '@BINPATH@tar.exe',
        MKDIR               => '@BINPATH@mkdir.exe',
        MKDIR_P             => '@PERLPATH@perl '."${CWD}/support/mkdir_p.pl",
        RMDIR               => '@BINPATH@rmdir.exe',

        ISWIN32             => 'yes',
        ISWIN64             => 'no',
        PATHSEP             => ';',
        DEFS                => '-DHAVE_CONFIG_H -DWIN32=0x501',

        INSTALL             => '@PERLPATH@perl '."${CWD}/win32/install.pl",
        INSTALL_PROGRAM     => '@PERLPATH@perl '."${CWD}/win32/install.pl",
        INSTALL_DATA        => '@PERLPATH@perl '."${CWD}/win32/install.pl",

        RANLIB              => '@echo',
        YACC                => '@BINPATH@bison -y',
        LEX                 => '@BINPATH@flex',
        GREP                => '@BINPATH@egrep',
        AWK                 => '@BINPATH@awk',
        SED                 => '@BINPATH@sed',
        PERL                => '@PERLPATH@perl',
        LIBTOOL             => '@PERLPATH@perl '.'$<LIBTOOL>',
        CPPDEP              => '',
        LT_OBJDIR           => '.libs/',
        RC                  => 'rc /nologo',

        LIBS                => '',
        EXTRALIBS           => 'advapi32.lib gdi32.lib'.
                                  ' shlwapi.lib shell32.lib psapi.lib ole32.lib'.
                                  ' userenv.lib user32.lib ws2_32.lib wsock32.lib',
        LIBMALLOC           => 'libdlmalloc.lib',
        LIBOPENSSL          => ''
        );

my %x_tokens        = (
        #host, build etc
        PACKAGE             => '',
        PACKAGE_BUGREPORT   => '',
        PACKAGE_NAME        => '',
        PACKAGE_STRING      => '',
        PACKAGE_TARNAME     => '',
        PACKAGE_URL         => '',
        PACKAGE_VERSION     => '',
        PATH_SEPARATOR      => ';',

        build               => 'i386-pc-win32',
        build_alias         => '',
        build_cpu           => 'i386',
        build_os            => 'win32',
        build_vendor        => 'pc',

        host                => 'i386-pc-win32',
        host_alias          => '',
        host_cpu            => 'i386',
        host_os             => 'win32',
        host_vendor         => 'pc',

        target              => 'i386-pc-win32',
        target_alias        => '',
        target_cpu          => 'i386',
        target_os           => 'win32',
        target_vendor       => 'pc',

        prefix              => '',
        exec_prefix         => '',
        datarootdir         => '',

        bindir              => '',              # WIN32 implied, see edconfig.h
        sbindir             => '',
        libexecdir          => '',
        libdir              => '',
        datadir             => '',
        includedir          => '',

        ABS_ROOT            => $CWD,
        abs_top_builddir    => $CWD,
        top_build_prefix    => '',
        top_builddir        => '',
        top_srcdir          => '',

        VSWITCH             => '-v',            # e.g. gcc version 4.5.3 (GCC)
        VPATTERN            => 'version ([0-9\.]+)',
        ISWITCH             => '-I',
        OSWITCH             => '-o',            # object specification
        LSWITCH             => '-l',            # library
        XSWITCH             => '-o',            # exec specification

        #makefile
        SET_MAKE            => 'MAKEFLAGS=',
        LD                  => 'link',
        CP                  => 'cp',
        RM                  => 'rm',
        MV                  => 'mv',
        TAR                 => 'tar',
        MKDIR               => 'mkdir',
        MKDIR_P             => 'mkdir -p',
        RMDIR               => 'rmdir',

        INSTALL             => 'install.pl',
        INSTALL_PROGRAM     => 'install.pl',
        INSTALL_DATA        => 'install.pl',

        RANLIB              => 'ranlib',
        YACC                => 'bison -y',
        GREP                => 'egrep',
        AWK                 => 'awk',
        SED                 => 'sed',
        WGET                => 'wget',          # special
        BUSYBOX             => 'busybox',       # special
        INNO                => 'C:/Program Files (x86)/Inno Setup 5/Compil32',
        PERL                => "${PERLPATH}perl",

        LIBTOOL             => 'libtool',
        LIBTOOL_DEPS        => '',
            # used to automaticlly update the libtool script if it becames out-of-date.

    #   CC                  => '',
    #   CXX                 => '',
    #   CLD                 => '',
    #   CXXLD               => '',

        CFLAGS              => '',
        CXXFLAGS            => '',

        CDEBUG              => '',
        CRELEASE            => '',
        CWARN               => '',

        CXXDEBUG            => '',
        CXXRELEASE          => '',
        CXXWARN             => '',

        DEFS                => '-DHAVE_CONFIG_H',
        CINCLUDE            => '',
        CXXINCLUDE          => '',

    #   EXTRA_CFLAGS        => '',
    #   EXTRA_CXXFLAGS      => '',

        LIBCURL_CPPFLAGS    => '',

        CURSES_CFLAGS       => '',

        LIBICU_CFLAGS       => '',
        LIBICU_CXXFLAGS     => '',
        LIBICU_VERSION      => '',
        ICU_CONFIG          => '',

        LDFLAGS             => '',
        LDDEBUG             => '',
        LDRELEASE           => '',
        LIBS                => '',
        EXTRALIBS           => '',

        LIBENCA             => '',
        LIBSPELL            => '',
        LIBYACC             => '',
        LIBICU              => '',
        LIBICONV            => '',
        LIBCURL             => '',
        LIBCLANG            => '',
        LIBEXPLAIN          => '',
        LIBMAGIC            => '',
        LIBARCHIVE          => '',
        LIBZ                => '',
        LIBBZ2              => '',
        LIBLZMA             => '',
        LIBREGEX            => '',
        TERMLIB             => '',
        LIBM                => '',
        LIBX11              => '',
        LIBMALLOC           => '',
        LIBTHREAD           => ''
        );

my %x_tokendefs     = (
        CXX                 => 'CC',
        CXXFLAGS            => 'CFLAGS',
        CXXDEBUG            => 'CDEBUG',
        CXXRELEASE          => 'CRELEASE'
    #   CLD                 => 'CC',
    #   CXXLD               => 'CXX',
        );

my @x_headers       = (     #headers
        'sys/types.h',
        'sys/cdefs.h',
        'sys/bsdtypes.h',
        'sys/param.h',
        'sys/socket.h',
        'sys/time.h',
        'sys/select.h',
        'sys/wait.h',
        'sys/mman.h',
        'sys/utime.h',
        'sys/timeb.h',
        'sys/mount.h',
        'sys/stat.h',
        'sys/statfs.h',
        'sys/statvfs.h',
        'sys/vfs.h',
        'stdarg.h',
        'stdlib.h',
        'stdio.h',
        'stddef.h',
        'limits.h',
        'inttypes.h',                           # c99
        'stdint.h',                             # c99
        'stdbool.h',                            # c99
        'stdatomic.h',                          # c11
        'stdalign.h',                           # c11
        'stdckdint.h', 'intsafe.h',             # integer maths (gnu/win32)
        'threads.h',                            # c11
        'pthread.h',                            # MINGW
        'string.h', 'strings.h',
        'errno.h',
        'locale.h',                             # setlocale()
        'wchar.h', 'wctype.h',
        'time.h',                               # TIME_WITH_SYS_TIME
        'alloca.h',                             # alloca()
        'env.h',
        'fcntl.h',
        'fenv.h',
        'math.h',                               # for isnan() etc
        'float.h',
        'poll.h',
        'io.h',
        'memory.h',
        'process.h',
        'libgen.h',                             # basename(), dirname()
        'share.h',
        'signal.h',
        'utime.h',
        'wait.h',

        'getopt.h',
        'unistd.h',
        'dirent.h',
        'dlfcn.h',                              # dlopen()
        'pwd.h',
        'grp.h',
        'langinfo.h'
        );

my @x_headers2      = (     #headers; check only
        'thr/xthreads.h',                       # MSVC +2017, almost C11
        'xthreads.h',
        'ntifs.h',                              # SDK; optional
        'ntdef.h',
        'windows.h',
        'wincrypt.h',
        'bcrypt.h',
        'intrin.h',
        'afunix.h'                              # AF_UNIX
        );

my @x_predefines    = (
        '_MSC_VER|_MSC_FULL_VER',
        '_WIN32|_WIN64',
        '__WATCOMC__',
        '__GNUC__|__GNUC_MINOR__',
        '__MINGW32__|__MINGW64__|__MINGW64_VERSION_MAJOR|__MINGW64_VERSION_MINOR',
        '__STDC__|__STDC_VERSION__',
        '_M_IX86|_M_IA64|_M_X64|_M_AMD64|_M_ARM',
        '_WIN32_WINNT',
        'cpp=__cplusplus',
        'cpp=__STDC_HOSTED__',
        'cpp=__STDC_NO_ATOMICS__',
        );

my @x_decls         = (     #stdint/intypes.h
        'SIZE_MAX',
        'RSIZE_MAX',
        'SSIZE_MAX',
        'INT16_C',
        'INT16_MIN',
        'INT16_MAX',
        'UINT16_MAX',
        'INT32_C',
        'INT32_MIN',
        'INT32_MAX',
        'UINT32_MAX',
        'INT64_C',
        'INT64_MIN',
        'INT64_MAX',
        'UINT64_MAX',
        'INTPTR_MIN',
        'INTPTR_MAX',
        'UINTPTR_MAX',
        'WCHAR_MIN',
        'WCHAR_MAX',
        'INTMAX_MIN',
        'INTMAX_MAX',
        'UINTMAX_MAX'
        );

my @x_types         = (     #stdint/inttypes/types.h
        'inline',
        '__inline',
        '__forceinline',
        '__int8',
        '__int16',
        '__int32',
        '__int64',
        'intmax_t',
        'uintmax_t',
        'intptr_t',
        'uintptr_t',
        'ptrdiff_t',
        'long double',
        'long long int',
        'unsigned long long int',
        'int8_t',
        'int16_t',
        'int32_t',
        'int64_t',
        'uint8_t',
        'uint16_t',
        'uint32_t',
        'uint64_t',
        'uint_fast8_t',
        'uint_fast16_t',
        'uint_fast32_t',
        'uint_fast64_t',
        'wchar_t',
        'mbstate_t',
        'char16_t',
        'char32_t',
        'bool',
        '_Bool:C99BOOL',
        '_bool',
        'rsize_t',
        'ssize_t',
        'struct option.name;getopt.h,unistd.h'
        );

my @x_sizes         = (
        'char',
        'short',
        'int',
        'long',
        'long long',
        'float',
        'double',
        'wchar_t',
        'void_p',
        'time_t'
        );

#TODO
# HAVE_DECL_ENVIRON:extern char **environ;
# HAVE_DECL__ENVIRON:extern char **_environ;
#   unix: <unistd.h> if the _GNU_SOURCE
#   win32: <stdlib.h>

my @x_functions     = (
        'putenv',
        'setenv',
        'rename',
        'bcopy', 'bcmp', 'bzero', 'explicit_bzero',
        'memcmp', 'memset', 'memmove',
            'memset_s',                         # __STDC_WANT_LIB_EXT1__
            'SecureZeroMemory',
        'memccpy', '_memccpy',                  # bsd/msvc
        'index', 'rindex',                      # bsd
        'strcasecmp', '__strcasecmp', 'stricmp',
        'strncasecmp', '__strncasecmp', 'strnicmp',
        'strnlen',
        'strerror',
        'strftime',
        'strchr', 'strrchr', 'strdup',
        'strlcpy', 'strlcat',                   # bsd/linux
            'strsep', 'strnstr', 'strcasestr', 'strcasestr_l', 'strtonum',
        'strtof', 'strtold',
        'strtoll', 'strtoul', 'strtoull',
        'strtok_r',
        'sprintf_s', 'wsprintf_s',
        'strverscmp', '__strverscmp',
        'mkdtemp',                              # bsd/linux
        'getw', 'putw',
        'err',                                  # bsd/linux: err, warn etc
             'setprocname', 'getprocname',
        'setproctitle',                         # bsd
        'alloca',
        '_alloca',                              # msvc
        'reallocarray', 'stringlist',           # bsd
        'isascii',
        '__isascii',                            # msvc
        'isblank',
        '__isblank',                            # msvc
        'iscsym',
        '__iscsym',                             # msvc
        'printf', 'vprintf', 'doprnt',
        'snprintf', '_snprintf', 'vsnprintf', '_vsnprintf',
        'strrchr', 'strdup',
        'asnprintf', 'vasnprintf',
        'setlocale',
        'mbrtowc', 'wcrtomb', 'wcsrtombs', 'wcstombs', 'wcscmp', 'wcscpy', 'wcslen', 'wctomb',
                'wmemcmp', 'wmemmove', 'wmemcpy',
        'wcwidth',
        '_tzset',                               # msvc
        'fgetpos', 'fsetpos',
        'fseeko', 'fgetln',                     # bsd/linux extensions
        'truncate', 'ftruncate',
        'getline', 'getdelim',                  # bsd/linux
        'ctime_r', 'localtime_r', 'gmtime_r', 'asctime_r', #posix.1
              'gmtime_s',                       # __STDC_WANT_LIB_EXT1__
              '_get_timezone',
        'mktime',
        'timegm',                               # bsd/linux extensions
        'feclearexpect',                        # fenv.h/c99
        'fpclassify',                           # math.h/c99
            'isnan', '_isnan',
            'isinf', '_isinf',
            'isfinite', '_isfinite',
            'finite', '_finite',
        'round',                                # c99
        'nearbyintf',
        'va_copy', '__va_copy',                 # c99/gnu
        'opendir',
        'mktemp', 'mkstemp',
        'findfirst', '_findfirst',              # msvc
        'getopt', 'getopt_long',                # bsd/compat
        'nl_langinfo'
        );

my @x_commands     = (     # commands explicity converted to <cmd>.exe
        'mkdir',
        'rmdir',
        'tar',
        'mv',
        'cp'
        );

my %CONFIG_O        = (     # optional config.h values
        HAVE_EIGHTBIT           => '1'
        );

our %CONFIG_H       = (     # predefined config.h values
        IS_LITTLE_ENDIAN        => '1',         # TODO
        STDC_HEADERS            => '1',
        HAVE_EIGHTBIT           => '1',
        HAVE_ENVIRON            => '1',
        HAVE_SYSERRLIST         => '1'
        );

my $config          = undef;                    # Loaded configuration

our @HEADERS        = ();
our @EXTHEADERS     = ();
our %DECLS          = ();
our %DECLSVALUE     = ();
our %TYPES          = ();
our %SIZES          = ();
our %FUNCTIONS      = ();
our %LIBRARIES      = ();

my @INCLUDES        = ();
my @LIBS            = ();
my @EXTRALIBS       = ();
my @DLLS            = ();

my $x_workdir       = '.makelib';
my $x_tmpdir        = undef;
my $x_compiler      = '';
my $x_version       = '';

my @x_include       = ();
my @x_sysinclude    = ();
my $x_command       = '';
my $x_signature     = undef;

my $o_makelib       = './makelib.in';
my $o_keep          = 0;
my $o_verbose       = 0;
my $o_summary       = 1;
my $o_gnuwin32      = 'auto';
my $o_contrib       = 1;
my $o_gnulibs       = 0;

my $o_icu           = 'auto';
my $o_libhunspell   = undef;
my $o_libarchive    = undef;
my $o_libmagic      = undef;

my $o_help          = 0;


#   Main ---
#       Mainline
#
sub Configure($$$);
sub ExeRealpath($);
sub LoadContrib($$$$$);
sub CheckCompiler($$);
sub CheckVAARGS();
sub CheckHeader($$);
sub CheckDecl($$$);
sub CheckType($$;$);
sub CheckSize($$);
sub CheckFunction($$;$);
sub CheckICUFunction($);
sub CheckCommand($$;$$);
sub CheckExec($$;$$);
sub ExpandENV($);
sub System($);
sub systemrcode($);
sub DumpList($$);
sub ExportPath($);
sub ImportDLL($$;$$);
sub Makefile($$$);
sub MakefileDir($);
sub Config($$$);

exit &main();

sub
main()
{
    my $o_version = undef;
    my $o_clean = 0;

    my $ret
        = GetOptions(
                'binpath=s'     => \$BINPATH,
                'perlpath=s'    => \$PERLPATH,
                'bison=s'       => \$BISON,
                'flex=s'        => \$FLEX,
                'busybox=s'     => \$BUSYBOX,
                'mfcdir=s'      => \$MFCDIR,
                'wget=s'        => \$WGET,
                'inno=s'        => \$INNO,
                'version=i'     => \$o_version,
                'icu=s'         => \$o_icu,
                'gnuwin32=s'    => \$o_gnuwin32,
                'gnulibs'       => \$o_gnulibs,
                'contrib'       => \$o_contrib,
                'makelib'       => \$o_makelib,
                'libtool'       => \$LIBTOOL,
                'libhunspell=s' => \$o_libhunspell,
                'libarchive=s'  => \$o_libarchive,
                'libmagic=s'    => \$o_libmagic,
                'clean'         => \$o_clean,
                'verbose'       => sub {++$o_verbose;},
                'keep'          => \$o_keep,
                'help'          => \$o_help,
                'help-options'  => sub {$o_help = 2;}
                );

    Usage() if (!$ret || $o_help);
    Usage("expected command") if (scalar @ARGV < 1);
    Usage("unexpected arguments $ARGV[1] ...") if (scalar @ARGV > 1);

    (-f $o_makelib) or
        Usage("missing makelib.in");

    #   See: https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B
    #
    #   MSVC++ 9.0   _MSC_VER == 1500 (Visual Studio 2008 version 9.0)
    #   MSVC++ 10.0  _MSC_VER == 1600 (Visual Studio 2010 version 10.0)
    #   MSVC++ 11.0  _MSC_VER == 1700 (Visual Studio 2012 version 11.0)
    #   MSVC++ 12.0  _MSC_VER == 1800 (Visual Studio 2013 version 12.0)
    #   MSVC++ 14.0  _MSC_VER == 1900 (Visual Studio 2015 version 14.0)
    #   MSVC++ 14.1x _MSC_VER == 1910 (Visual Studio 2017 version 15.x)
    #   MSVC++ 14.2x _MSC_VER == 192x (Visual Studio 2019 version 16.x)
    #   MSVC++ 14.3x _MSC_VER == 193x (Visual Studio 2022 version 17.x)
    #
    my ($cmd, $options)                         # posix,x64
        = split(/-/, $ARGV[0], 2);

    if    ('vc12' eq $cmd)      { $o_version = 1200, $cmd = 'vc'  }
    elsif ('vc14' eq $cmd)      { $o_version = 1400; $cmd = 'vc'  } elsif ('vc2005' eq $cmd) { $o_version = 1400; $cmd = 'vc' }
    elsif ('vc15' eq $cmd)      { $o_version = 1400; $cmd = 'vc'  } elsif ('vc2008' eq $cmd) { $o_version = 1500; $cmd = 'vc' }
    elsif ('vc16' eq $cmd)      { $o_version = 1600; $cmd = 'vc'  } elsif ('vc2010' eq $cmd) { $o_version = 1600; $cmd = 'vc' }
    elsif ('vc18' eq $cmd)      { $o_version = 1800; $cmd = 'vc'  } elsif ('vc2013' eq $cmd) { $o_version = 1800; $cmd = 'vc' }
    elsif ('vc19' eq $cmd)      { $o_version = 1900; $cmd = 'vc'  } elsif ('vc2015' eq $cmd) { $o_version = 1900; $cmd = 'vc' }
    elsif ('vc1910' eq $cmd)    { $o_version = 1910; $cmd = 'vc'  } elsif ('vc2017' eq $cmd) { $o_version = 1910; $cmd = 'vc' }
    elsif ('vc1920' eq $cmd)    { $o_version = 1920; $cmd = 'vc'  } elsif ('vc2019' eq $cmd) { $o_version = 1920; $cmd = 'vc' }
    elsif ('vc1930' eq $cmd)    { $o_version = 1930; $cmd = 'vc'  } elsif ('vc2022' eq $cmd) { $o_version = 1930; $cmd = 'vc' }
    elsif ('owc19' eq $cmd)     { $o_version = 1900; $cmd = 'owc' }
    elsif ('owc20' eq $cmd)     { $o_version = 2000; $cmd = 'owc' }
    elsif ('mingw' eq $cmd)     { $o_version = 0;    $cmd = 'mingw' }
    elsif ('mingw32' eq $cmd)   { $o_version = 32;   $cmd = 'mingw' }
    elsif ('mingw64' eq $cmd)   { $o_version = 64;   $cmd = 'mingw' }

    if (! $o_version) { # default versions
        if ($cmd eq 'vc')       { $o_version = 1400; } # review???
        elsif ($cmd eq 'wc')    { $o_version = 1300; }
        elsif ($cmd eq 'owc')   { $o_version = 1900; }
        else { $o_version = 0; }
    }

    $o_version .= '_x64'
        if ($options && $options =~ /x64/);

    if ($cmd eq 'vc' ||
            $cmd eq 'owc' || $cmd eq 'wc' ||
            $cmd eq 'dj' ||  $cmd eq 'mingw') {

        my $cache = "${x_workdir}/${cmd}${o_version}.cache";

        if (! $o_clean && -f $cache) {
            eval {
                print "loading <${cache}> ...\n";
                do "./${cache}";
            };
        };

        #build
        Configure($cmd, $o_version, $options);
        foreach (@{$config->{MAKEFILES}}) {
            Makefile($cmd, $_, 'Makefile');
        }
        Makefile($cmd, $config->{PACKAGE_PATH}, $config->{PACKAGE_FILE});
        Config($cmd, $config->{CONFIG_PATH}, $config->{CONFIG_FILE});

        #cache
        open(CACHE, ">${cache}") or
                die "cannot create <$cache> : $!\n";
        $Data::Dumper::Purity = 1;
        $Data::Dumper::Sortkeys = 1;
        print CACHE Data::Dumper->Dump([\%x_tokens],   [qw(*XXTOKENS)]);
        print CACHE Data::Dumper->Dump([\@HEADERS],    [qw(*XXHEADERS)]);
        print CACHE Data::Dumper->Dump([\@EXTHEADERS], [qw(*XXEXTHEADERS)]);
        print CACHE Data::Dumper->Dump([\%CONFIG_H],   [qw(*CONFIG_H)]);
        print CACHE Data::Dumper->Dump([\%DECLS],      [qw(*DECLS)]);
        print CACHE Data::Dumper->Dump([\%DECLSVALUE], [qw(*DECLSVALUE)]);
        print CACHE Data::Dumper->Dump([\%TYPES],      [qw(*TYPES)]);
        print CACHE Data::Dumper->Dump([\%SIZES],      [qw(*SIZES)]);
        print CACHE Data::Dumper->Dump([\%FUNCTIONS],  [qw(*FUNCTIONS)]);
        print CACHE Data::Dumper->Dump([\%LIBRARIES],  [qw(*LIBRARIES)]);
        print CACHE "1;\n";
        close CACHE;

        #summary
        DumpList('INCLUDES', \@INCLUDES);
        DumpList('LIBS',     \@LIBS);
        DumpList('EXTRALIB', \@EXTRALIBS);
        DumpList('DLLS',     \@DLLS);

    } elsif ($cmd eq 'clean') {
        my %env;

        MakelibProfile();
        foreach (@{$config->{MAKEFILES}}) {
            my $dir = MakefileDir($_);
            unlink "$dir/Makefile";
        }

    } else {
        Usage("unknown command '$cmd'");
        return 1;
    }

    return 0;
}


sub
ProgramFiles
{
    my $path = $ENV{ProgramFiles};
    $path =~ s/\\/\//g
        if ($path);
    return $path;
}



#   Usage ---
#       Makelib command line usage.
#
sub
Usage                   # (message)
{
    if ($o_help != 2) {
        print "\nmakelib @_\n\n" if (@_);
        print <<EOU;

Usage: perl makelib.pl [options] <command>

Options:
EOU
    }

    print <<EOU;

    --help                  Command line help.

    --libtool=<path>        Path to libtool_win32.pl.

    --binpath=<path>        Path to coreutils, otherwise these are assumed to be in the path.

    --perlpath=<path>       Perl binary path, otherwise assumed in the path.

    --gnuwin32=<path>       gnuwin32 g++ tool installation path; see --gnulibs

    --contib                Enable local contrib libraries (default).
    or --gnulibs            Search and enable gnuwin32 libraries, using gnuwin32 path.

    --version=<version>     compiler version

    --clean                 clean build, ignoring cache.

    --keep                  keep temporary file images.

Configuration:

    --libarchive=<path>     libarchive installation path.
    --libmagic=<path>       libmagic installation path.
    --icu=<path>            ICU installation path.

    --busybox=<path>        busybox-w32 installation path.
    --bison=<path>          yacc/bison installation path.
    --flex=<path>           flex installation path.
    --inno=<path>           inno-setup installation path.

Toolchain / command:

    vc[20xx]                Visual Studio C/C++ Makefiles.
    wc                      Watcom C/C++, using 'cl' interface.
    owc                     Open Watcom C/C++, using a direct interface.
    dj                      DJGPP.
    clean                   Clean.

EOU
    exit(42);
}


#   Config ---
#       Configuration.
#
sub
Configure($$$)          # (type, version, options)
{
    my ($type, $version, $options) = @_;
    my @CONTRIBINCS = ();
    my @EXTERNINCS = ();

    # paths
    if ($BINPATH) {
        $BINPATH = realpath($BINPATH);
        print "binpath:  ${BINPATH}\n";
        $BINPATH .= '/';
    }

    if ($PERLPATH) {
        $PERLPATH = realpath($PERLPATH);
        print "perlpath: ${PERLPATH}\n";
        $PERLPATH .= '/';
    }

    if ($BUSYBOX) {
        $BUSYBOX = ExeRealpath($BUSYBOX)
            if ($BUSYBOX ne 'busybox');
        print "busybox:  ${BUSYBOX}\n";
        $win_entries{BUSYBOX} = $BUSYBOX;
    } else {
        $BUSYBOX = 'busybox'                    # default
            if (! defined $BUSYBOX);
    }

    if ($WGET) {
        $WGET = ExeRealpath($WGET)
            if ($WGET ne 'wget');
        print "wget:     ${WGET}\n";
        $win_entries{WGET} = $WGET;
    }

    if ($INNO) {
        $INNO = ExeRealpath($INNO)
            if ($INNO ne 'inno');
        print "inno:     ${INNO}\n";
        $win_entries{INNO} = $INNO;
    }

    if ($BISON) {                               # override
        $BISON = ExeRealpath($BISON);
        if ($BISON =~ /bison$/i) {
            print "bison:    ${BISON}\n";
            $win_entries{YACC} = "${BISON} -y";
        } else {
            print "YACC:     ${BISON}\n";
            $win_entries{YACC} = "${BISON}";
        }
    }

    if ($FLEX) {                                # override
        $FLEX = ExeRealpath($FLEX);
        print "flex:     ${FLEX}\n";
        $win_entries{LEX} = "${FLEX}";
    }

    if (! $LIBTOOL) {                           # derive libtool location
        if (-f "${CWD}/win32/libtool_win32.pl") {
            $LIBTOOL = "${CWD}/win32/libtool_win32.pl";
        } elsif (-f "${CWD}/libtool_win32.pl") {
            $LIBTOOL = "${CWD}/libtool_win32.pl";
        } else {
            $LIBTOOL = "libtool_win32.pl";
        }
    }

    # environment
    my $signature =                             # ie. vc1600
        ($version ? sprintf("%s%s", $type, $version) : $type);

    $signature .= "_posix"
        if ($options && $options =~ /posix/);

    if (! exists $x_environment{$signature}) {
         if ($signature =~ /(.*)_x64(.*)/) {    # derive x64 profile
            my $base  = $1.$2;
            if (exists $x_environment{$base}) {
                $x_environment{$signature} = $x_environment{$base};
                $x_environment{$signature}->{TOOLCHAIN} .= '_x64';
                $x_environment{$signature}->{TOOLCHAINEXT} .= '/x64';
                $x_environment{$signature}->{ISWIN64} .= 'yes';
                $x_environment{$signature}->{ISWIN32} .= 'no';
            }
        }
    }

    (exists $x_environment{$signature}) or
        die "makelib:  unknown environment ${type}, version ${version} <${signature}>\n";

    $x_signature = $signature;                  # active environment
    my $env = $x_environment{$signature};

    if ('dj' ne $type) {                        # WIN32 default profile
        foreach my $entry (keys %win_entries) {
            my $token = $win_entries{$entry};

            $token =~ s/\$\<LIBTOOL>/${LIBTOOL}/;
            $x_tokens{$entry} = $token;
        }
    }

    # makelib configuration
    foreach my $entry (keys %$env) {            # target profile
        $x_tokens{$entry} = $$env{$entry};
    }
    if ($MFCDIR) { #override
        print "MFCDIR:   ${MFCDIR}\n";
        $x_tokens{MFCDIR} = $MFCDIR;
    }

    MakelibConfigure($type, $env);

    # toolchain dynamic configuration
    (-d $x_workdir || mkdir($x_workdir)) or
        die "makelib: unable to access/create workdir <$x_workdir> : $!\n";

    $x_tmpdir = "${x_workdir}/${type}${version}";

    (-d $x_tmpdir || mkdir($x_tmpdir)) or
        die "makelib: unable to access/create tmpdir <$x_tmpdir> : $!\n";

    CheckCompiler($type, $env);
    CheckVAARGS();

    # modules
    if ($o_gnuwin32 && ('auto' eq $o_gnuwin32)) {
        $o_gnuwin32 = undef;                    # TODO - search path.
    }

    if ($o_icu && ('auto' eq $o_icu)) {
        if (-d "contrib/icu/makelib.def") {     # local contrib
            $o_icu = undef;
        } else {
            my @icupkg = (
                    "pkg/icu",
                    "icu" );
            my @icudir = (
                    "./",
                    "./../",
                    "./../../",
                    "./../../../" );

            $o_icu = undef;
            foreach my $dir (@icudir) {
                foreach my $pkg (@icupkg) {
                    my $icu = $dir.$pkg;
                    if (-d $icu."./include") {
                        $o_icu = realpath($icu);
                        print "icuauto:  $o_icu\n";
                        last;
                    }
                }
                last if ($o_icu);
            }
        }
    }

    # header
    my @INCLUDE = ();

    push @INCLUDE, @x_include;                  # additional search directories
    push @INCLUDE, split(/;/, $x_tokens{INCLUDE}); # environment INCLUDE
    push @INCLUDE, @x_sysinclude;               # toolchain etc

    print "Scanning: @INCLUDE\n"
        if ($o_verbose);

    my $headerdefines = "";
    my $idx = -1;
    foreach my $header (@x_headers, @x_headers2) {
        my $headers2 = (++$idx >= scalar @x_headers);
        my $fullpath = "";
        my $include = "";
        my $check = -1;

        my $name = $header;
        $name =~ s/[\/.]/_/g;
        $name = "HAVE_".uc($name);              # HAVE_XXXX_H

        my $cached = (exists $CONFIG_H{$name});

        # present check
        print "header:   ${header} present ...";
        print " " x (28 - (length($header)+8));
        foreach $include (@INCLUDE) {
            $fullpath = "${include}/${header}";
            $fullpath =~ s/\\/\//g;
            last if (-f $fullpath);
            $fullpath = "";
        }
        print "[".($fullpath ? "yes" : "no")."] <$fullpath>\n";

        # usability check
        print "header:   ${header} usability ... ";
        print " " x (28 - (length($header)+11));
        if ($headers2) {
            $check = ($fullpath ? 0 : -1);      # found?
        } else {
            $check = ($cached ? 0 : CheckHeader($header, $headerdefines)); # build check
        }
        print "[".(0 == $check ? "yes" : "no").($cached?", cached":"")."]\n";

        if (0 == $check) {
            if ($headers2) {                    # headers2
                push @EXTHEADERS, $header;
            } else {
                push @HEADERS, $header;
                push @EXTHEADERS, $header
                    if ($include ne $x_libw32);
            }

            $CONFIG_H{$name} = '1';
            $headerdefines .= "#define ${name} 1\n";
        }
    }

    # predefines/decls
    foreach my $t_declspec (@x_predefines, @x_decls) {
        my $cpp = 0;

        if ($t_declspec =~ /^cpp=(.+)$/) {      # cpp prefix
            $t_declspec = $1;
            $cpp = 1;
        }

        my @predefines = split('\|', $t_declspec);
        foreach my $declspec (@predefines) {
            my $name   = $declspec;
            my $define = uc($declspec);

            $define =~ s/ /_/g;
            if ($declspec =~ /^(.+):(.+)$/) {
                $name   = $1;
                $define = $2;                   # optional explicit #define
            }

            my $cached = (exists $DECLS{$name});
            my $status = ($cached ? $DECLS{$name} : -1);
            my $value  = ($cached ? $DECLSVALUE{$name} : "");

            print "decl:     ${name} ...";
            print " " x (28 - length($name));

            if (-1 == $status) {
                $value = CheckDecl($type, $name, $cpp);
                $status = 1
                    if ($value ne "");
            }

            if (1 == $status) {
                $DECLS{$name} = 1;
                $CONFIG_H{"HAVE_DECL_${define}"} = 1;
                if ($cached) {
                    print "${define}=${value} [yes, cached]";
                } else {
                    $DECLSVALUE{$name} = $value;
                    print "[yes]";
                }
            } else {
                $DECLS{$name} = 0;
                print ($cached ? "[no, cached]" : "[no]");
            }
            print "\n";
        }
    }

    # types
    foreach my $typespec (@x_types) {
        my $field  = '';
        if ($typespec =~ /^([^.]+)\.(.+)$/) {   # struct name.field
            $typespec = $1;
            $field = $2;
        }
        my $name   = $typespec;
        my $define = uc($typespec);

        $define =~ s/ /_/g;                     # eg. "struct option" ==> STRUCT_OPTION
        if ($typespec =~ /^(.+):(.+)$/) {
            $name   = $1;
            $define = $2;                       # optional explicit #define
        }

        my $cached = (exists $TYPES{$name});
        my $status = ($cached ? $TYPES{$name} : -1);

        print "type:     ${name} ...";
        print " " x (28 - length($name));

        if (1 == $status ||
                (-1 == $status && 0 == CheckType($type, $name, $field))) {
            $TYPES{$name} = 1;
            $CONFIG_H{"HAVE_${define}"} = 1;
            print ($cached ? "[yes, cached]" : "[yes]");
        } else {
            $TYPES{$name} = 0;
            print ($cached ? "[no, cached]" : "[no]");
        }
        print "\n";
    }

    # size
    foreach my $name (@x_sizes) {
        my $cached = (exists $SIZES{$name});
        my $status = ($cached ? $SIZES{$name} : -1);

        print "size:     ${name} ...";
        print " " x (28 - length($name));
        if ($status > 0 ||
                (-1 == $status && ($status = CheckSize($type, $name)) > 0)) {
            $SIZES{$name} = $status;
            $name = uc($name);
            $name =~ s/ /_/g;
            $CONFIG_H{"SIZEOF_${name}"} = $status;
            print ($cached ? "[$status, cached]" : "[$status]");
        } else {
            $SIZES{$name} = 0;
            print ($cached ? "[unknown, cached]" : "[unknown]");
        }
        print "\n";
    }

    # functions
    foreach my $name (@x_functions) {
        my $cached = (exists $FUNCTIONS{$name});
        my $status = ($cached ? $FUNCTIONS{$name} : -1);

        print "function: ${name} ...";
        print " " x (28 - length($name));
        if (1 == $status ||
                (-1 == $status && 0 == CheckFunction($type, $name))) {
            $FUNCTIONS{$name} = 1;
            $name = uc($name);
            $CONFIG_H{"HAVE_${name}"} = 1;
            print ($cached ? "[yes, cached]" : "[yes]");
        } else {
            $FUNCTIONS{$name} = 0;
            print ($cached ? "[no, cached]" : "[no]");
        }
        print "\n";
    }

    # libraries
    if (exists $config->{TESTLIBRARIES}) {
        foreach my $lib (@{$config->{TESTLIBRARIES}}) {
            my ($libname, @options) = split('\|', $lib);
            my $cached = (exists $LIBRARIES{$libname});
            my $status = ($cached ? $LIBRARIES{$libname} : -1);

            print "library:  ${libname} ...";
            print " " x (28 - length($libname));
            if (1 == $status ||
                    (-1 == $status && 0 == CheckFunction($type, undef, $libname))) {
                $LIBRARIES{$libname} = 1;
                $libname = uc($libname);
                $CONFIG_H{"HAVE_LIB${libname}"} = 1;
                if (scalar @options >= 1) { #eg. LIBTHREAD=pthread
                    $x_tokens{$options[0]} = $libname;
                }
                print ($cached ? "[yes, cached]" : "[yes]");
            } else {
                $LIBRARIES{$libname} = 0;
                print ($cached ? "[no, cached]" : "[no]");
            }
            print "\n";
        }
    }

    # compiler/environment
    # TODO: move to sub-module .... extension of TESTLIBRARIES
    if ($type eq 'vc' || $type eq 'wc' || $type eq 'owc' || $type eq 'mingw') {
        my $gnuwin32lib = undef;
        my $gnuwin32inc = undef;

        #gnuwin32 native builds
        if ($o_gnuwin32 && $o_gnulibs) {
            $o_gnuwin32 =~ s/\\/\//g;

            print "gnuwin32: ${o_gnuwin32}\n";
            $gnuwin32inc = "${o_gnuwin32}/include";
            $gnuwin32lib = "${o_gnuwin32}/lib";

            if (-d $gnuwin32inc) {
                $o_libarchive = $o_gnuwin32 if (! $o_libarchive);
                $o_libmagic = $o_gnuwin32 if (! $o_libmagic);
                push @EXTERNINCS, $gnuwin32inc;
            }

            foreach my $lib (@{$config->{OPTLIBRARIES}}) {
                my $LIBNAME = uc($lib);

                if (-f "${gnuwin32lib}/lib${lib}.lib" ||
                        -f "${gnuwin32lib}/${lib}.lib") {
                                                # enable work arounds, see source
                    $CONFIG_H{"GNUWIN32_LIB${LIBNAME}"} = '1';
                    $CONFIG_H{"HAVE_LIB${LIBNAME}"} = '1';
                    $CONFIG_H{"HAVE_${LIBNAME}_H"} = '1'
                        if (-f "${gnuwin32inc}/${lib}.h");
                    ImportDLL("${o_libarchive}/bin", \@DLLS, $lib);
                }
            }
        }

        #contrib tree
        my %contribs;

        if ($o_contrib) {
            foreach (@{$config->{MAKEFILES}}) {
                my $dir = MakefileDir($_);

                if (-f "${dir}/makelib.def") {
                    my $name = basename($dir);
                    if (LoadContrib($type, $version, $name, $dir, \@CONTRIBINCS)) {
                        $contribs{$name} = 1;
                    }
                }
            }

            if (exists $config->{CONTRIBEXTRA}) {
                foreach (@{$config->{CONTRIBEXTRA}}) {
                    my $dir = MakefileDir($_);

                    if (-f "${dir}/makelib.def") {
                        my $name = basename($dir);
                        if (LoadContrib($type, $version, $name, $dir, \@CONTRIBINCS)) {
                            $contribs{$name} = 1;
                        }
                    }
                }
            }
        }

        #libarchive
        if (! exists $contribs{libarchive} && defined $o_libarchive) {
            $o_libarchive =~ s/\\/\//g;
            my $libarchivelib = "${o_libarchive}/lib";
            my $libarchiveinc = "${o_libarchive}/include";

            if (-d $libarchiveinc && -d $libarchivelib) {
                my @libarchives = ('libarchive', 'archive');
                my $LIBARCHIVE = undef;

                foreach my $lib (@libarchives) {
                    if (-f "${libarchivelib}/${lib}.lib") {
                        $LIBARCHIVE = "${libarchivelib}/${lib}.lib";
                        last;
                    }
                }

                if ($LIBARCHIVE) {
                    print "archive:  ${o_libarchive}\n";
                    $CONFIG_H{'HAVE_LIBARCHIVE'} = '1';
                    $CONFIG_H{"GNUWIN32_LIBARCHIVE"} = '1'
                        if ($libarchivelib eq $gnuwin32lib);
                    $CONFIG_H{'HAVE_ARCHIVE_H'} = '1'
                        if (-f "${libarchiveinc}/archive.h");
                    $x_tokens{LIBARCHIVE} = ExportPath($LIBARCHIVE);
                    ImportDLL("${o_libarchive}/bin", \@DLLS, "archive");
                }
                push @EXTERNINCS, $libarchiveinc;
            }
        }

        #libhunspell
        if (! exists $contribs{hunspell} && defined $o_libhunspell) {
            $o_libhunspell =~ s/\\/\//g;
            my $libhunspelllib = "${o_libhunspell}/lib";
            my $libhunspellinc = "${o_libhunspell}/include";

            if (-d $libhunspellinc && -d $libhunspelllib) {
                my @libhunspells = ('libhunspell', 'hunspell');
                my $LIBHUNSPELL = undef;

                foreach my $lib (@libhunspells) {
                    if (-f "${libhunspelllib}/${lib}.lib") {
                        $LIBHUNSPELL = "${libhunspelllib}/${lib}.lib";
                        last;
                    }
                }

                if ($LIBHUNSPELL) {
                    print "hunspell:  ${o_libhunspell}\n";
                    $CONFIG_H{'HAVE_LIBHUNSPELL'} = '1';
                    $CONFIG_H{"GNUWIN32_LIBHUNSPELL"} = '1'
                        if ($libhunspelllib eq $gnuwin32lib);
                    $CONFIG_H{'HAVE_HUNSPELL_H'} = '1'
                        if (-f "${libhunspellinc}/hunspell.h");
                    $x_tokens{LIBHUNSPELL} = ExportPath($LIBHUNSPELL);
                    ImportDLL("${o_libhunspell}/bin", \@DLLS, "hunspell");
                }
                push @EXTERNINCS, $libhunspellinc;
            }
        }

        #libmagic
        if (! exists $contribs{libmagic} && defined $o_libmagic) {
            $o_libmagic =~ s/\\/\//g;
            my $libmagiclib = "${o_libmagic}/lib";
            my $libmagicinc = "${o_libmagic}/include";

            if (-d $libmagicinc && -d $libmagiclib) {
                my @libmagics = ('libmagic.lib', 'magic.lib');
                my $LIBMAGIC = '';

                foreach my $lib (@libmagics) {
                    if (-f "${libmagiclib}/${lib}") {
                        $LIBMAGIC = "${libmagiclib}/${lib}";
                        last;
                    }
                }

                if ($LIBMAGIC) {
                    print "magic:    ${o_libmagic}\n";
                    $CONFIG_H{'HAVE_LIBMAGIC'} = '1';
                    $CONFIG_H{"GNUWIN32_LIBMAGIC"} = '1'
                        if ($libmagiclib eq $gnuwin32lib);
                    $CONFIG_H{'HAVE_MAGIC_H'} = '1'
                        if (-f "${libmagicinc}/magic.h");
                    $x_tokens{LIBMAGIC} = ExportPath($LIBMAGIC);
                    ImportDLL("${o_libmagic}/bin", \@DLLS, "magic");
                }
                push @EXTERNINCS, $libmagicinc;
            }
        }

        #ICU
        if (! exists $contribs{icu} && defined $o_icu) {
            $o_icu =~ s/\\/\//g;

            if (-f "${o_icu}/makelib.def") {
                my $name = basename($o_icu);
                if (LoadContrib($type, $version, $name, $o_icu, \@CONTRIBINCS)) {
                    $contribs{'icu'} = 1;
                }

            } else {
                my $iculibpath = "${o_icu}/lib";
                my $icuinc = "${o_icu}/include";

                if (-d $icuinc && -d $iculibpath) {
                    my @libicus = (
                            'icudt.lib',    # data/rsource
                            'icuin.lib',    # i18n
                            'icuio.lib',
                            'icule.lib',
                            'iculx.lib',
                            'icutu.lib',
                            'icuuc.lib'     # common
                            );
                    my $iculib = '';
                    my $LIBICU = '';

                    $CONFIG_H{'HAVE_LIBICU'} = '1';
                    push @EXTERNINCS, $icuinc;
                    foreach my $lib (@libicus) {
                        $LIBICU .= ExportPath("$iculibpath/$lib");
                        $iculib .= "$iculibpath/$lib ";
                    }
                    $$env{ICUINCLUDE} = $icuinc;
                    $$env{ICULIB} = $iculib;
                    ImportDLL("${o_icu}/bin", \@DLLS, "icu", 1);

                    print "icu:      ${o_icu}\n";
                    if ((my $icuver = CheckICUFunction("test1")) >= 40) {
                        $CONFIG_H{'HAVE_LIBICU'} = '1';
                        $CONFIG_H{"HAVE_LIBICU_${icuver}"} = '1';
                        $x_tokens{LIBICU} = $LIBICU;
                        printf "\tICU %d.%d.x available ...\n", $icuver / 10, $icuver % 10;
                        print "\tdef: HAVE_LIBICU\n";
                        print "\tdef: HAVE_LIBICU_${icuver}\n";
                        print "\tinc: $icuinc\n";
                        print "\tlib: $iculib".'(@LIBICU@)'."\n";
                    } elsif ($icuver > 10) {
                        printf "\tUnsupported ICU version %d.%d\n", $icuver / 10, $icuver % 10;
                    } else {
                        print "\tICU either not found or not available, check log for details ($icuver)\n";
                    }
                }
            }
        }
    }

    my %T_INCS;
    push @CONTRIBINCS, @EXTERNINCS;
    foreach my $inc (@CONTRIBINCS) {
        $inc = cannon_path($inc);
        if (! exists $T_INCS{$inc}) {
            $x_tokens{CINCLUDE} .= " -I${inc}";
            push @INCLUDES, $inc;
            $T_INCS{$inc} = 1;
        }
    }

    if (scalar @LIBS) {
        $x_tokens{LIBS} .= ' ';
        foreach my $lib (@LIBS) {
            $x_tokens{LIBS} .= ExportPath($lib);
        }
    }
}


sub
ExeRealpath($)
{
    my ($path) = @_;

    if (-e $path) {
        $path = realpath($path);

    } elsif (-e "${path}.exe") {            # <xxx.exe
        $path = realpath("${path}.exe");
        $path =~ s/\.exe//;

    } elsif ($path =~ /^\.[\/\\]/) {        # ./xxxx; assume a generated artifact
        $path =~ s/^\./\$(ROOT)/;

    } else {
        print "warning: unable to resolve path <${path}>\n"
            if ($path !~ /\$/);             # variable; assume a generated artifact
    }

    $path = "\"${path}\""                   # quote; contains spaces
        if ($path =~ / /);
    return $path;
}


sub
LoadContrib($$$$$)      # (type, version, name, dir, refIncludes)
{
    my ($type, $version, $name, $dir, $refIncludes) = @_;

    my $basepath = ($dir ? $dir : "contrib/${name}");
    my $def = "${basepath}/makelib.def";
    my $lbl = "HAVE_".uc($name);
    my $lib = '';
    my $ext = '';
    my $cnt = 0;

    return 0
        if (-f $basepath);

    print "contrib:  $basepath\n";

    open(CFG, "<${def}") or
        die "cannot open <$def> : $!\n";
    while (defined (my $line = <CFG>)) {
        $line =~ s/\s*([\n\r]+|$)//;
        next if (!$line || /^\s#/);

        my @parts = split(/=/, $line, 2);
        if (2 == scalar @parts) {
            my ($key, $val) = @parts;

            if ('toolchain' eq $parts[0]) {
                (0 == $cnt++) or
                    die "$def: toolchain must be first element\n";

                $version =~ /^(\d+)/;       # version[_x64]
                my $version1 = $1;

                if ($val !~ /(^${type}|,${type})(\d*)/) {
                    print "$def: $val [no], toolchain ${type} not supported\n";
                    return 0;
                }
                my $version2 = $2;

                if ($version2 && int($version1) < int($version2)) {
                    print "$def: $val [no], toolchain version ${version} not supported\n";
                    return 0;
                }
                print "\ttoolchain: $val [yes]\n";

            } elsif ('inc' eq $parts[0]) {
                $val = "${basepath}/".$parts[1]
                    if ($val !~ /^\//);
                push @$refIncludes, '$(ROOT)/'.$val;
                print "\tinc: $val\n";

            } elsif ('lbl' eq $key) {
                $lbl = uc($val);

            } elsif ('lib' eq $key) {
                die "$def: multiple lib specifications\n"
                    if ($lib);
                $lib = ExportPath($val);
                print "\tlib: $val (\@$lbl\@)\n";

            } elsif ('ext' eq $key) {
                $ext .= ' ' if ($ext);
                $ext .= ExportPath($val);
                print "\text: $val (\@$lbl\@)\n";

            } elsif ('def' eq $key) { # Makefile.in HAVE_xxx, default=1
                if ($val =~ /^(.+)=(.*)$/) {
                    my ($tkn, $def) = ($1, $2 ? $2 : '1');
                    if (exists $CONFIG_H{$tkn}) {
                        print "warning: redefinition of \@$tkn\@ encountered\n"
                            if ($CONFIG_H{$tkn} ne $def);
                    }
                    $CONFIG_H{$tkn} = $def;
                } else {
                    if (exists $CONFIG_H{$val}) {
                        print "warning: redefinition of \@$val\@ encountered\n"
                            if ($CONFIG_H{$val} ne '1');
                    }
                    $CONFIG_H{$val} = '1';
                }
                print "\tdef: $val\n";

            } elsif ('var' eq $key) { # Makefile.in variable, empty allowed
                if ($val =~ /^(.+)=(.*)$/) {
                    my ($tkn, $def) = ($1, $2 ? $2 : '');
                    if (exists $x_tokens{$tkn}) {
                        print "warning: redefinition of \@$tkn\@ encountered\n"
                            if ($x_tokens{$tkn} ne $def);
                    }
                    $x_tokens{$tkn} = $def;

                } else {
                    print "warning: redefinition of \@$val\@ encountered\n"
                        if (exists $x_tokens{$val});
                    $x_tokens{$val} = '';
                }
                print "\tvar: $val\n";
            }
        }
    }

    $x_tokens{$lbl} = $lib if ($lib);
    if ($ext) {
        $x_tokens{$lbl} .= ' ' if ($x_tokens{$lbl});
        $x_tokens{$lbl} .= $ext;
    }

    close(CFG);
    return 1;
}


#   Function: MakelibConfigure
#       Import the optional toolchain configuration.
#
sub
MakelibConfigure($$)    # (type)
{
    my ($type, $env) = @_;

    require './makeconfig.pm' or
        die "makeconfig.pm: couldn't load $@\n";

    $config = MakeConfig->New();

    $x_libw32 = './' if (! -d $x_libw32);
    $config->{PACKAGE_PATH} = $x_libw32;
    $config->{CONFIG_PATH} = $x_libw32;

    $config->LoadConfigure($o_makelib, $type, $env, \%x_tokens, $o_verbose);

    $x_tokens{PACKAGE} = $config->{PACKAGE};
    $x_tokens{PACKAGE_NAME} = $config->{PACKAGE_NAME}
        if ($config->{PACKAGE_NAME});
}


sub
MakelibProfile()        # ()
{
    require './makeconfig.pm' or
        die "makeconfig.pm: couldn't load $@\n";

    $config = MakeConfig->New();

    $config->LoadProfile($o_makelib);

    $x_tokens{PACKAGE} = $config->{PACKAGE};
    $x_tokens{PACKAGE_NAME} = $config->{PACKAGE_NAME}
        if ($config->{PACKAGE_NAME});
}


#   Function: CheckCompiler
#       Check whether the stated compiler exists.
#
sub
CheckCompiler($$)       # (type, env)
{
    my ($type, $env) = @_;

    if (!defined $$env{COMPILERPATH} || $$env{COMPILERPATH} eq '') {
        if (exists $$env{COMPILERPATHS}) {
            my $compilerpath = which $$env{CC};
            if ($compilerpath) {                # resolved path
                $$env{COMPILERPATH} = dirname($compilerpath);

            } else {
                my @PATHS = split(/\|/, $$env{COMPILERPATHS});
                foreach (@PATHS) {
                    my $path = ExpandENV($_);
                    if (-e $path && -d $path) {
                        $compilerpath = realpath($path);
                        my $ccpath = "${compilerpath}/".$$env{CC};

                        if (-f $ccpath || -f "${ccpath}.exe") {
                            $$env{COMPILERPATH} = $compilerpath;
                            last;
                        }
                    }
                }
            }
        }
        $x_compiler  = $$env{COMPILERPATH}.'/'
            if (exists $$env{COMPILERPATH});

    } else {
        $x_compiler  = ExpandENV($$env{COMPILERPATH}).'/'
            if (exists $$env{COMPILERPATH});
    }

    $x_compiler .= $$env{CC};
    $x_compiler =~ s/\//\\/g;
    $x_command   = "\"$x_compiler\" ";

    (-1 != System("$x_command junk-command-line >${x_tmpdir}/compiler.out 2>&1")) or
        die "makelib: unable to access compiler <$x_compiler>\n";

    if (exists $$env{VPATTERN}) {               # version information
        my $vpattern = $$env{VPATTERN};
        my $vswitch = (exists $$env{VSWITCH} ? $$env{VSWITCH} : '');
        my @vtext = ();

        (-1 != System("$x_command $vswitch >${x_tmpdir}/version.out 2>&1")) or
            die "makelib: unable to access compiler <$x_compiler $vswitch>\n";

        open(VERSION, "${x_tmpdir}/version.out") or
            die "makelib: cannot open <${x_tmpdir}/version.out> : $!";
        while (<VERSION>) {
            $_ =~ s/\s*(\n|$)//;
            push @vtext, $_;
        }
        close VERSION;

        if (defined $vpattern) {                # pattern
            my $vflat = "@vtext";
            $vflat =~ /$vpattern/i;
            $x_version = $1;
        } else {                                # first list
            $x_version = $vtext[0];
        }
    }

    if ($x_compiler eq 'gcc') {
        #   #include <...> search starts here:
        #    c:\mingw\bin\../lib/gcc/mingw32/9.2.0/include
        #    c:\mingw\bin\../lib/gcc/mingw32/9.2.0/../../../../include
        #    c:\mingw\bin\../lib/gcc/mingw32/9.2.0/include-fixed
        #   End of search list.
        (-1 != System("gcc -E -Wp,-v - <NUL >${x_tmpdir}/gcc.out 2>&1")) or
            die "makelib: unable to access compiler <cpp -v>\n";

        open(GCC, "${x_tmpdir}/gcc.out") or
            die "makelib: cannot open <${x_tmpdir}/gcc.out> : $!";
        my $line;
        while (defined($line = <GCC>)) {
            if ($line =~ /^#include </) {
                while (defined($line = <GCC>)) {
                    last if ($line =~ /^End of/i);
                    $line =~ s/^\s+|\s+$//g;
                    my $path = realpath($line);
                    print "gccinc:   <$path>\n";
                    push @x_sysinclude, $path
                        if ($path);
                }
                last;
            }
        }

        if (exists $ENV{"C_INCLUDE_PATH"}) {
            $x_tokens{INCLUDE} = $ENV{"C_INCLUDE_PATH"};
        } elsif (exists $ENV{"CPATH"}) {
            $x_tokens{INCLUDE} = $ENV{"CPATH"};
        }

    } else {
        $x_tokens{INCLUDE} = $ENV{"INCLUDE"};
    }

    my $INCLUDES = '';
    $INCLUDES .=                                # <edidentifier.h>, required??
         (exists $$env{ISWITCH} ? $$env{ISWITCH} : '-I')."${CWD}/include ";

    if (defined $x_tokens{XINCLUDE}) {          # extra includes
        my @xincludes = split(/;/, $x_tokens{XINCLUDE});

        foreach my $inc (@xincludes) {
            if ($inc) {
                $inc =~ s/\$\(ROOT\)/${CWD}/;
                $inc = realpath($inc)
                    if (-d $inc);

                $INCLUDES .= (exists $$env{ISWITCH} ? $$env{ISWITCH} : '-I').$inc.' ';
                push @x_include, $inc;
            }
        }
    }

    $CONFIG_H{MAKELIB_CC_COMPILER} = "\"".basename(${x_compiler})."\"";
    $CONFIG_H{MAKELIB_CC_VERSION} = "\"${x_version}\"";

    print "Compiler: ${x_compiler}\n";
    print "Version:  ${x_version}\n";
    print "Command:  ${x_command}\n";
    print "Includes: @{x_include}\n";
    print "SysInc:   @{x_sysinclude}\n";

    # build final command
    $x_command  .= "__FLAGS__ ";
    $x_command  .= $INCLUDES;
    $x_command  .= "$$env{OSWITCH}__OBJ__ "
        if ($$env{OSWITCH} ne '');
    $x_command  .= "__LIB__ ";                  # lib's
    $x_command  .= "$$env{XSWITCH}__EXE__ ";
    $x_command  .= "__SOURCE__ >__BASE__.out 2>&1";
    $x_command  =~ s/\//\\/g;

    $$env{CXX}  = $$env{CC}
        if (! exists $$env{CXX});
}

#   Function: CheckVAARGS
#       Determine whether the preprocessor supports __VA_ARGS__
#
sub
CheckVAARGS()           # (cpp)
{
    my $result = "whether the preprocessor allows variadic macros: ";

    for (my $cpp = 0; $cpp <= 1; $cpp++)
    {
        my $BASE   = "preprocessor_va_args_${cpp}";
        my $SOURCE = ($cpp ? "${BASE}.cpp" : "${BASE}.c");
        my ($cmd, $cmdparts)
                = CheckCommand($BASE, $SOURCE);
        my $config = CheckConfig();

        my $asctime = asctime(localtime());
        chop($asctime);
        open(TMP, ">${x_tmpdir}/$SOURCE") or
                die "cannot create ${x_tmpdir}/$SOURCE : $!\n";
        print TMP<<EOT;
/*
 *  Generated by makelib.pl, $asctime (CheckVAARGS)
$cmdparts
 */

#define __ELEVENTH(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, ...) a11
#define ARGCOUNT(...) __ELEVENTH(dummy, ## __VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

int main(int argc, char **argv) {
    return (ARGCOUNT(0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19) == 9 && ARGCOUNT(-1,-2,-3,-4,-5) == 5 && ARGCOUNT() == 0) ? 1 : 0;
}
EOT
    close TMP;

        $result .= ($cpp ? ",cpp=" : "c=");
        if (1 == CheckExec($BASE, $cmd, 1)) {
            if ($cpp) {                         # __VA_ARGS__
                $CONFIG_H{"CCX_VA_ARGS"} = 1;
            } else {
                $CONFIG_H{"CC_VA_ARGS"} = 1;
            }
            $result .= "yes";
        } else {
            $result .= "no";
        }
    }

    print $result."\n";
}


#   Function: CheckDecl
#       Determine whether of the stated 'devl' exists.
#
sub
CheckDecl($$$)          # (type, name, cpp)
{
    my ($type, $name, $cpp) = @_;

    my $t_name = $name;
    $t_name =~ s/ /_/g;

    my $BASE   = "${type}_${t_name}";
    my $SOURCE = ($cpp ? "${BASE}.cpp" : "${BASE}.c");
    my ($cmd, $cmdparts)
            = CheckCommand($BASE, $SOURCE);
    my $config = CheckConfig();

    $name = 'void *'
        if ($name eq 'void_p');

    my $asctime = asctime(localtime());
    chop($asctime);
    open(TMP, ">${x_tmpdir}/$SOURCE") or
            die "cannot create ${x_tmpdir}/$SOURCE : $!\n";
    print TMP<<EOT;
/*
 *  Generated by makelib.pl, $asctime (CheckDecl)
$cmdparts
 */
${config}
int main(int argc, char **argv) {
#if !defined($name)
#error $name is not defined ....
#endif
#define __STRIZE(__x) #__x
#define STRIZE(__x)  __STRIZE(__x)
    const int ret = strlen(STRIZE($name));
    FILE *out = fopen("${BASE}.value", "w+");
    fprintf(out, "%s", STRIZE($name));
    printf("${name}=%s : ", STRIZE($name));
    return ret ? 0 : 1;
}
EOT
    close TMP;

    my $result = "${BASE}.value";
    return $result
        if (0 == CheckExec($BASE, $cmd, 1, \$result));
    return "";
}


#   Function: CheckHeader
#       Determine whether of the stated 'header' is usage.
#
sub
CheckHeader($$)         # (header, $headerdefines)
{
    my ($header, $headerdefines) = @_;

    my $t_header = $header;
    $t_header =~ s/[\\\/\. ]/_/g;

    my $BASE   = "header_${t_header}";
    my $SOURCE = "${BASE}.c";
    my ($cmd, $cmdparts)
            = CheckCommand($BASE, $SOURCE);
    my $config = CheckConfig();

    my $asctime = asctime(localtime());
    chop($asctime);
    open(TMP, ">${x_tmpdir}/$SOURCE") or
            die "cannot create ${x_tmpdir}/$SOURCE : $!\n";
    print TMP<<EOT;
/*
 *  Generated by makelib.pl, $asctime (CheckHeader)
$cmdparts
 */
${headerdefines}
#include <$header>
int main(int argc, char **argv) {
    return 0;
}
EOT
    close TMP;

    return CheckExec($BASE, $cmd, 1);
}


#   Function: CheckType
#       Determine whether the stated 'type' exists.
#
sub
CheckType($$;$)         # (type, name, [field])
{
    my ($type, $name, $field) = @_;

    my $t_name = $name;
    $t_name =~ s/ /_/g;

    my $BASE   = "${type}_${t_name}";
    my $SOURCE = "${BASE}.c";
    my ($cmd, $cmdparts)
            = CheckCommand($BASE, $SOURCE);
    my $config = CheckConfig();

    if ($name =~ /^struct /) {
        if ($field =~ /([^;]+);(.+)$/) {        # field;header[,...]
            $field = $1;
            my @headers = split(/,/, $2);
            foreach my $header (@headers) {
                my $have_header_h = "HAVE_".uc($header);
                $have_header_h =~ s/[\\\/\. ]/_/g;
                $config .= "#include <${header}>\n"
                    if (exists $CONFIG_H{"${have_header_h}"});
            }
        }
    }

    my $asctime = asctime(localtime());
    chop($asctime);
    open(TMP, ">${x_tmpdir}/$SOURCE") or
            die "cannot create ${x_tmpdir}/$SOURCE : $!\n";
    print TMP<<EOT;
/*
 *  Generated by makelib.pl, $asctime (CheckType)
$cmdparts
 */
${config}
EOT

    if ($name =~ /inline/) {
        print TMP<<EOT;
static ${name} function(void) {
    return 1;
}
int main(int argc, char **argv) {
    return function();
}
EOT

    } elsif ($name =~ /^struct /) {
        print TMP<<EOT;
static ${name} var;
int main(int argc, char **argv) {
    var.${field};
    return 1;
}
EOT

    } else {
        print TMP<<EOT;
static ${name} name;
int main(int argc, char **argv) {
    name = 0;
    return 1;
}
EOT
    }
    close TMP;

    return CheckExec($BASE, $cmd);
}


#   Function: CheckSize
#       Determine size of the stated 'type' exists.
#
sub
CheckSize($$)           # (type, name)
{
    my ($type, $name) = @_;

    my $t_name = $name;
    $t_name =~ s/ /_/g;

    my $BASE   = "${type}_${t_name}";
    my $SOURCE = "${BASE}.c";
    my ($cmd, $cmdparts)
            = CheckCommand($BASE, $SOURCE);
    my $config = CheckConfig();

    $name = 'void *'
        if ($name eq 'void_p');

    my $asctime = asctime(localtime());
    chop($asctime);
    open(TMP, ">${x_tmpdir}/$SOURCE") or
            die "cannot create ${x_tmpdir}/$SOURCE : $!\n";
    print TMP<<EOT;
/*
 *  Generated by makelib.pl, $asctime (CheckSize)
$cmdparts
 */
${config}
int main(int argc, char **argv) {
    return sizeof($name);
}
EOT
    close TMP;

    return CheckExec($BASE, $cmd, 1);
}


#   Function: CheckFunction
#       Check that the function and/or library exists.
#   Returns:
#       0 on success, otherwise non-zero.
#
sub
CheckFunction($$;$)     # (type, name, [libname])
{
    my ($type, $name, $libname) = @_;

    my $BASE   = ($name ? "${type}_${name}" : "${type}_lib${libname}");
    my $SOURCE = "${BASE}.c";
    my ($cmd, $cmdparts)
            = CheckCommand($BASE, $SOURCE, undef, $libname);
    my $config = CheckConfig();

    my $tmpsource = "${x_tmpdir}/$SOURCE";
    my $asctime = asctime(localtime());

    chop($asctime);
    open(TMP, ">${tmpsource}") or
            die "cannot create ${tmpsource} : $!\n";
    print TMP<<EOT;
/*
 *  Generated by makelib.pl, $asctime (CheckFunction)
$cmdparts
 */
EOT

    my $headers = '';
    foreach my $header (@HEADERS) {
        $headers .= "#include <$header>\n";
    }

    ##############################################################################
    #   library check only
    #
    if (!defined $name) {
        print TMP<<EOT;
${config}
${headers}
int main(int argc, char **argv) {
    return 0;
}
EOT

    ##############################################################################
    #   alloca -- possible intrusive
    #
    } elsif ($name =~ /alloca$/) {
        $config  = "/*see: AC_FUNC_ALLOCA for details*/\n";
        $config .= "\n#define HAVE_STDLIB_H"
            if (exists $CONFIG_H{HAVE_STDLIB_H});
        $config .= "\n#define HAVE_ALLOCA_H"
            if (exists $CONFIG_H{HAVE_ALLOCA_H});

        print TMP<<EOT;
${config}
#if defined(STDC_HEADERS)
#   include <stdlib.h>
#   include <stddef.h>
#else
#   if defined(HAVE_STDLIB_H)
#       include <stdlib.h>
#   endif
#endif
#if defined(HAVE_ALLOCA_H)
#   include <alloca.h>
#elif !defined(alloca)
#   if defined(__GNUC__)
#        define alloca          __builtin_alloca
#   elif defined(_AIX)
#       define alloca           __alloca
#   elif defined(_MSC_VER) || defined(__WATCOMC__)
#       include <malloc.h>
#   else
#       ifdef __cplusplus
extern "C"
#       endif
void *alloca (size_t);
#   endif
#endif
int main(int argc, char **argv) {
    const char *mem = ${name}(1024);
    return (mem ? 1 : 0);
}
EOT

    ##############################################################################
    #   va_copy, generally a macro
    #
    } elsif ($name =~ /va_copy$/) {
        print TMP<<EOT;
${config}
#if defined(__STDC__) || defined(STDC_HEADERS)
#include <stdarg.h>
#endif
${headers}
int main(int argc, char **argv) {
    va_list ap, ap2;
    va_copy(ap, ap2);
    return 0;
}
EOT

    ##############################################################################
    #   finite,isfinite, isinf, insan can be macros
    #
    } elsif ($name =~ /finite$/ || $name =~ /finitef$/ || $name =~ /is(inf|nan|nanf)$/ || $name =~ /^isnormal/) {
        print TMP<<EOT;
${config}
${headers}
int main(int argc, char **argv) {
    double x = 0;
    $name(x);
    return 0;
}
EOT

    ##############################################################################
    #   generic
    #
    } else {
        if ($name =~ /_s$/) {   # enable C Library Extension 1 (MSVC/WC only)
            print TMP<<EOT;
#define __STDC_WANT_LIB_EXT1__ 1
EOT
        }

        print TMP<<EOT;
${config}
#if defined(__STDC__) || defined(STDC_HEADERS)
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif
${headers}
typedef void (*function_t)(void);
static function_t function;
int main(int argc, char **argv) {
    function = (function_t)(&$name);
    return 1;
}
EOT
    }
    close TMP;

    return CheckExec($BASE, $cmd);
}


#   Function: CheckICUFunction
#       Check that the ICU function exists
#   Returns:
#       0 on success, otherwise non-zero.
#
sub
CheckICUFunction($)     # (type, name)
{
    my ($name) = @_;

    my $BASE   = "icufunction_${name}";
    my $SOURCE = "${BASE}.cpp";
    my ($cmd, $cmdparts)
            = CheckCommand($BASE, $SOURCE, 'icu');

    my $tmpsource = "${x_tmpdir}/$SOURCE";
    my $asctime = asctime(localtime());

    chop($asctime);
    open(TMP, ">${tmpsource}") or
            die "cannot create ${tmpsource} : $!\n";
    print TMP<<EOT;
/*
 *  ICU test application.
$cmdparts
 */
#if defined(__STDC__) || defined(STDC_HEADERS)
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif
#include <unicode/uversion.h>
#include <unicode/uclean.h>             // C API
#include <unicode/utypes.h>
#include <unicode/ustdio.h>
#include <unicode/unistr.h>
#include <unicode/uchar.h>
#include <unicode/udata.h>
#include <unicode/ucal.h>               // UCAL_XXX
#include <unicode/calendar.h>           // Calendar::
#include <iostream>
#ifndef U_ICU_VERSION_SHORT
#error  U_ICU_VERSION_SHORT not defined ...
#endif

static void
CalendarTest() {
    UErrorCode status = U_ZERO_ERROR;

    Calendar *cal = Calendar::createInstance("en_UK\@calendar=gregorian", status);
    if (U_FAILURE(status)) {
        printf("Error creating Gregorian calendar.\\n");
        return;
    }

    {   UnicodeString result;
        result = cal->getTimeZone().getDisplayName(result);
        std::cout << "Calendar:";
        for (int i = 0; i < result.length(); ++i) {
            std::cout << (char)result[i];
        }
        std::cout << std::endl;
    }

    printf("1970 - 2020 Jan/July 01:00:00.\\n");
    for (int year = 1970; year < 2020; ++year) {
        for (int month = 1; month <= 2; ++month) {
            cal->set(year, (1 == month ? UCAL_JANUARY : UCAL_JULY), 1, 1, 0, 0);
            if (U_FAILURE(status)) {
                printf("Error setting date %d/%d/%d 01:00:00\\n", year, 1, 1);
                continue;
            }
            const UDate datetime = cal->getTime(status);
            if (U_FAILURE(status)) {
                printf("Error getting time.\\n");
                continue;
            }
            printf(" %04d%02d%02d=%11d:%d:%d:%d,",
                cal->get(UCAL_EXTENDED_YEAR,  status),
                cal->get(UCAL_MONTH, status) + 1,
                cal->get(UCAL_DATE, status),
                (int)(datetime / 1000),
                cal->get(UCAL_JULIAN_DAY /*Calendar::JULIAN_DAY*/, status),
                cal->inDaylightTime(status));
        }
        printf((year % 2) ? "\\n" : "");
    }
    printf("\\n");
}

int
main(int argc, char **argv) {
    UErrorCode status = U_ZERO_ERROR;
    UVersionInfo versionArray = {0};
    char versionString[17];                     /* xxx.xxx.xxx.xxx\\0 */

    u_init(&status);
    if (U_FAILURE(status)) {
        printf("Error in u_init: %s.\\n", u_errorName(status));
        return 0;
    }
    u_getVersion(versionArray);
    u_versionToString(versionArray, versionString);
    printf("ICU Version: %s.\\n", versionString);

    CalendarTest();

    return U_ICU_VERSION_MAJOR_NUM;             /* ie. 52 5.2.x */
}
EOT
    close TMP;

    return CheckExec($BASE, $cmd, 1);
}


#   Function: CheckConfig
#       Build the <config.h> definition to use during check commands.
#   Returns:
#       config_h
#
sub
CheckConfig()
{
    my $config = '';

    $config = "
#if (defined(_MSC_VER) || defined(__WATCOMC__)) && \\
           !defined(STDC_HEADERS)
#define STDC_HEADERS        /* VC, __STDC__ only under /Za */
#endif
#if defined(__STDC__) || defined(STDC_HEADERS)
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif";
    $config .= "\n#define HAVE_INTTYPES_H"
            if (exists $CONFIG_H{HAVE_INTTYPES_H});
    $config .= "\n#define HAVE_STDINT_H"
            if (exists $CONFIG_H{HAVE_STDINT_H});
    $config .= "\n#define HAVE_STDBOOL_H"
            if (exists $CONFIG_H{HAVE_STDBOOL_H});
    $config .= "\n#define HAVE_STDIO_H"
            if (exists $CONFIG_H{HAVE_STDIO_H});
    $config .= "\n#define HAVE_LIMITS_H"
            if (exists $CONFIG_H{HAVE_LIMITS_H});
    $config .= "\n#define HAVE_STRING_H"
            if (exists $CONFIG_H{HAVE_STRING_H});
    $config .= "\n#define HAVE_WCHAR_H"
            if (exists $CONFIG_H{HAVE_WCHAR_H});
    $config .= "
#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>       /* <stdint.h> plus printf/scanf format facilities */
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif
#if defined(HAVE_STDBOOL_H)
#include <stdbool.h>
#endif
#if !defined(__STDC__) && defined(HAVE_STDIO_H)
#include <stdio.h>
#endif
#if defined(HAVE_LIMITS_H)
#include <limits.h>         /* XXX_MAX/MIN etc */
#endif
#if defined(HAVE_STRING_H)
#include <string.h>         /* strlen()/memcpy() etc */
#endif
#if defined(HAVE_WCHAR_H)
#include <wchar.h>          /* wcxxx */
#endif
";
    return $config;
}


#   Function: CheckCommand
#       Build the compile check exec command.
#   Returns:
#       cmd, cmdparts
#
sub
CheckCommand($$;$$)     # (base, source, [pkg], [libname])
{
    my ($base, $source, $pkg, $libname) = @_;

    my $env = $x_environment{$x_signature};
    my $cmd = $x_command;

    my $flags = '';
    my $optional = '';
    my $lib = '';

    if ($source =~ /\.cpp/) {                   # CXX
        $flags = $$env{CXXFLAGS}.' '
            if (exists $$env{CXXFLAGS});
    } else {                                    # or CC
        $flags = $$env{CFLAGS}.' '
            if (exists $$env{CFLAGS});
    }

    if (defined $pkg) {
        if ('icu' eq $pkg) {                    # FIXME, generic interface
            my @includes = split(/ /, $$env{ICUINCLUDE});
            my @libs = split(/ /, $$env{ICULIB});

            foreach (@includes) {
                $flags .= (exists $$env{ISWITCH} ? $$env{ISWITCH} : '-I').$_.' '
                    if ($_);
            }

            foreach (@libs) {
                $lib .= (exists $$env{LSWITCH} ? $$env{LSWITCH} : '-l').$_.' '
                    if ($_);
            }
        } else {
            die "CheckCommand: unexpected package '$pkg'\n";
        }
    }

    if (defined $libname) {
        $lib .= '-l'.$libname.' ';
    }

    while ($flags =~ /\@(.+)\@/) {              # expand @xxxx@
        my $entry = $1;
        $flags =~ s/\@\Q${entry}\E\@/${x_tokens{$entry}}/g;
    }
    $flags =~ s/\$\(.+\)//g;                    # remove $(xxxx)

    $cmd =~ s/__FLAGS__/$flags/g;
    $cmd =~ s/__SOURCE__/$source/g;
    $cmd =~ s/__BASE__/$base/g;
    $cmd =~ s/__OBJ__/$base.obj/g;
    $cmd =~ s/__LIB__/$lib/g;
    $cmd =~ s/__EXE__/$base.exe/g;

    my $cmdparts = '';
    foreach (my @words = parse_line('\s+', 1, $cmd)) {
        $cmdparts .= "\n" if ($cmdparts);
        $cmdparts .= " *\t$_";
    }
    return ($cmd, $cmdparts);
}


#   Function: CheckExec
#       Execute the compile check command.
#   Parameters:
#       base - Base application name.
#       cmd  - Compiler command.
#       exec - Optional boolean flag, if *true* the resulting application is executed.
#       refRead - Optional file to read; filled with result.
#   Returns:
#       cmd, cmdparts
#
sub
CheckExec($$;$$)        # (base, cmd, [exec], [refRead])
{
    my ($base, $cmd, $exec, $refRead) = @_;

    print "(cd tmpdir; $cmd)\n"
        if ($o_verbose);

    chdir($x_tmpdir) or
        die "cannot access directory <$x_tmpdir> : $!\n";

    unlink("${base}.exe");
    my $ret = System($cmd);
    if (! -f "${base}.exe") {
        my $out = "${base}.out";
        if ($o_verbose && -f $out) {
            printf "  ::<%s>\n", $out;
            open(OUT, "<${out}") or
                die "cannot open ${out}";
            while (defined (my $line = <OUT>)) {
                chomp $line;
                printf "  |%s\n", $line;
            }
            close(OUT);
        }
        $ret = -999;
    }

    $ret = System($base)
        if (0 == $ret && $exec);

    if (defined $refRead) {
        if (0 == $ret) {
            open my $file, '<', $$refRead;
            $$refRead = <$file>;
            close $file;
        } else {
            $$refRead = "";
        }
    }

    if (! $o_keep) {
        opendir(DIR, '.') or
            die "error opening dir <${x_tmpdir}> : $!\n";
        my @FILES = grep /$base/, readdir(DIR);
        unlink(@FILES);
        close DIR;
    }

    chdir($CWD) or
        die "cannot restore directory <$CWD> : $!\n";
    return $ret;
}


sub
ExpandENV($)            # (var)
{
    my ($variable) = shift;

    while ($variable =~/%(.+)%/i) {
        my $var = $1;
        my $val = ${ENV{$var}};
        $val = "UNDEF_${var}_ENVVAR" if (! $val);
        $variable =~ s/%${var}%/$val/;
    }
    return $variable;
}


sub
DumpList($$)            # (prefix, arrayRef)
{
    my ($prefix, $list) = @_;

    if (scalar @$list) {
        my ($fragment, $length) = ('', 0);

        foreach my $elm (@$list) {
            my $newfragment = substr $elm, 0, 3;
            if (($newfragment eq $fragment) &&
                    ($length += length($elm)) < 80) {
                # group comment elements
                printf " $elm";

            } else {
                $fragment = $newfragment;
                $length = printf "%s%-9s $elm", ($prefix ? "" : "\n"), $prefix;
                $prefix = '';
            }
        }
        printf "\n";
    }
}


sub
ExportPath($)           # (name)
{
    my ($name) = @_;
    return $name
        if ($name !~ /\//);
    return '$(subst /,\\,' . $name . ') ';
}


sub
ImportDLL($$;$$)        # (dir, dlls, [name], [test])
{
    my ($dir, $dlls, $name, $test) = @_;

    return if (! -d $dir);

    opendir(DIR, $dir) or
        die "error opening dir <$dir> : $!\n";
    my @DLLS = grep /\.dll/, readdir(DIR);
    @DLLS = grep /$name/, @DLLS if ($name);
    if (scalar @DLLS) {
        foreach my $dll (@DLLS) {
            push @$dlls, $dll;
            copy("${dir}/${dll}", "bin/${dll}");
            copy("${dir}/${dll}", "${x_tmpdir}")
                if ($test);
        }
    }
    close DIR;
}


#   Makefile ---
#       Build a Makefile from the underlying Makefile.in
#
sub
Makefile($$$)           # (type, dir, file)
{
    my ($type, $dir, $file) = @_;
    my $optional = 0;
    my $recursive = 0;
    my $text = "";

    # Import
    if ($dir =~ /^\!(.+)$/) {
        $recursive = 1;
        $dir = $1;
    }
    if ($dir =~ /^\^(.+)$/) {
        $optional = 1;
        $dir = $1;
    }
    printf "building: $dir/$file\n";

    if (! open(MAKEFILE, "${dir}/${file}.in")) {
        if (! open(MAKEFILE, "${dir}/${file}in")) {
            return if ($optional);
            die "cannot open ${dir}/${file}[.]in : $!";
        }
    }

    my $relpath = (File::Spec->file_name_is_absolute($dir) ? $CWD :
                        dos2unix(File::Spec->abs2rel($CWD, "${CWD}/${dir}")));
    print "relpath=${relpath}\n"
        if ($o_verbose);

    my $continuation = 0;
    while (<MAKEFILE>) {
        $_ =~ s/\s*(\n|$)//;                    # kill trailing whitespace & nl

        if ($continuation) {                    # continuation
            $continuation = (/[\\]$/ ? 1 : 0);

        } else {
            if ($type eq 'vc' || $type eq 'wc') {
                if (! /LIBTOOL/) {              # not LIBTOOL command lines

                    # option conversion
                    s/(\$\(CFLAGS\).*) -o \$\@/$1 -Fo\$@ -Fd\$(\@D)\//;
                    s/(\$\(CXXFLAGS\).*) -o \$\@/$1 -Fo\$@ -Fd\$(\@D)\//;

                    s/(\$\(LDFLAGS\).*) -o \$\@/$1 -Fe\$@ -Fd\$(\@D)\//;

                    if ($type eq 'vc') {
                        s/-L/\/link \/LIBPATH:/;
                    } else {
                        s/-L([^\s]+)/-"LIBPATH $1"/;
                    }

                } elsif (/[\\]$/) {
                    $continuation = 1;          # LIBTOOL, continuation?
                }

            } elsif ($type eq 'owc') {
                if (! /LIBTOOL/) {              # not LIBTOOL command lines

                    # option and directory slash conversion
                    if ('-o' ne $x_tokens{OSWITCH}) {
                        s/(\$\(CFLAGS\).*) -o \$\@/$1 -Fo=\$(subst \/,\\,\$@)/;
                        s/(\$\(CXXFLAGS\).*) -o \$\@/$1 -Fo=\$(subst \/,\\,\$@)/;
                        s/(\$\(LDFLAGS\).*) -o \$@/$1 -Fe=\$(subst \/,\\,\$@)/;

                        if (/\(RC\)/) {         # resource compiler
                            s/ -fo[ ]?\$@/ -fo="\$(subst \/,\\,\$@)"/;
                            s/ \$</ "\$<"/;
                        }

                        s/-Fe(.*) \$\(([A-Z_]*OBJS)\)/-Fe$1 \$(subst \/,\\,\$($2))/;
                        s/-Fe(.*) \$\^/-Fe$1 \$(subst \/,\\,\$^)/;

                        s/-L([^\s]+)/-"LIBPATH \$(subst \/,\\,$1)"/;
                            # -"<linker directive>"

                    } else {
                        s/\$\(LDFLAGS\) (.*) \$\(([A-Z_]*OBJS)\)/\$(LDFLAGS) $1 \$(subst \/,\\,\$($2))/;
                        s/\$\(LDFLAGS\) (.*) \$</\$(LDFLAGS) $1 \$(subst \/,\\,\$<)/;

                        s/-L([^\s]+)/-"Wl,LIBPATH \$(subst \/,\\,$1)"/;
                            # -Wl,<linker directive>
                    }

                    if ('-i=' eq $x_tokens{ISWITCH}) {
                        # s/-I([^\s]+)/-i="$1"/g;
                        # s/-I ([^\s]+)/-i="$1"/g;
                            # gnuwin32 (gmake 3.x) quotes would be retained;
                            # this can not be guaranteed under an alt instance, for example gmake (4.x).
                        if (/\(RC\)/) {         # resource compiler (2024/01)
                            s/-I([^\s]+)/-i="\$(subst \/,\\,$1)"/g;
                            s/-I ([^\s]+)/-i="\$(subst \/,\\,$1)"/g;
                        } else {
                            s/-I([^\s]+)/-i=\$(subst \/,\\,$1)/g;
                            s/-I ([^\s]+)/-i=\$(subst \/,\\,$1)/g;
                        }
                    }

                    s/\$</\$(subst \/,\\,\$<)/;
                    s/\$\^/\$(subst \/,\\,\$^)/;

                } elsif (/[\\]$/) {
                    $continuation = 1;          # LIBTOOL, continuation?
                }
            }
        }

        $text .= "$_\n";
    }
    close MAKEFILE;

    # Installation paths and install options
    $text =~ s/(\nPREFIX=[ \t]*)[^\n]+/$1\/grief/g;
    $text =~ s/(\nBINDIR=[ \t]*)[^\n]+/$1\/grief\/bin/g;
    $text =~ s/(\nDATADIR=[ \t]*)[^\n]+/$1\/grief/g;
    $text =~ s/-o bin -g bin//g;

    $text =~ s/\Q>\/dev\/null\E/>NUL/g;         # NUL redirection

    # Commands
    if ('dj' ne $type) {
        if ($BINPATH) {
            foreach my $command (@x_commands) {
                $text =~ s/($command) /$BINPATH$1 /g;
            }
        } else {
            foreach my $command (@x_commands) {
                $text =~ s/($command) /$1.exe /g;
            }
        }
    }

    # Specific to the target
    if ('dj' eq $type || 'mingw' eq $type) {    # almost unix
        $text =~ s/\nE=\S*\n/\nE=\t\t.exe\n/g;

    } elsif ($type eq 'vc' || $type eq 'wc' || $type eq 'owc') {
                                                # Visual C/C++ and Watcom C/C++
        # extensions
        $text =~ s/\nO=\s*\.o/\nO=\t\t.obj/g;
        $text =~ s/\nA=\s*\.a/\nA=\t\t.lib/g;
        $text =~ s/\nE=\S*\n/\nE=\t\t.exe\n/g;

        # flags and name mangling
        $text =~ s/(\nARFLAGS=)[^\n]+/$1-nologo/g;
        $text =~ s/(\$\(ARFLAGS\))\s+(\$\@)/$1 \/OUT:$2/g;

        my $clean = '';
        my $xclean = '*.pdb *.ilk';

        if ($type eq 'wc') {                    # Watcom
            $clean .= ' *.err';
            $xclean .= ' $(D_OBJ)/*.mbr';

        } elsif ($type eq 'owc') {              # OpenWatcom
            $clean .= ' *.err';
            $xclean .= ' $(D_OBJ)/*.mbr';

        } else {
            $xclean .= ' $(D_OBJ)/*.pdb';
        }

        # libraries
        foreach my $library (@{$config->{LIBRARIES}}) {
            $text =~ s/-l${library}([\n\t \\])/lib${library}.lib$1/g;
        }

        foreach my $library (@{$config->{LIBRARIES2}}) {
            $text =~ s/-l${library}([\n\t \\])/${library}.lib$1/g;
        }

        # addition clean targets
        $text =~ s/(\nCLEAN=\s+)/$1${clean} /g
            if ($clean);
        if ($xclean) {
            $text =~ s/(\nXCLEAN=[\t ]+)/$1${xclean} /;
            $text =~ s/(\nXCLEAN=[\t ]*)\n/$1\t\t${xclean}\n/;
        }
    }

    # replace tags
    $x_tokens{top_builddir} = $relpath;
    $x_tokens{top_srcdir} = $relpath;
    if ($type eq 'owc') {                      # OpenWatcom
       if ('-i=' eq $x_tokens{ISWITCH}) {
            $x_tokens{CINCLUDE} =~ s/-I([^\s]+)/-i=\$(subst \/,\\,$1)/g;
            $x_tokens{CINCLUDE} =~ s/-I ([^\s]+)/-i=\$(subst \/,\\,$1)/g;
        }
    }

    foreach my $entry (keys %x_tokendefs) {
        if (! defined $x_tokens{$entry} || ! $x_tokens{$entry}) {
            my $parent = $x_tokens{$x_tokendefs{$entry}};
            if ($parent) {
                print "Defaulting CXXFLAGS = CFLAGS [${parent}]\n"
                     if ($o_verbose);
                $x_tokens{$entry} = $parent;   # example CXX=CC
            }
        }
    }

    my $count;
    do {
        $count = 0;
        foreach my $entry (keys %x_tokens) {
            my $quoted_entry = quotemeta($entry);
            my $replace = $x_tokens{$entry};

            ++$count
                if ($text =~ s/\@$quoted_entry\@/$replace/g);
        }
    } while ($count);

    if ($BUSYBOX) {                             # command interface rework
        $text =~ s/\@sh /\@${BUSYBOX} sh /g;
        $text =~ s/\-sh /-${BUSYBOX} sh /g;
        $text =~ s/\-\@sh /-\@${BUSYBOX} sh /g;
        $text =~ s/\@-sh /\@-${BUSYBOX} sh /g;

        $text =~ s/shell sh /shell ${BUSYBOX} sh /g;
        $text =~ s/shell date /shell ${BUSYBOX} date /g;
        $text =~ s/shell cat /shell ${BUSYBOX} cat /g;
        $text =~ s/shell test /shell ${BUSYBOX} test /g;
    }

    # Note:
    #   By default, busybox and some shells are built with globbing in the C runtime disabled.
    #   Hence when run from the Windows command prompt it can behave in ways that don't conform to expectations,
    #   as such
    #
    #     $(shell ls <pattern>)
    #
    #   hence the above cannot be exec'ed via busybox-32 as the pattern wont be expanded,
    #
    print "warning: encountered (shell ls), advise using \$(wildcard ..) for portability\n"
       if ($text =~ /shell ls/);

    $text =~ s/\@BINPATH\@/${BINPATH}/g;
    $text =~ s/\@PERLPATH\@/${PERLPATH}/g;

    $text =~ s/(\$\(RM\)) (.*)/$1 \$(subst \/,\\,$2)/g;
    $text =~ s/(\$\(RMDIR\)) (.*)/$1 \$(subst \/,\\,$2)/g;

    if ($file ne $config->{PACKAGE_FILE}) {
        my %missing = ();
        while ($text =~ /\@([A-Z0-9_]+)\@/g) {  # report unhandled elements
            if (! exists $missing{$1}) {
                print "warning: unknown element \@$1\@ encountered\n";
                $missing{$1} = 1;
            }
        }
    }

    # export
    my $asctime = asctime(localtime());

    chop($asctime);
    open(MAKEFILE, ">$dir/$file") or
        die "cannot create $dir/$file";
    if ($file eq 'Makefile') {
        print MAKEFILE "# Generated by makelib.pl, $asctime\n";
    } else {
        if (! ($file =~ s/\@configure_input\@/Generated by makelib.pl, $asctime/)) {
            if ($file =~ /.h$/) {
                print MAKEFILE "/* Generated by makelib.pl, $asctime */\n";
            }
        }
    }

    if ($file eq 'Makefile') {                  # compact whitespace
        $text =~ s/ [ ]+/ /g;
        $text =~ s/\t[ ]+/\t/g;
    }

    print MAKEFILE $text;
    close MAKEFILE;

    if ($recursive) {
        opendir(DIR, $dir) or
            die "error opening dir <${dir}> : $!\n";
        my @FILES = readdir(DIR);
        close DIR;

        foreach (@FILES) {
            next if (/^\./);
            my $subdir = "$dir/$_";
            if (-d $subdir) {
                if (-f "${subdir}/${file}.in" || -f "${subdir}/${file}in") {
                    Makefile($type, "!${subdir}", $file);
                }
            }
        }
    }
}


sub
MakefileDir($)
{
    my $name = shift;
    $name =~ s/^[\^\!]+//;
    return $name;
}


#   Config ---
#       Build a config from an underlying config.in
#
sub
Config($$$)             # (type, dir, file)
{
    my ($type, $dir, $file) = @_;
    my $text = "";

    # import
    if (! -f "${dir}/{$file}") {
        if ($dir =~ /libw32/) {                 # override 'w32config.h'
            $file = 'w32config.h'
                if (-f "${dir}/w32config.hin" || "${dir}/w32config.h.in");
        }
    }

    printf "building: $dir/$file\n";

    if (! open(CONFIG, "${dir}/${file}.in")) {
        open(CONFIG, "${dir}/${file}in") or
            die "cannot open ${dir}/${file}[.]in : $!";
    }
    while (<CONFIG>) {
        $_ =~ s/\s*(\n|$)//;                    # kill trailing whitespace & nl
        $text .= "$_\n";
    }
    close CONFIG;

    # update characteristics
    my @MISSING = ();
    my @MULTIPLE = ();

    foreach my $config (sort keys %CONFIG_H) {
        my $value = $CONFIG_H{$config};

        if ($text =~ /^([ \t]*#[ \t]*undef[ \t]+${config})([ \t]*|[ \t]+.+)$/m) {
            my $count = 0;

            while ($text =~ s/^([ \t]*#[ \t]*undef[ \t]+${config})([ \t]*|[ \t]+.+)$/#define ${config} ${value}/m) {
                ++$count;
            }
            push @MULTIPLE, $config
                if ($count > 1);

        } else {
            push @MISSING, $config
                if (!exists $CONFIG_O{$config});
        }
    }
    $text =~ s/(#undef[^*\n]+)\n/\/* $1 *\/\n/g;

    if (scalar @MISSING) {
        foreach my $config (@MISSING) {
            next if ($config =~ /^HAVE_DECL__/); # ignore _XXX decls (specials)
            print "missing:  $config\n";
        }
    }

    if (scalar @MULTIPLE) {
        foreach my $config (@MULTIPLE) {
            print "multiple: $config\n";
        }
    }


    # export
    my $asctime = asctime(localtime());

    chop($asctime);
    open(CONFIG, ">$dir/$file") or
        die "cannot create $dir/$file";
    print CONFIG "/* Generated by makelib.pl, $asctime */\n";
    print CONFIG $text;
    close CONFIG;
}


sub
cannon_path($)          #(name)
{
    my $path  = shift;
    my ($volume, $directories, $file) = File::Spec->splitpath(File::Spec->canonpath($path));
    my (@dur) = File::Spec->splitdir($directories);

    my @dar;
    foreach(@dur){
        if ($_ eq '..') {
            pop @dar;
        } else {
            push @dar, $_;
        }
    }
    $path = File::Spec->catpath($volume, File::Spec->catdir(@dar), $file);
    $path =~ s/\\/\//g;
    return $path;
}


sub
unix2dos($)             #(name)
{
    my $name = shift;
    $name =~ s/\//\\/g;
    return $name;
}


sub
dos2unix($)             #(name)
{
    my $name = shift;
    $name =~ s/\\/\//g;
    return $name;
}


sub
System($)               # (cmd)
{
    my $cmd = shift;
    system($cmd);
    return systemrcode($?);
}


sub
systemrcode($)          # (retcode)
{
    my $rc = shift;
    my $rcode = 0;

    if ($rc == -1) {
        $rcode = -1;                            # task exec error
    } elsif ($rc & 127) {
        $rcode = -2;                            # cored
    } elsif ($rc) {
        $rcode = $rc >> 8;                      # application return code
    }
    return $rcode;
}

#end
