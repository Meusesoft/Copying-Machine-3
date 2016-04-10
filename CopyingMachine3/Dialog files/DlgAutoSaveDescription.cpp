#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgAutoSaveDescription::CDlgAutoSaveDescription(HINSTANCE phInstance, HWND phParent, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, IDD_AUTOSAVEDESCRIPTION, poGlobalInstances) {

	CCopyingMachineCore* oCore;

	//Initialize variables				  
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oDocumentCollections = oCore->oDocumentCollections;
	oEditControlValidation = NULL;

	iDocumentType = 0;
	iDocumentId = 1;
	}

CDlgAutoSaveDescription::~CDlgAutoSaveDescription(void)
{
	if (oEditControlValidation!=NULL) delete oEditControlValidation;
}

//This function is called when the dialog is initialised
void 
CDlgAutoSaveDescription::OnInitDialog() {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	//set the font of the caption
	SetDlgItemFont(IDC_DIALOGCAPTION, hCaptionFont);

	Localize();

	//add validation to the edit control
	HWND hEditControl;
	hEditControl = GetDlgItem(hDlg, IDC_EDITDESCRIPTION);

	oEditControlValidation = new sEditControlValidation;
	oEditControlValidation->oGlobalInstances = &oGlobalInstances;
	oEditControlValidation->lOldProc = SetWindowLong(hEditControl , GWLP_WNDPROC, (LONG)EditValidateFilename);
	SetWindowLong(hEditControl, GWLP_USERDATA, (long)oEditControlValidation);

	DoShowControls();

	OnChangeDescription();
	}

BOOL 
CDlgAutoSaveDescription::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_DIALOGCAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}
             if ((HWND) lParam == GetDlgItem(hDlg, IDC_COPYINGMACHINEICON)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}

			break;

		};	


	return bReturn;
	}

BOOL 
CDlgAutoSaveDescription::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	switch (iCommand) {

		case IDC_EDITDESCRIPTION:

			if (HIWORD(wParam)==EN_CHANGE) {
				
				OnChangeDescription();
				}
			break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgAutoSaveDescription::OnCloseOk() {




	}

//This function handles the OK event
void
CDlgAutoSaveDescription::Localize() {

	CCopyingMachineCore* oCMCore;
	sExecutableInformation	 cVersionInformation;
	
	oCMCore = (CCopyingMachineCore*)oCore;

	LocalizeWindowCaption(IDS_AUTOSAVE);
	LocalizeControl(IDC_DIALOGCAPTION,	IDS_NEWDOCUMENT);
	LocalizeControl(IDC_TOPIC1,			IDS_ENTERDESCRIPTION);
	LocalizeControl(IDC_TOPIC2,			IDS_NEWDOCUMENTNAME);
	LocalizeControl(IDC_DIALOGCAPTION,	IDS_NEWDOCUMENT);
	LocalizeControl(IDOK,				IDS_OK);
	LocalizeControl(IDCANCEL,			IDS_CANCEL);
	}

//This functions shows or hides controls
void
CDlgAutoSaveDescription::DoShowControls() {

	CCopyingMachineCore* oCMCore;
	oCMCore = (CCopyingMachineCore*)oCore;

	}

//This function sets the current document type
void
CDlgAutoSaveDescription::SetDocumentCollection(int piDocumentType) {

	iDocumentType = piDocumentType;
	}

//This function sets the current document id
void
CDlgAutoSaveDescription::SetDocumentId(int piDocumentId) {

	iDocumentId = piDocumentId;
	}

//This function returns the description
std::wstring
CDlgAutoSaveDescription::GetDescription() {

	return sDescription;
	}

//This function processes changes to the description:
void 
CDlgAutoSaveDescription::OnChangeDescription() {

	std::wstring sFilename;
	
	//read out the description out of the edit control
	sDescription = GetStringFromEditControl(IDC_EDITDESCRIPTION);

	//Generate a preview for the filename
	oDocumentCollections->SetDescription(sDescription);
	sFilename = oDocumentCollections->CreateFilename(iDocumentType, iDocumentId);

	//update the control to display the updated filename
	SendDlgItemMessage(hDlg, IDC_STATICDOCUMENTTYPENAME, WM_SETTEXT, 0, (LPARAM)sFilename.c_str());
	}


