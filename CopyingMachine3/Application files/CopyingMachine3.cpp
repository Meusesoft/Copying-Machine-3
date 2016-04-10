// CopyingMachine3.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CopyingMachineCore.h"
#include "CopyingMachine3.h"
#include "RebarControl.h"
#include "Toolbar.h"
#include "ToolbarCopy.h"
#include "ToolbarPreferences.h"
#include "ToolbarMenu.h"
#include "DocumentList.h"
#include "DocumentCollectionList.h"
#include "PageThumbnails.h"
#include "Splitter.h"
#include "Listview.h"
#include "CopyFileListview.h"
#include "PageViewer.h"
#include "MenuIcon.h"
#include "Pane.h"
#include "WindowContainer.h"
#include "CopyFileListviewContainer.h"
#include "MainFrame.h"
#include "EasyFrame.h"
#include "XMLDocument.h"

// Global Variables:
HINSTANCE				hInst;					//Current instance of the Windows app
CMainFrame*				oMainFrame;				//An instance of the main frame class
CEasyFrame*				oEasyFrame;				//An instance of the easy frame class
CTracer*				oTrace;					//The instance of the trace class
CRegistry*				oRegistry;				//The instance of the registry class
CCopyingMachineCore*	oCore;					//The instance of the applications core classes
sGlobalInstances		cGlobalInstances;		//Structure with all globalinstances
HBRUSH					hBrushMainBackground;	//The brush for the main frame its background 


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int, bool pbDialog);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		DialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
int					DoStillImage(CScanDevice* poScanDevice);
bool				DoStillImageRegister(std::wstring psCommandLine);


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	MSG msg;
	HACCEL hAccelTable;
	sExecutableInformation cExecutableInformation;

	oEasyFrame = NULL;
	oMainFrame = NULL;
   
    //Initialize COM
	CoInitialize(NULL);

	//Create instances of core and helper classed
	oRegistry = new CRegistry();

	//Check if we are registering with stillimage, if so do not continue executing Copying Machine.
	if (DoStillImageRegister(lpCmdLine)) {

		CoUninitialize();
		delete oRegistry;
		return TRUE;
		}

	// Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


	//create instances
	oTrace = new CTracer(oRegistry);
	oTrace->InitTracing(hInstance);

	cGlobalInstances.oRegistry = oRegistry;
	cGlobalInstances.oTrace = oTrace;
	cGlobalInstances.oCopyingMachineCore = NULL;
	cGlobalInstances.hInstance = hInstance;
	cGlobalInstances.hLanguage = NULL;
	cGlobalInstances.oMenuIcon = NULL;

	oCore = new CCopyingMachineCore(cGlobalInstances);	
 
	// Get the file type icons, we do this only once or else the call will fail
	SHFILEINFO  sfi;
	cGlobalInstances.hSmallFileIcons = (HIMAGELIST)SHGetFileInfo(TEXT("C:\\"), 0, &sfi,
		sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	cGlobalInstances.hNormalFileIcons = (HIMAGELIST)SHGetFileInfo(TEXT("C:\\"), 0, &sfi,
		sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);


	//do a hash check
	oCore->GetCopyingMachineVersion(cExecutableInformation);


	if (false && cExecutableInformation.sHashValue != oCore->oRegistration->sHash) {

		MessageBox(NULL, L"The checksum check of this executable failed. This means that the file is corrupted. Please download the latest version from www.meusesoft.com.", L"Copying Machine", MB_OK | MB_ICONERROR);
		msg.wParam = 0;
		}
	else {

		cGlobalInstances.oCopyingMachineCore = (void*)oCore;
		cGlobalInstances.hLanguage = oCore->oGlobalInstances.hLanguage;


		// Initialize global strings
		MyRegisterClass(hInstance);



		// InitCommonControls() is required on Windows XP if an application
		// manifest specifies use of ComCtl32.dll version 6 or later to enable
		// visual styles.  Otherwise, any window creation will fail.
		InitCommonControls();

		// Perform application initialization:
		int iEasyFrame;;

		iEasyFrame = oRegistry->ReadInt(L"Window", L"EasyFrame", 0);

		if (!InitInstance (hInstance, nCmdShow, (iEasyFrame==1))) {
			delete oTrace;
			delete oRegistry;
			return FALSE;
			}

		hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COPYINGMACHINE3));

		//Add the commandline options to the mainframe
		int iStillImage;
		CScanDevice* oDevice;

		oDevice = new CScanDevice(cGlobalInstances);
		iStillImage = DoStillImage(oDevice);
		delete oDevice;
		
		if (iStillImage!=0) {

			switch (iStillImage) {

				case 1: //scan

					oCore->CommandlineOptionAdd(L"/scan");
					break;

				case 2: //scan and print

					oCore->CommandlineOptionAdd(L"/copy");
					break;

				case 3: //scan and email

					oCore->CommandlineOptionAdd(L"/email");
					break;
				}
			}
		else {
		
			if (wcslen(lpCmdLine)>0) {

				oCore->CommandlineInput(lpCmdLine);
				}
			}

		if (oMainFrame!=NULL) oCore->CommandlineOptionsProcess(oMainFrame->hWnd, false);
		if (oEasyFrame!=NULL) oCore->CommandlineOptionsProcess(oEasyFrame->hWnd, true);

		//let everybody know we are starting now
		oCore->oNotifications->CommunicateEvent(eNotificationApplicationInitialized, NULL);
		oCore->oNotifications->CommunicateEvent(eNotificationWindowsInitialized, NULL);

		// Main message loop:
		BOOL bRet;

		do {

			while ((bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
			{
				if (bRet == -1) {

					// an error occurred, what to do?

				}
				else {

					if (!oCore->ProcessMessage(msg)) {
			
						if (oEasyFrame!=NULL) {

							if (!IsDialogMessage (oEasyFrame->hWnd, &msg)) {

								TranslateMessage(&msg);
								DispatchMessage(&msg);
								}
							}

						if (oMainFrame!=NULL) {

							if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
							{
								TranslateMessage(&msg);
								DispatchMessage(&msg);
							}
						}
					}
				}
			}

			//clear up
			if (oEasyFrame!=NULL) delete oEasyFrame;
			if (oMainFrame!=NULL) delete oMainFrame;
			DeleteObject(hBrushMainBackground);

			if (msg.message == WM_QUIT && msg.wParam == 1) {
				InitInstance(hInstance, nCmdShow, false);
				oCore->oNotifications->CommunicateEvent(eNotificationApplicationInitialized, NULL);
				oCore->oNotifications->CommunicateEvent(eNotificationWindowsInitialized, NULL);
				}

			if (msg.message == WM_QUIT && msg.wParam == 2) InitInstance(hInstance, nCmdShow, true);

		} while (msg.message == WM_QUIT && msg.wParam != 0);

	}



	delete oCore;
	delete oTrace;
	delete oRegistry;

	GdiplusShutdown(gdiplusToken);

	CoUninitialize();

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

	//Create the background brush
	hBrushMainBackground = defUseCustomColors ? 
		CreateSolidBrush(defColorBorderMainFrame) :
		CreateSolidBrush(COLOR_BTNFACE);
	
	//Load the name for the main class
	LoadString(hInstance, IDC_COPYINGMACHINE3, szWindowClass, MAX_LOADSTRING);

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_DBLCLKS;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE3));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	//wcex.hbrBackground	= NULL;
	wcex.hbrBackground	= (HBRUSH)hBrushMainBackground;
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_COPYINGMACHINE3);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE_16));

	RegisterClassEx(&wcex);

	//Vertical splitter
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_DBLCLKS ;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE3));
	wcex.hCursor		= LoadCursor(NULL, IDC_SIZEWE);
	wcex.hbrBackground	= (HBRUSH)COLOR_BTNSHADOW;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= L"MeusesoftSplitterV";
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);

	//Horizontal splitter
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_DBLCLKS ;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE3));
	wcex.hCursor		= LoadCursor(NULL, IDC_SIZENS);
	wcex.hbrBackground	= (HBRUSH)COLOR_BTNSHADOW;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= L"MeusesoftSplitterH";
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_DBLCLKS;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE3));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= L"MeusesoftPane";
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_DBLCLKS;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE3));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= L"MeusesoftPaneScroller";
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_DBLCLKS;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE3));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= L"MeusesoftContainer";
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_DBLCLKS;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE3));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)COLOR_APPWORKSPACE;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= L"MeusesoftPageViewer";
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, bool pbEasyFrame)
{
	
	HWND hWnd;
	hInst = hInstance; // Store instance handle in our global variable

	if (pbEasyFrame) {

		oRegistry->WriteInt(L"Window", L"EasyFrame", 1);

		oEasyFrame = new CEasyFrame(cGlobalInstances);

		hWnd = CreateDialog (hInst, 
							MAKEINTRESOURCE (IDD_EASYFRAMEMAIN), 
							0, 
							(DLGPROC)DialogProc);

	    if (!hWnd) {
			return FALSE;
			}
		oEasyFrame->Create(hInst, hWnd);
		oMainFrame = NULL;
		}
	else {

		oRegistry->WriteInt(L"Window", L"EasyFrame", 0);

		oMainFrame = new CMainFrame(cGlobalInstances);
		hWnd = oMainFrame->Create(hInstance, nCmdShow);

	   if (!hWnd) {
			return FALSE;
			}

		ShowWindow(hWnd, oMainFrame->nCmdShow);
		UpdateWindow(hWnd);
		oEasyFrame = NULL;
		}

	oCore->hMainApplicationWindow = hWnd;
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent, iReturn;

	iReturn = 0;

	if (oMainFrame->WndProc(hWnd, message, wParam, lParam, iReturn)) return iReturn;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			PostQuitMessage(0);
			break;
		case ID_FILE_SAVE:

			break;
		
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return iReturn;
}

BOOL 
CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent, iReturn;

	BOOL bReturn;

	bReturn = FALSE;

	if (oEasyFrame->WndProc(hWnd, message, wParam, lParam, iReturn)) {
		
		SetWindowLong(hWnd, DWLP_MSGRESULT, iReturn);
		return TRUE;
		}

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			PostQuitMessage(0);
			break;
		
		default:
			return FALSE;//DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	default:
		return FALSE;//DefWindowProc(hWnd, message, wParam, lParam);
	}
	return bReturn;
}

//Function for registring to StillImage and detecting the action StillImage
//return value:	    0 = no action
//					1 = scan
//					2 = scan en print
//					3 = unknown

int 
DoStillImage(CScanDevice* poScanDevice) {

   int iReturn = 0;

   oTrace->StartTrace(L"DoStillImage");

  // if (oRegistry->ReadInt(L"General", L"StillImage", 0)==1) {

      HRESULT hRes;
      IStillImage * g_StillImage;

      //eerste versie van windows bepalen, still image is beperkt beschikbaar
      OSVERSIONINFO lpVersionInformation;

      lpVersionInformation.dwOSVersionInfoSize = sizeof(lpVersionInformation);

      if (GetVersionEx(&lpVersionInformation)!=0) {

         //StillImage is alleen beschikbaar vanaf win98 en win2000, dus geldt niet voor
         //win95 en winNT3 en winNT4.

         if (lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32s ||
            (lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && lpVersionInformation.dwMajorVersion<=4 && lpVersionInformation.dwMinorVersion<10) ||
            (lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT	&& lpVersionInformation.dwMajorVersion<5)) {
               //still image is not supported in these platform versions
               //Win32s, Win95, WinNT3 en WinNT4

               oTrace->Add(L"Error", L"No Still Image available");
               }
         else {
            //registering and interfacing with Still Image.

            hRes = StiCreateInstance(GetModuleHandle(NULL), STI_VERSION, &g_StillImage,NULL);

            if (SUCCEEDED(hRes)) {

				//hRes = g_StillImage->RegisterLaunchApplication(L"Copying Machine", wAppPath);

    //           if (!SUCCEEDED(hRes)) {
    //              //function seems to fail in user mode in NT.
    //              //::MessageBox(NULL, "Error registering launch Application", "Error", MB_OK);
    //              oTrace->Add(L"Error", L"Registering launch Application");
    //              }

               // Was this a STI launch?
               // Call STI-API and get the device name, event and event code
               poScanDevice->cScanningDevice = L"";
               poScanDevice->cScanningDeviceID = L"";

               char EventName[MAX_PATH];
               WCHAR   szDeviceName[65];
               WCHAR   szEventName[65];
               DWORD   dwEventCode;
               hRes = g_StillImage->GetSTILaunchInformation(szDeviceName,
                                                      &dwEventCode,
                                                      szEventName);
               oTrace->Add(L"Status", L"GetSTILaunchInformation");

               if (SUCCEEDED(hRes))
                  {
                   DWORD val = 250, type;
				  char    ActualDeviceName [256];
                   g_StillImage->GetDeviceValue(szDeviceName, STI_DEVICE_VALUE_TWAIN_NAME, &type,
                   (unsigned char*)&ActualDeviceName[0], &val);

					poScanDevice->cScanningDeviceID = szDeviceName;

                   WideCharToMultiByte(CP_ACP, NULL, szEventName, -1, EventName, MAX_PATH, "", 0);

                   oTrace->Add(L"oActualDevice.cScanningDeviceID", poScanDevice->cScanningDeviceID);
                   oTrace->Add(L"EventName", szEventName);

                   //scannen
                   if (_strcmpi(EventName, "{B6C5A715-8C6E-11d2-977A-0000F87A926F}")==0) {

                        iReturn = 1;
                     }
                   if (_strcmpi(EventName, "{B441F425-8C6E-11d2-977A-0000F87A926F}")==0) {

                        iReturn = 2;
                     	}

				   //Read from a preference from the registry, see if this event was already set to an action
				   if (iReturn == 0) {

						iReturn = oRegistry->ReadInt(L"StillImage", szEventName, 0);
					   }

				   if (iReturn == 0) {

					   iReturn = oCore->oDialogs->DlgUnknownEvent(NULL, szEventName);
						if (iReturn == -1) iReturn = 0;
					   }
                  }

               g_StillImage->Release();
               }
            }
         }
    //  }

	oTrace->EndTrace(L"DoStillImage, result: ", iReturn);

	return iReturn;
	}

bool 
DoStillImageRegister(std::wstring psCommandLine) {

	bool bResult;
	bool bTrace;
	bool bRegister;
	
	//initialise variables
	bResult = false;
	bTrace = (bool)(oRegistry->ReadInt(L"Debug", L"DoTrace", 0)!=0);

	
	if ((psCommandLine.find(L"/register")==std::wstring::npos) &&
		(psCommandLine.find(L"/unregister")==std::wstring::npos)) {

		return false;
		}

	bRegister = false;
	if (psCommandLine.find(L"/register")!=std::wstring::npos) bRegister = true;

	//Register with Still Image
	wchar_t wAppPath[MAX_PATH];

	HRESULT hRes;
	IStillImage * g_StillImage;

	//eerste versie van windows bepalen, still image is beperkt beschikbaar
	OSVERSIONINFO lpVersionInformation;

	lpVersionInformation.dwOSVersionInfoSize = sizeof(lpVersionInformation);

	if (GetVersionEx(&lpVersionInformation)!=0) {

	 //StillImage is only available since win98 en win2000, so exclude win95 en winNT3 en winNT4.

	 if (lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32s ||
		(lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && lpVersionInformation.dwMajorVersion<=4 && lpVersionInformation.dwMinorVersion<10) ||
		(lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT	&& lpVersionInformation.dwMajorVersion<5)) {
		   //still image is not supported on these platform versions

			bResult = false;
		   }
	 else {
		//(Un)registering and interfacing with StillImage.
		//Has to run as administrator in Windows Vista and beyond (User Access Control);

		GetModuleFileName(NULL, wAppPath, MAX_PATH);

		hRes = StiCreateInstance(GetModuleHandle(NULL), STI_VERSION, &g_StillImage,NULL);

		if (SUCCEEDED(hRes)) {

			if (bRegister) {
				hRes = g_StillImage->RegisterLaunchApplication(L"Copying Machine", wAppPath);
				}
			else {
				hRes = g_StillImage->UnregisterLaunchApplication(L"Copying Machine");
				}

		   if (!SUCCEEDED(hRes)) {

			  bResult = false;
			  if (bTrace) MessageBox(NULL, bRegister ? L"Register: Error" : L"Unregister: Error", L"Copying Machine StillImage", MB_OK); 
			  }
		   else {

				oRegistry->WriteIntLocalMachine(L"General", L"StillImage", bRegister ? 1 : 0);
				if (bTrace) MessageBox(NULL, bRegister ? L"Register: Success" : L"Unregister: Success", L"Copying Machine StillImage", MB_OK); 
			   }
			}
		 }
	  }

	//Return the result;
	return true;
	}



