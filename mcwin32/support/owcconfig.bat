@echo off
rem
rem Open Watcom C/C++ 1.9
rem
echo Configure: Open Watcom C/C++ 1.9

rem perl: assume within path
if not defined PERL (
        set PERL=perl
)

rem iscc: command line interface
if not defined INNO (
        set INNO="C:/Program Files (x86)/Inno Setup 5/iscc"
)

%PERL% ./support/config_windows.pl makelib.pl --inno=%INNO% %* owc
