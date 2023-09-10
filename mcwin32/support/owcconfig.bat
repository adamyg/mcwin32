@echo off
rem
rem Open Watcom C/C++ 1.9
rem
echo Configure: Open Watcom C/C++ 1.9
if not defined GNUWIN32 (
        set GNUWIN32=\devl\gnuwin32
)
if not defined PERL (
        set PERL=perl
)
if not defined INNO (
        set INNO="C:/Program Files (x86)/Inno Setup 5/iscc"
)
%PERL% makelib.pl --busybox=./support/busybox --inno=%INNO% owc %1 %2 %3 %4
