#include "StdAfx.h"
#include "..\Dialog Files\DialogTemplate.h"
#include "DlgTOcrDebug.h"

CDlgTOcrDebug::CDlgTOcrDebug(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {

	}

CDlgTOcrDebug::~CDlgTOcrDebug(void)
{

}

//This function is called when the dialog is initialised
void 
CDlgTOcrDebug::OnInitDialog() {

	//set the font of the caption
	SetDlgItemFont(IDC_CAPTION, hCaptionFont);

	}

BOOL 
CDlgTOcrDebug::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

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
CDlgTOcrDebug::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {


	return FALSE;
	}

//This function handles the OK event
void
CDlgTOcrDebug::OnCloseOk() {

	}
