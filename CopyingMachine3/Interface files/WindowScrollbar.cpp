#include "StdAfx.h"
#include "WindowScrollbar.h"

CWindowScrollbar::CWindowScrollbar(sGlobalInstances pInstances, std::wstring psWindowName) : 
	CWindowBase(pInstances, psWindowName)
{

	iContentHeight = 0;
	iContentWidth = 0;
}

CWindowScrollbar::~CWindowScrollbar(void)
{

}

void
CWindowScrollbar::InitializeScrollInfo() {

	//Initialise the scrollinfo structures
	SecureZeroMemory(&cSBIHorizontal, sizeof(SCROLLINFO));
	cSBIHorizontal.cbSize = sizeof(SCROLLINFO);

	SecureZeroMemory(&cSBIVertical, sizeof(SCROLLINFO));
	cSBIVertical.cbSize = sizeof(SCROLLINFO);
	}	

//This function updates the scrollbar. Variable pdType is either
//SB_VERT or SB_HORZ
void
CWindowScrollbar::UpdateScrollbar(DWORD pdType, SCROLLINFO &pcNewScrollInfo) {

	bool bVisible;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bVisible = (pcNewScrollInfo.nMax != pcNewScrollInfo.nPage);

	ShowScrollBar(hWnd, pdType, bVisible);
	
	if (bVisible) {

		//ZeroMemory(&pcNewScrollInfo, sizeof(SCROLLINFO));
		pcNewScrollInfo.cbSize = sizeof(SCROLLINFO);
		pcNewScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		SetScrollInfo(hWnd, pdType, &pcNewScrollInfo, TRUE);

		if (pdType==SB_VERT) {

			memcpy((VOID*)&cSBIVertical, (VOID*)&pcNewScrollInfo, sizeof(SCROLLINFO));
			}
		else {

			memcpy((VOID*)&cSBIHorizontal, (VOID*)&pcNewScrollInfo, sizeof(SCROLLINFO));
			}
		}
	else {

		if (pdType==SB_VERT) {

			cSBIVertical.nMax = pcNewScrollInfo.nMax;
			cSBIVertical.nPage = pcNewScrollInfo.nPage;
			}
		else {

			cSBIHorizontal.nMax = pcNewScrollInfo.nMax;
			cSBIHorizontal.nPage = pcNewScrollInfo.nPage;
			}
	}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function process the scrollbar movements
void 
CWindowScrollbar::OnEventScrollbar(UINT message, WPARAM wParam, LPARAM lParam) {

	bool bUpdate;
	bool bVisible;
	int iPosition;
	int iRequest;
	int iScrollbar;
	SCROLLINFO cNewScrollInfo;

	oTrace->StartTrace(__WFUNCTION__, eAll);


	bUpdate = false;
	iPosition = HIWORD(wParam);
	iRequest = LOWORD(wParam);
	iScrollbar = (message==WM_VSCROLL ? SB_VERT : SB_HORZ);

	if (iScrollbar==SB_VERT) {

		memcpy((VOID*)&cNewScrollInfo, (VOID*)&cSBIVertical, sizeof(SCROLLINFO));
		}
	else {

		memcpy((VOID*)&cNewScrollInfo, (VOID*)&cSBIHorizontal, sizeof(SCROLLINFO));
		}

	bVisible = (cNewScrollInfo.nMax != cNewScrollInfo.nPage);

	if (bVisible) {

		if (iPosition<0) iPosition = 0;
		if (iPosition>(int)(cNewScrollInfo.nMax - cNewScrollInfo.nPage)) iPosition = cNewScrollInfo.nMax - cNewScrollInfo.nPage;

		switch (iRequest) {

			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:

				cNewScrollInfo.nPos = iPosition;

				bUpdate = true;

				break;

			case SB_LINEUP:

				cNewScrollInfo.nPos--;
				if (cNewScrollInfo.nPos < 0) {
					cNewScrollInfo.nPos = 0;
					}
				bUpdate = true;
				break;

			case SB_LINEDOWN:

				cNewScrollInfo.nPos++;
				if (cNewScrollInfo.nPos + (int)cNewScrollInfo.nPage > cNewScrollInfo.nMax) {
					cNewScrollInfo.nPos = cNewScrollInfo.nMax - cNewScrollInfo.nPage;
					}
				bUpdate = true;
				break;

			case SB_PAGEUP:

				cNewScrollInfo.nPos -= cNewScrollInfo.nPage;
				if (cNewScrollInfo.nPos < 0) {
					cNewScrollInfo.nPos = 0;
					}
				bUpdate = true;
				break;

			case SB_PAGEDOWN:

				cNewScrollInfo.nPos += cNewScrollInfo.nPage;
				if (cNewScrollInfo.nPos + (int)cNewScrollInfo.nPage > cNewScrollInfo.nMax) {
					cNewScrollInfo.nPos = cNewScrollInfo.nMax - cNewScrollInfo.nPage;
					}
				bUpdate = true;
				break;

			case SB_TOP:

				cNewScrollInfo.nPos=0;
				bUpdate = true;
				break;

			case SB_BOTTOM:

				cNewScrollInfo.nPos = cNewScrollInfo.nMax - cNewScrollInfo.nPage;
				bUpdate = true;
				break;
			}

		//a change has occurred, update the scrollbars and redraw
		if (bUpdate) {

			int iDeltaX = 0;
			int iDeltaY = 0;
			int iFlags;

			iFlags = /*HIWORD(30) | SW_SMOOTHSCROLL |*/ SW_INVALIDATE;

			SetScrollInfo(hWnd, iScrollbar, &cNewScrollInfo, TRUE);

			if (iScrollbar==SB_VERT) {

				iDeltaY = cSBIVertical.nPos - cNewScrollInfo.nPos;

				//oSettings->fPosY = (float)cNewScrollInfo.nPos;
				memcpy((VOID*)&cSBIVertical, (VOID*)&cNewScrollInfo, sizeof(SCROLLINFO));
				
				}
			else {

				iDeltaX = cSBIHorizontal.nPos - cNewScrollInfo.nPos;

				//oSettings->fPosX = (float)cNewScrollInfo.nPos;
				memcpy((VOID*)&cSBIHorizontal, (VOID*)&cNewScrollInfo, sizeof(SCROLLINFO));
				}
			
			//ScrollWindowEx(hWnd, iDeltaX, iDeltaY, NULL, NULL, NULL, NULL, iFlags | SW_SCROLLCHILDREN);
			//DoRepositionControls();
			//Redraw();
			}
		}
	
	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//this function is the message handler for the pageviewer
bool CWindowScrollbar::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bool bReturn;

	bReturn = false;

	switch (message) {


		case WM_VSCROLL:
		case WM_HSCROLL:

			if (phWnd == this->hWnd) {
				OnEventScrollbar(message, wParam, lParam);
				bReturn = true;
				piReturn = 0;
				}
			break;
		}

	if (!bReturn) bReturn = CWindowBase::WndProc(phWnd, message, wParam, lParam, piReturn);

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return bReturn;
	}

bool 
CWindowScrollbar::OnSize() {

	DoUpdateScrollbar(SB_VERT);
	DoUpdateScrollbar(SB_HORZ);

	return CWindowBase::OnSize();
}


//This function is called when the scrollbar has been moved to reposition
//all the controls on the window
void 
CWindowScrollbar::DoRepositionControls() {

	



}

//This function sets the size of the content and updates the
//scrollbars accordingly
void 
CWindowScrollbar::SetContentSize(int piWidth, int piHeight) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	if (iContentWidth!=piWidth || iContentHeight!=piHeight) {
		iContentWidth  = piWidth;
		iContentHeight = piHeight;

		DoUpdateScrollbar(SB_VERT);
		DoUpdateScrollbar(SB_HORZ);
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function checks if the content is larger then the client window, and if
//so it will hide/show the scrollbars
void 
CWindowScrollbar::DoUpdateScrollbar(DWORD pdType) {

	int iContentSize;
	int iWindowSize;
	RECT cClientRect;
	bool bScrollbar;
	SCROLLINFO cNewScrollInfo;

	GetClientRect(hWnd, &cClientRect);

	iContentSize = (pdType==SB_VERT ? iContentHeight : iContentWidth);
	iWindowSize =  (pdType==SB_VERT ? cClientRect.bottom : cClientRect.right);

	bScrollbar = (iWindowSize < iContentSize);

	//Show or hide the scrollbar
	if (bScrollbar) {

		InvalidateRect(hWnd, NULL, TRUE);

		//Process vertical scrollbar
		cNewScrollInfo.nPage = iWindowSize;
		cNewScrollInfo.nMax = iWindowSize;

		if (iContentSize > iWindowSize) {

			cNewScrollInfo.nMin = 0;
			cNewScrollInfo.nMax = iContentSize-1;
			cNewScrollInfo.nPage = iWindowSize;
			cNewScrollInfo.nPos = max(0, (pdType==SB_VERT ? cSBIVertical.nPos : cSBIHorizontal.nPos));

			if (cNewScrollInfo.nPos + (int)cNewScrollInfo.nPage > cNewScrollInfo.nMax) {
				cNewScrollInfo.nPos = cNewScrollInfo.nMax - cNewScrollInfo.nPage;
				//iScrollPosition = (int)cNewScrollInfo.nPos;
				//Redraw();
				}
			}
		
		cNewScrollInfo.cbSize = sizeof(SCROLLINFO);
		cNewScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		UpdateScrollbar(pdType, cNewScrollInfo);
		}
	else {

		//Hide the scrollbar
		if (pdType == SB_VERT) {
			SecureZeroMemory(&cSBIVertical, sizeof(SCROLLINFO));
			cSBIVertical.cbSize = sizeof(SCROLLINFO);
			}
		else {
			SecureZeroMemory(&cSBIHorizontal, sizeof(SCROLLINFO));
			cSBIHorizontal.cbSize = sizeof(SCROLLINFO);
			}

		ShowScrollBar(hWnd, pdType, false);
		InvalidateRect(hWnd, NULL, TRUE);
		}
	}

//Returns the position of the scrollbars horizontal thumb
int
CWindowScrollbar::GetHorizontalPosition() {

	return (int)cSBIHorizontal.nPos;
	}

//Returns the position of the scrollbars horizontal thumb
int
CWindowScrollbar::GetVerticalPosition() {

	return (int)cSBIVertical.nPos;
	}

//Set the position of the scrollbars horizontal thumb
int
CWindowScrollbar::SetVerticalPosition(int piVertPos) {

	cSBIVertical.nPos = (int)piVertPos;
	DoUpdateScrollbar(SB_VERT);

	return GetVerticalPosition();
	}



