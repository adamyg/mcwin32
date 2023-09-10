@echo off
rem
rem Microsoft Visual Studio C/C++ 2019
rem
echo Configure: Microsoft Visual Studio C/C++ 2019
if not defined GNUWIN32 (
        set GNUWIN32=\devl\gnuwin32
)
if not defined PERL (
        set PERL=perl
)
if not defined INNO (
        set INNO="C:/Program Files (x86)/Inno Setup 5/iscc"
)
%PERL% makelib.pl --busybox=./support/busybox --inno=%INNO% vc2019 %1 %2 %3 %4

