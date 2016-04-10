#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "PaneImage.h"

CPaneImage::CPaneImage(sGlobalInstances poGlobalInstances, int piMenuCommand) : 
	CPaneBase(poGlobalInstances, piMenuCommand, L"PaneImage") {

	cType = ePaneImage;
	sImage = L"PNG_TOOLPANEIMAGE";
	iTitle = IDS_PAGE;
	}

CPaneImage::~CPaneImage(void) {

	Destroy();
	}

//Create the copy toolbar
bool 
CPaneImage::Create(HINSTANCE phInstance, HWND phParent) {
	
	oTrace->StartTrace(__WFUNCTION__);

	CPaneBase::Create(phInstance, phParent);

	Initialize();

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//Destroy all the controls and the main pane window.
void 
CPaneImage::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	oCore->oNotifications->UnsubscribeAll(hWnd);

	CPaneBase::Destroy();	

	oTrace->EndTrace(__WFUNCTION__);
	}

//Initialize the Copy pane
void 
CPaneImage::Initialize() {

	oTrace->StartTrace(__WFUNCTION__);

	//Add controls to the pane
	AddPaneStatic(IDS_TOOLS, -10);
	AddPaneButton(ID_IMAGE_CROP,	IDS_CROPIMAGE, L"PNG_TOOLCROP", 0);
	AddPaneButton(ID_IMAGE_FILLSELECTION,IDS_FILLSELECTION, L"PNG_TOOLPAINT", 0);
	AddPaneButton(ID_IMAGE_RECOGNIZETEXT,IDS_RECOGNIZETEXT, L"PNG_TOOLOCR", 10);
	AddPaneStatic(IDS_ROTATEANDFLIP, -10);
	AddPaneButton(ID_ROTATE_90,		IDS_ROTATE90, L"PNG_TOOLROTATE90", 0);
	AddPaneButton(ID_ROTATE_180,	IDS_ROTATE180, L"PNG_TOOLROTATE180", 0);
	AddPaneButton(ID_ROTATE_270,	IDS_ROTATE270, L"PNG_TOOLROTATE270", 10);
	AddPaneButton(ID_ROTATEANDFLIP_FLIPHORIZONTALLY, IDS_FLIPHORIZONTAL, L"PNG_TOOLFLIPHORIZONTAL", 00);
	AddPaneButton(ID_ROTATEANDFLIP_FLIPVERTICALLY, IDS_FLIPVERTICAL, L"PNG_TOOLFLIPVERTICAL", 10);
	AddPaneStatic(IDS_COLORDEPTH, -10);
	AddPaneButton(ID_CONVERTTO_GRAYSCALE, IDS_CONVERTGRAYSCALE, L"PNG_TOOLCONVERTGRAYSCALE", 0);
	AddPaneButton(ID_CONVERTTO_BLACKWHITE, IDS_CONVERTBLACKWHITE, L"PNG_TOOLCONVERTBLACKWHITE", 0);

	//Localize the labels
	Localize(CPaneBase::oGlobalInstances.hLanguage);

	//Subscribe to notifications
	oCore->oNotifications->Subscribe(hWnd, eNotificationPageSettingUpdate);
	oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentViewChanges);
	oCore->oNotifications->Subscribe(hWnd, eNotificationPageUpdate);
	oCore->oNotifications->Subscribe(hWnd, eNotificationPageViewChanges);	

	//Enable controls
	DoEnableControls();

	oTrace->EndTrace(__WFUNCTION__);
	}

void 
CPaneImage::Localize(HINSTANCE phLanguage) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	wchar_t* cText;

	oGlobalInstances.hLanguage = phLanguage;

	//Alloc text buffer
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	//Update title
	LoadString(phLanguage, IDS_PAGE, cText, 400);
	sTitle = cText;

	//Update pane buttons
	LocalizePaneButtons(phLanguage);

	//Free memory
	free(cText);
	
	oTrace->EndTrace(__WFUNCTION__, eAll);
	}


//This function handles the events from the core.
bool 
CPaneImage::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;

	bReturn = false;

	while (oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationPageSettingUpdate:
			case eNotificationDocumentViewChanges:
			case eNotificationPageUpdate:
			case eNotificationPageViewChanges:

				DoEnableControls();
				break;
			}
		}

	return bReturn;
	}


void 
CPaneImage::UpdateControls() {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	DoEnableControls();

	oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function positions the controls in the pane window
void
CPaneImage::DoPositionControls() {

	RECT cClientRect;

	GetClientRect(hWnd, &cClientRect);

	cButtonsControlRect.X = 20;
	cButtonsControlRect.Y = 10;
	cButtonsControlRect.Width = cClientRect.right - (cButtonsControlRect.X * 2);

	PositionPaneButtons(cButtonsControlRect);
	}


//This function processes all the messages, it returns true if it processed a message
bool 
CPaneImage::WndProc(HWND phWnd, UINT message, 
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
				
				//Make sure the complete background is repainted, and all
				//the controls too (fix to make them appear transparent)
				InvalidateRect(hWnd,				NULL, true);

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

		case WM_COMMAND:

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

//This function fills the comboboxes for resolution and colortype
void
CPaneImage::DoEnableControls() {

	bool bEnable;
	sCopyDocumentSelection cSelection;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	for (long lIndex=0; lIndex<(long)oPaneButtons.size(); lIndex++ ){

		bEnable = true;

		if (oPaneButtons[lIndex].oPaneButton!=NULL) {

			switch (oPaneButtons[lIndex].iCommandId) {

				case ID_IMAGE_CROP: 
					bEnable = oCore->CheckPageImageAction(eImageCrop, 0); break;
				case ID_IMAGE_FILLSELECTION:
					bEnable = oCore->CheckPageImageAction(eImageFillRectangle, 0); break;
				case ID_IMAGE_RECOGNIZETEXT:
					bEnable = false;
					if (oCore->GetCurrentPage()!=NULL) {
						oCore->GetCurrentPage()->GetSelection(cSelection);
						if (cSelection.bEnabled || !cSelection.bOuter) {
							bEnable = true;
							}
						}
					break;
				case ID_ROTATE_90: 
				case ID_ROTATE_180: 
				case ID_ROTATE_270: 
					bEnable = oCore->CheckPageImageAction(eImageRotate, 90); break;
				case ID_ROTATEANDFLIP_FLIPHORIZONTALLY: 
				case ID_ROTATEANDFLIP_FLIPVERTICALLY: 
					bEnable = oCore->CheckPageImageAction(eImageFlipVertical, 0); break;
				case ID_CONVERTTO_BLACKWHITE:
					bEnable = oCore->CheckPageImageAction(eImageBlackWhite, 0); break;
				case ID_CONVERTTO_GRAYSCALE:
					bEnable = oCore->CheckPageImageAction(eImageGrayscale, 0); break;
				}

			//Set the state of the button
			EnableWindow(oPaneButtons[lIndex].oPaneButton->hWnd, bEnable);
			}
		}
	
	oTrace->EndTrace(__WFUNCTION__, eAll);
}

//This function is called when the scrollbar has been moved to reposition
//all the controls on the window
void 
CPaneImage::DoRepositionControls() {

	DoPositionControls();
	InvalidateRect(hWnd, NULL, TRUE);
}

 



