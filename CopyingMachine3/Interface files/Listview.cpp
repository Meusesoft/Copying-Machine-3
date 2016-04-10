#include "StdAfx.h"
#include "Listview.h"

CListview::CListview(sGlobalInstances pInstances, 
					 std::wstring psWindowName) : 
	CWindowBase(pInstances, psWindowName)
{
	hWnd				= NULL;
	hInstance			= NULL;
	hNormalImagelist	= NULL;
	hSmallImagelist		= NULL;
	hStateImagelist		= NULL;
	hDragImageList		= NULL;
	bDestroyNormalImagelist = true;
	bDestroySmallImagelist = true;
	bDestroyStateImagelist = true;
	bDestroyDragImageList = true;

	oTrace				= pInstances.oTrace;
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &pInstances, sizeof(sGlobalInstances));
	bDragging			= false;
	hDragDropCursor		= LoadCursor(pInstances.hInstance, MAKEINTRESOURCE(IDC_CURSORDRAGDROP));

	//Get the version of windows. The method of drag and drop
	//WindowsXP and Windows Vista
	OSVERSIONINFO cVersionInformation;

	cWindowsVersion = eLVWindowsUnknown;
	cVersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	if (GetVersionEx(&cVersionInformation)) {

		cWindowsVersion = eLVWindowsOlderThanXP;
		if (cVersionInformation.dwMajorVersion==5 && cVersionInformation.dwMinorVersion>0) 
			cWindowsVersion = eLVWindowsXP;
		if (cVersionInformation.dwMajorVersion>=6) cWindowsVersion = eLVWindowsNewerThanVista;
		if (cVersionInformation.dwMajorVersion==6 && cVersionInformation.dwMinorVersion==0) cWindowsVersion = eLVWindowsVista;
		}
	}

CListview::~CListview(void)
{
	Destroy();
}

HWND CListview::Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra, HMENU phID) {

	oTrace->StartTrace(__WFUNCTION__);

	hInstance = phInstance;
	
	DWORD dwStyle = WS_TABSTOP | 
					WS_CHILD |
					LVS_AUTOARRANGE |
					/*LVS_SHAREIMAGELISTS |*/
					WS_VISIBLE |
					WS_CLIPCHILDREN | 
					WS_CLIPSIBLINGS |
					/*WS_BORDER |*/ 
					pdStyleExtra;

	//if (bUseCustomControlView) dwStyle = dwStyle & ~WS_BORDER;
            
	hWnd = CreateWindowEx(0,         // ex style
						 WC_LISTVIEW,               // class name - defined in commctrl.h
						 NULL,                      // window text
						 dwStyle,                   // style
						 0,                         // x position
						 30,                        // y position
						 300,                       // width
						 300,                       // height
						 phParent,					// parent
						 (HMENU)phID,				// ID
						 hInstance,                 // instance
						 NULL);                     // no extra data
	
	//if (bUseCustomControlView) {
	//	ListView_SetBkColor(hWnd, RGB(defBackgroundR, defBackgroundG, defBackgroundB));
	//	ListView_SetTextBkColor(hWnd, RGB(defBackgroundR, defBackgroundG, defBackgroundB));
	//	}

	DWORD dExStyle = ListView_GetExtendedListViewStyle(hWnd);
	ListView_SetExtendedListViewStyle(hWnd, dExStyle | LVS_EX_DOUBLEBUFFER | LVS_EX_BORDERSELECT );

	dExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	dExStyle &= ~WS_EX_CLIENTEDGE;
	SetWindowLong(hWnd, GWL_EXSTYLE, dExStyle);


	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
	}

void CListview::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	if (hWnd!=NULL) DestroyWindow(hWnd);

	if (hNormalImagelist!=NULL && bDestroyNormalImagelist)	ImageList_Destroy(hNormalImagelist);
	if (hSmallImagelist!=NULL && bDestroySmallImagelist)	ImageList_Destroy(hSmallImagelist);
	if (hStateImagelist!=NULL && bDestroyStateImagelist)	ImageList_Destroy(hStateImagelist);
	if (hDragImageList!=NULL && bDestroyDragImageList)		ImageList_Destroy(hDragImageList);

	oTrace->EndTrace(__WFUNCTION__);
}

//This function changes the selected item in the listview
bool 
CListview::SetSelectedItem(int piIndex) {

	int iItem;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	//Unselect item
	iItem = GetSelectedItem();
	if (iItem!=-1) {

		ListView_SetItemState(hWnd, iItem, 0, LVIS_SELECTED);
		ListView_EnsureVisible(hWnd, iItem, TRUE);
		}

	//Select new item
	ListView_SetItemState(hWnd, piIndex, LVIS_SELECTED, LVIS_SELECTED);

	oTrace->EndTrace(__WFUNCTION__, piIndex, eAll);

	return true;
	}

//This function returns the index of the selected item, or -1 if non is
//selected
int 
CListview::GetSelectedItem() {

	int iItemCount;
	int iResult;
	bool bFound;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	iResult = -1;
	bFound = false;

	iItemCount = ListView_GetItemCount(hWnd);

	while (iItemCount>0 && !bFound) {

		iItemCount--;

		if (ListView_GetItemState(hWnd, iItemCount, LVIS_SELECTED)!=0) {

			iResult = iItemCount;
			bFound = true;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, iResult, eAll);

	return iResult;
	}

//This is the message handler for the listview
bool 
CListview::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {


	bool bReturn;
	LPNMHDR cNotificationHeader;
	LPNMLISTVIEW cNotificationListview;
	LPNMITEMACTIVATE cNotificationItemActivate;

	bReturn = false;

	switch (message) {

		/*case WM_ERASEBKGND:

			bReturn = true;
			break;

		case WM_SIZE:
			bReturn = true;
			break;*/

		case WM_LBUTTONUP:

			if (bDragging) OnEndDrag(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_MOUSEMOVE:
			
			if (bDragging) OnMoveDrag(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_NOTIFY:

			cNotificationHeader = (LPNMHDR)lParam;
			
			if (cNotificationHeader->hwndFrom == hWnd) {

				cNotificationListview = (LPNMLISTVIEW)lParam;
						
				switch (cNotificationHeader->code) {

					case NM_RCLICK: //the user pressed the right mouse button

						OnRButtonClick();

						bReturn = true;
						break;

					case NM_DBLCLK: //the user double clicked on an item

						cNotificationItemActivate = (LPNMITEMACTIVATE)lParam;

						OnButtonDoubleClick(cNotificationItemActivate->iItem);

						bReturn = true;
						break;

				   case LVN_BEGINDRAG: //the user started a drag-operation

					    OnBeginDrag((LPNMHDR)lParam);

						bReturn = true;
						break;

				   case LVN_COLUMNCLICK: //the user clicks on a column;

						LPNMLISTVIEW oNmListView;

						oNmListView = (LPNMLISTVIEW)lParam;

						OnColumnClick(oNmListView->iSubItem);

						bReturn = true;
						break;

					case LVN_ITEMCHANGED: //the item is changed. Notify the pageviewer to update

						long lItemIndex;

						if ((cNotificationListview->uChanged & LVIF_STATE) == LVIF_STATE) {

							//detect if the state was a selection change
							if ((cNotificationListview->uNewState & LVIS_SELECTED)!=
								(cNotificationListview->uOldState & LVIS_SELECTED)) {

								//retrieve the page number
								lItemIndex = GetSelectedItem();
								
								if (lItemIndex!=-1) OnSelectionChanged(lItemIndex);
								
								bReturn = true;
								}
							}

						break;
					}
				}

			break;
		}

	//Process the base wndproc 
	if (!bReturn) {

		bReturn = CWindowBase::WndProc(phWnd, message, wParam, lParam, piReturn);
		}

	return bReturn;
	}



//This function is called when the selection in the listview has
//changed. 
void 
CListview::OnSelectionChanged(int piIndex) {

	}

//This function is called when the column is clicked
void 
CListview::OnColumnClick(int piColumn) {


	}

//This function is called when an item is double clicked
void 
CListview::OnButtonDoubleClick(int piItem) {


}

//This function is called when the user presses the right mouse button
//inside the listview. This function checks if an item in the listview
//was checked and if so, calls DoContextMenu
void 
CListview::OnRButtonClick() {

	LVHITTESTINFO cHitTest;
	POINT cPoint;
	HMENU hContextMenu;

	GetCursorPos(&cPoint);
	ScreenToClient(hWnd, &cPoint);

	SecureZeroMemory(&cHitTest, sizeof(cHitTest));

	cHitTest.pt.x = cPoint.x;
	cHitTest.pt.y = cPoint.y;
	cHitTest.flags = LVHT_ONITEM;

	if (SendMessage(hWnd, LVM_HITTEST, 0, (LPARAM)&cHitTest)!=-1) {

		//Create a handle to the new popup menu
		hContextMenu = CreatePopupMenu();

		if (DoContextMenu(cHitTest.iItem, hContextMenu)) {
		
			//Process the menu and show it
			oGlobalInstances.oMenuIcon->ProcessMenu(hContextMenu);

			GetCursorPos(&cPoint);

			TrackPopupMenu(hContextMenu, 
						   TPM_LEFTALIGN | TPM_TOPALIGN,
						   cPoint.x,
						   cPoint.y,
						   0,
						   GetParent(hWnd), NULL);
			}

		//Clean up
		DestroyMenu(hContextMenu);
		}
	else {

		//Create a handle to the new popup menu
		hContextMenu = CreatePopupMenu();

		if (DoContextMenu(hContextMenu)) {
	
			//Process the menu and show it
			oGlobalInstances.oMenuIcon->ProcessMenu(hContextMenu);

			GetCursorPos(&cPoint);

			TrackPopupMenu(hContextMenu, 
						   TPM_LEFTALIGN | TPM_TOPALIGN,
						   cPoint.x,
						   cPoint.y,
						   0,
						   GetParent(hWnd), NULL);
			}

		//Clean up
		DestroyMenu(hContextMenu);
		}
	}

//This function is called when the user presses the right mouse button
//on an item in the listview
bool 
CListview::DoContextMenu(int piItem, HMENU hContextMenu) {

	return false;
}

//This function is called when the user presses the right mouse button
//on the listview
bool 
CListview::DoContextMenu(HMENU hContextMenu) {

	return false;
}

//These functions perform the drag and drop operation within the listview
void 
CListview::OnBeginDrag(LPNMHDR pNmHdr) {

	LVHITTESTINFO       lvhti;
	POINT p;
	POINT pt;
	bool bFirst;
	int  iPos;
	int  iHeight;
	HIMAGELIST hOneImageList;
	HIMAGELIST hTempImageList;
	IMAGEINFO  imf;

	// You can set your customized cursor here
    p.x = 8;
    p.y = 8;

	if (cWindowsVersion == eLVWindowsVista) {

		// Ok, now we create a drag-image for all selected items
		bFirst = TRUE;
		iPos = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);

		while (iPos != -1) {

			if (bFirst) {
				// For the first selected item,
				// we simply create a single-line drag image
				hDragImageList = ListView_CreateDragImage(hWnd, iPos, &p);
				ImageList_GetImageInfo(hDragImageList, 0, &imf);
				iHeight = imf.rcImage.bottom;
				bFirst = FALSE;
				}
			else {
				// For the rest selected items,
				// we create a single-line drag image, then
				// append it to the bottom of the complete drag image
				hOneImageList = ListView_CreateDragImage(hWnd, iPos, &p);
				hTempImageList = ImageList_Merge(hDragImageList, 
								 0, hOneImageList, 0, 0, iHeight);
				ImageList_Destroy(hDragImageList);
				ImageList_Destroy(hOneImageList);
				hDragImageList = hTempImageList;
				ImageList_GetImageInfo(hDragImageList, 0, &imf);
				iHeight = imf.rcImage.bottom;
			}
			iPos = ListView_GetNextItem(hWnd, iPos, LVNI_SELECTED);
		}

		// Now we can initialize then start the drag action
		GetCursorPos(&p);
		lvhti.pt.x = p.x;
		lvhti.pt.y = p.y;
		ScreenToClient(hWnd, &lvhti.pt);
		ListView_HitTest(hWnd, &lvhti);

		RECT cRect;
		ListView_GetItemRect(hWnd, lvhti.iItem, &cRect, LVIR_ICON);

		ImageList_BeginDrag(hDragImageList, 0, lvhti.pt.x - cRect.left, lvhti.pt.y - cRect.top);

		pt = ((NM_LISTVIEW*)pNmHdr)->ptAction;
		ClientToScreen(hWnd, &pt);

		ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);
		}

    bDragging = TRUE;

	ListView_SetInsertMarkColor(hWnd, RGB(192, 192, 192));

    // Don't forget to capture the mouse
    SetCapture(GetParent(hWnd));

	//Set the cursor
	hStandardCursor = SetCursor(hDragDropCursor);
	}

void 
CListview::OnMoveDrag(int piX, int piY) {

	POINT p;
	
	if (bDragging) {

		p.x = piX;
		p.y = piY;

		ClientToScreen(GetParent(hWnd), &p);
		if (cWindowsVersion == eLVWindowsVista) ImageList_DragMove(p.x, p.y);
		ImageList_DragShowNolock(false);

	    // Determine the dropped item
		LVHITTESTINFO       lvhti;
		LVINSERTMARK		cInsertMark;
		POINT cPoint;

		SecureZeroMemory(&lvhti, sizeof(lvhti));
		lvhti.pt.x = piX;
		lvhti.pt.y = piY;
		ClientToScreen(GetParent(hWnd), &lvhti.pt);
		ScreenToClient(hWnd, &lvhti.pt);
		ListView_HitTest(hWnd, &lvhti);

		RECT cWindowRect;
		POINT cPositionOnScreen;

		//Make the given position signed
		SHORT sX;
		SHORT sY;

		sX = (SHORT)piX;
		sY = (SHORT)piY;

		cPositionOnScreen.x = sX;
		cPositionOnScreen.y = sY;
		ClientToScreen(GetParent(hWnd), &cPositionOnScreen);

		GetWindowRect(hWnd, &cWindowRect);
		
		//Determine if we need to scroll
		if (cPositionOnScreen.y < cWindowRect.top && cPreviousPosition.y > cPositionOnScreen.y) {

			ListView_Scroll(hWnd, 0, -5);
			}
		if (cPositionOnScreen.y > cWindowRect.bottom && cPreviousPosition.y < cPositionOnScreen.y) {

			ListView_Scroll(hWnd, 0, 5);
			}
		if (cPositionOnScreen.x < cWindowRect.left && cPreviousPosition.x > cPositionOnScreen.x) {

			ListView_Scroll(hWnd, -5, 0);
			}
		if (cPositionOnScreen.x > cWindowRect.right && cPreviousPosition.x < cPositionOnScreen.x) {

			ListView_Scroll(hWnd, 5, 0);
			}

		cPreviousPosition = cPositionOnScreen;

		//set the insertion mark
		cInsertMark.cbSize = sizeof(cInsertMark);
		cInsertMark.iItem = -1;
		cInsertMark.dwFlags = 0;

		cPoint.x = piX;
		cPoint.y = piY;
		ClientToScreen(GetParent(hWnd), &cPoint);
		ScreenToClient(hWnd, &cPoint);
		
		ListView_InsertMarkHitTest(hWnd, &cPoint, &cInsertMark);

		ListView_SetInsertMark(hWnd, &cInsertMark);

		ImageList_DragShowNolock(true);
		}
	}

void 
CListview::OnEndDrag(int piX, int piY) {

	LVHITTESTINFO       lvhti;
	LVITEM				lvi;
	LVINSERTMARK		cInsertMark;

	// End the drag-and-drop process
    bDragging = false;
    
	if (cWindowsVersion == eLVWindowsVista) {

		ImageList_DragLeave(hWnd);
		ImageList_EndDrag();
		ImageList_Destroy(hDragImageList);
		hDragImageList = NULL;
		}

	cInsertMark.cbSize = sizeof(cInsertMark);
	cInsertMark.iItem = -1;
	cInsertMark.dwFlags = 0;
	ListView_SetInsertMark(hWnd, &cInsertMark);
	
	ReleaseCapture();
	SetCursor(hStandardCursor);

    // Determine the dropped item
    lvhti.pt.x = piX;
    lvhti.pt.y = piY;
    ClientToScreen(GetParent(hWnd), &lvhti.pt);
    ScreenToClient(hWnd, &lvhti.pt);
    ListView_HitTest(hWnd, &lvhti);

    // Inside the ListView?
	if (lvhti.flags & (LVHT_NOWHERE | LVHT_ONITEM)) {
       
		POINT cPoint;
		cPoint.x = piX;
		cPoint.y = piY;
		ClientToScreen(GetParent(hWnd), &cPoint);
		ScreenToClient(hWnd, &cPoint);
		
		ListView_InsertMarkHitTest(hWnd, &cPoint, &cInsertMark);


		// In an item?
		//if (lvhti.flags & LVHT_ONITEM) {

		if (cInsertMark.iItem!=-1) {

			lvhti.iItem = cInsertMark.iItem;

			// Dropped item is selected?
			lvi.iItem = lvhti.iItem;
			lvi.iSubItem = 0;
			lvi.mask = LVIF_STATE;
			lvi.stateMask = LVIS_SELECTED;
			ListView_GetItem(hWnd, &lvi);

			if ((lvi.state & LVIS_SELECTED)==0) {

				if (cInsertMark.dwFlags==LVIM_AFTER) lvhti.iItem++;

				RearrangeAfterDrag(lvhti.iItem);
				}
			}
		}
	}

//This function is called to rearrange all the items when
//a drag-and-drop operation is done
void 
CListview::RearrangeAfterDrag(int piNewItemIndex) {


	}

//Wrappers for the standard api calls
int 
CListview::SetView(DWORD piView) {

	return ListView_SetView(hWnd, piView);
	}

//Get the column width
int 
CListview::GetColumnWidth(int iCol) {

	return ListView_GetColumnWidth(hWnd, iCol);
	}

//Get the column width
void 
CListview::SetColumnWidth(int iCol, int piWidth) {

	ListView_SetColumnWidth(hWnd, iCol, piWidth);
	}

int 
CListview::InsertColumn(int iCol, const LPLVCOLUMN pcol) {

	return ListView_InsertColumn(hWnd, iCol, pcol);
	}

int 
CListview::InsertColumn(int piCol, int piResource, int piWidth, int piFmt) {

	wchar_t cName[MAX_PATH];

	LoadString(oGlobalInstances.hLanguage, piResource, cName, MAX_PATH);

	return InsertColumn(piCol, cName, piWidth, piFmt);
	}

int 
CListview::InsertColumn(int piCol, std::wstring psName, int piWidth, int piFmt) {

	LVCOLUMN cColumn;

	cColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM ;//| LVCF_MINWIDTH;
	cColumn.cx = piWidth;
	cColumn.fmt = piFmt;
	//cColumn.cxMin = 10;
	cColumn.pszText = (LPWSTR)psName.c_str();

	return InsertColumn(piCol, &cColumn);
	}

//This function updates the title of the given column
void 
CListview::SetColumnTitle(int piCol, int piResource) {

	wchar_t cName[MAX_PATH];

	LoadString(oGlobalInstances.hLanguage, piResource, cName, MAX_PATH);

	SetColumnTitle(piCol, cName);
	}

void 
CListview::SetColumnTitle(int piCol, std::wstring psName) {

	LVCOLUMN cColumn;

	cColumn.mask = LVCF_TEXT;
	cColumn.pszText = (LPWSTR)psName.c_str();	

	ListView_SetColumn(hWnd, piCol, &cColumn);
	}

//Get the number of items in the listview
int
CListview::ItemCount() {

	return (int)ListView_GetItemCount(hWnd);
	}


