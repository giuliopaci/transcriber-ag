# Microsoft Developer Studio Project File - Name="regex" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=regex - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "regex.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "regex.mak" CFG="regex - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "regex - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "..\..\..\src\ag" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Target

# Name "regex - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\ag\regcomp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\regerror.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\regexec.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\regfree.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "ag"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\ag\ag\regex.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\ag\cclass.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\cname.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\engine.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\engine.ih
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\regcomp.ih
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\regerror.ih
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\regex2.h
# End Source File
# End Group
# End Target
# End Project
