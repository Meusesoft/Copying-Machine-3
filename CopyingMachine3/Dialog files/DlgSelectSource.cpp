#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgSelectSource::CDlgSelectSource(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {

	}

CDlgSelectSource::~CDlgSelectSource(void)
{
	oScanningDevices.clear();

	ImageList_Destroy(hImagelist);
}

//This function is called when the dialog is initialised
void 
CDlgSelectSource::OnInitDialog() {

	CCopyingMachineCore* oCMCore;
	CScanDevice* poDevice;
	HBITMAP hBitmap;

	oCMCore = (CCopyingMachineCore*)oCore;

	//set the font of the caption
	SetDlgItemFont(IDC_CAPTION, hCaptionFont);

	//create an imagelist and add 2 images
	hImagelist = ImageList_Create(
			16, 16,					//size of images
			ILC_MASK | ILC_COLOR32,	//flags
			16,						//initial size
			2);	

	hBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ICONSCANNER));			
	ImageList_AddMasked(hImagelist, (HBITMAP)hBitmap, (COLORREF)RGB(255, 0 ,255));

	hBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ICONSCANNERTWAIN));			
	ImageList_AddMasked(hImagelist, (HBITMAP)hBitmap, (COLORREF)RGB(255, 0 ,255));

	//get the scanners
	poDevice = oCMCore->oScanCore->GetFirstDevice();

	while (poDevice!=NULL) {

		oScanningDevices.push_back(poDevice);
		poDevice = oCMCore->oScanCore->GetNextDevice();
		}

	//Add the scanners to the listview
	HWND hScannerList;
	LVCOLUMN cColumn;
	LVITEM cItem;
	WCHAR cItemText[MAX_PATH];
	CScanDevice* oDefaultScanner;

	oDefaultScanner = oCMCore->oScanSettings->GetScanner();

	if (oDefaultScanner==NULL) {
		
		oDefaultScanner = oScanningDevices[0];	
		}

	hScannerList = GetDlgItem(hDlg, IDC_SCANNERLIST);

	ListView_SetImageList(hScannerList, hImagelist, LVSIL_STATE);

	DWORD dExStyle = ListView_GetExtendedListViewStyle(hScannerList);
	ListView_SetExtendedListViewStyle(hScannerList, dExStyle | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	cColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	cColumn.cx = 220;
	cColumn.pszText = L"Device";
	ListView_InsertColumn(hScannerList, 0, &cColumn);

	cColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	cColumn.cx = 80;
	cColumn.pszText = L"Type";
	ListView_InsertColumn(hScannerList, 1, &cColumn);

	for (long lIndex=0; lIndex<(long)oScanningDevices.size(); lIndex++) {

		wsprintf(cItemText, L"%s", oScanningDevices[lIndex]->cScanningDevice.c_str());
		
		SecureZeroMemory(&cItem, sizeof(cItem));
		cItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
		cItem.pszText = cItemText;
		cItem.lParam = lIndex;
		cItem.iItem = lIndex;
		cItem.state = INDEXTOSTATEIMAGEMASK(oScanningDevices[lIndex]->cInterface == eWIA ? 1 : 2);

		ListView_InsertItem(hScannerList, &cItem);

		ListView_SetItemText(hScannerList, lIndex, 1, oScanningDevices[lIndex]->cInterface == eWIA ? L"WIA" : L"TWAIN");

		if (oDefaultScanner == oScanningDevices[lIndex]) {

			ListView_SetItemState(hScannerList, lIndex, LVIS_SELECTED, LVIS_SELECTED);
			}
		}
	}

BOOL 
CDlgSelectSource::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_CAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}

			break;

		};	


	return bReturn;
	}

BOOL 
CDlgSelectSource::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {


	return FALSE;
	}

//This function handles the OK event
void
CDlgSelectSource::OnCloseOk() {

	//change the default scanner
	HWND hScannerList;
	int  iItemCount;
	bool bFound;
	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;
	
	hScannerList = GetDlgItem(hDlg, IDC_SCANNERLIST);

	bFound = false;
	iItemCount = ListView_GetItemCount(hScannerList);

	while (iItemCount>0 && !bFound) {

		iItemCount--;

		if (ListView_GetItemState(hScannerList, iItemCount, LVIS_SELECTED)!=0) {

			bFound = true;
			oCMCore->oScanSettings->SetScanner(oScanningDevices[iItemCount]);
			}
		}
	}
