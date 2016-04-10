#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPreferencesScannerImage::CDlgPreferencesScannerImage(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {

	}

CDlgPreferencesScannerImage::~CDlgPreferencesScannerImage(void)
{
	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	if (hDlg!=NULL) {
		DestroyWindow(hDlg);
		oCmCore->oNotifications->UnsubscribeAll(hDlg);
		}
}

void 
CDlgPreferencesScannerImage::OnInitDialog() {

	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	

	DoFillColorDepth(GetDlgItem(hDlg, IDC_COLORDEPTH));
	DoFillResolution(GetDlgItem(hDlg, IDC_RESOLUTION));

	////Fill type box
	//int iColorDepth = oCmCore->oScanSettings->GetInt(eColorDepthValue);
	//int iSelIndex = 0;
	//int iItemIndex = 0;
	//
	//LoadString(oGlobalInstances.hLanguage, IDS_BLACKWHITE, cText, 100);
	//if (oCmCore->oScanSettings->bBlackWhite) {
	//	iItemIndex=(int)SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_ADDSTRING, 0, (LPARAM)cText);
	//	if (iItemIndex!=CB_ERR) {
	//		SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)0); //BW
	//		if (iColorDepth == 0) iSelIndex = iItemIndex;
	//		}
	//	}
	//	
	//LoadString(oGlobalInstances.hLanguage, IDS_GRAY, cText, 100);
	//if (oCmCore->oScanSettings->bBlackWhite) {
	//	iItemIndex=(int)SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_ADDSTRING, 0, (LPARAM)cText);
	//	if (iItemIndex!=CB_ERR) {
	//		SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)1); //Gray
	//		if (iColorDepth == 1) iSelIndex = iItemIndex;
	//		}
	//	}

	//LoadString(oGlobalInstances.hLanguage, IDS_COLOR, cText, 100);
	//if (oCmCore->oScanSettings->bBlackWhite) {
	//	iItemIndex=(int)SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_ADDSTRING, 0, (LPARAM)cText);
	//	if (iItemIndex!=CB_ERR) {
	//		SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)2); //Color
	//		if (iColorDepth == 2) iSelIndex = iItemIndex;
	//		}
	//	}

	//if (SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_SETCURSEL, (WPARAM)iSelIndex, 0)==CB_ERR) {
	//	SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_SETCURSEL, (WPARAM)0, 0);
	//	}

	////Fill dpi combobox
	//if (oCmCore->oScanSettings->bResolution75) SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)L"75");
	//if (oCmCore->oScanSettings->bResolution100) SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)L"100");
	//if (oCmCore->oScanSettings->bResolution150) SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)L"150");
	//if (oCmCore->oScanSettings->bResolution200) SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)L"200");
	//if (oCmCore->oScanSettings->bResolution300) SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)L"300");
	//if (oCmCore->oScanSettings->bResolution400) SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)L"400");
	//if (oCmCore->oScanSettings->bResolution600) SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)L"600");
	//if (oCmCore->oScanSettings->bResolution1200) SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)L"1200");

	////set the resolution. If an error occurs the first resolution will be selected
	//int iResolution = oCmCore->oScanSettings->GetInt(eResolutionValue);
	//swprintf_s(cText, 100, L"%d", iResolution);
	//if (SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_SELECTSTRING, 0, (LPARAM)cText)==CB_ERR) {
	//	SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_SETCURSEL, (WPARAM)0, 0);
	//	}

	//set the sliders
	HWND hWndBuddy;
	int iBrightnessValue = oCmCore->oScanSettings->GetInt(eBrightnessValue);
	SendDlgItemMessage(hDlg, IDC_SLIDERBRIGHTNESS, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(-100, 100));
	SendDlgItemMessage(hDlg, IDC_SLIDERBRIGHTNESS, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iBrightnessValue);
	hWndBuddy = GetDlgItem(hDlg, IDC_EDITBRIGHTNESS);
	SendDlgItemMessage(hDlg, IDC_SLIDERBRIGHTNESS, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)20);
	SendDlgItemMessage(hDlg, IDC_EDITBRIGHTNESS, EM_SETLIMITTEXT, (WPARAM)4, 0);
	SendDlgItemMessage(hDlg, IDC_EDITBRIGHTNESS, WM_SETTEXT, 0, (LPARAM)L"+0");
	UpdateEditControl(IDC_SLIDERBRIGHTNESS);

	int iContrastValue = oCmCore->oScanSettings->GetInt(eContrastValue);
	SendDlgItemMessage(hDlg, IDC_SLIDERCONTRAST, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(-100, 100));
	SendDlgItemMessage(hDlg, IDC_SLIDERCONTRAST, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iContrastValue);
	hWndBuddy = GetDlgItem(hDlg, IDC_EDITCONTRAST);
	SendDlgItemMessage(hDlg, IDC_SLIDERCONTRAST, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)20);
	SendDlgItemMessage(hDlg, IDC_EDITCONTRAST, EM_SETLIMITTEXT, (WPARAM)4, 0);
	SendDlgItemMessage(hDlg, IDC_EDITCONTRAST, WM_SETTEXT, 0, (LPARAM)L"+0");
	UpdateEditControl(IDC_SLIDERCONTRAST);

	//Enable/Disable the controls
	DoEnableControls();

	//Subsribe to notification
	oCmCore->oNotifications->Subscribe(hDlg, eNotificationScanSettingsChanged);
	oCmCore->oNotifications->Subscribe(hDlg, eNotificationScanDeviceChanged);
	}

BOOL 
CDlgPreferencesScannerImage::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;
	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	bReturn = false;
	
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
			if (oCmCore->oNotifications->GetNotificationSpecific(hDlg, cNotification, eNotificationScanDeviceChanged)) {

				DoFillColorDepth(GetDlgItem(hDlg, IDC_COLORDEPTH));
				DoFillResolution(GetDlgItem(hDlg, IDC_RESOLUTION));
				DoEnableControls();
				}
			break;

		case WM_DRAWITEM:

			DoUpdateImage((LPDRAWITEMSTRUCT)lParam);

			bReturn = true;
			break;

		case WM_VSCROLL:
		case WM_HSCROLL:

			switch (LOWORD(wParam)) {
			
				case TB_THUMBTRACK:
				case TB_LINEUP:
				case TB_LINEDOWN:
				case TB_THUMBPOSITION:

					HWND hWnd;
					hWnd = GetDlgItem(hDlg, IDC_SLIDERBRIGHTNESS);
					if (lParam == (LPARAM)hWnd) UpdateEditControl(IDC_SLIDERBRIGHTNESS);
					hWnd = GetDlgItem(hDlg, IDC_SLIDERCONTRAST);
					if (lParam == (LPARAM)hWnd) UpdateEditControl(IDC_SLIDERCONTRAST);
					SendMessage(hParent, WM_ENABLEAPPLY, 0, 0);
					break;
				};
			break;
		
		
		case WM_NOTIFY:

			NMHDR* oNotification;

			oNotification = (LPNMHDR)lParam;

			switch (oNotification->idFrom) {

				case IDC_SLIDERBRIGHTNESS:
				case IDC_SLIDERCONTRAST:
				
					if (oNotification->code == NM_RELEASEDCAPTURE) {

						UpdateEditControl((int)oNotification->idFrom);
						}
					break;
				}	

			break;
		};	



	if (lParam==(LPARAM)GetDlgItem(hDlg, IDC_EDITBRIGHTNESS)) UpdateSliderControl(IDC_EDITBRIGHTNESS);
	if (lParam==(LPARAM)GetDlgItem(hDlg, IDC_EDITCONTRAST)) UpdateSliderControl(IDC_EDITCONTRAST);

	return bReturn;
	}

BOOL 
CDlgPreferencesScannerImage::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	switch (iCommand) {

		case IDC_RESOLUTION: 
		case IDC_COLORDEPTH: 
			
			if (HIWORD(wParam)==CBN_SELCHANGE) {
				HWND hWnd = GetDlgItem(hDlg, IDC_IMAGETEST);
				InvalidateRect(hWnd, NULL, FALSE);
				SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
				}
			break;

		//case IDC_EDITCONTRAST:
		//case IDC_EDITBRIGHTNESS:
		//	SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
		//	break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesScannerImage::OnCloseOk() {


	}

//This function handles the localize action
void 
CDlgPreferencesScannerImage::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	LocalizeControl(IDC_TOPIC1, IDS_DIMENSION);
	LocalizeControl(IDC_TOPIC2, IDS_BALANCE);
	LocalizeControl(IDC_TXTCOLORDEPTH, IDS_TYPE);
	LocalizeControl(IDC_TXTRESOLUTION, IDS_RESOLUTION);
	LocalizeControl(IDC_TXTBRIGHTNESS, IDS_BRIGHTNESS);
	LocalizeControl(IDC_TXTCONTRAST, IDS_CONTRAST);
	}

//This function handles the Apply event
void 
CDlgPreferencesScannerImage::OnApply() {

	int iSelIndex;
	int iNumber;
	wchar_t cNumber[10];
	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	//ColorDepth
	iSelIndex = (int)SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_GETCURSEL, 0, 0);
	int iColorDepthValue = (int)SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_GETITEMDATA, (WPARAM)iSelIndex, 0);
	if (iColorDepthValue!=CB_ERR) oCmCore->oScanSettings->SetInt(eColorDepthValue, iColorDepthValue);

	//Resolution
	iSelIndex = (int)SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_GETCURSEL, 0, 0);
	SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_GETLBTEXT, (WPARAM)iSelIndex, (LPARAM)cNumber);
	iNumber = _wtoi(cNumber);
	oCmCore->oScanSettings->SetInt(eResolutionValue, iNumber); 

	//Brightness
	int iBrightnessValue = (int)SendDlgItemMessage(hDlg, IDC_SLIDERBRIGHTNESS, TBM_GETPOS, 0, 0);
	oCmCore->oScanSettings->SetInt(eBrightnessValue, iBrightnessValue); 

	//Contrast
	int iContrastValue = (int)SendDlgItemMessage(hDlg, IDC_SLIDERCONTRAST, TBM_GETPOS, 0, 0);
	oCmCore->oScanSettings->SetInt(eContrastValue, iContrastValue); 

	}

//Update the edit control, the buddy of the given slider
void
CDlgPreferencesScannerImage::UpdateEditControl(int pIdControl) {

	int IdEdit;
	int iValue;
	int iEditValue;
	wchar_t cValue[10];

	iValue = (int)SendDlgItemMessage(hDlg, pIdControl, TBM_GETPOS, 0, 0);

	switch (pIdControl) {

		case IDC_SLIDERBRIGHTNESS: IdEdit=IDC_EDITBRIGHTNESS; break;
		case IDC_SLIDERCONTRAST: IdEdit=IDC_EDITCONTRAST; break;
		}

	SendDlgItemMessage(hDlg, IdEdit, WM_GETTEXT, (LPARAM)10, (WPARAM)cValue);
	iEditValue = _wtoi(cValue);

	if (iEditValue != iValue) {

		swprintf_s(cValue, 10, L"%+d", iValue);

		SendDlgItemMessage(hDlg, IdEdit, WM_SETTEXT, 0, (LPARAM)cValue);
			
		HWND hWnd = GetDlgItem(hDlg, IDC_IMAGETEST);
		InvalidateRect(hWnd, NULL, FALSE);
		}
}

void
CDlgPreferencesScannerImage::UpdateSliderControl(int pIdControl) {

	int IdSlider;
	int iValue;
	wchar_t cValue[10];

	if (SendDlgItemMessage(hDlg, pIdControl, EM_GETMODIFY, 0, 0)!=0) {

		//Reset the modify bit
		SendDlgItemMessage(hDlg, pIdControl, EM_SETMODIFY, (WPARAM)FALSE, 0);

		//Get the buddy control
		switch (pIdControl) {

			case IDC_EDITBRIGHTNESS: IdSlider=IDC_SLIDERBRIGHTNESS; break;
			case IDC_EDITCONTRAST: IdSlider=IDC_SLIDERCONTRAST; break;
			}

		//Retrieve the value in the edit
		SendDlgItemMessage(hDlg, pIdControl, WM_GETTEXT, (WPARAM)10, (LPARAM)cValue);
		iValue = _wtoi(cValue);
		
		//Set the value of the slider
		SendDlgItemMessage(hDlg, IdSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iValue);

		//Update image
		HWND hWnd = GetDlgItem(hDlg, IDC_IMAGETEST);
		InvalidateRect(hWnd, NULL, FALSE);
		}
	}

void 
CDlgPreferencesScannerImage::DoEnableControls() {

	CCopyingMachineCore* oCmCore;
	CScanDevice* oScanDevice;
	HWND hControl;
	bool bEnable;

	oCmCore = (CCopyingMachineCore*)oCore;

	oScanDevice = oCmCore->oScanSettings->GetScanner();

	if (oScanDevice!=NULL) {

		hControl = GetDlgItem(hDlg, IDC_COLORDEPTH);
		bEnable = oCmCore->oScanSettings->GetBool(eColorDepthUse) && !oCmCore->oScanSettings->GetBool(eAlwaysShowInterface) && oScanDevice->bColorDepth;
		EnableWindow(hControl, bEnable);
		
		hControl = GetDlgItem(hDlg, IDC_RESOLUTION);
		bEnable = oCmCore->oScanSettings->GetBool(eResolutionUse) && !oCmCore->oScanSettings->GetBool(eAlwaysShowInterface) && oScanDevice->bResolution;
		EnableWindow(hControl, bEnable);

		bEnable = oCmCore->oScanSettings->GetBool(eBrightnessUse) && !oCmCore->oScanSettings->GetBool(eAlwaysShowInterface) && oScanDevice->bBrightness;
		hControl = GetDlgItem(hDlg, IDC_EDITBRIGHTNESS);
		EnableWindow(hControl, bEnable);
		hControl = GetDlgItem(hDlg, IDC_SLIDERBRIGHTNESS);
		EnableWindow(hControl, bEnable);

		bEnable = oCmCore->oScanSettings->GetBool(eContrastUse) && !oCmCore->oScanSettings->GetBool(eAlwaysShowInterface) && oScanDevice->bContrast;
		hControl = GetDlgItem(hDlg, IDC_EDITCONTRAST);
		EnableWindow(hControl, bEnable);
		hControl = GetDlgItem(hDlg, IDC_SLIDERCONTRAST);
		EnableWindow(hControl, bEnable);
		}
	}

void 
CDlgPreferencesScannerImage::DoFillResolution(HWND hResolutionBox) {

	wchar_t cText[100];
	CCopyingMachineCore* oCmCore;
	CScanDevice* oScanDevice;

	oCmCore = (CCopyingMachineCore*)oCore;

	//Get the capablities of the current device
	oCmCore->oScanCore->GetDeviceCapabilities();

	//Clear resolution box
	SendMessage(hResolutionBox, CB_RESETCONTENT, 0, 0);

	oScanDevice = oCmCore->oScanSettings->GetScanner();

	if (oScanDevice!=NULL) {

		//Fill dpi combobox
		if (oScanDevice->bResolution75)  SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"75");
		if (oScanDevice->bResolution100) SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"100");
		if (oScanDevice->bResolution150) SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"150");
		if (oScanDevice->bResolution200) SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"200");
		if (oScanDevice->bResolution300) SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"300");
		if (oScanDevice->bResolution400) SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"400");
		if (oScanDevice->bResolution600) SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"600");
		if (oScanDevice->bResolution1200) SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"1200");
		}

	//set the resolution. If an error occurs the first resolution will be selected
	int iResolution = oCmCore->oScanSettings->GetInt(eResolutionValue);
	swprintf_s(cText, 100, L"%d", iResolution);
	if (SendMessage(hResolutionBox, CB_SELECTSTRING, 0, (LPARAM)cText)==CB_ERR) {
		SendMessage(hResolutionBox, CB_SETCURSEL, (WPARAM)0, 0);
		}
}

void 
CDlgPreferencesScannerImage::DoFillColorDepth(HWND hColorBox) {

	wchar_t cText[100];
	CCopyingMachineCore* oCmCore;
	CScanDevice* oScanDevice;

	oCmCore = (CCopyingMachineCore*)oCore;

	//Get the capablities of the current device
	oCmCore->oScanCore->GetDeviceCapabilities();

	//Clear color and resolution box
	SendMessage(hColorBox, CB_RESETCONTENT, 0, 0);

	//Fill type box
	int iColorDepth = oCmCore->oScanSettings->GetInt(eColorDepthValue);
	int iSelIndex = 0;
	int iItemIndex = 0;

	//Fill color type box
	oScanDevice = oCmCore->oScanSettings->GetScanner();

	if (oScanDevice!=NULL) {

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


//This function creates a preview image based on the settings in the dialog. It isn't the best
//and most efficient routine but it does the job and it will only be performed on small images.
void 
CDlgPreferencesScannerImage::DoUpdateImage(LPDRAWITEMSTRUCT pcDrawItem) {

	//Bitmap* oBitmap;
	int iImage = IDB_IMAGETEST;
	int iSelIndex;
	wchar_t cDPI[10];
	int iDPI;
	ColorMatrix* oColorMatrix;
	ColorMatrix* oBrightnessMatrix;
	ColorMatrix* oContrastMatrix;
	Bitmap* oBitmap;
	Bitmap* oTempBitmap;

	CCopyingMachineCore* oCmCore;
	CScanDevice *oScanDevice;

	oCmCore = (CCopyingMachineCore*)oCore;
	oScanDevice = oCmCore->oScanCore->GetCurrentScanner();


	//Only perform the update on the imagetest control
	if (pcDrawItem->CtlID != IDC_IMAGETEST) return;

	//Create a bitmap instance with the resouce image
	oBitmap = new Bitmap(oGlobalInstances.hInstance, MAKEINTRESOURCE(iImage));

	//retrieve the settings for brightness, contrast, resolution and color
	int iBrightnessValue = (int)SendDlgItemMessage(hDlg, IDC_SLIDERBRIGHTNESS, TBM_GETPOS, 0, 0);
	int iContrastValue = (int)SendDlgItemMessage(hDlg, IDC_SLIDERCONTRAST, TBM_GETPOS, 0, 0);
	int iColorDepth = (int)SendDlgItemMessage(hDlg, IDC_COLORDEPTH, CB_GETCURSEL, 0, 0);
	iSelIndex = (int)SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_GETCURSEL, 0, 0);
	SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_GETLBTEXT, (WPARAM)iSelIndex, (LPARAM)cDPI);
	
	//do some calculations on the resolution setting
	iDPI = _wtoi(cDPI);
	int iResize = 1;
	if (iDPI<=200) iResize *= 2;
	if (iDPI<=100) iResize *= 2;

	//If the current device is the file-input device, do not change the image.
	if (oScanDevice!=NULL) {
		if (oScanDevice->cInterface == eInputFile) {
			
			iResize = 1;
			iColorDepth = 2;
			iContrastValue = 0;
			iBrightnessValue = 0;
			}
		}

	//Create a color matrix for the brightess and contrast adjustments and place in an
	//ImageAttributes instance
	oColorMatrix = CBitmap::CreateStandardColorMatrix();
	oBrightnessMatrix = CBitmap::CreateBrightnessColorMatrix(iBrightnessValue);
	oContrastMatrix = CBitmap::CreateContrastColorMatrix(iContrastValue);
	CBitmap::MultiplyColorMatrix(oColorMatrix, oBrightnessMatrix, oColorMatrix);
	CBitmap::MultiplyColorMatrix(oColorMatrix, oContrastMatrix, oColorMatrix);

	ImageAttributes* oImageAttributes;
	oImageAttributes = new ImageAttributes();
	oImageAttributes->SetColorMatrix(oColorMatrix);

	//Perform the brightness and contrast operation on the image.
	Rect cImageRect = Rect(0, 0, oBitmap->GetWidth(), oBitmap->GetHeight());
	oTempBitmap = new Bitmap(cImageRect.Width, cImageRect.Height, PixelFormat24bppRGB);
	Graphics* oGraphics = new Graphics(oTempBitmap);
	oGraphics->DrawImage(oBitmap, cImageRect, 0, 0, cImageRect.Width, cImageRect.Height, UnitPixel, oImageAttributes, NULL, NULL);
	
	delete oBitmap;
	oBitmap = oTempBitmap;
	delete oGraphics;
	delete oColorMatrix;
	delete oImageAttributes;

	//Perform a color depth and resize on the bitmap. We do this directly on the
	//pixeldata because GDI 1.0 doesn't support a convert imageformat to a BW image. It would
	//have been nice to be able to do this with a color matrix.

	Bitmap* oDestinationBitmap = new Bitmap(cImageRect.Width, cImageRect.Height, PixelFormat24bppRGB);
	BitmapData* bitmapDataSource = new BitmapData;
	BitmapData* bitmapDataDestination = new BitmapData;

	oBitmap->LockBits(&cImageRect, ImageLockModeRead, 
					PixelFormat24bppRGB, bitmapDataSource); 
	oDestinationBitmap->LockBits(&cImageRect, ImageLockModeWrite, 
					PixelFormat24bppRGB, bitmapDataDestination); 

	BYTE *Source;
	BYTE *Destination;
	int iLuminance;

	//do the pixel calculations
	for (int iY=0; iY<(int)oBitmap->GetHeight(); iY++) {
	
		Source = (BYTE*)bitmapDataSource->Scan0 + ((iY / iResize) * iResize) * bitmapDataSource->Stride;
		Destination = (BYTE*)bitmapDataDestination->Scan0 + iY * bitmapDataDestination->Stride;

		for (int iX=0; iX<(int)oBitmap->GetWidth(); iX++) {

			//Colordepth
			if (iColorDepth<2) {

				iLuminance = (int)(Source[0]*0.3 + Source[1]*0.59 + Source[2]*0.11);

				if (iColorDepth<1) {
					
					if (iLuminance<128) iLuminance=0; else iLuminance=255;
					}

				Destination[0]=iLuminance;
				Destination[1]=iLuminance;
				Destination[2]=iLuminance;
				}
			else {
				Destination[0]=Source[0];
				Destination[1]=Source[1];
				Destination[2]=Source[2];
				}
			
			if ((iX % iResize)==0) Source += 3 * iResize;
			Destination += 3;
			}
		}


	oBitmap->UnlockBits(bitmapDataSource);
	oDestinationBitmap->UnlockBits(bitmapDataDestination);

	delete oBitmap;
	oBitmap = oDestinationBitmap;

	//Update the image in the dialog
	Graphics* oCanvas;
	oCanvas = new Graphics(pcDrawItem->hDC);

	oCanvas->DrawImage(oBitmap, cImageRect, 0, 0, cImageRect.Width, cImageRect.Height, UnitPixel, NULL);

	Pen* oPen = new Pen(Color::Black);
	oCanvas->DrawRectangle(oPen, Rect(0, 0, cImageRect.Width-1, cImageRect.Height-1));
	delete oPen;	

	delete oCanvas;
	delete oBitmap;
}

