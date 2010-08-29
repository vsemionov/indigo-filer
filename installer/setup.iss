
;
; Copyright (c) 2010, Victor Semionov
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;     * Redistributions of source code must retain the above copyright notice,
;       this list of conditions and the following disclaimer.
;     * Redistributions in binary form must reproduce the above copyright notice,
;       this list of conditions and the following disclaimer in the documentation
;       and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
; CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
; INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
; BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
; OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
; PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
; THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
; USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
; DAMAGE.
;

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{21EEE893-9E2A-474C-AF0C-8863E493B211}
AppName=Indigo Filer
AppVersion=0.1
;AppVerName=Indigo Filer 0.1
AppPublisher=Victor Semionov
AppPublisherURL=http://www.vsemionov.org/
AppSupportURL=http://www.vsemionov.org/indigo-filer/
AppUpdatesURL=http://www.vsemionov.org/indigo-filer/
DefaultDirName={pf}\Indigo Filer
DefaultGroupName=Indigo Filer
LicenseFile=..\LICENSE
OutputDir=..\build
OutputBaseFilename=IndigoFilerSetup
Compression=lzma
SolidCompression=yes

UninstallDisplayName=Indigo Filer 0.1
UsePreviousAppDir=yes
UsePreviousGroup=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\build\indigo-filer.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\src\indigo-filer.ini"; DestDir: "{app}"; Flags: onlyifdoesntexist
Source: "..\src\indigo-filer.ini"; DestDir: "{app}"; DestName: "indigo-filer-defaults.ini"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; DestName: "LICENSE.txt"; Flags: ignoreversion
Source: "..\VERSION"; DestDir: "{app}"; DestName: "VERSION.txt"; Flags: ignoreversion
Source: "..\CHANGES"; DestDir: "{app}"; DestName: "CHANGES.txt"; Flags: ignoreversion
Source: "..\CONTRIBUTORS"; DestDir: "{app}"; DestName: "CONTRIBUTORS.txt"; Flags: ignoreversion
Source: "..\README"; DestDir: "{app}"; DestName: "README.txt"; Flags: ignoreversion isreadme
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Indigo Filer"; Filename: "{app}\indigo-filer.exe"
Name: "{commondesktop}\Indigo Filer"; Filename: "{app}\indigo-filer.exe"; Tasks: desktopicon
Name: "{group}\Indigo Filer Settings"; Filename: "{app}\indigo-filer.ini"

[Run]
; The service could be registered by passing /registerService to the executable itself, but it would have manual startup
Filename: "sc"; Parameters: "create IndigoFiler DisplayName= ""Indigo Filer"" binPath= ""{app}\indigo-filer.exe"" depend= tcpip"; Description: "Register service"; Flags: postinstall runascurrentuser runhidden
Filename: "sc"; Parameters: "config IndigoFiler start= auto"; Description: "Automatic startup"; Flags: postinstall runascurrentuser runhidden
Filename: "sc"; Parameters: "start IndigoFiler"; Description: "Start service"; Flags: postinstall runascurrentuser runhidden

[UninstallRun]
Filename: "sc"; Parameters: "stop IndigoFiler"; RunOnceId: "StopService"; Flags: runhidden
Filename: "sc"; Parameters: "delete IndigoFiler"; RunOnceId: "DelService"; Flags: runhidden
Filename: "taskkill"; Parameters: "/F /IM indigo-filer.exe"; RunOnceId: "KillService"; Flags: runhidden

[Code]
{
  Stops and deletes the service, created by any previous installation.
  It's done during pre-install to avoid installation errors, caused by attempts to overwrite an open file.
}
function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  ResultCode: Integer;
begin
  Exec('sc', 'stop IndigoFiler', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec('sc', 'delete IndigoFiler', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec('taskkill', '/F /IM indigo-filer.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

