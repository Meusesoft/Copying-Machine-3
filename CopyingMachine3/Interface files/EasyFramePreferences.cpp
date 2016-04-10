#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "SharedPreferencesSelection.h"
#include "EasyFrame.h"


CEasyFramePreferences::CEasyFramePreferences(sGlobalInstances pInstances) :
	CSharedPreferencesSelection(pInstances) 
{
	hWnd = NULL;

	oTrace = pInstances.oTrace;
	oRegistry = pInstances.oRegistry;
	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;

	oGlobalInstances = pInstances;

	bControlsEnabled = true;
}

CEasyFramePreferences::~CEasyFramePreferences(void)
{
	oCore->oNotifications->UnsubscribeAll(hWnd);
	if (hWnd!=NULL) Destroy();
}

HWND CEasyFramePreferences::Create(HINSTANCE phInstance, HWND phParent) {

	oTrace->StartTrace(__WFUNCTION__);
	
	hInstance = phInstance;

	//Create the main window
	HWND hResult = (HWND)(CreateDialogParam (hInstance, MAKEINTRESOURCE(IDD_EASYFRAMEPREFERENCES), 
					phParent, (DLGPROC)DialogTemplateProc, (LPARAM)this)!=0); 


	oTrace->EndTrace(__WFUNCTION__, (bool)(hResult!=NULL));

	return hWnd;
}

void CEasyFramePreferences::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	DestroyWindow(hWnd);
	hWnd=NULL;

	oTrace->EndTrace(__WFUNCTION__);
	}


//This function processes all the message
BOOL CALLBACK CEasyFramePreferences::DialogTemplateProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	bool bReturn;

	bReturn = true;

	CEasyFramePreferences* oThisDialog;

	oThisDialog = (CEasyFramePreferences*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (message) {
	
		case WM_INITDIALOG:

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM)lParam); //save the pointer to the dialog 
			oThisDialog = (CEasyFramePreferences*)lParam;
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
CEasyFramePreferences::OnMessage(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	bool bResult;

	bResult = true;

	switch (message) {

		case WM_COMMAND:

			OnCommand(LOWORD(wParam), wParam, lParam);
			break;

		case WM_CORENOTIFICATION:

			OnEventCoreNotification();
			break;

		default:
			
			bResult = false;
			break;
		}

	return bResult;
	}


//This function handles the WM_COMMAND message
bool 
CEasyFramePreferences::OnCommand(int piCommand, WPARAM wParam, LPARAM lParam) {

	bool bReturn;

	oTrace->StartTrace(__WFUNCTION__);

	bReturn = true;

	bReturn = CSharedPreferencesSelection::OnCommand(piCommand, wParam, lParam);

	switch (piCommand) {

		case ID_FILE_PREFERENCES:

			oCore->oDialogs->DlgPreferences(hWnd, eAllPreferences);
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
CEasyFramePreferences::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;

	bReturn = false;

	while (oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationApplicationInitialized:

		
				break;

			case eNotificationLanguageChanged:

				oGlobalInstances.hLanguage = (HINSTANCE)cNotification.pData;
				DoLocalize();
				DoFillControls();
				break;

			case eNotificationScanDeviceChanged:
			case eNotificationScanSettingsChanged:
			case eNotificationPrintSettingsChanged:

				DoFillControls();
				DoEnableControls();

				//Remove remaining notifications. We only need to process it once
				oCore->oNotifications->RemoveNotificationSpecific(hWnd, eNotificationScanSettingsChanged);
				oCore->oNotifications->RemoveNotificationSpecific(hWnd, eNotificationPrintSettingsChanged);

				bReturn = true;	
				break;
			}
		}

	return bReturn;
	}

//This function enables or disables the controls
void
CEasyFramePreferences::SetEnableControl(bool pbEnable) {

	bControlsEnabled = pbEnable;
	DoEnableControls();
	}

//This function handles the WM_INITDIALOG message
void
CEasyFramePreferences::OnInitDialog() {

	//Retrieve the handles to the controls
	hColorBox = GetDlgItem(hWnd, IDC_COLORDEPTH);
	hResolutionBox = GetDlgItem(hWnd, IDC_RESOLUTION);
	hShowInterfaceCheckBox = GetDlgItem(hWnd, IDC_SHOWINTERFACE);
	hCopiesEdit = GetDlgItem(hWnd, IDC_PRINTCOPIES);
	hCopiesSpin = GetDlgItem(hWnd, IDC_SPINPRINTCOPIES);

	//Localize captions
	DoLocalize();

	//Fill controls	
	DoFillControls();

	//Enable or disable the controls
	DoEnableControls();

	//Subscribe to notification
	oCore->oNotifications->Subscribe(hWnd, eNotificationScanDeviceChanged);
	oCore->oNotifications->Subscribe(hWnd, eNotificationScanSettingsChanged);
	oCore->oNotifications->Subscribe(hWnd, eNotificationPrintSettingsChanged);
	oCore->oNotifications->Subscribe(hWnd, eNotificationLanguageChanged);

	//Show the window
	ShowWindow(hWnd, SW_SHOWNORMAL);
	}

	
//This function fills the comboboxes for resolution and colortype
void
CEasyFramePreferences::DoFillControls() {


	CSharedPreferencesSelection::DoFillControls();

	//Set the checkbox
	DoFillShowInterfaceCheckbox();
		
}

//This function fills the comboboxes for resolution and colortype
void
CEasyFramePreferences::DoEnableControls() {

	HWND hControl;

	CSharedPreferencesSelection::DoEnableControls();

	hControl = GetDlgItem(hWnd, ID_FILE_PREFERENCES);
	EnableWindow(hControl, bControlsEnabled);
}

//This function localizes the controls in this window
void 
CEasyFramePreferences::DoLocalize() {

	LocalizeWindowCaption(IDS_PREFERENCES);

	LocalizeControl(IDC_GRPPREFERENCES,		IDS_PREFERENCES);
	LocalizeControl(IDC_SHOWINTERFACE,		IDS_SHOWINTERFACE);
	LocalizeControl(IDC_TXTCOLORDEPTH,		IDS_COLORDEPTH);
	LocalizeControl(IDC_TXTRESOLUTION,		IDS_RESOLUTION);
	LocalizeControl(IDC_TXTCOPIES,			IDS_COPIES);
	LocalizeControl(ID_FILE_PREFERENCES,	IDS_ALLPREFERENCES);
	}


//This function changes the caption/text of a control
void 
CEasyFramePreferences::LocalizeControl(int piControl, int piResource) {

	HWND hControl;
	wchar_t* cText;

	hControl = GetDlgItem(hWnd, piControl);
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	LoadString(oGlobalInstances.hLanguage, piResource, cText, 400);

	SetWindowText(hControl, cText);

	free(cText);
}

void 
CEasyFramePreferences::LocalizeWindowCaption(int piResource) {

	wchar_t* cText;

	cText = (wchar_t*)malloc(sizeof(wchar_t) * MAX_PATH);

	LoadString(oGlobalInstances.hLanguage, piResource, cText, MAX_PATH);

	SetWindowText(hWnd, cText);

	free(cText);
	}

