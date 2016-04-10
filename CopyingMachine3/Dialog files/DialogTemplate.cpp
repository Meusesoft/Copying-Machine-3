#include "stdafx.h"
#include "Dialogs.h"

//static CDialogTemplate* oThisDialog;

CDialogTemplate::CDialogTemplate(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) {

	oGlobalInstances = poGlobalInstances;

	oTrace = poGlobalInstances.oTrace;
	oCore = poGlobalInstances.oCopyingMachineCore;
	iDialogResource = piResource;
	hParent = phParent;
	hInstance = phInstance;

	hCaptionFont = NULL;
	hDlg = NULL;

	LOGFONT cLogFont;

	SecureZeroMemory(&cLogFont, sizeof(cLogFont));
	wcscpy_s(cLogFont.lfFaceName, 32, L"Tahoma");
	cLogFont.lfHeight = 17;
	cLogFont.lfWeight = FW_BOLD;
	hCaptionFont = CreateFontIndirect(&cLogFont);
	}

CDialogTemplate::~CDialogTemplate() {

	if (hCaptionFont!=NULL) DeleteObject(hCaptionFont);

	}


//This function executes the dialog as a modal one and returns the result.
int 
CDialogTemplate::Execute() {

	int iReturn;

	oTrace->StartTrace(__WFUNCTION__);
	
	iReturn = (int)(DialogBoxParam(hInstance, MAKEINTRESOURCE(iDialogResource), 
					hParent, (DLGPROC)DialogTemplateProc, (LPARAM)this)); 

	oTrace->EndTrace(__WFUNCTION__, iReturn);	

	return iReturn;
	}

//This function creates a modeless dialog box
HWND
CDialogTemplate::Create() {

	HWND hwndReturn;

	oTrace->StartTrace(__WFUNCTION__);

	hwndReturn = CreateDialogParam(hInstance, MAKEINTRESOURCE(iDialogResource),
							  hParent, (DLGPROC)DialogTemplateProc, (LPARAM)this);
	hDlg = hwndReturn;

	oTrace->EndTrace(__WFUNCTION__, (int)hwndReturn);	

	return hwndReturn;
	}

//This is the callback message function for processing the dialog messages. This is a static function
INT_PTR 
CALLBACK CDialogTemplate::DialogTemplateProc(HWND hwndDlg, 
                             			UINT message, 
                             			WPARAM wParam, 
                             			LPARAM lParam) 
{ 
    
	BOOL bResult;
	CDialogTemplate* oThisDialog;

	bResult = FALSE;	
	oThisDialog = (CDialogTemplate*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (message) { 
        
		case WM_INITDIALOG:

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM)lParam); //save the pointer to the dialog 
			oThisDialog = (CDialogTemplate*)lParam;

			oThisDialog->hDlg = hwndDlg;

			oThisDialog->OnInitDialog();
			bResult = TRUE;
			break;


		case WM_COMMAND: 
            		
			if (oThisDialog) {
				bResult = oThisDialog->OnCommand(LOWORD(wParam), wParam, lParam);

				if (!bResult) {
					
					switch (LOWORD(wParam)) { 
	                
					case IDAPPLY:
						oThisDialog->OnApply();
						break;
					case IDOK: 
						oThisDialog->OnCloseOk();
					case IDCANCEL: 
						bResult = EndDialog(oThisDialog->hDlg, wParam); 
						break;
						}
					}
				}
			break;

		default:

			if (oThisDialog!=NULL) {
				bResult = oThisDialog->OnMessage(message, wParam, lParam);
				}
			break;

		} 
    
	return bResult; 
	}

void 
CDialogTemplate::OnInitDialog() {


	}

BOOL 
CDialogTemplate::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {


	return FALSE;
	}

BOOL 
CDialogTemplate::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {


	return FALSE;
	}

void
CDialogTemplate::OnCloseOk() {

}

//This function changes the font of an item in the dialog
bool    
CDialogTemplate::SetDlgItemFont(UINT uItem, HFONT hf) {

	if(hf != NULL) 
		SendMessage(GetDlgItem(hDlg, uItem), WM_SETFONT, (WPARAM)hf, (LPARAM) MAKELONG((WORD) TRUE,0)); 
	else 
		return false; 

        return true; 
	} 

//This function changes the caption/text of a control
void 
CDialogTemplate::LocalizeControl(int piControl, int piResource) {

	HWND hControl;
	wchar_t* cText;

	hControl = GetDlgItem(hDlg, piControl);
	cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

	LoadString(oGlobalInstances.hLanguage, piResource, cText, 400);

	SetWindowText(hControl, cText);

	free(cText);
}

void 
CDialogTemplate::LocalizeWindowCaption(int piResource) {

	wchar_t* cText;

	cText = (wchar_t*)malloc(sizeof(wchar_t) * MAX_PATH);

	LoadString(oGlobalInstances.hLanguage, piResource, cText, MAX_PATH);

	SetWindowText(hDlg, cText);

	free(cText);
	}


//This function localizes the controls
void 
CDialogTemplate::Localize(HINSTANCE phLanguage) {

	if (phLanguage!=NULL) oGlobalInstances.hLanguage=phLanguage;
}

//This function handles the Apply event
void 
CDialogTemplate::OnApply() {


}

//This function reads out an edit control and converts its value to
//an integer
int  
CDialogTemplate::GetIntegerFromEditControl(int piControl) {

	int iResult;
	wchar_t cNumber[10];

	SendDlgItemMessage(hDlg, piControl, WM_GETTEXT, (WPARAM)9, (LPARAM)cNumber);
	iResult = _wtoi(cNumber);

	return iResult;
	}

//This function reads out an edit control and returns the contents
//as a wstring
std::wstring 
CDialogTemplate::GetStringFromEditControl(int piControl) {

	std::wstring sResult;
	wchar_t *cText;
	int iTextLength;

	//Get the length of the text in the edit control
	iTextLength = SendDlgItemMessage(hDlg, piControl, WM_GETTEXTLENGTH, 0, 0);
	
	//Allocate the text buffer
	cText = new wchar_t[iTextLength+1];

	//Get the text from the edit control
	SendDlgItemMessage(hDlg, piControl, WM_GETTEXT, (WPARAM)iTextLength+1, (LPARAM)cText);

	//Process the result and clean up
	sResult = cText;
	delete[] cText;
	
	return sResult;
}

//This function shows or hides a control
void 
CDialogTemplate::ShowControl(int piControl, bool pbShow) {

	HWND hControl;

	hControl = GetDlgItem(hDlg, piControl);
	ShowWindow(hControl, pbShow ? SW_SHOW : SW_HIDE);
	}

//This function enables or disables a control
void 
CDialogTemplate::EnableControl(int piControl, bool pbEnable) {

	HWND hControl;

	hControl = GetDlgItem(hDlg, piControl);
	EnableWindow(hControl, pbEnable);
	}

//This function shows a balloon at the position of the given control
void 
CDialogTemplate::ShowBalloonTip(HINSTANCE phLanguage, HWND phControl, 
								int piMessage, int piTitle) {

	wchar_t cMessage[512];
	wchar_t cTitle[256];
	EDITBALLOONTIP cEditBalloon;

	//initialise
	cEditBalloon.cbStruct = sizeof(cEditBalloon);
	cEditBalloon.ttiIcon = 0;

	LoadString(phLanguage, piMessage, cMessage, 512);
	cEditBalloon.pszText = cMessage;

	LoadString(phLanguage, piTitle, cTitle, 255);
	cEditBalloon.pszTitle = cTitle;

	//show the balloon tip
	Edit_ShowBalloonTip(phControl, (LPARAM)&cEditBalloon);
	
	//do a message beep
	MessageBeep(0);
	}

