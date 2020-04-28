@echo off
rem
rem Microsoft Visual Studio C/C++ 2013+
rem
if not defined GNUWIN32 (
        set GNUWIN32=\devl\gnuwin32
)
if not defined PERL (
        set PERL=perl
)
%PERL% makelib.pl --gnuwin32=%GNUWIN32% --icu=auto vc2015 %1 %2 %3 %4

