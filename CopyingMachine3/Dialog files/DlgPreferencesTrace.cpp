#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPreferencesTrace::CDlgPreferencesTrace(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
	CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {
}

CDlgPreferencesTrace::~CDlgPreferencesTrace(void)
{
	if (hDlg!=NULL) DestroyWindow(hDlg);
}

void 
CDlgPreferencesTrace::OnInitDialog() {

	//set the checkbox
	bool bTrace = (bool)(oGlobalInstances.oRegistry->ReadInt(L"Debug", L"DoTrace", 0)!=0);
	SendDlgItemMessage(hDlg, IDC_TRACE, BM_SETCHECK, bTrace ? BST_CHECKED : BST_UNCHECKED, 0);
	}

BOOL 
CDlgPreferencesTrace::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_CAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}

			break;
		};	


	return bReturn;
	}

BOOL 
CDlgPreferencesTrace::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	switch (iCommand) {

		case IDC_TRACE: SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesTrace::OnCloseOk() {


	}

//This function handles the localize action
void 
CDlgPreferencesTrace::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	LocalizeControl(IDC_TOPIC1, IDS_TRACING);
	LocalizeControl(IDC_TRACEDESCRIPTION, IDS_TRACEDESCRIPTION);
	LocalizeControl(IDC_TRACE, IDS_TRACEACTIONS);
}

//This function handles the Apply event
void 
CDlgPreferencesTrace::OnApply() {

	//set the checkbox
	long lResult = (long)SendDlgItemMessage(hDlg, IDC_TRACE,	BM_GETCHECK, 0, 0);
	oGlobalInstances.oRegistry->WriteInt(L"Debug", L"DoTrace", lResult == BST_CHECKED ? 1 : 0);

}