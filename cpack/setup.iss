#include "ISPPBuiltins.iss"

#define Img2PdfFileVersion GetVersionNumbersString(Img2PdfExe)
#define Img2PdfFileCompany GetFileCompany(Img2PdfExe)
#define Img2PdfFileCopyright GetFileCopyright(Img2PdfExe)
#define Img2PdfFileDescription GetFileDescription(Img2PdfExe)
#define Img2PdfProductVersion GetFileProductVersion(Img2PdfExe)
#define Img2PdfFileVersionString GetFileVersionString(Img2PdfExe)

[Setup]
SourceDir={#Img2PdfFilesDir}
AppID={#Img2PdfBase}
AppName={cm:img2pdf}
AppVerName={cm:img2pdf} {#Img2PdfArch} {#Img2PdfFileVersion}
AppVersion={#Img2PdfFileVersion}
AppCopyright={#Img2PdfFileCopyright}
AppPublisher={#Img2PdfFileCompany}
VersionInfoProductName={#Img2PdfBase} ({#Img2PdfArch})
VersionInfoDescription={#Img2PdfFileDescription}
VersionInfoVersion={#Img2PdfFileVersion}
VersionInfoCompany={#Img2PdfFileCompany}
VersionInfoCopyright={#Img2PdfFileCopyright}
DefaultDirName={autopf}\{#Img2PdfBase}
;SetupIconFile={#SetupIconFile}
ShowLanguageDialog=no
Compression=lzma2/Max
DefaultGroupName={#Img2PdfBase}
ArchitecturesAllowed={#Img2PdfArch}
ArchitecturesInstallIn64BitMode={#Img2PdfArch}
TimeStampsInUTC=yes
TouchDate={#Img2PdfTouchDate}
TouchTime={#Img2PdfTouchTime}

[Languages]
Name: en; MessagesFile: compiler:Default.isl; LicenseFile: license.rtf

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked;

[Files]
Source: {#Img2PdfBase}-frontend.exe; DestDir: {app}; Flags: comparetimestamp touch
Source: *.dll; DestDir: {app}; Flags: comparetimestamp; Attribs: notcontentindexed;
Source: *.txt; DestDir: {app}; Flags: comparetimestamp setntfscompression overwritereadonly uninsremovereadonly; Attribs: readonly notcontentindexed;
Source: *.js; DestDir: {app}; Flags: comparetimestamp setntfscompression overwritereadonly uninsremovereadonly; Attribs: readonly notcontentindexed;

[Icons]
Name: {group}\{cm:img2pdf} {cm:gui}; Filename: {app}\{#Img2PdfBase}-frontend.exe; IconFilename: {app}\{#Img2PdfBase}-frontend.exe; Comment: {cm:gui_comment}
Name: {group}\{cm:license}; Filename: {app}\license.txt;
Name: {group}\{cm:UninstallProgram,{cm:img2pdf}}; Filename: {uninstallexe}

[Run]
Filename: {app}\{#Img2PdfBase}-frontend.exe; Flags: PostInstall RunAsOriginalUser NoWait; Description: {cm:gui_run}; 

[Registry]
; http://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\FileSystem"; ValueType: dword; ValueName: "LongPathsEnabled"; ValueData: 1; MinVersion: 10.0.14393; Check: IsAdminInstallMode;

[CustomMessages]
img2pdf=img2pdf
en.license=License
en.gui=GUI
en.gui_comment=Frontend to mutool console utility
en.gui_run=Run img2pdf frontend
 
[Code]

const
    VC_REG_KEY = 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\{#Img2PdfArch}';

function IsVCRedistInstalled: Boolean;
var 
  major, minor, bld, rbld: Cardinal;
  packedVersion, minPackedVersion : int64;
begin
  if
    RegQueryDWordValue(HKEY_LOCAL_MACHINE, VC_REG_KEY, 'Major', major) and
    RegQueryDWordValue(HKEY_LOCAL_MACHINE, VC_REG_KEY, 'Minor', minor) and
    RegQueryDWordValue(HKEY_LOCAL_MACHINE, VC_REG_KEY, 'Bld', bld) and
    RegQueryDWordValue(HKEY_LOCAL_MACHINE, VC_REG_KEY, 'Rbld', rbld)
  then
  begin
    packedVersion := PackVersionComponents(major, minor, bld, rbld);
    minPackedVersion := PackVersionComponents(14, 0, 0, 0);
    Result := packedVersion >= minPackedVersion;
  end
  else
    Result := False;
end;

function InitializeSetup: Boolean;
begin
  Result := IsVCRedistInstalled
  if not Result then
    SuppressibleMsgBox(
        FmtMessage(SetupMessage(msgWinVersionTooLowError), ['Visual C++ 2015-2022 Redistributable ({#Img2PdfArch})', '14.0']),
        mbCriticalError, MB_OK, IDOK
    );
end;
