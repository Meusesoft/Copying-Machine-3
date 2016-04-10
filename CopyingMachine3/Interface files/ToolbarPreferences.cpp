#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "ToolbarPreferences.h"

CPreferencesToolbar::CPreferencesToolbar(sGlobalInstances poGlobalInstances) : 
	CToolbar(poGlobalInstances), CSharedPreferencesSelection(poGlobalInstances) {

	//Copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &poGlobalInstances, sizeof(sGlobalInstances));

	dwStyleExtra = TBSTYLE_LIST;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	}

CPreferencesToolbar::~CPreferencesToolbar(void) {

	Destroy();
	}

//Create the copy toolbar
bool 
CPreferencesToolbar::Create(HINSTANCE phInstance, HWND phParent, int pID) {
	
	CToolbar::oTrace->StartTrace(__WFUNCTION__);

	//Create the toolbar
	CToolbar::Create(phInstance, phParent, pID);

	CToolbar::oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//Initialize the copy toolbar
void 
CPreferencesToolbar::Initialize() {

	CToolbar::oTrace->StartTrace(__WFUNCTION__);

	//Fill the stringpool
	SendMessage(hWnd, TB_ADDSTRING, NULL, (LPARAM)L"a");
	SendMessage(hWnd, TB_ADDSTRING, NULL, (LPARAM)L"b");

	//lStringIndex = 0;
	sToolbarButton CPreferencesToolbar[] = {
		{I_IMAGENONE, IDC_TOPIC1, 0, BTNS_BUTTON |  BTNS_DROPDOWN | BTNS_AUTOSIZE, 0, 0},
		{120, IDC_COLORDEPTH, TBSTATE_ENABLED, BTNS_SEP, 0, -1},
		{I_IMAGENONE, IDC_TOPIC2, 0, BTNS_BUTTON |  BTNS_DROPDOWN | BTNS_AUTOSIZE, 0, 1},
		{75, IDC_RESOLUTION, TBSTATE_ENABLED, BTNS_SEP, 0, -1},
		{I_IMAGENONE, IDC_TOPIC3, 0, BTNS_BUTTON |  BTNS_DROPDOWN | BTNS_AUTOSIZE, 0, 1},
		{60, IDC_PRINTCOPIES, TBSTATE_ENABLED, BTNS_SEP, 0, -1},
//		{I_IMAGENONE, IDC_TOPIC4, 0, BTNS_BUTTON |  BTNS_DROPDOWN | BTNS_AUTOSIZE, 0, 1},
		{15, 0, TBSTATE_ENABLED, BTNS_SEP, 0, -1},
		{200, IDC_SHOWINTERFACE, TBSTATE_ENABLED, BTNS_SEP, 0, -1},
//		{0, ID_SCAN_ACQUIRE, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_BUTTON, 0, 2}
		};

	//Add the buttons to the toolbar
	AddButtons(CPreferencesToolbar, 8);

	//Add the scan combobox
	hColorBox = CreateWindowEx(0L, L"COMBOBOX", NULL, 
		CBS_DROPDOWNLIST | WS_CHILD | WS_BORDER | WS_TABSTOP
		| WS_VISIBLE, 100, 0, 190, 150, hWnd, (HMENU)IDC_COLORDEPTH, 
		oGlobalInstances.hInstance, 0 );

	//Update the font of the combobox
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hColorBox, WM_SETFONT, (WPARAM)hFont, 0);

	//Add the print combobox
	hResolutionBox = CreateWindowEx(WS_EX_RIGHT, L"COMBOBOX", NULL, 
		CBS_DROPDOWNLIST | WS_CHILD | WS_BORDER | WS_TABSTOP
		| WS_VISIBLE, 100, 0, 190, 150, hWnd, (HMENU)IDC_RESOLUTION, 
		oGlobalInstances.hInstance, 0 );

	SendMessage(hResolutionBox, WM_SETFONT, (WPARAM)hFont, 0);

	wchar_t cCopies[20];
	long lCopies = oCore->oPrintSettings->GetInt(ePrintCopies);

	swprintf_s(cCopies, 10, L"%d", lCopies);

	hCopiesEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", cCopies, 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER | ES_RIGHT, 
		500, 0, 50, 20, hWnd, (HMENU)IDC_PRINTCOPIES, 
		oGlobalInstances.hInstance, 0 );
	SendMessage(hCopiesEdit, WM_SETFONT, (WPARAM)hFont, 0);

	hCopiesSpin = CreateWindowEx(0, UPDOWN_CLASS , L"Device Interface", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 
		500, 0, 50, 20, hWnd, (HMENU)IDC_COPIESSPIN, 
		oGlobalInstances.hInstance, 0 );

	hShowInterfaceCheckBox = CreateWindowEx(0, L"BUTTON", L"Device Interface", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_3STATE,
		500, 0, 50, 20, hWnd, (HMENU)IDC_SHOWINTERFACE, 
		oGlobalInstances.hInstance, 0 );
	SendMessage(hShowInterfaceCheckBox, WM_SETFONT, (WPARAM)hFont, 0);

	//Fill and enable the controls
	DoFillControls();
	DoEnableControls();

	//Localize the labels
	Localize(oGlobalInstances.hLanguage);

	CToolbar::oTrace->EndTrace(__WFUNCTION__);
	}

void 
CPreferencesToolbar::Localize(HINSTANCE phLanguage) {

	CToolbar::oTrace->StartTrace(__WFUNCTION__, eAll);

	wchar_t* cText;
	TBBUTTONINFO cButtonInfo;

	oGlobalInstances.hLanguage = phLanguage;

	//Initalize TBBUTTONINFO
	SecureZeroMemory(&cButtonInfo, sizeof(cButtonInfo));

	cButtonInfo.cbSize = sizeof(cButtonInfo);
	cButtonInfo.dwMask = TBIF_TEXT;

	//Alloc text buffer
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	//Update 'Color Depth'
	LoadString(phLanguage, IDS_COLORDEPTH, cText, 400);
	cButtonInfo.pszText = cText;
	SendMessage(hWnd, TB_SETBUTTONINFO, IDC_TOPIC1, (LPARAM)&cButtonInfo);

	//Update 'Resolution'
	LoadString(phLanguage, IDS_RESOLUTION, cText, 400);
	cButtonInfo.pszText = cText;
	SendMessage(hWnd, TB_SETBUTTONINFO, IDC_TOPIC2, (LPARAM)&cButtonInfo);

	//Update 'Copies'
	LoadString(phLanguage, IDS_COPIES, cText, 400);
	cButtonInfo.pszText = cText;
	SendMessage(hWnd, TB_SETBUTTONINFO, IDC_TOPIC3, (LPARAM)&cButtonInfo);

	//Update 'Device interface'
	LoadString(phLanguage, IDS_SHOWINTERFACE, cText, 400);
	SetWindowText(hShowInterfaceCheckBox, cText);

	//Free memory
	free(cText);

	//Resize and reposition
	SendMessage(hWnd, TB_AUTOSIZE, 0, 0);

	DoPositionControls();
	
	InvalidateRect(hWnd, NULL, true);
	
	CToolbar::oTrace->EndTrace(__WFUNCTION__, eAll);
	}


void 
CPreferencesToolbar::UpdateControls() {

	CToolbar::oTrace->StartTrace(__WFUNCTION__, eAll);

	DoFillControls();

	DoEnableControls();

	CToolbar::oTrace->EndTrace(__WFUNCTION__, eAll);
}

void 
CPreferencesToolbar::Destroy() {

	CToolbar::oTrace->StartTrace(__WFUNCTION__);

	CToolbar::Destroy();

	DestroyWindow(hColorBox);
	DestroyWindow(hResolutionBox);

	CToolbar::oTrace->EndTrace(__WFUNCTION__);
	}

//This function sets the position of the comboboxes exactly
//above their corresponding spacers / seperators;
void
CPreferencesToolbar::DoPositionControls() {

	RECT cPositionRect;

	CToolbar::oTrace->StartTrace(__WFUNCTION__, eAll);

	SendMessage(hWnd, TB_GETRECT, IDC_COLORDEPTH, (LPARAM)&cPositionRect);
	MoveWindow(hColorBox, cPositionRect.left, cPositionRect.top,
				cPositionRect.right - cPositionRect.left - 10,
				cPositionRect.bottom - cPositionRect.top,
				false);

	SendMessage(hWnd, TB_GETRECT, IDC_RESOLUTION, (LPARAM)&cPositionRect);
	MoveWindow(hResolutionBox, cPositionRect.left, cPositionRect.top,
				cPositionRect.right - cPositionRect.left - 10,
				cPositionRect.bottom - cPositionRect.top,
				false);

	SendMessage(hWnd, TB_GETRECT, IDC_PRINTCOPIES, (LPARAM)&cPositionRect);
	MoveWindow(hCopiesEdit, cPositionRect.left, cPositionRect.top,
				cPositionRect.right - cPositionRect.left - 10,
				cPositionRect.bottom - cPositionRect.top,
				false);
	SendMessage(hCopiesSpin, UDM_SETBUDDY, (WPARAM)hCopiesEdit, 0);

	SendMessage(hWnd, TB_GETRECT, IDC_SHOWINTERFACE, (LPARAM)&cPositionRect);
	MoveWindow(hShowInterfaceCheckBox, cPositionRect.left, cPositionRect.top,
				cPositionRect.right - cPositionRect.left - 10,
				cPositionRect.bottom - cPositionRect.top,
				false);

	CToolbar::oTrace->EndTrace(__WFUNCTION__, eAll);
	}


//This function processes all the messages, it returns true if it processed a message
bool 
CPreferencesToolbar::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {

	bool bReturn;

	bReturn = false;

	switch (message) {

		case WM_COMMAND:

			//if (phWnd==hWnd) {
				bReturn = CSharedPreferencesSelection::OnCommand(LOWORD(wParam), wParam, lParam);
			//	}
			break;
			}
		
	return bReturn;
	}

//This function fills the comboboxes for resolution and colortype
void
CPreferencesToolbar::DoEnableControls() {

	bool bEnable;

	CToolbar::oTrace->StartTrace(__WFUNCTION__, eAll);

	bEnable = oCore->oScanSettings->GetBool(eColorDepthUse) && !oCore->oScanSettings->GetBool(eAlwaysShowInterface);
	EnableWindow(hColorBox, bEnable);
	
	bEnable = oCore->oScanSettings->GetBool(eResolutionUse) && !oCore->oScanSettings->GetBool(eAlwaysShowInterface);
	EnableWindow(hResolutionBox, bEnable);
	
	CToolbar::oTrace->EndTrace(__WFUNCTION__, eAll);
}

