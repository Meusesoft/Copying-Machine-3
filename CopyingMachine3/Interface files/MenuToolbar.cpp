#include "stdafx.h"
#include "MenuToolbar.h"

//static members for hot-tracking messages
static CMenuToolbar* g_pMenuToolbar	= NULL;
static HHOOK g_hMsgHook	= NULL;

//creator and destructor
CMenuToolbar::CMenuToolbar(CTracer* poTrace): CToolbar(poTrace)
{
	hWnd = NULL;
	hMenu = NULL;
	hPreviousFocus = NULL;
	dwStyleExtra = TBSTYLE_LIST;
	lMenuOpen = -1;
	lButtonPressed = -1;
	bKeyboardAction = false;
}

CMenuToolbar::~CMenuToolbar(void)
{
	if (hWnd!=NULL) CToolbar::Destroy();
	if (hMenu!=NULL) DestroyMenu(hMenu);
}

//this function 'fills' the toolbar with menu items
void CMenuToolbar::Attach(HINSTANCE phResource, LPCTSTR lpMenuName) {

	oTrace->StartTrace(__FUNCTION__);

	TBBUTTON cButton[1];
	TCHAR sMenuText[MAX_PATH];
	long  lStringIndex;

	if (hWnd==NULL) {
		
		oTrace->EndTrace(__FUNCTION__, (bool)false);
		return;
		}

	//Load the menu
	hMenu = LoadMenu(phResource, lpMenuName);
	if (hMenu==NULL) {
		
		oTrace->EndTrace(__FUNCTION__, (bool)false);
		return;
		}

	//Determine the number of submenus
	lMenuCount = GetMenuItemCount(hMenu);

	for (long lIndex=0; lIndex<lMenuCount; lIndex++) {

		::ZeroMemory(sMenuText, sizeof(sMenuText));
		if (GetMenuString(hMenu, lIndex, sMenuText, MAX_PATH,MF_BYPOSITION)!=0) {
					
			lStringIndex = (long)SendMessage(hWnd, TB_ADDSTRING, NULL, (LPARAM)sMenuText);
			
			cButton[0].idCommand = lIndex;
			cButton[0].iBitmap = I_IMAGENONE;
			cButton[0].iString = lStringIndex;
			cButton[0].fsState = TBSTATE_ENABLED;
			cButton[0].fsStyle = BTNS_BUTTON |  BTNS_DROPDOWN | BTNS_AUTOSIZE;
			cButton[0].dwData = NULL;

			SendMessage(hWnd, TB_ADDBUTTONS, 1, (LPARAM)cButton);
			}
		}

	SendMessage(hWnd, TB_AUTOSIZE, 0, 0);

	lNumberButtons = (long)SendMessage(hWnd, TB_BUTTONCOUNT, 0, 0);

	oTrace->EndTrace(__FUNCTION__);
}

//This function processes all the messages, it returns true if it processed a message
bool CMenuToolbar::WndProc(HWND hWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {


	bool bReturn;
	LPNMHDR cNotificationHeader;
	LPNMTOOLBAR cNotificationToolbar;

	bReturn = false;

	switch (message) {

		case WM_NOTIFY:

			cNotificationHeader = (LPNMHDR)lParam;
			
			if (cNotificationHeader->code == TBN_DROPDOWN) {

				cNotificationToolbar = (LPNMTOOLBAR)lParam;

				PostMessage(hWnd, WM_OPENMENU, cNotificationToolbar->iItem, 0);

				bReturn = true;
				}
			break;
		
		case WM_OPENMENU:
			{
			OpenSubMenu((long)wParam);
			bReturn = true;
			break;
			}

		case WM_SYSCOMMAND:
			{
			switch (wParam) {

				case SC_KEYMENU:

					CloseMenuWithKey();
					bReturn = true;

					break;
					}
				break;
				}

		case WM_KEYDOWN:
			{
			switch (wParam)
				{
				case VK_LEFT:
				
					if (lMenuOpen==-1 && lButtonPressed!=-1) {

						PressButton(lButtonPressed, FALSE);
						
						lButtonPressed--;

						if (lButtonPressed<0) lButtonPressed = lMenuCount-1;

						PressButton(lButtonPressed, TRUE);

						bReturn = true;
						}

					break;
			
				case VK_RIGHT:
			
					if (lMenuOpen==-1 && lButtonPressed!=-1) {

						PressButton(lButtonPressed, FALSE);
						
						lButtonPressed++;

						if (lButtonPressed==lMenuCount) lButtonPressed = 0;

						PressButton(lButtonPressed, TRUE);

						bReturn = true;
						}

					break;
			
				case VK_DOWN:
			
					if (lMenuOpen==-1 && lButtonPressed!=-1) {

						PressButton(lButtonPressed, FALSE);
						bKeyboardAction = true;
						OpenSubMenu(lButtonPressed);

						bReturn = true;
						}
					break;

				case VK_ESCAPE:

					if (lMenuOpen==-1 && lButtonPressed!=-1) {
						
						CloseMenuWithKey();

						bReturn = true;
						}

					break;

				default:

					if (lMenuOpen==-1 && lButtonPressed!=-1) {
						
						long lMenuItem = OpenSubMenu((UINT)wParam);

						if (lMenuItem!=-1) PostMessage(hWnd, WM_OPENMENU, lMenuItem, 0);
						bReturn = (lMenuItem!=-1);
						}
					break;
					}	
				}
			}
			
	return bReturn;
	}

void CMenuToolbar::PressButton(long plIndex, bool pbPressed) {

	oTrace->StartTrace(__FUNCTION__, eAll);

	SendMessage(hWnd, TB_PRESSBUTTON, (WPARAM)plIndex, MAKELONG(pbPressed, 0));
		
	lButtonPressed = plIndex;
	//lButtonPressed = pbPressed ? plIndex : -1;

	oTrace->EndTrace(__FUNCTION__, eAll);
}

void CMenuToolbar::CloseMenuWithKey() {

	oTrace->StartTrace(__FUNCTION__, eAll);

	if (lMenuOpen==-1 && lButtonPressed!=-1) {

		//The toolbar has a selected menu. Unselect it.
		PressButton(lButtonPressed, FALSE);
		lButtonPressed=-1;
		if (hPreviousFocus!=NULL) {
			//SetFocus(hPreviousFocus);
			hPreviousFocus = NULL;
			}
		}
	else {

		//Set the focus to the toolbar and select the first
		//menu.
		//hPreviousFocus = SetFocus(hWnd);
		PressButton(0, TRUE);
		}	

	oTrace->EndTrace(__FUNCTION__, eAll);
}

//Open the submenu by the character
long CMenuToolbar::OpenSubMenu(UINT nChar) {

	TCHAR  cMenuItemText[MAX_PATH];
	TCHAR* cPointer;
	TCHAR  cCharacter;

	oTrace->StartTrace(__FUNCTION__, eAll);

	for (long lIndex=0; lIndex<lMenuCount; lIndex++) {

		GetMenuString(hMenu, lIndex, cMenuItemText, MAX_PATH, MF_BYPOSITION);
		
		cPointer = wcsstr(cMenuItemText, L"&");
		cPointer++;
		cCharacter = cPointer[0];

		if (cPointer>=0 && toupper(nChar)==towupper(cCharacter)) {

			// open matching menu
			PressButton(lButtonPressed, false);
			lButtonPressed = -1;
			bKeyboardAction = true;

			//PostMessage(hWnd, WM_OPENMENU, lIndex, 0);

			oTrace->EndTrace(__FUNCTION__, lIndex, eAll);
			return lIndex;
			}
		}

	oTrace->EndTrace(__FUNCTION__, -1, eAll);
	return -1;
}


//Open the submenu given by the index (if it isn't already open)
void CMenuToolbar::OpenSubMenu(long plIndex) {

	RECT cButtonPosition;
	POINT cMenuPosition;
	MENUITEMINFO cSubMenuInfo;

	oTrace->StartTrace(__FUNCTION__, eAll);

	lNextMenuOpen = plIndex;
	//if (hPreviousFocus==NULL) hPreviousFocus=SetFocus(hWnd);

	while (lNextMenuOpen!=-1) {
	
		lMenuOpen = lNextMenuOpen;
		lNextMenuOpen = -1;
		bMenuItemIsPopup = false;
		bInSubmenu = false;

		SendMessage(GetParent(hWnd), WM_MENUSELECT, MAKELONG(lMenuOpen, MF_POPUP | MF_HILITE | MF_MOUSESELECT), (LPARAM)hMenu);

		//Press the button
		if (lButtonPressed!=-1) PressButton(lButtonPressed, FALSE);
		PressButton(lMenuOpen, TRUE);
		
		//SendMessage(hWnd, TB_SETHOTITEM, (WPARAM)lMenuOpen, 0);

		//Get the position of the button
		SendMessage(hWnd, TB_GETITEMRECT, (WPARAM)lMenuOpen, (LPARAM)&cButtonPosition);

		//Get the submenu
		cSubMenuInfo.cbSize = sizeof(cSubMenuInfo);
		cSubMenuInfo.fMask = MIIM_SUBMENU;
		GetMenuItemInfo(hMenu, lMenuOpen, MF_BYPOSITION, &cSubMenuInfo);

		//Track the menu
		cMenuPosition.x = cButtonPosition.left;
		cMenuPosition.y = cButtonPosition.bottom;
		ClientToScreen(hWnd, &cMenuPosition);
		
		g_pMenuToolbar = this;
		g_hMsgHook = SetWindowsHookEx(WH_MSGFILTER, MenuMessageFilter, 0, GetCurrentThreadId());

		hSubMenu = cSubMenuInfo.hSubMenu;

		if (bKeyboardAction) {

			//if this menu was opened through the keyboard then
			//press the down key so the first item is displayed
			PostMessage(hWnd, WM_KEYDOWN, VK_DOWN, 0);
			bKeyboardAction = false;
			}

		TrackPopupMenu(hSubMenu, 
						TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION,
						cMenuPosition.x,
						cMenuPosition.y,
						0,
						hWnd,
						NULL);

		UnhookWindowsHookEx(g_hMsgHook);

		g_pMenuToolbar = NULL;
		g_hMsgHook = NULL;

		//Unpress the button. But do not if the menu is closed with ESC or F10.
		if (lNextMenuOpen!=-1 || (lNextMenuOpen==-1 && !bKeyboardAction)) {
			PressButton(lMenuOpen, FALSE);
			lButtonPressed=-1;
			}
		
		//SendMessage(hWnd, TB_SETHOTITEM, (WPARAM)lMenuOpen, -1);

		SendMessage(GetParent(hWnd), WM_MENUSELECT, MAKELONG(lMenuOpen, 0xFFFF), (LPARAM)hMenu);
	
		UpdateWindow(hWnd);
	}

	lMenuOpen = -1;

	if (!bKeyboardAction) {
		//SetFocus(hPreviousFocus);
		lButtonPressed = -1;
		}	

	oTrace->EndTrace(__FUNCTION__, eAll);
}

//This hook is for processing message while a submenu is visible. This sub
//menu is modal so we need something to track messages.
LRESULT CMenuToolbar::MenuMessageFilter(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG* pMsg = (MSG*)lParam;

	if (g_pMenuToolbar && nCode == MSGF_MENU)
	{
		// process menu message
		if (g_pMenuToolbar->OnMenuMessage(pMsg))
			return TRUE;
	}

	return CallNextHookEx(g_hMsgHook, nCode, wParam, lParam);
}

//This function processes the messages while a submenu is visible
BOOL CMenuToolbar::OnMenuMessage(MSG* pMsg) {
	
	BOOL bResult = FALSE;
	long lButtonCount;

	switch(pMsg->message)
	{
	case WM_MOUSEMOVE:
		{
		//check if another button is highlighted. If so then open another
		//submenu
		POINT cMousePosition;
		long  lButtonIndex;
			
		cMousePosition.x = LOWORD(pMsg->lParam);
		cMousePosition.y = HIWORD(pMsg->lParam);
		ScreenToClient(hWnd, &cMousePosition);

		if (cLastMousePosition.x == cMousePosition.x &&
				cLastMousePosition.y == cMousePosition.y) return TRUE; //mouse not moved

			cLastMousePosition.x = cMousePosition.x;
			cLastMousePosition.y = cMousePosition.y;

			lButtonIndex = (long)SendMessage(hWnd, TB_HITTEST, 0, (LPARAM)&cMousePosition);

			if (lButtonIndex>=0 && lButtonIndex<lNumberButtons && lButtonIndex!=lMenuOpen) {

				lNextMenuOpen = lButtonIndex;

				//SendMessage(hWnd, TB_PRESSBUTTON, lMenuOpen, FALSE);
				SendMessage(hWnd, WM_CANCELMODE, 0, 0);

				//PostMessage(GetParent(hWnd), WM_OPENMENU, lButtonIndex, 0);

				bResult = TRUE;
				}
		break;
			}

	case WM_LBUTTONDOWN:
		{
		//close menu if the same button is clicked again

		//check if another button is highlighted. If so then open another
		//submenu
		POINT cMousePosition;
		long  lButtonIndex;
			
		cMousePosition.x = LOWORD(pMsg->lParam);
		cMousePosition.y = HIWORD(pMsg->lParam);
		ScreenToClient(hWnd, &cMousePosition);

		lButtonIndex = (long)SendMessage(hWnd, TB_HITTEST, 0, (LPARAM)&cMousePosition);

		//the menu is already open, close it and do not directly open it again
		if (lButtonIndex==lMenuOpen) {
			lNextMenuOpen = -1;
			SendMessage(hWnd, WM_CANCELMODE, 0, 0);
			bResult = TRUE;
			}
		}
		break;

	case WM_KEYDOWN:
		{
			switch (pMsg->wParam)
			{
			case VK_LEFT: //Open the menu to the left
				
				if (!bInSubmenu) {
					lNextMenuOpen = lMenuOpen - 1;
					lButtonCount = (long)SendMessage(hWnd, TB_BUTTONCOUNT, 0, 0);
			
					if (lNextMenuOpen<0) lNextMenuOpen = lButtonCount-1;

					SendMessage(hWnd, WM_CANCELMODE, 0, 0);
					bResult = TRUE;
					bKeyboardAction = true;
					}
				break;

			case VK_RIGHT: //Open the menu to the right

				if (!bMenuItemIsPopup) {
				
					lNextMenuOpen = lMenuOpen + 1;
					lButtonCount = (long)SendMessage(hWnd, TB_BUTTONCOUNT, 0, 0);
			
					if (lNextMenuOpen==lButtonCount) lNextMenuOpen = 0;

					SendMessage(hWnd, WM_CANCELMODE, 0, 0);
					bKeyboardAction = true;
					bResult = true;
					}
				break;

			case VK_ESCAPE:
				lNextMenuOpen = -1;
				SendMessage(hWnd, WM_CANCELMODE, 0, 0);
				bKeyboardAction = true;
				bResult = TRUE;
				break;
			}
		}
		break;

	case WM_OPENMENU:
		{
		OpenSubMenu((long)pMsg->wParam);
		bResult = true;
		break;
		}

	case WM_MENUSELECT:
		{
		// store info about selected item
		bMenuItemIsPopup = (HIWORD(pMsg->wParam) & MF_POPUP) != 0;
		bInSubmenu = (hSubMenu != (HMENU)pMsg->lParam);

		bResult = false;
		break;
		}
	}

	return bResult;
}
