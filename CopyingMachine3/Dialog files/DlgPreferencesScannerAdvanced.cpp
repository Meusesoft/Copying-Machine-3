#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPreferencesScannerAdvanced::CDlgPreferencesScannerAdvanced(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
	CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {
}

CDlgPreferencesScannerAdvanced::~CDlgPreferencesScannerAdvanced(void)
{
	if (hDlg!=NULL) DestroyWindow(hDlg);
}

void 
CDlgPreferencesScannerAdvanced::OnInitDialog() {

	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	SendDlgItemMessage(hDlg, IDC_SHOWERRORS, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eShowErrorMessage) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_USEFEEDER, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eUseFeeder) ? BST_CHECKED : BST_UNCHECKED, 0);

	SendDlgItemMessage(hDlg, IDC_USERESOLUTION, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eResolutionUse) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_TESTRESOLUTION, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eResolutionNegotiate) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_USEBRIGHTNESS, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eBrightnessUse) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_TESTBRIGHTNESS, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eBrightnessNegotiate) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_USECONTRAST, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eContrastUse) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_TESTCONTRAST, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eContrastNegotiate) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_USECOLORDEPTH, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eColorDepthUse) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_TESTCOLORDEPTH, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eColorDepthNegotiate) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_USEORIENTATION, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eOrientationUse) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_TESTORIENTATION, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eOrientationNegotiate) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_USEPAGESIZE, BM_SETCHECK, oCmCore->oScanSettings->GetBool(ePageUse) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_TESTPAGESIZE, BM_SETCHECK, oCmCore->oScanSettings->GetBool(ePageNegotiate) ? BST_CHECKED : BST_UNCHECKED, 0);

	//Set the invalid resolution data
	wchar_t cAssumedDPI[10];

	long lAssumedDPI = oCmCore->oScanSettings->GetInt(eAssumeDPI);
	swprintf_s(cAssumedDPI, 10, L"%d", lAssumedDPI);
	
	SendDlgItemMessage(hDlg, IDC_DPI, CB_ADDSTRING, 0, (LPARAM)L"75");
	SendDlgItemMessage(hDlg, IDC_DPI, CB_ADDSTRING, 0, (LPARAM)L"100");
	SendDlgItemMessage(hDlg, IDC_DPI, CB_ADDSTRING, 0, (LPARAM)L"150");
	SendDlgItemMessage(hDlg, IDC_DPI, CB_ADDSTRING, 0, (LPARAM)L"200");
	SendDlgItemMessage(hDlg, IDC_DPI, CB_ADDSTRING, 0, (LPARAM)L"300");
	SendDlgItemMessage(hDlg, IDC_DPI, CB_ADDSTRING, 0, (LPARAM)L"400");
	SendDlgItemMessage(hDlg, IDC_DPI, CB_ADDSTRING, 0, (LPARAM)L"600");
	SendDlgItemMessage(hDlg, IDC_DPI, CB_ADDSTRING, 0, (LPARAM)L"1200");
	SendDlgItemMessage(hDlg, IDC_DPI, CB_SELECTSTRING, 0, (LPARAM)cAssumedDPI);

	SendDlgItemMessage(hDlg, IDC_RADIOASSUME, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eAssumeDPI) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_RADIOWARNING, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eAssumeDPI) ? BST_UNCHECKED : BST_CHECKED, 0);


	//Set the after acquisition checkboxes
	SendDlgItemMessage(hDlg, IDC_SCANNEXTPAGE, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eAskScanNextPage) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_INVERTIMAGE, BM_SETCHECK, oCmCore->oScanSettings->GetBool(eInvertBlackWhite) ? BST_CHECKED : BST_UNCHECKED, 0);

	DoEnableControls();
}

BOOL 
CDlgPreferencesScannerAdvanced::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_CAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}

			break;
		};	


	return bReturn;
	}

BOOL 
CDlgPreferencesScannerAdvanced::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {


	switch (iCommand) {

		case IDC_RADIOASSUME: 
		case IDC_RADIOWARNING: 
			DoEnableControls();

		case IDC_SHOWERRORS: 
		case IDC_USEFEEDER: 
		case IDC_USERESOLUTION: 
		case IDC_TESTRESOLUTION: 
		case IDC_USEBRIGHTNESS: 
		case IDC_TESTBRIGHTNESS: 
		case IDC_USECONTRAST: 
		case IDC_TESTCONTRAST: 
		case IDC_USECOLORDEPTH: 
		case IDC_TESTCOLORDEPTH: 
		case IDC_USEORIENTATION: 
		case IDC_TESTORIENTATION: 
		case IDC_USEPAGESIZE: 
		case IDC_TESTPAGESIZE: 
		case IDC_INVERTIMAGE: 
		case IDC_SCANNEXTPAGE: 
			SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
			break;

		case IDC_DPI: 
			if (HIWORD(wParam)==CBN_SELCHANGE) {
				SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
				}
			break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesScannerAdvanced::OnCloseOk() {


	}

//This function handles the localize action
void 
CDlgPreferencesScannerAdvanced::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	LocalizeControl(IDC_TOPIC1, IDS_MISCELLANEOUS);
	LocalizeControl(IDC_TOPIC2, IDS_TESTCAPABILITY);
	LocalizeControl(IDC_TOPIC3, IDS_USECAPABILITY);
	LocalizeControl(IDC_SHOWERRORS, IDS_SHOWERRORS);
	LocalizeControl(IDC_USEFEEDER, IDS_USEFEEDER);
	LocalizeControl(IDC_TESTRESOLUTION, IDS_RESOLUTION);
	LocalizeControl(IDC_TESTBRIGHTNESS, IDS_BRIGHTNESS);
	LocalizeControl(IDC_TESTCONTRAST, IDS_CONTRAST);
	LocalizeControl(IDC_TESTORIENTATION, IDS_ORIENTATION);
	LocalizeControl(IDC_TESTCOLORDEPTH, IDS_COLORDEPTH);
	LocalizeControl(IDC_TESTPAGESIZE, IDS_PAGESIZE);
	LocalizeControl(IDC_USERESOLUTION, IDS_RESOLUTION);
	LocalizeControl(IDC_USEBRIGHTNESS, IDS_BRIGHTNESS);
	LocalizeControl(IDC_USECONTRAST, IDS_CONTRAST);
	LocalizeControl(IDC_USEORIENTATION, IDS_ORIENTATION);
	LocalizeControl(IDC_USECOLORDEPTH, IDS_COLORDEPTH);
	LocalizeControl(IDC_USEPAGESIZE, IDS_PAGESIZE);

	LocalizeControl(IDC_TOPIC4, IDS_INVALIDRESOLUTIONDETECTED);
	LocalizeControl(IDC_TOPIC5, IDS_AFTERACQUISITION);

	LocalizeControl(IDC_RADIOASSUME, IDS_ASSUME);
	LocalizeControl(IDC_RADIOWARNING, IDS_GIVEWARNING);
	LocalizeControl(IDC_SCANNEXTPAGE, IDS_ASKTOSCANNEXTPAGE);
	LocalizeControl(IDC_INVERTIMAGE, IDS_INVERTBWIMAGE);
}

//This function handles the Apply event
void 
CDlgPreferencesScannerAdvanced::OnApply() {

	CCopyingMachineCore* oCmCore;
	wchar_t cNumber[10];
	int iNumber;
	int iSelection;

	oCmCore = (CCopyingMachineCore*)oCore;

	oCmCore->oScanSettings->SetBool(eShowErrorMessage,		(SendDlgItemMessage(hDlg, IDC_SHOWERRORS,		BM_GETCHECK, 0, 0)==BST_CHECKED));
	oCmCore->oScanSettings->SetBool(eUseFeeder,				(SendDlgItemMessage(hDlg, IDC_USEFEEDER,		BM_GETCHECK, 0, 0)==BST_CHECKED));
	
	oCmCore->oScanSettings->SetBool(eResolutionUse,			(SendDlgItemMessage(hDlg, IDC_USERESOLUTION,	BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eResolutionNegotiate,	(SendDlgItemMessage(hDlg, IDC_TESTRESOLUTION,	BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eBrightnessUse,			(SendDlgItemMessage(hDlg, IDC_USEBRIGHTNESS,	BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eBrightnessNegotiate,	(SendDlgItemMessage(hDlg, IDC_TESTBRIGHTNESS,	BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eContrastUse,			(SendDlgItemMessage(hDlg, IDC_USECONTRAST,		BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eContrastNegotiate,		(SendDlgItemMessage(hDlg, IDC_TESTCONTRAST,		BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eColorDepthUse,			(SendDlgItemMessage(hDlg, IDC_USECOLORDEPTH,	BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eColorDepthNegotiate,	(SendDlgItemMessage(hDlg, IDC_TESTCOLORDEPTH,	BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eOrientationUse,		(SendDlgItemMessage(hDlg, IDC_USEORIENTATION,	BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eOrientationNegotiate,	(SendDlgItemMessage(hDlg, IDC_TESTORIENTATION,	BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(ePageUse,				(SendDlgItemMessage(hDlg, IDC_USEPAGESIZE,		BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(ePageNegotiate,			(SendDlgItemMessage(hDlg, IDC_TESTPAGESIZE,		BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eAssumeDPI,				(SendDlgItemMessage(hDlg, IDC_RADIOASSUME,		BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eAskScanNextPage,		(SendDlgItemMessage(hDlg, IDC_SCANNEXTPAGE,		BM_GETCHECK, 0, 0)==BST_CHECKED)); 
	oCmCore->oScanSettings->SetBool(eInvertBlackWhite,		(SendDlgItemMessage(hDlg, IDC_INVERTIMAGE,		BM_GETCHECK, 0, 0)==BST_CHECKED)); 

	iSelection = (int)SendDlgItemMessage(hDlg, IDC_DPI, CB_GETCURSEL, 0, 0);
	SendDlgItemMessage(hDlg, IDC_DPI, CB_GETLBTEXT, (WPARAM)iSelection, (LPARAM)cNumber);
	iNumber = _wtoi(cNumber);
	oCmCore->oScanSettings->SetInt(eAssumeDPI,	iNumber); 

	oCmCore->oScanSettings->Save();
}

void 
CDlgPreferencesScannerAdvanced::DoEnableControls() {

	bool bAssumeDPI;
	HWND hComboDPI;

	//Enable/Disable the DPI combobox
	bAssumeDPI = (SendDlgItemMessage(hDlg, IDC_RADIOASSUME, BM_GETCHECK, 0, 0)==BST_CHECKED);
	hComboDPI = GetDlgItem(hDlg, IDC_DPI);
	EnableWindow(hComboDPI, bAssumeDPI);
	}
