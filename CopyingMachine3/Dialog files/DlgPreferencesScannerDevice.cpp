#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPreferencesScannerDevice::CDlgPreferencesScannerDevice(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {

	bInitialized = false;
	}

CDlgPreferencesScannerDevice::~CDlgPreferencesScannerDevice(void)
{
	oScanningDevices.clear();

	ImageList_Destroy(hImagelist);
}

//This function is called when the dialog is initialised
void 
CDlgPreferencesScannerDevice::OnInitDialog() {

	CCopyingMachineCore* oCMCore;
	CScanDevice* poDevice;
	Bitmap* oBitmap;

	oCMCore = (CCopyingMachineCore*)oCore;

	//set the font of the caption
	SetDlgItemFont(IDC_CAPTION, hCaptionFont);

	//set the checkbox
	SendDlgItemMessage(hDlg, IDC_SHOWINTERFACE, BM_SETCHECK, oCMCore->oScanSettings->GetBool(eAlwaysShowInterface) ? BST_CHECKED : BST_UNCHECKED, 0);

	//create an imagelist and add 2 images
	hImagelist = ImageList_Create(
			16, 16,					//size of images
			ILC_MASK | ILC_COLOR32,	//flags
			16,						//initial size
			2);	

	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLSCANNER", L"PNG", hInstance);
	CBitmap::AddToImagelist(oBitmap, hImagelist);
	delete oBitmap;
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLSCANNERTWAIN", L"PNG", hInstance);
	CBitmap::AddToImagelist(oBitmap, hImagelist);
	delete oBitmap;
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLFILE", L"PNG", hInstance);
	CBitmap::AddToImagelist(oBitmap, hImagelist);
	delete oBitmap;

	WCHAR cFile[50];
	WCHAR cDevice[50];
	WCHAR cType[50];

	LoadString(oGlobalInstances.hLanguage, IDS_FILE,	cFile, 50);
	LoadString(oGlobalInstances.hLanguage, IDS_DEVICES, cDevice, 50);
	LoadString(oGlobalInstances.hLanguage, IDS_TYPE,	cType, 50);

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
	cColumn.pszText = cDevice;
	ListView_InsertColumn(hScannerList, 0, &cColumn);

	cColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	cColumn.cx = 80;
	cColumn.pszText = cType;
	ListView_InsertColumn(hScannerList, 1, &cColumn);

	for (long lIndex=0; lIndex<(long)oScanningDevices.size(); lIndex++) {

		wsprintf(cItemText, L"%s", oScanningDevices[lIndex]->cScanningDevice.c_str());
		
		SecureZeroMemory(&cItem, sizeof(cItem));
		cItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
		cItem.pszText = cItemText;
		cItem.lParam = lIndex;
		cItem.iItem = lIndex;

		switch (oScanningDevices[lIndex]->cInterface) {

			case eInputFile:	cItem.state = INDEXTOSTATEIMAGEMASK(3); break;
			case eTWAIN:		cItem.state = INDEXTOSTATEIMAGEMASK(2); break;
			case eWIA:			cItem.state = INDEXTOSTATEIMAGEMASK(1); break;
			}

		ListView_InsertItem(hScannerList, &cItem);

		switch (oScanningDevices[lIndex]->cInterface) {

			case eInputFile:	ListView_SetItemText(hScannerList, lIndex, 1, cFile); break;
			case eTWAIN:		ListView_SetItemText(hScannerList, lIndex, 1, L"TWAIN"); break;
			case eWIA:			ListView_SetItemText(hScannerList, lIndex, 1, L"WIA"); break;
			}

		if (oDefaultScanner->cScanningDeviceID == oScanningDevices[lIndex]->cScanningDeviceID) {

			ListView_SetItemState(hScannerList, lIndex, LVIS_SELECTED, LVIS_SELECTED);
			}
		}

	bInitialized = true;
	}

BOOL 
CDlgPreferencesScannerDevice::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_CAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}

			break;

		case WM_NOTIFY:

			LPNMHDR pHeader;

			pHeader = (LPNMHDR)lParam;

			if (pHeader->code == LVN_ITEMCHANGED && bInitialized) {

				SendMessage(hParent, WM_ENABLEAPPLY, 0, 0);
				}

			break;
		};	

	return bReturn;
	}

BOOL 
CDlgPreferencesScannerDevice::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	switch (iCommand) {			
		
		case IDC_SHOWINTERFACE: 

			SendMessage(hParent, WM_ENABLEAPPLY, 0, 0);
			break;
			}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesScannerDevice::OnCloseOk() {

	}

//This function handles the localize action
void 
CDlgPreferencesScannerDevice::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	LocalizeControl(IDC_TOPIC1, IDS_DEVICES);
	LocalizeControl(IDC_SELECTSOURCE, IDS_SELECTSOURCE);
	LocalizeControl(IDC_SHOWINTERFACE, IDS_SHOWINTERFACE);
}

//This function handles the Apply event
void 
CDlgPreferencesScannerDevice::OnApply() {

	//change the default scanner
	HWND hScannerList;
	int  iItemCount;
	bool bFound;
	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;
	
	oCMCore->oScanSettings->SetBool(eAlwaysShowInterface,	(SendDlgItemMessage(hDlg, IDC_SHOWINTERFACE,	BM_GETCHECK, 0, 0)==BST_CHECKED));

	hScannerList = GetDlgItem(hDlg, IDC_SCANNERLIST);

	bFound = false;
	iItemCount = ListView_GetItemCount(hScannerList);

	while (iItemCount>0 && !bFound) {

		iItemCount--;

		if (ListView_GetItemState(hScannerList, iItemCount, LVIS_SELECTED)!=0) {

			bFound = true;
			oCMCore->oScanSettings->SetScanner(oScanningDevices[iItemCount]);
			oCMCore->oScanCore->SetCurrentScanner(oScanningDevices[iItemCount]);
			}
		}
	}