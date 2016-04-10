#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgProgressPrint::CDlgProgressPrint(HWND phParent, void* poDocument, void* poPrintSettings, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(poGlobalInstances.hInstance, phParent, IDD_PROGRESSDIALOG, poGlobalInstances) {

	//Initialise variables
	hDlg			= NULL;
	oPrintSettings	= poPrintSettings;
	oDocument		= poDocument;
	}

CDlgProgressPrint::~CDlgProgressPrint(void)
{
	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	oCMCore->oNotifications->UnsubscribeAll(hDlg);	
	if (hDlg!=NULL) DestroyWindow(hDlg);
}

//This function is called when the dialog is initialised
void 
CDlgProgressPrint::OnInitDialog() {

	CCopyingMachineCore* oCMCore;
	CPrintSettings* oSettings;

	oCMCore = (CCopyingMachineCore*)oCore;

	//set the font of the caption
	SetDlgItemFont(IDC_CAPTION, hCaptionFont);

	Localize();
	SetDlgItemFont(IDC_PROGRESSCAPTION, hCaptionFont);

	//Set the events	
	oCMCore->oNotifications->Subscribe(hDlg, eNotificationDocumentPrintingDone);

	//Initialize the settings
	HWND hProgressBar;
	HWND hStatusWindow;

	hProgressBar = GetDlgItem(hDlg, IDC_PROGRESS);
    if (hProgressBar!=NULL) {
        SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); 
        }
	hStatusWindow = GetDlgItem(hDlg, IDC_STATUS);

	oSettings = (CPrintSettings*)oPrintSettings;
	oSettings->hProgressBar		= hProgressBar;
	oSettings->hStatusWindow	= hStatusWindow;
	oSettings->hWindow			= hDlg;

	//Start printing images
	CCopyDocument* oPrintDocument;

	oPrintDocument = (CCopyDocument*)oDocument;
	iPrintTicket = oPrintDocument->PrintThread(oSettings);

	//Show error message, if any
	//oCMCore->oPrintCore->ShowErrorMessage(NULL);
	}

BOOL 
CDlgProgressPrint::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

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

		case WM_CORENOTIFICATION:

			OnEventCoreNotification();

			bReturn=true;
			break;

		default:

			bReturn = false;
			break;
		
		};	

	return bReturn;
	}

//This function handles the core event notifications
bool 
CDlgProgressPrint::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;
	CCopyingMachineCore* oCore;

	bReturn = false;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	while (oCore->oNotifications->GetNotification(hDlg, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationDocumentPrintingDone:
				
				sPrintDocumentWithSettings* oPrintDocument;

				oPrintDocument = (sPrintDocumentWithSettings*)cNotification.pData;
				
				if (iPrintTicket == oPrintDocument->iPrintRequest) {

					EndDialog(hDlg, IDCANCEL);
					}
				break;
			}
		}

	return bReturn;
	}

BOOL 
CDlgProgressPrint::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	BOOL bResult;
	CCopyingMachineCore* oCMCore;
	CPrintSettings*		 oSettings;

	bResult = FALSE;
	oCMCore = (CCopyingMachineCore*)oCore;

	switch (iCommand) {

		case IDCANCEL:

			//Stop messaging from the thread
			oSettings = (CPrintSettings*)oPrintSettings;
			oSettings->hStatusWindow = NULL;

			//Show the cancelling task status
			LocalizeControl(IDC_STATUS, IDS_CANCELLINGTASK);

			//Notify the thread to stop
			oCMCore->oPrintCore->SetEndThread();
			bResult = TRUE;
			break;
		}

	return bResult;
	}

//This function handles the OK event
void
CDlgProgressPrint::OnCloseOk() {

	}

//This function handles the localize action
void 
CDlgProgressPrint::Localize() {

	LocalizeWindowCaption(IDS_PROGRESS);
	LocalizeControl(IDC_PROGRESSCAPTION,	IDS_COPYINGDOCUMENT);
	LocalizeControl(IDCANCEL,				IDS_CANCEL);
	LocalizeControl(IDC_YES,				IDS_YES);
	LocalizeControl(IDC_NOANSWER,			IDS_NO);
	LocalizeControl(IDC_NEXTPAGE,			IDS_CONTINUE);
	LocalizeControl(IDC_MESSAGE,			IDS_NULL);
	LocalizeControl(IDC_STATUS,				IDS_NULL);
	LocalizeControl(IDC_ASKTOSCANNEXTPAGE,	IDS_ASKTOSCANNEXTPAGE);

	ShowEnableControl(IDC_YES,				false);
	ShowEnableControl(IDC_NOANSWER,			false);
	ShowEnableControl(IDC_NEXTPAGE,			false);
	ShowEnableControl(IDC_MESSAGE,			false);
	ShowEnableControl(IDC_ASKTOSCANNEXTPAGE,false);
}

void 
CDlgProgressPrint::SetProgress(int piValue) {

	HWND hProgressBar;

	hProgressBar = GetDlgItem(hDlg, IDC_PROGRESS);
    if (hProgressBar!=NULL) {
        SendMessage(hProgressBar, PBM_SETPOS, piValue, 0); 
        }
	}

void 
CDlgProgressPrint::ShowEnableControl(int piCommand, bool pbEnabled) {

	HWND hControl;

	hControl = GetDlgItem(hDlg, piCommand);
	ShowWindow(hControl, pbEnabled ? SW_SHOW : SW_HIDE);
	EnableWindow(hControl, pbEnabled);
	}

