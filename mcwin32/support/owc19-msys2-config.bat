@echo off
rem
rem Open Watcom C/C++ 1.9 with Strawberry Perl and MSYS2
rem

rem Path to MSYS2 binaries. Git for Windows can be used instead of MSYS2.
set MSYS2_PATH=C:\msys64\usr\bin
rem set MSYS2_PATH=C:\Program Files\Git\usr\bin

rem Path to Perl executable. Perl from MSYS2 is not currently supported.
set PERL_PATH=C:\Strawberry\perl\bin

rem Executables provided by this git repository.
set BUSYBOX=support\busybox.exe
set GMAKE=support\gmake-42.exe

rem Add MSYS2 binaries to PATH. Needed for gzip, the --binpath option won't help.
set PATH=%MSYS2_PATH%;%PATH%

rem Configure and create makefiles.
%PERL_PATH%\perl makelib.pl --perlpath=%PERL_PATH% --busybox=%BUSYBOX% owc %1 %2 %3 %4

rem Set the build number, otherwise the build would fail.
if not exist BUILDNUMBER %BUSYBOX% echo 1 >BUILDNUMBER

rem Run the build.
%GMAKE% release
