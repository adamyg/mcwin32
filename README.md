[![Website](https://img.shields.io/badge/View-Website-blue)](https://sourceforge.net/projects/mcwin32/)

[![Workflow](https://github.com/adamyg/mcwin32/actions/workflows/build.yml/badge.svg)](https://github.com/adamyg/mcwin32/actions)]
[![Build status](https://ci.appveyor.com/api/projects/status/4ckxapbwc3mt66x6?svg=true&passingText=MSVC%20Passing&failingText=MSVC%20Failing&pendingText=MSVC%20Pending)](https://ci.appveyor.com/project/adamyg/mcwin32-msvc)

# Midnight Commander for Windows

## Native Midnight Commander 4.8.33 for Windows/Win32

Windows XP+/32 bit native port of GNU Midnight Commander, based on the [4.8.33](https://midnight-commander.org/wiki/NEWS-4.8.33) release.

Supports both the recent Windows 10 plus the prior legacy console, 16 and 256 colour modes are available within either.

Midnight Commander (also known as mc) is a free cross-platform orthodox file manager and a clone of Norton Commander.

Features include the ability work with common archive formats as if they were simply another directory, and to function as an SFTP/FTP client.
Midnight Commander also includes an builtin editor/viewer, features include syntax highlighting for many languages, macros, code snippets, 
simple integration with external tools, automatic indentation, mouse support, clipboard and the ability to work in both ASCII and hex modes.

Midnight Commander supports accessing remote filesystems through several methods, including SSH’s Secure File Transfer Protocol, SFTP. 
This is in addition to FISH, using either a SSH client or legacy RSH connections. This way you can easily transfer files between servers.

Midnight Commander can also rename groups of files, move files to a different directory at the same time as it renames them. 
It lets the user specify the original and resulting file names using wildcard characters.


## Examples

Main panels.

![](https://github.com/adamyg/mcwin32/blob/master/mcwin32/art/sample03.png?raw=true)

Internal diff viewer.

![](https://github.com/adamyg/mcwin32/blob/master/mcwin32/art/sample04.png?raw=true)

Internal editor.

![](https://github.com/adamyg/mcwin32/blob/master/mcwin32/art/sample05.png?raw=true)

## Installation

Recent distributions are now bundled with an installer, the following shall prompt and then check for available upgrades.
Note: Automatic checks wont occur at this time; functionality staged for a later release.

```
mcupdater force
```

Updating older distribution shall require you download from one of the sites listed below or alternatively winget and then update if requried.

```
winget install --id=GNU.MidnightCommander -e
```

To utilise *Files transferred over Shell protocol* **(FISH)** over SSH, an ssh client is required, you can either install [Win32-OpenSSH](https://github.com/powershell/Win32-OpenSSH) or [WinXSH](https://github.com/adamyg/winxsh).  For legacy RSH based connections [WinXSH](https://github.com/adamyg/winxsh) is needed.

Finally the internal editor has built-in spell enabled. To utilise aspell is required, you can optionally install [Win32-Aspell](https://github.com/adamyg/aspell-win32).

## Documentation

The primary way to learn about midnight-commander is to use the context-sensitive online help available via **F1**.

![](https://github.com/adamyg/mcwin32/blob/master/mcwin32/art/sample06.png?raw=true)

There are also extensive manual pages, which are the primary source of official documentation.
Topics include ``mc``, ``mcdiff``, ``mcedit``, and ``mcview`` available using the **mchelp** utility.

```
mchelp mc
```

## Distributions

### Binaries and installers

Midnight Commander:

   * https://github.com/adamyg/mcwin32/releases
   * https://sourceforge.net/projects/mcwin32 (Mirror)

Aspell:

   * https://github.com/adamyg/aspell-win32/releases

### Source

   * https://github.com/adamyg/mcwin32
   
The project can be built from source, the method dependent on the target host. 
See [INSTALL](mcwin32/doc/INSTALL.md) for details, plus working examples are visible within the GitHub workflows.

    https://github.com/adamyg/mcwin32f/blob/master/.github/workflows/build.yml
   
For example, for several tool-chains configuration profiles are available.

```
    cd mcwin32
    .\support\vc2019config
```                      

the following build profile and options shall be available.

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

## Status:

Please feel free to raise tickets on **GitHub** when issues are encountered.

