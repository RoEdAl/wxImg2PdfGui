;
; installer script
;
#include "ISPPBuiltins.iss"

#define Img2PdfFileVersion GetVersionNumbersString(Img2PdfExe)
#define Img2PdfFileCompany GetFileCompany(Img2PdfExe)
#define Img2PdfFileCopyright GetFileCopyright(Img2PdfExe)
#define Img2PdfFileDescription GetFileDescription(Img2PdfExe)
#define Img2PdfProductVersion GetFileProductVersion(Img2PdfExe)
#define Img2PdfFileVersionString GetFileVersionString(Img2PdfExe)

[Setup]
SourceDir={#Img2PdfFilesDir}
WizardStyle=modern
WizardSizePercent=100,100
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
DefaultDirName={autopf}\{cm:img2pdf}
;SetupIconFile={#SetupIconFile}
ShowLanguageDialog=no
Compression=lzma2/Max
DefaultGroupName={cm:img2pdf}
ArchitecturesAllowed={#Img2PdfArch}
ArchitecturesInstallIn64BitMode={#Img2PdfArch}
TimeStampsInUTC=yes
TouchDate={#Img2PdfTouchDate}
TouchTime={#Img2PdfTouchTime}

[Languages]
Name: en; MessagesFile: compiler:Default.isl; LicenseFile: license.rtf

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked;
Name: tool; Description: {cm:Downloads}
Name: tool/mupdf; Description: {cm:MuPdf}; Flags: checkedonce
Name: tool/sumatrapdf; Description: {cm:SumatraPdf}; Flags: checkedonce unchecked

[Files]
; application files
Source: {#Img2PdfBase}.exe; DestDir: {app}; Flags: comparetimestamp touch
Source: *.dll; DestDir: {app}; Flags: comparetimestamp; Attribs: notcontentindexed
Source: *.txt; DestDir: {app}; Flags: comparetimestamp setntfscompression overwritereadonly uninsremovereadonly; Attribs: readonly notcontentindexed
Source: *.js; DestDir: {app}; Flags: comparetimestamp setntfscompression overwritereadonly uninsremovereadonly; Attribs: readonly notcontentindexed
; MuPDF files extracted from ZIP package
Source: {tmp}/mupdf/*.exe; DestDir: {app}/tools/mupdf; Flags: external comparetimestamp setntfscompression overwritereadonly uninsremovereadonly; Attribs: readonly notcontentindexed; Tasks: tool/mupdf
Source: {tmp}/mupdf/*.txt; DestDir: {app}/tools/mupdf; Flags: external comparetimestamp setntfscompression overwritereadonly uninsremovereadonly; Attribs: readonly notcontentindexed; Tasks: tool/mupdf
; SumatraPDF files extracted from ZIP package + configuration files
Source: {tmp}/sumatrapdf/*.exe; DestDir: {app}/tools/SumatraPDF; Flags: external comparetimestamp setntfscompression overwritereadonly uninsremovereadonly; Attribs: readonly notcontentindexed; Tasks: tool/sumatrapdf
Source: tools/sumatrapdf/*.ini; DestDir: {app}/tools/SumatraPDF; Flags: comparetimestamp setntfscompression overwritereadonly uninsremovereadonly; Attribs: readonly notcontentindexed; Tasks: tool/sumatrapdf
Source: tools/sumatrapdf/*.txt; DestDir: {app}/tools/SumatraPDF; Flags: comparetimestamp setntfscompression overwritereadonly uninsremovereadonly; Attribs: readonly notcontentindexed; Tasks: tool/sumatrapdf
; unpacker (used internally)
Source: "7za.exe"; Flags: dontcopy

[Icons]
Name: {group}\{cm:img2pdf}; Filename: {app}\{#Img2PdfBase}.exe; Comment: {cm:gui_comment}
Name: {group}\{cm:license}; Filename: {app}\license.txt;
Name: {group}\{cm:UninstallProgram,{cm:img2pdf}}; Filename: {uninstallexe}

[Run]
Filename: {app}\{#Img2PdfBase}.exe; Flags: PostInstall RunAsOriginalUser NoWait; Description: {cm:gui_run}; 

[Registry]
; http://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\FileSystem"; ValueType: dword; ValueName: "LongPathsEnabled"; ValueData: 1; MinVersion: 10.0.14393; Check: IsAdminInstallMode;

[CustomMessages]
img2pdf=WxImg2Pdf
en.license=License
en.gui_comment=Image do PDF converter
en.gui_run=Run application
en.Downloads=Downloads:
en.MuPdf=MuPDF - command line tools [version {#MuPdfVersion}]
en.SumatraPdf=SumatraPDF - lightweight PDF viewer [version {#SumatraPdfVersion}]
en.DownloadAbortedByUser=Download aborted by user
en.ExtractingTitle=Extracting downloaded files…
en.ExtractingDesc=Extracting %1
en.ExtractFailed=Fail to extract downloaded package %1
en.RenameFailed=Fail to rename extracted executable %1
 
[Code]

const
    VC_REG_KEY = 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\{#Img2PdfArch}';

var
	DownloadPage: TDownloadWizardPage;

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

<event('InitializeSetup')>
function CheckVCRedist: Boolean;
begin
  Result := IsVCRedistInstalled
  if not Result then
    SuppressibleMsgBox(
        FmtMessage(SetupMessage(msgWinVersionTooLowError), ['Visual C++ 2015-2022 Redistributable ({#Img2PdfArch})', '14.0']),
        mbCriticalError, MB_OK, IDOK
    );
end;

<event('InitializeWizard')>
procedure Download_Init;
begin
  DownloadPage := CreateDownloadPage(SetupMessage(msgWizardPreparing), SetupMessage(msgPreparingDesc), nil);
end;

<event('PrepareToInstall')>
function Download_Run(var NeedsRestart: Boolean): String;
var
	dlCnt: Integer;
	sevenZip: String;
	extractDir: String;
	exeRes: Integer;
begin
	DownloadPage.Clear;
	dlCnt := 0;
	if WizardIsTaskSelected('tool/mupdf') then
	begin
		DownloadPage.Add('https://mupdf.com/downloads/archive/mupdf-{#MuPdfVersion}-windows.zip', 'mupdf-{#MuPdfVersion}-windows.zip', '{#MuPdfChecksum}');
		dlCnt := dlCnt + 1;
	end;
	if WizardIsTaskSelected('tool/sumatrapdf') then
	begin
		DownloadPage.Add('https://www.sumatrapdfreader.org/dl/rel/{#SumatraPdfVersion}/SumatraPDF-{#SumatraPdfVersion}-64.zip', 'SumatraPDF-{#SumatraPdfVersion}-64.zip', '{#SumatraPdfChecksum}');
		dlCnt := dlCnt + 1;
	end;
	if dlCnt > 0 then
	begin
		DownloadPage.ProgressBar.Visible := True;
		DownloadPage.AbortButton.Visible := True;
		DownloadPage.Show;
		try
		  try
			DownloadPage.Download; // This downloads the files to {tmp}
			
			ExtractTemporaryFile('7za.exe');
			extractDir := ExpandConstant('{tmp}')
			sevenZip := ExpandConstant('{tmp}/7za.exe');
			
			DownloadPage.ProgressBar.Visible := False;
			DownloadPage.AbortButton.Visible := False;
			
			if WizardIsTaskSelected('tool/mupdf') then
			begin
				exeRes := 0;
				DownloadPage.SetText(ExpandConstant('{cm:ExtractingTitle}'), ExpandConstant('{cm:ExtractingDesc,MuPDF}'));
				Exec(sevenZip, 'e mupdf-{#MuPdfVersion}-windows.zip -omupdf -r *.txt mutool.exe -aoa -bso0', extractDir, SW_HIDE, ewWaitUntilTerminated, exeRes);
				if exeRes <> 0 then RaiseException(ExpandConstant('{cm:ExtractFailed,MuPDF'));
			end;
			
			if WizardIsTaskSelected('tool/sumatrapdf') then
			begin
				exeRes := 0;
				DownloadPage.SetText(ExpandConstant('{cm:ExtractingTitle}'), ExpandConstant('{cm:ExtractingDesc,SumatraPDF}'));
				Exec(sevenZip, 'e SumatraPDF-{#SumatraPdfVersion}-64.zip -osumatrapdf -aoa -bso0', extractDir, SW_HIDE, ewWaitUntilTerminated, exeRes);
				if exeRes <> 0 then RaiseException(ExpandConstant('{cm:ExtractFailed,SumatraPDF'));
				if not RenameFile(
					ExpandConstant('{tmp}/sumatrapdf/SumatraPDF-{#SumatraPdfVersion}-64.exe'),
					ExpandConstant('{tmp}/sumatrapdf/SumatraPDF.exe')
				) then RaiseException(ExpandConstant('{cm:RenameFailed,SumatraPDF'));
			end;
			
			Result := '';
		  except
			if DownloadPage.AbortedByUser then
				Result := ExpandConstant('{cm:DownloadAbortedByUser}')
			else
				Result := GetExceptionMessage;
		  end;
		finally
		  DownloadPage.Hide;
		end;
	end
	else Result := '';
end;
