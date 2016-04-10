#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPreferencesScannerPage::CDlgPreferencesScannerPage(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
	CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {
}

CDlgPreferencesScannerPage::~CDlgPreferencesScannerPage(void)
{
	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	if (hDlg!=NULL) {
		DestroyWindow(hDlg);
		oCmCore->oNotifications->UnsubscribeAll(hDlg);
		}
}

void 
CDlgPreferencesScannerPage::OnInitDialog() {

	CCopyingMachineCore* oCmCore;
	wchar_t cText[100];

	oCmCore = (CCopyingMachineCore*)oCore;

	//Fill page combobox
	LoadString(oGlobalInstances.hLanguage, IDS_DEFAULT, cText, 100);
	SendDlgItemMessage(hDlg, IDC_PAGESIZE, CB_ADDSTRING, 0, (LPARAM)cText);
	
	if (oCmCore->oScanSettings->GetBool(ePageUse)) {
		LoadString(oGlobalInstances.hLanguage, IDS_A4, cText, 100);
		SendDlgItemMessage(hDlg, IDC_PAGESIZE, CB_ADDSTRING, 0, (LPARAM)cText);
		LoadString(oGlobalInstances.hLanguage, IDS_A5, cText, 100);
		SendDlgItemMessage(hDlg, IDC_PAGESIZE, CB_ADDSTRING, 0, (LPARAM)cText);
		LoadString(oGlobalInstances.hLanguage, IDS_LETTER, cText, 100);
		SendDlgItemMessage(hDlg, IDC_PAGESIZE, CB_ADDSTRING, 0, (LPARAM)cText);
		LoadString(oGlobalInstances.hLanguage, IDS_LEGAL, cText, 100);
		SendDlgItemMessage(hDlg, IDC_PAGESIZE, CB_ADDSTRING, 0, (LPARAM)cText);
		}

	int iPage = oCmCore->oScanSettings->GetInt(ePageValue)+1;
	if (SendDlgItemMessage(hDlg, IDC_PAGESIZE, CB_SETCURSEL, (WPARAM)iPage, 0)==CB_ERR) {
		SendDlgItemMessage(hDlg, IDC_PAGESIZE, CB_SETCURSEL, (WPARAM)0, 0);
		}

	SendDlgItemMessage(hDlg, IDC_RADIOPORTRAIT, BM_SETCHECK, oCmCore->oScanSettings->GetInt(eOrientationValue)==0 ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_RADIOLANDSCAPE, BM_SETCHECK, oCmCore->oScanSettings->GetInt(eOrientationValue)==0 ? BST_UNCHECKED : BST_CHECKED, 0);


	//Enable/disable controls
	DoEnableControls();

	//Update the image
	DoUpdateImage();

	//Subsribe to notification
	oCmCore->oNotifications->Subscribe(hDlg, eNotificationScanSettingsChanged);
	}

BOOL 
CDlgPreferencesScannerPage::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;
	CCopyingMachineCore* oCmCore;

	bReturn = false;

	oCmCore = (CCopyingMachineCore*)oCore;

	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_CAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}

			break;
		
		case WM_CORENOTIFICATION:

			sCoreNotification cNotification;

			if (oCmCore->oNotifications->GetNotificationSpecific(hDlg, cNotification, eNotificationScanSettingsChanged)) {

				DoEnableControls();
				}
			break;
		};	


	return bReturn;
	}

BOOL 
CDlgPreferencesScannerPage::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	switch (iCommand) {

		case IDC_RADIOPORTRAIT:
		case IDC_RADIOLANDSCAPE:

			DoUpdateImage();

			SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
			break;
		}


	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesScannerPage::OnCloseOk() {


	}

//This function handles the localize action
void 
CDlgPreferencesScannerPage::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	LocalizeControl(IDC_TOPIC1, IDS_PAGESIZE);
	LocalizeControl(IDC_TOPIC2, IDS_ORIENTATION);
	LocalizeControl(IDC_RADIOLANDSCAPE, IDS_LANDSCAPE);
	LocalizeControl(IDC_RADIOPORTRAIT, IDS_PORTRAIT);
}

//This function handles the Apply event
void 
CDlgPreferencesScannerPage::OnApply() {

	CCopyingMachineCore* oCmCore;
	int iPage;
	int iOrientation;

	oCmCore = (CCopyingMachineCore*)oCore;

	iPage = (int)SendDlgItemMessage(hDlg, IDC_PAGESIZE, CB_GETCURSEL, 0, 0);
	if (iPage!=CB_ERR && iPage>=0 && iPage<=4) {
		oCmCore->oScanSettings->SetInt(ePageValue, iPage-1);
		}


	iOrientation = (SendDlgItemMessage(hDlg, IDC_RADIOPORTRAIT, BM_GETCHECK, 0, 0)==BST_CHECKED ? 0 : 1);
	oCmCore->oScanSettings->SetInt(eOrientationValue, iOrientation);
	}

void 
CDlgPreferencesScannerPage::DoEnableControls() {

	CCopyingMachineCore* oCmCore;
	HWND hControl;
	bool bEnable;

	oCmCore = (CCopyingMachineCore*)oCore;

	hControl = GetDlgItem(hDlg, IDC_PAGESIZE);
	bEnable = oCmCore->oScanSettings->GetBool(ePageUse) && !oCmCore->oScanSettings->GetBool(eAlwaysShowInterface);
	EnableWindow(hControl, bEnable);
	
	bEnable = oCmCore->oScanSettings->GetBool(eOrientationUse) && !oCmCore->oScanSettings->GetBool(eAlwaysShowInterface);
	hControl = GetDlgItem(hDlg, IDC_RADIOPORTRAIT);
	EnableWindow(hControl, bEnable);
	hControl = GetDlgItem(hDlg, IDC_RADIOLANDSCAPE);
	EnableWindow(hControl, bEnable);
	}

void 
CDlgPreferencesScannerPage::DoUpdateImage() {

	int iImage = (SendDlgItemMessage(hDlg, IDC_RADIOPORTRAIT, BM_GETCHECK, 0, 0)==BST_CHECKED ? IDB_IMAGEPORTRAIT : IDB_IMAGELANDSCAPE);
	
	HBITMAP hBitmap = LoadBitmap(oGlobalInstances.hInstance, MAKEINTRESOURCE(iImage));

	hBitmap = (HBITMAP)SendDlgItemMessage(hDlg, IDC_IMAGE, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
	
	if (hBitmap!=NULL) FreeResource(hBitmap);
}