// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define NTDDI_VERSION NTDDI_WINXP  
#define _WIN32_WINNT _WIN32_WINNT_WINXP  


// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0600		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0600 // Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0600 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _USE_MATH_DEFINES

#define _CRT_SECURE_NO_WARNINGS

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
//#define _AFXDLL

//#include <afxwin.h> 
#include <windows.h>
#include <commctrl.h>
//#include <afxres.h> 

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <initguid.h>
#include <gdiplus.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <io.h>
#include <time.h>
#include <AtlBase.h> 
#include <AtlConv.h>
#include <wia.h>
#include <sti.h>
#include <fstream>
#include <sstream>
#include <math.h>
#ifdef _WIN64
#import "msxml6.dll"
#else
//#import "msxml6.dll"
#endif

//using namespace MSXML2;
using namespace Gdiplus;
using namespace std;

// additional headers for Copying Machine 3
#include "resource.h"
#include "language.h"
#include "language."
#include "Registry.h"
#include "Trace.h"
#include "MenuIcon.h"

struct sGlobalInstances {

	CTracer*	oTrace;
	CRegistry*  oRegistry;
	CMenuIcon*  oMenuIcon;
	void*		oCopyingMachineCore;
	void*		oNotifications;
	HINSTANCE	hInstance;
	HMODULE		hLanguage;
	HIMAGELIST	hSmallFileIcons;		//The imagelist for the small icons for filetypes
	HIMAGELIST	hNormalFileIcons;		//The imagelist for the normal icons for filetypes
};

//global definitions
#define MAX_LOADSTRING 100

#define WM_OPENMENU WM_APP + 0
#define WM_CORENOTIFICATION WM_APP+10
#define WM_SCANIMAGESRECEIVED WM_APP+4
#define WM_ENABLEAPPLY WM_APP+3
#define IDC_MENUPANE 8000

#define ThumbnailSizeX 96
#define ThumbnailSizeY 96

#define MsgBox MessageBox(NULL, L"Ok", L"Ok", MB_OK)

#define defBackgroundR 132
#define defBackgroundG 190
#define defBackgroundB 234
#define bUseCustomControlView true
#define iSplitterWidth 4

#define defUseCustomColors		true
#define defColorBorderMainFrame RGB(224, 224, 224)
#define defColorSplitter		RGB(224, 224, 224)

//global enumerators
enum eActionResult {
	eOk,
	eGeneralOutOfMemory,
	eFileResultExternalFile,
	eFileResultNotFound,
	eFileResultNotSupported,
	eFileResultInvalidParameter,
	eFileResultUnknownError,
	eFileResultAccessDenied,
	eFileResultCouldNotCreate,
	ePrintError,
	ePrintErrorCheckMessages,
	eCancelled,
	eImageActionError,
	eOcrError
};