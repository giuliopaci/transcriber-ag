OutFile "TranscriberAG-setup.exe"

Name "TranscriberAG"
!define MUI_PRODUCT "TranscriberAG"
!define MUI_FILE "savefile"
!define MUI_BRANDINGTEXT "TranscriberAG"
CRCCheck On

!include "${NSISDIR}/Contrib/Modern UI/System.nsh"  

Var StartMenuFolder

InstallDir "$PROGRAMFILES\${MUI_PRODUCT}"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\source\COPYING.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU "TranscriberAG" $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_DIRECTORY
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

# For removing Start Menu shortcut in Windows 7
RequestExecutionLevel user

Section

SetOutPath $INSTDIR
File "installdir/bin/TranscriberAG.exe"
File "installdir/bin/*.dll"

SetOutPath $INSTDIR\etc
File /r "..\source\etc\TransAG\*"

# define uninstaller name
WriteUninstaller $INSTDIR\TranscriberAG-uninstall.exe
CreateShortCut "$SMPROGRAMS\TranscriberAG\Uninstall.lnk" "$INSTDIR\TranscriberAG-uninstall.exe"
CreateShortCut "$SMPROGRAMS\TranscriberAG\TranscriberAG.lnk" "$INSTDIR\TranscriberAG.exe"
SectionEnd
 
# create a section to define what the uninstaller does.
# the section will always be named "Uninstall"
Section "Uninstall"
 
# Always delete uninstaller first
Delete $INSTDIR\TranscriberAG-uninstall.exe
 
# now delete installed file
Delete $INSTDIR\test.txt
Delete "$SMPROGRAMS\TranscriberAG\Uninstall.lnk"
Delete "$SMPROGRAMS\TranscriberAG\TranscriberAG.lnk"
 
SectionEnd
