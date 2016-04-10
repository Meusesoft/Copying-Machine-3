#include "StdAfx.h"
#include "DocumentList.h"

CDocumentList::CDocumentList(sGlobalInstances pInstances,
							 std::wstring psWindowName) :
	CWindowBase(pInstances, psWindowName)
{
	hWnd = NULL;
	hInstance = NULL;
	hNormalImagelist = NULL;
	hSmallImagelist = NULL;
	hStateImagelist = NULL;

	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;
}

CDocumentList::~CDocumentList(void)
{
	oCore->oNotifications->UnsubscribeAll(hWnd);
	Destroy();
}

void CDocumentList::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	if (hWnd!=NULL) DestroyWindow(hWnd);
	if (hNormalImagelist!=NULL) ImageList_Destroy(hNormalImagelist);
	if (hSmallImagelist!=NULL) ImageList_Destroy(hSmallImagelist);
	if (hStateImagelist!=NULL) ImageList_Destroy(hStateImagelist);
	
	oTrace->EndTrace(__WFUNCTION__);
}


HWND CDocumentList::Create(HWND phParent, HINSTANCE phInstance) {

	oTrace->StartTrace(__WFUNCTION__);
	
	Bitmap* oBitmap;
	
	hInstance = phInstance;
	
	DWORD dwStyle = WS_TABSTOP | 
					WS_CHILD |
					WS_VISIBLE |
					TVS_FULLROWSELECT |
					TVS_SHOWSELALWAYS |
					WS_CLIPCHILDREN | 
					/*WS_BORDER |*/
					WS_CLIPSIBLINGS;

	//if (bUseCustomControlView) dwStyle = dwStyle & ~WS_BORDER;
            
	hWnd = CreateWindowEx(0,         // ex style
						 WC_TREEVIEW,               // class name - defined in commctrl.h
						 NULL,                      // window text
						 dwStyle,                   // style
						 0,                         // x position
						 30,                         // y position
						 300,                         // width
						 600,                         // height
						 phParent,						// parent
						 (HMENU)1009,				// ID
						 hInstance,                 // instance
						 NULL);                     // no extra data
	
	//if (bUseCustomControlView) {
	//	TreeView_SetBkColor(hWnd, RGB(defBackgroundR, defBackgroundG, defBackgroundB));
	//	//TreeView_SetTextBkColor(hWnd, RGB(defBackgroundR, defBackgroundG, defBackgroundB));
	//	}

	//Create an imagelist for the documentlist
	hSmallImagelist = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 1, 1);	
	
	//Load and add a standard image to the imagelist. This image will
	//be used as a temporary thumbnail
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLBOOK", L"PNG", phInstance);
	CBitmap::AddToImagelist(oBitmap, hSmallImagelist);
	delete oBitmap;
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLBOOKOPEN", L"PNG", phInstance);
	CBitmap::AddToImagelist(oBitmap, hSmallImagelist);
	delete oBitmap;
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLBOOKEDIT", L"PNG", phInstance);
	CBitmap::AddToImagelist(oBitmap, hSmallImagelist);
	delete oBitmap;
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLBOOKOPENEDIT", L"PNG", phInstance);
	CBitmap::AddToImagelist(oBitmap, hSmallImagelist);
	delete oBitmap;
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLBOOKADD", L"PNG", phInstance); 
	CBitmap::AddToImagelist(oBitmap, hSmallImagelist);
	delete oBitmap;
	
	//Attach the imagelist to the documentlist
	TreeView_SetImageList(hWnd, hSmallImagelist, TVSIL_NORMAL);

	//Add the standard items
	//TV_INSERTSTRUCT cAcquire;

	//cAcquire.hParent = NULL;
	//cAcquire.hInsertAfter = TVI_LAST;
	//cAcquire.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	//cAcquire.item.iImage = 4;
	//cAcquire.item.iSelectedImage = 4;
	//cAcquire.item.pszText = L"Acquire...";
	//cAcquire.item.lParam = -1;
	//TreeView_InsertItem(hWnd, &cAcquire);

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
	}

//This function is called when a language change is being processed
void 
CDocumentList::Localize(HINSTANCE phLanguage) {

	oGlobalInstances.hLanguage = phLanguage;
}


//This is the message handler for this class
bool CDocumentList::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	bool bReturn;
	LPNMHDR cNotificationHeader;
	LPNMTREEVIEW cNotificationTreeview;

	bReturn = false;

	//Handle the message since the base class didn't 
	switch (message) {

		//handle the notification from the core
		case WM_NOTIFY:

			cNotificationHeader = (LPNMHDR)lParam;
			
			if (cNotificationHeader->hwndFrom == hWnd) {

				cNotificationTreeview = (LPNMTREEVIEW)lParam;
						
				switch (cNotificationHeader->code) {

					case NM_RCLICK:

						OnRButtonClick();

						bReturn = true;
						break;
		
					case TVN_SELCHANGED: //the item is changed. Notify the pageviewer to update

						OnSelectionChanged((int)cNotificationTreeview->itemNew.lParam);
						bReturn = true;

						break;

					case TVN_SELCHANGING:

						if (cNotificationTreeview->itemNew.lParam==-1) {

							OnSelectionChanged((int)cNotificationTreeview->itemNew.lParam);
							piReturn = TRUE;
							bReturn = true;
							}
						break;
					}
				}

			break;
		}

	return bReturn;
	}

//This function is called when the right button is click on the
//control
void
CDocumentList::OnRButtonClick() {

	TVHITTESTINFO cHitTest;
	POINT cPoint;

	GetCursorPos(&cPoint);
	ScreenToClient(hWnd, &cPoint);

	SecureZeroMemory(&cHitTest, sizeof(cHitTest));

	cHitTest.pt.x = cPoint.x;
	cHitTest.pt.y = cPoint.y; 
	cHitTest.flags = TVHT_ONITEM;

	if (SendMessage(hWnd, TVM_HITTEST, 0, (LPARAM)&cHitTest)!=NULL) {

		TreeView_Select(hWnd, cHitTest.hItem, TVGN_CARET); 
		DoContextMenu(cHitTest.hItem);
		}
	else {

		DoContextMenu();
		}	
	}

//This function shows a context menu of a list item
void
CDocumentList::DoContextMenu(HTREEITEM phItem) {

	POINT cPoint;
	HMENU hContextMenu;
	wchar_t sMenuItem[100];

	oTrace->StartTrace(__WFUNCTION__);

	//Create a handle to the new popup menu
	hContextMenu = CreatePopupMenu();
	
	//The contents of the menu is dependent of the existence of a selection and where the cursor is hovering
	LoadString(oGlobalInstances.hLanguage, IDS_SAVE, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_FILE_SAVE, sMenuItem);

	LoadString(oGlobalInstances.hLanguage, IDS_SAVEAS, sMenuItem, 99);
	wcscat_s(sMenuItem, 99, L"...");
	AppendMenu(hContextMenu, MF_STRING, ID_FILE_SAVEAS, sMenuItem);

	LoadString(oGlobalInstances.hLanguage, IDS_CLOSE, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_FILE_CLOSE, sMenuItem);

	AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);

	LoadString(oGlobalInstances.hLanguage, IDS_PRINT, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_FILE_PRINT_DIRECT, sMenuItem);

	AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);

	LoadString(oGlobalInstances.hLanguage, IDS_ACQUIRENEWPAGE, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_DOCUMENT_ACQUIRENEWPAGE, sMenuItem);

	LoadString(oGlobalInstances.hLanguage, IDS_PASTENEWPAGE, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_EDIT_PASTEASNEWPAGE, sMenuItem);

	//Process the menu and show it
	oGlobalInstances.oMenuIcon->ProcessMenu(hContextMenu);

	GetCursorPos(&cPoint);

	TrackPopupMenu(hContextMenu, 
				   TPM_LEFTALIGN | TPM_TOPALIGN,
				   cPoint.x,
				   cPoint.y,
				   0,
				   GetParent(hWnd), NULL);

	//Clean up
	DestroyMenu(hContextMenu);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function shows the context item
void
CDocumentList::DoContextMenu() {

	POINT cPoint;
	HMENU hContextMenu;
	wchar_t sMenuItem[100];

	oTrace->StartTrace(__WFUNCTION__);

	//Create a handle to the new popup menu
	hContextMenu = CreatePopupMenu();
	
	//The contents of the menu is dependent of the existence of a selection and where the cursor is hovering
	LoadString(oGlobalInstances.hLanguage, IDS_NEWDOCUMENT, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_FILE_NEW, sMenuItem);

	AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);

	LoadString(oGlobalInstances.hLanguage, IDS_PASTEASNEWDOCUMENT, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_EDIT_PASTE, sMenuItem);

	//Process the menu and show it
	oGlobalInstances.oMenuIcon->ProcessMenu(hContextMenu);

	GetCursorPos(&cPoint);

	TrackPopupMenu(hContextMenu, 
				   TPM_LEFTALIGN | TPM_TOPALIGN,
				   cPoint.x,
				   cPoint.y,
				   0,
				   GetParent(hWnd), NULL);

	//Clean up
	DestroyMenu(hContextMenu);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function is called when the selection in the listview has
//changed. 
void 
CDocumentList::OnSelectionChanged(int piDocumentID) {

	switch (piDocumentID) {

		case -1:
			PostMessage(GetParent(hWnd), WM_COMMAND, ID_FILE_NEW, ID_FILE_NEW);
			break;

		case -2:
			PostMessage(GetParent(hWnd), WM_COMMAND, MAKEWORD(ID_FILE_OPEN, 0), 0);
			break;

		default:
	
			oCore->SetCurrentDocumentByID(piDocumentID);
			oCore->SetCurrentPage(0);
			break;
		}

	SyncDocumentSelection();
	}

//This function checks if a document is in the document list
bool 
CDocumentList::CheckItemInList(int piDocumentID) {

	TV_ITEM cItem;
	HTREEITEM hTreeItem;
	bool bExists;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bExists = false;

	//get the top node
	hTreeItem = TreeView_GetRoot(hWnd);
	
	while (hTreeItem!=NULL && !bExists) {

		cItem.hItem = hTreeItem;
		cItem.mask = TVIF_PARAM ;

		if (TreeView_GetItem(hWnd, &cItem)) {

			bExists = (piDocumentID == (int)cItem.lParam);
			}

		hTreeItem = TreeView_GetNextItem(hWnd, hTreeItem, TVGN_NEXT);
		}
	
	oTrace->EndTrace(__WFUNCTION__, bExists, eAll);

	return bExists;
}



//This function adds a document to the document list
bool 
CDocumentList::AddItem(int piDocumentID) {

	bool bReturn;
	CCopyDocument* oDocument;
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	bReturn = false;

	oDocument = oCore->oDocuments->GetDocumentByID(piDocumentID);

	if (oDocument!=NULL && !CheckItemInList(piDocumentID)) {

		//add item to listview					
		wchar_t cItemText[260];
		wcscpy_s(cItemText, 259, oDocument->GetTitle().c_str());

		TV_INSERTSTRUCT cInsertItem;
		ZeroMemory(&cInsertItem, sizeof(cInsertItem));
		cInsertItem.hParent = TVI_ROOT;
		cInsertItem.hInsertAfter = TVI_LAST;
		cInsertItem.item.mask = TVIF_PARAM | TVIF_TEXT;
		cInsertItem.item.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		cInsertItem.item.lParam = (LPARAM)piDocumentID;
		cInsertItem.item.pszText = cItemText;
		cInsertItem.item.iImage = 0;
		cInsertItem.item.iSelectedImage = 1;

		bReturn = (TreeView_InsertItem(hWnd, &cInsertItem)!=NULL);
		}

	if (bReturn) SyncDocumentSelection();

	oTrace->EndTrace(__WFUNCTION__, bReturn, eAll);

	return bReturn;
}

//This function adds a document to the document list
bool 
CDocumentList::DeleteItem(int piDocumentID) {

	bool bReturn;
	TV_ITEM cItem;
	HTREEITEM hTreeItem;
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	bReturn = false;

	//get the top node
	hTreeItem = TreeView_GetRoot(hWnd);
	
	while (hTreeItem!=NULL && !bReturn) {

		cItem.hItem = hTreeItem;
		cItem.mask = TVIF_PARAM | TVIF_STATE;
		cItem.stateMask = TVIS_SELECTED;

		if (TreeView_GetItem(hWnd, &cItem)) {

			if (piDocumentID == (int)cItem.lParam) {

				TreeView_DeleteItem(hWnd, hTreeItem);
				bReturn = true;
				}
			}

		hTreeItem = TreeView_GetNextItem(hWnd, hTreeItem, TVGN_NEXT);
		}

	if (bReturn) SyncDocumentSelection();

	oTrace->EndTrace(__WFUNCTION__, bReturn, eAll);

	return bReturn;
}

//This function selects an item in the document list based on the
//curent document
void
CDocumentList::SyncDocumentSelection() {

	CCopyDocument* oDocument;
	TV_ITEM cItem;
	HTREEITEM hTreeItem;
	HTREEITEM hSelectedItem;

	int	iDocumentID;
	bool bIsDirty;
	wchar_t cItemText[260];

	oTrace->StartTrace(__WFUNCTION__, eAll);

	iDocumentID = -1;
	hSelectedItem = NULL;
	oDocument = oCore->GetCurrentDocument();
	
	if (oDocument!=NULL) {
		iDocumentID = oDocument->GetDocumentID();
		}

	//get the top node
	hTreeItem = TreeView_GetRoot(hWnd);
	
	while (hTreeItem!=NULL) {

		cItem.hItem = hTreeItem;
		cItem.mask = TVIF_PARAM | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
		cItem.stateMask = TVIS_SELECTED;
		cItem.pszText = cItemText;
		cItem.cchTextMax = 260;

		if (TreeView_GetItem(hWnd, &cItem)) {

			if ((iDocumentID == (int)cItem.lParam)) hSelectedItem = cItem.hItem;
			cItem.state = ((iDocumentID == (int)cItem.lParam) ? TVIS_SELECTED : 0);
			oDocument = oCore->oDocuments->GetDocumentByID((int)cItem.lParam);
			bIsDirty = false;
			if (oDocument!=NULL) {
				bIsDirty = oDocument->GetDirty();
				wcscpy_s(cItemText, 259, oDocument->GetTitle().c_str());
				}

			cItem.iImage = bIsDirty ? 2 : 0;
			cItem.iSelectedImage = bIsDirty ? 3 : 1;
			
			TreeView_SetItem(hWnd, &cItem);
			}

		hTreeItem = TreeView_GetNextItem(hWnd, hTreeItem, TVGN_NEXT);
		}

	if (hSelectedItem!=NULL) TreeView_SelectItem(hWnd, hSelectedItem);
	
	oTrace->EndTrace(__WFUNCTION__, eAll);
}
