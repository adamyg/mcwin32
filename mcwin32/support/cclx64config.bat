@echo off
rem
rem Clang-cl/Microsoft Visual Studio/x64
rem
echo Configure: Clang-cl/Microsoft Visual Studio C/C++ x64

rem perl: assume within path
if not defined PERL (
        set PERL=perl
)

rem iscc: command line interface
if not defined INNO (
        set INNO="C:/Program Files (x86)/Inno Setup 5/iscc"
)

%PERL% ./support/config_windows.pl makelib.pl --inno=%INNO% %* clangcl-x64
