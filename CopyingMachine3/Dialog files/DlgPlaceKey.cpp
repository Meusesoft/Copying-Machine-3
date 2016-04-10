#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPlaceKey::CDlgPlaceKey(HINSTANCE phInstance, HWND phParent, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, IDD_PLACEKEY, poGlobalInstances) {

	
	}

CDlgPlaceKey::~CDlgPlaceKey(void)
{
}

//This function is called when the dialog is initialised
void 
CDlgPlaceKey::OnInitDialog() {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	Localize();
	}

BOOL 
CDlgPlaceKey::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

			break;

		};	


	return bReturn;
	}

BOOL 
CDlgPlaceKey::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	switch (iCommand) {

		case IDC_LOADKEY:
			
			LoadKey();
			break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPlaceKey::OnCloseOk() {

	wchar_t* cTextBuffer;
	std::wstring sKeyText;
	long lTextSize;
	HWND hEditControl;
	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	hEditControl = GetDlgItem(hDlg, IDC_KEYTEXT);
	lTextSize = (long)SendMessage(hEditControl, WM_GETTEXTLENGTH, 0, 0);

	if ((lTextSize>0) && !oCmCore->oRegistration->bRegistered) {

		cTextBuffer = (wchar_t*)malloc(sizeof(wchar_t) * (lTextSize+1));
		SendMessage(hEditControl, WM_GETTEXT, (WPARAM)(lTextSize+1), (LPARAM)cTextBuffer);

		sKeyText = cTextBuffer;
		
		oCmCore->oRegistration->PlaceKeyText(sKeyText);

		free(cTextBuffer);

		MessageKeyValid();
		}
	}

//This function handles the OK event
void
CDlgPlaceKey::Localize() {

	LocalizeWindowCaption(IDS_PLACEKEY);

	LocalizeControl(IDC_TOPIC1,			IDS_PLACEKEYFROMTEXT);
	LocalizeControl(IDC_TOPIC2,			IDS_PLACEKEYFROMFILE);
	LocalizeControl(IDOK,				IDS_OK);
	LocalizeControl(IDCANCEL,			IDS_CANCEL);
	LocalizeControl(IDC_LOADKEY,		IDS_LOAD);
	}

void	
CDlgPlaceKey::LoadKey() {

	bool bResult;
	wchar_t cFilename[MAX_PATH];
	int iDefaultFilter;
	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	oTrace->StartTrace(__WFUNCTION__);

	iDefaultFilter = 0;
	SecureZeroMemory(cFilename, sizeof(cFilename));
	
	bResult = oCmCore->oDialogs->DlgOpenFile(hDlg, L"Copying Machine Key (*.cmk)\0*.cmk\x00\x00", iDefaultFilter, cFilename);

	if (bResult) {

		oCmCore->oRegistration->PlaceKeyFile(cFilename);

		MessageKeyValid();

		if (oCmCore->oRegistration->bRegistered) EndDialog(hDlg, IDOK);
		}
	}

void	
CDlgPlaceKey::MessageKeyValid() {

	wchar_t  cMessage[200];
	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	//Check if the registration was succesfull
	if (oCmCore->oRegistration->bRegistered) {

		LoadString(oGlobalInstances.hLanguage, IDS_THANKYOU, cMessage, 200);
		MessageBox(hDlg, cMessage, L"Copying Machine", MB_OK);
		}
	else {

		LoadString(oGlobalInstances.hLanguage, IDS_KEYNOTVALID, cMessage, 200);
		MessageBox(hDlg, cMessage, L"Copying Machine", MB_OK | MB_ICONSTOP);
		}
	}

