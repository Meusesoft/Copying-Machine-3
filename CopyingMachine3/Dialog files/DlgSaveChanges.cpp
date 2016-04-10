#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgSaveChanges::CDlgSaveChanges(HINSTANCE phInstance, HWND phParent, std::wstring psMessage, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, IDD_SAVEDIRTYDOCUMENT, poGlobalInstances) {

	sMessage = psMessage;
	}

CDlgSaveChanges::~CDlgSaveChanges(void)
{
}

//This function is called when the dialog is initialised
void 
CDlgSaveChanges::OnInitDialog() {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	//Initialize the progress control
	
	//set the font of the caption
	//SetDlgItemFont(IDC_ABOUTCAPTION, hCaptionFont);
	
	//Set the question mark icon
	HICON hQuestionMark = LoadIcon(NULL, IDI_QUESTION);
	SendDlgItemMessage(hDlg, IDC_ICONQUESTIONMARK, STM_SETICON, (WPARAM)hQuestionMark, 0);

	Localize();
	}

BOOL 
CDlgSaveChanges::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             /*if ((HWND) lParam == GetDlgItem(hDlg, IDC_ABOUTCAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}
             if ((HWND) lParam == GetDlgItem(hDlg, IDC_COPYINGMACHINEICON)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}
             if ((HWND) lParam == GetDlgItem(hDlg, IDC_UNREGISTEREDLABEL)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}*/
			break;

		};	


	return bReturn;
	}

BOOL 
CDlgSaveChanges::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	switch (iCommand) {

		case IDYES:

			EndDialog(hDlg, IDYES);
			break;

		case IDYES2:

			EndDialog(hDlg, IDYES2);
			break;

		case IDNO:

			EndDialog(hDlg, IDNO);
			break;

		case IDNO2:

			EndDialog(hDlg, IDNO2);
			break;
	}

	return FALSE;
	}

//This function handles the OK event
void
CDlgSaveChanges::OnCloseOk() {




	}

//This function localizes the text labes in the dialog
void
CDlgSaveChanges::Localize() {

	HWND hControl = GetDlgItem(hDlg, IDC_MESSAGE);
	SetWindowText(hControl, sMessage.c_str());

	LocalizeControl(IDNO,				IDS_NO);
	LocalizeControl(IDCANCEL,			IDS_CANCEL);
	LocalizeControl(IDYES,				IDS_YES);
	LocalizeControl(IDYES2,				IDS_YESTOALL);
	LocalizeControl(IDNO2,				IDS_NOTOALL);
	}

