#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "SharedDeviceSelection.h"

CSharedDeviceSelection::CSharedDeviceSelection(sGlobalInstances poGlobalInstances) {

	//Copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &poGlobalInstances, sizeof(sGlobalInstances));

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oTrace = oGlobalInstances.oTrace;
	}

CSharedDeviceSelection::~CSharedDeviceSelection(void) {

	}

//This function fills the combobox for selecting the scanning device
void 
CSharedDeviceSelection::DoFillScanSelector() {

	//Add the scanners to the listview
	WCHAR cItemText[MAX_PATH];
	CScanDevice* oDevice;
	CScanDevice* oDefaultScanner;

	oDefaultScanner = oCore->oScanSettings->GetScanner();

	if (oDefaultScanner!=NULL) {

		oScanningDevices.clear();

		//get the scanners
		oDevice = oCore->oScanCore->GetFirstDevice();

		while (oDevice!=NULL) {

			oScanningDevices.push_back(oDevice);
			oDevice = oCore->oScanCore->GetNextDevice();
			}

		//Clear scanner box
		do {
		} while (SendMessage(hScanComboBox, CB_DELETESTRING, 0, 0)!=CB_ERR);

		//Fill the controls
		for (long lIndex=0; lIndex<(long)oScanningDevices.size(); lIndex++) {

			wsprintf(cItemText, L"%s", oScanningDevices[lIndex]->cScanningDevice.c_str());
			
			SendMessage(hScanComboBox, CB_ADDSTRING, 0, (LPARAM)cItemText);
			}

		//Select the default scanner
		wsprintf(cItemText, L"%s", oDefaultScanner->cScanningDevice.c_str());

		if (SendMessage(hScanComboBox, CB_SELECTSTRING, -1, (LPARAM)cItemText)==CB_ERR) {
			SendMessage(hScanComboBox, CB_SETCURSEL, 0, 0);
			}
		}
	}

//This function fills the combobox for selecting the print device
void 
CSharedDeviceSelection::DoFillPrintSelector() {

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
CSharedDeviceSelection::SelectOutputDevice(sOutputDevice poOutputDevice) {

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
CSharedDeviceSelection::DoApplySettings() {

	DoApplySettings(0);
}

//This function applies the changes to the settings in the comboboxes
void
CSharedDeviceSelection::DoApplySettings(int piCommand) {

	int iSelectedItem;
	sOutputDevice oOutputDevice;

	if (piCommand != IDC_COPYTO)
	{
		//change the default scanner
		iSelectedItem = (int)SendMessage(hScanComboBox, CB_GETCURSEL, 0, 0);

		if (iSelectedItem!=CB_ERR) {

			oCore->oScanSettings->SetScanner(oScanningDevices[iSelectedItem]);
			oCore->oScanCore->SetCurrentScanner(oScanningDevices[iSelectedItem]);
			}
	}

	if (piCommand != IDC_ACQUIREFROM)
	{
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
	}
//This function handles the WM_COMMAND message
bool 
CSharedDeviceSelection::OnCommand(int piCommand, WPARAM wParam, LPARAM lParam) {

	bool bReturn;

	bReturn = true;

	switch (piCommand) {

		case IDC_ACQUIREFROM:
		case IDC_COPYTO:

			if (HIWORD(wParam)==CBN_SELCHANGE) {
				DoApplySettings(piCommand);
				}
			break;

		default:

			bReturn = false;
			break;
		}

	return bReturn;
	}