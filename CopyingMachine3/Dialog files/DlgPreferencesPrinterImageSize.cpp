#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPreferencesPrinterImageSize::CDlgPreferencesPrinterImageSize(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
	CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {
}

CDlgPreferencesPrinterImageSize::~CDlgPreferencesPrinterImageSize(void)
{
	if (hDlg!=NULL) DestroyWindow(hDlg);
}

void 
CDlgPreferencesPrinterImageSize::OnInitDialog() {

	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	SendDlgItemMessage(hDlg, IDC_CHKSTRETCHTOFIT, BM_SETCHECK, oCmCore->oPrintSettings->GetBool(ePrintStretch) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_RADIOSHRINK, BM_SETCHECK, oCmCore->oPrintSettings->GetBool(ePrintShrink) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_RADIOCLIP, BM_SETCHECK, oCmCore->oPrintSettings->GetBool(ePrintShrink) ? BST_UNCHECKED : BST_CHECKED, 0);

	//set the magnification setting
	wchar_t cMagnification[20];

	long lMagnification = oCmCore->oPrintSettings->GetInt(ePrintMagnification);
	swprintf_s(cMagnification, 10, L"%d", lMagnification);

	SendDlgItemMessage(hDlg, IDC_MAGNIFICATION, WM_SETTEXT, 0, (LPARAM)cMagnification);
	SendDlgItemMessage(hDlg, IDC_MAGNIFICATION, EM_SETLIMITTEXT, (WPARAM)3, 0);
	SendDlgItemMessage(hDlg, IDC_MAGNIFICATIONSPIN, UDM_SETRANGE, (WPARAM)1, (LPARAM)999);

	//Process settings in the controls
	DoEnableControls();
	DoUpdateImages();
}

BOOL 
CDlgPreferencesPrinterImageSize::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

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
CDlgPreferencesPrinterImageSize::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {


	switch (iCommand) {

		case IDC_CHKSTRETCHTOFIT: 
		case IDC_RADIOSHRINK: 
		case IDC_RADIOCLIP: 
		case IDC_MAGNIFICATIONSPIN:
			SendMessage(hParent, WM_ENABLEAPPLY, 0, 0);
			DoUpdateImages();
			break;

		case IDC_MAGNIFICATION:
			
			//Check if the value in the edit control is different from
			//the setting
			CCopyingMachineCore* oCmCore;

			oCmCore = (CCopyingMachineCore*)oCore;

			if (oCmCore->oPrintSettings->GetInt(ePrintMagnification) != 
				GetIntegerFromEditControl(IDC_MAGNIFICATION)) {

				SendMessage(hParent, WM_ENABLEAPPLY, 0, 0);
				DoUpdateImages();
				}
			
			break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesPrinterImageSize::OnCloseOk() {


	}

//This function handles the localize action
void 
CDlgPreferencesPrinterImageSize::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	LocalizeControl(IDC_TOPIC1, IDS_MAGNIFICATION);
	LocalizeControl(IDC_TOPIC2, IDS_IMAGESMALLERTHANPAGE);
	LocalizeControl(IDC_TOPIC3, IDS_IMAGELARGERTHANPAGE);
	LocalizeControl(IDC_MAGNIFYBY, IDS_MAGNIFYIMAGEBY);
	LocalizeControl(IDC_CHKSTRETCHTOFIT, IDS_STRETCHTOFIT);
	LocalizeControl(IDC_RADIOCLIP, IDS_CLIP);
	LocalizeControl(IDC_RADIOSHRINK, IDS_SHRINKTOFIT);
}

//This function handles the Apply event
void 
CDlgPreferencesPrinterImageSize::OnApply() {

	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	//Shrink and stretch settings
	oCmCore->oPrintSettings->SetBool(ePrintShrink,		(SendDlgItemMessage(hDlg, IDC_RADIOSHRINK,			BM_GETCHECK, 0, 0)==BST_CHECKED));
	oCmCore->oPrintSettings->SetBool(ePrintStretch,		(SendDlgItemMessage(hDlg, IDC_CHKSTRETCHTOFIT,		BM_GETCHECK, 0, 0)==BST_CHECKED));

	//Magnification level
	oCmCore->oPrintSettings->SetInt(ePrintMagnification, GetIntegerFromEditControl(IDC_MAGNIFICATION));

	//Save the settings
	oCmCore->oPrintSettings->Save();
}

void 
CDlgPreferencesPrinterImageSize::DoEnableControls() {

	}

void 
CDlgPreferencesPrinterImageSize::DoUpdateImages() {

	int iNumber;
	int iImage;
	HBITMAP hBitmap;

	//Stretch to fit
	iImage = (SendDlgItemMessage(hDlg, IDC_CHKSTRETCHTOFIT, BM_GETCHECK, 0, 0)==BST_CHECKED ? IDB_IMAGENORMAL : IDB_IMAGESMALL);
	hBitmap = LoadBitmap(oGlobalInstances.hInstance, MAKEINTRESOURCE(iImage));
	hBitmap = (HBITMAP)SendDlgItemMessage(hDlg, IDC_IMAGESTRETCH, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
	if (hBitmap!=NULL) FreeResource(hBitmap);

	//Clip or shrink
	iImage = (SendDlgItemMessage(hDlg, IDC_RADIOSHRINK, BM_GETCHECK, 0, 0)==BST_CHECKED ? IDB_IMAGENORMAL : IDB_IMAGEHUGE);
	hBitmap = LoadBitmap(oGlobalInstances.hInstance, MAKEINTRESOURCE(iImage));
	hBitmap = (HBITMAP)SendDlgItemMessage(hDlg, IDC_IMAGESHRINK, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
	if (hBitmap!=NULL) FreeResource(hBitmap);

	//Magnification level
	iNumber = GetIntegerFromEditControl(IDC_MAGNIFICATION);

	iImage = IDB_IMAGENORMAL;
	if (iNumber<80) iImage = IDB_IMAGESMALL;
	if (iNumber>=150) iImage = IDB_IMAGEHUGE;
	hBitmap = LoadBitmap(oGlobalInstances.hInstance, MAKEINTRESOURCE(iImage));
	hBitmap = (HBITMAP)SendDlgItemMessage(hDlg, IDC_IMAGEMAGNIFICATION, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
	if (hBitmap!=NULL) FreeResource(hBitmap);
	}