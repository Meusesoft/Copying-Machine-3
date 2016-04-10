; CopyingMachine307.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install makensisw.exe into a directory that the user selects,

!define VER_FILE "309"
!define VER_DISPLAY "3 beta 9"

;--------------------------------

;Include Modern UI

  !include "MUI.nsh"
;--------------------------------

; The name of the installer
Name "Copying Machine ${VER_DISPLAY}"

!define MUI_ICON  "icon.ico"
!define MUI_UNICON  "icon.ico"

; The file to write
OutFile "copymach${VER_FILE}.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Meusesoft\Copying Machine 3"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Meusesoft\Copying Machine 3" ""

;--------------------------------

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "copymach_eula.rtf"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
 
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"


;--------------------------------

; The stuff to install
Section "Copying Machine Core (required)" CopyMachCore

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\release\copyingmachine3.exe"
  File "..\release\cm_english.dll"
  File "..\release\cm_dutch.dll"
  File "..\release\cm_german.dll"
  File "..\release\cm_tesseract.dll"
  File "..\release\jpeg62.dll"
  File "..\release\msvcm90.dll"
  File "..\release\msvcp90.dll"
  File "..\release\msvcr90.dll"
  File "..\release\SciLexer.dll"
  File "..\release\Microsoft.VC90.CRT.manifest"
  
  SetOutPath $INSTDIR\tessdata
  File "..\release\tessdata\eng.DangAmbigs"
  File "..\release\tessdata\eng.freq-dawg"
  File "..\release\tessdata\eng.inttemp"
  File "..\release\tessdata\eng.normproto"
  File "..\release\tessdata\eng.pffmtable"
  File "..\release\tessdata\eng.unicharset"
  File "..\release\tessdata\eng.user-words"
  File "..\release\tessdata\eng.word-dawg"
 
  File "..\release\tessdata\nld.DangAmbigs"
  File "..\release\tessdata\nld.freq-dawg"
  File "..\release\tessdata\nld.inttemp"
  File "..\release\tessdata\nld.normproto"
  File "..\release\tessdata\nld.pffmtable"
  File "..\release\tessdata\nld.unicharset"
  File "..\release\tessdata\nld.user-words"
  File "..\release\tessdata\nld.word-dawg"

  File "..\release\tessdata\deu.DangAmbigs"
  File "..\release\tessdata\deu.freq-dawg"
  File "..\release\tessdata\deu.inttemp"
  File "..\release\tessdata\deu.normproto"
  File "..\release\tessdata\deu.pffmtable"
  File "..\release\tessdata\deu.unicharset"
  File "..\release\tessdata\deu.user-words"
  File "..\release\tessdata\deu.word-dawg"

  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Meusesoft\Copying Machine" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CopyingMachine" "DisplayName" "Copying Machine ${VER_DISPLAY}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CopyingMachine" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CopyingMachine" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CopyingMachine" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  nsExec::exec '$INSTDIR\copyingmachine3.exe /register'
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Help files" CopyMachHelp

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR\help\english
  
  ; Put file there
  File "..\release\help\english\index.html"
  File "..\release\help\english\cm_help_commandlineoptions.html"
  File "..\release\help\english\cm_help_credits.html"
  File "..\release\help\english\cm_help_documentcollections.html"
  File "..\release\help\english\cm_help_interfaces.html"
  File "..\release\help\english\cm_help_introduction.html"
  File "..\release\help\english\cm_help_menu_document.html"
  File "..\release\help\english\cm_help_menu_edit.html"
  File "..\release\help\english\cm_help_menu_file.html"
  File "..\release\help\english\cm_help_menu_help.html"
  File "..\release\help\english\cm_help_menu_language.html"
  File "..\release\help\english\cm_help_menu_page.html"
  File "..\release\help\english\cm_help_menu_view.html"
  File "..\release\help\english\cm_help_mode_easy.html"
  File "..\release\help\english\cm_help_troubleshooting.html"
  File "..\release\help\english\cm_help_whatisnew.html"

  SetOutPath $INSTDIR\help\english\images

  File "..\release\help\english\images\default.css"
  File "..\release\help\english\images\easy.png"
  File "..\release\help\english\images\header_bg.gif"
  File "..\release\help\english\images\q_bg.gif"


  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts" CopyMachStart

  CreateDirectory "$SMPROGRAMS\Copying Machine"
  CreateShortCut "$SMPROGRAMS\Copying Machine\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Copying Machine\Copying Machine.lnk" "$INSTDIR\copyingmachine3.exe" "" "$INSTDIR\copyingmachine3.exe" 0
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcut" CopyMachDesktop

  CreateShortCut "$DESKTOP\Copying Machine.lnk" "$INSTDIR\copyingmachine3.exe" "" "$INSTDIR\copyingmachine3.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  SetDetailsPrint textonly
  DetailPrint "Deleting Registry Keys..."
  SetDetailsPrint listonly

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CopyingMachine"
  DeleteRegKey HKLM "SOFTWARE\Meusesoft\Copying Machine 3"

   nsExec::exec '$INSTDIR\copyingmachine3.exe /unregister'

  SetDetailsPrint textonly
  DetailPrint "Deleting Files..."
  SetDetailsPrint listonly

  ; Remove all files, if any
  Delete "$INSTDIR\*.*"
  Delete "$INSTDIR\help\*.*"
  Delete "$INSTDIR\help\english\*.*"
  Delete "$INSTDIR\help\english\images\*.*"
  Delete "$INSTDIR\tessdata\*.*"
  Delete "$DESKTOP\Copying Machine.lnk"
  Delete "$SMPROGRAMS\Copying Machine\Uninstall.lnk"
  Delete "$SMPROGRAMS\Copying Machine\Copying Machine.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Copying Machine"
  RMDir "$INSTDIR\help\english\images"
  RMDir "$INSTDIR\help\english"
  RMDir "$INSTDIR\help"
  RMDir "$INSTDIR\tessdata"
  RMDir "$INSTDIR"

  
  SetDetailsPrint both

SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${CopyMachCore} "The core files required to use Copying Machine"
  !insertmacro MUI_DESCRIPTION_TEXT ${CopyMachHelp} "The help files of Copying Machine"
  !insertmacro MUI_DESCRIPTION_TEXT ${CopyMachStart} "Adds icons to your start menu"
  !insertmacro MUI_DESCRIPTION_TEXT ${CopyMachDesktop} "Adds icon to your desktop for easy access"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

