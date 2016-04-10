#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgReminder::CDlgReminder(HINSTANCE phInstance, HWND phParent, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, IDD_REMINDER, poGlobalInstances) {

	
	}

CDlgReminder::~CDlgReminder(void)
{

	KillTimer(hDlg, 2493);	
	}

//This function is called when the dialog is initialised
void 
CDlgReminder::OnInitDialog() {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	//Initialize the progress control
	lCountDown = 10;
	hProgressControl = GetDlgItem(hDlg, IDC_COUNTDOWN);
	SendMessage(hProgressControl, PBM_SETRANGE32, 0, 100);
	SendMessage(hProgressControl, PBM_SETPOS, 100, 0);

	SetTimer(hDlg, 2493, 1000, NULL);             
	
	//set the font of the caption
	SetDlgItemFont(IDC_ABOUTCAPTION, hCaptionFont);

	Localize();
	}

BOOL 
CDlgReminder::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

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
             if ((HWND) lParam == GetDlgItem(hDlg, IDC_UNREGISTEREDLABEL)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}
			break;

		case WM_TIMER:
		
			if (lCountDown>0) {
				lCountDown--;
				SendMessage(hProgressControl, PBM_SETPOS, lCountDown*10, 0);
				
				//Enable OK button	
				if (lCountDown==0) {

					EnableControl(IDOK, true);
					}
				}
			
			break;
		};	


	return bReturn;
	}

BOOL 
CDlgReminder::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	switch (iCommand) {

		case IDC_REGISTERNOW:

			::ShellExecute(NULL, L"open", L"http://copyingmachine.meusesoft.com", NULL, NULL, SW_SHOWNORMAL);
			break;
		case IDC_PLACEKEY:

			oCMCore->oDialogs->DlgPlaceKey(hDlg);
			if (oCMCore->oRegistration->bRegistered) {

				//we are registered, close this dialog now			
				lCountDown=0;
				SendMessage(hProgressControl, PBM_SETPOS, lCountDown*10, 0);
				EnableControl(IDOK, true);
				EndDialog(hDlg, IDOK);
				}
			break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgReminder::OnCloseOk() {




	}

//This function localizes the text labes in the dialog
void
CDlgReminder::Localize() {

	CCopyingMachineCore* oCMCore;
	
	oCMCore = (CCopyingMachineCore*)oCore;

	LocalizeWindowCaption(IDS_UNREGISTERED);

	LocalizeControl(IDC_UNREGISTEREDLABEL, IDS_UNREGISTERED);

	LocalizeControl(IDC_TOPIC1,			IDS_INFORMATION);
	LocalizeControl(IDC_REGISTERINFO,	IDS_REGISTERINFO);
	LocalizeControl(IDC_REGISTERNOW,	IDS_REGISTERNOW);
	LocalizeControl(IDC_PLACEKEY,		IDS_PLACEKEY);
	LocalizeControl(IDOK,				IDS_OK);

	}

