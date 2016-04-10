#include "StdAfx.h"
#include "PageThumbnails.h"
#include "math.h"

CPageThumbnails::CPageThumbnails(sGlobalInstances pInstances, std::wstring psWindowName): 
	CListview(pInstances, psWindowName)
{
	//Initialize the critical section object. 
	bEndThread = !InitializeCriticalSectionAndSpinCount(&ThumbnailCriticalSection, 0x80000400);
	hThumbnailThread = NULL;

	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &pInstances, sizeof(sGlobalInstances));

	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;
}

CPageThumbnails::~CPageThumbnails(void)
{
	bEndThread = true;

	//Clean up the thread
	if (hThumbnailThread!=NULL) {
		
		//Wait for the thread to finish.
		WaitForSingleObject(hThumbnailThread, INFINITE);
	
		CloseHandle(hThumbnailThread);

		DeleteCriticalSection(&ThumbnailCriticalSection);
		}

	//Unsubscribe to all the notifications from the core
	oCore->oNotifications->UnsubscribeAll(hWnd);

	//clean up the vectors
	vThumbnails.clear();
	vToBeProcessedThumbnails.clear();
}

HWND CPageThumbnails::Create(HWND phParent, HINSTANCE phInstance) {

	oTrace->StartTrace(__WFUNCTION__);
	
	HBITMAP hBitmap;
	
	CListview::Create(phParent, phInstance, LVS_ICON | LVS_SHOWSELALWAYS | /*WS_HSCROLL |*/ LVS_EX_BORDERSELECT /*| LVS_ALIGNLEFT /* | LVS_AUTOARRANGE*/);
	
	DWORD dExStyle = ListView_GetExtendedListViewStyle(hWnd);
	ListView_SetExtendedListViewStyle(hWnd, dExStyle /*| LVS_EX_SINGLEROW*/);

	long lWindowStyle = GetWindowLong(hWnd, GWL_STYLE);
	lWindowStyle &= ~WS_VSCROLL;
	lWindowStyle |= WS_HSCROLL;
	SetWindowLong(hWnd, GWL_STYLE, lWindowStyle);

	//Create an imagelist for the listview
	hNormalImagelist = ImageList_Create(ThumbnailSizeX, ThumbnailSizeY, 
									ILC_COLOR32, 1, 1);	

	
	//Load and add a standard image to the imagelist. This image will
	//be used as a temporary thumbnail
	hBitmap = (HBITMAP)LoadImage(hInstance,
						MAKEINTRESOURCE(163),//IDB_IMAGEPAGE),
						IMAGE_BITMAP,
						ThumbnailSizeX, ThumbnailSizeY,
						LR_DEFAULTCOLOR);

	ImageList_Add(hNormalImagelist, hBitmap, NULL);

	DeleteObject(hBitmap);

	//Attach the imagelist to the listview
	ListView_SetImageList(hWnd, hNormalImagelist, LVSIL_NORMAL);


	//Create a thread for making thumbnails. Only if the critical section
	//is initialized
	if (!bEndThread) {

		hThumbnailThread = CreateThread(
			NULL,				// default security attributes
			0,					// use default stack size  
			&ThreadCreateThumbnail,        // thread function 
			this,				// argument to thread function 
			0,					// use default creation flags 
			NULL);				// returns the thread identifier 
 
			}

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
	}

//This function clears the imagelist so a new document / list of images
//can be added
bool 
CPageThumbnails::Clear() {

	int iImageCount;

	EnterCriticalSection(&ThumbnailCriticalSection); 

	//clean up the vectors
	vThumbnails.clear();
	vToBeProcessedThumbnails.clear();

	//clear the listview
	ListView_DeleteAllItems(hWnd);

	//clean up the imagelist, remove all images except the first
	iImageCount = ImageList_GetImageCount(hNormalImagelist);

	while (iImageCount>1) {

		ImageList_Remove(hNormalImagelist, iImageCount);
		
		iImageCount--;
		}

	LeaveCriticalSection(&ThumbnailCriticalSection); 

	return true;
	}

//This function is called when a language change is being processed
void 
CPageThumbnails::Localize(HINSTANCE phLanguage) {

	oGlobalInstances.hLanguage = phLanguage;

	RenumberItems();
}

//This function iterates through the items and renumbers the listview
void 
CPageThumbnails::RenumberItems() {

	int iItemIndex;
	wchar_t szPage[50];
	wchar_t szText[50];
	LVITEM  cItem;
	CCopyDocument* oDocument;

	oDocument = oCore->GetCurrentDocument();

	if (oDocument!=NULL) {

		//Set the template for the page title
		LoadString(oGlobalInstances.hLanguage, IDS_PAGE, szPage, 50);
		wcscat_s(szPage, 50, L" %d");

		//Iterate through the items
		iItemIndex = ListView_GetItemCount(hWnd);

		while (iItemIndex>0) {

			iItemIndex--;

			cItem.mask = LVIF_PARAM;
			cItem.iItem = iItemIndex;
			ListView_GetItem(hWnd, &cItem);

			swprintf_s(szText, 50, szPage, 1+oDocument->GetPageIndex((DWORD)cItem.lParam));

			ListView_SetItemText(hWnd, iItemIndex, 0, szText);
			}
		}
	}

//This is the message handler for this class
bool CPageThumbnails::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	bool bReturn;
	LPNMHDR cNotificationHeader;
	NMLVDISPINFO* cNotificationDisplayInfo;

	//First let the base class ClistView try to handle it
	bReturn = CListview::WndProc(phWnd, message, wParam, lParam, piReturn);
	if (bReturn) return bReturn;

	//Handle the message since the base class didn't 
	switch (message) {

		case WM_NOTIFY:

			cNotificationHeader = (LPNMHDR)lParam;
			cNotificationDisplayInfo = (NMLVDISPINFO*)lParam;
			
			if (cNotificationHeader->hwndFrom == hWnd) {

				switch (cNotificationHeader->code) {

					case LVN_GETDISPINFO: //request for display info. The image is in view and we need to show an image

						if (cNotificationDisplayInfo->item.mask & LVIF_IMAGE) {

							//retrieve the image
							cNotificationDisplayInfo->item.iImage = GetImage(cNotificationDisplayInfo->item);

							bReturn = true;
							}

						break;
					}
				}

			break;
		}

	return bReturn;
	}

//The handler of the event notification
bool 
CPageThumbnails::OnEventCoreNotification() {

	bool bReturn;

	bReturn = false;

	//Doesn't work. The listview eats up all the messages. These messages are handled by the MainFrame.

	return bReturn;
	}


//This function retrieves the number of the image from the thumbnail vector
//If no thumbnail is requested yet, it will be done
int CPageThumbnails::GetImage(LVITEM &pcItem) {

	int iReturn;
	int iIndex;
	DWORD dPageId;
	LVITEM cListviewItem;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	//is something goes wrong, return the standard image
	iReturn = 0;

	//Retrieve the info from the item
	ZeroMemory(&cListviewItem, sizeof(cListviewItem));

	cListviewItem.mask = LVIF_PARAM;
	cListviewItem.iItem = pcItem.iItem;

	//EnterCriticalSection(&ThumbnailCriticalSection); 

	if (ListView_GetItem(hWnd, &cListviewItem)) {

		dPageId = (DWORD)cListviewItem.lParam;
		iIndex  = -1;

		for (long lIndex=0; lIndex<(long)vThumbnails.size() && iIndex==-1; lIndex++) {

			if (vThumbnails[lIndex].dPageId == dPageId) iIndex = lIndex;
			}
		
		if (iIndex!=-1) {

			iReturn = vThumbnails[iIndex].iImagelistImage;

			//request to create a thumbnail
			bool bNewRequest;

			bNewRequest = true;

			for (long lIndex=(long)vToBeProcessedThumbnails.size()-1; lIndex>=0 && bNewRequest; lIndex--) {

				if (vToBeProcessedThumbnails[lIndex].dPageId == vThumbnails[iIndex].dPageId) bNewRequest = false;
				}
						
			if (bNewRequest) vToBeProcessedThumbnails.push_back(vThumbnails[iIndex]);
			}
		}

	//LeaveCriticalSection(&ThumbnailCriticalSection); 

	oTrace->EndTrace(__WFUNCTION__, iReturn, eAll);

	return iReturn;
}

//this function adds an item to the thumbnail listviewer. If succesful
//the function will return true.
bool 
CPageThumbnails::AddItem(DWORD pdPageId, THUMBNAILITEM &pcItem) {

	bool bReturn;
	LVITEM cItem;
	LVFINDINFO cFindItem;
	int		   iInsertPosition;
	int		   iListViewItem;
	CCopyDocument* oDocument;
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	bReturn = false;
	iInsertPosition = (int)vThumbnails.size();
	iListViewItem = -1;

	//check if item isn't already added
	ZeroMemory(&cFindItem, sizeof(cFindItem));
	
	cFindItem.flags = LVFI_PARAM;
	cFindItem.lParam = (LPARAM)pdPageId;

	if (ListView_FindItem(hWnd, -1, &cFindItem)==-1) {

		//get the position of the page in the document
		oDocument = oCore->GetCurrentDocument();
		if (oDocument!=NULL) {

			iInsertPosition = oDocument->GetPageIndex(pdPageId);
			}

		POINT cPoint;
		bool bPositionItem;

		bPositionItem = (ListView_GetItemPosition(hWnd, iInsertPosition, &cPoint)==TRUE);

				//add item to listview					
		ZeroMemory(&cItem, sizeof(cItem));

		cItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		cItem.pszText = pcItem.pszText;
		cItem.iItem = iInsertPosition;
		cItem.iImage = I_IMAGECALLBACK;
		cItem.lParam = (LPARAM)pdPageId; //id of added page

		EnterCriticalSection(&ThumbnailCriticalSection); 

		iListViewItem = ListView_InsertItem(hWnd, &cItem);

		LeaveCriticalSection(&ThumbnailCriticalSection); 

		if (bPositionItem) ListView_SetItemPosition(hWnd, iListViewItem, cPoint.x, cPoint.y);
		
		
		//if the item is added to the listview store its info in the thumbnails
		//vector
		pcItem.iImagelistImage	= 0;
		pcItem.bNewThumbnail	= true;
		pcItem.dPageId			= pdPageId;
		pcItem.dTimeLastUpdateRequest = 0;

		bReturn = !(iListViewItem==-1);
		
		if (bReturn) vThumbnails.insert(vThumbnails.begin() + iInsertPosition, pcItem);
		}

	oTrace->EndTrace(__WFUNCTION__, bReturn, eAll);

	return bReturn;
	}

//this function updates an item in the thumbnail listviewer. If succesful
//the function will return true.
bool 
CPageThumbnails::UpdateItem(DWORD pdPageId) {

	bool bReturn;
	bool bNewRequest;
	long lIndex;
	CCopyDocument* oDocument;
	CCopyDocumentPage * oPage;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bReturn = false;
	lIndex = vThumbnails.size();

	while (lIndex>0 && !bReturn) {

		lIndex--;
		if (vThumbnails[lIndex].dPageId == pdPageId) {

			oDocument = oCore->GetCurrentDocument();
			if (oDocument!=NULL) {
				
				oPage = oDocument->GetPageByID(pdPageId);
				if (oPage!=NULL) {

					//We found the page to update the thumbnail of.

					//request to create a thumbnail, only if this request is done yet and if the image
					//hasn't been created yet.
					bNewRequest = (vThumbnails[lIndex].iImagelistImage!=0); 

					for (long lRequestIndex=(long)vToBeProcessedThumbnails.size()-1; lRequestIndex>=0 && bNewRequest; lRequestIndex--) {

						if (vToBeProcessedThumbnails[lRequestIndex].dPageId == vThumbnails[lIndex].dPageId) {
							
							//vToBeProcessedThumbnails.erase(vToBeProcessedThumbnails.begin() + lRequestIndex);
							vToBeProcessedThumbnails[lRequestIndex].dTimeLastUpdateRequest = GetTickCount();
							bNewRequest = false;
							}
						}
								
					if (bNewRequest) {
						
						vThumbnails[lIndex].bNewThumbnail = false; //do an update instead of making a new one;
						vThumbnails[lIndex].oImage = oPage->GetImage();
						vThumbnails[lIndex].dTimeLastUpdateRequest = GetTickCount();
						vToBeProcessedThumbnails.push_back(vThumbnails[lIndex]);
						}

					//the update item request has been processed
					bReturn = true;
					}
				}
			}
		}
	
	oTrace->EndTrace(__WFUNCTION__, bReturn, eAll);

	return bReturn;
	}

//this function deletes an item to the thumbnail listviewer. If succesful
//the function will return true.
bool 
CPageThumbnails::DeleteItem(DWORD pdPageId) {

	bool bReturn;
	bool bNewRequest;
	long lIndex;
	CCopyDocumentPage * oPage;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bReturn = false;
	lIndex = vThumbnails.size();

	while (lIndex>0 && !bReturn) {

		lIndex--;
		if (vThumbnails[lIndex].dPageId == pdPageId) {

			//Remove the page from the list
			LVFINDINFO cFindInfo;
			int		   iItemIndex;

			cFindInfo.flags = LVFI_PARAM;
			cFindInfo.lParam = (LPARAM)pdPageId;
			iItemIndex = ListView_FindItem(hWnd, -1, &cFindInfo);

			if (iItemIndex!=-1) ListView_DeleteItem(hWnd, iItemIndex);


			//request to create a thumbnail, only if this request is done yet and if the image
			//hasn't been created yet.
			bNewRequest = (vThumbnails[lIndex].iImagelistImage!=0); 

			for (long lRequestIndex=(long)vToBeProcessedThumbnails.size()-1; lRequestIndex>=0 && bNewRequest; lRequestIndex--) {

				if (vToBeProcessedThumbnails[lRequestIndex].dPageId == vThumbnails[lIndex].dPageId) {
					
					vToBeProcessedThumbnails[lRequestIndex].dTimeLastUpdateRequest = GetTickCount();
					bNewRequest = false;
					}
				}
								
		
			//remove the  page from the vector
			vThumbnails.erase(vThumbnails.begin() + lIndex);

			//select the new current page
			oPage = oCore->GetCurrentPage();
			if (oPage!=NULL) SelectItem(oPage->GetPageID());

			//renumber the pages
			RenumberItems();

			//the delete item request has been processed
			bReturn = true;
			}
		}
	
	oTrace->EndTrace(__WFUNCTION__, bReturn, eAll);

	return bReturn;
	}

//This function selects the give page. It returns true if the pageid
//is selected.
bool 
CPageThumbnails::SelectItem(DWORD pdPageId) {

	bool bReturn;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bReturn = false;
	
	if (pdPageId!=0) {

		lIndex = vThumbnails.size();

		while (lIndex>0 && !bReturn) {

			lIndex--;

			if (vThumbnails[lIndex].dPageId == pdPageId) {

				SetSelectedItem(lIndex);
				bReturn = true;
				}
			}
		}

	oTrace->EndTrace(__WFUNCTION__, bReturn, eAll);

	return bReturn;
}



//this function creates a thumbnail of a 96x96 size. The thumbnail will be
//stored in the given imagelist. If succesful it will return the index
//of the thumbnail in the imagelist.
int 
CPageThumbnails::CreateThumbnail(HGLOBAL phImage, HIMAGELIST phImagelist, int piImagelistIndex) {

	int iReturn;
	Bitmap* OriginalBitmap;
	LPBITMAPINFO pBitmapInfo;
	void* pBitmapData;
	

	iReturn = -1;

	pBitmapInfo = (LPBITMAPINFO)GlobalLock(phImage);

	pBitmapData = ((BYTE*)pBitmapInfo->bmiColors + pBitmapInfo->bmiHeader.biClrUsed*sizeof(RGBQUAD));

	OriginalBitmap = new Bitmap(pBitmapInfo, pBitmapData);

	iReturn = CreateThumbnail(OriginalBitmap, phImagelist, piImagelistIndex);

	delete OriginalBitmap;

	GlobalUnlock(phImage);

	return iReturn;
}

int CPageThumbnails::CreateThumbnail(LPTSTR pszFile, HIMAGELIST phImagelist, int piImagelistIndex) {

	int iReturn;
	Bitmap* OriginalBitmap;

	iReturn = -1;

	OriginalBitmap = new Bitmap(pszFile);

	iReturn = CreateThumbnail(OriginalBitmap, phImagelist, piImagelistIndex);

	delete OriginalBitmap;

	return iReturn;
}
	
int CPageThumbnails::CreateThumbnail(Bitmap* poBitmap, HIMAGELIST phImagelist, int piImagelistIndex) {

	int iReturn;
	HBITMAP hBitmap;
	Bitmap* oScaledBitmap;
	SolidBrush* oWhiteBrush;
	Color* oColor;
	Pen* oPen;
	Rect cThumbRect;
	Rect cScaledRect;
	double dScaleFactor;
	long lDeltaX;
	long lDeltaY;

	
	cThumbRect = Rect(0, 0, ThumbnailSizeX, ThumbnailSizeY);
	
	oScaledBitmap = new Bitmap(ThumbnailSizeX, ThumbnailSizeY, PixelFormat32bppARGB);

	// Create a brush and a pen
	oColor = new Color(0xFF, defBackgroundR, defBackgroundG, defBackgroundB);
	oWhiteBrush = new SolidBrush(*oColor);
	oPen = new Pen(oColor->Black);

	// Calculate the scaling rectangle.
	dScaleFactor = (double)poBitmap->GetWidth() / (double)poBitmap->GetHeight();

	lDeltaX = lDeltaY = 0; 

	if (dScaleFactor>1) {

		lDeltaY = (long)(ThumbnailSizeY - (ThumbnailSizeY / dScaleFactor)) / 2;
		}
	else {

		lDeltaX = (long)(ThumbnailSizeX - (ThumbnailSizeX * dScaleFactor)) / 2;
		}

	if (poBitmap->GetWidth() < ThumbnailSizeX &&
		poBitmap->GetHeight() < ThumbnailSizeY) {
		
		//the image is smaller than the thumbnail its size. Just center it.

		lDeltaX = (ThumbnailSizeX - poBitmap->GetWidth()) / 2;
		lDeltaY = (ThumbnailSizeY - poBitmap->GetHeight()) / 2;
		}

	cScaledRect = Rect(lDeltaX, lDeltaY, ThumbnailSizeX - lDeltaX * 2, ThumbnailSizeY - lDeltaY * 2);


	// Do the scaling
	Graphics* oCanvas = Graphics::FromImage(oScaledBitmap);
	
	//Draw the background
	double dTan = ((double)cThumbRect.Height / (double)cThumbRect.Width);
	double dTanResult = atan((double)dTan);
	double dDegree = dTanResult * 270 / 3.14f;
	LinearGradientBrush* oBackgroundBrush = new LinearGradientBrush( 
							cThumbRect, 
							Color(255, defBackgroundR, defBackgroundG, defBackgroundB), 
							Color(255, (BYTE)(defBackgroundR * 0.6f), (BYTE)(defBackgroundG * 0.6f), (BYTE)(defBackgroundB * 0.6f)), 
							(REAL)dDegree, 
							false); 

	oCanvas->FillRectangle(oBackgroundBrush, cThumbRect);

	//oCanvas->FillRectangle(oWhiteBrush, cThumbRect);
	
	oCanvas->SetInterpolationMode(InterpolationModeBicubic);
	oCanvas->DrawImage(poBitmap, cScaledRect);

	// Add a border to the bitmap
	oCanvas->DrawRectangle(oPen, Rect(0, 0, ThumbnailSizeX-1, ThumbnailSizeY-1));

	// get an HBITMAP, with this we can add the thumbnail to the imagelist
	oScaledBitmap->GetHBITMAP(RGB(defBackgroundR,defBackgroundG,defBackgroundB),&hBitmap) ;

	if (piImagelistIndex==-1) {

		// add the thumbnail to the imagelist
		iReturn = ImageList_Add(phImagelist, hBitmap, hBitmap);
		}
	else {

		//replace the thumbnail in the imagelist
		iReturn = ImageList_Replace(phImagelist, piImagelistIndex, hBitmap, hBitmap);
		if (iReturn!=0) iReturn = piImagelistIndex;
		}

	//clean up
	delete oColor;
	delete oWhiteBrush;
	delete oPen;
	delete oCanvas;
	delete oScaledBitmap;

	return iReturn;
}

int 
CPageThumbnails::CreateThumbnail(CBitmap* poBitmap, HIMAGELIST phImagelist, int piImagelistIndex) {

	int iReturn;
	Bitmap* oThumbnail;
	HBITMAP hBitmap;

	oThumbnail = poBitmap->GetThumbnail();
	
	// get an HBITMAP, with this we can add the thumbnail to the imagelist
	oThumbnail->GetHBITMAP(RGB(defBackgroundR,defBackgroundG,defBackgroundB),&hBitmap) ;

	// add the thumbnail to the imagelist
	if (piImagelistIndex==-1) {

		// add the thumbnail to the imagelist
		iReturn = ImageList_Add(phImagelist, hBitmap, hBitmap);
		}
	else {

		//replace the thumbnail in the imagelist
		iReturn = ImageList_Replace(phImagelist, piImagelistIndex, hBitmap, hBitmap);
		if (iReturn!=0) iReturn = piImagelistIndex;
		}

	poBitmap->UpdateThumbnail();

	return iReturn;
	}

int 
CPageThumbnails::CreateThumbnail(LPTSTR pszFile, int piImagelistIndex) {

	//this is just a stub function. It is only used by the thread so it
	//doesn't have to have access to the handle of the imagelist.
	int iReturn;

	iReturn = CreateThumbnail(pszFile, hNormalImagelist, piImagelistIndex);

	return iReturn;
	}

int 
CPageThumbnails::CreateThumbnail(HGLOBAL phImage, int piImagelistIndex) {

	//this is just a stub function. It is only used by the thread so it
	//doesn't have to have access to the handle of the imagelist.
	int iReturn;

	iReturn = CreateThumbnail(phImage, hNormalImagelist, piImagelistIndex);

	return iReturn;
	}

int 
CPageThumbnails::CreateThumbnail(CBitmap* poBitmap, int piImagelistIndex) {

	//this is just a stub function. It is only used by the thread so it
	//doesn't have to have access to the handle of the imagelist.
	int iReturn;

	iReturn = CreateThumbnail(poBitmap, hNormalImagelist, piImagelistIndex);

	return iReturn;
	}


//This is the main function of the thumbnail creation thread. This function
//polls continuely if there are thumbnails to be created

DWORD 
WINAPI ThreadCreateThumbnail(LPVOID lpParameter) {

	CPageThumbnails* oThumbnails;
	int iImageIndex;
	int iItemIndex;
	int iImagelistImage;
	bool bVisible;
	RECT cImageRect;
	RECT cClientRect;
	THUMBNAILITEM cThumbnailItem;
	long lLoopIndex;
	

	oThumbnails = (CPageThumbnails*)lpParameter;

	while (!oThumbnails->bEndThread) {

		Sleep(500);

	    EnterCriticalSection(&oThumbnails->ThumbnailCriticalSection); 

		lLoopIndex = oThumbnails->vToBeProcessedThumbnails.size();

		//loop through the thumbnails to be created
		while (lLoopIndex>0 && !oThumbnails->bEndThread) {

			lLoopIndex--;
			
			if (oThumbnails->vToBeProcessedThumbnails[0].dTimeLastUpdateRequest > GetTickCount() - 5000) {

				//This request is not new, wait with processing it. For now we will place it at the back of
				//the vector and let others go first.
				cThumbnailItem  = oThumbnails->vToBeProcessedThumbnails[0];
				oThumbnails->vToBeProcessedThumbnails.erase(oThumbnails->vToBeProcessedThumbnails.begin());
				oThumbnails->vToBeProcessedThumbnails.push_back(cThumbnailItem);
				}
			else {
			
				LVFINDINFO cFindInfo;

				cFindInfo.flags = LVFI_PARAM;
				cFindInfo.lParam = (LPARAM)oThumbnails->vToBeProcessedThumbnails[0].dPageId;
				iItemIndex = ListView_FindItem(oThumbnails->hWnd, -1, &cFindInfo);
				
				ListView_GetItemRect(oThumbnails->hWnd, iItemIndex, &cImageRect, LVIR_ICON);
				GetClientRect(oThumbnails->hWnd, &cClientRect);

				LeaveCriticalSection(&oThumbnails->ThumbnailCriticalSection); 
				
				//check if the item is still visible
				bVisible = false;

				Rect Item(cImageRect.left, cImageRect.top, cImageRect.right - cImageRect.left, cImageRect.bottom-cImageRect.top);
				Rect Client(cClientRect.left, cClientRect.top, cClientRect.right, cClientRect.bottom);

				bVisible = (Client.IntersectsWith(Item)==TRUE);

				if (bVisible || !oThumbnails->vToBeProcessedThumbnails[0].bNewThumbnail) {
				
					iImagelistImage = oThumbnails->vToBeProcessedThumbnails[0].iImagelistImage;
					if (iImagelistImage==0) iImagelistImage=-1;

					//create the thumbnail
					if (oThumbnails->vToBeProcessedThumbnails[0].dPageId!=0) {
						
						//Thumbnail of the given page
						CCopyDocumentPage* oPage;

						oPage = oThumbnails->oCore->oDocuments->GetPageByID(oThumbnails->vToBeProcessedThumbnails[0].dPageId);
						if (oPage!=NULL) {
							iImageIndex = oThumbnails->CreateThumbnail(oPage->GetImage(), iImagelistImage);
							}
						}
					else {
						//Thumbnail of the given image
						if (oThumbnails->vToBeProcessedThumbnails[0].oImage!=NULL) {

							iImageIndex = oThumbnails->CreateThumbnail(oThumbnails->vToBeProcessedThumbnails[0].oImage, iImagelistImage);
							}
						else {

							//Thumbnail of the given file
							iImageIndex = oThumbnails->CreateThumbnail(oThumbnails->vToBeProcessedThumbnails[0].pszFile, iImagelistImage);
							}
						}
					
					//attach the newly created thumbnail to the listview item
					EnterCriticalSection(&oThumbnails->ThumbnailCriticalSection); 

					oThumbnails->SetListviewItemImage(oThumbnails->vToBeProcessedThumbnails[0].dPageId, iImageIndex);

					LeaveCriticalSection(&oThumbnails->ThumbnailCriticalSection); 
					}

				
				EnterCriticalSection(&oThumbnails->ThumbnailCriticalSection); 

				//erase the request
				oThumbnails->vToBeProcessedThumbnails.erase(oThumbnails->vToBeProcessedThumbnails.begin());
				}
			}

	    LeaveCriticalSection(&oThumbnails->ThumbnailCriticalSection); 
		}

	return 0;
	}

//This function is called when the selection in the listview has
//changed. 
void 
CPageThumbnails::OnSelectionChanged(int piIndex) {

	LVITEM cItem;

	cItem.mask = LVIF_PARAM;
	cItem.iItem = piIndex;
	ListView_GetItem(hWnd, &cItem);

	oCore->SetCurrentPageByID((DWORD)cItem.lParam);
	//oCore->SetCurrentPage(piIndex);	
	}

//This function sets the image of the listviewitem
void 
CPageThumbnails::SetListviewItemImage(DWORD pdPageId, int piImagelistIndex) {

	LVITEM cItem;
	LVFINDINFO cFindInfo;
	int iItemIndex;

	ZeroMemory(&cFindInfo, sizeof(cFindInfo));
	cFindInfo.flags = LVFI_PARAM;
	cFindInfo.lParam = (LPARAM)pdPageId;
	iItemIndex = ListView_FindItem(hWnd, -1, &cFindInfo);

	//set the image of the listview item
	ZeroMemory(&cItem, sizeof(cItem));
	cItem.mask = LVIF_IMAGE;
	cItem.iItem = iItemIndex;
	cItem.iImage = piImagelistIndex;

	ListView_SetItem(hWnd, &cItem);
	ListView_RedrawItems(hWnd, iItemIndex, iItemIndex);

	//set the image of the listview item in the thumbnail vector
	for (long lIndex=0; lIndex<(long)vThumbnails.size(); lIndex++) {

		if (vThumbnails[lIndex].dPageId == pdPageId) {

			vThumbnails[lIndex].iImagelistImage = piImagelistIndex;
			}
		}
	}

//Show the context menu
bool 
CPageThumbnails::DoContextMenu(int piItem, HMENU hContextMenu) {

	wchar_t sMenuItem[100];
	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;

	//The contents of the menu is dependent of the existence of a selection and where the cursor is hovering
	if (ListView_GetSelectedCount(hWnd)==1) {

		LoadString(oGlobalInstances.hLanguage, IDS_CUTPAGE, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_DOCUMENT_CUTPAGE, sMenuItem);

		LoadString(oGlobalInstances.hLanguage, IDS_COPYPAGE, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_DOCUMENT_COPYPAGE, sMenuItem);

		LoadString(oGlobalInstances.hLanguage, IDS_PASTENEWPAGE, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_EDIT_PASTEASNEWPAGEANDINSERT, sMenuItem);

		AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);
		
		LoadString(oGlobalInstances.hLanguage, IDS_ACQUIRENEWPAGE, sMenuItem, 99);
		AppendMenu(hContextMenu, MF_STRING, ID_DOCUMENT_ACQUIRENEWPAGEANDINSERT, sMenuItem);

		AppendMenu(hContextMenu, MF_MENUBREAK, NULL, NULL);
		}

	LoadString(oGlobalInstances.hLanguage, IDS_DELETEPAGE, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_DOCUMENT_DELETEPAGE, sMenuItem);

	LoadString(oGlobalInstances.hLanguage, IDS_SPLITONTHISPAGE, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, ID_DOCUMENT_SPLITDOCUMENT, sMenuItem);

	oTrace->EndTrace(__WFUNCTION__);
	
	return bResult;
	}

//This function is called when the dragging has ended
void 
CPageThumbnails::RearrangeAfterDrag(int piNewItemIndex) {

	int iPosition;
	int iItem;
	CCopyDocumentPage* oPage;
	CCopyDocument*     oDocument;


	LVITEM cItem;

	//Get the position of the item in the document structure
	oDocument = oCore->GetCurrentDocument();
	oPage     = oCore->GetCurrentPage();
	
	if (oDocument!=NULL) {

		if (piNewItemIndex>=oDocument->GetPageCount()) {

			//move the selected documents to the end of the document
			iPosition = oDocument->GetPageCount();
			}
		else {

			//move the selected documents to the position of the given page
			cItem.mask = LVIF_PARAM;
			cItem.iItem = piNewItemIndex;
			ListView_GetItem(hWnd, &cItem);

			iPosition = oDocument->GetPageIndex(cItem.lParam);
			}

		
		//Move all the selected items to the next start point
		iItem = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
		
		while (iItem!=-1 && iPosition!=-1) {

			cItem.mask = LVIF_PARAM;
			cItem.iItem = iItem;

			if (ListView_GetItem(hWnd, &cItem)) {

				iPosition = oDocument->DoMovePage((DWORD)cItem.lParam, iPosition); 
				}

			iItem = ListView_GetNextItem(hWnd, iItem, LVNI_SELECTED); 
			}
	
		ListView_SortItems(hWnd, &PageCompare, (LPARAM)oDocument);
		
		RenumberItems();
		}

	//Reset the current page, its index can be changed so we need to reset it so the core
	//can update its indexes too.
	if (oPage!=NULL) oCore->SetCurrentPageByID(oPage->GetPageID());
	}

//This function rearranges the items in the listview based on their
//index order in the document structure
void 
CPageThumbnails::Rearrange() {

	CCopyDocument*     oDocument;

	//Get the position of the item in the document structure
	oDocument = oCore->GetCurrentDocument();

	if (oDocument!=NULL) {
		ListView_SortItems(hWnd, &PageCompare, (LPARAM)oDocument);
		
		RenumberItems();
		}
}



//This is the function to compare two pages based on their index
//value in the document structure.
int 
CALLBACK PageCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {

	CCopyDocument* oDocument;
	CCopyDocumentPage* oPage1; 
	CCopyDocumentPage* oPage2;
	int iIndex1, iIndex2;
	int iResult;

	oDocument = (CCopyDocument*)lParamSort;
	iResult   = 0;

	if (oDocument!=NULL) {

		oPage1 = oDocument->GetPageByID((DWORD)lParam1);
		oPage2 = oDocument->GetPageByID((DWORD)lParam2);

		if (oPage1 && oPage2) {

			iIndex1 = oDocument->GetPageIndex(oPage1);
			iIndex2 = oDocument->GetPageIndex(oPage2);

			iResult = iIndex1 - iIndex2;
			}
		}
	
	return iResult;
	}






