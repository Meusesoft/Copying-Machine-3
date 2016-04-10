#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPreferencesPrinterCopiesAlignment::CDlgPreferencesPrinterCopiesAlignment(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
	CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {
}

CDlgPreferencesPrinterCopiesAlignment::~CDlgPreferencesPrinterCopiesAlignment(void)
{
	if (hDlg!=NULL) DestroyWindow(hDlg);
}

void 
CDlgPreferencesPrinterCopiesAlignment::OnInitDialog() {

	CCopyingMachineCore* oCmCore;
	ePrintAlignmentType cPrintAlignment;

	oCmCore = (CCopyingMachineCore*)oCore;

	//set the copies settings
	wchar_t cCopies[20];

	long lCopies = oCmCore->oPrintSettings->GetInt(ePrintCopies);
	swprintf_s(cCopies, 10, L"%d", lCopies);

	SendDlgItemMessage(hDlg, IDC_COPIESNUMBER, WM_SETTEXT, 0, (LPARAM)cCopies);
	SendDlgItemMessage(hDlg, IDC_COPIESNUMBER, EM_SETLIMITTEXT, (WPARAM)2, 0);
	SendDlgItemMessage(hDlg, IDC_COPIESSPIN, UDM_SETRANGE, (WPARAM)1, (LPARAM)99);

	SendDlgItemMessage(hDlg, IDC_CHKMULTIPLECOPIESONPAGE, BM_SETCHECK, oCmCore->oPrintSettings->GetBool(ePrintMultipleCopiesOnPage) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_CHKMULTIPLEPAGESONPAGE, BM_SETCHECK, oCmCore->oPrintSettings->GetBool(ePrintMultiPagesOnPage) ? BST_CHECKED : BST_UNCHECKED, 0);

	//Set the alignment
	cPrintAlignment = (ePrintAlignmentType)oCmCore->oPrintSettings->GetInt(ePrintAlignment);

	//Set vertical
	switch (cPrintAlignment) {

		case eAlignNorthWest:
		case eAlignNorthCenter:
		case eAlignNorthEast:

			SendDlgItemMessage(hDlg, IDC_RADIOTOP, BM_SETCHECK, BST_CHECKED, 0);
			break;

		case eAlignCenterWest:
		case eAlignCenterCenter:
		case eAlignCenterEast:

			SendDlgItemMessage(hDlg, IDC_RADIOVCENTER, BM_SETCHECK, BST_CHECKED, 0);
			break;

		case eAlignSouthWest:
		case eAlignSouthCenter:
		case eAlignSouthEast:

			SendDlgItemMessage(hDlg, IDC_RADIOBOTTOM, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	
	switch (cPrintAlignment) {

		case eAlignNorthWest:
		case eAlignCenterWest:
		case eAlignSouthWest:

			SendDlgItemMessage(hDlg, IDC_RADIOLEFT, BM_SETCHECK, BST_CHECKED, 0);
			break;

		case eAlignNorthCenter:
		case eAlignCenterCenter:
		case eAlignSouthCenter:

			SendDlgItemMessage(hDlg, IDC_RADIOHCENTER, BM_SETCHECK, BST_CHECKED, 0);
			break;

		case eAlignNorthEast:
		case eAlignCenterEast:
		case eAlignSouthEast:

			SendDlgItemMessage(hDlg, IDC_RADIORIGHT, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}

	DoEnableControls();
	DoUpdateImages();
}

BOOL 
CDlgPreferencesPrinterCopiesAlignment::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

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
CDlgPreferencesPrinterCopiesAlignment::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {


	switch (iCommand) {

		case IDC_RADIOTOP: 
		case IDC_RADIOVCENTER: 
		case IDC_RADIOBOTTOM: 
		case IDC_RADIOLEFT: 
		case IDC_RADIOHCENTER: 
		case IDC_RADIORIGHT: 
		case IDC_CHKMULTIPLECOPIESONPAGE: 
		case IDC_CHKMULTIPLEPAGESONPAGE: 
			DoEnableControls();
			DoUpdateImages();
			SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
			break;

		case IDC_COPIESNUMBER: 
			CCopyingMachineCore* oCmCore;

			oCmCore = (CCopyingMachineCore*)oCore;

			if (oCmCore->oPrintSettings->GetInt(ePrintCopies) != 
				GetIntegerFromEditControl(IDC_COPIESNUMBER)) {

				DoEnableControls();
				DoUpdateImages();
				SendMessage(hParent, WM_ENABLEAPPLY, 0, 0);
				}
			break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesPrinterCopiesAlignment::OnCloseOk() {

	}

//This function handles the localize action
void 
CDlgPreferencesPrinterCopiesAlignment::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);
	
	LocalizeControl(IDC_TOPIC1, IDS_COPIES);
	LocalizeControl(IDC_TOPIC2, IDS_ALIGNMENT);
	LocalizeControl(IDC_TOPICHORIZONTAL, IDS_HORIZONTAL);
	LocalizeControl(IDC_TOPICVERTICAL, IDS_VERTICAL);

	LocalizeControl(IDC_TXTCOPIES, IDS_NUMBEROFCOPIES);
	LocalizeControl(IDC_CHKMULTIPLECOPIESONPAGE, IDS_MULTIPLECOPIESONPAGE);
	LocalizeControl(IDC_CHKMULTIPLEPAGESONPAGE, IDS_MULTIPLEIMAGESONPAGE);
	LocalizeControl(IDC_RADIOTOP, IDS_TOP);
	LocalizeControl(IDC_RADIOVCENTER, IDS_CENTER);
	LocalizeControl(IDC_RADIOBOTTOM, IDS_BOTTOM);
	LocalizeControl(IDC_RADIOLEFT, IDS_LEFT);
	LocalizeControl(IDC_RADIOHCENTER, IDS_CENTER);
	LocalizeControl(IDC_RADIORIGHT, IDS_RIGHT);
	}

//This function handles the Apply event
void 
CDlgPreferencesPrinterCopiesAlignment::OnApply() {

	CCopyingMachineCore* oCmCore;
	ePrintAlignmentType cPrintAlignment;

	oCmCore = (CCopyingMachineCore*)oCore;

	//Multiple copies
	oCmCore->oPrintSettings->SetBool(ePrintMultiPagesOnPage,			(SendDlgItemMessage(hDlg, IDC_CHKMULTIPLEPAGESONPAGE,	BM_GETCHECK, 0, 0)==BST_CHECKED));
	oCmCore->oPrintSettings->SetBool(ePrintMultipleCopiesOnPage,		(SendDlgItemMessage(hDlg, IDC_CHKMULTIPLECOPIESONPAGE,	BM_GETCHECK, 0, 0)==BST_CHECKED));

	//Alignment
	cPrintAlignment = eAlignNorthWest;
	if (SendDlgItemMessage(hDlg, IDC_RADIOTOP, BM_GETCHECK, 0, 0)==BST_CHECKED) {

		if (SendDlgItemMessage(hDlg, IDC_RADIOLEFT, BM_GETCHECK, 0, 0)==BST_CHECKED) cPrintAlignment = eAlignNorthWest;
		if (SendDlgItemMessage(hDlg, IDC_RADIOHCENTER, BM_GETCHECK, 0, 0)==BST_CHECKED) cPrintAlignment = eAlignNorthCenter;
		if (SendDlgItemMessage(hDlg, IDC_RADIORIGHT, BM_GETCHECK, 0, 0)==BST_CHECKED) cPrintAlignment = eAlignNorthEast;
		}
	if (SendDlgItemMessage(hDlg, IDC_RADIOVCENTER, BM_GETCHECK, 0, 0)==BST_CHECKED) {

		if (SendDlgItemMessage(hDlg, IDC_RADIOLEFT, BM_GETCHECK, 0, 0)==BST_CHECKED) cPrintAlignment = eAlignCenterWest;
		if (SendDlgItemMessage(hDlg, IDC_RADIOHCENTER, BM_GETCHECK, 0, 0)==BST_CHECKED) cPrintAlignment = eAlignCenterCenter;
		if (SendDlgItemMessage(hDlg, IDC_RADIORIGHT, BM_GETCHECK, 0, 0)==BST_CHECKED) cPrintAlignment = eAlignCenterEast;
		}
	if (SendDlgItemMessage(hDlg, IDC_RADIOBOTTOM, BM_GETCHECK, 0, 0)==BST_CHECKED) {

		if (SendDlgItemMessage(hDlg, IDC_RADIOLEFT, BM_GETCHECK, 0, 0)==BST_CHECKED) cPrintAlignment = eAlignSouthWest;
		if (SendDlgItemMessage(hDlg, IDC_RADIOHCENTER, BM_GETCHECK, 0, 0)==BST_CHECKED) cPrintAlignment = eAlignSouthCenter;
		if (SendDlgItemMessage(hDlg, IDC_RADIORIGHT, BM_GETCHECK, 0, 0)==BST_CHECKED) cPrintAlignment = eAlignSouthEast;
		}

	oCmCore->oPrintSettings->SetInt(ePrintAlignment, (int)cPrintAlignment);

	//Number of copies
	oCmCore->oPrintSettings->SetInt(ePrintCopies, GetIntegerFromEditControl(IDC_COPIESNUMBER));

	//Save the settings
	oCmCore->oPrintSettings->Save();
}

void 
CDlgPreferencesPrinterCopiesAlignment::DoEnableControls() {

	}

void 
CDlgPreferencesPrinterCopiesAlignment::DoUpdateImages() {

	int iImage;
	HBITMAP hBitmap;

	//Copies image
	iImage = (SendDlgItemMessage(hDlg, IDC_CHKMULTIPLECOPIESONPAGE, BM_GETCHECK, 0, 0)==BST_CHECKED ? IDB_IMAGECOPIESONPAGE : IDB_IMAGENORMAL);
	iImage = (SendDlgItemMessage(hDlg, IDC_CHKMULTIPLEPAGESONPAGE, BM_GETCHECK, 0, 0)==BST_CHECKED ? IDB_IMAGEIMAGESONPAGE : iImage);
	hBitmap = LoadBitmap(oGlobalInstances.hInstance, MAKEINTRESOURCE(iImage));
	hBitmap = (HBITMAP)SendDlgItemMessage(hDlg, IDC_IMAGECOPIES, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
	if (hBitmap!=NULL) FreeResource(hBitmap);

	//Alignment image
	iImage = IDB_IMAGEALIGNNW;
	if (SendDlgItemMessage(hDlg, IDC_RADIOTOP, BM_GETCHECK, 0, 0)==BST_CHECKED) {

		if (SendDlgItemMessage(hDlg, IDC_RADIOLEFT, BM_GETCHECK, 0, 0)==BST_CHECKED) iImage = IDB_IMAGEALIGNNW;
		if (SendDlgItemMessage(hDlg, IDC_RADIOHCENTER, BM_GETCHECK, 0, 0)==BST_CHECKED) iImage = IDB_IMAGEALIGNNC;
		if (SendDlgItemMessage(hDlg, IDC_RADIORIGHT, BM_GETCHECK, 0, 0)==BST_CHECKED) iImage = IDB_IMAGEALIGNNE;
		}
	if (SendDlgItemMessage(hDlg, IDC_RADIOVCENTER, BM_GETCHECK, 0, 0)==BST_CHECKED) {

		if (SendDlgItemMessage(hDlg, IDC_RADIOLEFT, BM_GETCHECK, 0, 0)==BST_CHECKED) iImage = IDB_IMAGEALIGNCW;
		if (SendDlgItemMessage(hDlg, IDC_RADIOHCENTER, BM_GETCHECK, 0, 0)==BST_CHECKED) iImage = IDB_IMAGEALIGNCC;
		if (SendDlgItemMessage(hDlg, IDC_RADIORIGHT, BM_GETCHECK, 0, 0)==BST_CHECKED) iImage = IDB_IMAGEALIGNCE;
		}
	if (SendDlgItemMessage(hDlg, IDC_RADIOBOTTOM, BM_GETCHECK, 0, 0)==BST_CHECKED) {

		if (SendDlgItemMessage(hDlg, IDC_RADIOLEFT, BM_GETCHECK, 0, 0)==BST_CHECKED) iImage = IDB_IMAGEALIGNSW;
		if (SendDlgItemMessage(hDlg, IDC_RADIOHCENTER, BM_GETCHECK, 0, 0)==BST_CHECKED) iImage = IDB_IMAGEALIGNSC;
		if (SendDlgItemMessage(hDlg, IDC_RADIORIGHT, BM_GETCHECK, 0, 0)==BST_CHECKED) iImage = IDB_IMAGEALIGNSE;
		}
	
	hBitmap = LoadBitmap(oGlobalInstances.hInstance, MAKEINTRESOURCE(iImage));
	hBitmap = (HBITMAP)SendDlgItemMessage(hDlg, IDC_IMAGEALIGNMENT, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
	if (hBitmap!=NULL) FreeResource(hBitmap);
}