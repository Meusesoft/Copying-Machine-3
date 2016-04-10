#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "PagePreview.h"
#include <math.h>


CPagePreview::CPagePreview(sGlobalInstances pInstances)
{
	oTrace = pInstances.oTrace;
	oRegistry = pInstances.oRegistry;
	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;

	iDocumentID = -1;
	iPrintTicket = -1;
	oPreviewBitmap = NULL;
	oPrintPreviewSettings = oCore->oPrintSettings->Copy();

	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &pInstances, sizeof(sGlobalInstances));
}

CPagePreview::~CPagePreview(void)
{
}

//This function subscribes the given window to the notification it needs to respond to
//while working with previews
void 
CPagePreview::DoSubscribeNotifications(HWND pHwnd) {

	oCore->oNotifications->Subscribe(pHwnd, eNotificationPrintSettingsChanged);
	oCore->oNotifications->Subscribe(pHwnd, eNotificationDocumentPrintingDone);
	oCore->oNotifications->Subscribe(pHwnd, eNotificationPrintDeviceChanged);
	}

//This function handles the core notifications
bool 
CPagePreview::OnEventCoreNotification(HWND phWnd, sCoreNotification pcNotification) {

	bool bReturn;

	bReturn = false;

	switch (pcNotification.eNotification) {

		case eNotificationDocumentPrintingDone:
			
			sPrintDocumentWithSettings* oPrintDocument;

			oPrintDocument = (sPrintDocumentWithSettings*)pcNotification.pData;
			
			if (iPrintTicket == oPrintDocument->iPrintRequest) {
			
				delete oClientBitmap;
				oClientBitmap = NULL;

				delete oPrintDocument;

				Redraw(phWnd);
				}

			bReturn = true;
			break;

		case eNotificationPrintDeviceChanged:
		case eNotificationPrintSettingsChanged:

			DoClearPreview();

			oCore->oNotifications->RemoveNotificationSpecific(phWnd, eNotificationPrintSettingsChanged);
			oCore->oNotifications->RemoveNotificationSpecific(phWnd, eNotificationPrintDeviceChanged);

			Redraw(phWnd);

			bReturn = true;	
			break;
		}

	return bReturn;
	}

//Set the document_id of the document to preview
void
CPagePreview::SetPreviewDocument(int piDocumentID, CPrintSettings* poPrintPreviewSettings) {

	if (iDocumentID != piDocumentID) {
		
		iDocumentID = piDocumentID;
		oPrintPreviewSettings = poPrintPreviewSettings;
		if (oPrintPreviewSettings==NULL) oPrintPreviewSettings = oCore->oPrintSettings;

		iPrintTicket = -1;
		//DoCreatePreview();
		}
	}

//This function invalidates the viewer
void 
CPagePreview::Redraw(HWND phWnd) {

	InvalidateRect(phWnd, NULL, false);
	}

//This function clear the preview
void 
CPagePreview::DoClearPreview() {

	if (oPreviewBitmap!=NULL) {
		
		delete oPreviewBitmap;
		oPreviewBitmap = NULL;
		}
	iPrintTicket = -1;

	//iDocumentID = -1;
	}

//This function creates the bitmap to fill the preview control with
void
CPagePreview::CreateDoubleBuffer(HWND phWnd) {

	RECT	cPreviewRect;
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	GetWindowRect(phWnd, &cPreviewRect);
	if (oClientBitmap) delete oClientBitmap;
	oClientBitmap = new Bitmap(cPreviewRect.right - cPreviewRect.left, 
						 cPreviewRect.bottom - cPreviewRect.top, PixelFormat32bppRGB);

	DoubleBufferDrawBackground();
	DoubleBufferDrawPage();

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function draw the background of the preview window
void 
CPagePreview::DoubleBufferDrawBackground() {

	Graphics*	oCanvas;
	Rect		cRectPreview;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	cRectPreview.Height = oClientBitmap->GetHeight();
	cRectPreview.Width  = oClientBitmap->GetWidth();
	cRectPreview.X = 0;
	cRectPreview.Y = 0;
	oCanvas = Graphics::FromImage(oClientBitmap);

	//Draw the background
	//Pen oBorderPen(Color(255, 64, 64, 64));
	//oCanvas->DrawRectangle(&oBorderPen, cRectPreview);
	//
	//cRectPreview.Inflate(-1, -1);

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
CPagePreview::DoubleBufferDrawPage() {

	Graphics*	oCanvas;
	RectF		cRectPreview;
	sOutputDevice oOutputDevice;
	CPrintSettings* oTemporarySettings;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oPrintPreviewSettings->GetDevice(oOutputDevice);
	oTemporarySettings = oPrintPreviewSettings->Copy();
	oTemporarySettings->bNotifyChanges = false;
	oTemporarySettings->SetBool(ePrintMultiPagesOnPage, false);
	cRectPreview = oCore->oPrintCore->GetStandardPageImageArea(oOutputDevice, oTemporarySettings);
	delete oTemporarySettings;

	float fTranslation;

	fPreviewDpiX = (oClientBitmap->GetWidth()-40) / cRectPreview.Width;
	fPreviewDpiY = (oClientBitmap->GetHeight()-40) / cRectPreview.Height;
	fTranslation = fPreviewDpiX;
	if (fPreviewDpiX > fPreviewDpiY) fTranslation = fPreviewDpiY; 

	fPreviewDpiX = fPreviewDpiY = fTranslation;

	cPageRect = Rect(0, 0, (INT)(cRectPreview.Width * fTranslation),
						   (INT)(cRectPreview.Height * fTranslation));
	cPageRect.Offset((oClientBitmap->GetWidth() - cPageRect.Width) / 2,
				     (oClientBitmap->GetHeight() - cPageRect.Height) / 2);

	
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
	oCanvas = Graphics::FromImage(oClientBitmap);
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
	else {

		if (iPrintTicket == -1) {

			DoCreatePreview();
			}
		}

	cPageRect.X = 0;
	cPageRect.Y = 0;

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function requests the printqueue to 'print' the document
//to a bitmap
void
CPagePreview::DoCreatePreview() {

	CCopyDocument*	oDocument;
	CCopyDocumentPage* oPage;
	CPrintSettings*	oTemporarySettings;
	sOutputDevice oOutputDevice;
	Graphics* oCanvas;
	Bitmap*	  oPageBitmap;
	long	lPageNumber;
	
	oTrace->StartTrace(__WFUNCTION__);

	oDocument = oCore->oDocuments->GetDocumentByID(iDocumentID);
	oPage     = oCore->GetCurrentPage();

	if (oDocument!=NULL && oPage!=NULL) {

		lPageNumber = oDocument->GetPageIndex(oPage->GetPageID());


		//Create a new previewbitmap
		if (oPreviewBitmap!=NULL) delete oPreviewBitmap;

		oPageBitmap = new Bitmap(cPageRect.Width, cPageRect.Height, PixelFormat32bppRGB);
		oPageBitmap->SetResolution(fPreviewDpiX, fPreviewDpiY);

		SolidBrush oWhiteBrush(Color(255, 255, 255, 255));
		oCanvas = new Graphics(oPageBitmap);
		oCanvas->FillRectangle(&oWhiteBrush, (REAL)0, (REAL)0, (REAL)cPageRect.Width, (REAL)cPageRect.Height);
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

			lPageNumber = lPageNumber - (lPageNumber%4);

			oTemporarySettings->oPrintData.lPage.push_back(lPageNumber);
			oTemporarySettings->oPrintData.lPage.push_back(lPageNumber+1);
			oTemporarySettings->oPrintData.lPage.push_back(lPageNumber+2);
			oTemporarySettings->oPrintData.lPage.push_back(lPageNumber+3);
			}
		else {

			//Only print one page
			oTemporarySettings->oPrintData.lPage.clear();
			oTemporarySettings->oPrintData.lPage.push_back(lPageNumber);
			}

		//Add it to the print queue
		iPrintTicket = oDocument->PrintThread(oTemporarySettings);
		}

	oTrace->EndTrace(__WFUNCTION__);
	}