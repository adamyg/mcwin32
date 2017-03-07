@echo off
rem
rem Open Watcom C/C++ 1.9
rem
if not defined GNUWIN32 (
        set GNUWIN32=\devl\gnuwin32
)
if not defined PERL (
        set PERL=perl
)
%PERL% makelib.pl --gnuwin32=%GNUWIN32% owc %1 %2 %3 %4

