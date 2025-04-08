@echo off
rem
rem mingw32 - experimental
rem
echo Configure: mingw32

rem perl: assume within path
if not defined PERL (
        set PERL=perl
)

rem iscc: command line interface
if not defined INNO (
        set INNO="C:/Program Files (x86)/Inno Setup 5/iscc"
)
       
%PERL% ./support/config_windows.pl makelib.pl --busybox=./support/busybox --inno=%INNO% %* mingw32
