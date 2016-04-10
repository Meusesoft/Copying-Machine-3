#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "Pane.h"
#include <math.h> 

CPane::CPane(sGlobalInstances poGlobalInstances) :
	   CWindowContainer(poGlobalInstances, -1, L"Pane") {

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	
	oPaneScroller		= NULL;
	oCurrentPane		= NULL;
	oPaneCopy			= NULL;
	oPaneImage			= NULL;
	oPaneDocument       = NULL;
	oPaneActions		= NULL;

	bFirstSizeEvent = true;
	}

CPane::~CPane(void) {

	Destroy();
	}

//Create the copy toolbar
bool 
CPane::Create(HINSTANCE phInstance, HWND phParent, int pID) {
	
	sWindowContainerButton cWCButton;
	
	oTrace->StartTrace(__WFUNCTION__);

	CWindowContainer::Create(phParent, phInstance);

	//Create the scroller
	oPaneScroller = new CPaneScroller(oGlobalInstances);
	oPaneScroller->Create(phInstance, hWnd);
	AddChildWindow(oPaneScroller);
	SetContainedWindow(oPaneScroller->hWnd);

	//Create the panes
	oPaneCopy = new CPaneCopy(oGlobalInstances, ID_VIEW_PANECOPY);
	oPaneCopy->Create(phInstance, oPaneScroller->hWnd);
	oPanes.push_back(oPaneCopy);
	oPaneScroller->AddChildWindow(oPaneCopy);

	oPaneImage = new CPaneImage(oGlobalInstances, ID_VIEW_PANEIMAGE);
	oPaneImage->Create(phInstance, oPaneScroller->hWnd);
	oPanes.push_back(oPaneImage);
	oPaneScroller->AddChildWindow(oPaneImage);

	oPaneDocument = new CPaneDocument(oGlobalInstances, ID_VIEW_PANEPAGE);
	oPaneDocument->Create(phInstance, oPaneScroller->hWnd);
	oPanes.push_back(oPaneDocument);
	oPaneScroller->AddChildWindow(oPaneDocument);

	oPaneActions = new CPaneActions(oGlobalInstances, 0);
	oPaneActions->Create(phInstance, hWnd);
	oPaneActions->SetVisible(true);
	hFixedWnd = oPaneActions->hWnd;

	//Set initial visible pane and the icons of the panes
	ePaneType cVisiblePane;

	cVisiblePane = (ePaneType)oGlobalInstances.oRegistry->ReadInt(L"Window", L"Pane", 0);
	SetVisiblePane(oPaneCopy);


	long lIndex = (long)oPanes.size();

	while (lIndex>0) {

		lIndex--;

		//Add the windowcontainer buttons
		cWCButton.cRect				= Rect(0,0,0,0);
		cWCButton.hCommandWindow	= GetParent(hWnd);
		cWCButton.iCommand			= oPanes[lIndex]->iMenuCommand;
		cWCButton.sImage			= oPanes[lIndex]->sImage;
		cWCButton.iTooltip			= oPanes[lIndex]->iTitle;
		cWCButton.bImageHot			= (oPanes[lIndex]->cType == cVisiblePane);
		cWCButton.bVisible			= true;
		AddButton(cWCButton);

		//Set the visibility
		if (oPanes[lIndex]->cType == cVisiblePane) SetVisiblePane(oPanes[lIndex]);
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//Destroy all the controls and the main pane window.
void 
CPane::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	//Save current pane to registry
	oGlobalInstances.oRegistry->WriteInt(L"Window", L"Pane", (int)oCurrentPane->cType);	

	//Save current scroll position
	oGlobalInstances.oRegistry->WriteInt(L"Window", L"PaneScrollPos", oPaneScroller->GetVerticalPosition());	

	//Clean up
	if (oPaneCopy)			delete oPaneCopy;
	if (oPaneImage)			delete oPaneImage;
	if (oPaneDocument)		delete oPaneDocument;
	if (oPaneActions)		delete oPaneActions;

	CWindowContainer::Destroy();

	oTrace->EndTrace(__WFUNCTION__);
	}

void 
CPane::Localize(HINSTANCE phLanguage) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	CWindowContainer::Localize(phLanguage);
		
	if (oPaneCopy) oPaneCopy->Localize(phLanguage);
	if (oPaneImage) oPaneImage->Localize(phLanguage);
	if (oPaneDocument) oPaneDocument->Localize(phLanguage);
	if (oPaneActions) oPaneActions->Localize(phLanguage);

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function sets the visible pane
void 
CPane::SetVisiblePane(CPaneBase* poCurrentPane) {

	oTrace->StartTrace(__WFUNCTION__);
	sWindowContainerButton cButton;

	if (poCurrentPane != oCurrentPane) {

		if (oCurrentPane) {
			
			oCurrentPane->SetVisible(false);

			GetButton(oCurrentPane->iMenuCommand, cButton);
			cButton.bImageHot = false;
			UpdateButton(oCurrentPane->iMenuCommand, cButton);
			}
			
		oCurrentPane = poCurrentPane;
		oPaneScroller->SetVisiblePane(poCurrentPane);

		oCurrentPane->SetVisible(true);
		
		GetButton(oCurrentPane->iMenuCommand, cButton);
		cButton.bImageHot = true;
		UpdateButton(oCurrentPane->iMenuCommand, cButton);

		DoSetTitle();

		//SetContainedWindow(oCurrentPane->hWnd);

		InvalidateRect(hWnd, NULL, TRUE);
		
		oCore->oNotifications->CommunicateEvent(eNotificationPaneChange, (void*)oCurrentPane->iMenuCommand);
		}

	oTrace->EndTrace(__WFUNCTION__, poCurrentPane->sTitle.c_str());
}

//This function returns the current visible pane
CPaneBase* 
CPane::GetVisiblePane() {
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return oCurrentPane;
}

//This function processes all the messages, it returns true if it processed a message
bool 
CPane::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {

	bool bReturn;

	bReturn = false;

	switch (message) {

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:

			if (phWnd == this->hWnd) {
				
				//OnButtonDown(LOWORD(lParam), HIWORD(lParam));

				//bReturn = true;
				}
			else {
				
				long lIndex = oPanes.size();

				//Test if the button is down on one of the panes and if so
				//show the menu
				while (lIndex>0 && !bReturn) {

					lIndex--;

					if (phWnd == oPanes[lIndex]->hWnd) {

						DoSelectPaneMenu();
						bReturn = true;
						}					
					}
				}
			break;

		case WM_SIZE:

			if (phWnd == this->hWnd && bFirstSizeEvent) {
			
				oPaneScroller->SetVerticalPosition(oGlobalInstances.oRegistry->ReadInt(L"Window", L"PaneScrollPos", 0));
			}

			break;

		case WM_PAINT:


			if (phWnd == this->hWnd) {

				DoPaint();
				bReturn = true;
				bFirstSizeEvent = false; // stop changing the vertical position of the scrollbar. The window is initialized and shown in is initial state.
				}

			break;
		

		break;
	
	}



	if (!bReturn) bReturn = oPaneActions->WndProc(phWnd, message, wParam, lParam, piReturn);
	if (!bReturn) bReturn = CWindowContainer::WndProc(phWnd, message, wParam, lParam, piReturn);
	//if (oPaneCopy && !bReturn) bReturn = oPaneCopy->WndProc(phWnd, message, wParam, lParam, piReturn);
	//if (oPaneImage && !bReturn) bReturn = oPaneImage->WndProc(phWnd, message, wParam, lParam, piReturn);
	//if (oPaneDocument && !bReturn) bReturn = oPaneDocument->WndProc(phWnd, message, wParam, lParam, piReturn);

	return bReturn;
	}

//This function processes the button down event
void
CPane::OnButtonDown(int piX, int piY) {

	CWindowContainer::OnButtonDown(piX, piY);

/*	bool bFound;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__);

	bFound = false;
	lIndex = oPaneSwitchButtons.size();

	while (lIndex>0 && !bFound) {

		lIndex--;

		if (oPaneSwitchButtons[lIndex].cRect.Contains(piX, piY)) {

			//Send a command to change the visible pane
			SendMessage(GetParent(hWnd), WM_COMMAND, oPanes[oPaneSwitchButtons[lIndex].iIndex]->iMenuCommand, 0);

			bFound = true;
			}
		}

	if (!bFound) DoSelectPaneMenu();

	oTrace->EndTrace(__WFUNCTION__);*/
	}


//This function builds and shows a popup menu with all available panes
void 
CPane::DoSelectPaneMenu() {

	HMENU		hPaneMenu;
	CPaneBase*	oPane;
	RECT		cWindowRect;
	UINT		iFlags;

	oTrace->StartTrace(__WFUNCTION__);

	//Create the menu;
	hPaneMenu = CreatePopupMenu();

	for (long lIndex=0; lIndex<(long)oPanes.size(); lIndex++) {

		oPane = oPanes[lIndex];

		iFlags = MF_STRING/* | MF_OWNERDRAW*/;
		if (oCurrentPane == oPane) iFlags |= MF_CHECKED;
		
		AppendMenu(hPaneMenu, iFlags, oPane->iMenuCommand, oPane->sTitle.c_str());  

		}

	oGlobalInstances.oMenuIcon->ProcessMenu(hPaneMenu);

	//Show the menu
	GetWindowRect(hWnd, &cWindowRect);

	//TrackPopupMenu(hPaneMenu, TPM_TOPALIGN | TPM_CENTERALIGN | TPM_VERPOSANIMATION,
	//			   cWindowRect.left + (cWindowRect.right - cWindowRect.left)/2, 
	//			   cWindowRect.top + 20, 0, hWnd, NULL);

	POINT cPoint;
	GetCursorPos(&cPoint);

	TrackPopupMenu(hPaneMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_VERPOSANIMATION,
				   cPoint.x, 
				   cPoint.y, 0, hWnd, NULL);

	//Destroy the menu
	DestroyMenu(hPaneMenu);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function draws the images onto the pane
void 
CPane::DoPaint() {

	PAINTSTRUCT cPaintStruct;
	HDC hDC;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	//Start drawing
	SecureZeroMemory(&cPaintStruct, sizeof(cPaintStruct));
	hDC = BeginPaint(hWnd, &cPaintStruct);

	CWindowContainer::DoPaint(hDC);

	//Draw the title of this container
/*	RECT		cWindowRect;
	Rect		cSwitchButtonRect;
	GetWindowRect(hWnd, &cWindowRect);

	sPaneSwitchButton cPaneSwitchButton;

	cSwitchButtonRect.X      = cWindowRect.right - cWindowRect.left - 20;
	cSwitchButtonRect.Y      = 2;
	cSwitchButtonRect.Width  = 16;
	cSwitchButtonRect.Height = 17;

	lIndex = oPanes.size();
	lPanes = lIndex;

	while (lIndex>0) {
	
		lIndex--;

		//Add the switch button to the vector of switch buttons
		cPaneSwitchButton.cRect = cSwitchButtonRect;
		cPaneSwitchButton.iIndex = lIndex;
		oPaneSwitchButtons.push_back(cPaneSwitchButton);

		//Determine if the pane is selected, if so the 'hot' image is drawn.
		hImagelist = hWCButtonsDefault;
		if (oCurrentPane == oPanes[lIndex]) hImagelist = hWCButtonsHot;

		//Draw the image
		ImageList_Draw(hImagelist, lIndex, hDC, 
					cSwitchButtonRect.X, 
					cSwitchButtonRect.Y, 
					ILD_TRANSPARENT);

		//Adjust the window rect
		cSwitchButtonRect.X -= 20;
	}
	*/
	//End drawing
	EndPaint(hWnd, &cPaintStruct);

	oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function sets the title of the pane container
void 
CPane::DoSetTitle() {

	wchar_t* cTextTemplate;
	wchar_t cValue[400];

	//Alloc text buffer
	cTextTemplate = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	//Create title string
	LoadString(oGlobalInstances.hLanguage, IDS_PANETITLE, cTextTemplate, 400);

	swprintf_s(cValue, 400, cTextTemplate, oCurrentPane->sTitle.c_str());

	SetTitle(cValue);	
	
	//Free memory
	free(cTextTemplate);
}

//This function sizes the pane to the give size and moves it
//to the given position
void 
CPane::Resize(int piX, int piY, int piWidth, int piHeight) {

	MoveWindow(hWnd, piX, piY, piWidth, piHeight, TRUE);
	}

//This function is called when the size message is received. It
//checks if the complete pane fits inside the client window and if not
//will add a scrollbar
void 
CPane::OnResizePane() {

	//oTrace->StartTrace(__WFUNCTION__, eExtreme);

	//lWindowStyle = GetWindowLong(hPaneScroller, GWL_STYLE);
	//GetClientRect(hPaneScroller, &cClientRect);

	//bScrollbar = (cClientRect.bottom < oCurrentPane->iPaneMinimumHeight);

	////Show or hide the scrollbar
	//if (bScrollbar) {

	//	lWindowStyle |= WS_VSCROLL;
	//	SetWindowLong(hPaneScroller, GWL_STYLE, lWindowStyle);
	//	//iScrollPosition = 50;

	//	}
	//else {

	//	lWindowStyle &= ~WS_VSCROLL;
	//	SetWindowLong(hPaneScroller, GWL_STYLE, lWindowStyle);
	//	//iScrollPosition = 0;
	//	}

	//oTrace->EndTrace(__WFUNCTION__, eExtreme);
}

//This function resizes the visible pane
void 
CPane::ResizeVisiblePane() {

	//RECT cClientRect;
	//RECT cPaneRect;

	//OnResizePane();

	//GetClientRect(hPaneScroller, &cClientRect);
	//
	//if (oCurrentPane) {
	//	
	//	GetClientRect(oCurrentPane->hWnd, &cPaneRect);

	//	MoveWindow(oCurrentPane->hWnd, 0, 0, cClientRect.right, oCurrentPane->iPaneMinimumHeight, TRUE);

	//	InvalidateRect(hPaneScroller, NULL, TRUE);
	//	InvalidateRect(oCurrentPane->hWnd, NULL, TRUE);
	//	}
	}

//Event handler for the WM_SIZE event
bool 
CPane::OnSize() {

//	CWindowContainer::OnSize();

	RECT		cWindowRect;

	LayoutChildWindow();	

	GetWindowRect(hWnd, &cWindowRect);

	//MoveWindow(oPaneScroller->, 
	//			0,//cWindowRect.left,
	//			20,//cWindowRect.top + 20,
	//			cWindowRect.right - cWindowRect.left,
	//			cWindowRect.bottom - cWindowRect.top - 20,
	//			true);

	//determine the position of the switch buttons and make tooltips
/*	RECT		cWindowRect;
	Rect		cSwitchButtonRect;
	sPaneSwitchButton cPaneSwitchButton;
	long lIndex;
	long lPanes;

	GetWindowRect(hWnd, &cWindowRect);

	cSwitchButtonRect.X      = cWindowRect.right - cWindowRect.left - 20;
	cSwitchButtonRect.Y      = 2;
	cSwitchButtonRect.Width  = 16;
	cSwitchButtonRect.Height = 17;

	lIndex = oPanes.size();
	lPanes = lIndex;

	while (lIndex>0) {
	
		lIndex--;

		//Add the switch button to the vector of switch buttons
		cPaneSwitchButton.cRect = cSwitchButtonRect;
		cPaneSwitchButton.iIndex = lIndex;
		oPaneSwitchButtons.push_back(cPaneSwitchButton);

		//Update the rect of the tooltip
		DoTooltipUpdateRect(lIndex, cSwitchButtonRect);

		//Adjust the window rect
		cSwitchButtonRect.X -= 20;
	}*/

	return CWindowContainer::OnSize();
}
