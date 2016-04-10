#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPreferencesPrinterDevice::CDlgPreferencesPrinterDevice(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {

	bInitialized = false;
	}

CDlgPreferencesPrinterDevice::~CDlgPreferencesPrinterDevice(void)
{
	ImageList_Destroy(hImagelist);
}

//This function is called when the dialog is initialised
void 
CDlgPreferencesPrinterDevice::OnInitDialog() {

	CCopyingMachineCore* oCMCore;
	Bitmap* oBitmap;

	oCMCore = (CCopyingMachineCore*)oCore;

	//set the font of the caption
	SetDlgItemFont(IDC_CAPTION, hCaptionFont);

	//set the checkbox
	SendDlgItemMessage(hDlg, IDC_SHOWINTERFACE, BM_SETCHECK, oCMCore->oPrintSettings->GetBool(ePrintAlwaysShowInterface) ? BST_CHECKED : BST_UNCHECKED, 0);

	//create an imagelist and add 2 images
	hImagelist = ImageList_Create(
			16, 16,					//size of images
			ILC_MASK | ILC_COLOR32,	//flags
			16,						//initial size
			2);	

	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLPRINT", L"PNG", hInstance);
	CBitmap::AddToImagelist(oBitmap, hImagelist);
	delete oBitmap;
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLFILE", L"PNG", hInstance);
	CBitmap::AddToImagelist(oBitmap, hImagelist);
	delete oBitmap;
	oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLEMAILOPEN", L"PNG", hInstance);
	CBitmap::AddToImagelist(oBitmap, hImagelist);
	delete oBitmap;

	//Add the printers to the listview
	HWND hDeviceList;
	LVCOLUMN cColumn;
	LVITEM cItem;
	WCHAR cItemText[MAX_PATH];
	sOutputDevice sDefaultDevice;
	sOutputDevice sDevice;

	WCHAR cFile[50];
	WCHAR cEmail[50];
	WCHAR cPrinter[50];
	WCHAR cDevice[50];
	WCHAR cType[50];
	WCHAR* cSubItemText;

	LoadString(oGlobalInstances.hLanguage, IDS_FILE,	cFile, 50);
	LoadString(oGlobalInstances.hLanguage, IDS_EMAIL,	cEmail, 50);
	LoadString(oGlobalInstances.hLanguage, IDS_PRINTER, cPrinter, 50);
	LoadString(oGlobalInstances.hLanguage, IDS_DEVICES, cDevice, 50);
	LoadString(oGlobalInstances.hLanguage, IDS_TYPE,	cType, 50);

	hDeviceList = GetDlgItem(hDlg, IDC_OUTPUTLIST);

	ListView_SetImageList(hDeviceList, hImagelist, LVSIL_STATE);

	DWORD dExStyle = ListView_GetExtendedListViewStyle(hDeviceList);
	ListView_SetExtendedListViewStyle(hDeviceList, dExStyle | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	cColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	cColumn.cx = 220;
	cColumn.pszText = cDevice;
	ListView_InsertColumn(hDeviceList, 0, &cColumn);

	cColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	cColumn.cx = 80;
	cColumn.pszText = cType;
	ListView_InsertColumn(hDeviceList, 1, &cColumn);

	oCMCore->oPrintSettings->GetDevice(sDefaultDevice);

	for (long lIndex=0; lIndex<oCMCore->oPrintCore->GetOutputDeviceCount(); lIndex++) {

		oCMCore->oPrintCore->GetOutputDevice(lIndex, sDevice);

		wsprintf(cItemText, L"%s", sDevice.sDescription.c_str());
		
		SecureZeroMemory(&cItem, sizeof(cItem));
		cItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
		cItem.pszText = cItemText;
		cItem.lParam = lIndex;
		cItem.iItem = lIndex;
		
		switch (sDevice.cType) {

			case eOutputFile:
				cItem.state = INDEXTOSTATEIMAGEMASK(2);
				cSubItemText = cFile;
				break;

			case eOutputMail:
				cItem.state = INDEXTOSTATEIMAGEMASK(3);
				cSubItemText = cEmail;
				break;

			default:
				cItem.state = INDEXTOSTATEIMAGEMASK(1);
				cSubItemText = cPrinter;
				break;
			}
		
		ListView_InsertItem(hDeviceList, &cItem);

		ListView_SetItemText(hDeviceList, lIndex, 1, cSubItemText);

		if (sDevice.sDescription == sDefaultDevice.sDescription) {

			ListView_SetItemState(hDeviceList, lIndex, LVIS_SELECTED, LVIS_SELECTED);
			}
		}

	bInitialized = true;
	}

BOOL 
CDlgPreferencesPrinterDevice::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

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
CDlgPreferencesPrinterDevice::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	switch (iCommand) {			
		
		case IDC_SHOWINTERFACE: 

			SendMessage(hParent, WM_ENABLEAPPLY, 0, 0);
			break;
			}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesPrinterDevice::OnCloseOk() {

	}

//This function handles the localize action
void 
CDlgPreferencesPrinterDevice::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	LocalizeControl(IDC_TOPIC1, IDS_DEVICES);
	LocalizeControl(IDC_SELECTDEVICE, IDS_SELECTOUTPUTDEVICE);
	LocalizeControl(IDC_SHOWINTERFACE, IDS_SHOWINTERFACE);
}

//This function handles the Apply event
void 
CDlgPreferencesPrinterDevice::OnApply() {

	//change the default scanner
	HWND hDevicesList;
	int  iItemCount;
	bool bFound;
	CCopyingMachineCore* oCMCore;
	sOutputDevice sDevice;

	oCMCore = (CCopyingMachineCore*)oCore;
	
	oCMCore->oPrintSettings->SetBool(ePrintAlwaysShowInterface,	(SendDlgItemMessage(hDlg, IDC_SHOWINTERFACE,	BM_GETCHECK, 0, 0)==BST_CHECKED));

	hDevicesList = GetDlgItem(hDlg, IDC_OUTPUTLIST);

	bFound = false;
	iItemCount = ListView_GetItemCount(hDevicesList);

	while (iItemCount>0 && !bFound) {

		iItemCount--;

		if (ListView_GetItemState(hDevicesList, iItemCount, LVIS_SELECTED)!=0) {

			bFound = true;
			oCMCore->oPrintCore->GetOutputDevice(iItemCount, sDevice);
			oCMCore->oPrintSettings->SetDevice(sDevice);
			}
		}
}