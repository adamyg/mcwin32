;
; Midnight Commander - Inno Setup
;
; Copyright (c) 2012 - 2025 Adam Young.
; This file is part of the Midnight Commander.
;
; The Midnight Commander is free software: you can redistribute it
; and/or modify it under the terms of the GNU General Public License as
; published by the Free Software Foundation, either version 3 of the License,
; or (at your option) any later version.
;
; The Midnight Commander is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

#if defined(BUILD_INFO)
  #include "..\buildinfo.h"
#else
  #include "..\packageinfo.h"
#endif

#if defined(BUILD_TOOLCHAIN)
   #if defined(BUILD_TYPE)
      #define BinDir "bin" + BUILD_TOOLCHAIN + "\" + BUILD_TYPE
   #else
      #define BinDir "bin" + BUILD_TOOLCHAIN
   #endif
#else
   #define BinDir "bin"
#endif

#define APP_NAME          "Midnight Commander"
#if defined(BUILD_ISWIN64)
    #define FULL_APP_NAME "GNU " + APP_NAME
#else
    #define FULL_APP_NAME "GNU " + APP_NAME + " 64bit"
#endif
#define APP_PUBLISHER_URL "https://www.midnight-commander.org/"
#define APP_SUPPORT_URL   "https://github/adamyg/mcwin32/"
#define APP_AUTHOR        "The Free Software Foundation, Inc."
#define CURRENT_YEAR      GetDateTimeString('yyyy','','')
#define APP_COPYRIGHT     "(c) " + APP_AUTHOR + " " + CURRENT_YEAR

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
;
AppId={{CBB4464D-7081-4F1D-9F6D-F5288A4A9B82}
AppName={#FULL_APP_NAME}
AppVersion={#VERSION} (build: {#BUILD_DATE}-{#BUILD_NUMBER})
AppVerName={#FULL_APP_NAME} {#VERSION} (build: {#BUILD_DATE}-{#BUILD_NUMBER})

VersionInfoVersion={#VERSION_1}.{#VERSION_2}.{#VERSION_3}.{#VERSION_4}
VersionInfoDescription={#APP_NAME} installer
VersionInfoProductName={#APP_NAME}

UninstallDisplayName={#APP_NAME} {#VERSION_1}.{#VERSION_2}.{#VERSION_3}.{#VERSION_4}
UninstallDisplayIcon={app}\mc.exe
AppPublisher={#APP_AUTHOR}

AppPublisherURL={#APP_PUBLISHER_URL}
AppSupportURL={#APP_SUPPORT_URL}
AppUpdatesURL={#APP_SUPPORT_URL}

DefaultDirName={pf}\{#APP_NAME}
DefaultGroupName={#APP_NAME}
LicenseFile=..\{#BinDir}\doc\COPYING

ShowLanguageDialog=yes
UsePreviousLanguage=no
LanguageDetectionMethod=uilanguage

OutputDir=.
#if defined(BUILD_TOOLNAME)
   OutputBaseFilename=mcwin32-build{#BUILD_NUMBER}-{#BUILD_TOOLNAME}-setup
#else
   OutputBaseFilename=mcwin32-build{#BUILD_NUMBER}-setup
#endif
#if defined(BUILD_ISWIN64)
   ArchitecturesInstallIn64BitMode=x64
#endif
Compression=lzma
SolidCompression=yes
ChangesEnvironment=true


[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"

[CustomMessages]
en.AddFolderToPath=Add application directory to your environmental path
en.AlreadyInstalled=is currently installed.'
en.UninstallProgramFirst=Do you want to uninstall it first?

it.AddFolderToPath=Aggiungi la cartella dell'applicazione alla variabile ambiente PATH
it.AlreadyInstalled=è attualmnte installatao.'
it.UninstallProgramFirst=Vuoi disinstallarlo?

[Tasks]
Name: modifypath; Description: {cm:AddFolderToPath}; Flags: unchecked
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Registry]
; Update the default short-cut properties:
;
;   ScreenBufferSize REG_DWORD 0x12c0064      300
;   WindowSize       REG_DWORD 0x280064       100x40
;   FontSize         REG_DWORD 0x100000       16
;   FontFamily       REG_DWORD 0x36
;   FontWeight       REG_DWORD 0x190          Normal
;   FaceName         REG_SZ    Lucida Console
;
Root: HKCU; Subkey: "Console\C:_Program Files (x86)_Midnight Commander_mc.exe"; Flags: noerror; ValueType: dword;  ValueName: "ScreenBufferSize"; ValueData: "$12c0064"
Root: HKCU; Subkey: "Console\C:_Program Files (x86)_Midnight Commander_mc.exe"; Flags: noerror; ValueType: dword;  ValueName: "WindowSize";       ValueData: "$280064"
Root: HKCU; Subkey: "Console\C:_Program Files (x86)_Midnight Commander_mc.exe"; Flags: noerror; ValueType: dword;  ValueName: "FontSize";         ValueData: "$100000"
Root: HKCU; Subkey: "Console\C:_Program Files (x86)_Midnight Commander_mc.exe"; Flags: noerror; ValueType: dword;  ValueName: "FontFamily";       ValueData: "$36"
Root: HKCU; Subkey: "Console\C:_Program Files (x86)_Midnight Commander_mc.exe"; Flags: noerror; ValueType: dword;  ValueName: "FontWeight";       ValueData: "$190"
Root: HKCU; Subkey: "Console\C:_Program Files (x86)_Midnight Commander_mc.exe"; Flags: noerror; ValueType: string; ValueName: "FaceName";         ValueData: "Lucida Console"

Root: HKLM; Subkey: "Software\Midnight-Commander"; ValueType: string; ValueName: ""; ValueData: {app}; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\Midnight-Commander"; ValueType: string; ValueName: "Path"; ValueData: "{app}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\Midnight-Commander"; ValueType: string; ValueName: "UninstallString"; ValueData: {uninstallexe}; Flags: uninsdeletevalue

Root: HKLM; Subkey: "Software\Midnight-Commander"; ValueType: dword;  ValueName: "MajorVersion";  ValueData: "{#VERSION_1}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\Midnight-Commander"; ValueType: dword;  ValueName: "MinorVersion";  ValueData: "{#VERSION_2}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\Midnight-Commander"; ValueType: dword;  ValueName: "PatchVersion";  ValueData: "{#VERSION_3}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\Midnight-Commander"; ValueType: dword;  ValueName: "BuildVersion";  ValueData: "{#VERSION_4}"; Flags: uninsdeletevalue

[Files]
Source: "..\{#BinDir}\README.txt";    DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\ChangeLog.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mc.exe";        DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mcdiff.exe";    DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mcedit.exe";    DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mcview.exe";    DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mchelp.exe";    DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mcbsddiff.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mcupdater.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mcmandoc.exe";  DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mcfile.exe";    DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\mcstart.exe";   DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\busybox.exe";   DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\enca.exe";      DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\kbtest.exe";    DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\*.dll";         DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\man2hlp.pl";    DestDir: "{app}"; Flags: ignoreversion
Source: "..\{#BinDir}\etc\*";         Excludes: ".created"; DestDir: "{app}\etc";       Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\{#BinDir}\plugin\*";      Excludes: ".created"; DestDir: "{app}\plugin";    Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\{#BinDir}\libexec\*";     Excludes: ".created"; DestDir: "{app}\libexec";   Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\{#BinDir}\share\*";       Excludes: ".created"; DestDir: "{app}\share";     Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\{#BinDir}\share\man\*";   Excludes: ".created"; DestDir: "{app}\share\man"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\{#BinDir}\doc\*";         Excludes: ".created"; DestDir: "{app}\doc";       Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\{#BinDir}\locale\*";      Excludes: ".created"; DestDir: "{app}\locale";    Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Dont use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#FULL_APP_NAME}"; Filename: "{app}\mc.exe"
Name: "{commondesktop}\{#FULL_APP_NAME}"; Filename: "{app}\mc.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#FULL_APP_NAME}"; Filename: "{app}\mc.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\mc.exe"; Description: "{cm:LaunchProgram,GNU Midnight Commander}"; Flags: nowait postinstall skipifsilent

[Code]
function MidnightCommanderInstalled(var version, uninstallcmd:string):boolean;
var major,minor,patch,build:Cardinal;
begin
        Result := RegQueryDWordValue(HKLM, 'Software\Midnight-Commander', 'MajorVersion', major)
                  and RegQueryDWordValue(HKLM, 'Software\Midnight-Commander', 'MinorVersion', minor)
                  and RegQueryDWordValue(HKLM, 'Software\Midnight-Commander', 'PatchVersion', patch)
                  and RegQueryDWordValue(HKLM, 'Software\Midnight-Commander', 'BuildVersion', build);

        if not RegQueryStringValue(HKLM, 'Software\Midnight-Commander', 'UninstallString', uninstallcmd)
        then if not RegQueryStringValue(HKLM, 'Software\Midnight-Commander', 'Uninstall', uninstallcmd)
        then RegQueryStringValue(HKLM, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\Midnight-Commander', 'UninstallString', uninstallcmd);
        version := IntToStr(major)+'.'+IntToStr(minor)+'.'+IntToStr(patch)+'-'+IntToStr(build);
end;

function InitializeSetup(): Boolean;
        var version, uninst :string;
            msgres, execres :integer;
begin
        Result:=true;
        if MidnightCommanderInstalled(version,uninst)
        then
                begin
                msgres:= MsgBox('{#APP_NAME} '+version+' {cm:AlreadyInstalled}'+#13#13+'{cm:UninstallProgramFirst}', mbError, MB_YESNOCANCEL);
                case msgres of
                IdYes: begin
                        Exec(uninst, '', '', SW_SHOWNORMAL, true, execres);
                        Result:=InitializeSetup();
                        end;
                IdCancel:
                         Result:=false;
                IdNo: ;
                end;
        end;
end;

[Code]
const   ModPathName = 'modifypath';
        ModPathType = 'user';

function ModPathDir(): TArrayOfString;
begin
        setArrayLength(Result, 1)
        Result[0] := ExpandConstant('{app}');
end;

procedure DosToUnix();
var
        path : String;
        data : String;
        ANSIdata : AnsiString;
begin
        path := ExpandConstant(CurrentFileName);
        LoadStringFromFile(path, ANSIdata);
        data := String(ANSIData);
        StringChangeEx(data, #13#10, #10, True);
        SaveStringToFile(path, AnsiString(data), False);
end;

#include "modpath.iss"
