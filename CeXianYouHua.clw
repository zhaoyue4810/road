; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CCeXianYouHuaView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "CeXianYouHua.h"
LastPage=0

ClassCount=10
Class1=CCeXianYouHuaApp
Class2=CCeXianYouHuaDoc
Class3=CCeXianYouHuaView
Class4=CMainFrame

ResourceCount=8
Resource1=IDD_DIALOG1
Resource2=IDD_SHOWCOR
Class5=CChildFrame
Class6=CAboutDlg
Resource3=IDD_ABOUTBOX
Resource4=IDR_MAINFRAME
Class7=CGeoDialog
Resource5=IDD_JWDIALOG
Class8=CShowCoordinary
Resource6=IDR_CEXIANTYPE
Class9=CShow_Contrast
Resource7=IDR_POPMENU
Class10=ZoomDlg
Resource8=IDD_ZOOM

[CLS:CCeXianYouHuaApp]
Type=0
HeaderFile=CeXianYouHua.h
ImplementationFile=CeXianYouHua.cpp
Filter=N

[CLS:CCeXianYouHuaDoc]
Type=0
HeaderFile=CeXianYouHuaDoc.h
ImplementationFile=CeXianYouHuaDoc.cpp
Filter=N
BaseClass=CDocument
VirtualFilter=DC
LastObject=CCeXianYouHuaDoc

[CLS:CCeXianYouHuaView]
Type=0
HeaderFile=CeXianYouHuaView.h
ImplementationFile=CeXianYouHuaView.cpp
Filter=C
LastObject=ID_ERASE_AREA
BaseClass=CView
VirtualFilter=VWC


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=CMainFrame


[CLS:CChildFrame]
Type=0
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp
Filter=M
LastObject=CChildFrame


[CLS:CAboutDlg]
Type=0
HeaderFile=CeXianYouHua.cpp
ImplementationFile=CeXianYouHua.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_APP_ABOUT
CommandCount=8

[TB:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

[MNU:IDR_CEXIANTYPE]
Type=1
Class=CCeXianYouHuaView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_VIEW_TOOLBAR
Command16=ID_VIEW_STATUS_BAR
Command17=ID_MIXED_CHANNEL
Command18=ID_RED_CHANNEL
Command19=ID_GREEN_CHANNEL
Command20=ID_BLUE_CHANNEL
Command21=ID_GuassFilter
Command22=ID_CANNY
Command23=ID_AUTO_ROAD_EXTRACT
Command24=ID_AUTO_LAKE_EXTRACT
Command25=ID_ZOOM
Command26=ID_INTERACT_ROAD_EXTRACT
Command27=ID_INTERACT_LAKE_EXTRACT
Command28=ID_INTERACT_ALLLAKE_EXTRACT
Command29=ID_DRAW_POLYGON
Command30=ID_DRAW_LINE
Command31=ID_EARSER
Command32=ID_ERASE_AREA
Command33=ID_EXTEND
Command34=ID_RELOAD
Command35=ID_SCALE_SMALL
Command36=ID_SCALE_BIG
Command37=ID_WINDOW_NEW
Command38=ID_WINDOW_CASCADE
Command39=ID_WINDOW_TILE_HORZ
Command40=ID_WINDOW_ARRANGE
Command41=ID_APP_ABOUT
Command42=ID_Robert
Command43=ID_Prewitt
Command44=ID_Sobel
Command45=ID_Laplacian
Command46=ID_LOG
Command47=ID_CONTRAST
CommandCount=47

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[MNU:IDR_POPMENU]
Type=1
Class=?
Command1=ID_GETSRC
Command2=ID_INSERTLAL
Command3=ID_GETLAL
CommandCount=3

[CLS:CGeoDialog]
Type=0
HeaderFile=GeoDialog.h
ImplementationFile=GeoDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=CGeoDialog
VirtualFilter=dWC

[DLG:IDD_JWDIALOG]
Type=1
Class=CGeoDialog
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_LATITUDE,edit,1350631552
Control4=IDC_LONGITUDE,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352

[DLG:IDD_SHOWCOR]
Type=1
Class=CShowCoordinary
ControlCount=4
Control1=IDC_WIDTH,static,1342308352
Control2=IDC_HEIGHT,static,1342308352
Control3=IDC_WIDTH_IN,edit,1350631552
Control4=IDC_HEIGHT_IN,edit,1350631552

[CLS:CShowCoordinary]
Type=0
HeaderFile=ShowCoordinary.h
ImplementationFile=ShowCoordinary.cpp
BaseClass=CDialog
Filter=D
LastObject=ID_GETSRC
VirtualFilter=dWC

[DLG:IDD_DIALOG1]
Type=1
Class=CShow_Contrast
ControlCount=0

[CLS:CShow_Contrast]
Type=0
HeaderFile=Show_Contrast.h
ImplementationFile=Show_Contrast.cpp
BaseClass=CDialog
Filter=D
LastObject=CShow_Contrast
VirtualFilter=dWC

[DLG:IDD_ZOOM]
Type=1
Class=ZoomDlg
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_EDIT_X,edit,1350631552
Control7=IDC_EDIT_Y,edit,1350631552
Control8=IDC_Zoom_Adapt,button,1342242816

[CLS:ZoomDlg]
Type=0
HeaderFile=ZoomDlg.h
ImplementationFile=ZoomDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDOK
VirtualFilter=dWC

