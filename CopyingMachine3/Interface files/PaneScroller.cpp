#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "PaneScroller.h"
#include <math.h> 

#define iMinimumPaneWidth 200

CPaneScroller::CPaneScroller(sGlobalInstances poGlobalInstances) :
	CWindowScrollbar(poGlobalInstances, L"PaneScroller") {

	oCurrentPane = NULL;
	SetContentSize(0, 0);
	}

CPaneScroller::~CPaneScroller() {

	
	
	
	Destroy();

	//Add notifications
	CCopyingMachineCore* oCore;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	oCore->oNotifications->UnsubscribeAll(hWnd);
	}

bool 
CPaneScroller::Create(HINSTANCE phInstance, HWND phParent) {

	//create the parent window for the individual pane. This
	//parent window will be used as a scroller window

	DWORD dwStyle = WS_TABSTOP | 
					WS_CHILD |
					WS_VISIBLE |
					WS_CLIPCHILDREN;

	hWnd = CreateWindowEx(0,						// ex style
						 L"MeusesoftPaneScroller",  // class name - defined in commctrl.h
						 NULL,                      // window text
						 dwStyle,                   // style
						 0,                         // x position
						 30,                        // y position
						 20,                        // width
						 20,                        // height
						 phParent,					// parent
						 (HMENU)1110,				// ID
						 phInstance,                // instance
						 NULL);                     // no extra data

	InitializeScrollInfo();

	//Add notifications
	CCopyingMachineCore* oCore;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	oCore->oNotifications->Subscribe(hWnd, eNotificationPaneSizeChange);

	return (hWnd!=NULL);
}

//This function process the scrollbar movements
void 
CPaneScroller::OnEventScrollbar(UINT message, WPARAM wParam, LPARAM lParam) {

	CWindowBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	CWindowScrollbar::OnEventScrollbar(message, wParam, lParam);

	LayoutChildWindow();

	CWindowBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

void 
CPaneScroller::LayoutChildWindow() {

	int iIndex;
	int iVerticalPosition;
	int iHorizontalPosition;
	RECT cClientRect;

	GetClientRect(hWnd, &cClientRect);
	iVerticalPosition = max(0, GetVerticalPosition());
	iHorizontalPosition = max(0, GetHorizontalPosition());

	iIndex = ChildWindowCount();

	while (iIndex>0) {

		iIndex--;

		MoveWindow(GetChildWindow(iIndex)->hWnd,
			-iHorizontalPosition,
			-iVerticalPosition,
			max(iMinimumPaneWidth, cClientRect.right),
			cClientRect.bottom + iVerticalPosition,
			true);
		}

	DoCheckScrollbars();
	}

//This function is called when the size message is received. It
//checks if the complete pane fits inside the client window and if not
//will add a scrollbar
void 
CPaneScroller::DoCheckScrollbars() {

	if (oCurrentPane!=NULL) {
		SetContentSize(iMinimumPaneWidth, 
					   oCurrentPane->iPaneMinimumHeight);
		}
	}

//This function sets the visible pane
void 
CPaneScroller::SetVisiblePane(CPaneBase* poCurrentPane) {

	oTrace->StartTrace(__WFUNCTION__);

	oCurrentPane = poCurrentPane;

	//Adjust the scroller to this new pane
	if (oCurrentPane!=NULL) {

		SetContentSize(iMinimumPaneWidth, oCurrentPane->iPaneMinimumHeight);
		}
	else {

		SetContentSize(0, 0);
		}


	oTrace->EndTrace(__WFUNCTION__);
}

//This function returns the current visible pane
CPaneBase* 
CPaneScroller::GetVisiblePane() {
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return oCurrentPane;
}

//This function process the size event
bool
CPaneScroller::OnSize() {

	LayoutChildWindow();	

	return CWindowScrollbar::OnSize();
	}

//This function handles the events from the core.
bool 
CPaneScroller::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;
	CCopyingMachineCore* oCore;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	bReturn = false;

	while (oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationPaneSizeChange:

				//Check if the scrollbars needs to be adjusted
				DoCheckScrollbars();

				//Remove pending notifications to prevent the controls being updated
				//multiple times
				oCore->oNotifications->RemoveNotificationSpecific(this->hWnd, eNotificationPaneSizeChange);
				break;
			}
		}

	return bReturn;
	}


//This function process the window events
bool 
CPaneScroller::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {

	bool bReturn;

	bReturn = false;

	switch (message) {

		case WM_CORENOTIFICATION:

			if (phWnd == hWnd) {

				bReturn = OnEventCoreNotification();
				}
			bReturn=false;
			break;

	//	case WM_SIZE:
	//		if (phWnd==this->hWnd) {
	//			bReturn = true;
	//			}
	//		break;
		}

	if (!bReturn) bReturn = CWindowScrollbar::WndProc(phWnd, message, wParam, lParam, piReturn);
	
	return bReturn;
	}
