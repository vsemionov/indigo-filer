; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

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
AppSupportURL=http://www.vsemionov.org/
AppUpdatesURL=http://www.vsemionov.org/
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

