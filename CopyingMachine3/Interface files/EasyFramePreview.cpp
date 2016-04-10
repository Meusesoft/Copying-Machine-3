#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "EasyFrame.h"
#include <math.h>


CEasyFramePreview::CEasyFramePreview(sGlobalInstances pInstances)
{
	hWnd = NULL;

	oTrace = pInstances.oTrace;
	oRegistry = pInstances.oRegistry;
	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;

	iDocumentID = -1;
	oDoubleBufferBitmap = NULL;
	oPreviewBitmap = NULL;
	oPrintPreviewSettings = oCore->oPrintSettings;

	oGlobalInstances = pInstances;
}

CEasyFramePreview::~CEasyFramePreview(void)
{
	oCore->oNotifications->UnsubscribeAll(hWnd);
	if (hWnd!=NULL) Destroy();
}

HWND CEasyFramePreview::Create(HINSTANCE phInstance, HWND phParent) {

	oTrace->StartTrace(__WFUNCTION__);
	
	hInstance = phInstance;

	//Create the main window
	HWND hResult = (HWND)(CreateDialogParam (hInstance, MAKEINTRESOURCE(IDD_EASYFRAMEPREVIEW), 
					phParent, (DLGPROC)DialogTemplateProc, (LPARAM)this)!=0); 

	hParent = phParent;

	oTrace->EndTrace(__WFUNCTION__, (bool)(hResult!=NULL));

	return hWnd;
}

void CEasyFramePreview::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	DestroyWindow(hWnd);
	hWnd=NULL;

	oTrace->EndTrace(__WFUNCTION__);
	}


//This function processes all the message
BOOL CALLBACK CEasyFramePreview::DialogTemplateProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	bool bReturn;

	bReturn = true;

	CEasyFramePreview* oThisDialog;

	oThisDialog = (CEasyFramePreview*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (message) {
	
		case WM_INITDIALOG:

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG)lParam); //save the pointer to the dialog 
			oThisDialog = (CEasyFramePreview*)lParam;
			oThisDialog->hWnd = hwndDlg;

			oThisDialog->OnInitDialog();
			break;

		case WM_SYSCOMMAND: 
			switch(wParam & 0xFFF0) { 
              case SC_MOVE: 
              case SC_MAXIMIZE : 
              case SC_SIZE: 
                   bReturn = true; 
                   break;
			}
			break;

		default:
			bReturn = false;
			if (oThisDialog) bReturn = oThisDialog->OnMessage(hwndDlg, message, wParam, lParam);
			break;
		}

	return bReturn;
	}

//This function handles the messages for this dialog. This function has
//access to the private functions of the class. The CALLBACK function doesn't
bool 
CEasyFramePreview::OnMessage(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	bool bResult;

	bResult = true;

	switch (message) {

		case WM_COMMAND:

			OnCommand(LOWORD(wParam), wParam, lParam);
			break;

		case WM_CORENOTIFICATION:

			OnEventCoreNotification();
			break;

		case WM_DRAWITEM:

			DoUpdatePreview((LPDRAWITEMSTRUCT)lParam);
			break;

		default:
			
			bResult = false;
			break;
		}

	return bResult;
	}


//This function handles the WM_COMMAND message
bool 
CEasyFramePreview::OnCommand(int piCommand, WPARAM wParam, LPARAM lParam) {

	bool bReturn;

	oTrace->StartTrace(__WFUNCTION__);

	bReturn = true;

	switch (piCommand) {


		case IDC_ACCEPT:
		case IDC_DECLINE:

			PostMessage(hParent, WM_COMMAND, piCommand, 0);
			break;

		case IDC_PREFERENCES:

			oCore->oDialogs->DlgPreferences(hWnd, ePrinter);
			break;

		default:

			bReturn = false;
			break;
		}

	oTrace->EndTrace(__WFUNCTION__);

	return bReturn;
	}

//This function handles the core notifications
bool 
CEasyFramePreview::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;

	bReturn = false;

	while (oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationLanguageChanged:

				oGlobalInstances.hLanguage = (HINSTANCE)cNotification.pData;
				DoLocalize();
				break;
				
			case eNotificationDocumentPrintingDone:
				
				sPrintDocumentWithSettings* oPrintDocument;

				oPrintDocument = (sPrintDocumentWithSettings*)cNotification.pData;
				
				if (iPrintTicket == oPrintDocument->iPrintRequest) {
				
					HWND hControl;

					delete oDoubleBufferBitmap;
					oDoubleBufferBitmap = NULL;

					hControl = GetDlgItem(hWnd, IDC_PAGEPREVIEW);
					InvalidateRect(hControl, NULL, TRUE); 

					delete oPrintDocument;
					}
				break;

			case eNotificationPrintDeviceChanged:
			case eNotificationPrintSettingsChanged:

				DoFillControls();

				DoClearPreview();
				DoCreatePreview();

				oCore->oNotifications->RemoveNotificationSpecific(hWnd, eNotificationPrintSettingsChanged);
				oCore->oNotifications->RemoveNotificationSpecific(hWnd, eNotificationPrintDeviceChanged);

				bReturn = true;	
				break;
			}
		}

	return bReturn;
	}

//This function handles the WM_INITDIALOG message
void
CEasyFramePreview::OnInitDialog() {

	//Localize captions
	DoLocalize();

	//Fill controls	
	DoFillControls();
	DoEnableControls();

	//Subscribe to notification
	oCore->oNotifications->Subscribe(hWnd, eNotificationPrintSettingsChanged);
	oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentPrintingDone);
	oCore->oNotifications->Subscribe(hWnd, eNotificationPrintDeviceChanged);
	oCore->oNotifications->Subscribe(hWnd, eNotificationLanguageChanged);

	//Show the window
	ShowWindow(hWnd, SW_SHOWNORMAL);
	}

	
//This function fills the comboboxes for resolution and colortype
void
CEasyFramePreview::DoFillControls() {

}

void 
CEasyFramePreview::DoEnableControls() {

	HWND hControl;

	hControl = GetDlgItem(hWnd, IDC_ACCEPT);
	EnableWindow(hControl, iDocumentID!=-1);
	hControl = GetDlgItem(hWnd, IDC_DECLINE);
	EnableWindow(hControl, iDocumentID!=-1);
	}

//This function localizes the controls in this window
void 
CEasyFramePreview::DoLocalize() {

	LocalizeWindowCaption(IDS_PREVIEW);

	LocalizeControl(IDC_ACCEPT,				IDS_ACCEPT);
	LocalizeControl(IDC_DECLINE,			IDS_DECLINE);
	LocalizeControl(IDC_PREFERENCES,		IDS_PREFERENCES);
	}


//This function changes the caption/text of a control
void 
CEasyFramePreview::LocalizeControl(int piControl, int piResource) {

	HWND hControl;
	wchar_t* cText;

	hControl = GetDlgItem(hWnd, piControl);
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	LoadString(oGlobalInstances.hLanguage, piResource, cText, 400);

	SetWindowText(hControl, cText);

	free(cText);
}

void 
CEasyFramePreview::LocalizeWindowCaption(int piResource) {

	wchar_t* cText;

	cText = (wchar_t*)malloc(sizeof(wchar_t) * MAX_PATH);

	LoadString(oGlobalInstances.hLanguage, piResource, cText, MAX_PATH);

	SetWindowText(hWnd, cText);

	free(cText);
	}

//This function reads out an edit control and converts its value to
//an integer
int  
CEasyFramePreview::GetIntegerFromEditControl(int piControl) {

	int iResult;
	wchar_t cNumber[10];

	SendDlgItemMessage(hWnd, piControl, WM_GETTEXT, (WPARAM)9, (LPARAM)cNumber);
	iResult = _wtoi(cNumber);

	return iResult;
	}

//Set the document_id of the document to preview
void
CEasyFramePreview::SetPreviewDocument(int piDocumentID, CPrintSettings* poPrintPreviewSettings) {

	iDocumentID = piDocumentID;
	DoEnableControls();
	oPrintPreviewSettings = poPrintPreviewSettings;
	if (oPrintPreviewSettings==NULL) oPrintPreviewSettings = oCore->oPrintSettings;

	DoClearPreview();
	DoCreatePreview();
	}

//This function clear the preview and repaints the window
void 
CEasyFramePreview::DoClearPreview() {

	HWND hControl;

	delete oPreviewBitmap;
	oPreviewBitmap = NULL;
	delete oDoubleBufferBitmap;
	oDoubleBufferBitmap = NULL;

	hControl = GetDlgItem(hWnd, IDC_PAGEPREVIEW);
	InvalidateRect(hControl, NULL, TRUE); 
	}

//This function creates the bitmap to fill the preview control with
void
CEasyFramePreview::CreateDoubleBuffer() {

	Bitmap*	oBitmap;
	RECT	cPreviewRect;
	HWND	hControl;
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	hControl = GetDlgItem(hWnd, IDC_PAGEPREVIEW);
	GetWindowRect(hControl, &cPreviewRect);
	oBitmap = new Bitmap(cPreviewRect.right - cPreviewRect.left, 
						 cPreviewRect.bottom - cPreviewRect.top, PixelFormat32bppRGB);

	if (oDoubleBufferBitmap) delete oDoubleBufferBitmap;
	oDoubleBufferBitmap = new CBitmap(oBitmap, NULL);

	DoubleBufferDrawBackground();
	DoubleBufferDrawPage();

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function draw the background of the preview window
void 
CEasyFramePreview::DoubleBufferDrawBackground() {

	Bitmap*		oCanvasBitmap;
	Graphics*	oCanvas;
	Rect		cRectPreview;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oCanvasBitmap = oDoubleBufferBitmap->GetBitmap();
	cRectPreview.Height = oCanvasBitmap->GetHeight();
	cRectPreview.Width  = oCanvasBitmap->GetWidth();
	cRectPreview.X = 0;
	cRectPreview.Y = 0;
	oCanvas = Graphics::FromImage(oCanvasBitmap);

	//Draw the background
	Pen oBorderPen(Color(255, 64, 64, 64));
	oCanvas->DrawRectangle(&oBorderPen, cRectPreview);
	
	cRectPreview.Inflate(-1, -1);

	//we use a linear gradient brush to have a nice effect in the background
	double dTan = ((double)cRectPreview.Height / (double)cRectPreview.Width);
	double dTanResult = atan((double)dTan);
	double dDegree = dTanResult * 270 / 3.14f;
	LinearGradientBrush* oBackgroundBrush = new LinearGradientBrush( 
							cRectPreview, 
							Color(255, defBackgroundR, defBackgroundG, defBackgroundB), 
							Color(255, (BYTE)(defBackgroundR * 0.6f), (BYTE)(defBackgroundG * 0.6f), (BYTE)(defBackgroundB * 0.6f)), 
							(REAL)dDegree, 
							false); 
	oCanvas->FillRectangle(oBackgroundBrush, cRectPreview);

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}
	
//This function draws the page image of the preview image
void 
CEasyFramePreview::DoubleBufferDrawPage() {

	Bitmap*		oCanvasBitmap;
	Graphics*	oCanvas;
	RectF		cRectPreview;
	sOutputDevice oOutputDevice;
	CPrintSettings* oTemporarySettings;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oCanvasBitmap = oDoubleBufferBitmap->GetBitmap();

	oPrintPreviewSettings->GetDevice(oOutputDevice);
	oTemporarySettings = oPrintPreviewSettings->Copy();
	oTemporarySettings->bNotifyChanges = false;
	oTemporarySettings->SetBool(ePrintMultiPagesOnPage, false);
	cRectPreview = oCore->oPrintCore->GetStandardPageImageArea(oOutputDevice, oTemporarySettings);
	delete oTemporarySettings;

	float fTranslation;

	fPreviewDpiX = (oCanvasBitmap->GetWidth()-40) / cRectPreview.Width;
	fPreviewDpiY = (oCanvasBitmap->GetHeight()-40) / cRectPreview.Height;
	fTranslation = fPreviewDpiX;
	if (fPreviewDpiX > fPreviewDpiY) fTranslation = fPreviewDpiY; 

	fPreviewDpiX = fPreviewDpiY = fTranslation;

	cPageRect = Rect(0, 0, (INT)(cRectPreview.Width * fTranslation),
						   (INT)(cRectPreview.Height * fTranslation));
	cPageRect.Offset((oCanvasBitmap->GetWidth() - cPageRect.Width) / 2,
				     (oCanvasBitmap->GetHeight() - cPageRect.Height) / 2);

	
	//Bitmap* oDropShadowHelper;
	//RectF	cDropShadowRect;

	//oDropShadowHelper = new Bitmap(oCanvasBitmap->GetWidth()/3, 
	//							   oCanvasBitmap->GetHeight()/3, PixelFormat32bppARGB);
	//cDropShadowRect = RectF(cPageRect.X / 3.0f, cPageRect.Y / 3.0f,
	//					   (cPageRect.Width + 2) / 3.0f, (cPageRect.Height + 2) / 3.0f);
	//SolidBrush oDropShadowBrush(Color(50, 0, 0, 0));
	//oCanvas = Graphics::FromImage(oDropShadowHelper);
	//oCanvas->FillRectangle(&oDropShadowBrush, cDropShadowRect);
	//oCanvas->SetInterpolationMode(InterpolationModeHighQualityBicubic);

	//RectF cDestinationRect = RectF(1, 1, oCanvasBitmap->GetWidth()-1, oCanvasBitmap->GetHeight()-1);
	//oCanvas->DrawImage(oDropShadowHelper, cDestinationRect, 
	//				  0, 0, oDropShadowHelper->GetWidth(), oDropShadowHelper->GetHeight(), 
	//				   UnitPixel, NULL, NULL, NULL);

	//delete oDropShadowHelper;
	
	//Draw the shadow
	cPageRect.Offset(3, 3);
	SolidBrush oShadowBrush(Color(32, 0, 0, 0)); 
	oCanvas = Graphics::FromImage(oCanvasBitmap);
	oCanvas->FillRectangle(&oShadowBrush, cPageRect);
	cPageRect.Offset(-3, -3);

	//Draw the page canvas
	SolidBrush oWhiteBrush(Color(255, 255, 255, 255));
	oCanvas->FillRectangle(&oWhiteBrush, cPageRect);
	
	//Draw the border of the page
	Pen oBlackPen(Color(255, 96, 96, 96));
	oCanvas->DrawRectangle(&oBlackPen, cPageRect);

	//Draw the preview image
	if (oPreviewBitmap) {

		oCanvas->DrawImage(oPreviewBitmap->GetBitmap(), cPageRect.X+1, cPageRect.Y+1, cPageRect.Width-1, cPageRect.Height-1);
		}

	cPageRect.X = 0;
	cPageRect.Y = 0;

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function draw the preview page control
void 
CEasyFramePreview::DoUpdatePreview(LPDRAWITEMSTRUCT pcDrawItem) {

	//Only perform the update on the imagetest control
	if (pcDrawItem->CtlID != IDC_PAGEPREVIEW) return;

	//Check if the double buffer is available, if not make it
	if (oDoubleBufferBitmap==NULL) CreateDoubleBuffer();

	//Update the image in the dialog
	Graphics* oCanvas;
	oCanvas = new Graphics(pcDrawItem->hDC);

	oCanvas->DrawImage(oDoubleBufferBitmap->GetBitmap(), 0, 0);

	delete oCanvas;
}

//This function requests the printqueue to 'print' the document
//to a bitmap
void
CEasyFramePreview::DoCreatePreview() {

	CCopyDocument*	oDocument;
	CPrintSettings*	oTemporarySettings;
	sOutputDevice oOutputDevice;
	Graphics* oCanvas;
	Bitmap*	  oPageBitmap;
	
	oTrace->StartTrace(__WFUNCTION__);

	oDocument = oCore->oDocuments->GetDocumentByID(iDocumentID);

	if (oDocument!=NULL) {

		//Create a new previewbitmap
		if (oPreviewBitmap!=NULL) delete oPreviewBitmap;

		oPageBitmap = new Bitmap(cPageRect.Width, cPageRect.Height, PixelFormat32bppRGB);
		oPageBitmap->SetResolution(fPreviewDpiX, fPreviewDpiY);

		SolidBrush oWhiteBrush(Color(255, 255, 255, 255));
		oCanvas = new Graphics(oPageBitmap);
		oCanvas->FillRectangle(&oWhiteBrush, cPageRect);
		delete oCanvas;

		oPreviewBitmap = new CBitmap(oPageBitmap, NULL);

		//Set temporary print settings
		oOutputDevice.cType		= eOutputBitmap;
		oOutputDevice.oBitmap	= oPreviewBitmap;
		
		oTemporarySettings = oCore->oPrintSettings->Copy();
		oTemporarySettings->bDeleteAfterPrint = true;
		oTemporarySettings->bNotifyChanges	  = false;
		oTemporarySettings->SetDevice(oOutputDevice);
		oTemporarySettings->SetBool(ePrintShowInterface, false);
		oTemporarySettings->SetBool(ePrintAlwaysShowInterface, false);
		oTemporarySettings->SetInt(ePrintCopies, 1);
		oTemporarySettings->oPrintData.bInitializePageRange = false;

		if (oTemporarySettings->GetBool(ePrintMultiPagesOnPage)) {

			//Print 4 pages
			oTemporarySettings->oPrintData.lPage.clear();
			oTemporarySettings->oPrintData.lPage.push_back(0);
			oTemporarySettings->oPrintData.lPage.push_back(1);
			oTemporarySettings->oPrintData.lPage.push_back(2);
			oTemporarySettings->oPrintData.lPage.push_back(3);
			}
		else {

			//Only print one page
			oTemporarySettings->oPrintData.lPage.clear();
			oTemporarySettings->oPrintData.lPage.push_back(0);
			}

		//Add it to the print queue
		iPrintTicket = oDocument->PrintThread(oTemporarySettings);
		}

	oTrace->EndTrace(__WFUNCTION__);
	}