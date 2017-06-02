# Microsoft Developer Studio Project File - Name="app" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=app - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "app.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "app.mak" CFG="app - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "app - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "app - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "app - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "app - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "app - Win32 Release"
# Name "app - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ai"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\ai\ai.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\ai\ai.h
# End Source File
# End Group
# Begin Group "algo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\algo\binheap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\algo\binheap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\algo\checksum.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\algo\checksum.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\algo\mempool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\algo\mempool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\algo\random.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\algo\random.h
# End Source File
# End Group
# Begin Group "app"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\app\gmain.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\app\gmain.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\app\gres.h
# End Source File
# End Group
# Begin Group "drdump"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\drdump\CrashRpt.h
# End Source File
# End Group
# Begin Group "econ"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\econ\balance.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\balance.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\demand.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\demand.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\firm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\firm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\household.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\household.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\institution.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\institution.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\state.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\state.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\utility.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\econ\utility.h
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter ""
# Begin Group "layouts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\chattext.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\chattext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\edgui.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\edgui.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\gameover.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\gamover.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\ggui.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\ggui.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\gviewport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\gviewport.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\messbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\messbox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\playgui.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\layouts\playgui.h
# End Source File
# End Group
# Begin Group "widgets"

# PROP Default_Filter ""
# Begin Group "spez"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\blgraphs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\blgraphs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\blpreview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\blpreview.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\blview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\blview.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\botpan.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\botpan.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\cstrview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\cstrview.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\gengraphs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\gengraphs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\loadview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\loadview.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\lobby.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\lobby.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\newhost.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\newhost.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\pygraphs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\pygraphs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\resticker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\resticker.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\roleview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\roleview.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\saveview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\saveview.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\svlist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\svlist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\truckmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\spez\truckmgr.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\barbutton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\barbutton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\button.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\button.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\checkbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\checkbox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\droplist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\droplist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\editbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\editbox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\frame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\frame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\hscrollbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\hscrollbar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\image.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\image.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\insdraw.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\insdraw.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\link.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\link.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\listbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\listbox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\pane.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\pane.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\text.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\text.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\textarea.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\textarea.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\textblock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\textblock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\touchlistener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\touchlistener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\viewportw.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\viewportw.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\vscrollbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\vscrollbar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\winw.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widgets\winw.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\source\gui\cursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\cursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\draw2d.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\draw2d.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\font.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\font.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\gui.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\gui.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\icon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\icon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\inevent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\inevent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\keymap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\keymap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\richtext.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\richtext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\viewlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\viewlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\gui\widget.h
# End Source File
# End Group
# Begin Group "math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\math\3dmath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\3dmath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\barycentric.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\barycentric.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\brush.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\brush.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\camera.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\camera.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\edge.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\edge.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\fixmath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\fixmath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\frustum.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\frustum.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\hmapmath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\hmapmath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\isomath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\isomath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\line.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\line.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\matrix.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\physics.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\physics.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\plane2i.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\plane2i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\plane3f.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\plane3f.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\polygon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\polygon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\quaternion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\quaternion.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\triangle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec2c.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec2f.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec2i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec2s.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec2uc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec3f.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec3f.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec3i.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec3i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec4f.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\math\vec4f.h
# End Source File
# End Group
# Begin Group "net"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\net\client.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\client.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\download.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\download.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\lockstep.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\lockstep.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\net.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\net.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\netconn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\netconn.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\packets.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\parity.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\parity.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\readpackets.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\readpackets.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\sendpackets.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\sendpackets.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\server.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\server.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\synccheck.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\net\synccheck.h
# End Source File
# End Group
# Begin Group "path"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\path\anypath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\anypath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\astarpath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\astarpath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\collidertile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\collidertile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\fillbodies.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\fillbodies.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\groupmove.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\groupmove.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\itinerary.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\itinerary.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\jpsexpansion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\jpsexpansion.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\jpspartpath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\jpspartpath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\jpspath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\jpspath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\localavoid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\localavoid.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\partialpath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\partialpath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\pathdebug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\pathdebug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\pathjob.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\pathjob.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\pathnode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\pathnode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\reconstructpath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\reconstructpath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\tilepath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\tilepath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\trip.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\path\trip.h
# End Source File
# End Group
# Begin Group "phys"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\phys\collidable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\phys\collidable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\phys\collision.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\phys\collision.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\phys\trace.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\phys\trace.h
# End Source File
# End Group
# Begin Group "render"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\render\anim.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\anim.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\billboard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\billboard.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\cell.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\cell.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\depthable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\depthable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\drawqueue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\drawqueue.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\drawsort.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\drawsort.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\fogofwar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\fogofwar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\foliage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\foliage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\graph.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\graph.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\heightmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\heightmap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\infoov.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\infoov.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\particle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\particle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\projectile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\projectile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\screenshot.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\screenshot.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\shader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\shader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\sprite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\sprite.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\tile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\tile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\transaction.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\transaction.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\vertexarray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\vertexarray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\water.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\render\water.h
# End Source File
# End Group
# Begin Group "save"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\save\savecondition.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\save\savecondition.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\save\saveeffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\save\saveeffect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\save\savehint.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\save\savehint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\save\savemap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\save\savemap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\save\savetrigger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\save\savetrigger.h
# End Source File
# End Group
# Begin Group "script"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\script\objectscript.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\script\objectscript.h
# End Source File
# Begin Source File

SOURCE="..\..\..\source\script\os-binder-arg-cc-functions.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\source\script\os-binder-cc-functions.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\source\script\os-binder-function.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\source\script\os-binder.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\source\script\os-heap.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\source\script\os-heap.h"
# End Source File
# End Group
# Begin Group "sim"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\sim\bltype.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\bltype.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\border.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\border.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\build.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\build.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\building.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\building.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\conduit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\conduit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\connectable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\connectable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\deposit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\deposit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\evqueue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\evqueue.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\job.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\job.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\labourer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\labourer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\manuf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\manuf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\map.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\map.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\order.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\order.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\player.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\player.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\resources.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\resources.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\selection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\selection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\simdef.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\simdef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\simflow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\simflow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\simstate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\simstate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\transport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\transport.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\truck.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\truck.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\umove.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\umove.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\unit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\unit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\utype.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sim\utype.h
# End Source File
# End Group
# Begin Group "sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\sound\sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sound\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sound\soundch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sound\soundch.h
# End Source File
# End Group
# Begin Group "sys"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\sys\dirent.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sys\dirent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sys\namegen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sys\namegen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sys\regress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sys\unicode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sys\unicode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sys\utf8.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sys\utf8.h
# End Source File
# End Group
# Begin Group "trigger"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\trigger\condition.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\condition.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\console.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\console.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\effect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\effect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\hint.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\hint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\script.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\script.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\trigger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\trigger\trigger.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\source\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\language.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\language.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\platform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\platform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\texture.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\texture.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\ustring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\ustring.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\utils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\version.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\version.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\window.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\window.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
