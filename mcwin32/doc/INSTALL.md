
# Midnight Commander Win32

## Installation instructions

The project can be built from source, using one of several supported tool-chains.
The following environments and toolchains are supported.

  * Microsoft Visual C++ (MSVC) 2015 - 2022; or
  * Open-Watcom (OWC) 1.9 or 2.0; or
  * Mingw64, both 32 and 64 bit targets.

The build status of these packages is below.

[![Workflow](https://github.com/adamyg/mcwin32/actions/workflows/build.yml/badge.svg)](https://github.com/adamyg/mcwin32/actions)

## Prerequisites

To build and install mcwin32, you shall need:

  * Clone of the source repository.
  * A supported operating system; Windows 10 or 11, plus in theory older version XP thru to Windows 7.
  * Git tooling for windows.
  * Perl 5 with core modules, see [NOTES-PERL.md](doc/NOTES-PERL.md).
  * CoreUtils, includes various text and system utilities.
  * Gettext
  * Make.
  * An ANSI C/C++ compiler.
  * A development environment in the form of development libraries and C header files.

For additional platform specific requirements, solutions to specific issues and other details, please read one of these:

  * [Notes on Perl](doc/NOTES-PERL.md)

Plus additional information is available within the _GitHub_ [workflows](.github/workflows/build.yml).

Quick Installation Guide
========================

If you just want to get going without bothering too much about the details,
here is the short version of how to build and install Midnight Commander Win32.

Confirm both a _Perl_ and tooling which provides _CoreUtils_ and git are installed; for example [__GIT for Windows__](https://gitforwindows.org/). Additional information can be found in later sections.

Create a suitable developer command prompt. If you are using Visual Studio, for example Visual Studio C/C++ 2019, open a "Developer Command Prompt" and issue the following commands:

Clone the github repository

````
 git clone https://github.com/adamyg/mcwin32.git mc
````

Change directory to the primary tree

```
 cd mc\mcwin32
```

Update external dependencies

````
 git submodule update --init --recursive
````

Dependent on the available toolchain, prime using a suitable configuration profile.

```
 .\support\vc2019config
```

Several alternative profiles are available:

  * Microsoft Visual C++ (MSVC) 2015 - 2022; or

    * vc2015config - Visual Studio C/C++ 2015.
    * vc2017config - Visual Studio C/C++ 2017.
    * vc2019config - Visual Studio C/C++ 2019.
    * vc2022config - Visual Studio C/C++ 2022.

  * Open-Watcom (OWC) 1.9 or 2.0; or

    * owcconfig - Open Watcom 1.9
    * owc20config - Open Watcom 2.0

  * MingW64, both 32 and 64 bit targets.

    * mingw32config - mingW64 tool-chain.
    * mingw64config - mingW64, 64bit tool-chain.


The resulting build profile and options shall be available.

```
 -
 -  Configuration:
 -
 -               PackageName: Midnight Commander WIN32
 -                   Version: 4.8.33
 -
 -                 ToolChain: Visual Studio 2019
 -                  Compiler: cl / cl
 -                    CFLAGS: -nologo -MD$(RTSUFFIX) -fp:precise
 -                  CXXFLAGS: -nologo -MD$(RTSUFFIX) -EHsc -fp:precise -Zc:offsetof-
 -                       Release: -O2 -GL -Gy -DNDEBUG
 -                       Debug:   -Zi -RTC1 -Od
 -                   LDFLAGS: -nologo -MD$(RTSUFFIX)
 -
 -
 -      Virtual File Systems: cpio, extfs, shell, ftp, sfs, sftp, tar (see: config.h)
 -            Screen library: console
 -             Mouse support: native
 -          Subshell support: n/a
 -     Background operations: n/a
 -           Internal editor: yes
 -               Diff viewer: yes
 -

 Review the options above for accuracy.

 Execute to build:

    "make release"          - build software.

 To generate an installer:

    "make release package"  - build installer.

 Optionally after installation:

    "make release clean"    - remove build tree.
```

make options are presented when no target are stated, example

```
  |
  | make [VERBOSE=1] [release or debug] target
  |
  |       Build one or more of the following targets recursively within each sub-directory
  |       for the toolchain "Visual Studio 2019" (vs160).
  |
  | Options:
  |       VERBOSE - increase run-time diagnostics; suppress stderr filtering.
  |
  | Targets:
  |
  |       build - build everything.
  |       package - build package.
  |       clean - delete everything which can be remade.
  |       vclean - delete all.
  |       help - command line usage.
  .
```
Once reviewed execute the following:

    $ .\support\gmake-42 release

and optionally a local installer

    $ .\support\gmake-42 release package

alternatively zip and copy the ``bin.<toolchain>/release`` tree to your desired install location.

Tool-chains
===========

The follow offers a more detailed discussion of the requirements and instruction, for each of actively supported tool-chain and third-party tooling solutions:

  - [Native builds using Open-Watcom](#native-builds-using-openwatcom-c-c)
  - [Native builds using Visual C++](#native-builds-using-visual-c-c)
  - [Native builds using MinGW64](#native-builds-using-mingw)

Finally, please review the packaged example alternative configurations as win32 development environments can be problematic, dependent on the host setup:

  - .github/workflows, github build actions for owc, msvc and mingw64 toolchains.
  - Appveyor CI integration notes [Appveyor CI](CINotes.md).

Native builds using Open-Watcom C/C++
====================================

### Perl

A Perl installation needs to be available plus the installation should be visible within the current PATH.

Strawberry Perl is recommended, available from <http://strawberryperl.com/>,
as an alternative is ActiveState Perl, <https://www.activestate.com/ActivePerl>.

### Watcom C/C++

Watcom C/C++ (currently Open Watcom C/C++) is an integrated development environment (IDE) product from Watcom International Corporation for the C, C++, and Fortran programming languages. Watcom C/C++ was a commercial product until it was discontinued, then released under the Sybase ``Open Watcom`` Public License as ``Open Watcom C/C++``.

Two versions of ``Open Watcom C/C++`` are freely available, the legacy _1.9_ version and the current Open _2.0_ development stream, both are actively supported.
These are available from <https://www.openwatcom.org> and <https://github.com/open-watcom/open-watcom-v2>.

Note: Open-Watcom is the current tool-chain published builds utilise.

### Binary utilises

In addition to Perl and the selected compiler tool-chain, several utilises are required. Under Unix like environments these are referred to as the ``binutils`` package, yet are not generally installed on Windows host.

Minimal tools required are:

  * gmake - _GNU make utility_.

      make is a utility which can determine automatically which pieces of a large program need to be recompiled, and issue the commands to recompile them.

  * busybox - _The Swiss Army Knife of Embedded Linux_

      BusyBox combines tiny versions of many common UNIX utilities into a single small executable. It provides replacements for most of the utilities you usually find in GNU fileutils, shellutils, etc. The utilities in BusyBox generally have fewer options than their full-featured GNU cousins; however, the options that are included provide the expected functionality and behave very much like their GNU counterparts. BusyBox provides a fairly complete environment for any small or embedded system

  * coreutils - Collection of file and text manipulation utilities; including

      * cp - _copy files and directories_.

      * mv - _move (rename) files_.

      * rm - _remove files or directories_.

      * grep/egrep - _print lines that match patterns_.

      * gzip, gunzip, zcat - _compress or expand files_.

      * tar - _an archiving utility_.

      Coreutils are bundled with [__GIT for Windows__](https://gitforwindows.org/),
      alternatively install [__MSYS2__](https://www.msys2.org/).

      Once installed the required commands should be visible within the path.

  * gettext - gettext utilities are a set of tools that provides a framework to help packages produce multi-lingual messages.

      Several options are available including:

      * msys64 - ```pacman --noconfirm -S mingw-w64-i686-gettext-tools```

      * [gettext for windows](https://github.com/mlocati/gettext-iconv-windows)

To support native Windows builds, the make tool ``gmake-42``, web tool ``wget`` and the shell support tool ``busybox`` are bundled within the source repository sub-directory ``support/``.

  - ``gmake`` was built from its original source available from [GNU binutils](https://www.gnu.org/software/binutils/).

  - Whereas ``busybox`` was sourced from a recent stable release [BusyBox for Windows](https://frippery.org/busybox).

The configure front-end shall attempt to select the most suitable tools available on the build host.

### InnoSetup

To package the build application as an installer, ``Inno Setup`` is utilised.
``Inno Setup`` is a free installer for Windows program's, available from <https://jrsoftware.org/>.

The Inno package is optional and only required if an installer is being built.

``Inno Setup`` 5.6.x or greater is required. Note 6.x Inno and later shall only function on Vista or greater. 6.x and later versions of Inno no longer support Windows 2000, XP, and Server 2003.

Using Chocolatey, to install _Inno Setup_, run the following on a command line:

    $ choco install innosetup --version=5.6.1

Note: Inno-Setup 5.6.x (Unicode version) is used to package the current installers.


Quick start
-----------

  * Install _Perl_

  * Install _Inno-Setup_

    Install ``Inno Setup`` 5.6.x within its default installation path; if modified the arguments to the support scripts below shall need to be adjusted to match, set the environment variable ``INNO="<install-path>"`` prior to priming the tree.

  * Make sure _Perl_ is on your __\$PATH__.

  * Create a _Open Watcom_ command Prompt and confirm that ``wcl386`` is within your path.

    From the source root, a suitable environment can be setup using the one of the following dependent on the desired toolchain, were ``C:\Watcom`` is the toolchain installation directory.

      * owcconfig- Open Watcom 1.9
      * owc20config - Open Watcom 2.0

  * From the root of the source directory perform the following:

      * Configure and prime the build system.

            $ .\support\owcconfig C:\Watcom

            Note: Optional installation directory

        on completion the make system is ready, run only make shall present usage information:

            $ .\support\gmake-42

            |
            | make [VERBOSE=1] [release or debug] target
            |
            |       Build one or more of the following targets recursively within each sub-directory
            |       for the toolchain "Visual Studio 2019" (vs160).
            |
            | Options:
            |       VERBOSE - increase run-time diagnostics; suppress stderr filtering.
            |
            | Targets:
            |
            |       build - build everything.
            |       package - build package.
            |       clean - delete everything which can be remade.
            |       vclean - delete all.
            |       help - command line usage.
            .

      * Build mcwin32 and associated third-party components.

            $ .\support\gmake-42 release build

      * Optionally, build the installer.

            $ .\support\gmake-42 release package

         Alternatively zip and copy the ``bin.<toolchain>/release`` tree to your desired install location.

The resulting work flow could look like the following, inside a Open Watcom 1.9 developer prompt:

```
cd c:\projects

git clone https://github.com/adamyg/mcwin32.git mc

cd c:\projects\mc\mcwin32

git submodule update --init --recursive

set PERL=c:\Strawberry\perl\bin\perl
set PATH=c:\msys64\usr\bin;%PATH%

.\support\owcconfig

.\support\gmake-42 release
.\support\gmake-42 release package
```


Native builds using Visual C/C++
================================

Visual C/C++ offers an alternative way to build native __mcwin32__, similar to Open-Watcom C/C++ builds.

Microsoft Visual is available in several, all are suitable:

  * Microsoft Visual C++ 2015 - 2002 Professional -

      Standard Microsoft Visual C++ installations.

  * Microsoft Visual C++ 2015 - 2022 Community Edition -

      These free versions of Visual C++ 2015-2022 Professional contain the same compilers and linkers that ship with the full versions,
      and also contain everything necessary to build mcwin32.

  * Microsoft C++ Build Tools -

      There's also a standalone (IDE-less) version of the build tools mentioned above containing the MSVC compiler
      available for download from https://visualstudio.microsoft.com/visual-cpp-build-tools/.

Note: Since these are proprietary and ever-changing I cannot test them all. Older versions may not work, it is recommended to use a recent version wherever possible.

  * Install _Perl_

  * Install _Inno-Setup_

  * Make sure _Perl_ is on your __\$PATH__.

  * Use Visual Studio Developer Command Prompt with administrative privileges, choosing one of its variants depending on the intended architecture. Or run `cmd` and execute `vcvarsall.bat` with one of the options `x86`, `x86_amd64`, `x86_arm`, `x86_arm64`, `amd64`, `amd64_x86`, `amd64_arm`, or `amd64_arm64`. This all setup the environment variables needed for the compiler `cl.exe`.

    See also <https://docs.microsoft.com/cpp/build/building-on-the-command-line>

  * Select a suitable build profile

      * vc2015config - Visual Studio C/C++ 2015.
      * vc2017config - Visual Studio C/C++ 2017.
      * vc2019config - Visual Studio C/C++ 2019.
      * vc2022config - Visual Studio C/C++ 2022.

  * From the root of the source directory perform the following:

      * Configure and prime the build system.

            $ .\support\vc####config

        where #### representes the toolchain, for example 2019.

            $ .\support\vc2019config

      * Build mcwin32 and associated third-party components.

            $ .\support\gmake-42 release

      * Optionally, build the installer.

            $ .\support\gmake-42 release package

The resulting work flow could look like the following, inside a 2019 developer prompt:

```
cd c:\projects

git clone https://github.com/adamyg/mcwin32.git mc

cd c:\projects\mc\mcwin32

git submodule update --init --recursive

set PERL=c:\Strawberry\perl\bin\perl
set PATH=c:\msys64\usr\bin;%PATH%

.\support\vc2019config

.\support\gmake-42 release
.\support\gmake-42 release package
```

Native builds using Mingw
=========================

Mingw64 (32/64) offers another alternative way to build native __mcwin32__, similar to Open-Watcom C/C++ builds.

MSYS2 provides GNU tools, a Unix-like command prompt, and a UNIX compatibility layer for applications,
available from https://www.mingw-w64.org. However, in this context it is only used for building mcwin32.
The resulting application does not rely on MSYS2 to run and is fully native.

  * _MSYS2_ shell, from https://www.msys2.org/

  * _Perl_, at least version 5.10.0, which usually comes pre-installed with MSYS2.

  * Install _Inno-Setup_

  * Create a MSYS/Mingw64 Command Prompt.

    To install the minimal tools required:

        $ pacman --noconfirm -S base-devel

    plus one of the following

        $ pacman --noconfirm -S mingw-w64-x86_64-gcc
        $ pacman --noconfirm -S mingw-w64-i686-gcc

    These compilers must be on your MSYS2 \$PATH, example below assuming the default installation path ``c:/msys64/``.
    A common error is to not have these on your \$PATH. The MSYS2 version of gcc will not work correctly here.

    finally, any additional components

        $ pacman --noconfirm -S mingw-w64-i686-gettext-tools
        $ pacman --noconfirm -S zip

  * From the root of the source directory perform the following:

      * Configure and prime the build system.

          * x64 tool-chain

                  PATH=c:\msys64\mingw64\bin;c:\msys64\usr\bin
                  $ .\support\mingw64config

          * x86 tool-chain.

                  PATH=c:\msys64\mingw32\bin;c:\msys64\usr\bin
                  $ .\support\mingw32config

      * Build mcwin32 and associated third-party components.

            $ .\support\gmake-42 release

      * Optionally, build the installer.

            $ .\support\gmake-42 release package

The resulting work flow could look like the following, inside either a terminal/command or msys prompt:

Install any missing components

```
c:\msys64\usr\bin\pacman --noconfirm -S base-devel
c:\msys64\usr\bin\pacman --noconfirm -S mingw-w64-i686-gcc
c:\msys64\usr\bin\pacman --noconfirm -S mingw-w64-i686-gettext-tools
c:\msys64\usr\bin\pacman --noconfirm -S zip
```

Prime sandbox and build

```
cd c:\projects

git clone https://github.com/adamyg/mcwin32.git mc

cd c:\projects\mc\mcwin32

git submodule update --init --recursive

set PERL=c:\Strawberry\perl\bin\perl
set PATH=c:\msys64\mingw32\bin;c:\msys64\usr\bin;%PATH%

.\support\mingw32config

.\support\gmake-42 release
.\support\gmake-42 release package
```

Last updated: _April/25_

-end-
