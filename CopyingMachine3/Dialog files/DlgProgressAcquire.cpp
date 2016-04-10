#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgProgressAcquire::CDlgProgressAcquire(HWND phParent, void* poScanSettings, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(poGlobalInstances.hInstance, phParent, IDD_PROGRESSDIALOGENHANCED, poGlobalInstances) {

	hDlg = NULL;

	//CScanSettings* oSettings;

	//oSettings = new CScanSettings(poGlobalInstances);
	//oSettings->Load();

	oScanSettings = (void*)poScanSettings;
	}

CDlgProgressAcquire::~CDlgProgressAcquire(void)
{
	if (hDlg!=NULL) DestroyWindow(hDlg);

	//CScanSettings* oSettings;

	//oSettings = (CScanSettings*)oScanSettings;
	//delete oSettings;
}

//This function is called when the dialog is initialised
void 
CDlgProgressAcquire::OnInitDialog() {

	CCopyingMachineCore* oCMCore;
	CScanSettings* oSettings;

	oCMCore = (CCopyingMachineCore*)oCore;

	//set the font of the caption
	SetDlgItemFont(IDC_CAPTION, hCaptionFont);

	//Initialize the progress bar
	HWND hProgressBar;

	hProgressBar = GetDlgItem(hDlg, IDC_PROGRESS);
    if (hProgressBar!=NULL) {
        SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); 
        }

	oSettings = new CScanSettings(oGlobalInstances);
	oSettings->Copy((CScanSettings*)oScanSettings);
	oSettings->hProgressBar = hProgressBar;
	oSettings->hWindow = hDlg;
	oSettings->bSendNotifications = false;

	Localize();
	DoFillControls();
	SetDlgItemFont(IDC_PROGRESSCAPTION, hCaptionFont);

	//Start acquiring images
	eAcquireResult eResult;
	eResult = oCMCore->oScanCore->Acquire(oSettings);

	//Show or hide
	if (eResult==eAcquireOk && oCMCore->oScanCore->GetCurrentInterface()!=eInputFile) {

		ShowWindow(hDlg, SW_SHOW);
		}
	else {
	
		//Thread not started, quit dialog
		EndDialog(hDlg, IDCANCEL);
		}

	//Show error message, if any
	oCMCore->oScanCore->ShowErrorMessage(NULL);
	}

BOOL 
CDlgProgressAcquire::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;
	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_PROGRESSCAPTION)) { 
                 bReturn=((LONG)GetStockObject(WHITE_BRUSH)); //to do, this doesn't work anymore
				}

			break;

		case WM_SCANIMAGESRECEIVED:

			if (wParam==1) {
				
				//An error occurred/user pressed cancel while scanning, continu scanning?
				LocalizeStatusAfterUnsuccessfulAcquire();
				LocalizeControl(IDC_MESSAGE, IDS_ACQUIREMOREPAGES);

				ShowEnableControl(IDCANCEL, false);
				ShowEnableControl(IDC_YES, true);
				ShowEnableControl(IDC_NOANSWER, true);
				ShowEnableControl(IDC_NEXTPAGE, false);

				SetProgress(0);
				}
			else {

				CScanSettings* oSettings;

				oSettings = (CScanSettings*)oScanSettings;

				if (oSettings->GetBool(eAskScanNextPage)) {
				
					//Ask for next page
					lCountdownTimer = 10;
					SetTimer(hDlg, 301, 1000, NULL);

					LocalizeControl(IDC_PROGRESSCAPTION, IDS_ACQUIREMOREPAGES);
					LocalizeMessageCountdown();			
					LocalizeStatusAfterSuccessfulAcquire();

					ShowEnableControl(IDCANCEL, false);
					ShowEnableControl(IDC_YES, true);
					ShowEnableControl(IDC_NOANSWER, true);
					ShowEnableControl(IDC_NEXTPAGE, false);

					SetProgress(100);
					}
				else
					{
					//Close the dialog
					OnCommand(IDC_NOANSWER, 0, 0);
					}
				}

			break;

		case WM_TIMER:

			lCountdownTimer--;

			if (lCountdownTimer<0) {

				KillTimer(hDlg, 301);
				EndDialog(hDlg, IDCANCEL);
				oCMCore->oNotifications->CommunicateEvent(eNotificationAcquisitionPageDone, (void*)eAcquireCancelled);
				}

			LocalizeMessageCountdown();
			SetProgress(lCountdownTimer * 10);

			break;

		default:

			//Pump the message through the TWAIN process handler (if applicable)
			MSG cMessage;

			cMessage.hwnd = hDlg;
			cMessage.lParam = lParam;
			cMessage.wParam = wParam;
			cMessage.message = message;

			bReturn = oCMCore->ProcessMessage(cMessage);
			break;
		
		};	

	return bReturn;
	}

BOOL 
CDlgProgressAcquire::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	BOOL bResult;
	CCopyingMachineCore* oCMCore;

	bResult = FALSE;
	oCMCore = (CCopyingMachineCore*)oCore;

	switch (iCommand) {

		case IDCANCEL:

			LocalizeControl(IDC_STATUS, IDS_CANCELLINGTASK);
			oCMCore->oScanCore->SetEndThread();
			bResult = TRUE;
			break;
		
		case IDC_NEXTPAGE:

			eAcquireResult eResult;
			CScanSettings* oSettings;

			oSettings = new CScanSettings(oGlobalInstances);
			oSettings->Copy((CScanSettings*)oScanSettings);
			oSettings->hProgressBar = GetDlgItem(hDlg, IDC_PROGRESS);
			oSettings->hWindow = hDlg;
			oSettings->bSendNotifications = false;

			eResult = oCMCore->oScanCore->Acquire(oSettings);

			SetProgress(0);

			//Show or hide
			if (eResult==eAcquireOk) {

				LocalizeControl(IDC_PROGRESSCAPTION, IDS_ACQUIRINGIMAGES);
				LocalizeControl(IDC_MESSAGE, IDS_NULL);
				LocalizeStatusBeforeAcquire();

				ShowEnableControl(IDCANCEL, true);
				ShowEnableControl(IDC_YES, false);
				ShowEnableControl(IDC_NOANSWER, false);
				ShowEnableControl(IDC_NEXTPAGE, false);
				}
			else {
			
				//Thread not started, quit dialog
				EndDialog(hDlg, IDCANCEL);
				oCMCore->oNotifications->CommunicateEvent(eNotificationAcquisitionPageDone, (void*)eAcquireCancelled);
				}

			break;

		case IDC_YES:

			LocalizeControl(IDC_PROGRESSCAPTION, IDS_PLACENEXTPAGECAPTION);
			LocalizeControl(IDC_MESSAGE, IDS_PLACENEXTPAGEMESSAGE);

			ShowEnableControl(IDCANCEL, false);
			ShowEnableControl(IDC_YES, false);
			ShowEnableControl(IDC_NOANSWER, false);
			ShowEnableControl(IDC_NEXTPAGE, true);

			KillTimer(hDlg, 301);
			break;

		case IDC_NOANSWER:

			KillTimer(hDlg, 301);
			EndDialog(hDlg, IDOK);
			bResult = TRUE;

			oCMCore->oNotifications->CommunicateEvent(eNotificationAcquisitionPageDone, (void*)eAcquireCancelled);
			break;

		case IDC_ASKTOSCANNEXTPAGE:
		case IDC_USEFEEDER:
		case IDC_AUTOSAVEDOCUMENT:
		case IDC_SAVETYPE:
			
			DoApplySettings();
			break;
		}

	return bResult;
	}

//This function handles the OK event
void
CDlgProgressAcquire::OnCloseOk() {

	}

//This function handles the localize action
void 
CDlgProgressAcquire::Localize() {

	LocalizeWindowCaption(IDS_PROGRESS);
	LocalizeControl(IDC_PROGRESSCAPTION, IDS_ACQUIRINGIMAGES);
	LocalizeControl(IDCANCEL, IDS_CANCEL);
	LocalizeControl(IDC_YES, IDS_YES);
	LocalizeControl(IDC_NOANSWER, IDS_NO);
	LocalizeControl(IDC_NEXTPAGE, IDS_CONTINUE);
	LocalizeControl(IDC_MESSAGE, IDS_NULL);
	LocalizeControl(IDC_ASKTOSCANNEXTPAGE, IDS_ASKTOSCANNEXTPAGE);
	LocalizeControl(IDC_AUTOSAVEDOCUMENT, IDS_AUTOSAVEDOCUMENT);
	LocalizeControl(IDC_ASDOCUMENTTYPECAPTION, IDS_TYPE);
	LocalizeControl(IDC_USEFEEDER, IDS_USEFEEDER);

	LocalizeStatusBeforeAcquire();
}

void 
CDlgProgressAcquire::LocalizeStatusBeforeAcquire() {

	HWND hControl;
	wchar_t* cText;
	wchar_t* cFormat;
	CScanSettings* oSettings;
	CScanDevice* oScanDevice;

	hControl = GetDlgItem(hDlg, IDC_STATUS);
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);
	cFormat = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	LoadString(oGlobalInstances.hLanguage, IDS_ACQUIRINGFROM, cFormat, 400);

	oSettings = (CScanSettings*)oScanSettings;
	oScanDevice = oSettings->GetScanner();
	
	if (oScanDevice!=NULL) {

		swprintf_s(cText, 400, cFormat, oScanDevice->cScanningDevice.c_str());
		}

	SetWindowText(hControl, cText);


	free(cText);
	}	
	
void 
CDlgProgressAcquire::LocalizeStatusAfterSuccessfulAcquire() {

	HWND hControl;
	wchar_t* cText;
	wchar_t* cFormat;
	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	hControl = GetDlgItem(hDlg, IDC_STATUS);
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);
	cFormat = (wchar_t*)malloc(sizeof(wchar_t) * 200);

	LoadString(oGlobalInstances.hLanguage, IDS_ACQUIREDONE, cFormat, 200);

	swprintf_s(cText, 400, cFormat, oCMCore->oScanCore->ImagesAvailable());

	SetWindowText(hControl, cText);

	free(cFormat);
	free(cText);
	}

void 
CDlgProgressAcquire::LocalizeStatusAfterUnsuccessfulAcquire() {

	HWND hControl;
	wchar_t* cText;
	wchar_t* cFormat;
	wchar_t* cMessage;
	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	hControl = GetDlgItem(hDlg, IDC_STATUS);
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);
	cMessage = (wchar_t*)malloc(sizeof(wchar_t) * 200);
	cFormat = (wchar_t*)malloc(sizeof(wchar_t) * 200);

	LoadString(oGlobalInstances.hLanguage, IDS_ACQUIREUNSUCCESSFUL, cFormat, 200);
	LoadString(oGlobalInstances.hLanguage, oCMCore->oScanCore->GetLastErrorMessage(true).iMessageID, cMessage, 200);

	swprintf_s(cText, 400, cFormat, cMessage);

	SetWindowText(hControl, cText);

	free(cFormat);
	free(cMessage);
	free(cText);
	}

void 
CDlgProgressAcquire::LocalizeMessageCountdown() {

	HWND hControl;
	wchar_t* cText;
	wchar_t* cFormat;
	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	hControl = GetDlgItem(hDlg, IDC_MESSAGE);
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);
	cFormat = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	LoadString(oGlobalInstances.hLanguage, IDS_ACQUIREANOTHERPAGE, cFormat, 400);

	swprintf_s(cText, 400, cFormat, lCountdownTimer);

	SetWindowText(hControl, cText);

	free(cText);
}


void 
CDlgProgressAcquire::SetProgress(int piValue) {

	HWND hProgressBar;

	hProgressBar = GetDlgItem(hDlg, IDC_PROGRESS);
    if (hProgressBar!=NULL) {
        SendMessage(hProgressBar, PBM_SETPOS, piValue, 0); 
        }
	}

void 
CDlgProgressAcquire::ShowEnableControl(int piCommand, bool pbEnabled) {

	HWND hControl;

	hControl = GetDlgItem(hDlg, piCommand);
	ShowWindow(hControl, pbEnabled ? SW_SHOW : SW_HIDE);
	EnableWindow(hControl, pbEnabled);
	}

void 
CDlgProgressAcquire::DoFillControls() {

	CCopyingMachineCore* oCore;
	CScanSettings* oSettings;

	oSettings = (CScanSettings*)oScanSettings;

	//Fill the checkbox 'ask for next page'
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	SendDlgItemMessage(hDlg, IDC_ASKTOSCANNEXTPAGE, BM_SETCHECK, oSettings->GetBool(eAskScanNextPage) ? 1 : 0, 0);

	//Fill the checkbox 'use the document feeder'
	SendDlgItemMessage(hDlg, IDC_USEFEEDER, BM_SETCHECK, oSettings->GetBool(eUseFeeder) ? 1 : 0, 0);

	//Fill the autosave checkbox
	SendDlgItemMessage(hDlg, IDC_AUTOSAVEDOCUMENT, BM_SETCHECK, oGlobalInstances.oRegistry->ReadInt(L"General", L"AutoSave", 0) == 0 ? BST_UNCHECKED : BST_CHECKED, 0);

	//Fill the autosave documenttype combobox, and enable/disbale this control
	DoFillAutoSaveComboBox();	
	DoEnableControls();
	}

//This function fills the combobox with the document types for the
//autosave feature
void 
CDlgProgressAcquire::DoFillAutoSaveComboBox() {

	CDocumentCollections* oDocumentCollection;
	CCopyingMachineCore* oCore;
	int iSelectedItem;
	sDocumentCollection oDocumentType;

	//Fill the dropdown box with the document types
	long lIndex;
	long lStringIndex;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oDocumentCollection = oCore->oDocumentCollections;

	iSelectedItem = oGlobalInstances.oRegistry->ReadInt(L"General", L"AutoSaveDocumentType", 0);
	lIndex = (long)oDocumentCollection->GetDocumentCollectionCount();
	if ((iSelectedItem > lIndex) || (iSelectedItem < 0)) iSelectedItem = 0;

	CScanSettings* oSettings;
	oSettings = (CScanSettings*)oScanSettings;

	if (oSettings->cAcquireType == eAcquireNewDocumentToCollection) {

		iSelectedItem = oSettings->iCollection;
		}

	while (lIndex>0) {

		lIndex--;

		oDocumentCollection->GetDocumentCollection(lIndex, oDocumentType);

		lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_INSERTSTRING, (WPARAM)0, (LPARAM)oDocumentType.sName.c_str());
		SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)lIndex);
		}

	SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_SETCURSEL, (WPARAM)iSelectedItem, 0);
	
	if (oSettings->cAcquireType != eAcquireNewDocumentToCollection) {

		//reset the registry setting to make sure the combobox and the registry contain
		//the same values
		iSelectedItem = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETCURSEL, 0, 0);
		oGlobalInstances.oRegistry->WriteInt(L"General", L"AutoSaveDocumentType", iSelectedItem);
		}
	}


void 
CDlgProgressAcquire::DoApplySettings() {

	CCopyingMachineCore* oCore;

	//Set the 'ask for a next page' checkbox
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oCore->oScanSettings->SetBool(eAskScanNextPage, SendDlgItemMessage(hDlg, IDC_ASKTOSCANNEXTPAGE, BM_GETCHECK, 0, 0)==BST_CHECKED); 
	oCore->oScanSettings->SetBool(eUseFeeder, SendDlgItemMessage(hDlg, IDC_USEFEEDER, BM_GETCHECK, 0, 0)==BST_CHECKED); 

	//Set the autosave option
	oGlobalInstances.oRegistry->WriteInt(L"General", L"AutoSave", SendDlgItemMessage(hDlg, IDC_AUTOSAVEDOCUMENT, BM_GETCHECK, 0, 0)==BST_CHECKED ? 1 : 0);

	//Set the selected autosave document type
	int iSelectedItem;
	iSelectedItem = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETCURSEL, 0, 0);

	oGlobalInstances.oRegistry->WriteInt(L"General", L"AutoSaveDocumentType", iSelectedItem);
	
	//Enable/disable the controls. This might have been changed
	DoEnableControls();
}

void
CDlgProgressAcquire::DoEnableControls() {

	bool bAutoSaveEnabled;
	bool bAutoSave;
	bool bFeederPresent;

	CScanSettings* oSettings;

	oSettings = (CScanSettings*)oScanSettings;

	CScanDevice* oDevice;

	oDevice = oSettings->GetScanner();
	bFeederPresent = oDevice->bFeederPresent;

	bAutoSave = (oSettings->cAcquireType == eAcquireNewDocument);
	bAutoSaveEnabled = (SendDlgItemMessage(hDlg, IDC_AUTOSAVEDOCUMENT, BM_GETCHECK, 0, 0)==BST_CHECKED);

	EnableControl(IDC_AUTOSAVEDOCUMENT, bAutoSave);
	EnableControl(IDC_ASDOCUMENTTYPECAPTION, bAutoSaveEnabled && bAutoSave);
	EnableControl(IDC_SAVETYPE, bAutoSaveEnabled && bAutoSave);
	EnableControl(IDC_USEFEEDER, bFeederPresent);
	}

