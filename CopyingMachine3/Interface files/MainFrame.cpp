#include "StdAfx.h"
#include "RebarControl.h"
#include "DocumentList.h"
#include "DocumentCollectionList.h"
#include "PageThumbnails.h"
#include "Splitter.h"
#include "CopyingMachineCore.h"
#include "Toolbar.h"
#include "ToolbarMenu.h"
#include "ToolbarCopy.h"
#include "ToolbarPreferences.h"
#include "PageViewer.h"
#include "Pane.h"
#include "MenuIcon.h"
#include "WindowContainer.h"
#include "CopyFileListview.h"
#include "CopyFileListviewContainer.h"
#include "MainFrame.h"
#include "shellapi.h"

CMainFrame::CMainFrame(sGlobalInstances pInstances) :
	CWindowBase(pInstances, L"MainFrame")
{
	oRebarControl = NULL;
	oSplitterH = NULL;
	oSplitterH2 = NULL;
	oSplitterH3 = NULL;
	oSplitterV = NULL;
	oSplitterV2 = NULL;
	oPageViewer = NULL;
	oPane = NULL;
	oPageViewerSettings = NULL;
	oDocumentList = NULL;
	oThumbnails = NULL;
	oMenuIcon = NULL;
	oMenuToolbar = NULL;
	oCommandBar = NULL;
	oDocumentListContainer = NULL;
	oDocumentCollectionList = NULL;
	oDocumentCollectionListContainer = NULL;
	oThumbnailsContainer = NULL;
	oFileContainer = NULL;
	oPageViewerContainer = NULL;

	bCopyInProgress = false;

	oRegistry = pInstances.oRegistry;
	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;
	hLanguage = pInstances.hLanguage;
}

CMainFrame::~CMainFrame(void)
{
	oCore->oNotifications->UnsubscribeAll(hWnd);
	if (hWnd!=NULL) Destroy();
}

HWND CMainFrame::Create(HINSTANCE phInstance, int pnCmdShow) {

	TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
	TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

	oTrace->StartTrace(__WFUNCTION__);
	
	hInstance = phInstance;

	//Create the main window
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_COPYINGMACHINE3, szWindowClass, MAX_LOADSTRING);

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (hWnd) {

		//Create an instance of the class which handles
		//the icons in the menus
		oMenuIcon = new CMenuIcon(oGlobalInstances.hInstance);
		oGlobalInstances.oMenuIcon = oMenuIcon;

		COLORREF cTransparentColor;

		cTransparentColor = RGB(255,0,255);

		oMenuIcon->AddMenuCommandIcon(ID_EDIT_COPY,					L"PNG_TOOLCOPYPAGE");
		oMenuIcon->AddMenuCommandIcon(ID_EDIT_PASTEASNEWPAGE,		L"PNG_TOOLPASTE");
		oMenuIcon->AddMenuCommandIcon(ID_EDIT_CUT,					L"PNG_TOOLCUT");
		oMenuIcon->AddMenuCommandIcon(ID_FILE_NEW,					L"PNG_TOOLBOOKADD");
		oMenuIcon->AddMenuCommandIcon(IDC_ACQUIRETOCOLLECTION,		L"PNG_TOOLBOOKADD");
		oMenuIcon->AddMenuCommandIcon(IDC_ACQUIRETOCOLLECTIONFOLDER,L"PNG_TOOLBOOKADD");
		oMenuIcon->AddMenuCommandIcon(ID_FILE_PRINT,				L"PNG_TOOLPRINT");
		oMenuIcon->AddMenuCommandIcon(ID_FILE_OPEN,					L"PNG_TOOLOPEN");
		oMenuIcon->AddMenuCommandIcon(ID_FILE_SAVE,					L"PNG_TOOLSAVE");
		oMenuIcon->AddMenuCommandIcon(ID_SCAN_ACQUIRE,				L"PNG_TOOLBOOKADD");
		oMenuIcon->AddMenuCommandIcon(ID_FILE_PRINT_PREVIEW,		L"PNG_TOOLPREVIEW");
		oMenuIcon->AddMenuCommandIcon(ID_FILE_PREFERENCES,			L"PNG_TOOLPREFERENCES");
		oMenuIcon->AddMenuCommandIcon(ID_LANGUAGE_DUTCH,			L"PNG_FLAGNL");
		oMenuIcon->AddMenuCommandIcon(ID_LANGUAGE_ENGLISH,			L"PNG_FLAGEN");
		oMenuIcon->AddMenuCommandIcon(ID_LANGUAGE_ITALIAN,			L"PNG_FLAGIT");
		oMenuIcon->AddMenuCommandIcon(ID_LANGUAGE_GERMAN,			L"PNG_FLAGDE");
		oMenuIcon->AddMenuCommandIcon(ID_LANGUAGE_FRENCH,			L"PNG_FLAGFR");
		oMenuIcon->AddMenuCommandIcon(ID_IMAGE_CROP,				L"PNG_TOOLCROP");
		oMenuIcon->AddMenuCommandIcon(ID_IMAGE_FILLSELECTION,		L"PNG_TOOLPAINT");
		oMenuIcon->AddMenuCommandIcon(ID_ROTATEANDFLIP_FLIPVERTICALLY,		L"PNG_TOOLFLIPVERTICAL");
		oMenuIcon->AddMenuCommandIcon(ID_ROTATEANDFLIP_FLIPHORIZONTALLY,	L"PNG_TOOLFLIPHORIZONTAL");
		oMenuIcon->AddMenuCommandIcon(ID_ROTATE_90,					L"PNG_TOOLROTATE90");
		oMenuIcon->AddMenuCommandIcon(ID_ROTATE_180,				L"PNG_TOOLROTATE180");
		oMenuIcon->AddMenuCommandIcon(ID_ROTATE_270,				L"PNG_TOOLROTATE270");
		oMenuIcon->AddMenuCommandIcon(ID_CONVERTTO_BLACKWHITE,		L"PNG_TOOLCONVERTBLACKWHITE");
		oMenuIcon->AddMenuCommandIcon(ID_CONVERTTO_GRAYSCALE,		L"PNG_TOOLCONVERTGRAYSCALE");
		oMenuIcon->AddMenuCommandIcon(ID_VIEW_PANECOPY,				L"PNG_TOOLPANECOPY");
		oMenuIcon->AddMenuCommandIcon(ID_VIEW_PANEIMAGE,			L"PNG_TOOLPANEIMAGE");
		oMenuIcon->AddMenuCommandIcon(ID_VIEW_PANEPAGE,				L"PNG_TOOLPANEINFO");
		oMenuIcon->AddMenuCommandIcon(IDC_SWITCH,					L"PNG_TOOLSWITCH");
		oMenuIcon->AddMenuCommandIcon(IDC_DOCOPY,					L"PNG_TOOLCOPY");
		oMenuIcon->AddMenuCommandIcon(ID_ZOOM_ZOOMTOFIT,			L"PNG_TOOLZOOMTOWINDOW");
		oMenuIcon->AddMenuCommandIcon(ID_ZOOM_ZOOMTOSELECTION,		L"PNG_TOOLZOOMTOSELECTION");
		oMenuIcon->AddMenuCommandIcon(ID_IMAGE_RECOGNIZETEXT,		L"PNG_TOOLOCR");
		oMenuIcon->AddMenuCommandIcon(ID_VIEW_RECOGNITIONLAYER,		L"PNG_TOOLOCR");
		oMenuIcon->AddMenuCommandIcon(ID_DOCUMENT_DELETEPAGE,		L"PNG_TOOLDELETE");
		oMenuIcon->AddMenuCommandIcon(ID_PAGE_DELETETEXTBLOCKS,		L"PNG_TOOLDELETE");
		oMenuIcon->AddMenuCommandIcon(ID_EDIT_COPYTEXTBLOCKTOCLIPBOARD,	L"PNG_TOOLCOPYPAGE");
		oMenuIcon->AddMenuCommandIcon(ID_EDIT_COPYTEXTTOCLIPBOARD,	L"PNG_TOOLCOPYPAGE");
		oMenuIcon->AddMenuCommandIcon(ID_PAGE_DELETETEXTBLOCK,		L"PNG_TOOLDELETE");
		oMenuIcon->AddMenuCommandIcon(ID_IMAGE_UNDO,				L"PNG_TOOLUNDO");
		oMenuIcon->AddMenuCommandIcon(ID_IMAGE_REDO,				L"PNG_TOOLREDO");
		oMenuIcon->AddMenuCommandIcon(ID_DOCUMENT_ACQUIRENEWPAGE,	L"PNG_TOOLPAGEADD");
		oMenuIcon->AddMenuCommandIcon(ID_IMAGE_COLORPICKER,			L"PNG_TOOLCOLORDROPPER");
		oMenuIcon->AddMenuCommandIcon(ID_IMAGE_CHOOSECOLOR,			L"PNG_TOOLCOLORDROPPER");
		oMenuIcon->AddMenuCommandIcon(ID_EDIT_CLEARSELECTION,		L"PNG_TOOLDESELECT");
		oMenuIcon->AddMenuCommandIcon(ID_EDIT_SELECTALL,			L"PNG_TOOLSELECT");
		oMenuIcon->AddMenuCommandIcon(ID_HELP_CONTENTS,				L"PNG_TOOLHELP");
		oMenuIcon->AddMenuCommandIcon(ID_DOCUMENT_SPLITDOCUMENT,	L"PNG_TOOLSPLIT");
		oMenuIcon->AddMenuCommandIcon(IDC_EXPLORE,			L"PNG_TOOLFOLDEREXPLORE");
		oMenuIcon->AddMenuCommandIcon(IDC_EXPLOREFOLDER,	L"PNG_TOOLFOLDEREXPLORE");
		
		oCore->oNotifications->Subscribe(hWnd, eNotificationWindowsInitialized);

		//These could be added to documentlist but the listview doesn't process
		//wm_app messages somehow... to do
		oCore->oNotifications->Subscribe(hWnd, eNotificationScanDeviceChanged);
		oCore->oNotifications->Subscribe(hWnd, eNotificationPrintDeviceChanged);
		oCore->oNotifications->Subscribe(hWnd, eNotificationApplicationInitialized);
		oCore->oNotifications->Subscribe(hWnd, eNotificationNewDocument);
		oCore->oNotifications->Subscribe(hWnd, eNotificationNewPage);
		oCore->oNotifications->Subscribe(hWnd, eNotificationPageUpdate);
		oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentViewChanges);
		oCore->oNotifications->Subscribe(hWnd, eNotificationAcquisitionPageDone);
		oCore->oNotifications->Subscribe(hWnd, eNotificationLanguageChanged);
		oCore->oNotifications->Subscribe(hWnd, eNotificationPrintSettingsChanged);
		oCore->oNotifications->Subscribe(hWnd, eNotificationScanSettingsChanged);
		oCore->oNotifications->Subscribe(hWnd, eNotificationPageSettingUpdate);
		oCore->oNotifications->Subscribe(hWnd, eNotificationPageDeleted);
		oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentClosed);
		oCore->oNotifications->Subscribe(hWnd, eNotificationPageViewChanges);
		oCore->oNotifications->Subscribe(hWnd, eNotificationPaneChange);
		oCore->oNotifications->Subscribe(hWnd, eNotificationUpdateControls);
		oCore->oNotifications->Subscribe(hWnd, eNotificationColorChanged);
		oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentUpdate);
		oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentCollectionUpdate);
		oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentCollectionSelection);

		//Set the main menu
		HMENU hMainMenu;

		hMainMenu = LoadMenu(oGlobalInstances.hLanguage, MAKEINTRESOURCE(IDC_COPYINGMACHINE3));
		SetMenu(hWnd, hMainMenu);

		//Fill the main window
		CreateChildWindows();

		//Position the main window
		nCmdShow = pnCmdShow;
		LoadSettings();

		DragAcceptFiles(oPageViewer->hWnd,		true);
		}


	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
}

void CMainFrame::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

//	CWindowBase::Destroy();

	delete oRebarControl;
	delete oCommandBar;
	if (oMenuToolbar) delete oMenuToolbar;
	delete oThumbnails;
	delete oSplitterH;
	delete oSplitterH2;
	delete oSplitterH3;
	delete oSplitterV;
	delete oSplitterV2;
	delete oFileContainer;
	delete oPageViewer;
	delete oPageViewerSettings;
	delete oDocumentList;
	delete oDocumentCollectionList;
	delete oPane;
	delete oMenuIcon;
	delete oDocumentListContainer;
	delete oThumbnailsContainer;
	delete oDocumentCollectionListContainer;
	delete oPageViewerContainer;
	//delete oFileContainer;
	//delete oDocumentListContainer;
	//delete oThumbnailsContainer;

	CWindowBase::Destroy();

	oTrace->EndTrace(__WFUNCTION__);
	}


//This function processes all the message
bool CMainFrame::WndProc(HWND phWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn) {

	bool bReturn;
	PAINTSTRUCT ps;
	HDC hdc;
	LPNMHDR pcNotificationHeader;

	oTrace->StartTrace(__WFUNCTION__, eExtreme);

	bReturn = true;

	switch (message) {
	
		case WM_INITMENU:
		case WM_INITMENUPOPUP:

			DoEnableControlsPaste();
			break;

		case WM_COMMAND:

			//Process the command of the user
			bReturn = OnCommand(LOWORD(wParam));
			break;

		case WM_NOTIFY:

			pcNotificationHeader = (LPNMHDR)lParam;

			if (oRebarControl) {
				if (pcNotificationHeader->hwndFrom == oRebarControl->hWnd) {
				switch (pcNotificationHeader->code) {

					case RBN_LAYOUTCHANGED:
					case RBN_HEIGHTCHANGE:
						LayoutChildWindows();
						break;
					}
				}
			}
			bReturn=false;
			break;

		case WM_DROPFILES:

			DoAcceptDropFiles((HDROP)wParam);
			bReturn=false;
			break;

		case WM_SIZE:

			bReturn = false;
			if (phWnd==this->hWnd) {
				LayoutChildWindows();	
				bReturn = true;
				}
			break;

		case WM_PAINT:
			if (phWnd==this->hWnd) {
				hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any drawing code here...
				EndPaint(hWnd, &ps);
				}
			bReturn = false;
			break;

		case WM_CLOSE:
			
			if (oCore->SaveChangesToDirtyDocuments(hWnd)) {
			
				oCore->oRegistration->Reminder(hWnd);
				
				SaveSettings();
				oRebarControl->SaveRebarSettings();
				PostQuitMessage(0);
				bReturn = false;
				}
			else {

				piReturn = 0;
				bReturn = true;
				}

			break;

		case WM_DESTROY:
			//PostQuitMessage(0);
			break;
		
		case WM_CORENOTIFICATION:

			if (phWnd == hWnd) {

				bReturn = OnEventCoreNotification();
				}
			bReturn=false;
			break;

		default:
			bReturn = false;
		}

	if (!bReturn && oMenuToolbar)			bReturn = oMenuToolbar->WndProc(phWnd, message, wParam, lParam, piReturn);
	if (!bReturn && oMenuIcon)				bReturn = oMenuIcon->WndProc(phWnd, message, wParam, lParam, piReturn);

	//Process the base wndproc 
	if (!bReturn) {

		bReturn = CWindowBase::WndProc(phWnd, message, wParam, lParam, piReturn);
		}

	oTrace->EndTrace(__WFUNCTION__, bReturn, eExtreme);
	
	return bReturn;
	}

//This function handles the core notifications
bool 
CMainFrame::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;
	sDocumentCollection oCollection;

	bReturn = false;

	while (oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationNewDocument:

				if (bCopyInProgress) {

					//Do step 2 of copying the document, output it.
					DoCopyDocumentStep2();
					}
				else {

					//Add the new document to the document list
					oDocumentList->AddItem((int)cNotification.pData);
					}

				//Process the next line of a possible script after a
				//scan or copy action
				//oCore->oScriptProcessor->ExecuteNextScriptLine();
	
				DoEnableControls();
				break;

			case eNotificationDocumentCollectionUpdate:
				oDocumentCollectionList->SyncDocumentCollections();
				break;

			case eNotificationPageViewChanges:
			case eNotificationUpdateControls:
				DoEnableControls();
				break;

			case eNotificationPageSettingUpdate:
				DoEnableControls();
				oDocumentList->SyncDocumentSelection();
				ViewPage();
				break;

			case eNotificationDocumentUpdate:
			case eNotificationDocumentClosed:
			case eNotificationDocumentViewChanges:

				//The current document has changed, view it.
				if (!bCopyInProgress) {
					ViewDocument(oCore->GetCurrentDocument());
					oDocumentList->SyncDocumentSelection();
					DoEnableControls();
					DoUpdateTitle();					
					}
				break;

			case eNotificationNewPage:

				//A new page has been added
				CCopyDocument* oDocument;

				oDocument = oCore->GetCurrentDocument();

				if (oDocument!=NULL) {

					AddPageToThumbnailsControl(oDocument->GetPageByID((DWORD)cNotification.pData));
					}

				DoEnableControls();
				
				break;

			case eNotificationPageUpdate:

				//The current page has changed, update the thumbnail
				oThumbnails->UpdateItem((DWORD)cNotification.pData);
				oDocumentList->SyncDocumentSelection();
				DoEnableControls();
				
				break;

			
			case eNotificationPageDeleted:

				//The current page has changed, update the thumbnail
				oThumbnails->DeleteItem((DWORD)cNotification.pData);
				oDocumentList->SyncDocumentSelection();
				ViewPage();
				DoEnableControls();
				
				break;

			case eNotificationWindowsInitialized:

				//The main window is done with initializing, show the current document
				//if there is one
				ViewDocument(oCore->GetCurrentDocument());
		
				for (long lIndex=0; lIndex<oCore->oDocuments->GetDocumentCount(); lIndex++) {

					oDocumentList->AddItem(oCore->oDocuments->GetDocument(lIndex)->GetDocumentID());
					}
				DoEnableControls();
				
				oCore->oScanCore->GetDeviceCapabilities();

				break;

			case eNotificationAcquisitionPageDone:

				//Acquiring image(s) is done. Transfer them
				oCore->TransferImages(hWnd);
				
				//if (!bCopyInProgress) ViewDocument(oCore->GetCurrentDocument());
				DoEnableControls();

				break;

			case eNotificationApplicationInitialized:
				
				oCore->oRegistration->Reminder(hWnd);
				break;

			//change the toolbar pressed button
			case eNotificationPaneChange:

				SendMessage(oCommandBar->hWnd, TB_PRESSBUTTON, 
							(WPARAM)ID_VIEW_PANECOPY, 
							(LPARAM)MAKELONG(((int)cNotification.pData == ID_VIEW_PANECOPY), 0));
				SendMessage(oCommandBar->hWnd, TB_PRESSBUTTON, 
							(WPARAM)ID_VIEW_PANEIMAGE, 
							(LPARAM)MAKELONG(((int)cNotification.pData == ID_VIEW_PANEIMAGE), 0));
				SendMessage(oCommandBar->hWnd, TB_PRESSBUTTON, 
							(WPARAM)ID_VIEW_PANEPAGE, 
							(LPARAM)MAKELONG(((int)cNotification.pData == ID_VIEW_PANEPAGE), 0));
				
				oMenuIcon->SetCheckMenuCommand(ID_VIEW_PANECOPY, ((int)cNotification.pData == ID_VIEW_PANECOPY));
				oMenuIcon->SetCheckMenuCommand(ID_VIEW_PANEIMAGE, ((int)cNotification.pData == ID_VIEW_PANEIMAGE));
				oMenuIcon->SetCheckMenuCommand(ID_VIEW_PANEPAGE, ((int)cNotification.pData == ID_VIEW_PANEPAGE));
				break;

			//The color is changed for filling a rectangle
			case eNotificationColorChanged:

				DoChangeColor((DWORD)cNotification.pData);
				break;

			//Update the filelistview with the new folder
			case eNotificationDocumentCollectionSelection:

				if (oCore->oDocumentCollections->GetDocumentCollection((int)cNotification.pData, oCollection)) {

					if (oFileContainer->oFileListview->GetFolder()!=oCollection.sLocation) {

						oFileContainer->oFileListview->SetFolder(oCollection.sLocation);
						}

					oFileContainer->MakeVisible();
					}

				break;

			case eNotificationLanguageChanged:

				oGlobalInstances.hLanguage = (HINSTANCE)cNotification.pData;

				HMENU hMainMenu;

				hMainMenu = LoadMenu(oGlobalInstances.hLanguage, MAKEINTRESOURCE(IDC_COPYINGMACHINE3));
				SetMenu(hWnd, hMainMenu);

				if (oPane)					oPane->Localize(oGlobalInstances.hLanguage);
				if (oPageViewer)			oPageViewer->Localize(oGlobalInstances.hLanguage);
				if (oDocumentList)			oDocumentList->Localize(oGlobalInstances.hLanguage);
				if (oThumbnails)			oThumbnails->Localize(oGlobalInstances.hLanguage);
				if (oDocumentListContainer) oDocumentListContainer->Localize(oGlobalInstances.hLanguage);
				if (oDocumentCollectionListContainer) oDocumentCollectionListContainer->Localize(oGlobalInstances.hLanguage);
				if (oThumbnailsContainer)	oThumbnailsContainer->Localize(oGlobalInstances.hLanguage);
				if (oFileContainer)			oFileContainer->Localize(oGlobalInstances.hLanguage);
				if (oCommandBar)			oCommandBar->Localize(hMainMenu);

				DoEnableControls();
				break;
			}
		}

	return bReturn;
	}
	
//this function layouts the child windows after an event which
//causes the window or one of its child to change its size.
void CMainFrame::LayoutChildWindows() {


	RECT cClientRect;
	RECT cRebarControlRect;
	int iThumbnailSize;
	int iPaneSize;
	int iDocumentListSize;
	int iDocumentCollectionListSize;
	int iFileListSize;
	int iColumn2Size;
	int iTop;

	if (oRebarControl==NULL) return;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	GetClientRect(hWnd, &cClientRect);
	GetClientRect(oRebarControl->hWnd, &cRebarControlRect);
	iThumbnailSize =				oSplitterV->GetSiblingSize();
	iPaneSize =						oSplitterV2->GetSiblingSize();
	iDocumentListSize =				oSplitterH->GetSiblingSize();
	iDocumentCollectionListSize =	oSplitterH2->GetSiblingSize();
	iFileListSize =					oSplitterH3->GetSiblingSize();
	iColumn2Size =					cClientRect.right - cClientRect.left - iThumbnailSize - iPaneSize - iSplitterWidth*4;

	MoveWindow(oRebarControl->hWnd, 
				0, 0, 
				cClientRect.right, cRebarControlRect.bottom, 
				TRUE);

	cClientRect.left	+= iSplitterWidth;
	cClientRect.top		= cRebarControlRect.bottom + iSplitterWidth;
	cClientRect.bottom	-= iSplitterWidth;
	cClientRect.right	-= iSplitterWidth;

	//Column 1
	iTop = cClientRect.top;
	MoveWindow(oDocumentCollectionListContainer->hWnd, 
				cClientRect.left, iTop, 
				iThumbnailSize, iDocumentCollectionListSize, 
				TRUE);
	iTop += iDocumentCollectionListSize;
	MoveWindow(oSplitterH2->hWnd,
			    cClientRect.left, iTop,
				iThumbnailSize, iSplitterWidth, 
				TRUE);
	iTop += iSplitterWidth;
	MoveWindow(oDocumentListContainer->hWnd, 
				cClientRect.left, iTop, 
				iThumbnailSize, iDocumentListSize, 
				TRUE);
	iTop += iDocumentListSize;
	MoveWindow(oSplitterH->hWnd,
			    cClientRect.left, iTop,
				iThumbnailSize, iSplitterWidth, 
				TRUE);
	iTop += iSplitterWidth;
	MoveWindow(oThumbnailsContainer->hWnd, 
				cClientRect.left, iTop, 
				iThumbnailSize, cClientRect.bottom - iTop, 
				TRUE);

	//Variable Splitter between column 1 and 2
	MoveWindow(oSplitterV->hWnd,
			    cClientRect.left + iThumbnailSize, cClientRect.top,
				iSplitterWidth, cClientRect.bottom - cClientRect.top, 
				TRUE);

	//Column 2
	iTop = cClientRect.top;

	if (oFileContainer->IsVisible()) {

		MoveWindow(oFileContainer->hWnd, 
					cClientRect.left + iThumbnailSize + iSplitterWidth, iTop, 
					iColumn2Size, iFileListSize, 
					TRUE);
		iTop += iFileListSize;

		if (!IsWindowVisible(oSplitterH3->hWnd)) ShowWindow(oSplitterH3->hWnd, SW_SHOW);
		MoveWindow(oSplitterH3->hWnd,
					cClientRect.left + iThumbnailSize + iSplitterWidth, iTop, 
					iColumn2Size, iSplitterWidth, 
					TRUE);
		iTop += iSplitterWidth;
		}
	else {

		if (IsWindowVisible(oSplitterH3->hWnd)) ShowWindow(oSplitterH3->hWnd, SW_HIDE);
		}

	MoveWindow(oPageViewerContainer->hWnd, 
				cClientRect.left + iThumbnailSize + iSplitterWidth, iTop, 
				iColumn2Size, cClientRect.bottom - iTop, 
				TRUE);

	//Variable Splitter between column 2 and 3
	MoveWindow(oSplitterV2->hWnd,
			    cClientRect.right - iSplitterWidth - iPaneSize, cClientRect.top,
				iSplitterWidth, cClientRect.bottom - cClientRect.top, 
				TRUE);

	//Column 3
	oPane->Resize(cClientRect.right - iPaneSize, cClientRect.top, 
				iPaneSize, cClientRect.bottom - cClientRect.top);


	oTrace->EndTrace(__WFUNCTION__, eAll);
	}


//This function creates and adds the toolbars

void CMainFrame::CreateChildWindows() {

	oTrace->StartTrace(__WFUNCTION__);
	
	//Create the documentcollectionlist
	oDocumentCollectionListContainer = new CWindowContainer(oGlobalInstances, IDS_DOCUMENTCOLLECTIONS, L"WCDocumentCollections");
	oDocumentCollectionListContainer->Create(hWnd, hInstance, WC_BUTTONHIDE);
	AddChildWindow(oDocumentCollectionListContainer);

	oDocumentCollectionList = new CDocumentCollectionList(oGlobalInstances, L"WDocumentCollections");
	oDocumentCollectionList->Create(oDocumentCollectionListContainer->hWnd, hInstance);
	AddChildWindow(oDocumentCollectionList);
	
	oDocumentCollectionListContainer->SetContainedWindow(oDocumentCollectionList->hWnd);	

	//Create the documentlist
	oDocumentListContainer = new CWindowContainer(oGlobalInstances, IDS_DOCUMENTS, L"WCDocuments");
	oDocumentListContainer->Create(hWnd, hInstance, WC_BUTTONHIDE);
	AddChildWindow(oDocumentListContainer);

	oDocumentList = new CDocumentList(oGlobalInstances, L"WDocuments");
	oDocumentList->Create(oDocumentListContainer->hWnd, hInstance);
	AddChildWindow(oDocumentList);

	oDocumentListContainer->SetContainedWindow(oDocumentList->hWnd);

	//Create the page thumbnails
	oThumbnailsContainer = new CWindowContainer(oGlobalInstances, IDS_PAGES, L"WCPages");
	oThumbnailsContainer->Create(hWnd, hInstance);
	AddChildWindow(oThumbnailsContainer);

    oThumbnails = new CPageThumbnails(oGlobalInstances, L"WPages");
	oThumbnails->Create(oThumbnailsContainer->hWnd, hInstance);
	AddChildWindow(oThumbnails);

	oThumbnailsContainer->SetContainedWindow(oThumbnails->hWnd);

	//Create the filelistcontainer 
	oFileContainer = new CCopyFileListviewContainer(oGlobalInstances, IDS_DOCUMENTCOLLECTIONS, L"WCFiles");
	oFileContainer->Create(hWnd, hInstance, WC_BUTTONHIDE | WC_BUTTONCLOSE);
	AddChildWindow(oFileContainer);
	//ShowWindow(oFileContainer->hWnd, SW_HIDE);
	
	//Create the page viewer
	oPageViewerContainer = new CWindowContainer(oGlobalInstances, IDS_PAGE, L"WCPageViewer");
	oPageViewerContainer->Create(hWnd, hInstance);
	AddChildWindow(oPageViewerContainer);

	oPageViewer = new CPageViewer(oGlobalInstances, L"WPageViewer");
	oPageViewer->Create(oPageViewerContainer->hWnd, hInstance, 0);
	AddChildWindow(oPageViewer);

	oPageViewerContainer->SetContainedWindow(oPageViewer->hWnd);


	//Create the horizontal splitters
	oSplitterH = new CSplitter(oGlobalInstances, L"SplitterH");
	oSplitterH->Create(hWnd, hInstance, 0, eSplitterH, oDocumentListContainer->hWnd);
	oDocumentListContainer->oConnectedSplitter = oSplitterH;
	AddChildWindow(oSplitterH);
	oSplitterH2 = new CSplitter(oGlobalInstances, L"SplitterH2");
	oSplitterH2->Create(hWnd, hInstance, 0, eSplitterH, oDocumentCollectionListContainer->hWnd);
	oDocumentCollectionListContainer->oConnectedSplitter = oSplitterH2;
	AddChildWindow(oSplitterH2);
	oSplitterH3 = new CSplitter(oGlobalInstances, L"SplitterH3");
	oSplitterH3->Create(hWnd, hInstance, 0, eSplitterH, oFileContainer->hWnd);
	oFileContainer->oConnectedSplitter = oSplitterH3;
	AddChildWindow(oSplitterH3);

	//Create the vertical splitters
	oSplitterV = new CSplitter(oGlobalInstances, L"SplitterV");
	oSplitterV->Create(hWnd, hInstance, 0, eSplitterVRight, oThumbnailsContainer->hWnd);
	AddChildWindow(oSplitterV);

	//Create the pane container
	oPane = new CPane(oGlobalInstances);
	oPane->Create(hInstance, hWnd, 0);
	AddChildWindow(oPane);

	oSplitterV2 = new CSplitter(oGlobalInstances, L"SplitterV2");
	oSplitterV2->Create(hWnd, hInstance, 0, eSplitterVLeft, oPane->hWnd);
	oSplitterV2->SetDesiredSiblingSize(230);
	AddChildWindow(oSplitterV2);


	//Create the rebar control and the toolbars
	oRebarControl = new CRebarControl(oGlobalInstances);
	oRebarControl->Create(hInstance, hWnd);

	//int iToolImagesOn[]  = {IDB_TOOLNEWON, IDB_TOOLOPENON, IDB_TOOLSAVEON, IDB_TOOLPRINTON, 
	//						IDB_TOOLPREVIEWON, IDB_TOOLCUTON, IDB_TOOLCOPYON, IDB_TOOLPASTEON,
	//						IDB_PANECOPY, IDB_PANEIMAGE, IDB_PANEINFORMATION, IDB_PANEINFORMATION,
	//						IDB_TOOLROTATE270, IDB_TOOLROTATE90, IDB_ICONCOG, IDB_ICONSIMPLE};
	std::wstring sToolImages[]  = {L"PNG_TOOLBOOKADD", L"PNG_TOOLOPEN", L"PNG_TOOLSAVE", L"PNG_TOOLPRINT",
							   L"PNG_TOOLPREVIEW", L"PNG_TOOLPANECOPY", L"PNG_TOOLPANEIMAGE", L"PNG_TOOLPANEINFO",
							   L"PNG_TOOLPREFERENCES", L"PNG_TOOLUNDO", L"PNG_TOOLREDO", L"PNG_TOOLSWITCH", 
							   L"PNG_TOOLSWITCH", L"PNG_TOOLCOPY"};
    
   sToolbarButton cStandardToolbar[] = {
		{13 , IDC_DOCOPY, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{0, -1, TBSTATE_ENABLED, BTNS_SEP, 0, 0},
		{0 , ID_FILE_NEW, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{1, ID_FILE_OPEN, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{2, ID_FILE_SAVE, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{3, ID_FILE_PRINT_DIRECT, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{4, ID_FILE_PRINT_PREVIEW, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{0, -1, TBSTATE_ENABLED, BTNS_SEP, 0, 0},
		{11, ID_IMAGE_CHOOSECOLOR, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{0, -1, TBSTATE_ENABLED, BTNS_SEP, 0, 0},
		{9, ID_IMAGE_UNDO, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{10, ID_IMAGE_REDO, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{0, -1, TBSTATE_ENABLED, BTNS_SEP, 0, 0},
		{8, ID_FILE_PREFERENCES, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0},
		{12, IDC_SWITCH, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE | TBSTYLE_BUTTON | BTNS_SHOWTEXT, 0, 0}
//		{0, -1, TBSTATE_ENABLED, BTNS_SEP, 0, 0},
//		{5, ID_VIEW_PANECOPY, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_BUTTON, 0, 0},
//		{6, ID_VIEW_PANEIMAGE, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_BUTTON, 0, 0},
//		{7, ID_VIEW_PANEPAGE, TBSTATE_ENABLED, BTNS_AUTOSIZE | BTNS_BUTTON, 0, 0},
	   };

   oCommandBar = new CToolbar(oGlobalInstances);
	oCommandBar->Create(hInstance, hWnd, 1001);
	oCommandBar->SetDisabledImages(hInstance, sToolImages, 14);
	oCommandBar->SetDefaultImages(hInstance, sToolImages, 14);
	oCommandBar->SetHotImages(hInstance, sToolImages, 14);
	oCommandBar->AddButtons(cStandardToolbar, sizeof(cStandardToolbar)/sizeof(sToolbarButton));
	DoChangeColor(oRegistry->ReadInt(L"General", L"Color", 0xFFFFFF)); //Create the color button
	oCommandBar->Localize(GetMenu(hWnd));

	//oToolbar2 = new CToolbar(oGlobalInstances);
	//oToolbar2->Create(hInstance, hWnd, 1002);
	//oToolbar2->SetDefaultImages(hInstance, sToolImages, 11);
	//oToolbar2->SetHotImages(hInstance, sToolImages, 11);
	//oToolbar2->AddButtons(cStandardToolbar, 3);

	//oMenuToolbar = new CMenuToolbar(oGlobalInstances, oMenuIcon);
	//oMenuToolbar->Create(hInstance, hWnd, 1003);
	//oMenuToolbar->Attach(hInstance, MAKEINTRESOURCE(IDC_COPYINGMACHINE3));
   
	oRebarControl->AddBand(oCommandBar->hWnd, 1001);
	//oRebarControl->AddBand(oToolbar2->hWnd, 1002);
	//oRebarControl->AddBand(oMenuToolbar->hWnd, 1003);
	

	oRebarControl->LoadRebarSettings();

	oTrace->EndTrace(__WFUNCTION__);
}

void CMainFrame::LoadSettings() {

	oTrace->StartTrace(__WFUNCTION__);

	RECT cRect;
	int iMode;
	int iMaxX, iMaxY;

	iMode = oRegistry->ReadInt(L"Window", L"Mode", -1);

	if (iMode!=-1) {

		//read the registry
		cRect.left = oRegistry->ReadInt(L"Window", L"Left", 20);	
		cRect.top = oRegistry->ReadInt(L"Window", L"Top", 20);	
		cRect.right = oRegistry->ReadInt(L"Window", L"Right", 500);	
		cRect.bottom = oRegistry->ReadInt(L"Window", L"Bottom", 400);

        // make sure the window is not completely out of sight
        iMaxX = GetSystemMetrics(SM_CXSCREEN) - GetSystemMetrics(SM_CXICON);
        iMaxY = GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYICON);

        cRect.left = min(cRect.left, iMaxX);
        cRect.top  = min(cRect.top,  iMaxY);
		cRect.left = max(0, cRect.left);
		cRect.top  = max(0, cRect.top);

		//Restore the position
		MoveWindow(hWnd,
				cRect.left, cRect.top,
				cRect.right - cRect.left, cRect.bottom - cRect.top,
				FALSE);

		nCmdShow = iMode;
	}

	//Process the standard load settings
	CWindowBase::LoadSettings();
	
	oTrace->EndTrace(__WFUNCTION__);
}

void CMainFrame::SaveSettings() {

	WINDOWPLACEMENT wPlacement;

	oTrace->StartTrace(__WFUNCTION__);

	wPlacement.length = sizeof(wPlacement);

	GetWindowPlacement(hWnd, &wPlacement);

	oRegistry->WriteInt(L"Window", L"Left", wPlacement.rcNormalPosition.left);
	oRegistry->WriteInt(L"Window", L"Top", wPlacement.rcNormalPosition.top);
	oRegistry->WriteInt(L"Window", L"Right", wPlacement.rcNormalPosition.right);
	oRegistry->WriteInt(L"Window", L"Bottom", wPlacement.rcNormalPosition.bottom);
	oRegistry->WriteInt(L"Window", L"Mode", wPlacement.showCmd);

	//Process the standard save settings
	CWindowBase::SaveSettings();

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function changes the viewed page
void
CMainFrame::ViewPage(CCopyDocumentPage* poPage) {

	oTrace->StartTrace(__WFUNCTION__);

	if (poPage!=NULL) {
		oCore->SetCurrentPage(poPage->GetPageID());
		oThumbnails->SelectItem(poPage->GetPageID());
		}
	oPageViewer->NewPage();
	
	oTrace->EndTrace(__WFUNCTION__);
	}

//This function changes the viewed document
void 
CMainFrame::ViewDocument(CCopyDocument* poDocument, DWORD pdPageId) {

	int iNumberPages;
	CCopyDocumentPage *poPage;

	oTrace->StartTrace(__WFUNCTION__);

	oThumbnails->Clear();

	if (poDocument!=NULL) {

		iNumberPages = poDocument->GetPageCount();

		//add the thumbnails 
		for (long lIndex=0; lIndex<iNumberPages; lIndex++) {
		
			poPage = poDocument->GetPage(lIndex);
			if (poPage!=NULL) AddPageToThumbnailsControl(poPage);				
			}

		//Set the currently selected page in the thumbnails control
		if (!oThumbnails->SelectItem(pdPageId)) {
			oThumbnails->SetSelectedItem(0);
			}
		
		//Redraw the page viewer
		oPageViewer->NewPage();

		//Synchronize the document list
		oDocumentList->SyncDocumentSelection();
		}
	else {

		//Clear the page viewer
		oPageViewer->Redraw();
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function inserts the given page to thumbnails control
void 
CMainFrame::AddPageToThumbnailsControl(CCopyDocumentPage* poPage) {

	int iPageNumber;
	std::wstring sPage;
	THUMBNAILITEM cThumbnail;
	wchar_t szPage[50];
	CCopyDocumentPage	*oPage;
	CCopyDocument		*oDocument;

	oTrace->StartTrace(__WFUNCTION__);

	//Set the template for the page title
	LoadString(hLanguage, IDS_PAGE, szPage, 50);
	wcscat_s(szPage, 50, L" %d");

	//Get the current document
	oDocument = oCore->GetCurrentDocument();

	if (oDocument!=NULL && poPage!=NULL) {

		iPageNumber = ListView_GetItemCount(oThumbnails->hWnd);
		
		//Add the thumbnail to the end of the thumbnail list
		cThumbnail.oImage = poPage->GetImage();
		swprintf_s(cThumbnail.pszText, MAX_PATH, szPage, iPageNumber+1);

		oThumbnails->AddItem(poPage->GetPageID(), cThumbnail);


		//Set the currently selected page in the thumbnails control
		oPage = oCore->GetCurrentPage();
		if (oPage!=NULL) {
			if (!oThumbnails->SelectItem(oPage->GetPageID())) {
				oThumbnails->SetSelectedItem(0);
				}
			}

		//sort the items in the listview
		oThumbnails->Rearrange();
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function copies a new document
//Step 1: Acquire images
void
CMainFrame::DoCopyDocumentStep1(bool pbExit) {

	oTrace->StartTrace(__WFUNCTION__);

	//Show reminder screen for registering
	//oCore->oRegistration->Reminder(hWnd);

	bCopyInProgress = true;
	bExitAfterCopy = pbExit;
	oCore->MemorizeCurrentDocument();

	//Acquire images
	oCore->oScanSettings->SetBool(eShowInterface, false, false);

	CDlgProgressAcquire* oDlgProgress;
	oDlgProgress = oCore->oDialogs->DlgProgressAcquire(hWnd, oCore->oScanSettings);
	delete oDlgProgress;

	bCopyInProgress = (oCore->oScanCore->ImagesAvailable()>0); 

	//if a file was acquired, set the copy in progress to true (no images will be available).
	CScanDevice* oSelectedDevice = oCore->oScanSettings->GetScanner();
	if (oSelectedDevice!=NULL) {

		if (oSelectedDevice->cInterface == eInputFile) {
			bCopyInProgress = true;
		}
	}


	oTrace->EndTrace(__WFUNCTION__);
	}

//This function copies a new document
//Step 2: Output images
void
CMainFrame::DoCopyDocumentStep2() {

	oTrace->StartTrace(__WFUNCTION__);

	DoOutputDocument(oCore->GetCurrentDocument());
	oCore->oDocuments->CloseDocument(oCore->GetCurrentDocument());

	bCopyInProgress = false;
	oCore->RestoreMemorizedDocument();

	if (bExitAfterCopy) {
	
		//Close this window
		PostQuitMessage(0);
		DestroyWindow(hWnd);
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function outputs the image to the selected device
void 
CMainFrame::DoOutputDocument(CCopyDocument* poDocument, bool bShowPrinterSelectionDialog) {

	sOutputDevice oOutputDevice;

	oTrace->StartTrace(__WFUNCTION__);

	oCore->oPrintSettings->GetDevice(oOutputDevice);

	if (poDocument!=NULL) {

		//Do the output
		switch (oOutputDevice.cType) {

			case eOutputFile:

				oCore->SaveDocument(hWnd, poDocument);
				break;

			case eOutputPDF:

				poDocument->ForceFiletype(ePDF);
				oCore->SaveDocument(hWnd, poDocument);
				poDocument->ForceFiletype(eAny);
				break;

			case eOutputMail:

				poDocument->MailTo();
				break;

			case eOutputPrinter:

				DoPrintDocument(poDocument, bShowPrinterSelectionDialog);
				break;
			}
		}

	oTrace->EndTrace(__WFUNCTION__);
	}


//This function prints the current document
void
CMainFrame::DoPrintDocument(CCopyDocument* poDocument, bool ShowInterface) {

	oTrace->StartTrace(__WFUNCTION__);

	CPrintSettings* oTemporarySettings;

	oTemporarySettings = oCore->oPrintSettings->Copy();
	oTemporarySettings->bDeleteAfterPrint = false;
	oTemporarySettings->bNotifyChanges = false;
	oTemporarySettings->SetBool(ePrintShowInterface, ShowInterface /*!bCopyInProgress*/);
			
	CDlgProgressPrint* oDlgProgress;
	oDlgProgress = oCore->oDialogs->DlgProgressPrint(hWnd, poDocument, oTemporarySettings);

	delete oDlgProgress;
	delete oTemporarySettings;

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function deletes the selected page(s)
void 
CMainFrame::DoDeletePages() {

	CCopyDocument* oDocument;
	int iItem;
	LVITEM cItem;

	oTrace->StartTrace(__WFUNCTION__);

	if (ListView_GetSelectedCount(oThumbnails->hWnd)>1) {

		oDocument = oCore->GetCurrentDocument();

		iItem = ListView_GetNextItem(oThumbnails->hWnd, 0, LVNI_SELECTED);

		while (iItem!=-1) {

			cItem.mask = LVIF_PARAM;
			cItem.iItem = iItem;
			ListView_GetItem(oThumbnails->hWnd, &cItem);
				
			oDocument->DoDeletePage(cItem.lParam, oThumbnails->hWnd);

			iItem = ListView_GetNextItem(oThumbnails->hWnd, iItem, LVNI_SELECTED);
			}
		}
	else {

		oCore->DoPageImageAction(ePageDelete, 0, hWnd);
		}	

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function processes the dropped files
void 
CMainFrame::DoAcceptDropFiles(HDROP phDrop) {

	UINT iNumberFiles;
	UINT iFilenameSize;
	wchar_t* cFilename;

	oTrace->StartTrace(__WFUNCTION__);

	iNumberFiles = DragQueryFile(phDrop, 0xFFFFFFFF, NULL, NULL);

	oTrace->Add(L"iNumberFiles", (int)iNumberFiles);

	while (iNumberFiles>0) {

		iNumberFiles--;

		iFilenameSize = DragQueryFile(phDrop, iNumberFiles, NULL, NULL);
		
		cFilename = new wchar_t[iFilenameSize+1];

		if (DragQueryFile(phDrop, iNumberFiles, cFilename, iFilenameSize+1)==iFilenameSize) {

			oCore->LoadDocument(hWnd, cFilename);
			}

		delete cFilename;
		}

	DragFinish(phDrop);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function opens the help function; either on the local disk or online
void
CMainFrame::DoOpenHelp() {

	oTrace->StartTrace(__WFUNCTION__);

	oCore->OpenHelp(hWnd);

	oTrace->EndTrace(__WFUNCTION__);
}

//This function processes the WM_COMMAND message
bool 
CMainFrame::OnCommand(int piCommand) {

	CCopyDocumentPage* oPage;
	CCopyDocument* oDocument;
	bool bResult;

	bResult = true;

	switch (piCommand) {

		//Acquire a new document
		case ID_FILE_NEW:
		case ID_SCAN_ACQUIRE:

			oCore->Acquire(hWnd);
			break;

		case ID_DOCUMENT_ACQUIRENEWPAGE:

			oCore->Acquire(hWnd, eAcquireNewPage);
			break;

		case ID_DOCUMENT_ACQUIRENEWPAGEANDINSERT:

			oCore->Acquire(hWnd, eAcquireNewPageAndInsert);
			break;

		case ID_DOCUMENT_ACQUIREODDPAGES:

			oCore->Acquire(hWnd, eAcquireOddPages);
			break;

		case ID_DOCUMENT_ACQUIREEVENPAGES:

			oCore->Acquire(hWnd, eAcquireEvenPages);
			break;

		case IDC_ACQUIRETOCOLLECTION:

			//retrieve the currently selection collection
			oCore->oScanSettings->iCollection = oDocumentCollectionList->GetSelectedCollection(); 
			oCore->oScanSettings->sFolder = L"";					

			//start the acquisition
			oCore->Acquire(hWnd, eAcquireNewDocumentToCollection);
			break;

		case IDC_ACQUIRETOCOLLECTIONFOLDER:

			//retrieve the currently selection collection
			oCore->oScanSettings->iCollection = oDocumentCollectionList->GetSelectedCollection(); 
			oCore->oScanSettings->sFolder = oFileContainer->oFileListview->GetSelectedFolder();									

			//start the acquisition
			oCore->Acquire(hWnd, eAcquireNewDocumentToCollection);
			break;

		//Open a file
		case ID_FILE_OPEN:

			if (oCore->LoadDocument(hWnd)) {
				ViewDocument(oCore->GetCurrentDocument());
				}
			break;

		//Save the current document
		case ID_FILE_SAVE:

			oCore->SaveDocument(hWnd, oCore->GetCurrentDocument());
			oDocumentList->SyncDocumentSelection();
			DoEnableControls();
			break;

		//Save as the current document
		case ID_FILE_SAVEAS:
			
			oDocument = oCore->GetCurrentDocument();

			if (oDocument!=NULL) {

				oDocument->SaveAs(hWnd);
				oDocumentList->SyncDocumentSelection();
				DoEnableControls();
				}

			break;

		//Close the current document
		case ID_FILE_CLOSE:

			int iDocumentId;
			CCopyDocument* oDocument;

			oDocument = oCore->GetCurrentDocument();
				
			if (oDocument!=NULL) {
				
				iDocumentId = oDocument->GetDocumentID();
				
				int iResult = oCore->oDocuments->SaveChangesToDirtyDocument(hWnd, oDocument, true);

				if (iResult != IDCANCEL) {
					oCore->CloseDocument(hWnd, oCore->GetCurrentDocument());

					oDocumentList->DeleteItem(iDocumentId);
					ViewDocument(oCore->GetCurrentDocument());
					}
				}

			break;

		//Print the current document
		case ID_FILE_PRINT_DIRECT:
		case ID_FILE_PRINT:

			if (oCore->GetCurrentDocument()) {
				//DoPrintDocument(oCore->GetCurrentDocument(), piCommand == ID_FILE_PRINT);
				DoOutputDocument(oCore->GetCurrentDocument(), piCommand == ID_FILE_PRINT);
				}
			break;

		//Set the state of the pageviewer to printpreview
		case ID_FILE_PRINT_PREVIEW:

			if (oPageViewer->GetPageViewType()==ePVTImage) {
				oPageViewer->SetPageViewType(ePVTPrintPreview);
				}
			else
				{
				oPageViewer->SetPageViewType(ePVTImage);
				}
			
			SendMessage(oCommandBar->hWnd, TB_PRESSBUTTON, 
						(WPARAM)ID_FILE_PRINT_PREVIEW, 
						(LPARAM)MAKELONG((oPageViewer->GetPageViewType()==ePVTPrintPreview), 0));
			DoEnableControls();
			break;

		//copy the selection
		case ID_EDIT_COPY: 

			oCore->DoPageImageAction(eImageCopy, 0, hWnd);
			break;

		//copy the selection
		case ID_EDIT_CUT: 

			DWORD dColor;

			dColor = (DWORD)oGlobalInstances.oRegistry->ReadInt(L"General", L"Color", 0xFFFFFF);
			oCore->DoPageImageAction(eImageCut, dColor, hWnd);
			break;

		//Paste a new document
		case ID_EDIT_PASTE: 
		//Paste a new page into the current document at the end of the document
		case ID_EDIT_PASTEASNEWPAGE: 
		//Paste a new page into the current document at the position of the selected page
		case ID_EDIT_PASTEASNEWPAGEANDINSERT: 

			if (oCore->PasteImage(hWnd, piCommand)) {

				oPage = oCore->GetCurrentPage();

				if (piCommand == ID_EDIT_PASTE) ViewDocument(oCore->GetCurrentDocument());
				}
			break;

		case ID_EDIT_COPYTEXTTOCLIPBOARD:
		
			oPage = oCore->GetCurrentPage();

			if (oPage!=NULL) {
				
				oPage->GetRecognitionLayer()->DoCopyTextToClipboard(hWnd);
				}
			break;

		case ID_EDIT_COPYTEXTBLOCKTOCLIPBOARD:

			oPage = oCore->GetCurrentPage();

			if (oPage!=NULL) {
				
				oPage->GetRecognitionLayer()->DoCopyTextBlockToClipboard(hWnd,  (int)oPageViewer->cImagePositionContextMenu.X,
																				(int)oPageViewer->cImagePositionContextMenu.Y);
				}
			break;

		//Delete a page
		case ID_DOCUMENT_DELETEPAGE:
			
			DoDeletePages();
			break;

		//Delete a page
		case ID_DOCUMENT_COPYPAGE:

			oCore->DoPageImageAction(ePageCopy, 0, hWnd);
			DoEnableControls();
			break;

		//Delete a page
		case ID_DOCUMENT_CUTPAGE:

			oCore->DoPageImageAction(ePageCut, 0, hWnd);
			DoEnableControls();
			break;

		//Split the current document
		case ID_DOCUMENT_SPLITDOCUMENT: 

			oCore->DoPageImageAction(ePageSplit, 0, hWnd);
			DoEnableControls();
			break;

		//Copy a new document
		case IDC_DOCOPYEXIT:

			DoCopyDocumentStep1(true);
			break;

		//Copy a new document
		case IDC_DOCOPY:

			DoCopyDocumentStep1(false);
			break;

		case IDC_DOOUTPUTDOCUMENT:

			if (oCore->GetCurrentDocument()) {
				DoOutputDocument(oCore->GetCurrentDocument());
				}
			break;

		//Select a scanner
		case ID_SCAN_SELECTSOURCE:

			oCore->oDialogs->DlgSelectSource(hWnd);
			break;

		//Convert the image to a grayscale
		case ID_CONVERTTO_GRAYSCALE:

			oCore->DoPageImageAction(eImageGrayscale, 0);
			break;

		//Convert the image to a grayscale
		case ID_CONVERTTO_BLACKWHITE:

			oCore->DoPageImageAction(eImageBlackWhite, 0);
			break;

		//Crop the current image
		case ID_IMAGE_CROP:

			oCore->DoPageImageAction(eImageCrop, 0);
			break;

		//Set the page viewer in color picking mode
		case ID_IMAGE_COLORPICKER:
			
			oPageViewer->SetColorPicker();
			break;

		//Open the color dialog box 
		case ID_IMAGE_CHOOSECOLOR:

			oCore->oDialogs->DlgColor(hWnd);
			break;

		//Fill the selection with the currently selected color
		case ID_IMAGE_FILLSELECTION:

			dColor = (DWORD)oGlobalInstances.oRegistry->ReadInt(L"General", L"Color", 0xFFFFFF);
			oCore->DoPageImageAction(eImageFillRectangle, dColor);
			break;

		//Recognize the text on the given page,
		//to do: convert it to a page image action
		case ID_IMAGE_RECOGNIZETEXT:

			oCore->oDialogs->DlgRecognizeText(hWnd, oCore->GetCurrentPage()->GetPageID());
			oPageViewer->Redraw();
			DoEnableControls();

			break;

		//Remove all text blocks from the current page
		case ID_PAGE_DELETETEXTBLOCKS:

			oPage = oCore->GetCurrentPage();

			if (oPage!=NULL) {

				oPage->GetRecognitionLayer()->Clear();
				}	

			oPageViewer->Redraw();
			DoEnableControls();
			break;

		//Remove all text blocks from the current page
		case ID_PAGE_DELETETEXTBLOCK:

			oPage = oCore->GetCurrentPage();

			if (oPage!=NULL) {

				oPage->GetRecognitionLayer()->DeleteRecognizedText((int)oPageViewer->cImagePositionContextMenu.X, 
																   (int)oPageViewer->cImagePositionContextMenu.Y);
				}	

			oPageViewer->Redraw();
			DoEnableControls();
			break;

		//Rotate the image by 90 degrees
		case ID_ROTATE_90:

			oCore->DoPageImageAction(eImageRotate, 90);
			break;

		//Rotate the image by 180 degrees
		case ID_ROTATE_180:

			oCore->DoPageImageAction(eImageRotate, 180);
			break;

		//Rotate the image by 270 degrees
		case ID_ROTATE_270:

			oCore->DoPageImageAction(eImageRotate, 270);
			break;

		//Flip the image horizontally
		case ID_ROTATEANDFLIP_FLIPHORIZONTALLY:

			oCore->DoPageImageAction(eImageFlipHorizontal, 0);
			break;

		//Flip the image vertically
		case ID_ROTATEANDFLIP_FLIPVERTICALLY:

			oCore->DoPageImageAction(eImageFlipVertical, 0);
			break;

		//Undo the last action
		case ID_IMAGE_UNDO:

			oCore->oUndoRedo->Undo();
			break;

		//Undo the last undo
		case ID_IMAGE_REDO:

			oCore->oUndoRedo->Redo();
			break;

		//Show the copy pane
		case ID_VIEW_PANECOPY:

			oPane->SetVisiblePane(oPane->oPaneCopy);
			SendMessage(oCommandBar->hWnd, TB_PRESSBUTTON, (WPARAM)ID_VIEW_PANECOPY, (LPARAM)MAKELONG(true, 0));
			DoEnableControls();
			break;

		//Show the image pane
		case ID_VIEW_PANEIMAGE:

			oPane->SetVisiblePane(oPane->oPaneImage);
			SendMessage(oCommandBar->hWnd, TB_PRESSBUTTON, (WPARAM)ID_VIEW_PANEIMAGE, (LPARAM)MAKELONG(true, 0));
			DoEnableControls();
			break;

		//Show the page pane
		case ID_VIEW_PANEPAGE:

			oPane->SetVisiblePane(oPane->oPaneDocument);
			SendMessage(oCommandBar->hWnd, TB_PRESSBUTTON, (WPARAM)ID_VIEW_PANEPAGE, (LPARAM)MAKELONG(true, 0));
			DoEnableControls();
			break;

		//Toggle the recognition layer view
		case ID_VIEW_RECOGNITIONLAYER:

			oPageViewer->ToggleRecognitionLayer();
			DoEnableControls();			
			break;

		//Show the about dialog
		case IDM_ABOUT:

			oCore->oDialogs->DlgAbout(hWnd);
			break;

		//Show the preferences dialog
		case ID_FILE_PREFERENCES:

			oCore->oDialogs->DlgPreferences(hWnd, eAllPreferences);
			break;

		//Change the zoom/magnification setting of the current page
		case ID_ZOOM_ZOOMTOFIT:
			oPageViewer->SetSizeToFit();
			break;
		case ID_ZOOM_ZOOMTOSELECTION:
			oPageViewer->SetMagnificationToSelection();
			break;
		case ID_ZOOM_10:
			oPageViewer->SetMagnification(0.1f);
			break;
		case ID_ZOOM_20:
			oPageViewer->SetMagnification(0.2f);
			break;
		case ID_ZOOM_33:
			oPageViewer->SetMagnification(0.33f);
			break;
		case ID_ZOOM_50:
			oPageViewer->SetMagnification(0.5f);
			break;
		case ID_ZOOM_75:
			oPageViewer->SetMagnification(0.75f);
			break;
		case ID_ZOOM_100:
			oPageViewer->SetMagnification(1.0f);
			break;
		case ID_ZOOM_125:
			oPageViewer->SetMagnification(1.25f);
			break;
		case ID_ZOOM_150:
			oPageViewer->SetMagnification(1.5f);
			break;
		case ID_ZOOM_200:
			oPageViewer->SetMagnification(2.0f);
			break;

		//Make changes to the selection on the page
		case ID_EDIT_CLEARSELECTION:
			oPageViewer->SelectionClear();
			break;
		case ID_EDIT_SWAPSELECTION:
			oPageViewer->SelectionSwap();
			break;
		case ID_EDIT_SELECTALL:
			oPageViewer->SelectionAll();
			break;
		case ID_EDIT_SELECTCONTENT:
			oPageViewer->SelectContent();
			break;

		//Change the language
		case ID_LANGUAGE_ENGLISH:
			oCore->SetPreferredLanguage(LANG_ENGLISH);
			break;
		case ID_LANGUAGE_DUTCH:
			oCore->SetPreferredLanguage(LANG_DUTCH);
			break;
		case ID_LANGUAGE_FRENCH:
			oCore->SetPreferredLanguage(LANG_FRENCH);
			break;
		case ID_LANGUAGE_ITALIAN:
			oCore->SetPreferredLanguage(LANG_ITALIAN);
			break;
		case ID_LANGUAGE_GERMAN:
			oCore->SetPreferredLanguage(LANG_GERMAN);
			break;

		case ID_MACRO_EDITANDEXECUTE:
			oCore->oDialogs->DlgMacroEditor(hWnd);
			break;

		case ID_HELP_CONTENTS:
			DoOpenHelp();
			break;

		//Switch to 'easy' mode
		case IDC_SWITCH:
			SaveSettings();
			oRebarControl->SaveRebarSettings();
			PostQuitMessage(2);
			DestroyWindow(hWnd);
			break;

		default:

			bResult = false;
			break;
		}

	return bResult;
	}

//This function disables/enables the menu commands
void 
CMainFrame::DoEnableControls() {

	CCopyDocument* oDocument;
	CCopyDocumentPage* oPage;
	sCopyDocumentSelection cSelection;
	bool bSelection;
	bool bPrintPreview;
	bool bText;
	bool bDirty;

	oDocument = oCore->GetCurrentDocument();
	oPage = oCore->GetCurrentPage();
	bSelection = false;
	bText = false;
	bPrintPreview = (oPageViewer->GetPageViewType()==ePVTPrintPreview);

	if (oPage) {

		oPage->GetSelection(cSelection);
		bSelection = cSelection.bEnabled;;
		bText = (oPage->GetRecognitionLayer()->GetRecognizedTextCount()>0);
		}

	bDirty = false;
	if (oDocument!=NULL) {
		bDirty = oDocument->GetDirty();
		}

	oMenuIcon->SetEnableMenuCommand(ID_FILE_SAVE, bDirty);
	oMenuIcon->SetEnableMenuCommand(ID_FILE_SAVEAS, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_FILE_CLOSE, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_FILE_PRINT, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_FILE_PRINT_DIRECT, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_FILE_PRINT_PREVIEW, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_FILE_PRINT, oDocument!=NULL);
	oMenuIcon->SetCheckMenuCommand(ID_FILE_PRINT_PREVIEW, bPrintPreview);
	oMenuIcon->SetCheckMenuCommand(ID_VIEW_RECOGNITIONLAYER, oPageViewer->GetRecogntionLayerVisible());

	oMenuIcon->SetEnableMenuCommand(ID_EDIT_CUT, oCore->CheckPageImageAction(eImageCut, 0) && !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_EDIT_COPY, oCore->CheckPageImageAction(eImageCopy, 0) && !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_EDIT_SELECTALL, oPage!=NULL && !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_EDIT_SELECTCONTENT, oPage!=NULL && !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_EDIT_CLEARSELECTION, bSelection && !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_EDIT_SWAPSELECTION, bSelection && !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_IMAGE_COLORPICKER, !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_EDIT_COPYTEXTTOCLIPBOARD, bText);
	oMenuIcon->SetEnableMenuCommand(ID_EDIT_COPYTEXTBLOCKTOCLIPBOARD, bText);
	oMenuIcon->SetEnableMenuCommand(ID_PAGE_DELETETEXTBLOCK, bText);
	oMenuIcon->SetEnableMenuCommand(ID_PAGE_DELETETEXTBLOCKS, bText);

	oMenuIcon->SetEnableMenuCommand(ID_CONVERTTO_BLACKWHITE, oCore->CheckPageImageAction(eImageBlackWhite, 0));
	oMenuIcon->SetEnableMenuCommand(ID_CONVERTTO_GRAYSCALE, oCore->CheckPageImageAction(eImageGrayscale, 0));

	oMenuIcon->SetEnableMenuCommand(ID_IMAGE_RECOGNIZETEXT, bSelection);
	oMenuIcon->SetEnableMenuCommand(ID_IMAGE_UNDO, oCore->oUndoRedo->GetUndoCount()>0);
	oMenuIcon->SetEnableMenuCommand(ID_IMAGE_REDO, oCore->oUndoRedo->GetRedoCount()>0);
	oMenuIcon->SetEnableMenuCommand(ID_IMAGE_CROP, oCore->CheckPageImageAction(eImageCrop, 0) && !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_IMAGE_CROP, oCore->CheckPageImageAction(eImageCrop, 0) && !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_IMAGE_FILLSELECTION, oCore->CheckPageImageAction(eImageFillRectangle, 0) && !bPrintPreview);
	oMenuIcon->SetEnableMenuCommand(ID_ROTATE_90 , oCore->CheckPageImageAction(eImageRotate, 90));
	oMenuIcon->SetEnableMenuCommand(ID_ROTATE_180, oCore->CheckPageImageAction(eImageRotate, 90));
	oMenuIcon->SetEnableMenuCommand(ID_ROTATE_270, oCore->CheckPageImageAction(eImageRotate, 90));
	oMenuIcon->SetEnableMenuCommand(ID_ROTATEANDFLIP_FLIPHORIZONTALLY, oCore->CheckPageImageAction(eImageFlipVertical, 0));
	oMenuIcon->SetEnableMenuCommand(ID_ROTATEANDFLIP_FLIPVERTICALLY, oCore->CheckPageImageAction(eImageFlipVertical, 0));

	oMenuIcon->SetEnableMenuCommand(ID_DOCUMENT_COPYPAGE, oCore->CheckPageImageAction(ePageCopy, 0));
	oMenuIcon->SetEnableMenuCommand(ID_DOCUMENT_CUTPAGE, oCore->CheckPageImageAction(ePageCut, 0));
	oMenuIcon->SetEnableMenuCommand(ID_DOCUMENT_DELETEPAGE, oCore->CheckPageImageAction(ePageDelete, 0));
	oMenuIcon->SetEnableMenuCommand(ID_DOCUMENT_SPLITDOCUMENT, oCore->CheckPageImageAction(ePageSplit, 0));

	oMenuIcon->SetCheckMenuCommand(ID_VIEW_PANECOPY, oPane->GetVisiblePane()->cType == ePaneCopy);
	oMenuIcon->SetCheckMenuCommand(ID_VIEW_PANEIMAGE, oPane->GetVisiblePane()->cType == ePaneImage);
	oMenuIcon->SetCheckMenuCommand(ID_VIEW_PANEPAGE, oPane->GetVisiblePane()->cType == ePaneDocument);

	oCommandBar->EnableButton(ID_FILE_SAVE, false);
	oCommandBar->EnableButton(ID_FILE_SAVE_AS, oDocument!=NULL);
	oCommandBar->EnableButton(ID_FILE_SAVE_COPY_AS, oDocument!=NULL);
	oCommandBar->EnableButton(ID_FILE_CLOSE, oDocument!=NULL);
	oCommandBar->EnableButton(ID_FILE_PRINT_DIRECT, oDocument!=NULL);
	oCommandBar->EnableButton(ID_FILE_PRINT_PREVIEW, oDocument!=NULL);
	oCommandBar->EnableButton(ID_IMAGE_UNDO, oCore->oUndoRedo->GetUndoCount()>0);
	oCommandBar->EnableButton(ID_IMAGE_REDO, oCore->oUndoRedo->GetRedoCount()>0);
	oCommandBar->EnableButton(ID_IMAGE_COLORPICKER, !bPrintPreview);

	if (oDocument!=NULL) {

		oCommandBar->EnableButton(ID_FILE_SAVE, oDocument->GetDirty());
		}

	DoEnableControlsPaste();
	DoEnableControlsMagnification();
	DoEnableControlsLanguage();

	//Process the main menu
	HMENU hMainMenu;
	CMenuIcon*  oMenuIcon;	
	
	hMainMenu = GetMenu(hWnd);

	oMenuIcon = oGlobalInstances.oMenuIcon;
	oMenuIcon->ProcessMenu(hMainMenu);
}

//This function disables/enables the menu commands
void 
CMainFrame::DoEnableControlsPaste() {

	bool bPaste;

	bPaste = oCore->PasteImageCheck();

	oMenuIcon->SetEnableMenuCommand(ID_EDIT_PASTE, bPaste);
	oMenuIcon->SetEnableMenuCommand(ID_EDIT_PASTEASNEWPAGE, bPaste && (oCore->GetCurrentDocument()!=NULL));
}

void 
CMainFrame::DoEnableControlsMagnification() {

	CCopyDocument* oDocument;
	CCopyDocumentPage* oPage;
	sCopyDocumentSelection cSelection;
	float fMagnification;
	bool  bSizeToFit;
	bool  bSelection;
	
	oDocument = oCore->GetCurrentDocument();

	bSelection = false;
	oPage = oCore->GetCurrentPage();

	if (oPage) {

		oPage->GetSelection(cSelection);
		bSelection = cSelection.bEnabled;
		}

	fMagnification = oPageViewer->GetMagnification();
	bSizeToFit = oPageViewer->GetSizeToFit();

	//Set the checkmarks
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_10, fMagnification==0.1f		&& !bSizeToFit);
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_20, fMagnification==0.2f		&& !bSizeToFit);
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_33, fMagnification==0.33f	&& !bSizeToFit);
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_50, fMagnification==0.5f		&& !bSizeToFit);
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_75, fMagnification==0.75f	&& !bSizeToFit);
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_100, fMagnification==1.0f	&& !bSizeToFit);
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_125, fMagnification==1.25f	&& !bSizeToFit);
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_150, fMagnification==1.5f	&& !bSizeToFit);
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_200, fMagnification==2.0f	&& !bSizeToFit);
	oMenuIcon->SetCheckMenuCommand(ID_ZOOM_ZOOMTOFIT, bSizeToFit);

	//Enable the controls
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_10, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_20, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_33, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_50, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_75, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_100, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_125, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_150, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_200, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_ZOOMTOFIT, oDocument!=NULL);
	oMenuIcon->SetEnableMenuCommand(ID_ZOOM_ZOOMTOSELECTION, oDocument!=NULL && bSelection);
	}

//This function checks the currently selected language
void 
CMainFrame::DoEnableControlsLanguage() {

	LANGID cCurrentLanguage;

	cCurrentLanguage = oCore->GetPreferredLanguage();

	oMenuIcon->SetCheckMenuCommand(ID_LANGUAGE_ENGLISH, PRIMARYLANGID(cCurrentLanguage)==LANG_ENGLISH);
	oMenuIcon->SetCheckMenuCommand(ID_LANGUAGE_DUTCH,	PRIMARYLANGID(cCurrentLanguage)==LANG_DUTCH);
	oMenuIcon->SetCheckMenuCommand(ID_LANGUAGE_FRENCH,	PRIMARYLANGID(cCurrentLanguage)==LANG_FRENCH);
	oMenuIcon->SetCheckMenuCommand(ID_LANGUAGE_GERMAN,	PRIMARYLANGID(cCurrentLanguage)==LANG_GERMAN);
	oMenuIcon->SetCheckMenuCommand(ID_LANGUAGE_ITALIAN, PRIMARYLANGID(cCurrentLanguage)==LANG_ITALIAN);
}

//This function updates the caption of the main window; it is based
//on the title of the current document
void 
CMainFrame::DoUpdateTitle() {

	CCopyDocument* oDocument;
	wchar_t cTitle[MAX_PATH];

	oDocument = oCore->GetCurrentDocument();

	if (oDocument!=NULL) {

		swprintf_s(cTitle, MAX_PATH, L"Copying Machine - %s", oDocument->GetTitle().c_str());
		}
	else {

		wcscpy_s(cTitle, MAX_PATH, L"Copying Machine");
		}

	SetWindowText(hWnd, cTitle);
}

void 
CMainFrame::DoChangeColor(DWORD pdColor) {

	//Update the color button
	Bitmap* oColorButton;
	Bitmap* oPaintToolBitmap;
	Graphics* oCanvas;
	COLORREF  cCurrentColor;
	HBITMAP hColorBitmap;
	SolidBrush* oBlackBrush;
	SolidBrush* oColorBrush;

	cCurrentColor = pdColor;

	oColorButton = new Bitmap(16, 16, PixelFormat32bppARGB);
	oPaintToolBitmap = CBitmap::LoadFromResource(L"PNG_TOOLPAINT", L"PNG", oGlobalInstances.hInstance);
	oBlackBrush = new SolidBrush(Color::Black);
	oColorBrush = new SolidBrush(Color(GetRValue(cCurrentColor), GetGValue(cCurrentColor), GetBValue(cCurrentColor)));

	oCanvas = new Graphics(oColorButton);

	oCanvas->DrawImage(oPaintToolBitmap, 0, 0);
	oCanvas->FillRectangle(oBlackBrush, 1, 12, 14, 5);
	oCanvas->FillRectangle(oColorBrush, 0, 12, 16, 4);

	delete oCanvas;

	oColorButton->GetHBITMAP(Color(0,0,0,0), &hColorBitmap);

	ImageList_Replace(oCommandBar->hDefaultImagelist,	11, hColorBitmap, NULL);
	ImageList_Replace(oCommandBar->hDisabledImagelist,	11, hColorBitmap, NULL);
	ImageList_Replace(oCommandBar->hHotImagelist,		11, hColorBitmap, NULL);

	oMenuIcon->ReplaceMenuCommandIcon(ID_IMAGE_CHOOSECOLOR, hColorBitmap);
	oMenuIcon->ProcessMenu(GetMenu(hWnd)); //update the main menu

	delete oColorButton;
	delete oPaintToolBitmap;
	delete oBlackBrush;
	delete oColorBrush;

	RedrawWindow(oCommandBar->hWnd, NULL, NULL, RDW_INVALIDATE);
	}

//This function changes the visibility between two windows
void 
CMainFrame::DoSwitchWindowVisibility(HWND phWnd1, HWND phWnd2) {

	if (IsWindowVisible(phWnd1)) {

		ShowWindow(phWnd2, SW_SHOW);
		ShowWindow(phWnd1, SW_HIDE);
		}
	else {

		ShowWindow(phWnd1, SW_SHOW);
		ShowWindow(phWnd2, SW_HIDE);
		}
}



