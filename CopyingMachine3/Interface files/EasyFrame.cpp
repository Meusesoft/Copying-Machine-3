#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "EasyFrame.h"

CEasyFrame::CEasyFrame(sGlobalInstances pInstances) :
	CSharedDeviceSelection(pInstances)
{
	hWnd = NULL;
	oEasyFramePreferences	= NULL;
	oEasyFramePreview		= NULL;
	bCopyInProgress			= false;
	bExitAfterCopy			= false;
	hViewMenu				= NULL;
	oCopyButton				= NULL;

	oRegistry = pInstances.oRegistry;
	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;
	hLanguage = pInstances.hLanguage;

	oGlobalInstances = pInstances;

	oMenuIcon = new CMenuIcon(oGlobalInstances.hInstance);
	oMenuIcon->AddMenuCommandIcon(IDC_SWITCH,					L"PNG_TOOLSWITCH");
	oMenuIcon->AddMenuCommandIcon(ID_HELP_CONTENTS,				L"PNG_TOOLHELP");

}

CEasyFrame::~CEasyFrame(void)
{
	oCore->oNotifications->UnsubscribeAll(hWnd);

	if (oEasyFramePreferences!=NULL)	delete oEasyFramePreferences;
	if (oEasyFramePreview!=NULL)		delete oEasyFramePreview;
	if (oCopyButton!=NULL)				delete oCopyButton;
	if (oMenuIcon!=NULL)				delete oMenuIcon;

	if (hWnd!=NULL) Destroy();
}

HWND CEasyFrame::Create(HINSTANCE phInstance, HWND phWnd) {

	TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
	TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

	oTrace->StartTrace(__WFUNCTION__);
	
	hInstance = phInstance;

	//Create the main window
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_COPYINGMACHINE3, szWindowClass, MAX_LOADSTRING);

	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(phInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE3_32)));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(phInstance, MAKEINTRESOURCE(IDI_COPYINGMACHINE_16)));

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
}

void CEasyFrame::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);


	DestroyWindow(hWnd);
	hWnd=NULL;

	oTrace->EndTrace(__WFUNCTION__);
	}


//This function processes all the message
bool CEasyFrame::WndProc(HWND phWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn) {

	bool bReturn;

	oTrace->StartTrace(__WFUNCTION__, eExtreme);

	bReturn = true;

	switch (message) {
	
		case WM_INITDIALOG:

			hWnd = phWnd;

			hScanComboBox	= GetDlgItem(hWnd, IDC_ACQUIREFROM);
			hPrintComboBox	= GetDlgItem(hWnd, IDC_COPYTO);

			LoadSettings();
			DoLocalize();
			ShowWindow(phWnd, SW_SHOWNORMAL);

			if (hWnd) {

				oCore->oNotifications->Subscribe(hWnd, eNotificationWindowsInitialized);

				//These could be added to documentlist but the listview doesn't process
				//wm_app messages somehow... to do
		
				oCore->oNotifications->Subscribe(hWnd, eNotificationApplicationInitialized);
				oCore->oNotifications->Subscribe(hWnd, eNotificationNewDocument);
				oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentViewChanges);
				oCore->oNotifications->Subscribe(hWnd, eNotificationAcquisitionPageDone);
				oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentPrintingDone);
				oCore->oNotifications->Subscribe(hWnd, eNotificationScanSettingsChanged);
				oCore->oNotifications->Subscribe(hWnd, eNotificationScanDeviceChanged);
				oCore->oNotifications->Subscribe(hWnd, eNotificationPrintSettingsChanged);
				oCore->oNotifications->Subscribe(hWnd, eNotificationPrintDeviceChanged);
				oCore->oNotifications->Subscribe(hWnd, eNotificationLanguageChanged);
				}

			oCopyButton = new CButton(oTrace);
			oCopyButton->SetHandle(GetDlgItem(hWnd, IDC_DOCOPY), oGlobalInstances.hInstance);
			oCopyButton->SetImage(L"PNG_TOOLCOPY24", eImageTop, 24);

			DoFillControls();

			break;

		case WM_SCANIMAGESRECEIVED:

			eAcquireResult cResult;

			cResult = ((wParam==1) ? eAcquireOk : eAcquireFail);
			
			SetProgress(cResult==eAcquireOk ? 100 : 0);
			oCore->oNotifications->CommunicateEvent(eNotificationAcquisitionPageDone, (void*)cResult);
			
			break;

		case WM_MOVE:

			bReturn = MoveEasyFramePreferences();
			bReturn = MoveEasyFramePreview();

			break;
		
		case WM_WINDOWPOSCHANGING:

			LPWINDOWPOS cWindowPosition;

			cWindowPosition = (LPWINDOWPOS)lParam;

			DoValidatePositionEasyFrame(cWindowPosition);
			break;

		case WM_COMMAND:

			bReturn = false;

			switch (LOWORD(wParam)) {

				case IDC_ACQUIREFROM:
				case IDC_COPYTO:

					if (HIWORD(wParam)==CBN_SELCHANGE) {
						DoApplySettings();
						}
					break;

				case IDC_DOCOPYEXIT:
					bExitAfterCopy = true;
					//oCore->oRegistration->Reminder(hWnd);
					DoAcquireImages();
					break;

				case IDC_DOCOPY:
					bExitAfterCopy = false;
					//oCore->oRegistration->Reminder(hWnd);
					DoAcquireImages();
					break;

				case IDC_MENUVIEW:

					ShowViewMenu();
					break;

				case IDM_ABOUT:

					oCore->oDialogs->DlgAbout(hWnd);
					break;

				case ID_HELP_CONTENTS:

					oCore->OpenHelp(hWnd);
					break;

				case IDC_SWITCH:
					SaveSettings();
					PostQuitMessage(1); //close easyframe and open main frame
					break;

				case IDC_EXITBUTTON:
					SaveSettings();
					PostQuitMessage(0);
					break;

				case IDC_TOGGLEPREFERENCES:

					ShowEasyFramePreferences();
					break;

				case IDC_TOGGLEPREVIEW:

					ShowEasyFramePreview();
					break;

				case IDC_ACCEPT:
					oEasyFramePreview->SetPreviewDocument(-1, NULL);
					DoPrintCurrentDocument();
					break;

				case IDC_DECLINE:
					oEasyFramePreview->SetPreviewDocument(-1, NULL);
					oCore->CloseDocument(hWnd, oCore->GetCurrentDocument());
					bCopyInProgress = false;
					DoEnableControls();
					break;
					}
			break;

		case WM_CLOSE:
			SaveSettings();
			PostQuitMessage(0); //quit copying machine
			break;
		case WM_DESTROY:
			oCore->oRegistration->Reminder(hWnd);
			//PostQuitMessage(0);
			break;
		
		case WM_CORENOTIFICATION:

			if (phWnd == hWnd) {

				bReturn = OnEventCoreNotification();
				}
			bReturn=false;
			break;

		case WM_NOTIFY:

			if (oCopyButton) 
				bReturn = oCopyButton->WndProc(hWnd, message, wParam, lParam, piReturn);

			break;
			
		default:

				//Pump the message through the TWAIN process handler (if applicable)
				MSG cMessage;

				cMessage.hwnd = hWnd;
				cMessage.lParam = lParam;
				cMessage.wParam = wParam;
				cMessage.message = message;

				bReturn = oCore->ProcessMessage(cMessage);

				if (!bReturn && oMenuIcon)	bReturn = oMenuIcon->WndProc(phWnd, message, wParam, lParam, piReturn);
		}

	oTrace->EndTrace(__WFUNCTION__, bReturn, eExtreme);
	
	return bReturn;
	}

//This function validates the position of the main frame. If there is a problem it
//will reposition it. It also makes the edges of the screen sticky.
void 
CEasyFrame::DoValidatePositionEasyFrame(LPWINDOWPOS pcWindowPosition) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	RECT cEasyFrameRect;
	RECT cMonitorWorkArea;
	int  iWidth, iHeight;

	if ((pcWindowPosition->flags & SWP_NOMOVE)==0) {

		//Get the size of the current desktop
		GetCurrentDesktopSize(hWnd, &cMonitorWorkArea);

		//get the size of the dialog
		iWidth = pcWindowPosition->cx;
		iHeight  = pcWindowPosition->cy;

		cEasyFrameRect.left   = pcWindowPosition->x;
		cEasyFrameRect.top    = pcWindowPosition->y;
		cEasyFrameRect.right  = pcWindowPosition->x + iWidth;
		cEasyFrameRect.bottom = pcWindowPosition->y + iHeight;

		//Is it close the the left edge or complete out of view?
		if ((cEasyFrameRect.left < cMonitorWorkArea.left + 15 && cEasyFrameRect.left >= cMonitorWorkArea.left - 15) ||
			(cEasyFrameRect.left < cMonitorWorkArea.left - iWidth)) {

			cEasyFrameRect.left = cMonitorWorkArea.left;
			cEasyFrameRect.right = cEasyFrameRect.left + iWidth;
			}

		//Is it close the the top edge or complete out of view?
		if ((cEasyFrameRect.top < cMonitorWorkArea.top + 15 && cEasyFrameRect.top >= cMonitorWorkArea.top - 15) ||
			(cEasyFrameRect.top < cMonitorWorkArea.top - iHeight)) {

			cEasyFrameRect.top = cMonitorWorkArea.top;
			cEasyFrameRect.bottom = cEasyFrameRect.top + iHeight;
			}

		//Is it close the the right edge or complete out of view?
		if ((cEasyFrameRect.right > cMonitorWorkArea.right - 15 && cEasyFrameRect.right <= cMonitorWorkArea.right + 15) ||
			(cEasyFrameRect.right > cMonitorWorkArea.right + iWidth)) {

			cEasyFrameRect.right = cMonitorWorkArea.right;
			cEasyFrameRect.left = cEasyFrameRect.right - iWidth;
			}

		//Is it close the the bottom edge or complete out of view?
		if ((cEasyFrameRect.bottom > cMonitorWorkArea.bottom - 15 && cEasyFrameRect.bottom <= cMonitorWorkArea.bottom + 15) ||
			(cEasyFrameRect.bottom > cMonitorWorkArea.bottom + iHeight)) {

			cEasyFrameRect.bottom = cMonitorWorkArea.bottom;
			cEasyFrameRect.top = cEasyFrameRect.bottom - iHeight;
			}

		//Position the window
		pcWindowPosition->x = cEasyFrameRect.left;
		pcWindowPosition->y = cEasyFrameRect.top;
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function positions the preferences frame in relation to the main easy frame
//dialog.

bool 
CEasyFrame::MoveEasyFramePreferences() {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bResult = false;

	if (oEasyFramePreferences!=NULL) {
	
		bResult = true;

		RECT cEasyFrameRect;
		RECT cEasyFramePreferencesRect;
		RECT cMonitorWorkArea;

		//Get the size of the current desktop
		GetCurrentDesktopSize(hWnd, &cMonitorWorkArea);

		//get the size of the dialogs
		GetWindowRect(hWnd, &cEasyFrameRect);
		GetWindowRect(oEasyFramePreferences->hWnd, &cEasyFramePreferencesRect);

		cEasyFramePreferencesRect.left = cEasyFrameRect.left;
		cEasyFramePreferencesRect.right = cEasyFrameRect.right;
		int iHeight;
		iHeight = cEasyFramePreferencesRect.bottom - cEasyFramePreferencesRect.top;

		cEasyFramePreferencesRect.top = cEasyFrameRect.bottom + 15;
		cEasyFramePreferencesRect.bottom = cEasyFramePreferencesRect.top + iHeight;

		if (cEasyFramePreferencesRect.bottom > cMonitorWorkArea.bottom) {

			//we are moving outside the screen, position the preview to the
			//top side of the main window

			cEasyFramePreferencesRect.bottom = cEasyFrameRect.top - 15;
			cEasyFramePreferencesRect.top = cEasyFramePreferencesRect.bottom - iHeight;
			}

		//Position the window
		MoveWindow(oEasyFramePreferences->hWnd,
				cEasyFramePreferencesRect.left, cEasyFramePreferencesRect.top,
				cEasyFramePreferencesRect.right - cEasyFramePreferencesRect.left, 
				cEasyFramePreferencesRect.bottom - cEasyFramePreferencesRect.top,
				TRUE);
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return bResult;
}

//This function positions the preferences frame in relation to the main easy frame
//dialog.

bool 
CEasyFrame::MoveEasyFramePreview() {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bResult = false;

	if (oEasyFramePreview!=NULL) {
	
		bResult = true;

		RECT cEasyFrameRect;
		RECT cEasyFramePreviewRect;
		RECT cMonitorWorkArea;

		//Get the size of the current desktop
		GetCurrentDesktopSize(hWnd, &cMonitorWorkArea);

		//get the size of the dialogs
		GetWindowRect(hWnd, &cEasyFrameRect);
		GetWindowRect(oEasyFramePreview->hWnd, &cEasyFramePreviewRect);

		int iHeight, iWidth;
		iHeight = cEasyFramePreviewRect.bottom - cEasyFramePreviewRect.top;
		iWidth = cEasyFramePreviewRect.right - cEasyFramePreviewRect.left;

		cEasyFramePreviewRect.top = cEasyFrameRect.top;
		cEasyFramePreviewRect.bottom = cEasyFrameRect.top + iHeight;

		cEasyFramePreviewRect.left = cEasyFrameRect.right + 15;
		cEasyFramePreviewRect.right = cEasyFramePreviewRect.left + iWidth;

		if (cEasyFramePreviewRect.right > cMonitorWorkArea.right) {

			//we are moving outside the screen, position the preview on the left
			//side of the main window

			cEasyFramePreviewRect.right = cEasyFrameRect.left - 15;
			cEasyFramePreviewRect.left = cEasyFramePreviewRect.right - iWidth;
			}

		if (cEasyFramePreviewRect.bottom > cMonitorWorkArea.bottom) {

			//we are moving outside the screen, position the preview to the
			//bottom side of the main window

			cEasyFramePreviewRect.bottom = cEasyFrameRect.bottom;
			cEasyFramePreviewRect.top = cEasyFramePreviewRect.bottom - iHeight;
			}

		//Restore the position
		MoveWindow(oEasyFramePreview->hWnd,
				cEasyFramePreviewRect.left, cEasyFramePreviewRect.top,
				cEasyFramePreviewRect.right - cEasyFramePreviewRect.left, 
				cEasyFramePreviewRect.bottom - cEasyFramePreviewRect.top,
				TRUE);
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return bResult;
}

//This function shows or hids the EasyFramePreferences
void 
CEasyFrame::ShowEasyFramePreferences(eToggleParameter pcToggle) {

	oTrace->StartTrace(__WFUNCTION__);

	if (oEasyFramePreferences==NULL && (pcToggle==eShow || pcToggle==eToggle)){
		if (!oEasyFramePreferences) {
			oEasyFramePreferences = new CEasyFramePreferences(oGlobalInstances);
			oEasyFramePreferences->Create(oGlobalInstances.hInstance, hWnd);
			oEasyFramePreferences->SetEnableControl(!bCopyInProgress);
			MoveEasyFramePreferences();
			}
		}
	else {
		if (oEasyFramePreferences!=NULL && (pcToggle==eHide || pcToggle==eToggle)){
			if (oEasyFramePreferences) {
				delete oEasyFramePreferences;
				oEasyFramePreferences = NULL;
				}
			}
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function shows or hides the EasyFramePreview
void 
CEasyFrame::ShowEasyFramePreview(eToggleParameter pcToggle) {

	//bool bVisible;
	//bVisible = (SendDlgItemMessage(hWnd, IDC_TOGGLEPREVIEW,	BM_GETCHECK, 0, 0)==BST_CHECKED);

	oTrace->StartTrace(__WFUNCTION__);

	if (oEasyFramePreview==NULL && (pcToggle==eShow || pcToggle==eToggle)){
		if (!oEasyFramePreview) {
			oEasyFramePreview = new CEasyFramePreview(oGlobalInstances);
			oEasyFramePreview->Create(oGlobalInstances.hInstance, hWnd);
			MoveEasyFramePreview();
			}
		}
	else {
		if (oEasyFramePreview!=NULL && (pcToggle==eHide || pcToggle==eToggle)){
			if (oEasyFramePreview) {
				delete oEasyFramePreview;
				oEasyFramePreview = NULL;
				}
			}
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function handles the core notifications
bool 
CEasyFrame::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;

	bReturn = false;

	while (oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationLanguageChanged:

				oGlobalInstances.hLanguage = (HINSTANCE)cNotification.pData;
				DoLocalize();
				break;

			case eNotificationAcquisitionPageDone:

				//Acquiring image(s) is done. Transfer them
				if (cNotification.pData==(void*)eAcquireOk || oCore->oScanCore->ImagesAvailable()>0) {
					oCore->TransferImages(hWnd);
					DoOutputImages();
					}
				else {

					//No images, probably due to an error. Enable all controls
					bCopyInProgress = false;
					DoEnableControls();	
					}
				break;

			case eNotificationApplicationInitialized:

				oCore->oRegistration->Reminder(hWnd);
				break;
			
			case eNotificationScanDeviceChanged:

				DoFillScanSelector();
				break;

			case eNotificationPrintDeviceChanged:

				DoFillPrintSelector();
				break;

			case eNotificationDocumentPrintingDone:
				//
				//sPrintDocumentWithSettings* oPrintDocument;

				//oPrintDocument = (sPrintDocumentWithSettings*)cNotification.pData;
				//
				//if (iPrintTicket == oPrintDocument->iPrintRequest) {
				//	oCore->CloseDocument(hWnd, oPrintDocument->oDocument);
				//	delete oPrintDocument;

				//	bCopyInProgress = false;
				//	DoEnableControls();
				//	}
				break;
			}
		}

	return bReturn;
	}
	
//This function fills the controls on the dialog with the values of
//the settings structure
void 
CEasyFrame::DoFillControls() {

	DoFillScanSelector();
	DoFillPrintSelector();
	}


//This function loads the settings from the registry
void 
CEasyFrame::LoadSettings() {

	oTrace->StartTrace(__WFUNCTION__);

	RECT cRect;
	RECT cWindowRect;
	int iMaxX, iMaxY;

	//read the registry
	cRect.left = oRegistry->ReadInt(L"Window", L"LeftEasy", 20);	
	cRect.top = oRegistry->ReadInt(L"Window", L"TopEasy", 20);	

	//get the size of the dialog
	GetWindowRect(hWnd, &cWindowRect);
	
	// make sure the window is not completely out of sight
    iMaxX = GetSystemMetrics(SM_CXSCREEN) - GetSystemMetrics(SM_CXICON);
    iMaxY = GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYICON);

    cRect.left = min(cRect.left, iMaxX);
    cRect.top  = min(cRect.top,  iMaxY);

	cRect.left = max(0, cRect.left);
	cRect.top  = max(0, cRect.top);

	//Restore the position
	MoveWindow(hWnd,
			cRect.left, cRect.top,
			cWindowRect.right - cWindowRect.left, cWindowRect.bottom - cWindowRect.top,
			FALSE);

	//Show preferences windows

	//SendDlgItemMessage(hWnd, IDC_TOGGLEPREFERENCES,	BM_SETCHECK, 
	//	oRegistry->ReadInt(L"Window", L"EasyPreferences", 0)==1 ? BST_CHECKED : BST_UNCHECKED,
	//	0);
	ShowEasyFramePreferences(oRegistry->ReadInt(L"Window", L"EasyPreferences", 0)==1 ? eShow : eHide);

	//SendDlgItemMessage(hWnd, IDC_TOGGLEPREVIEW,	BM_SETCHECK, 
	//	oRegistry->ReadInt(L"Window", L"EasyPreview", 0)==1 ? BST_CHECKED : BST_UNCHECKED,
	//	0);
	ShowEasyFramePreview(oRegistry->ReadInt(L"Window", L"EasyPreview", 0)==1 ? eShow : eHide);

	oTrace->EndTrace(__WFUNCTION__);
}

//This function save the settings to the registry
void CEasyFrame::SaveSettings() {

	WINDOWPLACEMENT wPlacement;

	oTrace->StartTrace(__WFUNCTION__);

	wPlacement.length = sizeof(wPlacement);

	GetWindowPlacement(hWnd, &wPlacement);

	//Save the window settings
	oRegistry->WriteInt(L"Window", L"LeftEasy", wPlacement.rcNormalPosition.left);
	oRegistry->WriteInt(L"Window", L"TopEasy", wPlacement.rcNormalPosition.top);
	oRegistry->WriteInt(L"Window", L"EasyPreferences", oEasyFramePreferences==NULL ? 0 : 1);
	oRegistry->WriteInt(L"Window", L"EasyPreview", oEasyFramePreview==NULL ? 0 : 1);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function applies the changes to the settings in the comboboxes
void
CEasyFrame::DoApplySettings() {

	//Update the combobox settings
	CSharedDeviceSelection::DoApplySettings();

	}

//This function acquires the images 
void 
CEasyFrame::DoAcquireImages() {

	oTrace->StartTrace(__WFUNCTION__);

	bCopyInProgress = true;
	DoEnableControls();

	oCore->oScanSettings->SetBool(eShowInterface, false, false);

	CDlgProgressAcquire* oDlgProgress;
	oDlgProgress = oCore->oDialogs->DlgProgressAcquire(hWnd, oCore->oScanSettings);
	delete oDlgProgress;

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function performs the output of an image.
void 
CEasyFrame::DoOutputImages() {

	int iOutputIndex;
	sOutputDevice oOutputDevice;

	oTrace->StartTrace(__WFUNCTION__);

	iOutputIndex = GetCurrentOutputDevice();

	if (iOutputIndex!=-1) {

		if (oCore->oPrintCore->GetOutputDevice(iOutputIndex, oOutputDevice)) {

			//Do the output
			switch (oOutputDevice.cType) {

				case eOutputFile:

					oCore->SaveDocument(hWnd, oCore->GetCurrentDocument());
					bCopyInProgress = false;
					DoEnableControls();
					oCore->oDocuments->CloseDocument(oCore->GetCurrentDocument());
					break;

				case eOutputMail:

					oCore->GetCurrentDocument()->MailTo();
					bCopyInProgress = false;
					DoEnableControls();
					oCore->oDocuments->CloseDocument(oCore->GetCurrentDocument());
					break;

				case eOutputPrinter:

					if (oEasyFramePreview && !bExitAfterCopy) {
						oEasyFramePreview->SetPreviewDocument(oCore->GetCurrentDocument()->GetDocumentID(), NULL);
						}
					else {
						DoPrintCurrentDocument();
						}

					break;
				}
			}
		}

	//close the application
	if (bExitAfterCopy) {

		PostQuitMessage(0);
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function prints the current document
void
CEasyFrame::DoPrintCurrentDocument() {

	int iOutputIndex;
	sOutputDevice oOutputDevice;

	oTrace->StartTrace(__WFUNCTION__);

	iOutputIndex = GetCurrentOutputDevice();

	if (iOutputIndex!=-1) {

		if (oCore->oPrintCore->GetOutputDevice(iOutputIndex, oOutputDevice)) {

			CPrintSettings* oTemporarySettings;

			oTemporarySettings = oCore->oPrintSettings->Copy();
			oTemporarySettings->bDeleteAfterPrint = true;
			oTemporarySettings->bNotifyChanges = false;
			oTemporarySettings->SetDevice(oOutputDevice);
			oTemporarySettings->SetBool(ePrintShowInterface, false);
			
			//Initialize the progress bar
			//HWND hProgressBar;
			//HWND hStatusWindow;

			//hProgressBar = GetDlgItem(hWnd, IDC_PROGRESS);
			//hStatusWindow = GetDlgItem(hWnd, IDC_STATUSTEXT);

			//oTemporarySettings->hProgressBar = hProgressBar;
			//oTemporarySettings->hStatusWindow = hStatusWindow;
			//oTemporarySettings->hWindow = hWnd;

			////Do the printing
			//iPrintTicket = oCore->GetCurrentDocument()->PrintThread(oTemporarySettings);
			CCopyDocument* oPrintDocument;
			
			oPrintDocument = oCore->GetCurrentDocument();

			CDlgProgressPrint* oDlgProgress;
			oDlgProgress = oCore->oDialogs->DlgProgressPrint(hWnd, oPrintDocument, oTemporarySettings);
			delete oDlgProgress;
			
			oCore->CloseDocument(hWnd, oPrintDocument);

			bCopyInProgress = false;
			DoEnableControls();

			}
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function enables the main controls in a copy operation is in progress
void
CEasyFrame::DoEnableControls() {

	HWND hControl;

	hControl = GetDlgItem(hWnd, IDC_DOCOPY);
	EnableWindow(hControl, !bCopyInProgress);
	hControl = GetDlgItem(hWnd, IDC_SWITCH);
	EnableWindow(hControl, !bCopyInProgress);
	hControl = GetDlgItem(hWnd, IDC_ACQUIREFROM);
	EnableWindow(hControl, !bCopyInProgress);
	hControl = GetDlgItem(hWnd, IDC_COPYTO);
	EnableWindow(hControl, !bCopyInProgress);

	if (oEasyFramePreferences) oEasyFramePreferences->SetEnableControl(!bCopyInProgress);
	}

//This function set the progress value
void 
CEasyFrame::SetProgress(int piValue) {

	HWND hProgressBar;

	hProgressBar = GetDlgItem(hWnd, IDC_PROGRESS);
    if (hProgressBar!=NULL) {
        SendMessage(hProgressBar, PBM_SETPOS, piValue, 0); 
        }
	}

//This function retrieves the currently selected outputdevice. It returns the index
//in the outputdevices vector, and it returns -1 if there is an error.
int  
CEasyFrame::GetCurrentOutputDevice() {

	HWND hComboBox;
	int iResult;
	int iOutputIndex;

	oTrace->StartTrace(__WFUNCTION__);

	iOutputIndex = 0;

	//Get the selected item 
	hComboBox = GetDlgItem(hWnd, IDC_COPYTO);

	iResult = (int)SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
	if (iResult!=CB_ERR) {

		iResult = (int)SendMessage(hComboBox, CB_GETITEMDATA, (WPARAM)iResult, 0);
		}

	if (iResult<0 || iResult>=oCore->oPrintCore->GetOutputDeviceCount()) iResult = -1; //the iResult isn't a valid outputdevice

	oTrace->EndTrace(__WFUNCTION__, iResult);

	return iResult;
	}

//This function localizes all controls
void 
CEasyFrame::DoLocalize() {

	LocalizeControl(IDC_TOPIC1,				IDS_COPY);
	LocalizeControl(IDC_TOPIC2,				IDS_ACQUIREFROM);
	LocalizeControl(IDC_TOPIC3,				IDS_COPYTO);
	LocalizeControl(IDC_DOCOPY,				IDS_COPY);
	//LocalizeControl(IDC_TOGGLEPREFERENCES,	IDS_PREFERENCES);
	//LocalizeControl(IDC_TOGGLEPREVIEW,		IDS_PREVIEW);
	LocalizeControl(IDC_MENUVIEW,			IDS_VIEW);
	LocalizeControl(IDC_SWITCH,				IDS_TOADVANCED);
	LocalizeControl(IDC_EXITBUTTON,			IDS_EXIT);

	//HICON hIcon;
	//HWND  hButton;

	// load the icon, NOTE: you should delete it after assigning it. 
	//hIcon = LoadIcon(oGlobalInstances.hInstance,MAKEINTRESOURCE(IDI_COPYINGMACHINE)); 
	//hButton = GetDlgItem(hWnd, IDC_DOCOPY);
	
	// associate the icon with the button. 
	//SendMessage(hButton,BM_SETIMAGE,(WPARAM)IMAGE_ICON, 
	//		   (LPARAM)(HANDLE)hIcon); 



	}

//This function changes the caption/text of a control
void 
CEasyFrame::LocalizeControl(int piControl, int piResource) {

	HWND hControl;
	wchar_t* cText;

	hControl = GetDlgItem(hWnd, piControl);
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	LoadString(oGlobalInstances.hLanguage, piResource, cText, 400);

	SetWindowText(hControl, cText);

	free(cText);
}

//This function retrieves the rectangle/size of the desktop in which
//the given window is active
bool 
CEasyFrame::GetCurrentDesktopSize(HWND phWindow, LPRECT pcDesktopRect) {

	HMONITOR	hMonitor;
	MONITORINFO cMonitorInfo;
    RECT        cWindowRect;

	//Get the rectangle of the window
	GetWindowRect(phWindow, &cWindowRect);
	
	//Get the handle to the nearest monitor
	hMonitor = MonitorFromRect(&cWindowRect, MONITOR_DEFAULTTONEAREST);
	
	//
    // get the work area or entire monitor rect.
    //
    cMonitorInfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &cMonitorInfo);

	memcpy(pcDesktopRect, &cMonitorInfo.rcWork, sizeof(RECT));

	return true;
	}

//This function constructs the view menu in runtime and shows it below
//the view button
void 
CEasyFrame::ShowViewMenu() {

	wchar_t	cMenuItemText[96];
	UINT	uFlags;
	HWND	hMenuViewButton;
	RECT	cMenuViewRect;
	
	oTrace->StartTrace(__WFUNCTION__);

	hViewMenu = CreatePopupMenu();

	if (hViewMenu!=NULL) {
		
		//Build the menu
		LoadString(oGlobalInstances.hLanguage, IDS_PREFERENCES, cMenuItemText, 96);
		//uFlags = oEasyFramePreferences ? MF_CHECKED : MF_UNCHECKED;
		//uFlags |= bCopyInProgress ? MF_GRAYED : MF_ENABLED;
		AppendMenu(hViewMenu, MF_STRING/* | uFlags*/, IDC_TOGGLEPREFERENCES, cMenuItemText);
		oMenuIcon->SetCheckMenuCommand(IDC_TOGGLEPREFERENCES, oEasyFramePreferences!=NULL);
		oMenuIcon->SetEnableMenuCommand(IDC_TOGGLEPREFERENCES, !bCopyInProgress);

		LoadString(oGlobalInstances.hLanguage, IDS_PREVIEW, cMenuItemText, 96);
		//uFlags = oEasyFramePreview ? MF_CHECKED : MF_UNCHECKED;
		//uFlags |= bCopyInProgress ? MF_GRAYED : MF_ENABLED;
		AppendMenu(hViewMenu, MF_STRING/* | uFlags*/, IDC_TOGGLEPREVIEW, cMenuItemText);
		oMenuIcon->SetCheckMenuCommand(IDC_TOGGLEPREVIEW, oEasyFramePreview!=NULL);
		oMenuIcon->SetEnableMenuCommand(IDC_TOGGLEPREVIEW, !bCopyInProgress);
		AppendMenu(hViewMenu, MF_SEPARATOR, 0, 0);
		
		uFlags = bCopyInProgress ?  MF_GRAYED : MF_ENABLED;
		LoadString(oGlobalInstances.hLanguage, IDS_SWITCH, cMenuItemText, 96);
		AppendMenu(hViewMenu, MF_STRING/* | uFlags*/, IDC_SWITCH, cMenuItemText);
		oMenuIcon->SetEnableMenuCommand(IDC_SWITCH, !bCopyInProgress);
		AppendMenu(hViewMenu, MF_SEPARATOR, 0, 0);
		
		LoadString(oGlobalInstances.hLanguage, IDS_HELP, cMenuItemText, 96);
		wcscat_s(cMenuItemText, 96, L"...");
		AppendMenu(hViewMenu, MF_STRING, ID_HELP_CONTENTS, cMenuItemText);

		LoadString(oGlobalInstances.hLanguage, IDS_ABOUT, cMenuItemText, 96);
		wcscat_s(cMenuItemText, 96, L"...");
		AppendMenu(hViewMenu, MF_STRING, IDM_ABOUT, cMenuItemText);

		//Get the window rectangle of the button
		hMenuViewButton = GetDlgItem(hWnd, IDC_MENUVIEW);
		GetWindowRect(hMenuViewButton, &cMenuViewRect);

		//Process the menu
		oMenuIcon->ProcessMenu(hViewMenu);

		//Execute the popup menu
		TrackPopupMenu(hViewMenu, TPM_LEFTALIGN | TPM_TOPALIGN, cMenuViewRect.left, cMenuViewRect.bottom, 0, hWnd, 0);
		}

	oTrace->EndTrace(__WFUNCTION__);
	}
