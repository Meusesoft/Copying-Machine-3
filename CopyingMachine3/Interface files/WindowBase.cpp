#include "StdAfx.h"
#include "WindowBase.h"

CWindowBase::CWindowBase(sGlobalInstances pInstances, std::wstring psWindowName)
{
	hWnd = NULL;
	hInstance = NULL;
	sWindowName = psWindowName;

	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &pInstances, sizeof(sGlobalInstances));
	oTrace = pInstances.oTrace;
}

CWindowBase::~CWindowBase(void)
{
	Destroy();
}

void CWindowBase::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	if (hWnd!=NULL) DestroyWindow(hWnd);

	//loop through the child windows
	//int iIndex = oChildWindows.size();

	//while (iIndex>0) {
	//		
	//	iIndex--;
	//	delete oChildWindows[iIndex];
	//	}

	//oChildWindows.clear();
	
	oTrace->EndTrace(__WFUNCTION__);
}


//This is the message handler for this class
bool CWindowBase::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	bool bReturn;
	int iIndex;

	bReturn = false;

	switch (message) {

		case WM_COMMAND:

			if (hWnd == phWnd) {

				//Process the command of the user
				bReturn = OnCommand(LOWORD(wParam));
				break;
				}

		case WM_SIZE:

			if (hWnd == phWnd) {

				//Process the command of the user
				bReturn = OnSize();
				break;
				}
			}

	//loop through the child windows
	iIndex = oChildWindows.size();

	while (iIndex>0 && !bReturn) {
			
		iIndex--;

			bReturn = oChildWindows[iIndex]->WndProc(phWnd, message, 
												 wParam, lParam, 
												 piReturn);
		}

	return bReturn;
	}

//This function processes the WM_COMMAND message
bool 
CWindowBase::OnCommand(int piCommand) {


	return true;
	}

//This function processes the WM_SIZE message
bool 
CWindowBase::OnSize() {


	return false;
	}

//this function layouts the child windows after an event which
//causes the window or one of its child to change its size.
void 
CWindowBase::LayoutChildWindow() {

//	RECT cClientRect;

	oTrace->StartTrace(__WFUNCTION__, eAll);


	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function adds a child window to the internal vector
void 
CWindowBase::AddChildWindow(CWindowBase* poChild) {

	oChildWindows.push_back(poChild);
	}
	
//This function searches for the child window and
//removes it from the vector
bool 
CWindowBase::RemoveChildWindow(CWindowBase* poChild) {

	int iIndex;
	bool bFound;

	iIndex = oChildWindows.size();
	bFound = false;

	while (iIndex>0 && !bFound) {

		iIndex--;
		
		if (oChildWindows[iIndex] == poChild) {

			bFound = true;
			oChildWindows.erase(oChildWindows.begin() + iIndex);
			}
		}

	return bFound;
	}

//This function return the number of child windows
int  
CWindowBase::ChildWindowCount() {

	return (int)oChildWindows.size();
	}
	
//This function returns the requested child
CWindowBase* 
CWindowBase::GetChildWindow(int piIndex) {

	CWindowBase* oResult;

	oResult = NULL;

	if (piIndex>=0 && piIndex<(int)oChildWindows.size()) {

		oResult = oChildWindows[piIndex];
		}

	return oResult;
	}	

void
CWindowBase::SaveSettings() {

	//Save the settings of all the child windows
	int iIndex;
	CWindowBase* oChild;

	iIndex = ChildWindowCount();

	while (iIndex>0) {
	
		iIndex--;

		oChild = GetChildWindow(iIndex);

		if (oChild) oChild->SaveSettings();
		}
	}

void
CWindowBase::LoadSettings() {

	//Load the settings of all the child windows
	int iIndex;
	CWindowBase* oChild;

	iIndex = ChildWindowCount();

	while (iIndex>0) {
	
		iIndex--;

		oChild = GetChildWindow(iIndex);

		if (oChild) oChild->LoadSettings();
		}
	}


