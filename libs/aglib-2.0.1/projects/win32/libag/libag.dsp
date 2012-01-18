# Microsoft Developer Studio Project File - Name="libag" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=libag - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libag.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libag.mak" CFG="libag - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libag - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
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
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBAG_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\src\ag" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AGDB" /Fp"Release/ag.pch" /YX /FD /TP /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../lib/libag.dll"
# Begin Target

# Name "libag - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\ag\AG.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\AGAPI.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\AGDB.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\AGException.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\agfio.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\agfio_plugin.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\agfioError.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\AGSet.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Anchor.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Annotation.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\AnnotationIndex.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\db.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag_dlfcn.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\FeatureMap.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Identifiers.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Metadata.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Paired.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\RE.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Record.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Signal.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Timeline.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\tree_kernel.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\tree_others.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Utilities.cc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\Validation.cc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "ag"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\ag\ag\AG.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\AGAPI.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\AGException.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\agfio.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\agfio_plugin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\agfioError.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\AGSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\agtree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\AGTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Anchor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Annotation.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\AnnotationIndex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\db.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\FeatureMap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Hash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Identifiers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Metadata.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Paired.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\RE.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Record.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\regex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Signal.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Timeline.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Utilities.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ag\ag\Validation.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\ag\ag_dlfcn.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
