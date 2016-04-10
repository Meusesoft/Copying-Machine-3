#include "StdAfx.h"
#include "DlgColor.h"
#include "CopyingMachineCore.h"

CDlgColor::CDlgColor()
{
}

CDlgColor::~CDlgColor()
{
}

bool 
CDlgColor::ExecuteColorDialog(sGlobalInstances pcInstances, HWND hParent) {

	bool				bResult;
	CRegistry*			oRegistry;
	CHOOSECOLOR			cChooseColor;
	static COLORREF		acrCustClr[16]; // array of custom colors 
	DWORD				rgbCurrent;        // initial color selection
	CCopyingMachineCore* oCore;

	oRegistry = pcInstances.oRegistry;
	bResult = false;
	rgbCurrent = (DWORD)oRegistry->ReadInt(L"General", L"Color", 0xFFFFFF);

	//Initialise custom colors
	acrCustClr[0] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color0", 0);
	acrCustClr[1] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color1", 0);
	acrCustClr[2] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color2", 0);
	acrCustClr[3] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color3", 0);
	acrCustClr[4] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color4", 0);
	acrCustClr[5] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color5", 0);
	acrCustClr[6] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color6", 0);
	acrCustClr[7] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color7", 0);
	acrCustClr[8] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color8", 0);
	acrCustClr[9] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color9", 0);
	acrCustClr[10] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color10", 0);
	acrCustClr[11] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color11", 0);
	acrCustClr[12] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color12", 0);
	acrCustClr[13] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color13", 0);
	acrCustClr[14] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color14", 0);
	acrCustClr[15] = (COLORREF)oRegistry->ReadInt(L"Color", L"Color15", 0);

	//Initialise the ChooseColor structure
	ZeroMemory(&cChooseColor, sizeof(cChooseColor));
	cChooseColor.lStructSize = sizeof(cChooseColor);
	cChooseColor.hwndOwner = hParent;
	cChooseColor.lpCustColors = (LPDWORD) acrCustClr;
	cChooseColor.rgbResult = rgbCurrent;
	cChooseColor.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ENABLEHOOK;
	cChooseColor.lpfnHook = (LPCCHOOKPROC)HookProc;

	if (ChooseColor(&cChooseColor)) {

		oRegistry->WriteInt(L"General", L"Color", (int)cChooseColor.rgbResult);
		
		oRegistry->WriteInt(L"Color", L"Color0", (int)acrCustClr[0]);
		oRegistry->WriteInt(L"Color", L"Color1", (int)acrCustClr[1]);
		oRegistry->WriteInt(L"Color", L"Color2", (int)acrCustClr[2]);
		oRegistry->WriteInt(L"Color", L"Color3", (int)acrCustClr[3]);
		oRegistry->WriteInt(L"Color", L"Color4", (int)acrCustClr[4]);
		oRegistry->WriteInt(L"Color", L"Color5", (int)acrCustClr[5]);
		oRegistry->WriteInt(L"Color", L"Color6", (int)acrCustClr[6]);
		oRegistry->WriteInt(L"Color", L"Color7", (int)acrCustClr[7]);
		oRegistry->WriteInt(L"Color", L"Color8", (int)acrCustClr[8]);
		oRegistry->WriteInt(L"Color", L"Color9", (int)acrCustClr[9]);
		oRegistry->WriteInt(L"Color", L"Color10", (int)acrCustClr[10]);
		oRegistry->WriteInt(L"Color", L"Color11", (int)acrCustClr[11]);
		oRegistry->WriteInt(L"Color", L"Color12", (int)acrCustClr[12]);
		oRegistry->WriteInt(L"Color", L"Color13", (int)acrCustClr[13]);
		oRegistry->WriteInt(L"Color", L"Color14", (int)acrCustClr[14]);
		oRegistry->WriteInt(L"Color", L"Color15", (int)acrCustClr[15]);

		//Send a notification about the change
		oCore = (CCopyingMachineCore*)pcInstances.oCopyingMachineCore;
		oCore->oNotifications->CommunicateEvent(eNotificationColorChanged, (void*)cChooseColor.rgbResult);
		
		bResult = true;
		}

	return bResult;
	}

UINT
CALLBACK CDlgColor::HookProc(HWND hDlg, UINT msg, WPARAM, LPARAM param2) {

    HWND hParent;
    RECT tempRect, dlgRect;
    POINT newOrigin;
    UINT iResult;

	iResult = 0;

    if(msg == WM_INITDIALOG) {

        //center the dialog in the screen
        GetWindowRect(hDlg, &dlgRect);

		hParent = GetParent(hDlg);
        GetWindowRect(hParent, &tempRect);

        newOrigin.x = tempRect.left + ((tempRect.right - tempRect.left) - (dlgRect.right - dlgRect.left))/ 2;
        newOrigin.y = tempRect.top + ((tempRect.bottom - tempRect.top) - (dlgRect.bottom - dlgRect.top))/ 2;

        MoveWindow(hDlg, newOrigin.x, newOrigin.y, (dlgRect.right - dlgRect.left),
                                (dlgRect.bottom - dlgRect.top),true);

        iResult = 1;
        }

    return iResult;
}
