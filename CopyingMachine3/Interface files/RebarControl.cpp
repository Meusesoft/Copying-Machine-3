#include "StdAfx.h"
#include <atlbase.h>
#include "RebarControl.h"

CRebarControl::CRebarControl(sGlobalInstances pInstances)
{
	oTrace = pInstances.oTrace;
	oRegistry = pInstances.oRegistry;

	hWnd = NULL;
}

CRebarControl::~CRebarControl(void)
{
	Destroy();
}

//Create the coolbar
bool CRebarControl::Create(HINSTANCE phInstance, HWND phParent) {

	if (hWnd!=NULL) return true; //this bar is already created

	oTrace->StartTrace(__WFUNCTION__);
	
	//1. Initialise the common controls so we can use the coolbar
		INITCOMMONCONTROLSEX icex;

		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_COOL_CLASSES;   

		if (!::InitCommonControlsEx(&icex)) {
			
			MsgBox;
			};

	//2. Create the COOLBAR control.
		hWnd = CreateWindowEx( 
			0L,
			REBARCLASSNAME,
			NULL,
			WS_VISIBLE | /*WS_BORDER | */WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
			CCS_NOPARENTALIGN | CCS_TOP | CCS_NODIVIDER |
			RBS_VARHEIGHT | RBS_BANDBORDERS,
			0, 0, 400, 80,
			phParent,
			(HMENU)NULL,
			phInstance,
			NULL);

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
}

//This function adds a new child to the coolbar
bool CRebarControl::AddBand(HWND phChild, UINT pID) {

	REBARINFO rbi;
	REBARBANDINFO rbBand;
	bool bReturn;

	if (hWnd==NULL) return false;

	oTrace->StartTrace(__WFUNCTION__);

	// Initialize and send the REBARINFO structure.
		rbi.cbSize = sizeof(REBARINFO);  // Required when using this structure.
		rbi.fMask  = 0;
		rbi.himl   = 0;//(HIMAGELIST)hImagelist;
		if (!SendMessage(hWnd, RB_SETBARINFO, 0, (LPARAM)&rbi)) return false;

	// Initialize structure members that both bands will share.
		SecureZeroMemory(&rbBand, sizeof(REBARBANDINFO));
		rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
		//rbBand.cbSize = 80;  // Required
		rbBand.fMask  = /*RBBIM_TEXT | /*RBBIM_BACKGROUND | */
						RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | 
						RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_ID;
		rbBand.fStyle = RBBS_CHILDEDGE | /*RBBS_GRIPPERALWAYS |*/ RBBS_BREAK | RBBS_USECHEVRON; // | RBBS_FIXEDBMP;
		//rbBand.hbmBack = LoadBitmap(phInstance, MAKEINTRESOURCE(IDB_TOOLBAR));   
		//rbBand.lpText     = _T("Commands");
		
		SIZE sMaxSizeToolbar;
		SendMessage(phChild, TB_GETMAXSIZE, 0, (LPARAM)&sMaxSizeToolbar);
		
		rbBand.hwndChild  = phChild;
		rbBand.cxMinChild = sMaxSizeToolbar.cx;
		rbBand.cyMinChild = sMaxSizeToolbar.cy;
		rbBand.cx         = sMaxSizeToolbar.cx;
		rbBand.cyIntegral = 0;
		rbBand.cxIdeal	  = sMaxSizeToolbar.cx;
		rbBand.wID		  = pID;

	// Add the band that has the toolbar.

	bReturn = (SendMessage(hWnd, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand)!=0);

	oTrace->EndTrace(__WFUNCTION__, bReturn);

	return true;
	}

//Destroy the window of the coolbar
void CRebarControl::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	if (hWnd!=NULL) DestroyWindow(hWnd);

	oTrace->EndTrace(__WFUNCTION__);
	}

//---------------------------------------------------------------------------------

void CRebarControl::SaveRebarSettings() {

	//This function save the settings (coordinates, styles etc) of the tools in the rebar
	
	CBandInfo *oBandInfo;
	std::wstring sKey;
	std::wstring sEntry;

	oTrace->StartTrace(__WFUNCTION__);

	long lBandCount;

	//Retrieve the information from the ReBar control
	lBandCount = (long)SendMessage(hWnd, RB_GETBANDCOUNT, 0, 0);	

	if (lBandCount > 0)	{
		
		oBandInfo = new CBandInfo[lBandCount];

		for (long lIndex=0; lIndex<lBandCount; lIndex++) {

			REBARBANDINFO cRbbi;

			cRbbi.cbSize = sizeof(cRbbi);
			cRbbi.fMask = RBBIM_ID | RBBIM_SIZE | RBBIM_STYLE;

			SendMessage(hWnd, RB_GETBANDINFO, lIndex, (LPARAM)&cRbbi);

			oBandInfo[lIndex].lId = cRbbi.wID;
			oBandInfo[lIndex].lcx = cRbbi.cx;
			oBandInfo[lIndex].bGripper = (cRbbi.fStyle & RBBS_NOGRIPPER) != 0;
			oBandInfo[lIndex].bHidden = (cRbbi.fStyle & RBBS_HIDDEN) != 0;
			oBandInfo[lIndex].bBreakLine = (cRbbi.fStyle & RBBS_BREAK) != 0;
			}


		//Save the retrieved information in the registry
			sKey = _T("Toolbars");
			
			oRegistry->WriteInt(sKey, _T("Count"), lBandCount);

			for (long lIndex=0; lIndex<lBandCount; lIndex++) {

				//sEntry.Format("%s%i", "Band", lIndex);
				wchar_t sBandName[25];

				swprintf(sBandName, 25, _T("%s%i"), _T("Band"), lIndex);
				sEntry = sBandName;

				oRegistry->WriteInt(sKey, sEntry + _T("Id"), oBandInfo[lIndex].lId);
				oRegistry->WriteInt(sKey, sEntry + _T("X"), oBandInfo[lIndex].lcx);
				oRegistry->WriteInt(sKey, sEntry + _T("Gripper"), oBandInfo[lIndex].bGripper);
				oRegistry->WriteInt(sKey, sEntry + _T("Hidden"), oBandInfo[lIndex].bHidden);
				oRegistry->WriteInt(sKey, sEntry + _T("Break"), oBandInfo[lIndex].bBreakLine);
			}

			delete[] oBandInfo;
		}

	oTrace->EndTrace(__WFUNCTION__);
}

//---------------------------------------------------------------------------------

void CRebarControl::LoadRebarSettings() {

	//This function restores the settings (coordinates, styles etc) of the tools in the rebar
	
	CBandInfo *oBandInfo;
	std::wstring sKey;
	std::wstring sEntry;

	long lBandIndex;
	long lBandCount;
	REBARBANDINFO cRbbi;

	oTrace->StartTrace(__WFUNCTION__);

	//Open the registry key
	sKey = _T("Toolbars");

	lBandCount = oRegistry->ReadInt(sKey, _T("Count"), 0);
	
	if (lBandCount>0) {

		//Read the information about the bands from the registry
		
		oBandInfo = new CBandInfo[lBandCount];

		for (long lIndex=0; lIndex<lBandCount; lIndex++) {

			wchar_t sBandName[25];

			swprintf(sBandName, 25, L"%s%i", L"Band", lIndex);
			sEntry = sBandName;

			oBandInfo[lIndex].lId = oRegistry->ReadInt(sKey, sEntry + L"Id", 0);
			oBandInfo[lIndex].lcx = oRegistry->ReadInt(sKey, sEntry + L"X", 0);
			oBandInfo[lIndex].bGripper = oRegistry->ReadInt(sKey, sEntry + L"Gripper", 0);
			oBandInfo[lIndex].bHidden = oRegistry->ReadInt(sKey, sEntry + L"Hidden", 0);
			oBandInfo[lIndex].bBreakLine = oRegistry->ReadInt(sKey, sEntry + L"Break", 0);
		}		
	
		//Apply the information from the registry to the bands
		for (long lIndex=0; lIndex<lBandCount; lIndex++) {

			lBandIndex = SendMessage(hWnd, RB_IDTOINDEX, (WPARAM)oBandInfo[lIndex].lId, (LPARAM)0);
			
			if (lBandIndex!=-1) {
			
				SendMessage(hWnd, RB_MOVEBAND, (WPARAM)lBandIndex, (LPARAM)lIndex);

				cRbbi.cbSize = sizeof(cRbbi);
				cRbbi.fMask = RBBIM_ID | RBBIM_SIZE | RBBIM_STYLE;
				
				SendMessage(hWnd, RB_GETBANDINFO, (WPARAM)lIndex, (LPARAM)&cRbbi); 

				cRbbi.cx = oBandInfo[lIndex].lcx;
				
				cRbbi.fStyle &= ~RBBS_BREAK;
				if (oBandInfo[lIndex].bBreakLine) {
					cRbbi.fStyle |= RBBS_BREAK;
					}

				if (oBandInfo[lIndex].bGripper) {
					cRbbi.fStyle |= RBBS_NOGRIPPER;
					}
				else {
					cRbbi.fStyle &= ~RBBS_NOGRIPPER;
					}

				if (oBandInfo[lIndex].bHidden) {
					cRbbi.fStyle |= RBBS_HIDDEN;
					}
				else {
					cRbbi.fStyle &= ~RBBS_HIDDEN;
					}

				SendMessage(hWnd, RB_SETBANDINFO, (WPARAM)lIndex, (LPARAM)&cRbbi);
				SendMessage(hWnd, RB_SHOWBAND, (WPARAM) lIndex, (LPARAM)(BOOL)FALSE);
				SendMessage(hWnd, RB_SHOWBAND, (WPARAM) lIndex, (LPARAM)(BOOL)TRUE);
			}
		}

		delete[] oBandInfo;
	}

	oTrace->EndTrace(__WFUNCTION__);
}


