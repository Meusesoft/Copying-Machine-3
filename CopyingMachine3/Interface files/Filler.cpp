#include "StdAfx.h"
#include "Filler.h"

CFiller::CFiller(CTracer* poTrace)
{
	hWnd = NULL;
	oTrace = poTrace;

}

CFiller::~CFiller(void)
{
	Destroy();
}

//Create the window for the splitter
HWND 
CFiller::Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra) {

	oTrace->StartTrace(__WFUNCTION__);

	hInstance = phInstance;

	DWORD dwStyle = WS_CHILD |
					WS_VISIBLE |
					WS_CLIPCHILDREN | 
					WS_CLIPSIBLINGS |
					pdStyleExtra;
            
	hWnd = CreateWindowEx(0,						// ex style
						 L"MeusesoftSplitterH",     // class name
						 L"Filler",					// window text
						 dwStyle,                   // style
						 0,                         // x position
						 30,                        // y position
						 300,                       // width
						 300,                       // height
						 phParent,					// parent
						 (HMENU)1009,				// ID
						 hInstance,                 // instance
						 NULL);                     // no extra data

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
	}

//Destroy the window of the splitter
void 
CFiller::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	if (hWnd!=NULL) DestroyWindow(hWnd);

	oTrace->EndTrace(__WFUNCTION__);
	}

//this function is the message handler for the splitter
bool 
CFiller::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	bool bReturn;

	if (hWnd == NULL) return false;

	oTrace->StartTrace(__WFUNCTION__, eExtreme);

	bReturn = false;
	
	if (hWnd == phWnd) {

		switch (message) {

			case WM_ERASEBKGND:

				RECT cClientRect;
				//HBRUSH hOldBrush;

				GetClientRect(hWnd, &cClientRect); 
				//hOldBrush = (HBRUSH)SelectObject((HDC)wParam, CreateSolidBrush(RGB(defBackgroundR, defBackgroundG, defBackgroundB))); 
				
				HBRUSH hBackgroundBrush;

				if (bUseCustomControlView) {
					hBackgroundBrush = CreateSolidBrush(RGB(defBackgroundR * 0.75, defBackgroundG * 0.75, defBackgroundB * 0.75));
					}
				else {
					hBackgroundBrush = CreateSolidBrush(RGB(192,192,192));
					}
					
				FillRect((HDC)wParam, &cClientRect, hBackgroundBrush); 
				DeleteObject(hBackgroundBrush);
				//DeleteObject(SelectObject((HDC)wParam, hOldBrush)); 

				piReturn = 1; // we erase it
				bReturn = true;
				break;
		}
	}

	oTrace->EndTrace(__WFUNCTION__, eExtreme);

	return bReturn;
	}


