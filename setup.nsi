; ==============================================================================
; AiraLang v1.5.0 Official Windows Setup Wizard Script
; Engineered by Adam Eehan (Aira Group of Technology)
; ==============================================================================

Unicode true
Name "AiraLang v1.5.0 (64-bit)"
OutFile "/data/data/com.termux/files/home/airalang-website/downloads/AiraLang-Setup-v1.5.0.exe"
InstallDir "$LOCALAPPDATA\Programs\AiraLang"
InstallDirRegKey HKCU "Software\AiraLang" "Install_Dir"
RequestExecutionLevel user

SetCompressor /SOLID lzma

; Pages
Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

LicenseData "LICENSE"
LicenseText "Please review the license terms before installing AiraLang v1.5.0." "I Agree"

; ==============================================================================
; Sections
; ==============================================================================

Section "AiraLang Core Engine (required)" SecCore
  SectionIn RO

  SetOutPath "$INSTDIR"
  File "LICENSE"
  File "README.md"
  File "welcome.aira"

  SetOutPath "$INSTDIR\src"
  File /r "src\*.*"

  SetOutPath "$INSTDIR\bin"
  File "bin\airalang.cmd"
  File "bin\airalang.bat"
  File "bin\airalang.ps1"

  ; Store installation folder
  WriteRegStr HKCU "Software\AiraLang" "Install_Dir" "$INSTDIR"

  ; Write Uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Register in Windows Add/Remove Programs (Control Panel)
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "DisplayName" "AiraLang Programming Language v1.5.0"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "DisplayVersion" "1.5.0"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "Publisher" "Adam Eehan (Aira Group of Technology)"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "HelpLink" "https://adameehantech.gitlab.io/airalang-website"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "URLInfoAbout" "https://adameehantech.gitlab.io/airalang-website"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang" "NoRepair" 1
SectionEnd

Section "Standard Library (22 Modules)" SecModules
  SetOutPath "$INSTDIR\modules"
  File /r "modules\*.*"
SectionEnd

Section "Add AiraLang to User PATH" SecPath
  ; Read user PATH
  ReadRegStr $0 HKCU "Environment" "Path"
  StrCmp $0 "" PathEmpty

  ; Append to PATH
  WriteRegStr HKCU "Environment" "Path" "$0;$INSTDIR\bin"
  Goto PathDone

PathEmpty:
  WriteRegStr HKCU "Environment" "Path" "$INSTDIR\bin"

PathDone:
  ; Broadcast environment change to Windows explorer/terminals
  SendMessage 0xffff 0x001A 0 "STR:Environment" /TIMEOUT=5000
SectionEnd

Section "Desktop & Start Menu Shortcuts" SecShortcuts
  CreateDirectory "$SMPROGRAMS\AiraLang"
  CreateShortcut "$SMPROGRAMS\AiraLang\AiraLang REPL.lnk" "$INSTDIR\bin\airalang.cmd" "" "$INSTDIR\bin\airalang.cmd" 0
  CreateShortcut "$SMPROGRAMS\AiraLang\Uninstall AiraLang.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$DESKTOP\AiraLang REPL.lnk" "$INSTDIR\bin\airalang.cmd" "" "$INSTDIR\bin\airalang.cmd" 0
SectionEnd

; ==============================================================================
; Uninstaller Section
; ==============================================================================

Section "Uninstall"
  ; Remove shortcuts
  Delete "$DESKTOP\AiraLang REPL.lnk"
  Delete "$SMPROGRAMS\AiraLang\AiraLang REPL.lnk"
  Delete "$SMPROGRAMS\AiraLang\Uninstall AiraLang.lnk"
  RMDir "$SMPROGRAMS\AiraLang"

  ; Remove installed files
  RMDir /r "$INSTDIR\src"
  RMDir /r "$INSTDIR\modules"
  RMDir /r "$INSTDIR\bin"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\welcome.aira"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"

  ; Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AiraLang"
  DeleteRegKey HKCU "Software\AiraLang"
SectionEnd
