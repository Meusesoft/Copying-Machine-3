#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "Dialogs.h"
#include "shlobj.h"

CDlgPreferencesAutoSave::CDlgPreferencesAutoSave(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
	CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {

	bInitialised = false;
	bSuspendUpdate = false;

	oDocumentCollections = new CDocumentCollections(poGlobalInstances);
	oDocumentCollections->EnableNotifications(false);
	}

CDlgPreferencesAutoSave::~CDlgPreferencesAutoSave(void)
{
	if (hDlg!=NULL) DestroyWindow(hDlg);
	delete oDocumentCollections;
}

void 
CDlgPreferencesAutoSave::OnInitDialog() {

	//add validation to the edit control
	HWND hEditControl;
	hEditControl = GetDlgItem(hDlg, IDC_EDITASFILENAME);

	oEditControlValidation = new sEditControlValidation;
	oEditControlValidation->oGlobalInstances = &oGlobalInstances;
	oEditControlValidation->lOldProc = SetWindowLong(hEditControl , GWLP_WNDPROC, (LONG)EditValidateFilename);
	SetWindowLong(hEditControl, GWLP_USERDATA, (long)oEditControlValidation);

	DoFillControls();
	DoValidateControls();

	bInitialised = true;
	}

//this function fills/initialises the controls of the dialog
void 
CDlgPreferencesAutoSave::DoFillControls() {

	CCopyingMachineCore* oCmCore;
	sDocumentCollection oDocumentCollection;

	oCmCore = (CCopyingMachineCore*)oCore;
	
	SendDlgItemMessage(hDlg, IDC_ENABLEAUTOSAVE, BM_SETCHECK, oGlobalInstances.oRegistry->ReadInt(L"General", L"AutoSave", 0) == 0 ? BST_UNCHECKED : BST_CHECKED, 0);
	
	//Fill the dropdown box with the document types
	DoSynchronizeComboBox(bInitialised ? -1 : 1); //default
	DoFillFileTypeCombo(); 
	DoFillColorDepth(); 
	DoFillResolution();
	DoFillDocumentTypeForm(0); //default
}
	
//This function fills the resolution combobox
void
CDlgPreferencesAutoSave::DoFillResolution() {

	HWND hResolutionBox;
	int iItemIndex;

	hResolutionBox = GetDlgItem(hDlg, IDC_COMBOASRESOLUTION);
	
	iItemIndex = SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"75");
	SendMessage(hResolutionBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)75);	
	iItemIndex = SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"100");
	SendMessage(hResolutionBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)100);	
	iItemIndex = SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"150");
	SendMessage(hResolutionBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)150);	
	iItemIndex = SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"200");
	SendMessage(hResolutionBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)200);	
	iItemIndex = SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"300");
	SendMessage(hResolutionBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)300);	
	iItemIndex = SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"400");
	SendMessage(hResolutionBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)400);	
	iItemIndex = SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"600");
	SendMessage(hResolutionBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)600);	
	iItemIndex = SendMessage(hResolutionBox, CB_ADDSTRING, 0, (LPARAM)L"1200");
	SendMessage(hResolutionBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)1200);	
	}

//This function fills the color depth combobox
void
CDlgPreferencesAutoSave::DoFillColorDepth() {

	wchar_t cText[100];
	int iItemIndex;
	int iSelIndex;
	int piColorDepth;
	HWND hColorBox;

	hColorBox = GetDlgItem(hDlg, IDC_COMBOASCOLOR);
	iSelIndex = 0;
	piColorDepth = 2;

	LoadString(oGlobalInstances.hLanguage, IDS_BLACKWHITE, cText, 100);
	iItemIndex=(int)SendMessage(hColorBox, CB_ADDSTRING, 0, (LPARAM)cText);
	if (iItemIndex!=CB_ERR) {
		SendMessage(hColorBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)0); //BW
		if (piColorDepth == iItemIndex) iSelIndex = iItemIndex;
		}
			
	LoadString(oGlobalInstances.hLanguage, IDS_GRAY, cText, 100);
	iItemIndex=(int)SendMessage(hColorBox, CB_ADDSTRING, 0, (LPARAM)cText);
	if (iItemIndex!=CB_ERR) {
		SendMessage(hColorBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)1); //BW
		if (piColorDepth == iItemIndex) iSelIndex = iItemIndex;
		}

	LoadString(oGlobalInstances.hLanguage, IDS_COLOR, cText, 100);
	iItemIndex=(int)SendMessage(hColorBox, CB_ADDSTRING, 0, (LPARAM)cText);
	if (iItemIndex!=CB_ERR) {
		SendMessage(hColorBox, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)2); //BW
		if (piColorDepth == iItemIndex) iSelIndex = iItemIndex;
		}

	if (SendMessage(hColorBox, CB_SETCURSEL, (WPARAM)iSelIndex, 0)==CB_ERR) {
		SendMessage(hColorBox, CB_SETCURSEL, (WPARAM)0, 0);
		}
	}

BOOL 
CDlgPreferencesAutoSave::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

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
CDlgPreferencesAutoSave::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	if (!bInitialised) return FALSE;

	switch (iCommand) {

		case IDC_SAVETYPE:

			switch (HIWORD(wParam)) {

				case CBN_SELCHANGE:
					OnChangeDocumentType();			
					break;
				}
			break;

		case IDC_EDITASNAME:

			if (HIWORD(wParam)==EN_KILLFOCUS) {
				OnKillFocusEditName();
				}

		case IDC_EDITASFILENAME:
		case IDC_EDITASLOCATION:

			if (HIWORD(wParam)==EN_CHANGE) {
				
				DoProcessChanges();
				SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
				}
			break;

		case IDC_COMBOASFILETYPE:
		case IDC_COMBOASCOLOR:
		case IDC_COMBOASRESOLUTION:

			DoProcessChanges();
			SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
			break;

		case IDC_DELETEDOCUMENTTYPE:
			OnDeleteDocumentCollection();
			break;

		case IDC_ADDTAG:
			DoShowTagMenu();
			break;
		
		case IDC_BROWSE:
			DoBrowseForDefaultFolder();
		case IDC_ENABLEAUTOSAVE:
			SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
			break;

		case IDC_TAGYEAR:
			DoAddTag(L"%year%");
			break;
		case IDC_TAGMONTH:
			DoAddTag(L"%month%");
			break;
		case IDC_TAGDAY:
			DoAddTag(L"%day%");
			break;
		case IDC_TAGHOUR:
			DoAddTag(L"%hour%");
			break;
		case IDC_TAGMINUTES:
			DoAddTag(L"%minutes%");
			break;
		case IDC_TAGSECONDS:
			DoAddTag(L"%seconds%");
			break;
		case IDC_TAGCOUNTER:
			DoAddTag(L"%counter%");
			break;
		case IDC_TAGTYPE:
			DoAddTag(L"%collection%");
			break;
		case IDC_TAGNAME:
			DoAddTag(L"%name%");
			break;
			}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesAutoSave::OnCloseOk() {


	}

//This function handles the localize action
void 
CDlgPreferencesAutoSave::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	LocalizeControl(IDC_ENABLEAUTOSAVE,					IDS_AUTOMATICALLYSAVE);
	LocalizeControl(IDC_SAVETYPE,						IDS_DOCUMENTTYPE);
	LocalizeControl(IDC_STATICDOCUMENTTYPENAME,			IDS_NAME);
	LocalizeControl(IDC_STATICASFILENAME,				IDS_FORMATFILENAME);
	LocalizeControl(IDC_STATICASLOCATION,				IDS_DEFAULTFOLDER);
	LocalizeControl(IDC_DELETEDOCUMENTTYPE,				IDS_DELETE);
	LocalizeControl(IDC_ADDTAG,							IDS_TAGS);
	LocalizeControl(IDC_STATICDOCUMENTTYPE,				IDS_DOCUMENTTYPE);
	LocalizeControl(IDC_STATICASFILETYPE,				IDS_PREFERREDFILETYPE);
	LocalizeControl(IDC_STATICDOCUMENTTYPERESOLUTION,	IDS_RESOLUTION);
	LocalizeControl(IDC_STATICDOCUMENTTYPECOLOR,		IDS_COLORDEPTH);

	//LocalizeControl(IDC_TOPIC1, IDS_SETTINGS);
	}

//This function handles the Apply event
void 
CDlgPreferencesAutoSave::OnApply() {

	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	oGlobalInstances.oRegistry->WriteInt(L"General", L"AutoSave", SendDlgItemMessage(hDlg, IDC_ENABLEAUTOSAVE, BM_GETCHECK, 0, 0)==BST_CHECKED ? 1 : 0);
	
	oDocumentCollections->Save();

	//reload the core document collections to update it with the latest changes
	oCmCore->oDocumentCollections->Load();
	}

void
CDlgPreferencesAutoSave::DoValidateControls() {

	}

//This function opens the browse dialog box so the user can
//select a default folder
void 
CDlgPreferencesAutoSave::DoBrowseForDefaultFolder() {

	BROWSEINFO cBrowseInfo;
	wchar_t cSelectedFolderName[MAX_PATH+1];
	wchar_t cTitle[MAX_PATH+1];
	PIDLIST_ABSOLUTE cPIDL;

	//initialise the browse structure
	SecureZeroMemory(&cBrowseInfo, sizeof(BROWSEINFO));

	cBrowseInfo.hwndOwner = hDlg;
	cBrowseInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS; 
	cBrowseInfo.pszDisplayName = cSelectedFolderName;

	LoadString(oGlobalInstances.hLanguage, IDS_BROWSEHELPDEFAULT, cTitle, MAX_PATH);
	cBrowseInfo.lpszTitle = cTitle;

	cPIDL = SHBrowseForFolder(&cBrowseInfo);

	if (cPIDL!=NULL) {

		if (SHGetPathFromIDList(cPIDL, cSelectedFolderName)) {

			SendDlgItemMessage(hDlg, IDC_EDITASLOCATION, WM_SETTEXT, 0, (LPARAM)cSelectedFolderName);
			}
		}
	}

//This function process the event of the selection of another document type
void 
CDlgPreferencesAutoSave::OnChangeDocumentType() {

	int iItemIndex;
	int iSelectedItem;
	int iComboBoxItem;
	wchar_t cText[255];
	sDocumentCollection oDocumentCollection;

	bSuspendUpdate = true;

	//get the current selected item
	iSelectedItem = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETCURSEL, 0, 0);
	iItemIndex = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETITEMDATA, (WPARAM)iSelectedItem, 0);
	
	if (iItemIndex==-1) {

		//Create a new document type

		//Get the default value
		oDocumentCollections->GetDocumentCollection(0, oDocumentCollection);

		LoadString(oGlobalInstances.hLanguage, IDS_NEW, cText, 254);
		oDocumentCollection.sName = cText;
		iItemIndex = oDocumentCollections->AddDocumentCollection(oDocumentCollection);
		
		//add the item to the combobox
		iComboBoxItem = (long)SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETCOUNT, 0, 0);
		iComboBoxItem = (long)SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_INSERTSTRING, (WPARAM)iComboBoxItem, (LPARAM)oDocumentCollection.sName.c_str());
		SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_SETITEMDATA, (WPARAM)iComboBoxItem, (LPARAM)iItemIndex);
		
		SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_SETCURSEL, (WPARAM)iComboBoxItem, 0);
		}

	//get the current selected item
	iSelectedItem = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETCURSEL, 0, 0);
	iItemIndex = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETITEMDATA, (WPARAM)iSelectedItem, 0);

	//update the documenttype form
	DoFillDocumentTypeForm(iItemIndex);

	bSuspendUpdate = false;
	}

//This function deletes the selected document type
void 
CDlgPreferencesAutoSave::OnDeleteDocumentCollection() {

	int iSelectedItem;
	int iItemIndex;

	//get the current selected item
	iSelectedItem = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETCURSEL, 0, 0);
	iItemIndex = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETITEMDATA, (WPARAM)iSelectedItem, 0);

	oDocumentCollections->DeleteDocumentCollection(iItemIndex);

	DoSynchronizeComboBox(1); //default
	DoFillDocumentTypeForm(0); //default
	DoFillColorDepth(); //default;

	SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
	}

//This function rebuilds the combobox with documenttypes so that it is in sync again
//with the data in the autosave object
void 
CDlgPreferencesAutoSave::DoSynchronizeComboBox(int piSelectedItem) {

	int iSelectedItem;
	sDocumentCollection oDocumentCollection;
	wchar_t cText[255];

	iSelectedItem = piSelectedItem;

	if (iSelectedItem==-1) iSelectedItem = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETCURSEL, 0, 0);
	
	SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_RESETCONTENT, 0, 0);

	//Fill the dropdown box with the document types
	long lIndex;
	long lStringIndex;

	lIndex = (long)oDocumentCollections->GetDocumentCollectionCount();

	while (lIndex>0) {

		lIndex--;

		oDocumentCollections->GetDocumentCollection(lIndex, oDocumentCollection);

		lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_INSERTSTRING, (WPARAM)0, (LPARAM)oDocumentCollection.sName.c_str());
		SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)lIndex);
		}

	LoadString(oGlobalInstances.hLanguage, IDS_NEWDOCUMENTTYPE, cText, 254);
	lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_INSERTSTRING, (WPARAM)0, (LPARAM)cText);
	SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)-1);

	SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_SETCURSEL, (WPARAM)iSelectedItem, 0);
	}

//This function fills the document type form
void 
CDlgPreferencesAutoSave::DoFillDocumentTypeForm(int piItem) {

	sDocumentCollection oDocumentCollection;

	//Enable/disable controls
	EnableControl(IDC_EDITASNAME, oDocumentCollections->CheckNameEditable(piItem));
	EnableControl(IDC_DELETEDOCUMENTTYPE, oDocumentCollections->CheckDocumentTypeDeletable(piItem));

	//Fill the edit controls
	oDocumentCollections->GetDocumentCollection(piItem, oDocumentCollection);

	SendDlgItemMessage(hDlg, IDC_EDITASNAME,	 WM_SETTEXT, 0, (LPARAM)oDocumentCollection.sName.c_str());
	SendDlgItemMessage(hDlg, IDC_EDITASFILENAME, WM_SETTEXT, 0, (LPARAM)oDocumentCollection.sFilenameTemplate.c_str());
	SendDlgItemMessage(hDlg, IDC_EDITASLOCATION, WM_SETTEXT, 0, (LPARAM)oDocumentCollection.sLocation.c_str());
	DoSelectFileType(oDocumentCollection.cFileType);
	DoSelectColorDepth(oDocumentCollection.iColorDepth);
	DoSelectResolution(oDocumentCollection.iResolution);
	}

//This function process the kill focus event of the name edit control. It updates the combobox
void 
CDlgPreferencesAutoSave::OnKillFocusEditName() {

	//update the contents of the combobox
	DoSynchronizeComboBox(-1);
	}


//This function update the document type by the values in the form.
void 
CDlgPreferencesAutoSave::DoProcessChanges() {

	int iItemIndex;
	int iSelectedItem;
	int iTextLength;
	int iItemData;
	wchar_t* cText;
	sDocumentCollection oDocumentCollection;

	if (!bSuspendUpdate) {

		//get the current selected item
		iSelectedItem = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETCURSEL, 0, 0);
		iItemIndex = SendDlgItemMessage(hDlg, IDC_SAVETYPE, CB_GETITEMDATA, (WPARAM)iSelectedItem, 0);

		//get the item from the autosave object
		oDocumentCollections->GetDocumentCollection(iItemIndex, oDocumentCollection);

		//read the text from the edit controls
		iTextLength = SendDlgItemMessage(hDlg, IDC_EDITASNAME, WM_GETTEXTLENGTH, 0, 0);
		cText = new wchar_t[iTextLength+2];
		SendDlgItemMessage(hDlg, IDC_EDITASNAME, WM_GETTEXT, (WPARAM)iTextLength+1, (LPARAM)cText);
		oDocumentCollection.sName = cText;
		delete[] cText;
		
		iTextLength = SendDlgItemMessage(hDlg, IDC_EDITASFILENAME, WM_GETTEXTLENGTH, 0, 0);
		cText = new wchar_t[iTextLength+2];
		SendDlgItemMessage(hDlg, IDC_EDITASFILENAME, WM_GETTEXT, (WPARAM)iTextLength+1, (LPARAM)cText);
		oDocumentCollection.sFilenameTemplate = cText;
		delete[] cText;

		iTextLength = SendDlgItemMessage(hDlg, IDC_EDITASLOCATION, WM_GETTEXTLENGTH, 0, 0);
		cText = new wchar_t[iTextLength+2];
		SendDlgItemMessage(hDlg, IDC_EDITASLOCATION, WM_GETTEXT, (WPARAM)iTextLength+1, (LPARAM)cText);
		oDocumentCollection.sLocation = cText;
		delete[] cText;

		//filetype
		iSelectedItem = SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_GETCURSEL, 0, 0);
		iItemData = SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_GETITEMDATA, (WPARAM)iSelectedItem, 0);
		oDocumentCollection.cFileType = (eCopyDocumentFiletype)iItemData;

		//colordepth
		iSelectedItem = SendDlgItemMessage(hDlg, IDC_COMBOASCOLOR, CB_GETCURSEL, 0, 0);
		iItemData = SendDlgItemMessage(hDlg, IDC_COMBOASCOLOR, CB_GETITEMDATA, (WPARAM)iSelectedItem, 0);
		oDocumentCollection.iColorDepth = iItemData;

		//resolution
		iSelectedItem = SendDlgItemMessage(hDlg, IDC_COMBOASRESOLUTION, CB_GETCURSEL, 0, 0);
		iItemData = SendDlgItemMessage(hDlg, IDC_COMBOASRESOLUTION, CB_GETITEMDATA, (WPARAM)iSelectedItem, 0);
		oDocumentCollection.iResolution = iItemData;

		//update the item in the autosave object
		oDocumentCollections->SetDocumentCollection(iItemIndex, oDocumentCollection);
		}
	}

//This function shows the menu with all the available tags
void 
CDlgPreferencesAutoSave::DoShowTagMenu() {

	wchar_t	cMenuItemText[96];
	HWND	hTagButton;
	RECT	cTagRect;
	HMENU	hTagMenu;
	
	oTrace->StartTrace(__WFUNCTION__);

	hTagMenu = CreatePopupMenu();

	if (hTagMenu!=NULL) {
		
		//Build the menu
		LoadString(oGlobalInstances.hLanguage, IDS_TAGNAME, cMenuItemText, 96);
		AppendMenu(hTagMenu, MF_STRING/* | uFlags*/, IDC_TAGNAME, cMenuItemText);
		LoadString(oGlobalInstances.hLanguage, IDS_TAGCOUNTER, cMenuItemText, 96);
		AppendMenu(hTagMenu, MF_STRING/* | uFlags*/, IDC_TAGCOUNTER, cMenuItemText);
		LoadString(oGlobalInstances.hLanguage, IDS_TAGTYPE, cMenuItemText, 96);
		AppendMenu(hTagMenu, MF_STRING/* | uFlags*/, IDC_TAGTYPE, cMenuItemText);

		AppendMenu(hTagMenu, MF_SEPARATOR, 0, 0);

		LoadString(oGlobalInstances.hLanguage, IDS_TAGYEAR, cMenuItemText, 96);
		AppendMenu(hTagMenu, MF_STRING/* | uFlags*/, IDC_TAGYEAR, cMenuItemText);
		LoadString(oGlobalInstances.hLanguage, IDS_TAGMONTH, cMenuItemText, 96);
		AppendMenu(hTagMenu, MF_STRING/* | uFlags*/, IDC_TAGMONTH, cMenuItemText);
		LoadString(oGlobalInstances.hLanguage, IDS_TAGDAY, cMenuItemText, 96);
		AppendMenu(hTagMenu, MF_STRING/* | uFlags*/, IDC_TAGDAY, cMenuItemText);
		LoadString(oGlobalInstances.hLanguage, IDS_TAGHOUR, cMenuItemText, 96);
		AppendMenu(hTagMenu, MF_STRING/* | uFlags*/, IDC_TAGHOUR, cMenuItemText);
		LoadString(oGlobalInstances.hLanguage, IDS_TAGMINUTES, cMenuItemText, 96);
		AppendMenu(hTagMenu, MF_STRING/* | uFlags*/, IDC_TAGMINUTES, cMenuItemText);
		LoadString(oGlobalInstances.hLanguage, IDS_TAGSECONDS, cMenuItemText, 96);
		AppendMenu(hTagMenu, MF_STRING/* | uFlags*/, IDC_TAGSECONDS, cMenuItemText);

		//Get the window rectangle of the button
		hTagButton = GetDlgItem(hDlg, IDC_ADDTAG);
		GetWindowRect(hTagButton, &cTagRect);

		//Execute the popup menu
		TrackPopupMenu(hTagMenu, TPM_LEFTALIGN | TPM_TOPALIGN, cTagRect.left, cTagRect.bottom, 0, hDlg, 0);
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function adds the given tag to the edit control. It will be
//inserted at the position the cursor is.
void 
CDlgPreferencesAutoSave::DoAddTag(std::wstring psTag) {

	//insert the tag at the insertion point, or replace the current selection
	SendDlgItemMessage(hDlg, IDC_EDITASFILENAME, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)psTag.c_str());

	//enable the apply button
	SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
	}

//This function fills the filetype combobox
void 
CDlgPreferencesAutoSave::DoFillFileTypeCombo() {

	long lStringIndex;
	wchar_t cFileTypeText[50];

	LoadString(oGlobalInstances.hLanguage, IDS_FILETYPETIFF, cFileTypeText, 49);
	lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)cFileTypeText);
	SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)eTIFFUncompressed);
	LoadString(oGlobalInstances.hLanguage, IDS_FILETYPETIFFCOMPRESSED, cFileTypeText, 49);
	lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)cFileTypeText);
	SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)eTIFFCompressed);
	LoadString(oGlobalInstances.hLanguage, IDS_FILETYPEPDF, cFileTypeText, 49);
	lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)cFileTypeText);
	SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)ePDF);
	LoadString(oGlobalInstances.hLanguage, IDS_FILETYPEJPG, cFileTypeText, 49);
	lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)cFileTypeText);
	SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)eJPG);
	LoadString(oGlobalInstances.hLanguage, IDS_FILETYPEPNG, cFileTypeText, 49);
	lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)cFileTypeText);
	SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)ePNG);
	LoadString(oGlobalInstances.hLanguage, IDS_FILETYPEPCX, cFileTypeText, 49);
	lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)cFileTypeText);
	SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)ePCX);
	LoadString(oGlobalInstances.hLanguage, IDS_FILETYPEBMP, cFileTypeText, 49);
	lStringIndex = (long)SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_ADDSTRING, (WPARAM)0, (LPARAM)cFileTypeText);
	SendDlgItemMessage(hDlg, IDC_COMBOASFILETYPE, CB_SETITEMDATA, (WPARAM)lStringIndex, (LPARAM)eBMP);
}

//This function selects the requested filetype in the combobox
void 
CDlgPreferencesAutoSave::DoSelectFileType(int pcFileType) {

	DoSelectComboboxItem(IDC_COMBOASFILETYPE, pcFileType);
	}

//This function selects the requested color depth in the colorbox
void 
CDlgPreferencesAutoSave::DoSelectColorDepth(int piColorDepth) {

	DoSelectComboboxItem(IDC_COMBOASCOLOR, piColorDepth);
	}

//This function selects the requested color depth in the colorbox
void 
CDlgPreferencesAutoSave::DoSelectResolution(int piResolution) {

	DoSelectComboboxItem(IDC_COMBOASRESOLUTION, piResolution);
	}

//This function select the item in the requested combobox
void 
CDlgPreferencesAutoSave::DoSelectComboboxItem(int piComboId, int piItem) {

	bool bFound;
	long lIndex;
	long lItemData;

	lIndex = SendDlgItemMessage(hDlg, piComboId, CB_GETCOUNT, 0, 0);
	bFound = false;

	while (!bFound && lIndex>=0) {

		lIndex--;
		
		lItemData = SendDlgItemMessage(hDlg, piComboId, CB_GETITEMDATA, (WPARAM)lIndex, 0);

		bFound = (lItemData == (int)piItem);
		}

	if (bFound) SendDlgItemMessage(hDlg, piComboId, CB_SETCURSEL, (WPARAM)lIndex, 0);
	}









