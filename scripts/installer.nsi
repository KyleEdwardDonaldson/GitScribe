; GitScribe NSIS Installer Script
; Creates a Windows installer that handles everything automatically

!define PRODUCT_NAME "GitScribe"
!define PRODUCT_VERSION "0.1.0"
!define PRODUCT_PUBLISHER "GitScribe Team"
!define PRODUCT_WEB_SITE "https://github.com/gitscribe/gitscribe"

!include "MUI2.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "LICENSE"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN_TEXT "Restart Windows Explorer"
!define MUI_FINISHPAGE_RUN_FUNCTION "RestartExplorer"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language
!insertmacro MUI_LANGUAGE "English"

; Installer details
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "GitScribe-Setup-${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES64\GitScribe"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

Section "MainSection" SEC01
  SetOutPath "$INSTDIR\bin"
  SetOverwrite on

  ; Copy DLLs
  File "dist\GitScribe\bin\GitScribeShell.dll"
  File "dist\GitScribe\bin\gitscribe_core.dll"

  ; Copy resources
  SetOutPath "$INSTDIR\resources"
  File /r "dist\GitScribe\resources\*.*"

  ; Register shell extension
  DetailPrint "Registering shell extension..."
  ExecWait 'regsvr32 /s "$INSTDIR\bin\GitScribeShell.dll"' $0
  ${If} $0 != 0
    MessageBox MB_OK|MB_ICONEXCLAMATION "Failed to register shell extension. Error code: $0"
  ${EndIf}

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Add to Add/Remove Programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoRepair" 1
SectionEnd

Section "Uninstall"
  ; Unregister shell extension
  DetailPrint "Unregistering shell extension..."
  ExecWait 'regsvr32 /u /s "$INSTDIR\bin\GitScribeShell.dll"'

  ; Kill Explorer (will auto-restart)
  KillProcWMI::KillProc "explorer.exe"
  Sleep 2000
  Exec "explorer.exe"

  ; Delete files
  Delete "$INSTDIR\bin\*.dll"
  Delete "$INSTDIR\uninstall.exe"
  RMDir /r "$INSTDIR\resources"
  RMDir /r "$INSTDIR"

  ; Remove from Add/Remove Programs
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
SectionEnd

Function RestartExplorer
  ; Kill and restart Explorer to load shell extension
  KillProcWMI::KillProc "explorer.exe"
  Sleep 2000
  Exec "explorer.exe"
FunctionEnd
