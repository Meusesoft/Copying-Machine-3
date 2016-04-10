#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "PaneDocument.h"
#include <math.h> 

CPaneDocument::CPaneDocument(sGlobalInstances poGlobalInstances, int piMenuCommand) : 
	CPaneBase(poGlobalInstances, piMenuCommand, L"PaneDocument") {

	cType = ePaneDocument;
	sImage = L"PNG_TOOLPANEINFO";
	iTitle = IDS_INFORMATION;
	oPageProperties = NULL;
	}

CPaneDocument::~CPaneDocument(void) {

	Destroy();
	}

//Create the copy toolbar
bool 
CPaneDocument::Create(HINSTANCE phInstance, HWND phParent) {
	
	CPaneBase::oTrace->StartTrace(__WFUNCTION__);

	CPaneBase::Create(phInstance, phParent);

	Initialize();

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//Destroy all the controls and the main pane window.
void 
CPaneDocument::Destroy() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__);

	CPaneBase::oCore->oNotifications->UnsubscribeAll(hWnd);

	CPaneBase::Destroy();

	if (oPageProperties!=NULL) delete oPageProperties;

	CPaneBase::oTrace->EndTrace(__WFUNCTION__);
	}

//Initialize the Document pane
void 
CPaneDocument::Initialize() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__);

	CPaneBase::Initialize();

	//Add the controls for the information
	//Add controls to the pane
	AddPaneStatic(IDS_DOCUMENT, 55);
	AddPaneStatic(IDS_PAGE, 0);

	//Localize the labels
	Localize(CPaneBase::oGlobalInstances.hLanguage);

	hLabelPageMemorySize = AddStaticControl(L"");
	hTextPageMemorySize = AddStaticControl(L"");

	hLabelDocumentMemorySize = AddStaticControl(L"");
	hTextDocumentMemorySize = AddStaticControl(L"");

	hLabelDocumentPages = AddStaticControl(L"");
	hTextDocumentPages = AddStaticControl(L"");
	
	hLabelPageSizePixels = AddStaticControl(L"");
	hTextPageSizePixels = AddStaticControl(L"");

	//Add the controls for the resolution
	hHeaderResolution = AddStaticControl(L"");
	hTextx = AddStaticControl(L"\\cx");
	hTextdpi = AddStaticControl(L"dpi");

	hEditResolutionX = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"0", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER | ES_RIGHT, 
		20, 230, 50, 20, hWnd, (HMENU)IDC_RESOLUTIONX, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	oToBeDeletedHWND.push_back(hEditResolutionX);
	oPaneDocumentEdit[ePDResolutionX].hWnd = hEditResolutionX;
	
	hEditResolutionY = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"0", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER | ES_RIGHT, 
		20, 230, 50, 20, hWnd, (HMENU)IDC_RESOLUTIONY, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	oToBeDeletedHWND.push_back(hEditResolutionY);
	oPaneDocumentEdit[ePDResolutionY].hWnd = hEditResolutionY;

	//The controls for the page size
	hHeaderSize = AddStaticControl(L"");
	hTextx2 = AddStaticControl(L"\\cx");
	
	hEditPageSizeX = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"0", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER | ES_RIGHT, 
		20, 230, 50, 20, hWnd, (HMENU)IDC_PAGESIZEX, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	oToBeDeletedHWND.push_back(hEditPageSizeX);
	oPaneDocumentEdit[ePDPageSizeX].hWnd = hEditPageSizeX;

	hEditPageSizeY = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"0", 
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER | ES_RIGHT, 
		20, 230, 50, 20, hWnd, (HMENU)IDC_PAGESIZEY, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	oToBeDeletedHWND.push_back(hEditPageSizeY);
	oPaneDocumentEdit[ePDPageSizeY].hWnd = hEditPageSizeY;

	hEditMeasurement = CreateWindowEx(0L, L"COMBOBOX", NULL, 
		CBS_DROPDOWNLIST | WS_CHILD | WS_BORDER
		| WS_VISIBLE, 20, 80, 190, 150, hWnd, (HMENU)IDC_PAGESIZEMEASURE, 
		CPaneBase::oGlobalInstances.hInstance, 0 );
	LRESULT lInch = SendMessage(hEditMeasurement, CB_ADDSTRING, 0, (LPARAM)L"inch");
	SendMessage(hEditMeasurement, CB_SETITEMDATA, (WPARAM)lInch, (LPARAM)eInch);
	LRESULT lCm = SendMessage(hEditMeasurement, CB_ADDSTRING, 0, (LPARAM)L"cm");
	SendMessage(hEditMeasurement, CB_SETITEMDATA, (WPARAM)lCm, (LPARAM)eCm);
	
	eSizeMeasure cSelectedMeasure;
	cSelectedMeasure = (eSizeMeasure)oGlobalInstances.oRegistry->ReadInt(L"General", L"Measurement", (int)eInch);
	
	switch (cSelectedMeasure) {

		case eCm: //cm

			SendMessage(hEditMeasurement, CB_SETCURSEL, lCm, 0);
			break;

		default: //inch

			SendMessage(hEditMeasurement, CB_SETCURSEL, lInch, 0);
			break;
		}
	
	oToBeDeletedHWND.push_back(hEditMeasurement);

	//Create the page properties listview
	oPageProperties = new CPagePropertiesListview(oGlobalInstances, L"PageProperties");
	oPageProperties->Create(hWnd, oGlobalInstances.hInstance, WS_BORDER);
	AddChildWindow(oPageProperties);

	//Fill and enable the controls
	DoFillControls();
	DoEnableControls();

	//Localize the labels
	Localize(CPaneBase::oGlobalInstances.hLanguage);

	//Set the font for all the controls
	for (int iIndex=0; iIndex<(int)oToBeDeletedHWND.size(); iIndex++) {

		SendMessage(oToBeDeletedHWND[iIndex], WM_SETFONT, (WPARAM)hNormalFont, 0);
		}
	//SendMessage(hHeaderDocument, WM_SETFONT, (WPARAM)hBoldFont, 0);
	//SendMessage(hHeaderPage, WM_SETFONT, (WPARAM)hBoldFont, 0);

	//Position
	DoPositionControls();

	//Add notifications
	CPaneBase::oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentViewChanges);
	CPaneBase::oCore->oNotifications->Subscribe(hWnd, eNotificationPageSettingUpdate);
	CPaneBase::oCore->oNotifications->Subscribe(hWnd, eNotificationNewPage);
	CPaneBase::oCore->oNotifications->Subscribe(hWnd, eNotificationNewDocument);
	CPaneBase::oCore->oNotifications->Subscribe(hWnd, eNotificationPageViewChanges);

	CPaneBase::oTrace->EndTrace(__WFUNCTION__);
	}

void 
CPaneDocument::Localize(HINSTANCE phLanguage) {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	wchar_t* cText;

	CPaneBase::oGlobalInstances.hLanguage = phLanguage;

	//Alloc text buffer
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	//Update title
	LoadString(phLanguage, IDS_INFORMATION, cText, 400);
	sTitle = cText;
	//SetWindowText(hHeaderPage, cText);

	LoadString(phLanguage, IDS_DOCUMENT, cText, 400);
	//SetWindowText(hHeaderDocument, cText);

	LoadString(phLanguage, IDS_SIZEINMEMORY, cText, 400);
	SetWindowText(hLabelDocumentMemorySize, cText);
	SetWindowText(hLabelPageMemorySize, cText);

	LoadString(phLanguage, IDS_NUMBEROFPAGES, cText, 400);
	SetWindowText(hLabelDocumentPages, cText);

	LoadString(phLanguage, IDS_SIZEINPIXELS, cText, 400);
	SetWindowText(hLabelPageSizePixels, cText);

	LoadString(phLanguage, IDS_RESOLUTION, cText, 400);
	SetWindowText(hHeaderResolution, cText);

	LoadString(phLanguage, IDS_PAGESIZE, cText, 400);
	SetWindowText(hHeaderSize, cText);

	//Free memory
	free(cText);

	CPaneBase::LocalizePaneButtons(phLanguage);

	if (oPageProperties) oPageProperties->Localize(phLanguage);
	
	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function sets a pointer to the image containing the background for the
//pane window.
void 
CPaneDocument::SetBackgroundImagePointer(CBitmap* poBitmap) {

	oDoubleBufferBitmap = poBitmap;
	}

//This function handles the events from the core.
bool 
CPaneDocument::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;

	bReturn = false;

	while (CPaneBase::oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationDocumentViewChanges:
			case eNotificationPageSettingUpdate:
			case eNotificationNewPage:
			case eNotificationNewDocument:
			case eNotificationPageViewChanges:

				//Update the controls to synchronize it with the settings
				UpdateControls();

				//Remove pending notifications to prevent the controls being updated
				//multiple times
				CPaneBase::oCore->oNotifications->RemoveNotificationSpecific(this->hWnd, eNotificationDocumentViewChanges);
				CPaneBase::oCore->oNotifications->RemoveNotificationSpecific(this->hWnd, eNotificationPageSettingUpdate);
				CPaneBase::oCore->oNotifications->RemoveNotificationSpecific(this->hWnd, eNotificationNewPage);
				CPaneBase::oCore->oNotifications->RemoveNotificationSpecific(this->hWnd, eNotificationNewDocument);
				CPaneBase::oCore->oNotifications->RemoveNotificationSpecific(this->hWnd, eNotificationPageViewChanges);
				break;
			}
		}

	return bReturn;
	}


void 
CPaneDocument::UpdateControls() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	DoFillControls();

	DoEnableControls();

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function positions the controls in the pane window
void
CPaneDocument::DoPositionControls() {

	RECT cPositionRect;
	RECT cClientRect;
	int iMaxWidth;  

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll); 

	GetClientRect(hWnd, &cClientRect);

	cButtonsControlRect.X = 20;
	cButtonsControlRect.Y = 10;
	cButtonsControlRect.Width = cClientRect.right - (cButtonsControlRect.X * 2);

	PositionPaneButtons(cButtonsControlRect);


	GetWindowRect(hEditMeasurement, &cClientRect);
	iFontHeight = cClientRect.bottom - cClientRect.top;

	GetClientRect(hWnd, &cClientRect);

	cSettingsControlRect.X = 10;
	cSettingsControlRect.Y  = 10;
	cSettingsControlRect.Width = cClientRect.right - (cSettingsControlRect.X * 2);

	iMaxWidth = (int)(cSettingsControlRect.Width - (cSettingsControlRect.X * 2));
	cPositionRect.left = 20;
	cPositionRect.top = 20 - iFontHeight;

	//Document
	cPositionRect.top += iFontHeight;
	//MoveWindow(hHeaderDocument, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, false);

	//Document properties
	cPositionRect.top += (long)(iFontHeight*1.5);
	MoveWindow(hLabelDocumentMemorySize, cPositionRect.left, cPositionRect.top, (int)(iMaxWidth*0.5f), iFontHeight, true);
	MoveWindow(hTextDocumentMemorySize, (int)(cPositionRect.left + iMaxWidth*0.55f), cPositionRect.top, (int)(iMaxWidth*0.45f), iFontHeight, true);
	
	cPositionRect.top += iFontHeight;
	MoveWindow(hLabelDocumentPages, cPositionRect.left, cPositionRect.top, (int)(iMaxWidth*0.5f), iFontHeight, true);
	MoveWindow(hTextDocumentPages, (int)(cPositionRect.left + iMaxWidth*0.55f), cPositionRect.top, (int)(iMaxWidth*0.45f), iFontHeight, true);

	//Page
	cPositionRect.top += iFontHeight*2;
	//MoveWindow(hHeaderPage, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, false);
	
	//Page properties
	cPositionRect.top += (long)(iFontHeight*1.5);
	MoveWindow(hLabelPageMemorySize, cPositionRect.left, cPositionRect.top, (int)(iMaxWidth*0.5f), iFontHeight, true);
	MoveWindow(hTextPageMemorySize, (int)(cPositionRect.left + iMaxWidth*0.55f), cPositionRect.top, (int)(iMaxWidth*0.45f), iFontHeight, true);

	cPositionRect.top += iFontHeight;
	MoveWindow(hLabelPageSizePixels, cPositionRect.left, cPositionRect.top, (int)iMaxWidth*0.5f, iFontHeight, true);
	MoveWindow(hTextPageSizePixels, (int)(cPositionRect.left + iMaxWidth*0.55f), cPositionRect.top, (int)(iMaxWidth*0.45f), iFontHeight, true);

	cPositionRect.left += 15;
	iMaxWidth -= 15;

	//Resolution
	cPositionRect.top += (long)(iFontHeight*2.0f);
	MoveWindow(hHeaderResolution, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);
	cPositionRect.top += iFontHeight;
	MoveWindow(hEditResolutionX, cPositionRect.left, cPositionRect.top, (int)(iMaxWidth*0.22), iFontHeight, true);
	MoveWindow(hTextx, cPositionRect.left + (int)(iMaxWidth*0.22), cPositionRect.top, (int)(iMaxWidth*0.1), iFontHeight, true);
	MoveWindow(hEditResolutionY, cPositionRect.left + (int)(iMaxWidth*0.32), cPositionRect.top, (int)(iMaxWidth*0.22), iFontHeight, true);
	MoveWindow(hTextdpi, cPositionRect.left + (int)(iMaxWidth*0.57), cPositionRect.top, (int)(iMaxWidth*0.2), iFontHeight, true);

	//Physical Size
	cPositionRect.top += (long)(iFontHeight*1.5);
	MoveWindow(hHeaderSize, cPositionRect.left, cPositionRect.top, iMaxWidth, iFontHeight, true);
	cPositionRect.top += iFontHeight;
	MoveWindow(hEditPageSizeX, cPositionRect.left, cPositionRect.top, (int)(iMaxWidth*0.22), iFontHeight, true);
	MoveWindow(hTextx2, cPositionRect.left + (int)(iMaxWidth*0.22), cPositionRect.top, (int)(iMaxWidth*0.1), iFontHeight, true);
	MoveWindow(hEditPageSizeY, cPositionRect.left + (int)(iMaxWidth*0.32), cPositionRect.top, (int)(iMaxWidth*0.22), iFontHeight, true);
	MoveWindow(hEditMeasurement, cPositionRect.left + (int)(iMaxWidth*0.57), cPositionRect.top, (int)(iMaxWidth*0.3), iFontHeight, true);

	cPositionRect.top += iFontHeight;
	
	//Page properties
	cPositionRect.top += iFontHeight;

	MoveWindow(oPageProperties->hWnd, cPositionRect.left, cPositionRect.top,
		iMaxWidth, 250, true);
	if (IsWindowVisible(oPageProperties->hWnd)) cPositionRect.top += 250;

	//Update the minimum height of the pane
	if (iPaneMinimumHeight != cPositionRect.top + 10) {

		iPaneMinimumHeight = cPositionRect.top + 10;
		CPaneBase::oCore->oNotifications->CommunicateEvent(eNotificationPaneSizeChange, NULL);
		}

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll); 
	}


//This function processes all the messages, it returns true if it processed a message
bool 
CPaneDocument::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {

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
				
				//Reposition the controls;
				DoPositionControls();
				
				//Rebuild the background double buffer
				//DoDrawBackground();

				//Make sure the complete background is repainted, and all
				//the controls too (fix to make them appear transparent)
				//InvalidateRect(hWnd,				NULL, true);
				//InvalidateRect(hHeaderResolution,	NULL, true);
				//InvalidateRect(hTextx,				NULL, true);
				//InvalidateRect(hTextdpi,			NULL, true);

				//We processed this message
				//bReturn = true;
				//piReturn = 0;
				}
			break;


		case WM_PAINT:

			if (phWnd == this->hWnd) {

				DoPaint();
				}
			break;

		case WM_CORENOTIFICATION:

			if (phWnd == hWnd) {

				bReturn = OnEventCoreNotification();
				}
			bReturn=false;
			break;

		case WM_DRAWITEM:

			if (phWnd == this->hWnd) {
				//Draw the static controls with ownerdraw style
				LPDRAWITEMSTRUCT oDrawItemInfo;

				oDrawItemInfo = (LPDRAWITEMSTRUCT)lParam;

				DoDrawStaticControls(oDrawItemInfo);

				bReturn = true;
				}
			break;

		case WM_COMMAND:

			if (phWnd==hWnd) {
				bReturn = OnCommand(LOWORD(wParam), wParam, lParam);
				}
			break;

		default:

			if (!bReturn) bReturn = CPaneBase::WndProc(phWnd, message, wParam, lParam, piReturn);
			break;
			}
		
	return bReturn;
	}

void
CPaneDocument::DoEnableControls() {

	bool bEnable;

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	bEnable = (CPaneBase::oCore->GetCurrentDocument()!=NULL);

	EnableWindow(hEditMeasurement, bEnable);
	EnableWindow(hEditPageSizeX, bEnable);
	EnableWindow(hEditPageSizeY, bEnable);
	EnableWindow(hEditResolutionX, bEnable);
	EnableWindow(hEditResolutionY, bEnable);

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}

void
CPaneDocument::DoFillControls() {

	CCopyDocument* oDocument;
	CCopyDocumentPage* oPage;
	wchar_t cValue[50];
	DWORD dPageMemorySize = 0;
	DWORD dDocumentMemorySize = 0;
	DWORD dPages = 0;
	DWORD dPageWidth = 0;
	DWORD dPageHeight = 0;
	bool bPropertiesVisible;

	//Update the page properties listview
	oPageProperties->ClearProperties();

	oDocument = CPaneBase::oCore->GetCurrentDocument();
	
	if (oDocument!=NULL) {

		oPage = CPaneBase::oCore->GetCurrentPage();
		dDocumentMemorySize = oDocument->GetMemorySize();
		dPages = oDocument->GetPageCount();

		if (oPage!=NULL) {

			dPageMemorySize = oPage->GetMemorySize();
			dPageWidth	 = oPage->GetImage()->GetBitmap()->GetWidth();
			dPageHeight	 = oPage->GetImage()->GetBitmap()->GetHeight();

			bPropertiesVisible = (bool)IsWindowVisible(oPageProperties->hWnd);

			//Update the page properties listview
			oPageProperties->SetProperties(oPage);

			//Check if the listview with the properties changed in visibility, if so
			//do an update of the control position to update the minimum height property
			if (bPropertiesVisible != (bool)IsWindowVisible(oPageProperties->hWnd)) DoPositionControls();
			}
		}

	//Fill the edit controls
	DoFillEditControl(ePDResolutionX);
	DoFillEditControl(ePDResolutionY);
	DoFillEditControl(ePDPageSizeX);
	DoFillEditControl(ePDPageSizeY);

	//Fill the number of page
	swprintf_s(cValue, 25, L"%d", dPages);
	SendMessage(hTextDocumentPages, WM_SETTEXT, 0, (LPARAM)cValue);

	//Fill the page size in pixels
	swprintf_s(cValue, 49, L"%d x %d", dPageWidth, dPageHeight);
	SendMessage(hTextPageSizePixels, WM_SETTEXT, 0, (LPARAM)cValue);

	//Fill the memory size
	swprintf_s(cValue, 25, L"%1.1f GB", (float)dPageMemorySize/1000000000.0f);
	if (dPageMemorySize<1000000000) {
		swprintf_s(cValue, 25, L"%1.1f MB", (float)dPageMemorySize/1000000.0f);
		}
	if (dPageMemorySize<1000000) {
		swprintf_s(cValue, 25, L"%1.1f kB", (float)dPageMemorySize/1000.0f);
		}
	if (dPageMemorySize<1000) {
		swprintf_s(cValue, 25, L"%d bytes", dPageMemorySize);
		}
	SendMessage(hTextPageMemorySize, WM_SETTEXT, 0, (LPARAM)cValue);

	swprintf_s(cValue, 25, L"%1.1f GB", (float)dDocumentMemorySize/1000000000.0f);
	if (dDocumentMemorySize<1000000000) {
		swprintf_s(cValue, 25, L"%1.1f MB", (float)dDocumentMemorySize/1000000.0f);
		}
	if (dDocumentMemorySize<1000000) {
		swprintf_s(cValue, 25, L"%1.1f kB", (float)dDocumentMemorySize/1000.0f);
		}
	if (dDocumentMemorySize<1000) {
		swprintf_s(cValue, 25, L"%d bytes", dDocumentMemorySize);
		}
	SendMessage(hTextDocumentMemorySize, WM_SETTEXT, 0, (LPARAM)cValue);
	}

//This function processes the WM_COMMAND messages.
bool 
CPaneDocument::OnCommand(int piCommand, WPARAM wParam, LPARAM lParam) {

	bool bResult;

	bResult = false;

	switch (piCommand) {

		case IDC_PAGESIZEMEASURE: 
			
			//the measurement has changed. Update the registry and the controls
			int iCurrentItem;
			LRESULT lItemData;

			iCurrentItem = SendMessage(hEditMeasurement, CB_GETCURSEL, 0, 0);
			lItemData    = SendMessage(hEditMeasurement, CB_GETITEMDATA, (WPARAM)iCurrentItem, 0);
			oGlobalInstances.oRegistry->WriteInt(L"General", L"Measurement", (int)lItemData);

			DoFillControls();
			bResult = true;
			break;

		case IDC_PAGESIZEX:
		case IDC_PAGESIZEY:
		case IDC_RESOLUTIONX:
		case IDC_RESOLUTIONY:
		
			if (HIWORD(wParam) == EN_KILLFOCUS) {
					
					//iDirtyControl = piCommand;
					DoProcessUpdate(piCommand);
					bResult = true;
					}	
			break;

		default: 
			bResult = false;
			break;
		}

	return bResult;
	}

void 
CPaneDocument::DoProcessUpdate(int piControlId) {

	float fValue;
	float fPixels;
	bool bUpdate;
	CCopyDocumentPage* oPage;
	wchar_t cCurrentValue[30];
	ePaneDocumentEdit peEditControl;
	eSizeMeasure cMeasure = (eSizeMeasure)oGlobalInstances.oRegistry->ReadInt(L"General", L"Measurement", (int)eInch);
	
	//Convert the control id 
	switch (piControlId) {
		
		case IDC_RESOLUTIONX:	peEditControl=ePDResolutionX;	break;
		case IDC_RESOLUTIONY:	peEditControl=ePDResolutionY;	break;
		case IDC_PAGESIZEX:		peEditControl=ePDPageSizeX;		break;
		default:				peEditControl=ePDPageSizeY;		break;
		}

	//Get the current value of the given control and check if it has changed
	SendMessage(oPaneDocumentEdit[peEditControl].hWnd, WM_GETTEXT, (WPARAM)30, (LPARAM)cCurrentValue); 

	if (wcscmp(oPaneDocumentEdit[peEditControl].sValue.c_str(), cCurrentValue)!=0) {

		oPage = oCore->GetCurrentPage();
		bUpdate = true;

		//Update the properties
		if (oPage!=NULL) {

			fValue = GetFloatFromEditControl(oPaneDocumentEdit[peEditControl].hWnd);

			switch (peEditControl) {

				case ePDResolutionX:
					oPage->SetResolutionX(max(1, min(9999, (int)fValue)));
					break;

				case ePDResolutionY:
					oPage->SetResolutionY(max(1, min(9999, (int)fValue)));
					break;

				case ePDPageSizeX:
					fPixels = (float)oPage->GetImage()->GetBitmap()->GetWidth();
					if (fValue<=0) {
						bUpdate = false;
						}
					else {
						fValue = fPixels / fValue;
						if (cMeasure==eCm) fValue *= 2.54f;
						oPage->SetResolutionX((int)floor(fValue+0.5));
						}
					break;

				case ePDPageSizeY:

					fPixels = (float)oPage->GetImage()->GetBitmap()->GetHeight();
					if (fValue<=0) {
						bUpdate = false;
						}
					else {
						fValue = fPixels / fValue;
						if (cMeasure==eCm) fValue *= 2.54f;
						oPage->SetResolutionY((int)floor(fValue+0.5));
						}
					break;
				}

			if (bUpdate) {

				//Update the controls
				DoFillEditControl(ePDResolutionX);
				DoFillEditControl(ePDResolutionY);
				DoFillEditControl(ePDPageSizeX);
				DoFillEditControl(ePDPageSizeY);
				}
			}
		}
	}

//This function updates/fills the give edit control
void 
CPaneDocument::DoFillEditControl(ePaneDocumentEdit peEditControl) {

	CCopyDocumentPage* oPage;
	wchar_t cValue[30];
	wchar_t cCurrentValue[30];
	SizeF cPageSize;

	//Initialise settings
	long lResolutionX = 0;
	long lResolutionY = 0;
	cPageSize.Width = 0;
	cPageSize.Height = 0;

	//Get the current settings of the visible page (if there is one)
	oPage = CPaneBase::oCore->GetCurrentPage();

	if (oPage!=NULL) {

		lResolutionX = oPage->GetResolutionX();
		lResolutionY = oPage->GetResolutionY();
		cPageSize    = oPage->GetPhysicalSize();
		}

	//Process the value the a presentable format
	switch (peEditControl) {

		case ePDResolutionX:
			swprintf_s(cValue, 10, L"%d", lResolutionX);
			break;
		case ePDResolutionY:
			swprintf_s(cValue, 10, L"%d", lResolutionY);
			break;
		case ePDPageSizeX:
			swprintf_s(cValue, 10, L"%1.1f", cPageSize.Width);
			break;
		default: //hEditPageSizeY:
			swprintf_s(cValue, 10, L"%1.1f", cPageSize.Height);
			break;
		}

	SendMessage(oPaneDocumentEdit[peEditControl].hWnd, WM_GETTEXT, (WPARAM)30, (LPARAM)cCurrentValue); 

	//Only an update if the value is changed
	if (wcsncmp(cValue, cCurrentValue, 30)!=0) {
		
		SendMessage(oPaneDocumentEdit[peEditControl].hWnd, WM_SETTEXT, 0, (LPARAM)cValue);
		oPaneDocumentEdit[peEditControl].sValue = cValue;
		}
	}


//This function reads out an edit control and converts its value to
//a float
float  
CPaneDocument::GetFloatFromEditControl(HWND phEditControl) {

	float fResult;
	wchar_t cNumber[20];

	SendMessage(phEditControl, WM_GETTEXT, (WPARAM)19, (LPARAM)cNumber);
	fResult = (float)_wtof(cNumber); 

	return fResult;
	}

//This function is called when the scrollbar has been moved to reposition
//all the controls on the window
void 
CPaneDocument::DoRepositionControls() {

	DoPositionControls();
	InvalidateRect(hWnd, NULL, TRUE);
}

