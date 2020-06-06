@echo off
rem
rem Microsoft Visual Studio C/C++ 2015
rem
if not defined GNUWIN32 (
        set GNUWIN32=\devl\gnuwin32
)
if not defined PERL (
        set PERL=perl
)
%PERL% makelib.pl --busybox=./support/busybox "--inno=C:/Program Files (x86)/Inno Setup 5/iscc" vc2015 %1 %2 %3 %4

