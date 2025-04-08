@echo off
rem
rem mingw64 - experimental
rem
echo Configure: mingw64

rem perl: assume within path
if not defined PERL (
        set PERL=perl
)

rem iscc: command line interface
if not defined INNO (
        set INNO="C:/Program Files (x86)/Inno Setup 5/iscc"
)

%PERL% ./support/config_windows.pl makelib.pl --busybox=./support/busybox64 --inno=%INNO% mingw64 %1 %2 %3 %4
