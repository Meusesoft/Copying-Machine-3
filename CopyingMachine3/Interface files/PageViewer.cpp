#include "StdAfx.h"
#include "PageViewer.h"

CPageViewer::CPageViewer(sGlobalInstances pInstances,
						 std::wstring psWindowName) : CPagePreview(pInstances), 
													  CWindowScrollbar(pInstances, psWindowName)
{
	hWnd = NULL;
	hWndTooltip = NULL;
	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;
	oRegistry = pInstances.oRegistry;
	oNotifications = (CCoreNotifications*)pInstances.oNotifications;
	hInstance = pInstances.hInstance;

	oMySettings		= CreateSettings(0);
	oSettings		= oMySettings;
	oCachedBitmap	= NULL;
	oClientBitmap	= NULL;
	oNewCachedBitmap = NULL;

	bEndThread = !InitializeCriticalSectionAndSpinCount(&ThreadCriticalSection, 0x80000400);
	hCachedBitmapThread = NULL;
	hSelectionGripImages = NULL;

	oMouseState.cMouseState = eMouseStateStandard;
	cPageViewType = ePVTImage;
	bRecognitionLayerVisible = (oRegistry->ReadInt(L"General", L"LayerRecognition", 1)==1);

}

CPageViewer::~CPageViewer(void)
{
	Destroy();
}

//Create the window for the page viewer
HWND CPageViewer::Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra) {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__);

	hInstance = phInstance;

	DWORD dwStyle = WS_CHILD |
					WS_VISIBLE |
					WS_CLIPCHILDREN | 
					WS_CLIPSIBLINGS |
					//WS_BORDER |
					pdStyleExtra;

	//if (bUseCustomControlView) dwStyle = dwStyle & ~WS_BORDER;

	hWnd = CreateWindowEx(0L, 						// ex style
						 L"MeusesoftPageViewer",	// class name
						 L"MeusesoftPageViewer",	// window text
						 dwStyle,                   // style
						 0,                         // x position
						 30,                        // y position
						 300,                       // width
						 300,                       // height
						 phParent,					// parent
						 (HMENU)1010,				// ID
						 hInstance,                 // instance
						 NULL);                     // no extra data

	oCore->oNotifications->Subscribe(hWnd, eNotificationPageViewChanges);
	oCore->oNotifications->Subscribe(hWnd, eNotificationPageUpdate);
	DoSubscribeNotifications(hWnd); //for page preview

	InitializeScrollInfo();

	//Create a thread for making cachedbitmaps. Only if the critical section
	//is initialized
	if (!bEndThread) {

		hCachedBitmapThread = CreateThread(
			NULL,				// default security attributes
			0,					// use default stack size  
			&ThreadCreateCachedImage,        // thread function 
			this,				// argument to thread function 
			0,					// use default creation flags 
			NULL);				// returns the thread identifier 
 
			}

	//Load the cursors
	hCursorArrow	 = LoadCursor(NULL, IDC_ARROW);
	hCursorSizeAll   = LoadCursor(NULL, IDC_SIZEALL);
	hCursorSizeNS    = LoadCursor(NULL, IDC_SIZENS);
	hCursorSizeWE    = LoadCursor(NULL, IDC_SIZEWE);
	hCursorSizeNWSE  = LoadCursor(NULL, IDC_SIZENWSE);
	hCursorSizeNESW  = LoadCursor(NULL, IDC_SIZENESW);
	hCursorHand	     = LoadCursor(NULL, IDC_HAND);

	oSelectionGripImages.push_back(CBitmap::LoadFromResource(L"PNG_GRIPN", L"PNG", CWindowBase::oGlobalInstances.hInstance));
	oSelectionGripImages.push_back(CBitmap::LoadFromResource(L"PNG_GRIPE", L"PNG", CWindowBase::oGlobalInstances.hInstance));
	oSelectionGripImages.push_back(CBitmap::LoadFromResource(L"PNG_GRIPS", L"PNG", CWindowBase::oGlobalInstances.hInstance));
	oSelectionGripImages.push_back(CBitmap::LoadFromResource(L"PNG_GRIPW", L"PNG", CWindowBase::oGlobalInstances.hInstance));
	oSelectionGripImages.push_back(CBitmap::LoadFromResource(L"PNG_GRIPNW", L"PNG", CWindowBase::oGlobalInstances.hInstance));
	oSelectionGripImages.push_back(CBitmap::LoadFromResource(L"PNG_GRIPNE", L"PNG", CWindowBase::oGlobalInstances.hInstance));
	oSelectionGripImages.push_back(CBitmap::LoadFromResource(L"PNG_GRIPSE", L"PNG", CWindowBase::oGlobalInstances.hInstance));
	oSelectionGripImages.push_back(CBitmap::LoadFromResource(L"PNG_GRIPSW", L"PNG", CWindowBase::oGlobalInstances.hInstance));

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
	}

//Destroy the window of the splitter
void CPageViewer::Destroy() {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__);

	bEndThread = true;

	if (hWnd!=NULL) DestroyWindow(hWnd);
	if (oClientBitmap!=NULL) delete oClientBitmap;
	ClearAllSettings();
	ClearCachedBitmap();
	oHotSpots.clear();

	//Clean up the thread
	if (hCachedBitmapThread!=NULL) {
		
		//Wait for the thread to finish.
		WaitForSingleObject(hCachedBitmapThread, INFINITE);
	
		CloseHandle(hCachedBitmapThread);

		DeleteCriticalSection(&ThreadCriticalSection);
		}

	vToBeProcessed.clear();

	if (oNewCachedBitmap!=NULL) oNewCachedBitmap; //delete this after the thread is finished

	//Clear up the grip images
	long lIndex;

	lIndex = oSelectionGripImages.size();

	while(lIndex>0) {
		
		lIndex--;
		delete oSelectionGripImages[lIndex];
		}
	oSelectionGripImages.clear();

	//Clean up cursors (not necessary, loadcursor load a shared resource)
	//DestroyCursor(hCursorArrow);
	//DestroyCursor(hCursorSizeAll);
	//DestroyCursor(hCursorSizeNS);
	//DestroyCursor(hCursorSizeWE);
	//DestroyCursor(hCursorSizeNWSE);
	//DestroyCursor(hCursorSizeNESW);

	CWindowBase::oTrace->EndTrace(__WFUNCTION__);
	}

//This function invalidates the viewer
void 
CPageViewer::Redraw() {

	InvalidateRect(hWnd, NULL, false);
	}


//Retrieve the current settings
sPageViewerSettings* 
CPageViewer::GetSettings(DWORD pdPageID) {

	sPageViewerSettings* oResult;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);
	CWindowBase::oTrace->Add(L"PageID", (int)pdPageID, eAll);

	oResult = NULL;

	//search the vector for the settings
	for (long iIndex=0; iIndex<(long)oPageSettings.size() && oResult==NULL; iIndex++) {

		if (oPageSettings[iIndex]->dPageID == pdPageID) {

			oResult = oPageSettings[iIndex];
			}
		}

	//if nothing found, make a new one
	if (oResult==NULL) {

		oResult = CreateSettings(pdPageID);
		}
	
	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);

	return oResult;
	}

//This function creates a new settings object with default settings
sPageViewerSettings* 
CPageViewer::CreateSettings(DWORD pdPageID) {

	sPageViewerSettings* oResult;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);
	CWindowBase::oTrace->Add(L"PageID", (int)pdPageID, eAll);

	oResult = new sPageViewerSettings();
	oResult->dPageID = pdPageID;
	oResult->fMagnification = 1.0f;
	oResult->fPosX = 0.0f;
	oResult->fPosY = 0.0f;
	oResult->bSizeToFit = true;

	//save it to the vector
	oPageSettings.push_back(oResult);

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);

	return oResult;
	}

//Set the pageview settings
bool 
CPageViewer::SetSettings(sPageViewerSettings* pcSettings) {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	oSettings = pcSettings;

	Redraw();

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);

	return true;
	}

//This function clear a specific settings in the vector
bool 
CPageViewer::ClearSettings(DWORD pdPageID) {

	bool bResult;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);
	CWindowBase::oTrace->Add(L"PageID", (int)pdPageID, eAll);

	bResult = false;

	for (int iIndex=0; iIndex<(long)oPageSettings.size() && !bResult; iIndex++) {
		
		if (oPageSettings[iIndex]->dPageID == pdPageID) {
		
			bResult = true;
			delete oPageSettings[iIndex];
			}
		}
	oPageSettings.clear();
	
	CWindowBase::oTrace->EndTrace(__WFUNCTION__, bResult, eAll);

	return bResult;
}

//This function clears all the settings from the vector
bool 
CPageViewer::ClearAllSettings() {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	for (int iIndex=0; iIndex<(int)oPageSettings.size(); iIndex++) {
		delete oPageSettings[iIndex];
		}
	oPageSettings.clear();
	
	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);

	return true;
}

//Set the view type
void 
CPageViewer::SetPageViewType(ePageViewType pcNewType) {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__);

	if (pcNewType!=cPageViewType) {

		cPageViewType = pcNewType;
		ProcessUpdatedSettings();
		Redraw();
		ClearMouseState();
		}

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, (int)pcNewType);
	}

//Get the view type
ePageViewType 
CPageViewer::GetPageViewType() {

	return cPageViewType;
	}


//Get the magnification
float 
CPageViewer::GetMagnification() {

	float fResult;
	
	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	fResult = oSettings->fMagnification;

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);

	return fResult;
	}

void 
CPageViewer::SetMagnification(float pfMagnification) {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	//update and validate the updated magnification
	oSettings->fMagnification = pfMagnification;

	if (oSettings->fMagnification<=0.1f) oSettings->fMagnification = 0.1f;
	if (oSettings->fMagnification>2.0f) oSettings->fMagnification = 2.0f;
	oSettings->bSizeToFit = false;

	//process the update inthe viewer.
	ProcessUpdatedSettings();

	//communicatie the change in the magnification
	oCore->oNotifications->CommunicateEvent(eNotificationUpdateControls, 0);

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function sets the magnification so that the current selection is maximized
//in the view.
void 
CPageViewer::SetMagnificationToSelection() {

	CCopyDocumentPage*		oPage;
	sCopyDocumentSelection	cSelection;
	RECT cClientRect;
	float fMagnificationV, fMagnificationH;
	sPageViewerPosition cPosition;
	POINT cClientPosition;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		oPage->GetSelection(cSelection);

		//recalculate the size to fit
		GetClientRect(hWnd, &cClientRect);

		fMagnificationV = (float)cClientRect.bottom / (cSelection.fBottom - cSelection.fTop);
		fMagnificationH = (float)cClientRect.right  / (cSelection.fRight - cSelection.fLeft);
		
		SetMagnification(min(fMagnificationH, fMagnificationV) * 0.97f);

		//we do it twice to because only now we can adjust for some added scrollbars
		//in the viewer.
		GetClientRect(hWnd, &cClientRect);

		fMagnificationV = (float)cClientRect.bottom / (cSelection.fBottom - cSelection.fTop);
		fMagnificationH = (float)cClientRect.right  / (cSelection.fRight - cSelection.fLeft);
		
		SetMagnification(min(fMagnificationH, fMagnificationV) * 0.97f);

		//move the viewer to the center of the selection
		cClientPosition.x = cClientRect.left + (cClientRect.right  - cClientRect.left) / 2;
		cClientPosition.y = cClientRect.top + (cClientRect.bottom - cClientRect.top) / 2;

		cPosition = GetImagePosition(cClientPosition);

		cPosition.cImagePoint.x = (long)(cSelection.fLeft + (cSelection.fRight - cSelection.fLeft)/2); 
		cPosition.cImagePoint.y = (long)(cSelection.fTop + (cSelection.fBottom - cSelection.fTop)/2); 

		SetImagePosition(cPosition);
		}

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function selects the complete image
void 
CPageViewer::SelectionAll() {

	CCopyDocumentPage*		oPage;
	sCopyDocumentSelection	cSelection;
	Bitmap*					oBitmap;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		oPage->GetSelection(cSelection);
		oBitmap = oPage->GetImage()->GetBitmap();
		
		cSelection.bEnabled = true;
		cSelection.bOuter = false;

		cSelection.fLeft = 0;
		cSelection.fTop = 0;
		cSelection.fRight = (float)oBitmap->GetWidth();
		cSelection.fBottom = (float)oBitmap->GetHeight();

		oPage->SetSelection(cSelection);

		Redraw();

		SetCursor(hCursorSizeAll);
		oMouseState.cMouseState = eMouseStateStandard;
		}

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function disables the selection
void 
CPageViewer::SelectionClear() {

	CCopyDocumentPage*		oPage;
	sCopyDocumentSelection	cSelection;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		oPage->GetSelection(cSelection);
		
		cSelection.bEnabled = false;
		cSelection.bOuter = true;

		oPage->SetSelection(cSelection);

		SetCursor(hCursorArrow);
		oMouseState.cMouseState = eMouseStateStandard;
		ReleaseCapture();
	}

	Redraw();

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function changes the selection from inner to outer and visa versa
void 
CPageViewer::SelectionSwap() {
	
	CCopyDocumentPage*		oPage;
	sCopyDocumentSelection	cSelection;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		oPage->GetSelection(cSelection);
		
		cSelection.bOuter = !cSelection.bOuter;

		oPage->SetSelection(cSelection);
		}

	Redraw();

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function adjust the selection so it contains all the content on the page. The outer
//part of the selection doesn't contain any viable information
void 
CPageViewer::SelectContent() {

	CCopyDocumentPage*		oPage;
	sCopyDocumentSelection	cSelection;
	RectF					cRectangle;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		oPage->GetSelection(cSelection);

		cRectangle = oPage->GetImage()->DetermineContentRectangleBitmap();

		cSelection.fLeft	= cRectangle.X;
		cSelection.fTop		= cRectangle.Y;
		cSelection.fRight	= cRectangle.X + cRectangle.Width;
		cSelection.fBottom	= cRectangle.Y + cRectangle.Height ;
		cSelection.bEnabled = true;
		cSelection.bOuter	= true;

		oPage->SetSelection(cSelection);
		}

	Redraw();

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}

void
CPageViewer::SetSizeToFit() {

	RECT cClientRect;
	Bitmap* oBitmap;
	CCopyDocumentPage* oPage;
	float fMagnificationV, fMagnificationH;
	
	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	//recalculate the size to fit
	GetClientRect(hWnd, &cClientRect);

	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		oBitmap = oPage->GetImage()->GetBitmap();

		fMagnificationV = (float)cClientRect.bottom / (float)oBitmap->GetHeight();
		fMagnificationH = (float)cClientRect.right / (float)oBitmap->GetWidth();
		
		SetMagnification(min(fMagnificationH, fMagnificationV) * 0.97f);		
		}
	
	oSettings->bSizeToFit = true;

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//Check if size to fit is set.
bool
CPageViewer::GetSizeToFit() {

	return 	oSettings->bSizeToFit;
	}

//Get the position of the image in the client
sPageViewerPosition 
CPageViewer::GetImagePosition(POINT pcClientPosition) {

	sPageViewerPosition cResult;
	
	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	cResult.cClientPoint = pcClientPosition;

	cResult.cImagePoint.x = (long)((oSettings->fPosX + pcClientPosition.x) / oSettings->fMagnification);
	cResult.cImagePoint.y = (long)((oSettings->fPosY + pcClientPosition.y) / oSettings->fMagnification);

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);

	return cResult;
	}


//Set the position of the image in the client
void 
CPageViewer::SetImagePosition(sPageViewerPosition pcPosition) {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	oSettings->fPosX = max(0, (pcPosition.cImagePoint.x * oSettings->fMagnification) - pcPosition.cClientPoint.x);
	oSettings->fPosY = max(0, (pcPosition.cImagePoint.y * oSettings->fMagnification) - pcPosition.cClientPoint.y);

	ProcessUpdatedSettings();

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}



//this function is the message handler for the pageviewer
bool CPageViewer::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	bool bReturn;

	bReturn = false;

	switch (message) {

		case WM_ERASEBKGND:

			if (phWnd == this->hWnd) {
				bReturn = true;
				piReturn = 1; //we erased the background... NOT!
				}
			break;

		case WM_SIZE:

			if (phWnd == this->hWnd) {
				if (oSettings->bSizeToFit) SetSizeToFit();
				ProcessUpdatedSettings();
				}
			break;

		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:

			if (phWnd == this->hWnd && cPageViewType==ePVTImage) { //only handle an event when it is for our window
				bReturn = OnEventMouse(message, wParam, lParam);

				if (bReturn) piReturn = 0;
				}
			break;

		case WM_MOUSEWHEEL:
			
			if (phWnd == this->hWnd && cPageViewType==ePVTImage) bReturn = OnEventMouseWheel(wParam, lParam);
			break;

		case WM_PAINT:

			if (phWnd == this->hWnd) {

				DrawClient();
				}
			break;

		//handle the notification from the core
		case WM_CORENOTIFICATION:

			if (phWnd == hWnd) {

				bReturn = OnEventCoreNotification();
				}
			break;

		default: 

			bReturn = CWindowScrollbar::WndProc(phWnd, message, wParam, lParam, piReturn);
			break;
			}


	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);

	return bReturn;
	}

//This function is called when a new image has been selected to be viewed.
void 
CPageViewer::NewPage() {

	CCopyDocumentPage*	oPage;
	CCopyDocument*		oDocument;
	sPageViewerSettings* oPageSettings;

	//Get or create new settings
	oPage = oCore->GetCurrentPage();
	if (oPage!=NULL) {

		oPageSettings = GetSettings(oPage->GetPageID());
		SetSettings(oPageSettings);
		}

	oDocument = oCore->GetCurrentDocument();
	if (oDocument!=NULL) {

		SetPreviewDocument(oDocument->GetDocumentID(), NULL);
		}

	//Process it
	ClearCachedBitmap();
	DoClearPreview();
	if (oSettings->bSizeToFit) SetSizeToFit();

	Redraw();
	}

//This function changes the state of the viewer to a color picking mode
void 
CPageViewer::SetColorPicker() {

	if (oMouseState.cMouseState == eMouseStateStandard) {

		oMouseState.cMouseState = eMouseStateColorPicker;
		}
	}


//This function processed the update in the settings
void 
CPageViewer::ProcessUpdatedSettings() {

	CCopyDocumentPage* oPage;
	Bitmap* oDocumentBitmap;
	RECT cClientRect;
	SCROLLINFO cNewScrollInfo;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	int iImageWidth		= 0;
	int iImageHeight	= 0;
	int iClientWidth	= 0;
	int iClientHeight	= 0;

	//Clear caches
	//ClearCachedBitmap();

	//Retrieve image dimensions
	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL && cPageViewType!=ePVTPrintPreview) {

		int iWidth;
		int iHeight;

		oDocumentBitmap = oPage->GetImage()->GetBitmap();

		iWidth = oDocumentBitmap->GetWidth();
		iHeight = oDocumentBitmap->GetHeight();

		if (iWidth==0 || iHeight==0) { //Dirty workaround for a ObjectBusy status
			
			Sleep(50);

			iWidth = oDocumentBitmap->GetWidth();
			iHeight = oDocumentBitmap->GetHeight();
			}

		iImageWidth = (int)(iWidth * oSettings->fMagnification);
		iImageHeight = (int)(iHeight * oSettings->fMagnification);
		}

	//Retrieve client dimensions
	GetClientRect(hWnd, &cClientRect);

	iClientWidth = (int)cClientRect.right;
	iClientHeight = (int)cClientRect.bottom;

	//Process vertical scrollbar
	cNewScrollInfo.nPage = iClientHeight;
	cNewScrollInfo.nMax = iClientHeight;

	if (iImageHeight > iClientHeight) {

		if (oSettings!=NULL) {
			if (oSettings->fPosY < 0) oSettings->fPosY = 0;
			cSBIVertical.nPos = (int)oSettings->fPosY;
			}

		cNewScrollInfo.nMin = 0;
		cNewScrollInfo.nMax = iImageHeight-1;
		cNewScrollInfo.nPage = iClientHeight;
		cNewScrollInfo.nPos = cSBIVertical.nPos;

		if (cNewScrollInfo.nPos + (int)cNewScrollInfo.nPage > cNewScrollInfo.nMax) {
			cNewScrollInfo.nPos = cNewScrollInfo.nMax - cNewScrollInfo.nPage;
			oSettings->fPosY = (float)cNewScrollInfo.nPos;
			Redraw();
			}
		}
	else {

		if (oSettings!=NULL && cPageViewType!=ePVTPrintPreview) oSettings->fPosY = 0;
		Redraw();
		}
	
	UpdateScrollbar(SB_VERT, cNewScrollInfo);

	//Process horizontal scrollbar
	cNewScrollInfo.nPage = iClientWidth;
	cNewScrollInfo.nMax = iClientWidth;

	if (iImageWidth > iClientWidth) {

		if (oSettings!=NULL) {
			if (oSettings->fPosX < 0) oSettings->fPosX = 0;

			cSBIHorizontal.nPos = (int)oSettings->fPosX;
			}
	
		cNewScrollInfo.nMin = 0;
		cNewScrollInfo.nMax = iImageWidth-1;
		cNewScrollInfo.nPage = iClientWidth;
		cNewScrollInfo.nPos = cSBIHorizontal.nPos;

		if (cNewScrollInfo.nPos + (int)cNewScrollInfo.nPage > cNewScrollInfo.nMax) {
			cNewScrollInfo.nPos = cNewScrollInfo.nMax - cNewScrollInfo.nPage;
			oSettings->fPosX = (float)cNewScrollInfo.nPos;
			Redraw();
			}
		}
	else {

		if (oSettings!=NULL && cPageViewType!=ePVTPrintPreview) oSettings->fPosX = 0;
		Redraw();
		}
	
	UpdateScrollbar(SB_HORZ, cNewScrollInfo);

	//Make it so... on the screen
	Redraw();

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}


//This function creates a cached bitmap. A cached bitmap is the image of the page
//in the same pixelformat as the screen. The cached bitmap is also magnified if
//magnification < 1.0f. Both properties of the cached bitmap speeds up drawing to
//the screen.
Bitmap* 
CPageViewer::CreateCachedBitmap(Bitmap* poBitmap, float pfMagnification, Graphics* poCanvas) {

	Bitmap* oMagnifiedBitmap;
	Graphics* oBitmapCanvas;

	oMagnifiedBitmap = new Bitmap((int)(poBitmap->GetWidth() * pfMagnification),
									(int)(poBitmap->GetHeight() * pfMagnification),
									poCanvas);
	oBitmapCanvas = Graphics::FromImage(oMagnifiedBitmap);
	
	RectF SourceRect		= RectF(0, 0, (REAL)poBitmap->GetWidth(), (REAL)poBitmap->GetHeight());
	RectF DestinationRect	= RectF(0, 0, poBitmap->GetWidth() * pfMagnification,
								  poBitmap->GetHeight() * pfMagnification);

	oBitmapCanvas->SetInterpolationMode(InterpolationModeNearestNeighbor);
	if (pfMagnification < 1.0f) oBitmapCanvas->SetInterpolationMode(InterpolationModeHighQuality);
	Gdiplus::Status cStatus;
	
	cStatus = oBitmapCanvas->DrawImage(poBitmap, DestinationRect, SourceRect.X, SourceRect.Y, SourceRect.Width, SourceRect.Height, UnitPixel);

	if (cStatus != Ok) {
		::MessageBox(NULL, L"Error CreateCachedBitmap", L"Error", MB_OK);
		}

	delete oBitmapCanvas;

	return oMagnifiedBitmap;
	}


//This function handles the request for a cached bitmap. If none exists
//it will create one on its own or else it will do a request to the tread
void 
CPageViewer::RequestCachedBitmap(Bitmap* poBitmap, Graphics* poCanvas) {

	sCachedImageRequest cRequest;

	//check if we received a new cached bitmap
	EnterCriticalSection(&ThreadCriticalSection); 

	if (oNewCachedBitmap!=NULL) {

		oCachedBitmap = oNewCachedBitmap;
		oNewCachedBitmap = NULL;
		fMagnificationCachedBitmap = fMagnificationNewCachedBitmap;
		}

	LeaveCriticalSection(&ThreadCriticalSection); 
	
	//check if the cached bitmap is stil valid
	if (oSettings->fMagnification != fMagnificationCachedBitmap ||
		oCachedBitmap == NULL) {

		if (oCachedBitmap!=NULL) {

			//there is a cachedbitmap available. Request the thread
			//for an update
			cRequest.dRequestedOnTick = GetTickCount();
			cRequest.fMagnification = oSettings->fMagnification;
			cRequest.oImage = poBitmap;

			EnterCriticalSection(&ThreadCriticalSection); 

			vToBeProcessed.push_back(cRequest);
			
			LeaveCriticalSection(&ThreadCriticalSection); 
			}
		else {

			//we need a cachedbitmap and we don't have one now. Create
			//it immediately.
			oCachedBitmap = CreateCachedBitmap(poBitmap, oSettings->fMagnification, poCanvas); 

			fMagnificationCachedBitmap = oSettings->fMagnification;
			}
		}
	}

//Clears/deletes the cached bitmap
void
CPageViewer::ClearCachedBitmap() {

	if (oCachedBitmap!=NULL) {
		
		delete oCachedBitmap;
		oCachedBitmap=NULL;
		}
	}

//This function creates a client bitmap. This bitmap is used as a double buffer
void 
CPageViewer::CreateClientBitmap(Graphics* poCanvas) {

	RECT cClientRect;
	RectF ClientRect;

	//get the dimensions of the client rect
	GetClientRect(hWnd, &cClientRect);
	ClientRect = RectF(0, 0, (REAL)cClientRect.right, (REAL)cClientRect.bottom);

	//test if the size of the client has changed
	if (oClientBitmap!=NULL) {

		if (oClientBitmap->GetWidth()!=ClientRect.Width || 
			oClientBitmap->GetHeight()!=ClientRect.Height) {

			ClearClientBitmap();
			}
		}

	//Test if we need to create a new client bitmap
	if (oClientBitmap==NULL) {
	
		oClientBitmap = new Bitmap((int)ClientRect.Width, (int)ClientRect.Height, poCanvas);
		}
	}

//This function deletes/clears the client bitmap
void 
CPageViewer::ClearClientBitmap() {

	if (oClientBitmap!=NULL) {

		delete oClientBitmap;
		oClientBitmap = NULL;
		}
	}

//This functions handles the drawing to the client
void 
CPageViewer::DrawClient() {

	PAINTSTRUCT cPaintStruct;
	HDC hDC;
	Graphics* oCanvas;

	//Start drawing
	SecureZeroMemory(&cPaintStruct, sizeof(cPaintStruct));
	hDC = BeginPaint(hWnd, &cPaintStruct);

	//Create a canvas for drawing to the screen
	oCanvas = new Graphics(hDC);

	//Get the client bitmap for double buffering
	CreateClientBitmap(oCanvas);

	//Do the actual drawing
	switch (cPageViewType) {

		case ePVTPrintPreview:

			DrawPrintPreview(Graphics::FromImage(oClientBitmap));
			break;

		default:

			DrawImage(Graphics::FromImage(oClientBitmap));
			break;
		}

	//Draw the buffer to the client	
	oCanvas->DrawImage(oClientBitmap, 0, 0);

	//Clean up
	delete oCanvas;
	
	//End drawing
	EndPaint(hWnd, &cPaintStruct);
	}


//This function draws the current image to the screen
void 
CPageViewer::DrawImage(Graphics* oCanvas) {

	CCopyDocumentPage* oPage;
	Bitmap* oBitmap;
	RECT cClientRect;
	RectF SourceRect;
	RectF DestinationRect;
	//RectF BackgroundRect;
	RectF ClientRect;

	//SolidBrush oBackgroundBrush(Color(255, defBackgroundR, defBackgroundG, defBackgroundB));
	

	GetClientRect(hWnd, &cClientRect);
	ClientRect = RectF(0, 0, (REAL)cClientRect.right, (REAL)cClientRect.bottom);

	DoubleBufferDrawBackground();

	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		oBitmap = oPage->GetImage()->GetBitmap();

		RequestCachedBitmap(oBitmap, oCanvas);

		SourceRect = RectF(oSettings->fPosX / oSettings->fMagnification * fMagnificationCachedBitmap, oSettings->fPosY / oSettings->fMagnification * fMagnificationCachedBitmap, (REAL)oCachedBitmap->GetWidth(), (REAL)oCachedBitmap->GetHeight());
		DestinationRect = RectF(0, 0, oBitmap->GetWidth() * oSettings->fMagnification,
									  oBitmap->GetHeight() * oSettings->fMagnification);

		if (ClientRect.Width > DestinationRect.Width) {

			DestinationRect.Offset((ClientRect.Width - DestinationRect.Width)/2, 0);	
			}
		if (ClientRect.Height > DestinationRect.Height) {

			DestinationRect.Offset(0, (ClientRect.Height - DestinationRect.Height)/2);	
			}

		oCanvas->SetInterpolationMode(InterpolationModeNearestNeighbor);
		oCanvas->DrawImage(oCachedBitmap, DestinationRect, SourceRect.X, SourceRect.Y, SourceRect.Width, SourceRect.Height, UnitPixel);
		//oCanvas->DrawImage(oBitmap, DestinationRect, SourceRect.X, SourceRect.Y, SourceRect.Width, SourceRect.Height, UnitPixel);
		//oCanvas->DrawImage(oCachedBitmap, DestinationRect, oSettings->fPosX, oSettings->fPosY, DestinationRect.Width, DestinationRect.Height, UnitPixel);
		  
		//Draw the background around the image
		//if (DestinationRect.X > 0) {

		//	BackgroundRect = RectF(0, 0, DestinationRect.X, ClientRect.Height);
		//	oCanvas->FillRectangle(&oBackgroundBrush, BackgroundRect);
		//	}
		//if (DestinationRect.Y > 0) {

		//	BackgroundRect = RectF(DestinationRect.X, 0, DestinationRect.X + DestinationRect.Width, DestinationRect.Y);
		//	oCanvas->FillRectangle(&oBackgroundBrush, BackgroundRect);
		//	}
		//if (ClientRect.Width > DestinationRect.Width + DestinationRect.X) {
	
		//	BackgroundRect = RectF(DestinationRect.Width + DestinationRect.X, 0, ClientRect.Width - DestinationRect.Width - DestinationRect.X, ClientRect.Height);
		//	oCanvas->FillRectangle(&oBackgroundBrush, BackgroundRect);
		//	}
		//if (ClientRect.Height > DestinationRect.Height + DestinationRect.Y) {
	
		//	BackgroundRect = RectF(DestinationRect.X, DestinationRect.Height  + DestinationRect.Y, DestinationRect.Width  + DestinationRect.X, ClientRect.Height - DestinationRect.Height - DestinationRect.Y);
		//	oCanvas->FillRectangle(&oBackgroundBrush, BackgroundRect);
		//	}

		//Draw the selection
		oCanvas->SetInterpolationMode(InterpolationModeNearestNeighbor);
		DrawSelection(oCanvas, DestinationRect);
		
		DrawRecognitionLayer(oCanvas);

		//wchar_t szSettings[250];

		//swprintf(szSettings, 250, L"PosX: %d  PosY: %d\nImageW: %d, ImageH: %d", (int)(oSettings->fPosX), (int)(oSettings->fPosY), (int)(oBitmap->GetWidth() *  oSettings->fMagnification), (int)(oBitmap->GetHeight() * oSettings->fMagnification));

		//// Initialize arguments.
		//Font myFont(L"Arial", 16);
		//PointF origin(0.0f, 0.0f);
		//SolidBrush blackBrush(Color(255, 0, 0, 0));

		//oCanvas->DrawString(szSettings, wcslen(szSettings), &myFont, origin, &blackBrush);
		}
	else {

		//oCanvas->FillRectangle(&oBackgroundBrush, ClientRect);
		oHotSpots.clear();
		ClearMouseState();
		}
	}

//This function draw the current document as a print preview
void 
CPageViewer::DrawPrintPreview(Graphics* oCanvas) {

	//RECT cClientRect;
	//RectF ClientRect;

	//SolidBrush oBackgroundBrush(Color(255, defBackgroundR, 0, defBackgroundB));
	//
	//GetClientRect(hWnd, &cClientRect);
	//ClientRect = RectF(0, 0, (REAL)cClientRect.right, (REAL)cClientRect.bottom);
			
	//oCanvas->FillRectangle(&oBackgroundBrush, ClientRect);

	DoubleBufferDrawBackground();
	DoubleBufferDrawPage();
}

//This function draws the recognition layer onto the canvas
void 
CPageViewer::DrawRecognitionLayer(Graphics* oCanvas) {
	
	CRecognitionLayer*	oLayer;
	CRecognizedText*	oText;
	CCopyDocumentPage*	oPage;
	Rect  cRectangle;
	RectF  cDrawRectangle;
	PointF cDelta;
	PointF cTopLeft;
	PointF cBottomRight;
	long lTexts;
	long lCharacters;

	if (bRecognitionLayerVisible) {

		oPage = oCore->GetCurrentPage();

		SolidBrush	oLayerBrush(Gdiplus::Color(64, 255, 0, 0));
		Pen			oLayerBorderPen(Gdiplus::Color(255,255,0,0));
		Pen			oLayerBorderPenWhite(Gdiplus::Color(255,255,255,255));

		if (oPage!=NULL) {

			oLayer = oPage->GetRecognitionLayer();

			lTexts = oLayer->GetRecognizedTextCount();

			while (lTexts>0) {

				lTexts--;

				oText = oLayer->GetRecognizedText(lTexts);

				if (oText!=NULL) {

					//Draw the bounding box of the text area
					cRectangle =	oText->GetBoundingBox();
					cDelta   =		PointF((REAL)cRectangle.X, (REAL)cRectangle.Y);
					cTopLeft =		ConvertImageToClient((float)cRectangle.X,		(float)cRectangle.Y);
					cBottomRight =	ConvertImageToClient((float)(cRectangle.X + cRectangle.Width), (float)(cRectangle.Y + cRectangle.Height));
					cDrawRectangle = RectF(cTopLeft.X, cTopLeft.Y, cBottomRight.X - cTopLeft.X, cBottomRight.Y - cTopLeft.Y);
					oCanvas->FillRectangle(&oLayerBrush, cDrawRectangle);
					oCanvas->DrawRectangle(&oLayerBorderPen, cDrawRectangle);
					cDrawRectangle.Inflate(-1,-1);
					oCanvas->DrawRectangle(&oLayerBorderPenWhite, cDrawRectangle);

					//Loop through all the characters
					lCharacters = oText->GetCharacterCount();

					while (lCharacters>0) {

						lCharacters--;

						//Draw the rectangles of the individual characters
						
						cRectangle =	oText->GetCharacterBoundingBox(lCharacters);
						cTopLeft =		ConvertImageToClient(cDelta.X + cRectangle.X, cDelta.Y + cRectangle.Y);
						cBottomRight =	ConvertImageToClient(cDelta.X + cRectangle.X + cRectangle.Width, cDelta.Y + cRectangle.Y + cRectangle.Height);
						cDrawRectangle = RectF(cTopLeft.X, cTopLeft.Y, cBottomRight.X - cTopLeft.X, cBottomRight.Y - cTopLeft.Y);
						//oCanvas->DrawRectangle(&oLayerBorderPen, cDrawRectangle);
						oCanvas->FillRectangle(&oLayerBrush, cDrawRectangle);
						}
					}
				}
			}
		}
	}


//This function draws the selection on the canvas
void 
CPageViewer::DrawSelection(Graphics* oCanvas, RectF cDestinationRect) {

	PointF cImageTopLeft;
	PointF cImageBottomRight;
	PointF cSelectionTopLeft;
	PointF cSelectionBottomRight;
	RectF SelectionRect;
	RectF ImageRect;
	SolidBrush oSelectionBrush(Gdiplus::Color(128, 0, 0, 0));
	Pen oSelectionBorderPenDark(Color::Black);
	Pen oSelectionBorderPenLight(Color::White);
	CCopyDocumentPage* oPage;
	Bitmap* oBitmap;
	sCopyDocumentSelection cSelection;

	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		oBitmap = oPage->GetImage()->GetBitmap();
		ImageRect = RectF(0, 0, (REAL)oBitmap->GetWidth(), (REAL)oBitmap->GetHeight());
		oPage->GetSelection(cSelection);

		cImageTopLeft =			ConvertImageToClient(0, 0);
		cImageBottomRight =		ConvertImageToClient((float)oBitmap->GetWidth(), (float)oBitmap->GetHeight());
		cSelectionTopLeft =		ConvertImageToClient(cSelection.fLeft, cSelection.fTop);
		cSelectionBottomRight = ConvertImageToClient(cSelection.fRight, cSelection.fBottom);
	
		if (cSelection.bEnabled) {

			//oCanvas->SetCompositingMode(CompositingModeSourceOver);

			if (cSelection.bOuter) {
		
				//Left
				SelectionRect = RectF(	cImageTopLeft.X, 
										cImageTopLeft.Y, 
										cSelectionTopLeft.X - cImageTopLeft.X,
										cImageBottomRight.Y - cImageTopLeft.Y);
				oCanvas->FillRectangle(&oSelectionBrush, SelectionRect);

				//Right
				SelectionRect = RectF(	cSelectionBottomRight.X, 
										cImageTopLeft.Y, 
										cImageBottomRight.X - cSelectionBottomRight.X,
										cImageBottomRight.Y - cImageTopLeft.Y);
				oCanvas->FillRectangle(&oSelectionBrush, SelectionRect);

				//Top
				SelectionRect = RectF(	cSelectionTopLeft.X, 
										cImageTopLeft.Y, 
										cSelectionBottomRight.X - cSelectionTopLeft.X,
										cSelectionTopLeft.Y - cImageTopLeft.Y);
				oCanvas->FillRectangle(&oSelectionBrush, SelectionRect);

				//Bottom
				SelectionRect = RectF(	cSelectionTopLeft.X, 
										cSelectionBottomRight.Y, 
										cSelectionBottomRight.X - cSelectionTopLeft.X,
										cImageBottomRight.Y - cSelectionBottomRight.Y);
				oCanvas->FillRectangle(&oSelectionBrush, SelectionRect);

				}
			else {
				
				//inner selection
				SelectionRect = RectF(	cSelectionTopLeft.X,
										cSelectionTopLeft.Y, 
										cSelectionBottomRight.X - cSelectionTopLeft.X,
										cSelectionBottomRight.Y - cSelectionTopLeft.Y);
				oCanvas->FillRectangle(&oSelectionBrush, SelectionRect);

				}
			//Border
			SelectionRect = RectF(	cSelectionTopLeft.X,
									cSelectionTopLeft.Y, 
									cSelectionBottomRight.X - cSelectionTopLeft.X,
									cSelectionBottomRight.Y - cSelectionTopLeft.Y);
			oCanvas->DrawRectangle(&oSelectionBorderPenDark, SelectionRect);

			SelectionRect.Inflate(-1, -1);
			oCanvas->DrawRectangle(&oSelectionBorderPenLight, SelectionRect);
			}
		else {

			//no selection present. Fill the selection rectangle with the image
			//so we can draw the grippers around the edges of the image.
			
				SelectionRect = RectF(	cImageTopLeft.X,
										cImageTopLeft.Y, 
										cImageBottomRight.X - cImageTopLeft.X,
										cImageBottomRight.Y - cImageTopLeft.Y);
			}

		//Draw the gripper around the image
		RectF	cGripperRect;
		RectF   cSourceGripperRect;

		cSourceGripperRect = RectF(0, 0, 10, 10);
		
		if (SelectionRect.Height>30 && SelectionRect.Width>30) {
			//gripper top
			cGripperRect = RectF((SelectionRect.X + 3 + (SelectionRect.Width - 10)/2),
								 (SelectionRect.Y + 3),
								  (REAL)10,
								  (REAL)10);
			
//			ImageList_Draw(hSelectionGripImages, 0, hCanvasDC, (int)cGripperRect.X, (int)cGripperRect.Y, ILD_TRANSPARENT);// ILD_BLEND25 | ILD_TRANSPARENT);
			oCanvas->DrawImage(oSelectionGripImages[0], (int)cGripperRect.X, (int)cGripperRect.Y, 10, 10);

			SetHotSpot(cGripperRect, eHSTSelectionTop);

			//gripper bottom
			cGripperRect = RectF((SelectionRect.X + 3 + (SelectionRect.Width - 10)/2),
								 (SelectionRect.Y + SelectionRect.Height - 10),
								  (REAL)10,
								  (REAL)10);

//			ImageList_Draw(hSelectionGripImages, 2, hCanvasDC, (int)cGripperRect.X, (int)cGripperRect.Y, ILD_TRANSPARENT);
			oCanvas->DrawImage(oSelectionGripImages[2], (int)cGripperRect.X, (int)cGripperRect.Y, 10, 10);
			
			SetHotSpot(cGripperRect, eHSTSelectionBottom);

			//gripper right

			cGripperRect = RectF((SelectionRect.X + SelectionRect.Width - 10),
								 (SelectionRect.Y + 3 + (SelectionRect.Height - 10)/2),
								  (REAL)10,
								  (REAL)10);

//			ImageList_Draw(hSelectionGripImages, 1, hCanvasDC, (int)cGripperRect.X, (int)cGripperRect.Y, ILD_TRANSPARENT);
			oCanvas->DrawImage(oSelectionGripImages[1], (int)cGripperRect.X, (int)cGripperRect.Y, 10, 10);

			SetHotSpot(cGripperRect, eHSTSelectionRight);

			//gripper left
			cGripperRect = RectF((SelectionRect.X + 3),
								 (SelectionRect.Y + 3 + (SelectionRect.Height - 20)/2),
								  (REAL)10,
								  (REAL)10);

//			ImageList_Draw(hSelectionGripImages, 3, hCanvasDC, (int)cGripperRect.X, (int)cGripperRect.Y, ILD_TRANSPARENT);
			oCanvas->DrawImage(oSelectionGripImages[3], (int)cGripperRect.X, (int)cGripperRect.Y, 10, 10);

			SetHotSpot(cGripperRect, eHSTSelectionLeft);

			//Gripper SW
			cGripperRect = RectF(SelectionRect.X + 3,
								 SelectionRect.Y + SelectionRect.Height - 10,
								 10, 10);
//			ImageList_Draw(hSelectionGripImages, 7, hCanvasDC, (int)cGripperRect.X, (int)cGripperRect.Y, ILD_TRANSPARENT);
			oCanvas->DrawImage(oSelectionGripImages[7], (int)cGripperRect.X, (int)cGripperRect.Y, 10, 10);

			SetHotSpot(cGripperRect, eHSTSelectionSouthWest);

			//Gripper NE
			cGripperRect = RectF(SelectionRect.X + SelectionRect.Width - 10,
								 SelectionRect.Y + 3,
								 10, 10);
//			ImageList_Draw(hSelectionGripImages, 5, hCanvasDC, (int)cGripperRect.X, (int)cGripperRect.Y, ILD_TRANSPARENT);
			oCanvas->DrawImage(oSelectionGripImages[5], (int)cGripperRect.X, (int)cGripperRect.Y, 10, 10);

			SetHotSpot(cGripperRect, eHSTSelectionNorthEast);
			}
		else {

			RemoveHotSpot(eHSTSelectionBottom);
			RemoveHotSpot(eHSTSelectionTop);
			RemoveHotSpot(eHSTSelectionLeft);
			RemoveHotSpot(eHSTSelectionRight);
			RemoveHotSpot(eHSTSelectionNorthEast);
			RemoveHotSpot(eHSTSelectionSouthWest);
			}


		//Gripper NW
		cGripperRect = RectF(SelectionRect.X + 2,
					         SelectionRect.Y + 3,
							 20, 20);

		if (SelectionRect.Height>15 && SelectionRect.Width>15) {
		//	ImageList_Draw(hSelectionGripImages, 4, hCanvasDC, (int)cGripperRect.X, (int)cGripperRect.Y, ILD_TRANSPARENT);
			oCanvas->DrawImage(oSelectionGripImages[4], (int)cGripperRect.X, (int)cGripperRect.Y, 10, 10);
			}

		SetHotSpot(cGripperRect, eHSTSelectionNorthWest);


		//Gripper SE
		cGripperRect = RectF(SelectionRect.X + SelectionRect.Width - 10,
					         SelectionRect.Y + SelectionRect.Height - 10,
							 10, 10);

		if (SelectionRect.Height>15 && SelectionRect.Width>15) {
			oCanvas->DrawImage(oSelectionGripImages[6], (int)cGripperRect.X, (int)cGripperRect.Y, 10, 10);
			}

		SetHotSpot(cGripperRect, eHSTSelectionSouthEast);

//		oCanvas->ReleaseHDC(hCanvasDC);

		//Set the hot spot of the whole selection
		if (cSelection.bEnabled) {
			SetHotSpot(SelectionRect, eHSTSelection, false);
			}
		else {
			RemoveHotSpot(eHSTSelection);
			}
		}
	}

//This is the main function of the thumbnail creation thread. This function
//polls continuely if there are thumbnails to be created

DWORD 
WINAPI ThreadCreateCachedImage(LPVOID lpParameter) {

	CPageViewer*		oOwner;
	HDC					hClientDC;
	sCachedImageRequest cRequest;
	DWORD				dTicksPassed;
	Bitmap*				oCreatedCachedBitmap;
	Graphics*			oCanvas;

	oOwner = (CPageViewer*)lpParameter;

	while (!oOwner->bEndThread) {

		Sleep(250);

		//loop through the thumbnails to be created
		while (oOwner->vToBeProcessed.size()>0 && !oOwner->bEndThread) {
			
			EnterCriticalSection(&oOwner->ThreadCriticalSection); 

			//remove old requests
			while (oOwner->vToBeProcessed.size()>1) {
				oOwner->vToBeProcessed.erase(oOwner->vToBeProcessed.begin());
				}

			//get the latest request
			cRequest = oOwner->vToBeProcessed[0];

			LeaveCriticalSection(&oOwner->ThreadCriticalSection); 
			
			dTicksPassed = (GetTickCount() - cRequest.dRequestedOnTick);

			//The request has to be at least a second old to be processed
			if (dTicksPassed<0 || dTicksPassed>=500) {
				
				//Create the cached bitmap
				hClientDC = CreateCompatibleDC(NULL);
				oCanvas = new Graphics(hClientDC);				
				
				oCreatedCachedBitmap = oOwner->CreateCachedBitmap(cRequest.oImage, cRequest.fMagnification, oCanvas);
				
				delete oCanvas;
				DeleteDC(hClientDC);

				EnterCriticalSection(&oOwner->ThreadCriticalSection); 
				
				//erase the request
				oOwner->vToBeProcessed.erase(oOwner->vToBeProcessed.begin());

				if (oOwner->vToBeProcessed.size()==0) {

					//let our owner know we are done, and give it
					//the pointer to our bitmap

					if (oOwner->oNewCachedBitmap!=NULL) delete oOwner->oNewCachedBitmap;

					oOwner->oNewCachedBitmap = oCreatedCachedBitmap;
					oOwner->fMagnificationNewCachedBitmap = cRequest.fMagnification;
					oOwner->Redraw();
					}
				else {
					
					//a new request has arrived. Delete ours.
					delete oCreatedCachedBitmap;
					}

				LeaveCriticalSection(&oOwner->ThreadCriticalSection);
				}
			}
		}

	return 0;
	}

//This functions sets a hotspot
void 
CPageViewer::SetHotSpot(RectF pcHotSpotRect, eHotSpotType peType, bool pbInFront) {

	bool bExists;
	sPageViewerHotSpot cHotSpot;

	bExists = false;

	//check if such a hot spot already exists, if so update it
	for (int iIndex=0; iIndex<(int)oHotSpots.size() && !bExists; iIndex++) {

		if (oHotSpots[iIndex].eType == peType) {
			
			bExists = true;
			oHotSpots[iIndex].cHotSpotRect = pcHotSpotRect;
			}
		}

	//non exists, add it
	if (!bExists) {

		cHotSpot.cHotSpotRect = pcHotSpotRect;
		cHotSpot.eType = peType;

		if (pbInFront) {

			oHotSpots.insert(oHotSpots.begin(), cHotSpot);
			}
		else {

			oHotSpots.push_back(cHotSpot);
			}
		}
	}

void 
CPageViewer::RemoveHotSpot(eHotSpotType peType) {

	bool bRemoved;

	bRemoved = false;

	for (int iIndex=0; iIndex < (int)oHotSpots.size() && !bRemoved; iIndex++) {

		if (oHotSpots[iIndex].eType == peType) {

			bRemoved = true;
			oHotSpots.erase(oHotSpots.begin() + iIndex);
			}
		}
	}

//This function checks if the mouse hovers above a hotspot. If it does
//it returns the type of hotspot
eHotSpotType 
CPageViewer::MouseOverHotSpot(int piX, int piY) {

	//This function works in Client coordinates

	eHotSpotType eResult;

	eResult = eHSTNone;

	for (int iIndex=0; iIndex<(int)oHotSpots.size() && eResult==eHSTNone; iIndex++) {

		if (oHotSpots[iIndex].cHotSpotRect.Contains((REAL)piX, (REAL)piY)) {

			eResult = oHotSpots[iIndex].eType;
			}
		}

	return eResult;
	}

//This function updates the selection dimensions, or moves the selection
void 
CPageViewer::UpdateHotSpotPosition(eHotSpotType peType, int iX, int iY){

	//This function works in Image coordinates

	float fX, fY;
	CCopyDocumentPage* oPage;
	Bitmap* oBitmap;
	sCopyDocumentSelection cSelection;

	oPage = oCore->GetCurrentPage();
	if (oPage==NULL) return;

	//convert the mouse position to a position on the image

	fX = (float)iX;
	fY = (float)iY;

	oBitmap = oPage->GetImage()->GetBitmap();

	if (fX<0) fX=0;
	if (fY<0) fY=0;
	if (fX>oBitmap->GetWidth()) fX = (float)oBitmap->GetWidth();
	if (fY>oBitmap->GetHeight()) fY = (float)oBitmap->GetHeight();

	//update the rectangle
	oPage->GetSelection(cSelection);
	if (!cSelection.bEnabled) {

		cSelection.fLeft = 0;
		cSelection.fTop = 0;
		cSelection.fRight = (float)oBitmap->GetWidth();
		cSelection.fBottom = (float)oBitmap->GetHeight();
		}
	
	cSelection.bEnabled = true;

	switch (peType) {

		case eHSTSelectionSouthEast:
			
			cSelection.fBottom = fY;
			cSelection.fRight  = fX;
			break;

		case eHSTSelectionNorthEast:
			
			cSelection.fTop = fY;
			cSelection.fRight  = fX;
			break;

		case eHSTSelectionSouthWest:
			
			cSelection.fBottom = fY;
			cSelection.fLeft  = fX;
			break;

		case eHSTSelectionNorthWest:
			
			cSelection.fTop = fY;
			cSelection.fLeft  = fX;
			break;

		case eHSTSelectionTop:

			cSelection.fTop = fY;
			if (fY > cSelection.fBottom) {
				
				cSelection.fTop = cSelection.fBottom;
				cSelection.fBottom = fY;
				oMouseState.cType = eHSTSelectionBottom;
				}
			break;

		case eHSTSelectionBottom:

			cSelection.fBottom = fY;
			break;

		case eHSTSelectionLeft:

			cSelection.fLeft = fX;
			if (fX > cSelection.fRight) {
				
				cSelection.fLeft = cSelection.fRight;
				cSelection.fRight = fX;
				oMouseState.cType = eHSTSelectionRight;
				}
			break;

		case eHSTSelectionRight:

			cSelection.fRight = fX;
			break;

		case eHSTSelection:

			//move the selection, and keep it within the image
			float fNewLeft, fNewTop;
			float fDeltaX, fDeltaY;

			fNewLeft = fX - oMouseState.cMouseClickPoint.X;
			fNewTop =  fY - oMouseState.cMouseClickPoint.Y;

			if (fNewTop<0) fNewTop = 0;
			if (fNewTop + cSelection.fBottom - cSelection.fTop > oBitmap->GetHeight()) 
				fNewTop = oBitmap->GetHeight() - cSelection.fBottom + cSelection.fTop;
			if (fNewLeft<0) fNewLeft = 0;
			if (fNewLeft + cSelection.fRight - cSelection.fLeft > oBitmap->GetWidth()) 
				fNewLeft = oBitmap->GetWidth() - cSelection.fRight + cSelection.fLeft;

			fDeltaX = fNewLeft - cSelection.fLeft;
			fDeltaY = fNewTop - cSelection.fTop;
	
			cSelection.fLeft += fDeltaX;
			cSelection.fRight += fDeltaX;
			cSelection.fTop += fDeltaY;
			cSelection.fBottom += fDeltaY;

			break;
		}

	//Validate selection
	if (cSelection.fRight < cSelection.fLeft) {
		
		//The width of the selection is negative. Change the active hotspot
		switch (oMouseState.cType) {

			case eHSTSelectionRight:	 oMouseState.cType=eHSTSelectionLeft; break;
			case eHSTSelectionLeft:		 oMouseState.cType=eHSTSelectionRight; break;
			case eHSTSelectionNorthEast: oMouseState.cType=eHSTSelectionNorthWest; break;
			case eHSTSelectionNorthWest: oMouseState.cType=eHSTSelectionNorthEast; break;
			case eHSTSelectionSouthEast: oMouseState.cType=eHSTSelectionSouthWest; break;
			case eHSTSelectionSouthWest: oMouseState.cType=eHSTSelectionSouthEast; break;
			}
		}

	if (cSelection.fBottom < cSelection.fTop) {
		
		//The height of the selection is negative. Change the active hotspot
		switch (oMouseState.cType) {

			case eHSTSelectionTop: 		 oMouseState.cType=eHSTSelectionBottom; break;
			case eHSTSelectionBottom:	 oMouseState.cType=eHSTSelectionTop; break;
			case eHSTSelectionNorthEast: oMouseState.cType=eHSTSelectionSouthEast; break;
			case eHSTSelectionNorthWest: oMouseState.cType=eHSTSelectionSouthWest; break;
			case eHSTSelectionSouthEast: oMouseState.cType=eHSTSelectionNorthEast; break;
			case eHSTSelectionSouthWest: oMouseState.cType=eHSTSelectionNorthWest; break;
			}
		}

	//Update the changes to the selection
	oPage->SetSelection(cSelection);

	//Update the cursor
	SetCursor(GetCursorType(oMouseState.cType));

	//Redraw the pageviewer
	Redraw();
	}
	
//This function handles the mouse events. Used to clear up the WndProc
bool 
CPageViewer::OnEventMouse(UINT message, WPARAM wParam, LPARAM lParam) {

	bool bResult;

					PointF cPosition;
					Color  cPixelColor;
					CCopyDocumentPage* oPage;
					DWORD dColor;
	bResult = true;

	switch (message) {

		case WM_MOUSEMOVE:

			HCURSOR hCursor;
			hCursor = NULL;

			switch (oMouseState.cMouseState) {

				case eMouseStateColorPicker:

					SetCursor(hCursorHand);
					
					cPosition = ConvertClientToImage(LOWORD(lParam), HIWORD(lParam));

					oPage = oCore->GetCurrentPage();

					if (oPage!=NULL) {

						oPage->GetImage()->GetBitmap()->GetPixel((int)cPosition.X, (int)cPosition.Y, &cPixelColor);
						
						dColor = (DWORD)(cPixelColor.GetB()*65536 + cPixelColor.GetG()*256 + cPixelColor.GetR());
						CWindowBase::oGlobalInstances.oRegistry->WriteInt(L"General", L"Color", dColor);

						//To be able to update the color button in the toolbar while moving over the
						//image
						oCore->oNotifications->CommunicateEvent(eNotificationColorChanged, (void*)dColor);
						}
					
					break;
		
				case eMouseStateStandard:

					hCursor = GetCursorType(MouseOverHotSpot(LOWORD(lParam), HIWORD(lParam)));
					SetCursor(hCursor);
					
					//if (bRecognitionLayerVisible) {

					//	oPage = oCore->GetCurrentPage();

					//	if (oPage!=NULL) {
					//			
					//		PointF cNewPosition;
					//		std::wstring sText;

					//		cNewPosition = ConvertClientToImage(LOWORD(lParam), HIWORD(lParam));
					//		CRecognitionLayer* oLayer = oPage->GetRecognitionLayer();
					//		sRecognizedCharacter cCharacter = oLayer->GetCharacter((int)cNewPosition.X, (int)cNewPosition.Y);
					//		
					//		if (cCharacter.cCharacter!=0x00) {
					//		sText=L"";
					//		sText+=cCharacter.cCharacter;
					//		//MessageBox(hWnd, sText.c_str(), L"Test", MB_OK);
					//		POINT cPoint;

					//		cPoint.x = LOWORD(lParam);
					//		cPoint.y = HIWORD(lParam);
					//		ShowToolTip(sText, cPoint);
					//			}
					//		}
						//}					
					
					break;

				case eMouseStateHotSpot:

					PointF cNewPosition;

					cNewPosition = ConvertClientToImage(LOWORD(lParam), HIWORD(lParam));
					
					if (cNewPosition.X!=-1) UpdateHotSpotPosition(oMouseState.cType, (int)cNewPosition.X, (int)cNewPosition.Y);
	
					//hCursor = GetCursorType(MouseOverHotSpot(LOWORD(lParam), HIWORD(lParam)));
					//SetCursor(hCursor);
					break;

				}
			break;

		case WM_RBUTTONDOWN:

			//The right mouse button is clicked. When we are above the selection
			//we will show a contextmenu.
			if (ClientPositionInImage(LOWORD(lParam), HIWORD(lParam))) {
				DoSelectionContextMenu(LOWORD(lParam), HIWORD(lParam));
				}

			break;

		case WM_MBUTTONDOWN:

			SetFocus(hWnd);
			break;
		
		case WM_LBUTTONDOWN:

			eHotSpotType pcType;
			CCopyDocumentPage* oPage;
			sCopyDocumentSelection cSelection;

			//The left mouse button is clicked. If we are above a hotspot
			//select it and capture the mouse. We can move the hotspot.

			switch (oMouseState.cMouseState) {

				case eMouseStateColorPicker:

					oMouseState.cMouseState = eMouseStateStandard;

					dColor = (DWORD)CWindowBase::oGlobalInstances.oRegistry->ReadInt(L"General", L"Color", 0xFFFFFF);
					oCore->oNotifications->CommunicateEvent(eNotificationColorChanged, (void*)dColor);
					break;

				default:

					pcType = MouseOverHotSpot(LOWORD(lParam), HIWORD(lParam));

					if (pcType!=eHSTNone) {

						oPage = oCore->GetCurrentPage();

						if (oPage!=NULL) {

							oPage->GetSelection(cSelection);
							
							ClearMouseState();
							SetMouseStateMoveHotspot(pcType);
							oMouseState.cMouseClickPoint = ConvertClientToImage(LOWORD(lParam), HIWORD(lParam));
							oMouseState.cMouseClickPoint.X -= cSelection.fLeft;
							oMouseState.cMouseClickPoint.Y -= cSelection.fTop;

							oPage->SetSelection(cSelection);

							SetCursor(GetCursorType(pcType));
							}
						}
					else {

						//Build a new selection 
						PointF cNewPosition;
						CCopyDocumentPage* oPage;
			
						oPage = oCore->GetCurrentPage();

						if (oPage!=NULL && ClientPositionInImage(LOWORD(lParam), HIWORD(lParam))) {

							pcType = eHSTSelectionSouthEast;

							ClearMouseState();
							SetMouseStateMoveHotspot(pcType);

							cNewPosition = ConvertClientToImage(LOWORD(lParam), HIWORD(lParam));

							cSelection.bEnabled = true;
							cSelection.bOuter = false;
							cSelection.fLeft = cNewPosition.X;
							cSelection.fTop  = cNewPosition.Y;
							cSelection.fRight = cNewPosition.X;
							cSelection.fBottom = cNewPosition.Y;

							oPage->SetSelection(cSelection);
							
							SetCursor(GetCursorType(pcType));

							Redraw();
							}
						else {

							SelectionClear();
							}

						SetFocus(hWnd);
						}

					break; //end default mousestate
				}

			break;
		
		case WM_LBUTTONUP:

			//The mouse button is release. If the mouse was in a state in which
			//it was moving a hotspot. Update the position of the hotspot.
			if (oMouseState.cMouseState == eMouseStateHotSpot) {

				PointF cNewPosition;

				cNewPosition = ConvertClientToImage(LOWORD(lParam), HIWORD(lParam));
				
				if (cNewPosition.X!=-1) UpdateHotSpotPosition(oMouseState.cType, (int)cNewPosition.X, (int)cNewPosition.Y);
				
				DoSelectionCommit();
				}

			ClearMouseState();
			break;

		default:

			bResult = false; // we didn't handle an event
			break;
		}

	return bResult;
	}

//This function processes the mousewheel message
bool 
CPageViewer::OnEventMouseWheel(WPARAM wParam, LPARAM lParam) {

	bool bReturn;
	WPARAM wNewPosition;
	int iNewPosition;
	int iDelta;
	int iVirtualKey;

	bReturn = false;

	iDelta = (int)HIWORD(wParam);

	if (iDelta > 32000) {
		iDelta -= 65536;
		}

	iVirtualKey = LOWORD(wParam);

	switch (iVirtualKey) {

		case MK_MBUTTON:

			float fNewMagnification;
			sPageViewerPosition cPosition;
			POINT cClientPosition;

			cClientPosition.x= LOWORD(lParam);
			cClientPosition.y = HIWORD(lParam);

			ScreenToClient(hWnd, &cClientPosition);

			cPosition = GetImagePosition(cClientPosition);

			fNewMagnification = GetMagnification();
			fNewMagnification *= (1.0f+((float)iDelta / 1200.0f));

			SetMagnification(fNewMagnification);

			SetImagePosition(cPosition);
			break;

		default:

			if (cSBIVertical.nMax != cSBIVertical.nPage) {
			
				iNewPosition = (int)oSettings->fPosY;
				float fDelta;

				fDelta = ((float)cSBIVertical.nMax / 1000.0f) * (float)(iDelta / 2); 
				iNewPosition -= (int)fDelta;

				if (iNewPosition<0) iNewPosition=0;

				wNewPosition = MAKEWPARAM(SB_THUMBPOSITION, iNewPosition);

				OnEventScrollbar(WM_VSCROLL, wNewPosition, 0);
				}
			break;
		}

	bReturn = true;

	return bReturn;
	}

//This function processes the notification from the core
bool 
CPageViewer::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;

	bReturn = false;

	while (oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationPageUpdate:
			case eNotificationPageViewChanges: //The selected page in the pageviewer has changed

				NewPage();
				ProcessUpdatedSettings();

				bReturn = true;
				break;

			case eNotificationPageDeleted: //The page in the parameter is deleted, this is the change to clear the vector

				ClearSettings((DWORD)cNotification.pData);

				bReturn = true;
				break;

			default: //Try to process it by our base class CPagePreview

				if (cPageViewType==ePVTPrintPreview) {
					bReturn = CPagePreview::OnEventCoreNotification(this->hWnd, cNotification);
					}
				break;
				}
			}

	return bReturn;
	}

//This function process the scrollbar movements
void 
CPageViewer::OnEventScrollbar(UINT message, WPARAM wParam, LPARAM lParam) {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	CWindowScrollbar::OnEventScrollbar(message, wParam, lParam);

	oSettings->fPosY = max(0, (float)cSBIVertical.nPos);
	oSettings->fPosX = max(0, (float)cSBIHorizontal.nPos);

	Redraw();
	
	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function clear the mouse state and sets it to its default value
void 
CPageViewer::ClearMouseState() {

	if (oMouseState.cMouseState != eMouseStateStandard) {

		ReleaseCapture();
		}

	oMouseState.cMouseState = eMouseStateStandard;
	}

void 
CPageViewer::SetMouseStateMoveHotspot(eHotSpotType peType) {

	sMouseState cNewState;

	cNewState.cMouseState = eMouseStateHotSpot;
	cNewState.cType = peType;

	SetMouseState(cNewState);

	SetCapture(hWnd);
}

void 
CPageViewer::SetMouseState(sMouseState &pcState) {

	oMouseState = pcState;
}


//This function test if the coordinats are within the image
bool   
CPageViewer::ClientPositionInImage(float pfX, float pfY) {

	bool bResult;
	CCopyDocumentPage* oPage;
	Bitmap* oBitmap;
	RECT cClientRect;
	RectF DestinationRect;
	RectF ClientRect;

	bResult = false;

	//Retrieve the page
	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		bResult = true;

		//Retrieve the image and the client rectange
		oBitmap = oPage->GetImage()->GetBitmap();
		GetClientRect(hWnd, &cClientRect);
		ClientRect = RectF(0, 0, (REAL)cClientRect.right, (REAL)cClientRect.bottom);

		//the destination rectange. Where to draw the image
		DestinationRect = RectF(-oSettings->fPosX, 
								-oSettings->fPosY, 
								oBitmap->GetWidth() * oSettings->fMagnification,
								oBitmap->GetHeight() * oSettings->fMagnification);

		if (ClientRect.Width > DestinationRect.Width) {

			DestinationRect.Offset((ClientRect.Width - DestinationRect.Width)/2, 0);	
			}
		if (ClientRect.Height > DestinationRect.Height) {

			DestinationRect.Offset(0, (ClientRect.Height - DestinationRect.Height)/2);	
			}

		//Convert the coordinates
		pfX = (pfX - DestinationRect.X) / oSettings->fMagnification;
		pfY = (pfY - DestinationRect.Y) / oSettings->fMagnification;

		//Validate
		if (pfX<0 || pfY<0) bResult = false;
		if (pfX>=oBitmap->GetWidth()) bResult = false;
		if (pfY>=oBitmap->GetHeight()) bResult = false;
		}

	return bResult;
	}


//This function convert the coordinats from the client to the image
PointF 
CPageViewer::ConvertClientToImage(float pfX, float pfY) {

	PointF cResult;
	CCopyDocumentPage* oPage;
	Bitmap* oBitmap;
	RECT cClientRect;
	RectF DestinationRect;
	RectF ClientRect;

	cResult.X = 0;
	cResult.Y = 0;

	//work around. When mouse is outside client the position is greater than
	//32768. This function sees it as a very large number instead of a negative one.
	if (pfX>32768) {
		pfX=0;
		}
	if (pfY>32768) {
		pfY=0;
		}
	
	//Retrieve the page
	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		//Retrieve the image and the client rectange
		oBitmap = oPage->GetImage()->GetBitmap();
		GetClientRect(hWnd, &cClientRect);
		ClientRect = RectF(0, 0, (REAL)cClientRect.right, (REAL)cClientRect.bottom);

		//the destination rectange. Where to draw the image
		DestinationRect = RectF(-oSettings->fPosX, 
								-oSettings->fPosY, 
								oBitmap->GetWidth() * oSettings->fMagnification,
								oBitmap->GetHeight() * oSettings->fMagnification);

		if (ClientRect.Width > DestinationRect.Width) {

			DestinationRect.Offset((ClientRect.Width - DestinationRect.Width)/2, 0);	
			}
		if (ClientRect.Height > DestinationRect.Height) {

			DestinationRect.Offset(0, (ClientRect.Height - DestinationRect.Height)/2);	
			}

		//Convert the coordinates
		cResult.X = (pfX - DestinationRect.X) / oSettings->fMagnification;
		cResult.Y = (pfY - DestinationRect.Y) / oSettings->fMagnification;

		//Validate
		if (cResult.X<0) cResult.X=0;
		if (cResult.X>=oBitmap->GetWidth()) cResult.X=(REAL)oBitmap->GetWidth()-1;
		if (cResult.Y<0) cResult.Y=0;
		if (cResult.Y>=oBitmap->GetHeight()) cResult.Y=(REAL)oBitmap->GetHeight()-1;

		}

	return cResult;
}

//This function convert the coordinates from the image to the client
PointF 
CPageViewer::ConvertImageToClient(float pfX, float pfY) {

	PointF cResult;
	CCopyDocumentPage* oPage;
	Bitmap* oBitmap;
	RECT cClientRect;
	RectF DestinationRect;
	RectF ClientRect;

	cResult.X = 0;
	cResult.Y = 0;

	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {

		//Retrieve the image and the client rectange
		oBitmap = oPage->GetImage()->GetBitmap();
		GetClientRect(hWnd, &cClientRect);
		ClientRect = RectF(0, 0, (REAL)cClientRect.right, (REAL)cClientRect.bottom);

		//the destination rectange. Where to draw the image
		DestinationRect = RectF(-oSettings->fPosX, 
								-oSettings->fPosY, 
								oBitmap->GetWidth() * oSettings->fMagnification,
								oBitmap->GetHeight() * oSettings->fMagnification);

		if (ClientRect.Width > DestinationRect.Width) {

			DestinationRect.Offset((ClientRect.Width - DestinationRect.Width)/2, 0);	
			}
		if (ClientRect.Height > DestinationRect.Height) {

			DestinationRect.Offset(0, (ClientRect.Height - DestinationRect.Height)/2);	
			}

		//Convert the coordinates
		cResult.X = (pfX * oSettings->fMagnification) + DestinationRect.X;
		cResult.Y = (pfY * oSettings->fMagnification) + DestinationRect.Y;
		}

	return cResult;
}

void 
CPageViewer::DoSelectionContextMenu(int piX, int piY) {

	HMENU	hContextMenu;
	bool    bInImage;
	bool    bInTextBlock;
	wchar_t sMenuItem[100];
	POINT	cPoint;
	sCopyDocumentSelection cSelection;
	CCopyDocumentPage* oPage;
	eHotSpotType cHotSpotType;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__);

	//Get the selection rectangle of the current page
	oPage = oCore->GetCurrentPage();

	bInImage = false;
	bInTextBlock = false;

	if (oPage!=NULL) {
	
		oPage->GetSelection(cSelection);

		cImagePositionContextMenu = ConvertClientToImage((float)piX, (float)piY);

		bInImage		= ClientPositionInImage((float)piX, (float)piY);
		bInTextBlock	= (oPage->GetRecognitionLayer()->GetRecognizedText((int)cImagePositionContextMenu.X, (int)cImagePositionContextMenu.Y)!=NULL);
		cHotSpotType	= MouseOverHotSpot(piX, piY);
		}

	//Create a handle to the new popup menu
	hContextMenu = CreatePopupMenu();
	
	//The contents of the menu is dependent of the existence of a selection and where the cursor is hovering
	if (cSelection.bEnabled && cHotSpotType == eHSTSelection) {
		
		LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_FILLSELECTION, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_IMAGE_FILLSELECTION, sMenuItem);

		LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_CROPIMAGE, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_IMAGE_CROP, sMenuItem);

		LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_RECOGNIZETEXT, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_IMAGE_RECOGNIZETEXT, sMenuItem);

		AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);

		LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_CUT, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_EDIT_CUT, sMenuItem);

		LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_COPY, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_EDIT_COPY, sMenuItem);

		AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);
		}

	//Add options for zooming
	if ((cSelection.bEnabled && cHotSpotType == eHSTSelection) || (!oSettings->bSizeToFit)) {
		
		if (!oSettings->bSizeToFit) {
			
			LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_ZOOMTOFIT, sMenuItem, 99);
			AppendMenu(hContextMenu, MF_STRING, ID_ZOOM_ZOOMTOFIT, sMenuItem);
			}

		if (cSelection.bEnabled && cHotSpotType == eHSTSelection) {

			LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_ZOOMTOSELECTION, sMenuItem, 99);
			AppendMenu(hContextMenu, MF_STRING, ID_ZOOM_ZOOMTOSELECTION, sMenuItem);
			}

		AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);
		}

	//Add options voor zoom blocks
	if (bInTextBlock && bRecognitionLayerVisible) {

		LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_COPYTEXTTOCLIPBOARD, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_EDIT_COPYTEXTBLOCKTOCLIPBOARD, sMenuItem);

		LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_DELETETEXTBLOCK, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_PAGE_DELETETEXTBLOCK, sMenuItem);

		AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);
		}

	//Add options for the selection
	LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_SELECTALL, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_EDIT_SELECTALL, sMenuItem);

	if (cSelection.bEnabled) {

		LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_CLEARSELECTION, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_EDIT_CLEARSELECTION, sMenuItem);
		}

	if (cSelection.bEnabled && cHotSpotType == eHSTSelection) {
		LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_SWAPSELECTION, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_EDIT_SWAPSELECTION, sMenuItem);
		}

	AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);

	LoadString(CWindowBase::oGlobalInstances.hLanguage, IDS_RECOGNITIONLAYER, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_VIEW_RECOGNITIONLAYER, sMenuItem);

	//Process the menu and show it
	CWindowBase::oGlobalInstances.oMenuIcon->ProcessMenu(hContextMenu);

	GetCursorPos(&cPoint);

	TrackPopupMenu(hContextMenu, 
				   TPM_LEFTALIGN | TPM_TOPALIGN,
				   cPoint.x,
				   cPoint.y,
				   0,
				   hWnd, NULL);

	//Clean up
	DestroyMenu(hContextMenu);

	CWindowBase::oTrace->EndTrace(__WFUNCTION__);
	}

//This function 'commits' a selection. It checks if the selection is large
//enough to be useful. This will help prefent the situation where a user
//clicks on the page and creates a very small selection in this way.
void
CPageViewer::DoSelectionCommit() {

	sCopyDocumentSelection cSelection;
	CCopyDocumentPage* oPage;
	PointF cPointTopLeft;
	PointF cPointBottomRight;
	float  fArea;

	CWindowBase::oTrace->StartTrace(__WFUNCTION__);

	//Get the selection rectangle of the current page
	oPage = oCore->GetCurrentPage();

	if (oPage!=NULL) {
	
		oPage->GetSelection(cSelection);

		if (cSelection.bEnabled) {

			cPointTopLeft = ConvertImageToClient(cSelection.fLeft, cSelection.fTop);
			cPointBottomRight = ConvertImageToClient(cSelection.fRight, cSelection.fBottom);
			fArea = (cPointBottomRight.X - cPointTopLeft.X) * (cPointBottomRight.Y - cPointTopLeft.Y);

			//The selection is commit when its area is larger than 8 pixels
			if (abs((int)fArea)<9) {
				
				SelectionClear();
				}

			UINT iImageWidth = oPage->GetImage()->GetBitmap()->GetWidth();
			UINT iSelectionWidth = (UINT)(cSelection.fRight - cSelection.fLeft);
			UINT iImageHeight = (UINT)(cSelection.fBottom - cSelection.fTop);
			UINT iSelectionHeight = oPage->GetImage()->GetBitmap()->GetHeight();
			
			//Check if the outer selection contains the entire image
			if (abs((long)(iImageWidth - iSelectionWidth))<2 &&
				abs((long)(iImageHeight - iSelectionHeight))<2 &&
				cSelection.bOuter) {

				SelectionClear();
				}
			}
		}

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, cSelection.bEnabled);
	}

void 
CPageViewer::Localize(HINSTANCE phLanguage) {

	CWindowBase::oGlobalInstances.hLanguage = phLanguage;
	}

//This function returns the associated cursor to the given hotspot type
HCURSOR 
CPageViewer::GetCursorType(eHotSpotType peType) {

	HCURSOR hResult;

	switch (peType) {

		case eHSTSelection:

			hResult = hCursorSizeAll;
			break;

		case eHSTSelectionBottom:
		case eHSTSelectionTop:

			hResult = hCursorSizeNS;
			break;

		case eHSTSelectionLeft:
		case eHSTSelectionRight:

			hResult = hCursorSizeWE;
			break;

		case eHSTSelectionNorthEast:
		case eHSTSelectionSouthWest:

			hResult = hCursorSizeNESW;
			break;

		case eHSTSelectionNorthWest:
		case eHSTSelectionSouthEast:

			hResult = hCursorSizeNWSE;
			break;

		default:

			hResult = hCursorArrow;
			break;
		}

	return hResult;
	}

//This function toggles the visibility of the recognition layer
void 
CPageViewer::ToggleRecognitionLayer() {

	bRecognitionLayerVisible = !bRecognitionLayerVisible;

	oRegistry->WriteInt(L"General", L"LayerRecognition", bRecognitionLayerVisible ? 1 : 0);

	Redraw();
	}

//This function returns the visibility of the recognition layer
bool 
CPageViewer::GetRecogntionLayerVisible() {

	return bRecognitionLayerVisible;
	}

//This function will show a tool tip at the requested position
//void 
//CPageViewer::ShowToolTip(std::wstring psText, POINT pcPosition) {
//
//	
//	TOOLINFO		ti;
//	bool Balloon;
//
//	Balloon = true;
//
//	if (hWndTooltip==NULL) {
//
//		if(Balloon)//If you have choosen the Boolen Toop Tip will set the Windows style according to that
//		{
//			hWndTooltip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
//				WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
//				CW_USEDEFAULT, CW_USEDEFAULT,
//				CW_USEDEFAULT, CW_USEDEFAULT,
//				hWnd, NULL, CWindowBase::oGlobalInstances.hInstance,
//				NULL);
//		}
//		else
//		{
//
//			hWndTooltip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
//				WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
//				CW_USEDEFAULT, CW_USEDEFAULT,
//				CW_USEDEFAULT, CW_USEDEFAULT,
//				hWnd, NULL, CWindowBase::oGlobalInstances.hInstance,
//				NULL);
//		}
//
//	ti.cbSize = sizeof(TOOLINFO);
//	ti.uFlags =  TTF_TRACK | TTF_ABSOLUTE | TTF_SUBCLASS | TTF_IDISHWND;
//	ti.hwnd   = hWnd;							//Handle of the window in which the Control resides
//	ti.uId    = (UINT_PTR)hWnd;       //ID of the Cotrol for which Tool Tip will be Displyed
//	ti.hinst  = 0;
//	ti.lpszText  = (LPWSTR)psText.c_str();				//Tip you want to Display;
//	ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0; 
//
//	if(!SendMessage(hWndTooltip,TTM_ADDTOOL,0,(LPARAM)&ti)){ //Will add the Tool Tip on Control
//		MessageBox(NULL,L"Couldn't create the ToolTip control.",L"Error",MB_OK);
//		}
//
//	SendMessage(hWndTooltip, TTM_ACTIVATE, TRUE, 0); //Will Active the Tool Tip Control
//	SendMessage(hWndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti); //Will Active the Tool Tip Control
//	}
//
//
//	SendMessage(hWndTooltip, TTM_TRACKPOSITION, 0, MAKELONG(pcPosition.x, pcPosition.y)); //Will Active the Tool Tip Control
//
//	}
