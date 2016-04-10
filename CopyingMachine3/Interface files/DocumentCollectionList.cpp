#include "StdAfx.h"
#include "DocumentCollectionList.h"

CDocumentCollectionList::CDocumentCollectionList(sGlobalInstances pInstances,
												 std::wstring psWindowName) :
	CWindowBase(pInstances, psWindowName)
{
	hInstance = NULL;
	hNormalImagelist = NULL;
	hSmallImagelist = NULL;
	hStateImagelist = NULL;

	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;
}

CDocumentCollectionList::~CDocumentCollectionList(void)
{
	oCore->oNotifications->UnsubscribeAll(hWnd);
	Destroy();
}

void CDocumentCollectionList::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	if (hWnd!=NULL) DestroyWindow(hWnd);
	if (hNormalImagelist!=NULL) ImageList_Destroy(hNormalImagelist);
	if (hSmallImagelist!=NULL) ImageList_Destroy(hSmallImagelist);
	if (hStateImagelist!=NULL) ImageList_Destroy(hStateImagelist);
	
	oTrace->EndTrace(__WFUNCTION__);
}


HWND CDocumentCollectionList::Create(HWND phParent, HINSTANCE phInstance) {

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
	
	//Load and add images to the imagelist. 
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLFOLDER", L"PNG", phInstance);
	CBitmap::AddToImagelist(oBitmap, hSmallImagelist);
	delete oBitmap;
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLFOLDEREXPLORE", L"PNG", phInstance);
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

	SyncDocumentCollections();

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
	}

//This function is called when a language change is being processed
void 
CDocumentCollectionList::Localize(HINSTANCE phLanguage) {

	oGlobalInstances.hLanguage = phLanguage;
}


//This is the message handler for this class
bool CDocumentCollectionList::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	bool bReturn;
	LPNMHDR cNotificationHeader;
	LPNMTREEVIEW cNotificationTreeview;
	TVITEM cTreeviewItem;
	HTREEITEM hItem;

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
		
					case NM_CLICK:

						hItem = TreeView_GetSelection(hWnd);
						cTreeviewItem.mask = TVIF_PARAM;
						cTreeviewItem.hItem = hItem;

						if (TreeView_GetItem(hWnd, &cTreeviewItem)) {									

							OnSelectionChanged((int)cTreeviewItem.lParam);
							}

						bReturn = true;
						break;

					case TVN_SELCHANGED: 

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
CDocumentCollectionList::OnRButtonClick() {

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
	}

//This function shows a context menu of a list item
void
CDocumentCollectionList::DoContextMenu(HTREEITEM phItem) {

	POINT cPoint;
	HMENU hContextMenu;
	wchar_t sMenuItem[100];

	oTrace->StartTrace(__WFUNCTION__);

	//Create a handle to the new popup menu
	hContextMenu = CreatePopupMenu();
	
	//The contents of the menu is dependent of the existence of a selection and where the cursor is hovering
	LoadString(oGlobalInstances.hLanguage, IDS_NEWDOCUMENT, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, IDC_ACQUIRETOCOLLECTION, sMenuItem);

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
CDocumentCollectionList::OnSelectionChanged(int piIndex) {

	oCore->oNotifications->CommunicateEvent(eNotificationDocumentCollectionSelection, (void*)piIndex);
}

//This function returns the currently selected collection
int  
CDocumentCollectionList::GetSelectedCollection() {

	int iSelectedCollection;
	HTREEITEM hItem;
	TVITEM    oItem;

	hItem = TreeView_GetSelection(hWnd);

	SecureZeroMemory(&oItem, sizeof(TVITEM));

	oItem.hItem = hItem;
	oItem.mask = TVIF_PARAM;
	TreeView_GetItem(hWnd, &oItem);

	iSelectedCollection = oItem.lParam;

	return iSelectedCollection;
}

//This function selects an item in the document list based on the
//curent document
void
CDocumentCollectionList::SyncDocumentCollections() {

	sDocumentCollection oDocumentCollection;
	int iCollections;
	int iTreeItems;
	HTREEITEM hItem;
	TV_INSERTSTRUCT cInsertItem;
	TV_ITEM cTreeItem;
	wchar_t cItemText[260];

	oTrace->StartTrace(__WFUNCTION__, eAll);

	iCollections = oCore->oDocumentCollections->GetDocumentCollectionCount();
	iTreeItems   = TreeView_GetCount(hWnd);

	//TreeView_DeleteAllItems(hWnd);

	//Synchronize the number of items in the treeview
	while (iCollections != iTreeItems) {

		if (iCollections > iTreeItems) {

			//Add an item
			ZeroMemory(&cInsertItem, sizeof(cInsertItem));
			cInsertItem.hParent = TVI_ROOT;
			cInsertItem.hInsertAfter = TVI_FIRST;
			cInsertItem.item.mask = TVIF_PARAM | TVIF_TEXT;
			cInsertItem.item.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			cInsertItem.item.lParam = (LPARAM)0;
			cInsertItem.item.pszText = L"";
			cInsertItem.item.iImage = 0;
			cInsertItem.item.iSelectedImage = 1;

			TreeView_InsertItem(hWnd, &cInsertItem);

			iTreeItems++;
			}
		else {

			//Delete an item
			hItem = TreeView_GetLastVisible(hWnd);
			TreeView_DeleteItem(hWnd, hItem);
				
			iTreeItems--;
			}
		}

	//Update the contents of the tree items
	hItem = TreeView_GetRoot(hWnd);

	for (long iIndex=0; iIndex<iCollections; iIndex++) {

		oCore->oDocumentCollections->GetDocumentCollection(iIndex, oDocumentCollection);

		//update item of the treeview
		wcscpy_s(cItemText, 259, oDocumentCollection.sName.c_str());

		ZeroMemory(&cTreeItem, sizeof(cTreeItem));
		cTreeItem.hItem = hItem;
		cTreeItem.mask = TVIF_PARAM | TVIF_TEXT;
		cTreeItem.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		cTreeItem.lParam = (LPARAM)iIndex;
		cTreeItem.pszText = cItemText;
		cTreeItem.iImage = 0;
		cTreeItem.iSelectedImage = 1;

		TreeView_SetItem(hWnd, &cTreeItem);

		//Get the next item
		hItem = TreeView_GetNextSibling(hWnd, hItem);
		}

	TreeView_SortChildren(hWnd, NULL, false);
	
	oTrace->EndTrace(__WFUNCTION__, eAll);
}
