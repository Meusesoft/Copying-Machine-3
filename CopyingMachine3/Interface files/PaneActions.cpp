#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "PaneActions.h"
#include <math.h> 

CPaneActions::CPaneActions(sGlobalInstances poGlobalInstances, int piMenuCommand) : 
	CPaneBase(poGlobalInstances, piMenuCommand, L"PaneActions") {

	cType = ePaneActions;
	sImage = L"";
	iTitle = IDS_ACTIONS;
	}

CPaneActions::~CPaneActions(void) {

	Destroy();
	}

//Create the copy toolbar
bool 
CPaneActions::Create(HINSTANCE phInstance, HWND phParent) {
	
	CPaneBase::oTrace->StartTrace(__WFUNCTION__);

	CPaneBase::Create(phInstance, phParent);

	Initialize();

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//Destroy all the controls and the main pane window.
void 
CPaneActions::Destroy() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__);

	CPaneBase::oCore->oNotifications->UnsubscribeAll(hWnd);

	CPaneBase::Destroy();

	CPaneBase::oTrace->EndTrace(__WFUNCTION__);
	}

//Initialize the Copy pane
void 
CPaneActions::Initialize() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__);

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	AddPaneStatic(IDS_ACTIONS, 0);
	hButtonAcquire	= AddPaneButton(ID_SCAN_ACQUIRE,		IDS_ACQUIRE,	L"PNG_TOOLBOOKADD", 0,	1.4f);
	hButtonPrint	= AddPaneButton(IDC_DOOUTPUTDOCUMENT,	IDS_PRINT,		L"PNG_TOOLPRINT", 0, 1.4f);
	hButtonCopy		= AddPaneButton(IDC_DOCOPY,				IDS_COPY,		L"PNG_TOOLCOPY", 0,	1.4f);

	//Fill and enable the controls
	DoEnableControls();

	//Localize the labels
	Localize(CPaneBase::oGlobalInstances.hLanguage);

	//Position
	DoPositionControls();

	CPaneBase::oTrace->EndTrace(__WFUNCTION__);
	}

void 
CPaneActions::Localize(HINSTANCE phLanguage) {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	CPaneBase::oGlobalInstances.hLanguage = phLanguage;

	CPaneBase::LocalizePaneButtons(phLanguage);

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function sets a pointer to the image containing the background for the
//pane window.
void 
CPaneActions::SetBackgroundImagePointer(CBitmap* poBitmap) {

	oDoubleBufferBitmap = poBitmap;
	}

//This function handles the events from the core.
bool 
CPaneActions::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;

	bReturn = false;

	/*while (CPaneBase::oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {


			}
		}*/

	return bReturn;
	}


void 
CPaneActions::UpdateControls() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	DoEnableControls();

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function positions the controls in the pane window
void
CPaneActions::DoPositionControls() {

	RECT cClientRect;

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);

	GetClientRect(hWnd, &cClientRect);

	cButtonsControlRect.X = 20;
	cButtonsControlRect.Y = 10;
	cButtonsControlRect.Width = cClientRect.right - (cButtonsControlRect.X * 2);

	PositionPaneButtons(cButtonsControlRect);

	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}


//This function processes all the messages, it returns true if it processed a message
bool 
CPaneActions::WndProc(HWND phWnd, UINT message, 
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
				
				//We processed this message
				bReturn = true;
				piReturn = 0;
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

		default:

			if (!bReturn) bReturn = CPaneBase::WndProc(phWnd, message, wParam, lParam, piReturn);
			break;

			}
		
	return bReturn;
	}

void
CPaneActions::DoEnableControls() {

	CPaneBase::oTrace->StartTrace(__WFUNCTION__, eAll);


	CPaneBase::oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function is called when the scrollbar has been moved to reposition
//all the controls on the window
void 
CPaneActions::DoRepositionControls() {

	DoPositionControls();
}


