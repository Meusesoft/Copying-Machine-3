#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgUnknownEvent::CDlgUnknownEvent(sGlobalInstances poGlobalInstances, HWND phParent, std::wstring psEvent) :
				  CDialogTemplate(poGlobalInstances.hInstance, phParent, IDD_UNKNOWNEVENT, poGlobalInstances) {

	
	sEvent = psEvent;
	iDialogResult = -1;
	}

CDlgUnknownEvent::~CDlgUnknownEvent(void)
{

	}

//This function is called when the dialog is initialised
void 
CDlgUnknownEvent::OnInitDialog() {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	//set the font of the caption
	SetDlgItemFont(IDC_EVENTCAPTION, hCaptionFont);

	SendDlgItemMessage(hDlg, IDC_CHKDOALWAYSTHISEVENT, BM_SETCHECK, BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_EVENTNOTHING, BM_SETCHECK, BST_CHECKED, 0);

	Localize();
	}

BOOL 
CDlgUnknownEvent::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_EVENTCAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}
			break;

		};	


	return bReturn;
	}

BOOL 
CDlgUnknownEvent::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	switch (iCommand) {

		case IDOK:

				OnCloseOk();
			break;

		case IDCANCEL:


				EndDialog(hDlg, -1);
			break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgUnknownEvent::OnCloseOk() {

	int iResult;
	CRegistry* oRegistry;

	//Get the selected radio button
	if (SendDlgItemMessage(hDlg, IDC_EVENTNOTHING,	BM_GETCHECK, 0, 0)==BST_CHECKED) iResult = 0;
	if (SendDlgItemMessage(hDlg, IDC_EVENTSCAN,		BM_GETCHECK, 0, 0)==BST_CHECKED) iResult = 1;
	if (SendDlgItemMessage(hDlg, IDC_EVENTCOPY,		BM_GETCHECK, 0, 0)==BST_CHECKED) iResult = 2;
	if (SendDlgItemMessage(hDlg, IDC_EVENTEMAIL,	BM_GETCHECK, 0, 0)==BST_CHECKED) iResult = 3;

	//If checked then save this preference to the registry
	if (SendDlgItemMessage(hDlg, IDC_CHKDOALWAYSTHISEVENT,BM_GETCHECK, 0, 0)==BST_CHECKED) {

		oRegistry = (CRegistry*)oGlobalInstances.oRegistry;
		oRegistry->WriteInt(L"StillImage", sEvent, iResult);
		}

	iDialogResult = iResult;
	
	EndDialog(hDlg, iResult);
	}

//This function localizes the text labes in the dialog
void
CDlgUnknownEvent::Localize() {

	CCopyingMachineCore* oCMCore;
	
	oCMCore = (CCopyingMachineCore*)oCore;

	LocalizeControl(IDC_EVENTCAPTION,				IDS_UNKNOWNEVENT);
	LocalizeControl(IDC_DESCRIPTIONUNKNWONEVENT,	IDS_UNKNONWEVENTDESC);
	
	LocalizeControl(IDC_CHKDOALWAYSTHISEVENT,		IDS_REMEMBEREVENT);
	
	LocalizeControl(IDC_EVENTSCAN,					IDS_ACQUIRE);
	LocalizeControl(IDC_EVENTCOPY,					IDS_COPY);
	LocalizeControl(IDC_EVENTEMAIL,					IDS_EMAIL);
	
	LocalizeControl(IDOK,							IDS_OK);
	}

