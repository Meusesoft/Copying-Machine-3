#include "StdAfx.h"
#include "Splitter.h"

#define SplitterThreshold 5

CSplitter::CSplitter(sGlobalInstances pInstances, std::wstring psWindowName) : 
	CWindowBase(pInstances, psWindowName)
{
	hWnd = NULL;
	bInCapture = false;
	cSplitterMode = eSplitterUserSize;
	iMinimalSize = SplitterThreshold;
	iDesiredSize = 0;

	cSplitterType = eSplitterDirect;
}

CSplitter::~CSplitter(void)
{
	Destroy();
}

//Create the window for the splitter
HWND CSplitter::Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra, 
					   eSplitterHV pcSplitterHV, HWND phSizingSibling) {

	oTrace->StartTrace(__WFUNCTION__);

	hInstance = phInstance;
	hSibling  = phSizingSibling;
	cSplitterHV = pcSplitterHV;

	DWORD dwStyle = WS_CHILD |
					WS_VISIBLE |
					WS_CLIPCHILDREN | 
					WS_CLIPSIBLINGS |
					pdStyleExtra;
            
	hWnd = CreateWindowEx(0,						// ex style
						 pcSplitterHV==eSplitterH ? L"MeusesoftSplitterH" : L"MeusesoftSplitterV",      // class name
						 pcSplitterHV==eSplitterH ? L"MeusesoftSplitterH" : L"MeusesoftSplitterV",      // window text
						 dwStyle,                   // style
						 0,                         // x position
						 30,                        // y position
						 300,                       // width
						 300,                       // height
						 phParent,					// parent
						 (HMENU)1009,				// ID
						 hInstance,                 // instance
						 NULL);                     // no extra data

	iSize = 200;
	iDefiniteSize = 200;
	
	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
	}


//this function is the message handler for the splitter
bool CSplitter::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	bool bReturn;
	RECT cDrawSplitterRect;

	if (hWnd == NULL) return false;

	oTrace->StartTrace(__WFUNCTION__, eExtreme);

	bReturn = false;
	
	if (hWnd == phWnd) {

		switch (message) {

			case WM_ERASEBKGND:

				RECT cClientRect;

				GetClientRect(hWnd, &cClientRect);
				
				HBRUSH hBackgroundBrush;

				hBackgroundBrush = defUseCustomColors ?
					CreateSolidBrush(defColorSplitter) :
					CreateSolidBrush(COLOR_BTNFACE);
					
				FillRect((HDC)wParam, &cClientRect, hBackgroundBrush); 
				DeleteObject(hBackgroundBrush);

				piReturn = 1; // we erase it
				bReturn = true;
				break;

			case WM_LBUTTONDBLCLK:

				//Toggle between visible and hidden
				ToggleMode();
				
				//send a WM_SIZE to the mainframe. The mainframe will layout all the child
				//windows
				PostMessage(GetParent(hWnd), WM_SIZE, 0, 0);					

				//we processed this message
				piReturn = 0;
				bReturn = true;
				break;

			case WM_LBUTTONDOWN:
		
				if (!bInCapture) {

					bInCapture = true;
					hPreviousCapture = SetCapture(hWnd);

					iSize = iDefiniteSize;
					iStartSize = iSize;

					if (GetMode() == eSplitterHidden) {

						iSize = iStartSize = 0;
						}

					if (cSplitterType == eSplitterPreview) {
						GetClientRect(hWnd, &cDrawSplitterRect);
						DrawSplitterPreview(cDrawSplitterRect);
						}

					bReturn = true;
					}

				break;

			case WM_LBUTTONUP:

				if (bInCapture) {

					bInCapture = false;
					ReleaseCapture();

					if (GetMode() != eSplitterHidden) iDefiniteSize = iSize;

					if (cSplitterType == eSplitterPreview) {

						PostMessage(GetParent(hWnd), WM_SIZE, 0, 0);

						GetClientRect(hWnd, &cDrawSplitterRect);
						DrawSplitterPreview(cDrawSplitterRect);
						}

					bReturn = true;
					}

				break;

			case WM_MOUSEMOVE:

				if (bInCapture) {
		
					RECT cMainFrameRect;
					RECT cSiblingRect;
					RECT cSplitterRect;
					RECT cDrawSplitterRect;
					POINT cCursorPosition;
					POINT cSiblingPosition;

					//get cursor position
					GetCursorPos(&cCursorPosition);
					ScreenToClient(GetParent(hWnd), &cCursorPosition);

					//get client rect of splitter, sibling and mainframe
					GetClientRect(GetParent(hWnd), &cMainFrameRect);
					GetClientRect(hWnd, &cSplitterRect);
					//GetClientRect(hSibling, &cSiblingRect);
					GetWindowRect(hSibling, &cSiblingRect);
					cSiblingPosition.x = cSiblingRect.left;
					cSiblingPosition.y = cSiblingRect.top;
					ScreenToClient(GetParent(hWnd), &cSiblingPosition);

					switch (cSplitterHV) {

						case eSplitterVRight:

							//do some position checking
							if (cCursorPosition.x<cSiblingPosition.x) cCursorPosition.x=cSiblingPosition.x;
							if (cCursorPosition.x>cMainFrameRect.right - cSplitterRect.right) cCursorPosition.x = cMainFrameRect.right - cSplitterRect.right;

							if (cSplitterType == eSplitterPreview) {
								
								GetClientRect(hWnd, &cDrawSplitterRect);
								DrawSplitterPreview(cDrawSplitterRect);
								}

							//set the new size
							if (GetMode() != eSplitterHidden) {
								
								iSize = cCursorPosition.x - cSiblingPosition.x;

								if (iSize<iMinimalSize) {

									SetMode(eSplitterHidden);
									iSize = 0;
									}
								}
							else {

								if (cCursorPosition.x - cSiblingPosition.x > iMinimalSize) {

									iSize = cCursorPosition.x - cSiblingPosition.x;
									SetMode(eSplitterUserSize);
									}
								else {
									iSize = 0;
									}
								}
							break;

						case eSplitterVLeft:

							cSiblingPosition.x = cSiblingRect.right;
							cSiblingPosition.y = cSiblingRect.top;
							ScreenToClient(GetParent(hWnd), &cSiblingPosition);

							//do some position checking
							if (cCursorPosition.x>cSiblingRect.right) cCursorPosition.x=cSiblingRect.right;
							if (cSiblingPosition.x - cCursorPosition.x > cMainFrameRect.right) cCursorPosition.x = cSiblingPosition.x - cMainFrameRect.right;

							if (cSplitterType == eSplitterPreview) {
								
								GetClientRect(hWnd, &cDrawSplitterRect);
								DrawSplitterPreview(cDrawSplitterRect);
								}

							//set the new size
							if (GetMode() != eSplitterHidden) {
								
								iSize = cSiblingPosition.x - cCursorPosition.x;

								if (iSize<iMinimalSize) {

									SetMode(eSplitterHidden);
									iSize = 0;
									}
								}
							else {

								if (cSiblingPosition.x - cCursorPosition.x > iMinimalSize) {

									iSize = cSiblingPosition.x - cCursorPosition.x;
									SetMode(eSplitterUserSize);
									}
								else {
									iSize = 0;
									}
								}

							break;

						case eSplitterH:

							//do some position checking
							if (cCursorPosition.y<cSiblingPosition.y) cCursorPosition.y=cSiblingPosition.y;
							if (cCursorPosition.y>cMainFrameRect.bottom - cSplitterRect.bottom) cCursorPosition.y = cMainFrameRect.bottom - cSplitterRect.bottom;

							if (cSplitterType == eSplitterPreview) {
								
								GetClientRect(hWnd, &cDrawSplitterRect);
								DrawSplitterPreview(cDrawSplitterRect);
								}

							//set the new size
							if (GetMode() != eSplitterHidden) {
								
								iSize = cCursorPosition.y - cSiblingPosition.y;

								if (iSize<iMinimalSize) {

									SetMode(eSplitterHidden);
									iSize = 0;
									}
								}
							else {

								if (cCursorPosition.y  - cSiblingPosition.y > iMinimalSize) {

									iSize = cCursorPosition.y - cSiblingPosition.y;
									SetMode(eSplitterUserSize);
									}
								else {
									iSize = 0;
									}
								}

							break;
						}

						if (iSize < iDesiredSize+25 && iSize > iDesiredSize-25 && iDesiredSize>0) { 
							
							SetMode(eSplitterDesiredSize);
							}
						else {

							SetMode(eSplitterUserSize);
							}
					

					//send a WM_SIZE to the mainframe. The mainframe will layout all the child
					//windows
					if (cSplitterType==eSplitterDirect) PostMessage(GetParent(hWnd), WM_SIZE, 0, 0);					

					if (cSplitterType == eSplitterPreview) {
						
						DrawSplitterPreview(cDrawSplitterRect);
						}

					//return true, we have processed this message
					bReturn = true;
					}

				break;
			}
		}

	//Process the base wndproc 
	if (!bReturn) {

		bReturn = CWindowBase::WndProc(phWnd, message, wParam, lParam, piReturn);
		}
		
	oTrace->EndTrace(__WFUNCTION__, eExtreme);

	return bReturn;
	}

void CSplitter::SetSiblingSize(int piSize) {

	iDefiniteSize = piSize;
	iSize = piSize;
	}

int	CSplitter::GetSiblingSize() {

	int iReturn;

	iReturn = iDefiniteSize;

	if (GetMode()==eSplitterHidden) iReturn = 0;
	if (bInCapture) iReturn = iSize;
	if (GetMode()==eSplitterDesiredSize) iReturn = iDesiredSize;

	return iReturn;
	}

void CSplitter::SetMode(eSplitterMode pcMode) {

	cSplitterMode = pcMode;
	}

void CSplitter::ToggleMode() {

	switch (cSplitterMode) {

		case eSplitterUserSize:

			cSplitterMode = ((iDesiredSize!=0 && iDesiredSize!=iSize) ? eSplitterDesiredSize : eSplitterHidden); 
			break;

		case eSplitterDesiredSize:

			cSplitterMode = eSplitterHidden; 
			break;

		case eSplitterHidden:

			cSplitterMode = eSplitterUserSize;
			break;
		}
	}

eSplitterMode 
CSplitter::GetMode() {

	return cSplitterMode;
}

//This function sets the minimal size of the sibling window. If it gets
//smaller it will be hidden
void 
CSplitter::SetMinimalSiblingSize(int piSize) {

	iMinimalSize = piSize;
	}

//This function sets the desired size of the sibling window. If it
//gets in the range of this desired size the splitter will get sticky
//to that size
void
CSplitter::SetDesiredSiblingSize(int piSize) {

	iDesiredSize = piSize;	
	}

void CSplitter::DrawSplitterPreview(RECT &pcRect) {

	HDC hClientDC;
	int iPreviousRop;
	RECT cDrawRect;
	POINT cPoint;

	oTrace->StartTrace(__WFUNCTION__, eExtreme);

	//Convert the rectangle to screen coordinates
	CopyMemory(&cDrawRect, &pcRect, sizeof(RECT));
	cPoint.x = 0;
	cPoint.y = 0;
	ClientToScreen(hWnd, &cPoint);

	switch (cSplitterHV) {

		case eSplitterVLeft:
		case eSplitterVRight:
	
			//Position the splitter rectangle
			cDrawRect.left += cPoint.x - iStartSize + iSize;
			cDrawRect.right += cPoint.x - iStartSize + iSize;
			cDrawRect.top += cPoint.y;
			cDrawRect.bottom += cPoint.y;

			break;

		case eSplitterH:
	
			//Position the splitter rectangle
			cDrawRect.top += cPoint.y - iStartSize + iSize;
			cDrawRect.bottom += cPoint.y - iStartSize + iSize;
			cDrawRect.left += cPoint.x;
			cDrawRect.right += cPoint.x;

			break;
		}

	//Draw the inverted rectangle
	hClientDC = NULL;
	hClientDC = GetDC(NULL);
	iPreviousRop = SetROP2(hClientDC, R2_XORPEN);
	Rectangle(hClientDC, cDrawRect.left, cDrawRect.top, cDrawRect.right, cDrawRect.bottom);
	SetROP2(hClientDC, iPreviousRop);
	ReleaseDC(NULL, hClientDC);

	oTrace->EndTrace(__WFUNCTION__, eExtreme);
	}

//This functions saves the settings to the registry
void
CSplitter::SaveSettings() {

	std::wstring sKey;
	CRegistry* oRegistry;

	oRegistry = oGlobalInstances.oRegistry;

	sKey = sWindowName;
	sKey += L"Size";
	oRegistry->WriteInt(L"Window", sKey, iSize);
	sKey = sWindowName;
	sKey += L"Mode";	
	oRegistry->WriteInt(L"Window", sKey, (int)GetMode());

	CWindowBase::SaveSettings();
	}

//This functions loads the settings to the registry
void
CSplitter::LoadSettings() {

	std::wstring sKey;
	CRegistry* oRegistry;

	oRegistry = oGlobalInstances.oRegistry;
	sKey = sWindowName;
	sKey += L"Mode";
	SetMode((eSplitterMode)oRegistry->ReadInt(L"Window", sKey, (int)eSplitterUserSize));
	sKey = sWindowName;
	sKey += L"Size";	
	SetSiblingSize(oRegistry->ReadInt(L"Window", sKey, iDesiredSize==0 ? 200 : iDesiredSize));
	
	CWindowBase::LoadSettings();
	}


