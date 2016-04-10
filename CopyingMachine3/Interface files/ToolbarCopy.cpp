#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "ToolbarCopy.h"

CCopyToolbar::CCopyToolbar(sGlobalInstances poGlobalInstances) : 
	CToolbar(poGlobalInstances), CSharedDeviceSelection(poGlobalInstances) {

	//Copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &poGlobalInstances, sizeof(sGlobalInstances));

	dwStyleExtra = TBSTYLE_LIST;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	}

CCopyToolbar::~CCopyToolbar(void) {

	Destroy();
	}

//Create the copy toolbar
bool 
CCopyToolbar::Create(HINSTANCE phInstance, HWND phParent, int pID) {
	
	CToolbar::oTrace->StartTrace(__WFUNCTION__);

	//Create the toolbar
	CToolbar::Create(phInstance, phParent, pID);

	CToolbar::oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//Initialize the copy toolbar
void 
CCopyToolbar::Initialize() {

	CToolbar::oTrace->StartTrace(__WFUNCTION__);

	//Fill the stringpool
	SendMessage(hWnd, TB_ADDSTRING, NULL, (LPARAM)L"a");
	SendMessage(hWnd, TB_ADDSTRING, NULL, (LPARAM)L"b");
	SendMessage(hWnd, TB_ADDSTRING, NULL, (LPARAM)L"c");

	//lStringIndex = 0;
	sToolbarButton cCopyToolbar[] = {
		//{I_IMAGENONE, IDC_TOPIC1, 0, TBSTYLE_LIST | BTNS_AUTOSIZE | BTNS_BUTTON | BTNS_SHOWTEXT, 0, (INT_PTR)lStringIndex},
		{I_IMAGENONE, IDC_TOPIC1, 0, BTNS_BUTTON |  BTNS_DROPDOWN | BTNS_AUTOSIZE, 0, 0},
		{200, IDC_ACQUIREFROM, TBSTATE_ENABLED, BTNS_SEP, 0, -1},
		{I_IMAGENONE, IDC_TOPIC2, 0, BTNS_BUTTON |  BTNS_DROPDOWN | BTNS_AUTOSIZE, 0, 1},
		{200, IDC_COPYTO, TBSTATE_ENABLED, BTNS_SEP, 0, -1},
		{0, ID_SCAN_ACQUIRE, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_BUTTON, 0, 2}
		};

	//Add the buttons to the toolbar
	AddButtons(cCopyToolbar, 5);

	//Add the scan combobox
	hScanComboBox = CreateWindowEx(0L, L"COMBOBOX", NULL, 
		CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_BORDER
		| WS_VISIBLE, 100, 0, 190, 150, hWnd, (HMENU)IDC_ACQUIREFROM, 
		oGlobalInstances.hInstance, 0 );

	DoFillScanSelector();

	//Update the font of the combobox
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hScanComboBox, WM_SETFONT, (WPARAM)hFont, 0);

	//Add the print combobox
	hPrintComboBox = CreateWindowEx(0L, L"COMBOBOX", NULL, 
		CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_BORDER
		| WS_VISIBLE, 100, 0, 190, 150, hWnd, (HMENU)IDC_COPYTO, 
		oGlobalInstances.hInstance, 0 );

	DoFillPrintSelector();

	SendMessage(hPrintComboBox, WM_SETFONT, (WPARAM)hFont, 0);

	//Localize the labels
	Localize(oGlobalInstances.hLanguage);

	CToolbar::oTrace->EndTrace(__WFUNCTION__);
	}

void 
CCopyToolbar::Localize(HINSTANCE phLanguage) {

	wchar_t* cText;
	TBBUTTONINFO cButtonInfo;

	CToolbar::oTrace->StartTrace(__WFUNCTION__, eAll);

	oGlobalInstances.hLanguage = phLanguage;

	//Initalize TBBUTTONINFO
	SecureZeroMemory(&cButtonInfo, sizeof(cButtonInfo));

	cButtonInfo.cbSize = sizeof(cButtonInfo);
	cButtonInfo.dwMask = TBIF_TEXT;

	//Alloc text buffer
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	//Update 'Acquire from'
	LoadString(phLanguage, IDS_ACQUIREFROM, cText, 400);
	cButtonInfo.pszText = cText;
	SendMessage(hWnd, TB_SETBUTTONINFO, IDC_TOPIC1, (LPARAM)&cButtonInfo);

	//Update 'Copy to'
	LoadString(phLanguage, IDS_COPYTO, cText, 400);
	cButtonInfo.pszText = cText;
	SendMessage(hWnd, TB_SETBUTTONINFO, IDC_TOPIC2, (LPARAM)&cButtonInfo);

	//Update 'Copy'
	LoadString(phLanguage, IDS_COPY, cText, 400);
	cButtonInfo.pszText = cText;
	SendMessage(hWnd, TB_SETBUTTONINFO, ID_SCAN_ACQUIRE, (LPARAM)&cButtonInfo);

	//Free memory
	free(cText);

	//Resize and reposition
	SendMessage(hWnd, TB_AUTOSIZE, 0, 0);

	DoPositionComboBoxes();
	
	InvalidateRect(hWnd, NULL, true);

	CToolbar::oTrace->EndTrace(__WFUNCTION__, eAll);
	}


void 
CCopyToolbar::Destroy() {

	CToolbar::oTrace->StartTrace(__WFUNCTION__);

	CToolbar::Destroy();

	DestroyWindow(hScanComboBox);
	DestroyWindow(hPrintComboBox);

	CToolbar::oTrace->EndTrace(__WFUNCTION__);
	}

//This function sets the position of the comboboxes exactly
//above their corresponding spacers / seperators;
void
CCopyToolbar::DoPositionComboBoxes() {

	RECT cPositionRect;

	CToolbar::oTrace->StartTrace(__WFUNCTION__, eAll);

	SendMessage(hWnd, TB_GETRECT, IDC_ACQUIREFROM, (LPARAM)&cPositionRect);
	MoveWindow(hScanComboBox, cPositionRect.left, cPositionRect.top,
				cPositionRect.right - cPositionRect.left - 10,
				cPositionRect.bottom - cPositionRect.top,
				false);

	SendMessage(hWnd, TB_GETRECT, IDC_COPYTO, (LPARAM)&cPositionRect);
	MoveWindow(hPrintComboBox, cPositionRect.left, cPositionRect.top,
				cPositionRect.right - cPositionRect.left - 10,
				cPositionRect.bottom - cPositionRect.top,
				false);
	
	CToolbar::oTrace->EndTrace(__WFUNCTION__, eAll);
	}


void 
CCopyToolbar::UpdateCurrentScanDevice() {

	CToolbar::oTrace->StartTrace(__WFUNCTION__, eAll);

	DoFillScanSelector();

	CToolbar::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

void 
CCopyToolbar::UpdateCurrentPrintDevice() {

	CToolbar::oTrace->StartTrace(__WFUNCTION__, eAll);

	DoFillPrintSelector();
	
	CToolbar::oTrace->EndTrace(__WFUNCTION__, eAll);
	}



//This function processes all the messages, it returns true if it processed a message
bool 
CCopyToolbar::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {

	bool bReturn;

	bReturn = false;

	switch (message) {

		case WM_COMMAND:

			OnCommand(message, wParam, lParam);

			break;
			}
		
	return bReturn;
	}
