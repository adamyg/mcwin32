Notes on Perl
=============

 - [General Notes](#general-notes)
 - [Perl on Linux](#perl-on-linux) 
 - [Perl on Windows](#perl-on-windows)

General Notes
-------------

The build environment relies on Perl to perform a mix of tasks. Under Linux Perl is generally available.

However, if you need to install Perl as binary packages, a recent version of Perl-5 is required.

Perl on Linux
-------------

On Linux distributions based on Debian, the package `perl` will install the core Perl modules as well, so you will be fine.

On Linux distributions based on RPMs, you will need to install `perl-core` rather than just `perl`.


Perl on Windows
---------------

There are a number of build targets that can be viewed as "Windows". There are several configs targeting __Open-Watcom C/C++__, __Visual Studio C/C++__, as well as __MinGW__ and __Cygwin__.

The key recommendation is to use a Perl installation that matches the build environment. For example, if you will build on Cygwin be sure to use the Cygwin package manager to install Perl. For MSYS builds use the MSYS provided Perl. 

For standard target _Strawberry Perl_ is recommend, from <http://strawberryperl.com>. An alternative is ActiveState Perl, from <http://www.activestate.com/ActivePerl>.
