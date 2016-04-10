#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "CopyToolbar.h"

CCopyToolbar::CCopyToolbar(sGlobalInstances poGlobalInstances) : 
	CToolbar(poGlobalInstances.oTrace) {

	//Copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &poGlobalInstances, sizeof(sGlobalInstances));

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	}

CCopyToolbar::~CCopyToolbar(void) {

	Destroy();
	}

//Create the copy toolbar
bool 
CCopyToolbar::Create(HINSTANCE phInstance, HWND phParent, int pID) {
	
	oTrace->StartTrace(__WFUNCTION__);

	//Create the toolbar
	CToolbar::Create(phInstance, phParent, pID);

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//Initialize the copy toolbar
void 
CCopyToolbar::Initialize() {

	oTrace->StartTrace(__WFUNCTION__);

	long lStringIndex;

	//Add the title of the toolbar
	lStringIndex = (long)SendMessage(hWnd, TB_ADDSTRING, NULL, (LPARAM)L"Copy");
	//lStringIndex = 0;
	TBBUTTON cCopyToolbar[] = {
		//{I_IMAGENONE, IDC_TOPIC1, 0, TBSTYLE_LIST | BTNS_AUTOSIZE | BTNS_BUTTON | BTNS_SHOWTEXT, 0, (INT_PTR)lStringIndex},
		{I_IMAGENONE, IDC_TOPIC1, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, 0, 0, 0, lStringIndex},
		{200, IDC_ACQUIREFROM, TBSTATE_ENABLED, BTNS_SEP, 0, 0, 0, -1},
		{200, IDC_COPYTO, TBSTATE_ENABLED, BTNS_SEP, 0, 0, 0, -1},
		{0, ID_SCAN_ACQUIRE, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_BUTTON, 0, 0, 0, -1}
		};

	//Add the buttons to the toolbar
	AddButtons(cCopyToolbar, 4);

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

	SendMessage(hWnd, TB_AUTOSIZE, 0, 0);

	oTrace->EndTrace(__WFUNCTION__);
	}

void 
CCopyToolbar::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	CToolbar::Destroy();

	DestroyWindow(hScanComboBox);
	DestroyWindow(hPrintComboBox);

	oTrace->EndTrace(__WFUNCTION__);
	}

void 
CCopyToolbar::UpdateCurrentScanDevice() {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	DoFillScanSelector();

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

void 
CCopyToolbar::UpdateCurrentPrintDevice() {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	DoFillPrintSelector();
	
	oTrace->EndTrace(__WFUNCTION__, eAll);
	}


//This function fills the combobox for selecting the scanning device
void 
CCopyToolbar::DoFillScanSelector() {

	//Add the scanners to the listview
	WCHAR cItemText[MAX_PATH];
	sScanDevice oDevice;
	sScanDevice sDefaultScanner;

	oCore->oScanSettings->GetScanner(sDefaultScanner);
	oScanningDevices.clear();

	//get the scanners
	if (oCore->oScanCore->GetFirstDevice(oDevice)) {

		do {

			oScanningDevices.push_back(oDevice);
			} while (oCore->oScanCore->GetNextDevice(oDevice));
		}

	//Clear scanner box
	do {
	} while (SendMessage(hScanComboBox, CB_DELETESTRING, 0, 0)!=CB_ERR);

	//Fill the controls
	for (long lIndex=0; lIndex<(long)oScanningDevices.size(); lIndex++) {

		wsprintf(cItemText, L"%s", oScanningDevices[lIndex].cScanningDevice.c_str());
		
		SendMessage(hScanComboBox, CB_ADDSTRING, 0, (LPARAM)cItemText);
		}

	//Select the default scanner
	wsprintf(cItemText, L"%s", sDefaultScanner.cScanningDevice.c_str());

	if (SendMessage(hScanComboBox, CB_SELECTSTRING, -1, (LPARAM)cItemText)==CB_ERR) {
		SendMessage(hScanComboBox, CB_SETCURSEL, 0, 0);
		}
	}

//This function fills the combobox for selecting the print device
void 
CCopyToolbar::DoFillPrintSelector() {

	//Add the scanners to the listview
	WCHAR cItemText[MAX_PATH];
	sOutputDevice oOutputDevice;
	int iItemIndex;

	//Clear the print device box
	do {
	} while (SendMessage(hPrintComboBox, CB_DELETESTRING, 0, 0)!=CB_ERR);

	//Add the printers, email and file output
	for (long lIndex=0; lIndex<oCore->oPrintCore->GetOutputDeviceCount(); lIndex++) {

		if (oCore->oPrintCore->GetOutputDevice(lIndex, oOutputDevice)) {

			wsprintf(cItemText, L"%s", oOutputDevice.sDescription.c_str());

			iItemIndex = (int)SendMessage(hPrintComboBox, CB_ADDSTRING, 0, (LPARAM)cItemText);
			if (iItemIndex!=CB_ERR) {
				SendMessage(hPrintComboBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)lIndex);
				}
			}
		}

	//Select the current output device
	oCore->oPrintSettings->GetDevice(oOutputDevice);
	SelectOutputDevice(oOutputDevice);
	}

void 
CCopyToolbar::SelectOutputDevice(sOutputDevice poOutputDevice) {

	int iCount;
	int iData;
	bool bFound;
	sOutputDevice oOutputDevice;

	oTrace->StartTrace(__WFUNCTION__);

	//Get the handle to the combobox
	iCount = (int)SendMessage(hPrintComboBox, CB_GETCOUNT, 0, 0);
	bFound = false;

	for (int iIndex=0; iIndex<iCount && !bFound; iIndex++) {

		iData = (int)SendMessage(hPrintComboBox, CB_GETITEMDATA, iIndex, 0);
		
		if (oCore->oPrintCore->GetOutputDevice(iData, oOutputDevice)) {

			if ((oOutputDevice.cType == poOutputDevice.cType && oOutputDevice.cType != eOutputPrinter) ||
				(oOutputDevice.sDescription == poOutputDevice.sDescription && oOutputDevice.cType == eOutputPrinter)) {

				bFound = (SendMessage(hPrintComboBox, CB_SETCURSEL, iIndex, 0) != CB_ERR);
				}
			}
		}

	if (!bFound) SendMessage(hPrintComboBox, CB_SETCURSEL, 0, 0);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function applies the changes to the settings in the comboboxes
void
CCopyToolbar::DoApplySettings() {

	//change the default scanner
	int iSelectedItem;
	sOutputDevice oOutputDevice;

	iSelectedItem = (int)SendMessage(hScanComboBox, CB_GETCURSEL, 0, 0);

	if (iSelectedItem!=CB_ERR) {

		oCore->oScanSettings->SetScanner(oScanningDevices[iSelectedItem]);
		oCore->oScanCore->SetCurrentScanner(oScanningDevices[iSelectedItem]);
		}

	//change the default output device
	iSelectedItem = 0;

	//Get the selected item 
	iSelectedItem = (int)SendMessage(hPrintComboBox, CB_GETCURSEL, 0, 0);
	if (iSelectedItem!=CB_ERR) {

		iSelectedItem = (int)SendMessage(hPrintComboBox, CB_GETITEMDATA, (WPARAM)iSelectedItem, 0);
		}

	if (iSelectedItem<0 || iSelectedItem>=oCore->oPrintCore->GetOutputDeviceCount()) iSelectedItem = -1; //the iResult isn't a valid outputdevice

	//Update the print settings
	if (iSelectedItem!=-1) {

		if (oCore->oPrintCore->GetOutputDevice(iSelectedItem, oOutputDevice)) {

			oCore->oPrintSettings->SetDevice(oOutputDevice);
			}
		}
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

			switch (LOWORD(wParam)) {
				
				case IDC_ACQUIREFROM:

					if (HIWORD(wParam)==CBN_SELCHANGE) {
						DoApplySettings();
						}
					break;
				}
			
			break;
			}
		
	return bReturn;
	}
