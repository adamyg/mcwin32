@echo off
echo Open Watcom 1.9 Build Environment
if not "%1" == "" SET OWC19=%1
if not defined OWC19 (
        SET OWC19=d:\tools\WC19
)
PATH %OWC19%\BINNT;%OWC19%\BINW;%PATH%
SET  INCLUDE=%OWC19%\H;%OWC19%\H\NT;%OWC19%\H\NT\DIRECTX;%OWC19%\H\NT\DDK;%INCLUDE%
SET  WATCOM=%OWC19%
SET  EDPATH=%OWC19%\EDDAT
SET  WHTMLHELP=%OWC19%\BINNT\HELP
SET  WIPFC=%OWC19%\WIPFC
