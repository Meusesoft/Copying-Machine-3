#include "StdAfx.h"
#include "PaneBase.h"

CPaneBase::CPaneBase(sGlobalInstances poGlobalInstances, int piMenuCommand,
					 std::wstring psWindowName) : 
			CWindowBase(poGlobalInstances, psWindowName) {

	//Copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &poGlobalInstances, sizeof(sGlobalInstances));

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oTrace = oGlobalInstances.oTrace;

	oDoubleBufferBitmap = NULL;
	oWhiteBrush = new SolidBrush(Color::White);

	cType = ePaneUndefined;
	hBoldFont = NULL;
	iMenuCommand = piMenuCommand;

	iPaneMinimumHeight = 0;
	iTitle = 0;
	}

CPaneBase::~CPaneBase(void) {

	//Clean up buttons
	for (long lIndex=0; lIndex<(long)oPaneButtons.size(); lIndex++) {

		if (oPaneButtons[lIndex].oPaneButton!=NULL) delete oPaneButtons[lIndex].oPaneButton;
		}

	oPaneButtons.clear();

	//Clean up windows
	for (long lIndex=0; lIndex<(long)oToBeDeletedHWND.size(); lIndex++) {

		DestroyWindow(oToBeDeletedHWND[lIndex]);
		}

	oToBeDeletedHWND.clear();

	if (hBoldFont!=NULL) DeleteObject(hBoldFont);
	if (oWhiteBrush!=NULL) delete oWhiteBrush;
	}

//Create the pane window
bool 
CPaneBase::Create(HINSTANCE phInstance, HWND phParent) {
	
	oTrace->StartTrace(__WFUNCTION__);

	DWORD dwStyle = WS_CHILD |
					//WS_VISIBLE |
					WS_CLIPCHILDREN | 
					WS_CLIPSIBLINGS;

	//if (bUseCustomControlView) dwStyle = dwStyle & ~WS_BORDER;

	hWnd = CreateWindowEx(0,						// ex style
						 L"MeusesoftPane",			// class name
						 L"MeusesoftPane",			// window text
						 dwStyle,                   // style
						 0,                         // x position
						 30,                        // y position
						 20,                       // width
						 20,                       // height
						 phParent,					// parent
						 NULL,						// menu
						 oGlobalInstances.hInstance,// instance
						 NULL);                     // no extra data

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//Initialize the pane
void
CPaneBase::Initialize() {

	LOGFONT lfSystemVariableFont;

	hNormalFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	GetObject (hNormalFont, sizeof(LOGFONT), &lfSystemVariableFont );

	lfSystemVariableFont.lfWeight = FW_SEMIBOLD;
	iFontHeight = abs(lfSystemVariableFont.lfHeight)*2;

	hBoldFont = CreateFontIndirect ( &lfSystemVariableFont );
}

//Destroy all the controls and the main pane window.
void 
CPaneBase::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	DestroyWindow(hWnd);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function sets a pointer to the image containing the background for the
//pane window.
void 
CPaneBase::SetBackgroundImagePointer(CBitmap* poBitmap) {

	//oDoubleBufferBitmap = poBitmap;
	}

//This function sets the visibility of the pane
void 
CPaneBase::SetVisible(bool pbVisible) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	ShowWindow(hWnd, pbVisible ? SW_SHOW : SW_HIDE);
	
	oTrace->EndTrace(__WFUNCTION__, eAll);
}

//Paints the pane
void 
CPaneBase::DoPaint() {

	PAINTSTRUCT cPaintStruct;
	HDC hDC;
	Graphics*	oCanvas;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	//Start drawing
	SecureZeroMemory(&cPaintStruct, sizeof(cPaintStruct));
	hDC = BeginPaint(hWnd, &cPaintStruct);

	//Create a canvas for drawing to the screen
	oCanvas = new Graphics(hDC);

	//Draw the background
	if (oDoubleBufferBitmap!=NULL) {
		oCanvas->DrawImage(oDoubleBufferBitmap->GetBitmap(), 0, 0);
		}
	else {

		oCanvas->FillRectangle(oWhiteBrush,
							   cPaintStruct.rcPaint.left,
							   cPaintStruct.rcPaint.top,
							   cPaintStruct.rcPaint.right,
							   cPaintStruct.rcPaint.bottom);

		}

	DoDrawUnderlineHeaders(oCanvas);

	//End drawing
	EndPaint(hWnd, &cPaintStruct);

	oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function adds the line under the headings in the pane
void 
CPaneBase::DoDrawUnderlineHeaders(Graphics* poCanvas) {

	long lIndex;
	RECT cRectPane;
	RECT cRectControl;

	Pen blackPen(Color(255, 128, 128, 128), 1);
	GetWindowRect(hWnd, &cRectPane);

	lIndex = oPaneButtons.size();

	while (lIndex>0) {

		lIndex--;

		if (oPaneButtons[lIndex].hStaticControl!=NULL) {

			GetWindowRect(oPaneButtons[lIndex].hStaticControl, &cRectControl);

			poCanvas->DrawLine(&blackPen, cRectControl.left - cRectPane.left, cRectControl.bottom - cRectPane.top + 2, 
										  cRectControl.right - cRectPane.left, cRectControl.bottom - cRectPane.top + 2);
			}
		}	
	}


//This function draws the background of the checkbox
void 
CPaneBase::DoDrawCheckboxBackground(HWND phControl, HDC phDC) {

	RectF   cDestinationRect;
	RECT	cWindowRect;
	POINT   cPosition;
	Graphics* oCanvas;

	//Get position of control in client
	GetWindowRect(phControl, &cWindowRect);
	cPosition.x = cWindowRect.left;
	cPosition.y = cWindowRect.top;
	ScreenToClient(hWnd, &cPosition);

	oCanvas = new Graphics(phDC);

	cDestinationRect.X = 0;
	cDestinationRect.Y = 0;
	cDestinationRect.Width = (REAL)cWindowRect.right - cWindowRect.left;
	cDestinationRect.Height = (REAL)cWindowRect.bottom - cWindowRect.top;
	
	if (oDoubleBufferBitmap!=NULL) {
		oCanvas->DrawImage(oDoubleBufferBitmap->GetBitmap(),
			cDestinationRect,
			(REAL)cPosition.x,
			(REAL)cPosition.y,
			(REAL)cDestinationRect.Width,
			(REAL)cDestinationRect.Height,
			UnitPixel,
			NULL,
			NULL,
			NULL);
		}
	else {
		
		oCanvas->FillRectangle(oWhiteBrush, cDestinationRect);
		}

	delete oCanvas;
	}

//This function draws the static controls
void 
CPaneBase::DoDrawStaticControls(LPDRAWITEMSTRUCT oDrawItemInfo) {

	RectF   cDestinationRect;
	RECT	cWindowRect;
	POINT   cPosition;
	Graphics* oCanvas;
	UINT iFormat;
	long lIndex;

	//Get position of control in client
	GetWindowRect(oDrawItemInfo->hwndItem, &cWindowRect);
	cPosition.x = cWindowRect.left;
	cPosition.y = cWindowRect.top;
	ScreenToClient(hWnd, &cPosition);

	oCanvas = new Graphics(oDrawItemInfo->hDC);

	cDestinationRect.X = 0;
	cDestinationRect.Y = 0;
	cDestinationRect.Width = (REAL)oDrawItemInfo->rcItem.right;
	cDestinationRect.Height = (REAL)oDrawItemInfo->rcItem.bottom;
	
	if (oDoubleBufferBitmap!=NULL) {
		oCanvas->DrawImage(oDoubleBufferBitmap->GetBitmap(),
			cDestinationRect,
			(REAL)cPosition.x,
			(REAL)cPosition.y,
			(REAL)oDrawItemInfo->rcItem.right,
			(REAL)oDrawItemInfo->rcItem.bottom,
			UnitPixel,
			NULL,
			NULL,
			NULL);
		}
	else {
		
		oCanvas->FillRectangle(oWhiteBrush, cDestinationRect);
		}
	

	//Draw the text
	wchar_t cText[50];
	GetWindowText(oDrawItemInfo->hwndItem, cText, 49);

	//To do: Terrible solution, the text will be centered when the length is 1.
	//Maybe in the future enhance it with \ codes, ie \c for center.
	iFormat = DT_LEFT;

	if (wcslen(cText)>2) {

		if (wcsncmp(cText, L"\\c", 2)==0) iFormat = DT_CENTER;
		if (wcsncmp(cText, L"\\r", 2)==0) iFormat = DT_RIGHT;

		if (iFormat!=DT_LEFT) {
			wcscpy_s(cText, 50, cText+2);
			}
		}
	
	iFormat |= DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;

	SetBkMode(oDrawItemInfo->hDC, TRANSPARENT);
	
	//check if the handle is a static pane control
	lIndex = oPaneButtons.size();

	while (lIndex>0) {

		lIndex--;

		if (oPaneButtons[lIndex].hStaticControl == oDrawItemInfo->hwndItem) {
			SetTextColor(oDrawItemInfo->hDC, RGB(96,96,96));
			}
		}
	
	DrawText(oDrawItemInfo->hDC,
			 cText,
			 -1,
			 &oDrawItemInfo->rcItem,
			 iFormat);

	delete oCanvas;
	}

//This function adds an image to the pane button structure
HWND 
CPaneBase::AddPaneButton(int piCommand, int piCaption, int piImage, int piExtraSpace, float pfHeightMultiply) {

	sPaneButton cNewButton; 

	oTrace->StartTrace(__WFUNCTION__, eAll);

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	cNewButton.bEnabled = true;
	cNewButton.bCollapsed = false;
	cNewButton.iCommandId = piCommand;
	cNewButton.iTextResourceId = piCaption;
	cNewButton.iExtraSpace = piExtraSpace;
	cNewButton.hStaticControl = NULL;
	cNewButton.fHeightMultiply = pfHeightMultiply;

	cNewButton.oPaneButton = new CButton(oTrace);
	cNewButton.oPaneButton->Create(hWnd, oGlobalInstances.hInstance, piCommand, 
							20, 20, 20, 20, 0);
	cNewButton.oPaneButton->SetImage(piImage);
	SendMessage(cNewButton.oPaneButton->hWnd, WM_SETFONT, (WPARAM)hFont, 0);

	oPaneButtons.push_back(cNewButton);

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return cNewButton.oPaneButton->hWnd;
	}

HWND 
CPaneBase::AddPaneButton(int piCommand, int piCaption, std::wstring psImage, int piExtraSpace, float pfHeightMultiply) {

	sPaneButton cNewButton; 

	oTrace->StartTrace(__WFUNCTION__, eAll);

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	cNewButton.bEnabled = true;
	cNewButton.iCommandId = piCommand;
	cNewButton.iTextResourceId = piCaption;
	cNewButton.iExtraSpace = piExtraSpace;
	cNewButton.hStaticControl = NULL;
	cNewButton.fHeightMultiply = pfHeightMultiply;

	cNewButton.oPaneButton = new CButton(oTrace);
	cNewButton.oPaneButton->Create(hWnd, oGlobalInstances.hInstance, piCommand, 
							20, 20, 20, 20, 0);
	cNewButton.oPaneButton->SetImage(psImage);
	SendMessage(cNewButton.oPaneButton->hWnd, WM_SETFONT, (WPARAM)hFont, 0);

	oPaneButtons.push_back(cNewButton);

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return cNewButton.oPaneButton->hWnd;
	}

//This function adds an image to the pane button structure
HWND 
CPaneBase::AddPaneStatic(int piCaption, int piExtraSpace, float pfHeightMultiply) {

	sPaneButton cNewStatic; 

	oTrace->StartTrace(__WFUNCTION__, eAll);

	CPaneBase::Initialize();

	cNewStatic.bEnabled = true;
	cNewStatic.iCommandId = 0;
	cNewStatic.bCollapsed = false;
	cNewStatic.iTextResourceId = piCaption;
	cNewStatic.iExtraSpace = piExtraSpace;
	cNewStatic.oPaneButton = NULL;
	cNewStatic.hStaticControl = AddStaticControl(L"");
	cNewStatic.fHeightMultiply = pfHeightMultiply;

	//SendMessage(cNewStatic.hStaticControl, WM_SETFONT, (WPARAM)hBoldFont, 0);
	SendMessage(cNewStatic.hStaticControl, WM_SETFONT, (WPARAM)hNormalFont, 0);

	oPaneButtons.push_back(cNewStatic);

	oTrace->EndTrace(__WFUNCTION__, eAll);
	
	return cNewStatic.hStaticControl;
	}

//This function localizes the caption of all pane buttons
void 
CPaneBase::LocalizePaneButtons(HINSTANCE phLanguage) {

	wchar_t* cText;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oGlobalInstances.hLanguage = phLanguage;

	lIndex = (long)oPaneButtons.size();
	
	if (lIndex>0) {

		//Alloc text buffer
		cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

		while (lIndex>0) {
			
			lIndex--;

			LoadString(phLanguage, oPaneButtons[lIndex].iTextResourceId, cText, 400);
			
			if (oPaneButtons[lIndex].oPaneButton!=NULL) {
			
				oPaneButtons[lIndex].oPaneButton->SetText(cText);
				}
			else {
				
				SetWindowText(oPaneButtons[lIndex].hStaticControl, cText);
				}
			}

		//Free memory
		free(cText);
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function adds an static control
HWND
CPaneBase::AddStaticControl(std::wstring psLabel) {

	HWND hResult;

	hResult = CreateWindowEx(0L, L"STATIC", psLabel.c_str(), 
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | SS_NOTIFY,
		20, 210, 190, 20, hWnd, (HMENU)-1, 
		oGlobalInstances.hInstance, 0);	
	oToBeDeletedHWND.push_back(hResult);

	return hResult;
	}
//This function positions all the pane buttons in the give rectangle
void 
CPaneBase::PositionPaneButtons(RectF pcButtonsControlRect) {

	int iFontHeight;
	HWND hPaneControl; 

	oTrace->StartTrace(__WFUNCTION__, eExtreme);

	iFontHeight = 20;
	iPaneMinimumHeight = 0;

	for (long lIndex=0; lIndex<(long)oPaneButtons.size(); lIndex++) {

		hPaneControl = oPaneButtons[lIndex].oPaneButton!=NULL ? oPaneButtons[lIndex].oPaneButton->hWnd : oPaneButtons[lIndex].hStaticControl;

		if (oPaneButtons[lIndex].oPaneButton!=NULL) {

			MoveWindow(hPaneControl, 
					(int)pcButtonsControlRect.X+10, 
					(int)pcButtonsControlRect.Y+10, 
					(int)pcButtonsControlRect.Width-20, 
					(int)(iFontHeight*1.5*oPaneButtons[lIndex].fHeightMultiply), true);
			}
		else {

			MoveWindow(hPaneControl, 
					(int)pcButtonsControlRect.X+10, 
					(int)pcButtonsControlRect.Y+10, 
					(int)pcButtonsControlRect.Width-20, 
					(int)(iFontHeight*oPaneButtons[lIndex].fHeightMultiply), true);
			}


		pcButtonsControlRect.Y += (int)(iFontHeight * 1.5 * oPaneButtons[lIndex].fHeightMultiply) + 10;
		pcButtonsControlRect.Y += oPaneButtons[lIndex].iExtraSpace;

		iPaneMinimumHeight = (int)(pcButtonsControlRect.Y + 10);
		}

	oTrace->EndTrace(__WFUNCTION__, eExtreme);
	}

//This function is called when the size message is received. It
//checks if the complete pane fits inside the client window and if not
//will add a scrollbar
void 
CPaneBase::OnResizePane() {

	//long lWindowStyle;
	//RECT cClientRect;
	//bool bScrollbar;
	//SCROLLINFO cNewScrollInfo;
	//
	//
	//oTrace->StartTrace(__WFUNCTION__, eExtreme);

	//lWindowStyle = GetWindowLong(hWnd, GWL_STYLE);
	//GetClientRect(hWnd, &cClientRect);

	//bScrollbar = (cClientRect.bottom < iPaneMinimumHeight);

	////Show or hide the scrollbar
	//if (bScrollbar) {

	//	lWindowStyle |= WS_VSCROLL;
	//	SetWindowLong(hWnd, GWL_STYLE, lWindowStyle);
	//	InvalidateRect(hWnd, NULL, TRUE);
	//	//iScrollPosition = 50;

	//	//Process vertical scrollbar
	//	cNewScrollInfo.nPage = cClientRect.bottom;
	//	cNewScrollInfo.nMax = cClientRect.bottom;

	//	if (iPaneMinimumHeight > cClientRect.bottom) {

	//		if (cSBIVertical.nPos < 0) cSBIVertical.nPos = 0;

	//		cNewScrollInfo.nMin = 0;
	//		cNewScrollInfo.nMax = iPaneMinimumHeight-1;
	//		cNewScrollInfo.nPage = cClientRect.bottom;
	//		cNewScrollInfo.nPos = cSBIVertical.nPos;

	//		if (cNewScrollInfo.nPos + (int)cNewScrollInfo.nPage > cNewScrollInfo.nMax) {
	//			cNewScrollInfo.nPos = cNewScrollInfo.nMax - cNewScrollInfo.nPage;
	//			//iScrollPosition = (int)cNewScrollInfo.nPos;
	//			//Redraw();
	//			}
	//		}
	//
	//	cNewScrollInfo.cbSize = sizeof(SCROLLINFO);
	//	cNewScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	//	UpdateScrollbar(SB_VERT, cNewScrollInfo);
	//	//SetScrollInfo(hWnd, SB_VERT, &cNewScrollInfo, TRUE);

	//	}
	//else {

	//	lWindowStyle &= ~WS_VSCROLL;
	//	SetWindowLong(hWnd, GWL_STYLE, lWindowStyle);
	//	InvalidateRect(hWnd, NULL, TRUE);

	//	InitializeScrollInfo();
	//	//iScrollPosition = 0;
	//	}

	//oTrace->EndTrace(__WFUNCTION__, eExtreme);
}

//This function sizes the pane to the give size and moves it
//to the given position
void 
CPaneBase::Resize(int piX, int piY, int piWidth, int piHeight) {

	MoveWindow(hWnd, piX, piY, piWidth-2, piHeight, TRUE);

	OnResizePane();
}



//A message handler for the PaneBase
bool 
CPaneBase::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {

	bool bReturn;

	bReturn = false;
	if (phWnd==this->hWnd) bReturn = CWindowBase::WndProc(phWnd, message, wParam, lParam, piReturn);

	switch (message) {

		case WM_WINDOWPOSCHANGED:

			if (phWnd==this->hWnd) {
				OnResizePane();
				//bReturn = true;
				}
			break;
		}
	//Walk through the message handler of the pane buttons. This will
	//make the icons appear
	for (long lIndex=0; lIndex<(long)oPaneButtons.size() && !bReturn; lIndex++) {

		if (oPaneButtons[lIndex].oPaneButton!=NULL) {
			bReturn = oPaneButtons[lIndex].oPaneButton->WndProc(phWnd, message, wParam, lParam, piReturn);
			}
		else
		{
		/*	if (oPaneButtons[lIndex].hStaticControl == phWnd)
			{
				if (message == WM_COMMAND)
				{
					oPaneButtons[lIndex].bCollapsed = !oPaneButtons[lIndex].bCollapsed;
				}
			}*/
		}
		}

	return bReturn;
	}






