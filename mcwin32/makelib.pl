#!/usr/bin/perl
# $Id: makelib.pl,v 1.8 2017/03/13 16:52:06 cvsuser Exp $
# Makefile generation under WIN32 (MSVC/WATCOMC/MINGW) and DJGPP.
# -*- tabs: 8; indent-width: 4; -*-
# Automake emulation for non-unix environments.
#
# Copyright Adam Young 2012-2018
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
use POSIX 'asctime';
use Data::Dumper;
use Text::ParseWords;

my $CWD                     = getcwd();
my $BINPATH                 = '';
my $PERLPATH                = '';
my $BUSYBOX                 = 'busybox';
my $LIBTOOL                 = '';
my $PROGRAMFILES            = ProgramFiles();

my $x_libw32                = 'libw32';

my %x_environment   = (
        'dj'            => {    # DJGPPP
            TOOLCHAIN       => 'dj',
            TOOLCHAINEXT    => '.dj',
            CC              => 'gcc',
            CXX             => 'g++',
            AR              => 'ar',
            CFLAGS          => '-g -fno-strength-reduce -I$(ROOT)/djgpp',
            CWARN           => '-W -Wall -Wshadow -Wmissing-prototypes',
            },

        'mingw'         => {    # MingW
            build_os        => 'mingw32',
            TOOLCHAIN       => 'mingw',
            TOOLCHAINEXT    => '.mingw',
            CC              => 'gcc',
            CXX             => 'g++',
            OSWITCH         => '',
            LSWITCH         => '-l',
            XSWITCH         => '-o',
            AR              => 'ar',
            RC              => 'windres -DGCC_WINDRES',
            DEFS            => '-DHAVE_CONFIG_H -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x501',
            CINCLUDE        => '-I$(ROOT)/libw32',
            CFLAGS          => '-std=gnu11 -g -fno-strength-reduce',
            CXXFLAGS        => '-std=c++11 -g -fno-strength-reduce',
            CWARN           => '-W -Wall -Wshadow -Wmissing-prototypes',
            CXXWARN         => '-W -Wall -Wshadow',
            LDEBUG          => '',
            LDMAPFILE       => '-Xlinker -Map=$(MAPFILE)',
            LIBS            => '-lw32',
            EXTRALIBS       => '-lshlwapi -lpsapi -lole32 -luuid -lgdi32 '.
                                    '-luserenv -lnetapi32 -ladvapi32 -lshell32 -lWs2_32',
            LIBTHREAD       => '-lpthread',
            LIBMALLOC       => '-ldlmalloc',
            },

        'vc1200'        => {    # Visual Studio 7
            TOOLCHAIN       => 'vs70',
            TOOLCHAINEXT    => '.vs70',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32 -I$(ROOT)/libw32/msvc',
            CFLAGS          => '-nologo -Zi -Yd -GZ -MTd',
            CXXFLAGS        => '-nologo -Zi -Yd -GZ -MTd',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -Zi -GZ -MTd',
            LDMAPFILE       => '-MAP:$(MAPFILE)'
            },

        'vc1400'        => {    # 2005, Visual Studio 8
            TOOLCHAIN       => 'vs80',
            TOOLCHAINEXT    => '.vs80',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32 -I$(ROOT)/libw32/msvc',
            CFLAGS          => '-nologo -Zi -RTC1 -MTd',
            CXXFLAGS        => '-nologo -Zi -RTC1 -MTd -EHsc',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -Zi -RTC1 -MTd',
            LDMAPFILE       => '-MAP:$(MAPFILE)',

            MFCDIR          => '',
            MFCCFLAGS       => '-nologo -Zi -RTC1 -MD$(USE_DEBUG)',
            MFCCXXFLAGS     => '-nologo -Zi -RTC1 -MD$(USE_DEBUG) -EHsc',
            MFCCOPT         => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCXXOPT       => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCINCLUDE     => '',
            MFCLIBS         => ''
            },

        'vc1500'        => {    # 2008, Visual Studio 9
            TOOLCHAIN       => 'vs90',
            TOOLCHAINEXT    => '.vs90',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32 -I$(ROOT)/libw32/msvc_compat -I$(ROOT)/libw32/msvc',
            CFLAGS          => '-nologo -Zi -RTC1 -MTd',
            CXXFLAGS        => '-nologo -Zi -RTC1 -MTd -EHsc',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -Zi -RTC1 -MTd',
            LDMAPFILE       => '-MAP:$(MAPFILE)'
            },

        'vc1600'        => {    # 2010, Visual Studio 10
            TOOLCHAIN       => 'vs100',
            TOOLCHAINEXT    => '.vs100',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32 -I$(ROOT)/libw32/msvc',
            CFLAGS          => '-nologo -Zi -RTC1 -MTd',
            CXXFLAGS        => '-nologo -Zi -RTC1 -MTd -EHsc',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -Zi -RTC1 -MTd',
            LDMAPFILE       => '-MAP:$(MAPFILE)',

            MFCDIR          => '/tools/WinDDK/7600.16385.1',
            MFCCFLAGS       => '-nologo -Zi -RTC1 -MD$(USE_DEBUG)',
            MFCCXXFLAGS     => '-nologo -Zi -RTC1 -MD$(USE_DEBUG) -EHsc',
            MFCCOPT         => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCXXOPT       => '-Zc:wchar_t- -Zc:forScope -Gm',
            MFCCINCLUDE     => '-I$(MFCDIR)/inc/atl71 -I$(MFCDIR)/inc/mfc42',
            MFCLIBS         => '/LIBPATH:$(MFCDIR)\lib\atl\i386 /LIBPATH:$(MFCDIR)\lib\mfc\i386'
            },

       'vc1900'        => {    # 2015, Visual Studio 19
            TOOLCHAIN       => 'vs140',
            TOOLCHAINEXT    => '.vs140',
            CC              => 'cl',
            COMPILERPATH    => '%VCINSTALLDIR%/bin',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32 -I$(ROOT)/libw32/msvc',
            CFLAGS          => '-nologo -Zi -RTC1 -MDd -fp:precise',
            CXXFLAGS        => '-nologo -Zi -RTC1 -MDd -EHsc -fp:precise',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -Zi -RTC1 -MTd',
            LDMAPFILE       => '-Fm$(MAPFILE)',
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
            TOOLCHAIN       => 'vs141',
            TOOLCHAINEXT    => '.vs141',
            CC              => 'cl',
            COMPILERPATH    => '%VCToolsInstallDir%/bin/Hostx86/x86',
            VSWITCH         => '',
            VPATTERN        => undef,
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32 -I$(ROOT)/libw32/msvc',
            CFLAGS          => '-nologo -Zi -RTC1 -MDd -fp:precise',
            CXXFLAGS        => '-nologo -Zi -RTC1 -MDd -EHsc -fp:precise',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -Zi -RTC1 -MTd',
            LDMAPFILE       => '-Fm$(MAPFILE)',
            },

        'wc1300'        => {    # Watcom 11
            TOOLCHAIN       => 'wc11',
            TOOLCHAINEXT    => '.wc11',
            CC              => 'wcl386',
            COMPILERPATH    => '%WATCOM%/binnt',
            VSWITCH         => '-c',
            VPATTERN        => '(Watcom .*? Version [0-9\.]+)',
            OSWITCH         => '-Fo',
            LSWITCH         => '',
            XSWITCH         => '-Fe',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32',
            CFLAGS          => '-nologo -showwopts -passwopts:"-q -d2" -Yd -MTd',
            CXXFLAGS        => '-nologo -showwopts -passwopts:"-q -d2" -Yd -MTd',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-nologo -passwopts:"-q -d2" -MTd',
            LDMAPFILE       => '-MAP:$(MAPFILE)'
            },

        'owc1900'       => {    # Open Watcom 1.9
            TOOLCHAIN       => 'owc19',
            TOOLCHAINEXT    => '.owc19',
            CC              => 'wcl386',
            COMPILERPATH    => '%WATCOM%/binnt',
            VSWITCH         => '-c',
            VPATTERN        => '(Open Watcom .*? Version [0-9\.]+)',
            ISWITCH         => '-i=',
            OSWITCH         => '-fo=',
            LSWITCH         => '',
            XSWITCH         => '-fe=',
            AR              => 'lib',
            CINCLUDE        => '-I$(ROOT)/libw32',

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
                #  or -hw   Generate Watcomc debugging information.
                # -db       Generate browsing information (.mbr).
                # -of+      Always generate traceable stack frames.
                # -zlf      Add default library information to objects.
                # -aa       Allow non-constant initialisation expressions.
                # -bt=nt    Build target Windows NT (or greater).
                # -bm       Multi-threaded environment.
                # -br       Build with dll run-time library.
                # -sg       Enable stack guard management (for functions with >= 4K of local variables).
                #
                # -cc++     Treat source files as C++ code.
                # -xs       Exception handling: balanced (C++).
                # -xr       Enable RTTI (C++).
                #
                # Others:
                # -za99     Enable C99 ANSI compliance (1).
                # -ecc      __cdecl (2).
                #
                # (1) Use with caution, beta undocumented feature and not 100% stable.
                # (2) Avoid changing the call convention from #r/#s, otherwise runtime library issues.
                #
            CFLAGS          => '-q -6r -j -ei -d2  -hw -db -of+ -zlf -bt=nt -bm -br -aa -sg',
            CXXFLAGS        => '-q -6r -j -ei -d2i -nw -db -of+ -zlf -bt=nt -bm -br -cc++ -xs -xr',
            CWARN           => '-W3',
            CXXWARN         => '-W3',
            LDEBUG          => '-q -6r -d2 -hw -db -bt=nt -bm',
            LDMAPFILE       => '-fm=$(MAPFILE)',

            MFCDIR          => '\tools\WinDDK\7600.16385.1',
            MFCCOPT         => '-q -j -ei -6r -d2  -hw -db -ofr -zlf -bt=nt -bm -br -aa',
            MFCCXXOPT       => '-q -j -ei -6r -d2i -nw -db -ofr -zlf -bt=nt -bm -br -xs -xr -cc++',
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
        RMDIR               => '@BINPATH@rmdir.exe',

        ISWIN32             => 'yes',
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
        LT_OBJDIR           => '.libs/',
        RC                  => 'rc',

        LIBS                => '$(D_LIB)/libw32.lib',
        EXTRALIBS           => 'advapi32.lib gdi32.lib'.
                                  ' shlwapi.lib shell32.lib psapi.lib ole32.lib'.
                                  ' userenv.lib user32.lib ws2_32.lib wsock32.lib',
        LIBMALLOC           => 'libdlmalloc.lib',
        LIBOPENSSL          => ''
        );

my %x_tokens        = (
        #host, build etc
        PACKAGE             => 'MC',
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
        OSWITCH             => '',              # object specification
        LSWITCH             => '',              # library
        XSWITCH             => '-o',            # exec specification

        #makefile
        SET_MAKE            => 'MAKEFLAGS=',
        CP                  => 'cp',
        RM                  => 'rm',
        MV                  => 'mv',
        TAR                 => 'tar',
        MKDIR               => 'mkdir',
        RMDIR               => 'rmdir',

        INSTALL             => 'install.pl',
        INSTALL_PROGRAM     => 'install.pl',
        INSTALL_DATA        => 'install.pl',

        RANLIB              => 'ranlib',
        YACC                => 'bison -y',
        GREP                => 'egrep',
        AWK                 => 'awk',
        SED                 => 'sed',
        PERL                => 'perl',
        LIBTOOL             => 'libtool',

        CDEBUG              => '',
        CWARN               => '',
        CXXDEBUG            => '',
        CXXWARN             => '',
        DEFS                => '-DHAVE_CONFIG_H',
        CINCLUDE            => '',

        LIBCURL_CPPFLAGS    => '',

        LDFLAGS             => '',
        LIBS                => '',
        LIBENCA             => '',
        LIBSPELL            => '',
        LIBYACC             => '',
        LIBICU              => '',
        LIBICONV            => '',
        LIBCURL             => '',
        LIBMAGIC            => '',
        LIBARCHIVE          => '',
        LIBZ                => '',
        LIBBZ2              => '',
        LIBLZMA             => '',
        LIBREGEX            => '',
        EXTRALIBS           => '',
        TERMLIB             => '',
        LIBM                => '',
        LIBX11              => '',
        LIBMALLOC           => '',
        LIBTHREAD           => ''
        );

my @x_headers       = (
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
        'sys/mount.h',
        'sys/stat.h',
        'sys/statfs.h',
        'sys/statvfs.h',
        'sys/vfs.h',
        'stdarg.h',
        'stdlib.h',
        'stdio.h',
        'limits.h',
        'inttypes.h',                           # c99
        'stdint.h',                             # c99
        'stdbool.h',                            # c99
        'stdatomic.h',                          # c11
        'stdalign.h',                           # c11
        'threads.h',                            # c11
        'pthread.h',                            # MINGW
        'string.h', 'strings.h',
        'errno.h',
        'wchar.h', 'wctype.h',
        'time.h',                               # TIME_WITH_SYS_TIME
        'alloca.h',                             # alloca()
        'env.h',
        'fcntl.h',
        'fenv.h',
        'float.h',
        'poll.h',
        'io.h',
        'memory.h',
        'process.h',
        'libgen.h',                             # basename(), dirname()
        'limits.h',
        'share.h',
        'signal.h',
        'utime.h',
        'wait.h',

        'windows.h',
        'wincrypt.h',
        'bcrypt.h',

        'unistd.h',
        'dirent.h',
        'dlfcn.h',                              # dlopen()
        'pwd.h',
        'grp.h'
        );

my @x_types         = (
        'inline',
        '__inline',
        '__int8',
        '__int16',
        '__int32',
        '__int64',
        'intmax_t',
        'uintmax_t',
        'intptr_t',
        'uintptr_t',
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
        'char16_t',
        'char32_t',
        'bool',
        '_Bool:C99BOOL',
        '_bool',
        'ssize_t',
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
        'void_p'
        );

my @x_functions     = (
        'putenv',
        'setenv',
        'rename',
        'bcmp', 'bzero',
        'memcmp', 'memset', 'memmove',
        'index', 'rindex',                      # bsd
        'strcasecmp', '__strcasecmp', 'stricmp',
        'strtoul',
        'strnlen',
        'strerror',
        'strftime',
        'strchr', 'strrchr', 'strdup',
        'strlcpy', 'strlcat',                   # bsd/linux
            'strsep', 'strnstr', 'strcasestr', 'strcasestr_l', 'strtonum',
        'strtof', 'strtold', 'strtoll',
        'strverscmp', '__strverscmp',
        'mkdtemp',                              # bsd/linux
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
        'mbrtowc', 'wcrtomb', 'wcscmp', 'wcscpy', 'wcslen', 'wctomb', 'wmemcmp', 'wmemmove', 'wmemcpy',
        'fgetpos', 'fsetpos',
        'fseeko', 'fgetln',                     # bsd/linux extensions
        'truncate', 'ftruncate',
        'getline', 'getdelim',                  # bsd/linux
        'ctime_r', 'localtime_r', 'gmtime_r', 'asctime_r', #posix.1
        'mktime',
        'timegm',                               # bsd/linux extensions
        'feclearexpect',                        # fenv.h/c99
        'round',                                # c99
        'nearbyintf',
        'va_copy', '__va_copy',                 # c99/gnu
        'opendir',
        'findfirst', '_findfirst',              # msvc
        );

my @x_commands      = (
        'mkdir',
        'rmdir',
        'tar',
        'mv',
        'cp'
        );

our @x_libraries    = ();   # local libraries -l<xxx> lib<xxx>.lib
our @x_libraries2   = ();   # local libraries -l<xxx> xxx.lib
our @x_optlibraries = ();   # optional libraries
our @x_makefiles    = ();   # local makefiles; build order

my %CONFIG_O        = (     # optional config.h values
        HAVE_EIGHTBIT           => '1'
        );

my %CONFIG_H        = (     # predefined config.h values
        IS_LITTLE_ENDIAN        => '1',         # TODO
        STDC_HEADERS            => '1',
        HAVE_EIGHTBIT           => '1',
        HAVE_BCMP               => '1',
        HAVE_BZERO              => '1',
        HAVE_ENVIRON            => '1',
        HAVE_SYSERRLIST         => '1'
        );

our @HEADERS        = ();
our @EXTHEADERS     = ();
our %TYPES          = ();
our %SIZES          = ();
our %FUNCTIONS      = ();

my @INCLUDES        = ();
my @LIBS            = ();
my @EXTRALIBS       = ();
my @DLLS            = ();

my $x_tmpdir        = '.makelib';
my $x_compiler      = '';
my $x_version       = '';
my @x_include       = ();
my $x_command       = '';
my $x_signature     = undef;

my $o_keep          = 0;
my $o_version       = undef;
my $o_gnuwin32      = 'auto';
my $o_contrib       = 1;
my $o_gnulibs       = 0;

my $o_icu           = 'auto';
my $o_libhunspell   = undef;
my $o_libarchive    = undef;
my $o_libmagic      = undef;


#   Main ---
#       Mainline
#
sub Configure($$);
sub LoadContrib($$$);
sub CheckCompiler($$);
sub CheckType($$);
sub CheckSize($$);
sub CheckFunction($$);
sub CheckICUFunction($);
sub CheckCommand($$;$);
sub CheckExec($$;$);
sub ExpandENV($);
sub System($);
sub systemrcode($);
sub DumpList($$);
sub ExportPath($);
sub ImportDLL($$;$$);
sub Makefile($$$);
sub MakefileDir($);
sub Config($$);

exit &main();

sub
main()
{
    my $o_clean         = 0;
    my $o_help          = 0;

    require "makelib.in";

    my $ret
        = GetOptions(
                'binpath=s'     => \$BINPATH,
                'perlpath=s'    => \$PERLPATH,
                'busybox=s'     => \$BUSYBOX,
                'version=i'     => \$o_version,
                'icu=s'         => \$o_icu,
                'gnuwin32=s'    => \$o_gnuwin32,
                'gnulibs'       => \$o_gnulibs,
                'contrib'       => \$o_contrib,
                'libtool'       => \$LIBTOOL,
                'libhunspell=s' => \$o_libhunspell,
                'libarchive=s'  => \$o_libarchive,
                'libmagic=s'    => \$o_libmagic,
                'clean'         => \$o_clean,
                'keep'          => \$o_keep,
                'help'          => \$o_help
                );

    Usage() if (!$ret || $o_help);
    Usage("expected command") if (scalar @ARGV < 1);
    Usage("unexpected arguments $ARGV[1] ...") if (scalar @ARGV > 1);

    my $cmd = $ARGV[0];

    # see: https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B
    #
    #   MSVC++ 9.0   _MSC_VER == 1500 (Visual Studio 2008 version 9.0)
    #   MSVC++ 10.0  _MSC_VER == 1600 (Visual Studio 2010 version 10.0)
    #   MSVC++ 11.0  _MSC_VER == 1700 (Visual Studio 2012 version 11.0)
    #   MSVC++ 12.0  _MSC_VER == 1800 (Visual Studio 2013 version 12.0)
    #   MSVC++ 14.0  _MSC_VER == 1900 (Visual Studio 2015 version 14.0)
    #   MSVC++ 14.1  _MSC_VER == 1910 (Visual Studio 2017 version 15.0)
    #   MSVC++ 14.11 _MSC_VER == 1911 (Visual Studio 2017 version 15.3)
    #   MSVC++ 14.12 _MSC_VER == 1912 (Visual Studio 2017 version 15.5)
    #   MSVC++ 14.13 _MSC_VER == 1913 (Visual Studio 2017 version 15.6)
    #   MSVC++ 14.14 _MSC_VER == 1914 (Visual Studio 2017 version 15.7)
    #
    if ('vc12' eq $cmd)         { $o_version = 1200, $cmd = 'vc' }
    elsif ('vc14' eq $cmd)      { $o_version = 1400; $cmd = 'vc' } elsif ('vc2005' eq $cmd) { $o_version = 1400; $cmd = 'vc' }
    elsif ('vc15' eq $cmd)      { $o_version = 1400; $cmd = 'vc' } elsif ('vc2008' eq $cmd) { $o_version = 1500; $cmd = 'vc' }
    elsif ('vc16' eq $cmd)      { $o_version = 1600; $cmd = 'vc' } elsif ('vc2010' eq $cmd) { $o_version = 1600; $cmd = 'vc' }
    elsif ('vc19' eq $cmd)      { $o_version = 1900; $cmd = 'vc' } elsif ('vc2015' eq $cmd) { $o_version = 1900; $cmd = 'vc' }
    elsif ('vc1910' eq $cmd)    { $o_version = 1910; $cmd = 'vc' } elsif ('vc2017' eq $cmd) { $o_version = 1910; $cmd = 'vc' }
    if (! $o_version) {
        if ($cmd eq 'vc')       { $o_version = 1400; }
        elsif ($cmd eq 'wc')    { $o_version = 1300; }
        elsif ($cmd eq 'owc')   { $o_version = 1900; }
        else { $o_version = 0; }
    }

    if ($cmd eq 'vc' ||
            $cmd eq 'owc' || $cmd eq 'wc' ||
            $cmd eq 'dj' || $cmd eq 'mingw') {

        my $cache = "${x_tmpdir}/${cmd}${o_version}.cache";

        eval {
            do "$cache" if (! $o_clean && -f $cache);
        };

        #build
        Configure($cmd, $o_version);

        foreach (@x_makefiles) {
            Makefile($cmd, $_, 'Makefile');
        }
        Makefile($cmd, $x_libw32, 'package.h');
        Config($cmd, $x_libw32);

        #cache
        open(CACHE, ">${cache}") or
                die "cannot create <$cache> : $!\n";
        $Data::Dumper::Purity = 1;
        $Data::Dumper::Sortkeys = 1;
        print CACHE Data::Dumper->Dump([\%x_tokens],   [qw(*XXTOKENS)]);
        print CACHE Data::Dumper->Dump([\%CONFIG_H],   [qw(*XXCONFIG_H)]);
        print CACHE Data::Dumper->Dump([\@HEADERS],    [qw(*XXHEADERS)]);
        print CACHE Data::Dumper->Dump([\@EXTHEADERS], [qw(*XXEXTHEADERS)]);
        print CACHE Data::Dumper->Dump([\%TYPES],      [qw(*TYPES)]);
        print CACHE Data::Dumper->Dump([\%SIZES],      [qw(*SIZES)]);
        print CACHE Data::Dumper->Dump([\%FUNCTIONS],  [qw(*FUNCTIONS)]);
        print CACHE "1;\n";
        close CACHE;

        #summary
        DumpList('INCLUDES', \@INCLUDES);
        DumpList('LIBS',     \@LIBS);
        DumpList('EXTRALIB', \@EXTRALIBS);
        DumpList('DLLS',     \@DLLS);

    } elsif ($cmd eq 'clean') {
        foreach (@x_makefiles) {
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
    $path =~ s/\\/\//g;
    return $path;
}



#   Usage ---
#       Makelib command line usage.
#
sub
Usage                   # (message)
{
    print "\nmakelib @_\n\n" if (@_);
    print <<EOU;

Usage: perl makelib.pl [options] <command>

Options:
    --help                  command line help.

    --libtool=<path>        path to libtool_win32.pl.

    --binpath=<path>        path of support binaries (gmake etc), otherwise these are assumed to be in the path.

    --perlpath=<path>       PERL binary path, otherwise assumed in the path.

    --gnuwin32=<path>       gnuwin32 tool installation path.

    --contib                enable local contrib libraries (default).

    or --gnulibs            search and enable gnuwin32 libraries, using
                            gnuwin32 path.

    --libarchive=<path>     libarchive installation path.

    --libmagic=<path>       libmagic installation path.

    --icu=<path>            ICU installation path.

    --version=<version      compiler version

    --clean                 clean build, ignoring cache.

    --keep                  keep temporary file images.

Commands:
    vc[14|16]           Visual Studio C/C++ Makefiles.
    wc                  Watcom C/C++, using 'cl' interface.
    owc                 Open Watcom C/C++, using a direct interface.
    dj                  DJGPP.
    clean               clean.

EOU
    exit(42);
}


#   Config ---
#       Configuration.
#
sub
Configure($$)           # (type, version)
{
    my ($type, $version) = @_;
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
            ($version ? sprintf("%s%d", $type, $version) : $type);

    (exists $x_environment{$signature}) or
        die "makelib: unknown environment $type, version $version\n";

    $x_signature = $signature;                  # active environment
    my $env = $x_environment{$signature};

    if ('dj' ne $type) {                        # WIN32 profile
        foreach my $entry (keys %win_entries) {
            my $token = $win_entries{$entry};

            $token =~ s/\$\<LIBTOOL>/${LIBTOOL}/;
            $x_tokens{$entry} = $token;
        }
    }

    foreach my $entry (keys %$env) {            # target profile
        $x_tokens{$entry} = $$env{$entry};
    }

    (-d $x_tmpdir || mkdir($x_tmpdir)) or
        die "makelib: unable to access/create tmpdir <$x_tmpdir> : $!\n";

    CheckCompiler($type, $env);

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
    my @INCLUDE = split(/;/, $ENV{"INCLUDE"});

    push @INCLUDE, @x_include;
    foreach my $header (@x_headers) {
        my $fullpath = undef;

        print "header:   ${header} ...";
        print " " x (28 - length($header));
        foreach my $include (@INCLUDE) {
            $fullpath = "${include}/${header}";
            $fullpath =~ s/\\/\//g;
            if (-f $fullpath) {
                print "[${fullpath}]";

                push @HEADERS, $header;
                push @EXTHEADERS, $header
                    if ($include ne $x_libw32);
                $header =~ s/[\/.]/_/g;
                $header = uc($header);
                $CONFIG_H{"HAVE_${header}"} = '1';
                last;
            }
            $fullpath = undef;
        }
        print "[not found]" if (! defined $fullpath);
        print "\n";
    }

    # types
    foreach my $typespec (@x_types) {
        my $name   = $typespec;
        my $define = uc($typespec);
        $define =~ s/ /_/g;
        if ($typespec =~ /^(.+):(.+)$/) {
            $name   = $1;
            $define = $2;                       # optional explicit #define
        }

        my $cached = (exists $TYPES{$name});
        my $status = ($cached ? $TYPES{$name} : -1);

        print "type:     ${name} ...";
        print " " x (28 - length($name));

        if (1 == $status ||
                (-1 == $status && 0 == CheckType($type, $name))) {
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

    # compiler/environment
    if ($type eq 'vc' || $type eq 'wc' || $type eq 'owc') {
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

            foreach my $lib (@x_optlibraries) {
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
            foreach (@x_makefiles) {
                my $dir = MakefileDir($_);

                if (-f "${dir}/makelib.def") {
                    my $name = basename($dir);
                    LoadContrib($name, $dir, \@CONTRIBINCS);
                    $contribs{$name} = 1;
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
                LoadContrib($name, $o_icu, \@CONTRIBINCS);
                $contribs{'icu'} = 1;

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
LoadContrib($$$)        # (name, dir, refIncludes)
{
    my ($name, $dir, $refIncludes) = @_;
    my $basepath = ($dir ? $dir : "contrib/${name}");
    my $def = "${basepath}/makelib.def";
    my $lbl = "HAVE_".uc($name);

    return 0 if (-f $basepath);

    print "contrib:  $basepath\n";

    open(CFG, "<${def}") or
        die "cannot open <$def> : $!\n";
    while (defined (my $line = <CFG>)) {
        $line =~ s/\s*([\n\r]+|$)//;
        next if (!$line || /^\s#/);

        my @parts = split(/=/, $line, 2);
        if (2 == scalar @parts) {
            my ($key, $val) = @parts;

            if ('inc' eq $parts[0]) {
                $val = "${basepath}/".$parts[1]
                    if ($val !~ /^\//);
                push @$refIncludes, '$(ROOT)/'.$val;
                print "\tinc: $val\n";

            } elsif ('lbl' eq $key) {
                $lbl = uc($val);

            } elsif ('lib' eq $key) {
                $x_tokens{$lbl} = ExportPath($val);
                print "\tlib: $val (\@$lbl\@)\n";

            } elsif ('ext' eq $key) {
                $x_tokens{$lbl} .= ' '.ExportPath($val);
                print "\text: $val (\@$lbl\@)\n";

            } elsif ('def' eq $key) {
                if ($val =~ /^(.+)=(.*)$/) {
                    $CONFIG_H{$1} = ($2 ? $2 : '1');
                } else {
                    $CONFIG_H{$val} = '1';
                }
                print "\tdef: $val\n";
            }
        }
    }
    close(CFG);
    return 1;
}


sub
CheckCompiler($$)       # (type, env)
{
    my ($type, $env) = @_;

    $x_compiler  = ExpandENV($$env{COMPILERPATH}).'/'
        if (exists $$env{COMPILERPATH});
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

    my @CINCLUDE = split(/ /, $$env{CINCLUDE});

    foreach (@CINCLUDE) {
        if (/^-I\$\(ROOT\)(.*)$/) {
            push @x_include, "${CWD}$1";
        } elsif (/^-I(.*)$/) {
            push @x_include, $1;
        } else {
            push @x_include, $_;
        }
    }

    my $includes = '';
    $includes .=                                # <edidentifier.h>
         (exists $$env{ISWITCH} ? $$env{ISWITCH} : '-I')."${CWD}/include ";
    foreach (@x_include) {
        $includes .=                            # implied
           (exists $$env{ISWITCH} ? $$env{ISWITCH} : '-I').$_.' ';
    }

    $CONFIG_H{MAKELIB_CC_COMPILER} = "\"".basename(${x_compiler})."\"";
    $CONFIG_H{MAKELIB_CC_VERSION} = "\"${x_version}\"";

    print "compiler: ${x_compiler}\n";
    print "version:  ${x_version}\n";
    print "command:  ${x_command}\n";
    print "includes: ${includes}\n";

    # build final command
    $x_command  .= "__FLAGS__ ";
    $x_command  .= $includes;
    $x_command  .= "$$env{OSWITCH}__OBJ__ "
        if ($$env{OSWITCH} ne '');
    $x_command  .= "__LIB__ ";                  # lib's
    $x_command  .= "$$env{XSWITCH}__EXE__ ";
    $x_command  .= "__SOURCE__ >__BASE__.out 2>&1";
    $x_command  =~ s/\//\\/g;

    $$env{CXX}  = $$env{CC}
        if (! exists $$env{CXX});
}


sub
CheckType($$)           # (type, name)
{
    my ($type, $name) = @_;

    my $t_name = $name;
    $t_name =~ s/ /_/g;

    my $BASE   = "${type}_${t_name}";
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
 *  Generated by makelib.pl, $asctime (CheckType)
$cmdparts
 */
${config}
EOT

    if ($t_name =~ /inline/) {
        print TMP<<EOT;
static ${name} function(void) {
    return 1;
}
int main(int argc, char **argv) {
    return function();
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
#       Check that the function exists
#   Returns:
#       0 on success, otherwise non-zero.
#
sub
CheckFunction($$)       # (type, name)
{
    my ($type, $name) = @_;

    my $BASE   = "${type}_${name}";
    my $SOURCE = "${BASE}.c";
    my ($cmd, $cmdparts)
            = CheckCommand($BASE, $SOURCE);
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
${config}
#if defined(__STDC__)
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif
EOT

    foreach my $header (@HEADERS) {
        print TMP "#include <$header>\n";
    }

    print TMP<<EOT;
typedef void (*function_t)(void);
static function_t function;
int main(int argc, char **argv) {
    function = (function_t)(&$name);
    return 1;
}
EOT
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
#if defined(__STDC__)
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
#if defined(__STDC__)
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
    $config .= "\n#define HAVE_WCHAR_H"
            if (exists $CONFIG_H{HAVE_WCHAR_H});
    $config .= "
#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif
#if defined(HAVE_STDBOOL_H)
#include <stdbool.h>
#endif
#if defined(HAVE_WCHAR_H)
#include <wchar.h>
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
CheckCommand($$;$)      # (base, source, pkg)
{
    my ($base, $source, $pkg) = @_;

    my $env = $x_environment{$x_signature};
    my $cmd = $x_command;

    my $flags = '';
    my $optional = '';
    my $lib = '';

    if ($source =~ /\.cpp/) {
        $flags = $$env{CXXFLAGS}.' '
            if (exists $$env{CXXFLAGS});
    } else {
        $flags = $$env{CFLAGS}.' '
            if (exists $$env{CFLAGS});
    }

    if (defined $pkg) {
        if ('icu' eq $pkg) {
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
        }
    }

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
#       cmd - Compiler command.
#       exec - Optional boolean flag, if *true* the resulting application
#           is executed.
#   Returns:
#       cmd, cmdparts
#
sub
CheckExec($$;$)         # (base, cmd, [exec])
{
    my ($base, $cmd, $exec) = @_;

    print "(cd tmpdir; $cmd)\n"
        if ($o_keep);

    chdir($x_tmpdir) or
        die "cannot access directory <$x_tmpdir> : $!\n";

    unlink("${base}.exe");
    my $ret = System($cmd);
    $ret = -999
        if (! -f "${base}.exe");
    $ret = System($base)
        if (0 == $ret && $exec);

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
    while (<MAKEFILE>) {
        $_ =~ s/\s*(\n|$)//;                    # kill trailing whitespace & nl
        $text .= "$_\n";
    }
    close MAKEFILE;

    # Installation paths and install options
    $text =~ s/(\nPREFIX=[ \t]*)[^\n]+/$1\/grief/g;
    $text =~ s/(\nBINDIR=[ \t]*)[^\n]+/$1\/grief\/bin/g;
    $text =~ s/(\nDATADIR=[ \t]*)[^\n]+/$1\/grief/g;
    $text =~ s/-o bin -g bin//g;

    $text =~ s/\Q>\/dev\/null\E/>nul/g;         # nul redirection

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

        if ($type ne 'owc') {
            $text =~ s/(\$\(CFLAGS\).*) -o \$\@/$1 -Fo\$@ -Fd\$(\@D)\//g;
            $text =~ s/(\$\(CXXFLAGS\).*) -o \$\@/$1 -Fo\$@ -Fd\$(\@D)\//g;
            $text =~ s/(\$\(LDFLAGS\).*) -o \$\@/$1 -Fe\$@ -Fd\$(\@D)\//g;
        }

        if ($type eq 'vc') {                    # LIBPATH usage
            $text =~ s/-L/\/link \/LIBPATH:/g;

        } else {
            $text =~ s/-L([^\s]+)/-"LIBPATH $1"/g;

            if ($type eq 'owc') {               # OpenWatcom
                # options
                $text =~ s/(\$\(CFLAGS\).*) -o \$\@/$1 -Fo=\$(subst \/,\\,\$@)/g;
                $text =~ s/(\$\(CXXFLAGS\).*) -o \$\@/$1 -Fo=\$(subst \/,\\,\$@)/g;
                $text =~ s/(\$\(LDFLAGS\).*) -o \$@/$1 -Fe=\$(subst \/,\\,\$@)/g;
                $text =~ s/-Fe(.*) \$\(([A-Z_]*OBJS)\)/-Fe$1 \$(subst \/,\\,\$($2))/g;

                # directory slash conversion
                $text =~ s/-I([^\s]+)/-i="$1"/g;
                $text =~ s/-I ([^\s]+)/-i="$1"/g;
                $text =~ s/\$</\$(subst \/,\\,\$<)/g;
                $text =~ s/\$\^/\$(subst \/,\\,\$^)/g;
            }

            $clean .= ' *.err';
            $xclean .= ' $(D_OBJ)/*.mbr';
        }

        # libraries
        foreach my $library (@x_libraries) {
            $text =~ s/-l${library}([\n\t \\])/lib${library}.lib$1/g;
        }

        foreach my $library (@x_libraries2) {
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
    $x_tokens{top_builddir} = ($dir eq '.' ? '.' : '..');
    $x_tokens{top_srcdir} = ($dir eq '.' ? '.' : '..');
    if ('-i=' eq $x_tokens{ISWITCH}) {
        $x_tokens{CINCLUDE} =~ s/-I([^\s]+)/-i="$1"/g;
    }

    $x_tokens{CXX} = $x_tokens{CC}              # CXX=CC
        if (!exists $x_tokens{CXX});
    foreach my $entry (keys %x_tokens) {
        my $quoted_entry = quotemeta($entry);
        my $replace = $x_tokens{$entry};

        $text =~ s/\@$quoted_entry\@/$replace/g;
    }

    if ($BUSYBOX) {                             # command interface rework
        $text =~ s/\@sh /\@\@BUSYBOX\@ sh /g;
        $text =~ s/\-sh /-\@BUSYBOX\@ sh /g;
        $text =~ s/shell date /shell \@BUSYBOX\@ date /g;
        $text =~ s/shell cat /shell \@BUSYBOX\@ cat /g;
    }

    $text =~ s/\@BINPATH\@/${BINPATH}/g;
    $text =~ s/\@PERLPATH\@/${PERLPATH}/g;
    $text =~ s/\@BUSYBOX\@/${BUSYBOX}/g;

    $text =~ s/(\$\(RM\)) (.*)/$1 \$(subst \/,\\,$2)/g;
    $text =~ s/(\$\(RMDIR\)) (.*)/$1 \$(subst \/,\\,$2)/g;

    # export
    my $asctime = asctime(localtime());

    chop($asctime);
    open(MAKEFILE, ">$dir/$file") or
        die "cannot create $dir/$file";
    if ($file eq 'Makefile') {
        print MAKEFILE "# Generated by makelib.pl, $asctime\n";
    } elsif ($file =~ /.h$/) {
        print MAKEFILE "/* Generated by makelib.pl, $asctime */\n";
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
Config($$)              # (type, dir)
{
    my ($type, $dir) = @_;
    my $file = 'config.h';
    my $text = "";

    # import
    if ($dir =~ /libw32/) {                     # override 'w32config.h'
        $file = 'w32config.h'
            if (-f "${dir}/w32config.hin" || "${dir}/w32config.h.in");
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
cannon_path($)
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





