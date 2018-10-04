#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgAbout::CDlgAbout(HINSTANCE phInstance, HWND phParent, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, 
#ifdef UWP
					  IDD_ABOUTBOXUWP,
					  #else		   
					  IDD_ABOUTBOX,
#endif
					  poGlobalInstances) {

	
	}

CDlgAbout::~CDlgAbout(void)
{
}

//This function is called when the dialog is initialised
void 
CDlgAbout::OnInitDialog() {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	//set the font of the caption
	SetDlgItemFont(IDC_ABOUTCAPTION, hCaptionFont);

	Localize();
	DoShowControls();
	}

BOOL 
CDlgAbout::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_ABOUTCAPTION)) { 
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
CDlgAbout::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	switch (iCommand) {

		case IDC_REGISTERNOW:

			::ShellExecute(NULL, L"open", L"http://copyingmachine.meusesoft.com", NULL, NULL, SW_SHOWNORMAL);
			break;
		case IDC_PLACEKEY:

			oCMCore->oDialogs->DlgPlaceKey(hDlg);
			Localize();
			DoShowControls();
			break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgAbout::OnCloseOk() {




	}

//This function handles the OK event
void
CDlgAbout::Localize() {

	CCopyingMachineCore* oCMCore;
	HWND				 hControl;
	wchar_t				 cText[100];
	wchar_t				 cTemplate[100];
	sExecutableInformation	 cVersionInformation;
	
	oCMCore = (CCopyingMachineCore*)oCore;

	LocalizeWindowCaption(IDS_ABOUT);

	LocalizeControl(IDC_TOPIC1,			IDS_INFORMATION);
	LocalizeControl(IDC_TOPIC2,			oCMCore->oRegistration->bRegistered ? IDS_REGISTEREDTO : IDS_UNREGISTERED);
	
	LocalizeControl(IDC_ABOUTCAPTION,	IDS_ABOUTCM);
	LocalizeControl(IDC_REGISTERNOW,	IDS_REGISTERNOW);
	LocalizeControl(IDC_PLACEKEY,		IDS_PLACEKEY);
	LocalizeControl(IDOK,				IDS_OK);
	LocalizeControl(IDC_VERSION,		IDS_CMVERSION);
	LocalizeControl(IDC_COPYRIGHT,		IDS_CMCOPYRIGHT);

	oCMCore->GetCopyingMachineVersion(cVersionInformation);
	LoadString(oGlobalInstances.hLanguage, IDS_CMVERSION, cTemplate, 100);
	swprintf_s(cText, 100, cTemplate, cVersionInformation.dVersion >> 16, cVersionInformation.dVersion & 0xFFFF);
	hControl = GetDlgItem(hDlg, IDC_VERSION);
	SetWindowText(hControl, cText);

	LoadString(oGlobalInstances.hLanguage, IDS_CMCOPYRIGHT, cTemplate, 100);
	swprintf_s(cText, 100, cTemplate, cVersionInformation.sLegalCopyright.c_str());
	hControl = GetDlgItem(hDlg, IDC_COPYRIGHT);
	SetWindowText(hControl, cText);

	LoadString(oGlobalInstances.hLanguage, IDS_REGISTEREDNAME, cTemplate, 100);
	swprintf_s(cText, 100, cTemplate, oCMCore->oRegistration->sName.c_str());
	hControl = GetDlgItem(hDlg, IDC_NAME);
	SetWindowText(hControl, cText);

	LoadString(oGlobalInstances.hLanguage, IDS_REGISTEREDORG, cTemplate, 100);
	swprintf_s(cText, 100, cTemplate, oCMCore->oRegistration->sOrganisation.c_str());
	hControl = GetDlgItem(hDlg, IDC_ORGANISATION);
	SetWindowText(hControl, cText);
	}

//This functions shows or hides controls
void
CDlgAbout::DoShowControls() {

	CCopyingMachineCore* oCMCore;
	oCMCore = (CCopyingMachineCore*)oCore;

	ShowControl(IDC_REGISTERNOW,	!oCMCore->oRegistration->bRegistered);
	ShowControl(IDC_PLACEKEY,		!oCMCore->oRegistration->bRegistered);
	ShowControl(IDC_ORGANISATION,	oCMCore->oRegistration->bRegistered);
	ShowControl(IDC_NAME,			oCMCore->oRegistration->bRegistered);
	}

