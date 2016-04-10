#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"

CDlgPreferences::CDlgPreferences(HINSTANCE phInstance, HWND phParent, ePreferences pcPreferences, sGlobalInstances poGlobalInstances) :
CDialogTemplate(phInstance, phParent, (pcPreferences==eAllPreferences || pcPreferences==ePrinter || pcPreferences==eScanner) ? IDD_PREFERENCESSINGLECONTAINER : IDD_PREFERENCESCONTAINER, poGlobalInstances) {

	//Initialise variables
	hImagelist = NULL;	

	oPreferencesGeneral = NULL;	
	oPreferencesScannerImage = NULL;	
	oPreferencesScannerPage = NULL;	
	oPreferencesScannerDevice = NULL;	
	oPreferencesScannerAdvanced = NULL;
	oPreferencesTrace = NULL;
	oPreferencesPrinterCopiesAlignment = NULL;
	oPreferencesPrinterImageSize = NULL;
	oPreferencesPrinterDevice = NULL;	
	oPreferencesAutoSave = NULL;	
	cPreferences = pcPreferences;
}

CDlgPreferences::~CDlgPreferences(void)
{
	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;
	oCmCore->oNotifications->UnsubscribeAll(hDlg);

	//delete all pages
	for (long lIndex=0; lIndex<(long)oPreferencePages.size(); lIndex++) {
		
		delete oPreferencePages[lIndex];
		}
	oPreferencePages.clear();

	//destroy the imagelist
	if (hImagelist!=NULL) ImageList_Destroy(hImagelist);	
}

//This function is called when the dialog is initialised
void 
CDlgPreferences::OnInitDialog() {

	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;
	oCmCore->oNotifications->Subscribe(hDlg, eNotificationLanguageChanged);

	if (cPreferences==eAllPreferences || cPreferences==eGeneral) {
		oPreferencesGeneral = new CDlgPreferencesGeneral(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESGENERAL, oGlobalInstances);				  
		oPreferencesGeneral->Create();
		oPreferencePages.push_back(oPreferencesGeneral);
		}
	if (cPreferences==eAllPreferences || cPreferences==eAutoSave) {
		oPreferencesAutoSave = new CDlgPreferencesAutoSave(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESAUTOSAVE, oGlobalInstances);				  
		oPreferencesAutoSave->Create();
		oPreferencePages.push_back(oPreferencesAutoSave);
		}
	if (cPreferences==eAllPreferences || cPreferences==eScanner || cPreferences==eScannerImage) {
		oPreferencesScannerImage = new CDlgPreferencesScannerImage(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESSCANNERIMAGE, oGlobalInstances);				  
		oPreferencesScannerImage->Create();
		oPreferencePages.push_back(oPreferencesScannerImage);
		}
	if (cPreferences==eAllPreferences || cPreferences==eScanner || cPreferences==eScannerPage) {
		oPreferencesScannerPage = new CDlgPreferencesScannerPage(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESSCANNERPAGE, oGlobalInstances);				  
		oPreferencesScannerPage->Create();
		oPreferencePages.push_back(oPreferencesScannerPage);
		}
	if (cPreferences==eAllPreferences || cPreferences==eScanner || cPreferences==eScannerDevice) {
		oPreferencesScannerDevice = new CDlgPreferencesScannerDevice(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESSCANNERDEVICE, oGlobalInstances);				  
		oPreferencesScannerDevice->Create();
		oPreferencePages.push_back(oPreferencesScannerDevice);
		}
	if (cPreferences==eAllPreferences || cPreferences==eScanner || cPreferences==eScannerAdvanced) {
		oPreferencesScannerAdvanced = new CDlgPreferencesScannerAdvanced(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESSCANNERADVANCED, oGlobalInstances);				  
		oPreferencesScannerAdvanced->Create();
		oPreferencePages.push_back(oPreferencesScannerAdvanced);
		}
	if (cPreferences==eAllPreferences || cPreferences==eTrace) {
		oPreferencesTrace = new CDlgPreferencesTrace(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESTRACE, oGlobalInstances);				  
		oPreferencesTrace->Create();
		oPreferencePages.push_back(oPreferencesTrace);
		}
	if (cPreferences==eAllPreferences || cPreferences==ePrinter || cPreferences==ePrinterCopiesAlignment) {
		oPreferencesPrinterCopiesAlignment = new CDlgPreferencesPrinterCopiesAlignment(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESPRINTERALIGNMENT, oGlobalInstances);				  
		oPreferencesPrinterCopiesAlignment->Create();
		oPreferencePages.push_back(oPreferencesPrinterCopiesAlignment);
		}
	if (cPreferences==eAllPreferences || cPreferences==ePrinter || cPreferences==ePrinterMagnificationClipStretch) {
		oPreferencesPrinterImageSize = new CDlgPreferencesPrinterImageSize(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESPRINTERSIZE, oGlobalInstances);				  
		oPreferencesPrinterImageSize->Create();
		oPreferencePages.push_back(oPreferencesPrinterImageSize);
		}
	if (cPreferences==eAllPreferences || cPreferences==ePrinter || cPreferences==ePrinterDevice) {
		oPreferencesPrinterDevice = new CDlgPreferencesPrinterDevice(oGlobalInstances.hInstance, hDlg, IDD_PREFERENCESPRINTERDEVICE, oGlobalInstances);				  
		oPreferencesPrinterDevice->Create();
		oPreferencePages.push_back(oPreferencesPrinterDevice);
		}

	Localize(oGlobalInstances.hLanguage);
	
	if (cPreferences==eAllPreferences || cPreferences==eScanner || cPreferences==ePrinter) InitializeTreeView();

	ShowPreferences(cPreferences);
}

//This function localizes all controls in the dialog and all its child controls
void 
CDlgPreferences::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	FillTreeView();
	ShowPreferences(cPreferences);

	LocalizeWindowCaption(IDS_PREFERENCES);
	LocalizeControl(IDOK, IDS_OK);
	LocalizeControl(IDCANCEL, IDS_CANCEL);
	LocalizeControl(IDAPPLY, IDS_APPLY);

	for (long lIndex=0; lIndex<(long)oPreferencePages.size(); lIndex++) {
		
		oPreferencePages[lIndex]->Localize(phLanguage);
		}
}

BOOL 
CDlgPreferences::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;
	HWND hApplyButton;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_CAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}

			break;

		case WM_NOTIFY:
			
			LPNMTREEVIEW pNotificationTreeview;
			LPNMHDR pNotificationHeader;
			ePreferences cPage;

			pNotificationHeader = (LPNMHDR)lParam;

			if (pNotificationHeader->code == TVN_SELCHANGED) {

				//determine if the selection has changed from the treeview								
				if (pNotificationHeader->idFrom == IDC_TREEPREFERENCES) {

					pNotificationTreeview = (LPNMTREEVIEW)lParam;

					//get the new selected item
					cPage = (ePreferences)pNotificationTreeview->itemNew.lParam;
					ShowPreferences(cPage);
					}
				}

			break;

		case WM_CORENOTIFICATION:
			
			bReturn = OnEventCoreNotification();
			break;

		case WM_ENABLEAPPLY:

			hApplyButton = GetDlgItem(hDlg, IDAPPLY);
			EnableWindow(hApplyButton, TRUE);
			break;

		};	

	return bReturn;
	}

//This function handles the core notifications
bool 
CDlgPreferences::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;
	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	bReturn = false;

	while (oCmCore->oNotifications->GetNotification(hDlg, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationLanguageChanged:

				oGlobalInstances.hLanguage = (HINSTANCE)cNotification.pData;

				Localize(oGlobalInstances.hLanguage);
				break;
			}
		}

	return bReturn;
	}


BOOL 
CDlgPreferences::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	BOOL bResult;

	bResult = TRUE;

	//switch (iCommand) {

	//	default:

	//		bResult = FALSE;
	//		break;
	//	}

	bResult = FALSE;

	return bResult;
	}

//This function handles the OK event
void
CDlgPreferences::OnCloseOk() {

	OnApply();
	}

//This function toggles between the preference pages

void 
CDlgPreferences::ShowPreferences(ePreferences pePage) {

	oTrace->StartTrace(__WFUNCTION__);
	
	//make current page invisble
	if (hVisiblePage!=NULL) {

		ShowWindow(hVisiblePage, FALSE);
		}

	//get the handle to the new page
	switch (pePage) {
	
		case eScanner:
		case eScannerImage: hVisiblePage = oPreferencesScannerImage->hDlg; break;
		case eScannerPage: hVisiblePage = oPreferencesScannerPage->hDlg; break;
		case eScannerDevice: hVisiblePage = oPreferencesScannerDevice->hDlg; break;
		case eScannerAdvanced: hVisiblePage = oPreferencesScannerAdvanced->hDlg; break;
		case eTrace: hVisiblePage = oPreferencesTrace->hDlg; break;
		case ePrinter:
		case ePrinterCopiesAlignment: hVisiblePage=oPreferencesPrinterCopiesAlignment->hDlg; break;
		case ePrinterMagnificationClipStretch: hVisiblePage=oPreferencesPrinterImageSize->hDlg; break;
		case ePrinterDevice: hVisiblePage=oPreferencesPrinterDevice->hDlg; break;
		case eAutoSave: hVisiblePage=oPreferencesAutoSave->hDlg; break;

		default: hVisiblePage = oPreferencesGeneral->hDlg; break;
		}

	//reposition the new page and make it visible
	HWND hContainer = GetDlgItem(hDlg, IDC_CONTAINER);
	RECT cRect;
	POINT cPoint;
	GetWindowRect(hContainer, &cRect);

	cPoint.x = cRect.left;
	cPoint.y = cRect.top;
	ScreenToClient(hDlg, &cPoint);

	UINT nFlags = SWP_SHOWWINDOW|SWP_NOSIZE;
	SetWindowPos(hVisiblePage, hContainer, cPoint.x, cPoint.y, 0, 0, nFlags);

	oTrace->EndTrace(__WFUNCTION__, (int)pePage);
}

//This function fills the treeview with all the types of preferences in their
//hierarchy
void 
CDlgPreferences::InitializeTreeView() {

	Bitmap* oBitmap;
	HWND hTreeView;

	oTrace->StartTrace(__WFUNCTION__);

	hTreeView = GetDlgItem(hDlg, IDC_TREEPREFERENCES);
	
	//Create and fill an imagelist
	hImagelist = ImageList_Create(
			16, 16,					//size of images
			ILC_MASK | ILC_COLOR32,	//flags
			16,						//initial size
			2);						//numbers of images to grow when needed

	if (hImagelist!=NULL) {

		oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLPREFERENCES", L"PNG", hInstance);
		CBitmap::AddToImagelist(oBitmap, hImagelist);
		delete oBitmap;
		oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLSCANNER", L"PNG", hInstance);
		CBitmap::AddToImagelist(oBitmap, hImagelist);
		delete oBitmap;
		oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLPRINT", L"PNG", hInstance);
		CBitmap::AddToImagelist(oBitmap, hImagelist);
		delete oBitmap;
		oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLPREFERENCES", L"PNG", hInstance);
		CBitmap::AddToImagelist(oBitmap, hImagelist);
		delete oBitmap;
		oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLBULLETBLUE", L"PNG", hInstance);
		CBitmap::AddToImagelist(oBitmap, hImagelist);
		delete oBitmap;
		oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLPREFERENCES", L"PNG", hInstance);
		CBitmap::AddToImagelist(oBitmap, hImagelist);
		delete oBitmap;
		oBitmap = CBitmap::LoadFromResource(L"PNG_TOOLZOOM", L"PNG", hInstance);
		CBitmap::AddToImagelist(oBitmap, hImagelist);
		delete oBitmap;
		
		TreeView_SetImageList(hTreeView, hImagelist, TVSIL_NORMAL);
		}

	oTrace->EndTrace(__WFUNCTION__);
	}
	
void 
CDlgPreferences::FillTreeView() {

	HTREEITEM hTreeItem;
	HTREEITEM hParentItem;
	HWND hTreeView;

	oTrace->StartTrace(__WFUNCTION__);

	hTreeView = GetDlgItem(hDlg, IDC_TREEPREFERENCES);

	TreeView_DeleteAllItems(hTreeView);

	//Fill the list
	if (cPreferences==eAllPreferences) {
		
		hParentItem = AddTreeItem(NULL, 5, IDS_GENERAL, eGeneral);
		hTreeItem = AddTreeItem(hParentItem, 4, IDS_GENERAL, eGeneral);
		hTreeItem = AddTreeItem(hParentItem, 4, IDS_COLLECTIONS, eAutoSave);
		TreeView_Expand(hTreeView, hParentItem, TVE_EXPAND);
		}

	if (cPreferences==eAllPreferences || cPreferences==eScanner) {
		hParentItem = AddTreeItem(NULL, 1, IDS_SCANNER, eScannerImage);
		hTreeItem = AddTreeItem(hParentItem, 4, IDS_IMAGE, eScannerImage);
		hTreeItem = AddTreeItem(hParentItem, 4, IDS_PAGES, eScannerPage);
		hTreeItem = AddTreeItem(hParentItem, 4, IDS_DEVICES, eScannerDevice);
		hTreeItem = AddTreeItem(hParentItem, 4, IDS_ADVANCED, eScannerAdvanced);
		TreeView_Expand(hTreeView, hParentItem, TVE_EXPAND);
		}

	if (cPreferences==eAllPreferences || cPreferences==ePrinter) {
		hParentItem = AddTreeItem(NULL, 2, IDS_PRINTER, ePrinterCopiesAlignment);
		hTreeItem = AddTreeItem(hParentItem, 4, IDS_COPIESALIGNMENT, ePrinterCopiesAlignment);
		hTreeItem = AddTreeItem(hParentItem, 4, IDS_MAGNIFICATIONCLIPSTRETCH, ePrinterMagnificationClipStretch);
		hTreeItem = AddTreeItem(hParentItem, 4, IDS_DEVICES, ePrinterDevice);
		TreeView_Expand(hTreeView, hParentItem, TVE_EXPAND);
		}

	if (cPreferences==eAllPreferences) hParentItem = AddTreeItem(NULL, 6, IDS_TRACING, eTrace);

	oTrace->EndTrace(__WFUNCTION__);
	}	

HTREEITEM 
CDlgPreferences::AddTreeItem(HTREEITEM phParent, int piIcon, 
							 int piResource, ePreferences pePage) {

	HTREEITEM hTreeItem;
	TV_INSERTSTRUCT cInsertItem;
	wchar_t* cText;
	HWND hTreeView;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	//get the handle to the tree control
	hTreeView = GetDlgItem(hDlg, IDC_TREEPREFERENCES);

	//get the string
	cText = (wchar_t*)malloc(sizeof(wchar_t) * MAX_PATH);

	LoadString(oGlobalInstances.hLanguage, piResource, cText, MAX_PATH);

	//insert the item
	cInsertItem.hParent = phParent;
	cInsertItem.hInsertAfter = phParent==NULL ? TVI_ROOT : TVI_LAST;
	cInsertItem.item.mask = TVIF_PARAM | TVIF_TEXT;
	cInsertItem.item.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	cInsertItem.item.lParam = (LPARAM)pePage;
	cInsertItem.item.pszText = cText;
	cInsertItem.item.iImage = piIcon;
	cInsertItem.item.iSelectedImage = piIcon;

	hTreeItem = TreeView_InsertItem(hTreeView, &cInsertItem);

	free(cText);

	oTrace->EndTrace(__WFUNCTION__, (int)hTreeItem, eAll);

	return hTreeItem;
}

//This function gives all propertypages the apply command
void 
CDlgPreferences::OnApply() {

	for (long lIndex=0; lIndex<(long)oPreferencePages.size(); lIndex++) {
		
		oPreferencePages[lIndex]->OnApply();
		}

	HWND hApply = GetDlgItem(hDlg, IDAPPLY);
	EnableWindow(hApply, FALSE);
}
