#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "PaneCopy.h"
#include <math.h> 

CPaneCopy::CPaneCopy(sGlobalInstances poGlobalInstances, int piMenuCommand) : 
	CSharedPreferencesSelection(poGlobalInstances), 
	CSharedDeviceSelection(poGlobalInstances),
	CPaneBase(poGlobalInstances, piMenuCommand, L"PaneCopy") {

	cType = ePaneCopy;
	sImage = L"PNG_TOOLPANECOPY";
	iTitle = IDS_COPY;

	bEndThread = !InitializeCriticalSectionAndSpinCount(&ControlsUpdateCriticalSection, 0x80000400);
	hControlsUpdateThread = NULL;

	}

CPaneCopy::~CPaneCopy(void) {

	Destroy();
	}

//Create the copy toolbar
bool 
CPaneCopy::Create(HINSTANCE phInstance, HWND phParent) {
	
	CPaneBase::oTrace->StartTrace(__WFUNCTION__);

	CPaneBase::Create(phInstance, phParent);

	Initialize();

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//Destroy all the controls and the main pane window.
void 
CPaneCopy::Destroy() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__);

	bEndThread = true;

	//Clean up the thread
	if (hControlsUpdateThread != NULL) {

		//Wait for the thread to finish.
		WaitForSingleObject(hControlsUpdateThread, INFINITE);

		CloseHandle(hControlsUpdateThread);

		DeleteCriticalSection(&ControlsUpdateCriticalSection);
	}

	CPaneBase::oCore->oNotifications->UnsubscribeAll(hWnd);

	DestroyWindow(hScanComboBox);
	DestroyWindow(hPrintComboBox);
	DestroyWindow(hColorBox);
	DestroyWindow(hResolutionBox);
	DestroyWindow(hCopiesEdit);
	DestroyWindow(hCopiesSpin);
	DestroyWindow(hShowInterfaceCheckBox);
	DestroyWindow(hUseDocumentFeeder);
	DestroyWindow(hHeaderAcquireFrom);
	DestroyWindow(hHeaderCopyTo);
	DestroyWindow(hHeaderColor);
	DestroyWindow(hHeaderCopies);
	DestroyWindow(hHeaderResolution);

	CPaneBase::Destroy();

	CPaneBase::oTrace->EndTrace(__WFUNCTION__);
	}

//Initialize the Copy pane
void 
CPaneCopy::Initialize() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__);

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	//Add controls to the pane
	hScanControls = AddPaneStatic(IDS_SETTINGS, 400);

	//Add the scan combobox
	hHeaderAcquireFrom = CreateWindowEx(0L, L"STATIC", L"ABC", 
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		20, 10, 190, 20, hWnd, (HMENU)IDC_TOPIC1, 
		CPaneBase::oGlobalInstances.hInstance, 0);	
	SendMessage(hHeaderAcquireFrom, WM_SETFONT, (WPARAM)hFont, 0);
	
	hScanComboBox = CreateWindowEx(0L, L"COMBOBOX", NULL, 
		CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_BORDER
		| WS_VISIBLE, 20, 30, 190, 20, hWnd, (HMENU)IDC_ACQUIREFROM, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hScanComboBox, WM_SETFONT, (WPARAM)hFont, 0);

	DoFillScanSelector();

	//Add the print combobox
	hHeaderCopyTo = CreateWindowEx(0L, L"STATIC", L"ABC", 
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		20, 60, 190, 20, hWnd, (HMENU)IDC_TOPIC2, 
		CPaneBase::oGlobalInstances.hInstance, 0);	
	SendMessage(hHeaderCopyTo, WM_SETFONT, (WPARAM)hFont, 0);

	hPrintComboBox = CreateWindowEx(0L, L"COMBOBOX", NULL, 
		CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_BORDER
		| WS_VISIBLE, 20, 80, 190, 150, hWnd, (HMENU)IDC_COPYTO, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hPrintComboBox, WM_SETFONT, (WPARAM)hFont, 0);

	DoFillPrintSelector();

	//Add the color combobox
	hHeaderColor = CreateWindowEx(0L, L"STATIC", L"ABC", 
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		20, 110, 190, 20, hWnd, (HMENU)IDC_TOPIC3, 
		CPaneBase::oGlobalInstances.hInstance, 0);	
	SendMessage(hHeaderColor, WM_SETFONT, (WPARAM)hFont, 0);

	hColorBox = CreateWindowEx(0L, L"COMBOBOX", NULL, 
		CBS_DROPDOWNLIST | WS_CHILD | WS_BORDER | WS_TABSTOP
		| WS_VISIBLE, 20, 130, 190, 150, hWnd, (HMENU)IDC_COLORDEPTH, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hColorBox, WM_SETFONT, (WPARAM)hFont, 0);

	//Add the resolution combobox
	hHeaderResolution = CreateWindowEx(0L, L"STATIC", L"ABC", 
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		20, 160, 190, 20, hWnd, (HMENU)IDC_TOPIC4, 
		CPaneBase::oGlobalInstances.hInstance, 0);	
	SendMessage(hHeaderResolution, WM_SETFONT, (WPARAM)hFont, 0);

	hResolutionBox = CreateWindowEx(WS_EX_RIGHT, L"COMBOBOX", NULL, 
		CBS_DROPDOWNLIST | WS_CHILD | WS_BORDER | WS_TABSTOP
		| WS_VISIBLE, 20, 180, 190, 150, hWnd, (HMENU)IDC_RESOLUTION, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hResolutionBox, WM_SETFONT, (WPARAM)hFont, 0);

	//Add the Copies edit
	hHeaderCopies = CreateWindowEx(0L, L"STATIC", L"ABC", 
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		20, 210, 190, 20, hWnd, (HMENU)IDC_TOPIC5, 
		CPaneBase::oGlobalInstances.hInstance, 0);	
	SendMessage(hHeaderCopies, WM_SETFONT, (WPARAM)hFont, 0);

	wchar_t cCopies[20];
	long lCopies = CPaneBase::oCore->oPrintSettings->GetInt(ePrintCopies);

	swprintf_s(cCopies, 10, L"%d", lCopies);

	hCopiesEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", cCopies, 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER | ES_RIGHT, 
		20, 230, 50, 20, hWnd, (HMENU)IDC_PRINTCOPIES, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hCopiesEdit, WM_SETFONT, (WPARAM)hFont, 0);

	hCopiesSpin = CreateWindowEx(0, UPDOWN_CLASS , L"", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 
		70, 230, 12, 20, hWnd, (HMENU)IDC_COPIESSPIN, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hCopiesSpin, UDM_SETBUDDY, (WPARAM)hCopiesEdit, 0);

	//Add the contrast slider
	hHeaderContrast = CreateWindowEx(0L, L"STATIC", L"C", 
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		20, 260, 190, 20, hWnd, (HMENU)IDC_TOPIC4, 
		CPaneBase::oGlobalInstances.hInstance, 0);	
	SendMessage(hHeaderContrast, WM_SETFONT, (WPARAM)hFont, 0);

	hSliderContrast = CreateWindowEx(0, TRACKBAR_CLASS , L"", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | TBS_NOTICKS | TBS_BOTH, 
		70, 230, 12, 20, hWnd, (HMENU)IDC_SLIDERCONTRAST, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hSliderContrast, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(-100, 100));
	SendMessage(hSliderContrast, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0);
	SendMessage(hSliderContrast, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)20);

	//Add the brightness slider
	hHeaderBrightness = CreateWindowEx(0L, L"STATIC", L"B", 
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		20, 260, 190, 20, hWnd, (HMENU)IDC_TOPIC4, 
		CPaneBase::oGlobalInstances.hInstance, 0);	
	SendMessage(hHeaderBrightness, WM_SETFONT, (WPARAM)hFont, 0);

	hSliderBrightness = CreateWindowEx(0, TRACKBAR_CLASS , L"", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | TBS_NOTICKS | TBS_BOTH, 
		70, 230, 12, 20, hWnd, (HMENU)IDC_SLIDERBRIGHTNESS, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hSliderBrightness, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(-100, 100));
	SendMessage(hSliderBrightness, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0);
	SendMessage(hSliderBrightness, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)20);

	//Add the document feeder checkbox
	hUseDocumentFeeder = CreateWindowEx(0L, L"BUTTON", L"ADF", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_3STATE /*| BS_OWNERDRAW*/,
		20, 290, 190, 20, hWnd, (HMENU)IDC_USEFEEDER, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hUseDocumentFeeder, WM_SETFONT, (WPARAM)hFont, 0);

	//Add the interface checkbox
	hShowInterfaceCheckBox = CreateWindowEx(0L, L"BUTTON", L"Device Interface", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_3STATE /*| BS_OWNERDRAW*/,
		20, 290, 190, 20, hWnd, (HMENU)IDC_SHOWINTERFACE, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	SendMessage(hShowInterfaceCheckBox, WM_SETFONT, (WPARAM)hFont, 0);

	//Fill and enable the controls
	//DoFillControls();
	//DoEnableControls();
	//FillAndEnableControls();

	//Localize the labels
	Localize(CPaneBase::oGlobalInstances.hLanguage);

	//Position
	DoPositionControls();

	//Add notifications
	CPaneBase::oCore->oNotifications->Subscribe(hWnd, eNotificationScanSettingsChanged);
	CPaneBase::oCore->oNotifications->Subscribe(hWnd, eNotificationPrintSettingsChanged);
	CPaneBase::oCore->oNotifications->Subscribe(hWnd, eNotificationScanDeviceChanged);
	CPaneBase::oCore->oNotifications->Subscribe(hWnd, eNotificationPrintDeviceChanged);

	CPaneBase::oTrace->EndTrace(__WFUNCTION__);
	}

void
CPaneCopy::FillAndEnableControls()
{
	if (!bEndThread) {

		hControlsUpdateThread = CreateThread(
			NULL,				// default security attributes
			0,					// use default stack size  
			&ThreadFillAndEnable,        // thread function 
			this,				// argument to thread function 
			0,					// use default creation flags 
			NULL);				// returns the thread identifier 
	}
}

//This is the main function of the fill and enable thread.
DWORD
WINAPI ThreadFillAndEnable(LPVOID lpParameter) {

	CPaneCopy *oPaneCopy;

	oPaneCopy = (CPaneCopy*)lpParameter;

	oPaneCopy->bEndThread = true;

	EnterCriticalSection(&oPaneCopy->ControlsUpdateCriticalSection);

	oPaneCopy->UpdateControls();

	LeaveCriticalSection(&oPaneCopy->ControlsUpdateCriticalSection);

	oPaneCopy->bEndThread = false;

	return 0;
}

void 
CPaneCopy::Localize(HINSTANCE phLanguage) {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	wchar_t* cText;

	CPaneBase::oGlobalInstances.hLanguage = phLanguage;

	CPaneBase::LocalizePaneButtons(phLanguage);

	//Alloc text buffer
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	//Update title
	LoadString(phLanguage, IDS_COPY, cText, 400);
	sTitle = cText;
	
	//Update 'Color Depth'
	LoadString(phLanguage, IDS_ACQUIREFROM, cText, 400);
	SetWindowText(hHeaderAcquireFrom, cText);

	//Update 'Color Depth'
	LoadString(phLanguage, IDS_COPYTO, cText, 400);
	SetWindowText(hHeaderCopyTo, cText);

	//Update 'Color Depth'
	LoadString(phLanguage, IDS_COLORDEPTH, cText, 400);
	SetWindowText(hHeaderColor, cText);

	//Update 'Resolution'
	LoadString(phLanguage, IDS_RESOLUTION, cText, 400);
	SetWindowText(hHeaderResolution, cText);

	//Update 'Copies'
	LoadString(phLanguage, IDS_COPIES, cText, 400);
	SetWindowText(hHeaderCopies, cText);

	//Update 'Device interface'
	LoadString(phLanguage, IDS_SHOWINTERFACE, cText, 400);
	SetWindowText(hShowInterfaceCheckBox, cText);

	//Update 'Use Feeder'
	LoadString(phLanguage, IDS_USEFEEDER, cText, 400);
	SetWindowText(hUseDocumentFeeder, cText);

	//Update 'Contract'
	LoadString(phLanguage, IDS_CONTRAST, cText, 400);
	SetWindowText(hHeaderContrast, cText);

	//Update 'Brightness'
	LoadString(phLanguage, IDS_BRIGHTNESS, cText, 400);
	SetWindowText(hHeaderBrightness, cText);

	//Free memory
	free(cText);
	
	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function sets a pointer to the image containing the background for the
//pane window.
void 
CPaneCopy::SetBackgroundImagePointer(CBitmap* poBitmap) {

	oDoubleBufferBitmap = poBitmap;
	}

//This function handles the events from the core.
bool 
CPaneCopy::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;

	bReturn = false;

	while (CPaneBase::oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationScanSettingsChanged:
			case eNotificationPrintSettingsChanged:

				//Update the controls to synchronize it with the settings
				UpdateControls();
				break;

			case eNotificationScanDeviceChanged:

				//Update the current selected scanner in the copy toolbar
				DoFillScanSelector();
				DoFillControls();
				DoEnableControls();
				break;

			case eNotificationPrintDeviceChanged:

				//Update the current selected printer in the copy toolbar
				DoFillPrintSelector();
				DoFillControls();
				DoEnableControls();
				break;
			}
		}

	return bReturn;
	}


void 
CPaneCopy::UpdateControls() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	DoFillControls();

	DoEnableControls();

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function positions the controls in the pane window
void
CPaneCopy::DoPositionControls() {

	RECT cPositionRect;
	RECT cClientRect;
	int iMaxWidth;  
	int iFontHeight;

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll); 

	GetClientRect(hWnd, &cClientRect);

	cButtonsControlRect.X = 10;
	cButtonsControlRect.Y = 10;
	cButtonsControlRect.Width = cClientRect.right - (cButtonsControlRect.X * 2);


	RectF cChildRect;
	cChildRect.X = 0;
	cChildRect.Y = 0;
	cChildRect.Width = 200;
	cChildRect.Height = 200;


	//MoveWindow(oChildPane->hWnd, 0,0,100,100,true);
	PositionPaneButtons(cButtonsControlRect);

	//oPaneButtons[0].iExtraSpace = cClientRect.bottom - 300;

	cSettingsControlRect.X = 10;
	cSettingsControlRect.Y  = 10;
	cSettingsControlRect.Width = cClientRect.right - (cSettingsControlRect.X * 2);

	cPositionRect.left = 20;
	cPositionRect.top = 50;
	iMaxWidth = (int)(cSettingsControlRect.Width - (cSettingsControlRect.X * 2));
	iFontHeight = 20;

	//Acquire from
	MoveWindow(hHeaderAcquireFrom, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);
	cPositionRect.top += iFontHeight;
	MoveWindow(hScanComboBox, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);

	//Copy to
	cPositionRect.top += iFontHeight + 10;
	MoveWindow(hHeaderCopyTo, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);
	cPositionRect.top += iFontHeight;
	MoveWindow(hPrintComboBox, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);

	//Resolution
	cPositionRect.top += iFontHeight + 10;
	MoveWindow(hHeaderResolution, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);
	cPositionRect.top += iFontHeight;
	MoveWindow(hResolutionBox, cPositionRect.left, cPositionRect.top, 60, iFontHeight, true);

	//Color
	cPositionRect.top += iFontHeight + 10;
	MoveWindow(hHeaderColor, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);
	cPositionRect.top += iFontHeight;
	MoveWindow(hColorBox, cPositionRect.left, cPositionRect.top, 110, iFontHeight, true);

	//Copies
	cPositionRect.top += iFontHeight + 10;
	MoveWindow(hHeaderCopies, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);
	cPositionRect.top += iFontHeight;
	MoveWindow(hCopiesEdit, cPositionRect.left, cPositionRect.top, 40, iFontHeight, true);
	MoveWindow(hCopiesSpin, cPositionRect.left + 40, cPositionRect.top, 15, iFontHeight, true);

	//Brightness
	cPositionRect.top += iFontHeight + 10;
	MoveWindow(hHeaderBrightness, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);
	cPositionRect.top += iFontHeight;
	MoveWindow(hSliderBrightness, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);

	//Contrast
	cPositionRect.top += iFontHeight + 10;
	MoveWindow(hHeaderContrast, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);
	cPositionRect.top += iFontHeight;
	MoveWindow(hSliderContrast, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);

	//Docment Feeder
	cPositionRect.top += iFontHeight + 10;
	MoveWindow(hUseDocumentFeeder, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);

	//Device checkbox
	cPositionRect.top += iFontHeight + 10;
	MoveWindow(hShowInterfaceCheckBox, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);

	cSettingsControlRect.Height = (Gdiplus::REAL)(cPositionRect.top + iFontHeight);
	cSettingsControlRect.Height = 100;

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll); 
	}


//This function processes all the messages, it returns true if it processed a message
bool 
CPaneCopy::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {

	bool bReturn;

	bReturn = false;

	switch (message) {

		case WM_ERASEBKGND:

			if (phWnd == this->hWnd) {
				bReturn = true;
				piReturn = 1; //we erased the background... NOT!
				}
			break;

		case WM_CTLCOLORBTN:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:

			if (((HWND)lParam == hShowInterfaceCheckBox) ||
				((HWND)lParam == hSliderContrast) ||
				((HWND)lParam == hSliderBrightness) ||
				((HWND)lParam == hUseDocumentFeeder)) {

				HBRUSH hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);

				piReturn = (int)hbr;
				bReturn = true;
				}


			break;

		case WM_SIZE:
			
			if (phWnd == this->hWnd) {
				
				//Reposition the controls;
				DoPositionControls();
				
				//Rebuild the background double buffer
				//DoDrawBackground();

				//Make sure the complete background is repainted, and all
				//the controls too (fix to make them appear transparent)
				/*InvalidateRect(hWnd,				NULL, true);
				InvalidateRect(hHeaderAcquireFrom,	NULL, true);
				InvalidateRect(hHeaderColor,		NULL, true);
				InvalidateRect(hHeaderCopies,		NULL, true);
				InvalidateRect(hHeaderCopyTo,		NULL, true);
				InvalidateRect(hHeaderResolution,	NULL, true);
				InvalidateRect(hShowInterfaceCheckBox,	NULL, true);*/

				//We processed this message
				bReturn = true;
				piReturn = 0;
				}
			break;


		case WM_PAINT:

			if (phWnd == this->hWnd) {

				DoPaint();
				}
			break;

		case WM_CORENOTIFICATION:

			if (phWnd == hWnd) {

				bReturn = OnEventCoreNotification();
				}
			bReturn=false;
			break;

		case WM_COMMAND:

			if (phWnd==hWnd) {
				bReturn = CSharedPreferencesSelection::OnCommand(LOWORD(wParam), wParam, lParam);
				if (!bReturn) {
					bReturn = CSharedDeviceSelection::OnCommand(LOWORD(wParam), wParam, lParam);
					}
				}
			break;

		case WM_VSCROLL:
		case WM_HSCROLL:

			if (lParam == (LPARAM)hSliderContrast) 
				bReturn = CSharedPreferencesSelection::OnCommand(IDC_SLIDERCONTRAST, wParam, lParam);
			if (lParam == (LPARAM)hSliderBrightness) 
				bReturn = CSharedPreferencesSelection::OnCommand(IDC_SLIDERBRIGHTNESS, wParam, lParam);

			break;


		case WM_DRAWITEM:

			if (phWnd == this->hWnd) {
				//Draw the static controls with ownerdraw style
				LPDRAWITEMSTRUCT oDrawItemInfo;

				oDrawItemInfo = (LPDRAWITEMSTRUCT)lParam;

				DoDrawStaticControls(oDrawItemInfo);

				bReturn = true;
				}
			break;

		default:

			if (!bReturn) bReturn = CPaneBase::WndProc(phWnd, message, wParam, lParam, piReturn);
			break;

		//default:

		//	if (oButtonAcquire && !bReturn) bReturn = oButtonAcquire->WndProc(hWnd, message, wParam, lParam, piReturn);
			//if (oButtonPrint && !bReturn)	bReturn = oButtonPrint->WndProc(hWnd, message, wParam, lParam, piReturn);
			//if (oButtonCopy && !bReturn)	bReturn = oButtonCopy->WndProc(hWnd, message, wParam, lParam, piReturn);

			/*if (!bReturn && message==WM_NOTIFY) {

				LPNMCUSTOMDRAW cNMButton;

				cNMButton = (LPNMCUSTOMDRAW)lParam;

				if (cNMButton->hdr.hwndFrom == hShowInterfaceCheckBox) {

					switch (cNMButton->dwDrawStage) {
					
						 case CDDS_PREERASE: 
						 case CDDS_ITEMPREERASE: 
							//FillRect(cNMButton->hdc,&cNMButton->rc,(HBRUSH)GetStockObject(BLACK_BRUSH)); 
							DoDrawCheckboxBackground(hShowInterfaceCheckBox, cNMButton->hdc);
						piReturn = CDRF_NOTIFYPOSTERASE;
						bReturn = true;
						}
					}
				}*/
			}
		
	return bReturn;
	}

void
CPaneCopy::DoEnableControls() {

	bool bEnable;
	CScanDevice* oScanDevice;

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	oScanDevice = CPaneBase::oCore->oScanSettings->GetScanner();

	if (oScanDevice!=NULL) {

		bEnable = CPaneBase::oCore->oScanSettings->GetBool(eColorDepthUse) && !CPaneBase::oCore->oScanSettings->GetBool(eAlwaysShowInterface)  && oScanDevice->bColorDepth;
		EnableWindow(hColorBox, bEnable);
		
		bEnable = CPaneBase::oCore->oScanSettings->GetBool(eResolutionUse) && !CPaneBase::oCore->oScanSettings->GetBool(eAlwaysShowInterface)  && oScanDevice->bResolution;
		EnableWindow(hResolutionBox, bEnable);

		bEnable = oScanDevice->bFeederPresent;
		EnableWindow(hUseDocumentFeeder, bEnable);
		}
	
	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function is called when the scrollbar has been moved to reposition
//all the controls on the window
void 
CPaneCopy::DoRepositionControls() {

	DoPositionControls();
}


