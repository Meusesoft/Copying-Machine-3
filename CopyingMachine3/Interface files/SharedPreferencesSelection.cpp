#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "SharedPreferencesSelection.h"

CSharedPreferencesSelection::CSharedPreferencesSelection(sGlobalInstances poGlobalInstances) {

	//Copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &poGlobalInstances, sizeof(sGlobalInstances));

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oTrace = oGlobalInstances.oTrace;

	hColorBox				= NULL;
	hResolutionBox			= NULL;
	hShowInterfaceCheckBox	= NULL;
	hUseDocumentFeeder		= NULL;
	hCopiesEdit				= NULL;
	hCopiesSpin				= NULL;
	hSliderBrightness		= NULL;
	hSliderContrast			= NULL;
	}

CSharedPreferencesSelection::~CSharedPreferencesSelection(void) {

	}

//Apply all changes to the controls to the settings
void 
CSharedPreferencesSelection::DoApplySettings() {

	DoApplyScanSettings();
	DoApplyPrintSettings();
	}

//This function applies the changes to the settings
void 
CSharedPreferencesSelection::DoApplyScanSettings() {

	int iSelIndex;
	int iNumber;
	wchar_t cNumber[10];

	//ColorDepth
	iSelIndex = (int)SendMessage(hColorBox, CB_GETCURSEL, 0, 0);
	int iColorDepthValue = (int)SendMessage(hColorBox, CB_GETITEMDATA, (WPARAM)iSelIndex, 0);
	if (iColorDepthValue!=CB_ERR) oCore->oScanSettings->SetInt(eColorDepthValue, iColorDepthValue);

	//Resolution
	iSelIndex = (int)SendMessage(hResolutionBox, CB_GETCURSEL, 0, 0);
	SendMessage(hResolutionBox, CB_GETLBTEXT, (WPARAM)iSelIndex, (LPARAM)cNumber);
	iNumber = _wtoi(cNumber);
	oCore->oScanSettings->SetInt(eResolutionValue, iNumber); 

	//Always show interface
	if (SendMessage(hShowInterfaceCheckBox, BM_GETCHECK, 0, 0)!=BST_INDETERMINATE) {

		oCore->oScanSettings->SetBool(eAlwaysShowInterface, (SendMessage(hShowInterfaceCheckBox, BM_GETCHECK, 0, 0)==BST_CHECKED));
		}

	//Use Feeder
	if (SendMessage(hUseDocumentFeeder, BM_GETCHECK, 0, 0)!=BST_INDETERMINATE) {

		oCore->oScanSettings->SetBool(eUseFeeder, (SendMessage(hUseDocumentFeeder, BM_GETCHECK, 0, 0)==BST_CHECKED));
		}

	//Contrast
	if (hSliderContrast != NULL) {
		int iContrastValue = (int)SendMessage(hSliderContrast, TBM_GETPOS, 0, 0);
		oCore->oScanSettings->SetInt(eContrastValue, iContrastValue);
		}

	//Brightness
	if (hSliderBrightness != NULL) {
		int iBrightnessValue = (int)SendMessage(hSliderBrightness, TBM_GETPOS, 0, 0);
		oCore->oScanSettings->SetInt(eBrightnessValue, iBrightnessValue);
		}
	}

//This function applies the changes to the settings
void 
CSharedPreferencesSelection::DoApplyPrintSettings() {

	//Copies
	oCore->oPrintSettings->SetInt(ePrintCopies, GetIntegerFromEditControl(hCopiesEdit));

	//Always show interface
	if (SendMessage(hShowInterfaceCheckBox, BM_GETCHECK, 0, 0)!=BST_INDETERMINATE) {

		oCore->oPrintSettings->SetBool(ePrintAlwaysShowInterface,	(SendMessage(hShowInterfaceCheckBox, BM_GETCHECK, 0, 0)==BST_CHECKED));
		}
}


//Fill all the shared preferences controls
void
CSharedPreferencesSelection::DoFillControls() {

	//Get the latest capabilities from the devices
	oCore->oScanCore->GetDeviceCapabilities();

	//Fill Resolution and Color Depth
	DoFillResolution();
	DoFillColorDepth();
	DoFillShowInterfaceCheckbox();
	DoFillUseDocumentFeederCheckbox();

	//Set the number of copies
	wchar_t cCopies[20];
	long lCopies = oCore->oPrintSettings->GetInt(ePrintCopies);

	swprintf_s(cCopies, 10, L"%d", lCopies);
	SendMessage(hCopiesEdit, WM_SETTEXT, 0, (LPARAM)cCopies);
	SendMessage(hCopiesEdit, EM_SETLIMITTEXT, (WPARAM)2, 0);
	SendMessage(hCopiesSpin, UDM_SETRANGE32, (WPARAM)1, (LPARAM)99);

	//Set the contrastvalue
	if (hSliderContrast != NULL) {
		int iContrastValue = oCore->oScanSettings->GetInt(eContrastValue);
		SendMessage(hSliderContrast, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iContrastValue);
		}
	
	//Set the brightnessvalue
	if (hSliderBrightness != NULL) {
		int iBrightnessValue = oCore->oScanSettings->GetInt(eBrightnessValue);
		SendMessage(hSliderBrightness, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iBrightnessValue);
		}
}

void 
CSharedPreferencesSelection::DoFillUseDocumentFeederCheckbox() {

	SendMessage(hUseDocumentFeeder, BM_SETCHECK, oCore->oScanSettings->GetBool(eUseFeeder) ? BST_CHECKED : BST_UNCHECKED, 0);
	}

void 
CSharedPreferencesSelection::DoFillShowInterfaceCheckbox() {

	//Set the interface checkbox
	bool bPrintInterface, bScanInterface;

	bScanInterface  = oCore->oScanSettings->GetBool(eAlwaysShowInterface);
	bPrintInterface = oCore->oPrintSettings->GetBool(ePrintAlwaysShowInterface);
	
	if (bPrintInterface == bScanInterface) {

		SendMessage(hShowInterfaceCheckBox, BM_SETCHECK, bScanInterface ? BST_CHECKED : BST_UNCHECKED, 0);
		}
	else {

		SendMessage(hShowInterfaceCheckBox, BM_SETCHECK, BST_INDETERMINATE, 0);
		}
	}

void
CSharedPreferencesSelection::DoInitialiseResolution() {

	wchar_t cText[100];
	CScanDevice* oScanDevice;

	//Clear resolution box
	SendMessage(hResolutionBox, CB_RESETCONTENT, 0, 0);

	//Fill dpi combobox
	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"75");
	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"100");
	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"150");
	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"200");
	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"300");
	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"400");
	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"600");
	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"1200");

	//set the resolution. If an error occurs the first resolution will be selected
	int iResolution = oCore->oScanSettings->GetInt(eResolutionValue);
	swprintf_s(cText, 100, L"%d", iResolution);
	if (SendMessage(hResolutionBox, CB_SELECTSTRING, 0, (LPARAM)cText) == CB_ERR) {
		SendMessage(hResolutionBox, CB_SETCURSEL, (WPARAM)0, 0);
	}
}
void 
CSharedPreferencesSelection::DoFillResolution() {

	wchar_t cText[100];
	CScanDevice* oScanDevice;

	//Clear resolution box
	SendMessage(hResolutionBox, CB_RESETCONTENT, 0, 0);

	//Fill dpi combobox
	oScanDevice = oCore->oScanSettings->GetScanner();
	if (oScanDevice!=NULL) {
		if (oScanDevice->bResolution75)		SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"75");
		if (oScanDevice->bResolution100)	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"100");
		if (oScanDevice->bResolution150)	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"150");
		if (oScanDevice->bResolution200)	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"200");
		if (oScanDevice->bResolution300)	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"300");
		if (oScanDevice->bResolution400)	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"400");
		if (oScanDevice->bResolution600)	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"600");
		if (oScanDevice->bResolution1200)	SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"1200");
		}

	//set the resolution. If an error occurs the first resolution will be selected
	int iResolution = oCore->oScanSettings->GetInt(eResolutionValue);
	swprintf_s(cText, 100, L"%d", iResolution);
	if (SendMessage(hResolutionBox, CB_SELECTSTRING, 0, (LPARAM)cText)==CB_ERR) {
		SendMessage(hResolutionBox, CB_SETCURSEL, (WPARAM)0, 0);
		}
}

void
CSharedPreferencesSelection::DoInitialiseColorDepth() {

	wchar_t cText[100];

	//Clear color and resolution box
	SendMessage(hColorBox, CB_RESETCONTENT, 0, 0);

	//Fill type box
	int iColorDepth = oCore->oScanSettings->GetInt(eColorDepthValue);
	int iSelIndex = 0;
	int iItemIndex = 0;

	//Fill color type box
	LoadString(oGlobalInstances.hLanguage, IDS_BLACKWHITE, cText, 100);
	iItemIndex = (int)SendMessage(hColorBox, CB_ADDSTRING, 0, (LPARAM)cText);
	if (iItemIndex != CB_ERR) {
		SendMessage(hColorBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)0); //BW
		if (iColorDepth == 0) iSelIndex = iItemIndex;
	}

	LoadString(oGlobalInstances.hLanguage, IDS_GRAY, cText, 100);
	iItemIndex = (int)SendMessage(hColorBox, CB_ADDSTRING, 0, (LPARAM)cText);
	if (iItemIndex != CB_ERR) {
		SendMessage(hColorBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)1); //Gray
		if (iColorDepth == 1) iSelIndex = iItemIndex;
	}

	LoadString(oGlobalInstances.hLanguage, IDS_COLOR, cText, 100);
	iItemIndex = (int)SendMessage(hColorBox, CB_ADDSTRING, 0, (LPARAM)cText);
	if (iItemIndex != CB_ERR) {
		SendMessage(hColorBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)2); //Color
		if (iColorDepth == 2) iSelIndex = iItemIndex;
	}

	if (SendMessage(hColorBox, CB_SETCURSEL, (WPARAM)iSelIndex, 0) == CB_ERR) {
		SendMessage(hColorBox, CB_SETCURSEL, (WPARAM)0, 0);
	}
}

void
CSharedPreferencesSelection::DoFillColorDepth() {

	wchar_t cText[100];
	CScanDevice* oScanDevice;

	//Clear color and resolution box
	SendMessage(hColorBox, CB_RESETCONTENT, 0, 0);

	//Fill type box
	int iColorDepth = oCore->oScanSettings->GetInt(eColorDepthValue);
	int iSelIndex = 0;
	int iItemIndex = 0;

	oScanDevice = oCore->oScanSettings->GetScanner();

	if (oScanDevice!=NULL) {

		//Fill color type box
		LoadString(oGlobalInstances.hLanguage, IDS_BLACKWHITE, cText, 100);
		if (oScanDevice->bBlackWhite) {
			iItemIndex=(int)SendMessage(hColorBox, CB_ADDSTRING, 0, (LPARAM)cText);
			if (iItemIndex!=CB_ERR) {
				SendMessage(hColorBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)0); //BW
				if (iColorDepth == 0) iSelIndex = iItemIndex;
				}
			}
			
		LoadString(oGlobalInstances.hLanguage, IDS_GRAY, cText, 100);
		if (oScanDevice->bGray) {
			iItemIndex=(int)SendMessage(hColorBox, CB_ADDSTRING, 0, (LPARAM)cText);
			if (iItemIndex!=CB_ERR) {
				SendMessage(hColorBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)1); //Gray
				if (iColorDepth == 1) iSelIndex = iItemIndex;
				}
			}

		LoadString(oGlobalInstances.hLanguage, IDS_COLOR, cText, 100);
		if (oScanDevice->bColor) {
			iItemIndex=(int)SendMessage(hColorBox, CB_ADDSTRING, 0, (LPARAM)cText);
			if (iItemIndex!=CB_ERR) {
				SendMessage(hColorBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)2); //Color
				if (iColorDepth == 2) iSelIndex = iItemIndex;
				}
			}

		if (SendMessage(hColorBox, CB_SETCURSEL, (WPARAM)iSelIndex, 0)==CB_ERR) {
			SendMessage(hColorBox, CB_SETCURSEL, (WPARAM)0, 0);
			}
		}
	}

//This function reads out an edit control and converts its value to
//an integer
int  
CSharedPreferencesSelection::GetIntegerFromEditControl(HWND phEditControl) {

	int iResult;
	wchar_t cNumber[10];

	SendMessage(phEditControl, WM_GETTEXT, (WPARAM)9, (LPARAM)cNumber);
	iResult = _wtoi(cNumber);

	return iResult;
	}

//This function enables/disables the controls
void
CSharedPreferencesSelection::DoEnableControls() {

	bool bEnable;
	CScanDevice* oScanDevice;

	oScanDevice = oCore->oScanSettings->GetScanner();

	if (oScanDevice!=NULL) {

		bEnable = bControlsEnabled && oCore->oScanSettings->GetBool(eColorDepthUse) && !oCore->oScanSettings->GetBool(eAlwaysShowInterface) && oScanDevice->bColorDepth;
		EnableWindow(hColorBox, bEnable);
		
		bEnable = bControlsEnabled && oCore->oScanSettings->GetBool(eResolutionUse) && !oCore->oScanSettings->GetBool(eAlwaysShowInterface) && oScanDevice->bResolution;
		EnableWindow(hResolutionBox, bEnable);

		bEnable = bControlsEnabled && oScanDevice->bFeederPresent;
		EnableWindow(hUseDocumentFeeder, bEnable);
		}

	EnableWindow(hCopiesEdit, bControlsEnabled);
	EnableWindow(hCopiesSpin, bControlsEnabled);
	EnableWindow(hShowInterfaceCheckBox, bControlsEnabled);
}

//This function handles the WM_COMMAND message
bool 
CSharedPreferencesSelection::OnCommand(int piCommand, WPARAM wParam, LPARAM lParam) {

	bool bReturn;

	bReturn = true;

	switch (piCommand) {

		case IDC_COLORDEPTH:
		case IDC_RESOLUTION:

			if (HIWORD(wParam)==CBN_SELCHANGE) {
				DoApplyScanSettings();
				}
			break;

		case IDC_SLIDERCONTRAST:
		case IDC_SLIDERBRIGHTNESS:

			DoApplyScanSettings();
			break;

		case IDC_SHOWINTERFACE:

			if (SendMessage(hShowInterfaceCheckBox, BM_GETCHECK, 0, 0)==BST_CHECKED) {
				SendMessage(hShowInterfaceCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
				}
			else {
				SendMessage(hShowInterfaceCheckBox, BM_SETCHECK, BST_CHECKED, 0);
				}

			DoApplyScanSettings();
			DoApplyPrintSettings();
			break;

		case IDC_USEFEEDER:

			if (SendMessage(hUseDocumentFeeder, BM_GETCHECK, 0, 0)==BST_CHECKED) {
				SendMessage(hUseDocumentFeeder, BM_SETCHECK, BST_UNCHECKED, 0);
				}
			else {
				SendMessage(hUseDocumentFeeder, BM_SETCHECK, BST_CHECKED, 0);
				}

			DoApplyScanSettings();
			break;

		case IDC_PRINTCOPIES:
		
			if (HIWORD(wParam) == EN_CHANGE) {
				if ((oCore->oPrintSettings->GetInt(ePrintCopies) !=
					GetIntegerFromEditControl(hCopiesEdit)) &&
					hCopiesEdit!=NULL) {

					DoApplyPrintSettings();
					}	
				}
			break;

		default:

			bReturn = false;
			break;
		}

	return bReturn;
	}




