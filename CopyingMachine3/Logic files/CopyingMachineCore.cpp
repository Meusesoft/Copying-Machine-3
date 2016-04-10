#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "Wincrypt.h"

CCopyingMachineCore::CCopyingMachineCore(sGlobalInstances pInstances) :
	CScriptObject(L"Application")
{
	//set the helper classes
	oTrace = pInstances.oTrace;
	oRegistry = pInstances.oRegistry;
	pInstances.hLanguage = LoadLanguage();

	memcpy(&oGlobalInstances, &pInstances, sizeof(pInstances));

	oTrace->StartTrace(__WFUNCTION__);

	//create a hash of the executable, to check if the executable hasn't
	//changed
	DoHashExecutable();

	pInstances.oCopyingMachineCore = (void*)this;

	//create the core classes
	oNotifications =		new CCoreNotifications();
	pInstances.oNotifications = (void*)oNotifications;
	oDocuments =			new CCopyDocuments(pInstances);
	oUndoRedo =				new CCopyPageUndoRedo(pInstances); 
	oPrintSettings =		new CPrintSettings(pInstances);
	oPrintCore =			new CPrintCore(pInstances);
	oScanSettings =			new CScanSettings(pInstances);
	oScanCore =				new CScanCore(pInstances, oScanSettings);
	oOCRCore =				new COCRCore(pInstances);
	oDialogs =				new CDialogs(pInstances);
	oRegistration =			new CRegistration(pInstances);
	oDocumentCollections =	new CDocumentCollections(pInstances);
	oScriptProcessor	=		new CScriptProcessor(pInstances);

	//load the settings
	oPrintSettings->Load();
	oScanSettings->Load();
	oDocumentCollections->Load();
	CScanDevice* oCurrentDevice;
	oCurrentDevice = oScanSettings->GetScanner();
	oScanCore->SetCurrentScanner(oCurrentDevice);

	//initialise the variables
	lCurrentDocument = -1;
	iDocumentIDCounter = 1;
	DoGetCopyingMachineVersion(cExecutableInformation);

	//Register script objects
	RegisterChild(oScanCore);
	RegisterChild(oScanSettings);
	RegisterFunction(L"exit", true);
	RegisterFunction(L"currentdocument", true);
	RegisterFunction(L"currentpage", true);

	oTrace->EndTrace(__WFUNCTION__);
}

CCopyingMachineCore::~CCopyingMachineCore(void)
{
	oTrace->StartTrace(__WFUNCTION__);

	oScanSettings->Save();
	oPrintSettings->Save();
	oDocumentCollections->Save();

	delete oDialogs;
	delete oScriptProcessor;
	delete oOCRCore;
	delete oScanCore;
	delete oScanSettings;
	delete oPrintCore;
	delete oPrintSettings;
	delete oDocuments;
	delete oUndoRedo;
	delete oRegistration;
	delete oNotifications;

	FreeLibrary(oGlobalInstances.hLanguage);

	oTrace->EndTrace(__WFUNCTION__);
}

//This function sets the current document
bool
CCopyingMachineCore::SetCurrentDocument(long plIndex) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	//Check if the index is valid given the current document
	if (lCurrentDocument != plIndex) {
		
		lCurrentDocument = plIndex;
		if (lCurrentDocument<0) lCurrentDocument = 0;
		if (lCurrentDocument>=oDocuments->GetDocumentCount()) lCurrentDocument = oDocuments->GetDocumentCount()-1;	
		
		oNotifications->CommunicateEvent(eNotificationDocumentViewChanges, NULL);
		
		lCurrentPage=-1;
		SetCurrentPage(0);
		}

	bResult = true;
	
	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

bool
CCopyingMachineCore::SetCurrentDocumentByID(long plIndex) {

	bool bResult;
	long lDocumentIndex;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;
	lDocumentIndex = -1;

	//Find the document index
	for (int iIndex=0; iIndex<oDocuments->GetDocumentCount() && lDocumentIndex==-1; iIndex++) {

		if (oDocuments->GetDocument(iIndex)->GetDocumentID() == plIndex) lDocumentIndex=iIndex;
		}

	if (lDocumentIndex!=-1) bResult = SetCurrentDocument(lDocumentIndex);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}


//This function returns the current document
CCopyDocument* 
CCopyingMachineCore::GetCurrentDocument() {

	CCopyDocument* oResult;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oResult = NULL;

	//check if the current document is a valid number, if not then make it so
	if (lCurrentDocument>=oDocuments->GetDocumentCount()) SetCurrentDocument(oDocuments->GetDocumentCount()-1);
	
	//get the current document from the vector or return NULL if no current document
	//is selected
	if (lCurrentDocument<0) {
		
		oResult = NULL;
		}
	else {

		oResult = oDocuments->GetDocument(lCurrentDocument);
		}
	
	oTrace->EndTrace(__WFUNCTION__, (bool)(oResult!=NULL), eAll);

	return oResult;
	}

//This function sets the current page of the current document
bool 
CCopyingMachineCore::SetCurrentPage(long plIndex) {

	bool bResult;
	CCopyDocument* oDocument;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	oDocument = GetCurrentDocument();
	
	if (oDocument!=NULL) {

		//Check if the index is valid given the current document
		if (lCurrentPage != plIndex) {
			
			lCurrentPage = plIndex;
			if (lCurrentPage<0) lCurrentPage = 0;
			if (lCurrentPage>=oDocument->GetPageCount()) lCurrentPage = oDocument->GetPageCount()-1;	

			oNotifications->CommunicateEvent(eNotificationPageViewChanges, NULL);
			}

		bResult = true;
		}	
	
	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}


//This function sets the current page of the current document
bool 
CCopyingMachineCore::SetCurrentPageByID(DWORD pdPageId) {

	bool bResult;
	long lIndex;
	CCopyDocument* oDocument;
	CCopyDocumentPage* oPage;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;
	oDocument = GetCurrentDocument();
	
	if (oDocument!=NULL) {

		lIndex = oDocument->GetPageCount();

		while (!bResult && lIndex>=0) {

			lIndex--;
			
			oPage = oDocument->GetPage(lIndex);
			if (oPage!=NULL) {
	
				if (oPage->GetPageID() == pdPageId) {

					//Set the current page based on the index
					bResult = SetCurrentPage(lIndex);
					}
				}
			}
		}	
	
	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function returns the current page in the current document
CCopyDocumentPage* 
CCopyingMachineCore::GetCurrentPage() {

	CCopyDocumentPage* oResult;
	CCopyDocument* oDocument;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oResult = NULL;
	oDocument = GetCurrentDocument();

	if (oDocument!=NULL) {

		//check if the current page is a valid number, if not then make it so
		if (lCurrentPage>=oDocument->GetPageCount()) lCurrentPage = oDocument->GetPageCount()-1;
			
		//get the current page from the vector or return NULL if no current document
		//is selected
		if (lCurrentPage<0) {
			
			oResult = NULL;
			}
		else {

			oResult = oDocument->GetPage(lCurrentPage);
			}
		}
	
	oTrace->EndTrace(__WFUNCTION__, (bool)(oResult!=NULL), eAll);

	return oResult;
	}

//the message handler for the core. For TWAIN communication the message handler of windows
//must be used.
bool 
CCopyingMachineCore::ProcessMessage(MSG &pMessage) {

	bool bResult;

	bResult = false;
	
	if (oScanCore!=NULL) bResult = oScanCore->ProcessMessage(pMessage);

	return bResult;
	}

//This function starts the acquire process. It uses the cCurrentDevice and the oScanSettings classes
//and structures for its settings.
bool
CCopyingMachineCore::Acquire(HWND phWnd, eAcquireType pcAcquireType, CScanSettings* poScanSettings) {

	bool bResult;
	CScanSettings* oAppliedSettings;

	oTrace->StartTrace(__WFUNCTION__);

	oAppliedSettings = poScanSettings==NULL ? oScanSettings : poScanSettings;

	oAppliedSettings->cAcquireType = pcAcquireType;
	bResult = oScanCore->Acquire(phWnd, oAppliedSettings);

	oTrace->EndTrace(__WFUNCTION__);

	return bResult;	
}

//this function imports the images from the scanner. 
bool 
CCopyingMachineCore::TransferImages(HWND phWnd) {

	bool bResult;
	cImageTransferStruct cImage;
	eActionResult eResult;
	CCopyDocument*		oDocument;
	CCopyDocument*		oNewDocument;
	CCopyDocumentPage*	oPage;
	eAcquireType		cAcquireType;
	long				lPageIndex;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;
	eResult = eOk;

	if (oScanCore->ImagesAvailable()>0) {

		cAcquireType = oScanCore->GetLastAcquisitionType();
		oDocument = NULL;

		//Get a document instance to which the pages will be added
		switch (cAcquireType) {

			case eAcquireNewDocument:
			case eAcquireNewDocumentToCollection:
				lPageIndex = 0;
				break;

			case eAcquireEvenPages:
				lPageIndex = 1;
				oDocument = GetCurrentDocument();
				break;

			case eAcquireOddPages:
				lPageIndex = 0;
				oDocument = GetCurrentDocument();
				break;

			case eAcquireNewPageAndInsert:
				lPageIndex = 0;
				oDocument = GetCurrentDocument();
				if (oDocument!=NULL) {
					lPageIndex = oDocument->GetPageIndex(GetCurrentPage());
					}
				break;

			default:
				lPageIndex = 0;
				oDocument = GetCurrentDocument();
				if (oDocument!=NULL) {
					lPageIndex = oDocument->GetPageCount();
					}
				break;
			}

		//No current document. Create a new one for the acquire images.
		if (oDocument==NULL) {

			oNewDocument = oDocuments->CreateNewDocument();
			eResult = oNewDocument->AutoSaveSetDocumentName(phWnd);
			oDocument = oNewDocument;
			oNotifications->CommunicateEvent(eNotificationNewDocument, (void*)oNewDocument->GetDocumentID());
			}

		oTrace->Add(L"oDocument", (int)oDocument, eAll);

		//temporarily disable the undo function
		oUndoRedo->EnableUndo(false);

		//Process the pages
		while (oScanCore->ImagesAvailable()>0 && oDocument!=NULL) {

			cImage = oScanCore->GetImage();
			
			oPage = oDocument->InsertNewPage(lPageIndex, cImage.hImageHandle);
			oPage->SetResolutionX(cImage.iDPIX);
			oPage->SetResolutionY(cImage.iDPIY);
			oPage->SetDirty(true);

			oNotifications->CommunicateEvent(eNotificationNewPage, (void*)oPage->GetPageID());

			if (cAcquireType==eAcquireOddPages || cAcquireType==eAcquireEvenPages) lPageIndex++;

			lPageIndex++;
			bResult = true;
			}

		//Action after processing the images
		switch (cAcquireType) {

			case eAcquireNewDocument:
			case eAcquireNewDocumentToCollection:

				if (bResult && eResult!=eCancelled) {

					eResult = oNewDocument->AutoSave(phWnd);
				
					if (eResult!=eOk) {

						oDialogs->DlgError(phWnd, eResult, 1, oNewDocument->GetFilename().c_str());
						}
					}
				//no break - this type also needs to process the default case

			default:

				if (!bResult && oNewDocument!=NULL) delete oNewDocument;

				if (bResult) {
					
					//set the new document as the current document
					SetCurrentDocument(oDocuments->GetDocumentCount()-1);
					}
				break;
			}
		}

	//enable the undo function again
	oUndoRedo->EnableUndo(true);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This functions shows the openfile dialog and loads a new document
bool 
CCopyingMachineCore::LoadDocument(HWND phWnd) {

	bool bResult;
	wchar_t cFilename[MAX_PATH];
	int iDefaultFilter;

	oTrace->StartTrace(__WFUNCTION__);

	iDefaultFilter = oRegistry->ReadInt(L"General", L"FilterOpen", 0);
	SecureZeroMemory(cFilename, sizeof(cFilename));
	
	bResult = oDialogs->DlgOpenFile(phWnd, L"All files (*.*)\0*.*\0PC Paintbrush (*.pcx)\0*.pcx\0Portable Network Graphics (*.png)\0*.png\0JPEG Compliant (*.jpg, *.jpeg)\0*.jpg;*.jpeg\0Tagged Image File Format (*.tif, *.tiff)\0*.tif;*.tiff\0Windows Bitmap (*.bmp);*.bmp\x00\x00", iDefaultFilter, cFilename);

	if (bResult) {

		oRegistry->WriteInt(L"General", L"FilterOpen", iDefaultFilter);

		bResult = LoadDocument(phWnd, cFilename);
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This functions loads a new document with the give filename
bool 
CCopyingMachineCore::LoadDocument(HWND phWnd, std::wstring psFilename) {

	bool bResult;
	eActionResult eResult;
	CCopyDocument* poNewDocument;

	oTrace->StartTrace(__WFUNCTION__);

	poNewDocument = NULL;

	eResult = oDocuments->LoadDocument(psFilename, poNewDocument);

	bResult = (eResult == eOk);
	
	if (bResult) {
			
		SetCurrentDocument(oDocuments->GetDocumentCount()-1);

		oNotifications->CommunicateEvent(eNotificationNewDocument, (void*)GetCurrentDocument()->GetDocumentID());
	}
	else {

		if (eResult!=eFileResultExternalFile) oDialogs->DlgError(phWnd, eResult, 1, psFilename.c_str());
		}

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return bResult;
	}

bool 
CCopyingMachineCore::SaveDocument(HWND phWnd, CCopyDocument* poDocument) {

	bool bResult;
	eActionResult eResult;

	if (poDocument==NULL) return false;

	oTrace->StartTrace(__WFUNCTION__);

	eResult = poDocument->Save(phWnd);
	bResult = (eResult==eOk);

	if (!bResult) {

		oDialogs->DlgError(phWnd, eResult, 1, poDocument->GetFilename().c_str());
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function closes the given document
bool 
CCopyingMachineCore::CloseDocument(HWND phWnd, CCopyDocument* poDocument) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;

	if (poDocument==NULL) bResult = false;

	if (bResult) {
		
		bResult = oDocuments->CloseDocument(poDocument);

		SetCurrentDocument(oDocuments->GetDocumentCount()-1);
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function asks the user if he wants to save changes to
//a dirty document. This function is called prior to a termination
//of the application. Result = false when a save isn't succesful or
//the users pressed cancel.
bool 
CCopyingMachineCore::SaveChangesToDirtyDocuments(HWND phWnd) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);
	
	bResult = oDocuments->SaveChangesToDirtyDocuments(phWnd);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function pastes the content of the clipboard as a new page into
//a new or existing document
bool 
CCopyingMachineCore::PasteImage(HWND phWnd, int piCommand) {

	bool bResult;
	CCopyDocument* oDocument;
	CCopyDocumentPage* oPage;
	Bitmap*	oBitmap;
	long lPosition;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;
	oDocument = GetCurrentDocument();

	oBitmap = CBitmap::GetFromClipboard(phWnd);

	if (oBitmap!=NULL) {

		//Create a new document
		if (piCommand == ID_EDIT_PASTE) {
			oDocument=oDocuments->CreateNewDocument();
			
			if (oDocument!=NULL) oNotifications->CommunicateEvent(eNotificationNewDocument, (void*)oDocument->GetDocumentID());
			}

		//Add the page to the document
		if (oDocument!=NULL) {

			lPosition = oDocument->GetPageCount();

			if (piCommand == ID_EDIT_PASTEASNEWPAGEANDINSERT) lPosition = oDocument->GetPageIndex(GetCurrentPage());

			oPage = oDocument->InsertNewPage(lPosition, oBitmap);
			oPage->SetDirty(true);

			//SetCurrentDocumentByID(oDocument->GetDocumentID());

			oNotifications->CommunicateEvent(eNotificationNewPage, (void*)oPage->GetPageID());
			
			SetCurrentPageByID(oPage->GetPageID());

			bResult = true;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}


//This function tests if the clipboard contains an image to paste
bool 
CCopyingMachineCore::PasteImageCheck() {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__, eExtreme);

	bResult = IsClipboardFormatAvailable(CF_BITMAP) || 
			 IsClipboardFormatAvailable(CF_DIB);

	oTrace->EndTrace(__WFUNCTION__, bResult, eExtreme);

	return bResult;
}

//This function does an action on the current page in the imageviewer.
eActionResult 
CCopyingMachineCore::DoPageImageAction(ePageImageAction pcAction, 
									   DWORD dActionData,
									   HWND phWnd) {

	eActionResult cResult;
	CCopyDocumentPage *oPage;
	CCopyDocument *oDocument;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;

	//Get the current page
	oDocument = GetCurrentDocument();
	oPage = GetCurrentPage();

	if (oPage!=NULL && oDocument!=NULL) {

		switch (pcAction) {

			case eImageCrop:			cResult = oPage->DoCropImage(); break;
			case eImageRotate:			cResult = oPage->DoRotateImage(dActionData); break;
			case eImageFillRectangle:	cResult = oPage->DoFillRectangle(dActionData); break;
			case eImageFlipHorizontal:	cResult = oPage->DoFlipImage(0); break;
			case eImageFlipVertical:	cResult = oPage->DoFlipImage(1); break;
			case eImageGrayscale:		cResult = oPage->DoConvertGrayscale(0); break;
			case eImageBlackWhite:		cResult = oPage->DoConvertBlackWhite(0); break;
			case eImageCopy:		    cResult = oPage->DoCopyImage(phWnd); break;
			case eImageCut:				cResult = oPage->DoCutImage(dActionData, phWnd); break;
			case ePageCopy:				cResult = oDocument->DoCopyPage(oPage->GetPageID(), phWnd); break;
			case ePageCut:				cResult = oDocument->DoCutPage(oPage->GetPageID(), phWnd); break;
			case ePageDelete: 			cResult = oDocument->DoDeletePage(oPage->GetPageID(), phWnd); break;
			case ePageSplit:			cResult = oDocuments->DoSplitDocument(oDocument, oPage->GetPageID(), phWnd); break;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, cResult);

	if (cResult!=eOk) oDialogs->DlgError(phWnd, cResult, 0);
	
	return cResult;
	}

//This function checks if the requested image can be performed, given
//the current condition/state
bool 
CCopyingMachineCore::CheckPageImageAction(ePageImageAction pcAction, DWORD dActionData) {

	bool bResult;
	CCopyDocumentPage *oPage;
	CCopyDocument* oDocument;

	bResult = false;

	//Get the current page
	oPage = GetCurrentPage();
	oDocument = GetCurrentDocument();

	if (oPage!=NULL && oDocument!=NULL) {

		switch (pcAction) {

			case eImageCrop:
			case eImageRotate:
			case eImageFillRectangle:
			case eImageFlipHorizontal:
			case eImageFlipVertical:
			case eImageCopy:
			case eImageCut:
			case eImageBlackWhite:
			case eImageGrayscale:
				
				bResult = oPage->CheckPageImageAction(pcAction, dActionData); 
				break;

			case ePageDelete:			
			case ePageCut:
			case ePageCopy:
			case ePageSplit:

				bResult = oDocument->CheckPageImageAction(pcAction, dActionData); 
				break;
			}
		}

	return bResult;
	}

void
CCopyingMachineCore::CommandlineInput(std::wstring psOptions) {

	long lLength;
	long lIndex;
	bool bInQuotes;
	std::wstring sOption;

	lIndex=0;
	lLength = psOptions.length();
	bInQuotes = false;
	sOption = L"";

	while (lIndex < lLength) {

		if (psOptions.substr(lIndex,1)==L"\"") {
			
			bInQuotes = !bInQuotes;
			}
		else {

			if ((psOptions.substr(lIndex,1)==L" " && !bInQuotes) || (lIndex==lLength-1)) {

				if (lIndex==lLength-1) sOption += psOptions[lIndex];
				if (!sOption.empty()) CommandlineOptionAdd(sOption); //to do: chopping it up into parts
				sOption=L"";
				}
			else {
				
				sOption += tolower(psOptions[lIndex]);
				}
			}

		lIndex++;
		}

	if (!sOption.empty()) CommandlineOptionAdd(sOption); //to do: chopping it up into parts
	}


void 
CCopyingMachineCore::CommandlineOptionAdd(std::wstring psOption) {

	bool bAdd;
	
	oTrace->StartTrace(__WFUNCTION__);

	bAdd = true;

	//remove the stidevice and stievent options
	if (psOption.length()>10) {
		
		if (psOption.substr(0,10)==L"/stidevice") bAdd = false;
		if (psOption.substr(0,9) ==L"/stievent")  bAdd = false;
		}

	if (bAdd) oCommandlineOptions.push_back(psOption);

	oTrace->EndTrace(__WFUNCTION__, psOption);
	}

void 
CCopyingMachineCore::CommandlineOptionsProcess(HWND phWnd, bool pbEasyFrame) {

	bool bCommandExecuted;
	bool bScanCommand;
	bool bCopyCommand;
	bool bExitCommand;
	bool bEmailCommand;
	std::wstring sNumber;

	oTrace->StartTrace(__WFUNCTION__);
	
	bScanCommand = false;
	bCopyCommand = false;
	bEmailCommand = false;
	bExitCommand = false;
	bEmailCommand = false;

	while (oCommandlineOptions.size()>0) {

		oTrace->Add(L"Option", oCommandlineOptions[0]);
		
		bCommandExecuted = false;

		if (oCommandlineOptions[0]==L"/scan") {
			bCommandExecuted = true;
			bScanCommand = true;
			}	
		
		if (oCommandlineOptions[0]==L"/email") {
			bCommandExecuted = true;
			bEmailCommand = true;
			}	

		if (oCommandlineOptions[0]==L"/copy") {
			bCommandExecuted = true;
			bCopyCommand = true;
			}	
		
		if (oCommandlineOptions[0]==L"/exit") {
			bCommandExecuted = true;
			bExitCommand = true;
			}

		//Set the number of copies
		if (wcsncmp(oCommandlineOptions[0].c_str(), L"/copies:", 8)==0) {

			bCommandExecuted = true;

			sNumber.assign(oCommandlineOptions[0].begin()+8, oCommandlineOptions[0].end());

			long lCopies;

			lCopies = _wtoi(sNumber.c_str());

			if (lCopies>0 && lCopies<99) {

				oPrintSettings->SetInt(ePrintCopies, lCopies);
				}
			}
		
		//not a known commandline command, try to load it as a document
		if (!bCommandExecuted && !pbEasyFrame) LoadDocument(phWnd, oCommandlineOptions[0]);

		oCommandlineOptions.erase(oCommandlineOptions.begin());
		}


	if (bEmailCommand) {

		std::wstring sEmailScript;

		sEmailScript = L"begin scan\n";
		sEmailScript += L"set scantype ScanDocument\n";
		sEmailScript += L"end\n";
		sEmailScript += L"begin print\n";
		sEmailScript += L"set device = \"E-mail\"\n";
		sEmailScript += L"end\n";
		
		oScriptProcessor->Process(sEmailScript);
		oScriptProcessor->ExecuteNextScriptLine();
		}

	if (bScanCommand && !pbEasyFrame && !bCopyCommand) PostMessage(phWnd, WM_COMMAND, MAKEWPARAM(ID_SCAN_ACQUIRE, 0), 0);
	if (bCopyCommand && !bExitCommand) PostMessage(phWnd, WM_COMMAND, MAKEWPARAM(IDC_DOCOPY, 0), 0);
	if (bCopyCommand && bExitCommand) PostMessage(phWnd, WM_COMMAND, MAKEWPARAM(IDC_DOCOPYEXIT, 0), 0);
	if (bExitCommand && ! bCopyCommand) PostMessage(phWnd, WM_COMMAND, MAKEWPARAM(IDM_EXIT, 0), 0);


	oTrace->EndTrace(__WFUNCTION__);
	}

int  
CCopyingMachineCore::CreateDocumentID() {

	int iReturn;

	oTrace->StartTrace(__WFUNCTION__);

	iReturn = iDocumentIDCounter;
	iDocumentIDCounter++;

	oTrace->EndTrace(__WFUNCTION__, iReturn);

	return iReturn;
	}

//Memorize the ID of the current document
void 
CCopyingMachineCore::MemorizeCurrentDocument() {

	oTrace->StartTrace(__WFUNCTION__);

	lCurrentDocumentMemorized = lCurrentDocument;

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function restores the memorized document as the current one
void 
CCopyingMachineCore::RestoreMemorizedDocument() {

	oTrace->StartTrace(__WFUNCTION__);

	SetCurrentDocument(lCurrentDocumentMemorized);

	oTrace->EndTrace(__WFUNCTION__);
	}


LANGID
CCopyingMachineCore::GetPreferredLanguage() {

	return (LANGID)oRegistry->ReadInt(L"General", L"Language", 0);
	}

void			
CCopyingMachineCore::SetPreferredLanguage(LANGID plLanguage) {

	if (plLanguage!=GetPreferredLanguage()) {
		
		//Preferred language has changed, reload and notify the world
		oRegistry->WriteInt(L"General", L"Language", plLanguage);

		FreeLibrary(oGlobalInstances.hLanguage);
		oGlobalInstances.hLanguage = LoadLanguage();

		oNotifications->CommunicateEvent(eNotificationLanguageChanged, oGlobalInstances.hLanguage);
	}
}


//This function loads a language module based on the given preferences. It 
//also fills the cLanguage vector with the languages supported by the
//present language DLL's.
HINSTANCE	
CCopyingMachineCore::LoadLanguage() {

	wchar_t cDll[MAX_PATH];
	wchar_t cFilename[100];
	HINSTANCE hResult;
	LANGID	  cUserLanguage;

	wcscpy_s(cFilename, 100, L"");

	//Get the current folder
	GetModuleFileName(NULL, cDll, MAX_PATH); 
	PathRemoveFileSpec(cDll);

	//Get the user language of the user
	cUserLanguage = oRegistry->ReadInt(L"General", L"Language", 0);
	if (cUserLanguage==0) cUserLanguage = PRIMARYLANGID(GetUserDefaultLangID());

	//Walk through the files in the directory and look for language
	//dlls, cm*.dll
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
		};
	WIN32_FIND_DATA cFindFileData;
	wchar_t			cFind[MAX_PATH];
	HANDLE			hFind;
	DWORD			dBufferSize;
	DWORD			dHandle;
	void*			cVersionInfoBuffer;
	//void*			cLanguage;
	UINT			dLanguageLength;
	LANGANDCODEPAGE *cLanguage;
	bool			bFound;
	bool			bLangIdPresent;

	wcscpy_s(cFind, MAX_PATH, cDll);
	wcscat_s(cFind, MAX_PATH, L"\\cm_*.dll");

	hFind = FindFirstFile(cFind, &cFindFileData);
	bFound = (hFind != INVALID_HANDLE_VALUE);

	while (bFound) {

		//Get the versioninfo from the file
		dBufferSize =  GetFileVersionInfoSize(cFindFileData.cFileName, &dHandle);

		if (dBufferSize>0) {
			
			cVersionInfoBuffer = malloc(dBufferSize);
			GetFileVersionInfo(cFindFileData.cFileName, 0, dBufferSize, cVersionInfoBuffer);

			//query the version info
			VerQueryValue(cVersionInfoBuffer, L"\\VarFileInfo\\Translation", (LPVOID*)&cLanguage, &dLanguageLength);
						  
			for(unsigned int i=0; i < (dLanguageLength/sizeof(struct LANGANDCODEPAGE)); i++ ) {
	 
				//Is this the one we need?
				if (PRIMARYLANGID(cLanguage[i].wLanguage) == cUserLanguage) {

					wcscpy_s(cFilename, 100, L"\\");
					wcscat_s(cFilename, 100, cFindFileData.cFileName);
					}

				//Add the langid to the vector, but make sure it isn't added twice
				bLangIdPresent = false;

				for (long lIndex=0; lIndex<(long)cLanguages.size(); lIndex++) {
					if (cLanguages[lIndex]==PRIMARYLANGID(cLanguage[i].wLanguage)) bLangIdPresent = true;
					}
				
				if (!bLangIdPresent) cLanguages.push_back(PRIMARYLANGID(cLanguage[i].wLanguage));
				}

			//clean up
			free(cVersionInfoBuffer);
			}

		//find the next file
		bFound = (FindNextFile(hFind, &cFindFileData)==TRUE);
		}

	FindClose(hFind);

	//If no DLL is found use the english
	if (wcslen(cFilename)==0) {
		//Get the current user language
		//MessageBox(NULL, L"The requested language module isn't present. The english language module will be used instead.", L"Error Copying Machine", MB_OK|MB_ICONERROR);
		wcscpy_s(cFilename, 100, L"\\cm_english.dll");
		}
	
	//Try to open the language DLL
	wcscat_s(cDll, MAX_PATH, cFilename);
	hResult = LoadLibraryW(cDll);

	if (hResult == NULL) {

		MessageBox(NULL, L"Couldn't open the language module", L"Error Copying Machine", MB_OK|MB_ICONERROR);
		}

	return hResult;
	}

//This function return the number of language dll found. This
//function call be called after a loadlanguage.
int			
CCopyingMachineCore::GetLanguageCount() {

	return (int)cLanguages.size();

	}

//This function returns the language Id of the requested
LANGID		
CCopyingMachineCore::GetLanguage(int piIndex) {

	LANGID lResult;

	lResult = LANG_ENGLISH;

	if (piIndex<(int)cLanguages.size() && piIndex>=0) {

		lResult = cLanguages[piIndex];
		}

	return lResult;
	}

//This function retrieves the version information from the executable.
void
CCopyingMachineCore::DoGetCopyingMachineVersion(sExecutableInformation &pcVersionInformation) {

	DWORD				dBufferSize;
	DWORD				dHandle;
	wchar_t				cFilename[MAX_PATH];
	void*				cVersionInfoBuffer;
	VS_FIXEDFILEINFO*	cFixedFileInfo;
	UINT				iFixedFileInfoSize;
	wchar_t*			cLegalCopyright;

	oTrace->StartTrace(__WFUNCTION__);

	//Get the filename of the current process
	GetModuleFileName(oGlobalInstances.hInstance, cFilename, MAX_PATH);

	//Get the version information
	dBufferSize =  GetFileVersionInfoSize(cFilename, &dHandle);

	if (dBufferSize>0) {

		cVersionInfoBuffer = malloc(dBufferSize);
		GetFileVersionInfo(cFilename, 0, dBufferSize, cVersionInfoBuffer);

		iFixedFileInfoSize = sizeof(cFixedFileInfo);
		VerQueryValue(cVersionInfoBuffer, L"\\", (LPVOID*)&cFixedFileInfo, &iFixedFileInfoSize);
		
		//Get the product version
		pcVersionInformation.dVersion = cFixedFileInfo->dwProductVersionMS;

		// Get the legal copyright
		iFixedFileInfoSize = 255;
		VerQueryValue(cVersionInfoBuffer, L"\\StringFileInfo\\000904b0\\LegalCopyright", (LPVOID*)&cLegalCopyright, &iFixedFileInfoSize);
		pcVersionInformation.sLegalCopyright = cLegalCopyright;
		}
		
	oTrace->EndTrace(__WFUNCTION__);
	}

//This function returns the previously determined product version of Copying Machine
void		
CCopyingMachineCore::GetCopyingMachineVersion(sExecutableInformation &pcExecutableInformation) {

	pcExecutableInformation.dVersion =			cExecutableInformation.dVersion;
	pcExecutableInformation.sLegalCopyright =	cExecutableInformation.sLegalCopyright;
	pcExecutableInformation.sHashValue =		cExecutableInformation.sHashValue;
	}

//This function creates a hash of this executable. It uses
//the cryptoapi from windows for it.
void 
CCopyingMachineCore::DoHashExecutable() {

	HCRYPTPROV hCryptProv;
	HCRYPTHASH hHash;
	wchar_t	   cFilename[MAX_PATH+2];
	FILE*	   hExecutable;
	char*	   cBuffer;
	int		   iDataRead;
	bool	   bSuccess;
	BYTE         *pbHash;
	BYTE         *pbHashSize;
	DWORD        dwHashLen = sizeof(DWORD);

	GetModuleFileName(NULL, cFilename, MAX_PATH);
	
	_wfopen_s(&hExecutable, cFilename, L"rb");

	cBuffer = (char*)malloc(2048);

	if (hExecutable) {

		if(CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL,0)) {

			if(CryptCreateHash(hCryptProv, CALG_SHA, 0, 0, &hHash)) {

				bSuccess = true;

				//Hash the executable
				do {
					
					iDataRead = (int)fread_s(cBuffer, 2048, 1, 2048, hExecutable);

					bSuccess = (CryptHashData(hHash, (BYTE*)cBuffer, iDataRead, 0)==TRUE);

					} while (iDataRead==2048 && bSuccess);				
				
				free(cBuffer);

				//Read out the hash
				if (bSuccess) {
		
					bSuccess = false;

					pbHashSize = NULL;
					if(CryptGetHashParam(hHash, HP_HASHSIZE, pbHashSize, &dwHashLen,0)) {

						free(pbHashSize);

						if(CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0)) {

							pbHash = (BYTE*)malloc(dwHashLen);

							if (pbHash) {
	
								if(CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0)) {

									wchar_t cHashByte[16];

									cExecutableInformation.sHashValue;

									for (long lIndex=0; lIndex<(long)dwHashLen; lIndex++) {
			
										swprintf_s(cHashByte, 16, L"%x", pbHash[lIndex]);

										cExecutableInformation.sHashValue += cHashByte;
										}

									//MessageBox(NULL, cExecutableInformation.sHashValue.c_str(), L"Copying Machine", MB_OK);

									bSuccess = true;
									}

								free (pbHash);
								}
							}
						}
					}
				
				CryptDestroyHash(hHash);
				}

			CryptReleaseContext(hCryptProv,0);
			}

		fclose(hExecutable);
		}
	}

//This function opens the help function; either on the local disk or online
void
CCopyingMachineCore::OpenHelp(HWND phWnd) {

	wchar_t cFilename[MAX_PATH];
	wchar_t cLanguage[30];
	std::wstring sHelpIndex;
	bool bSucces;

	oTrace->StartTrace(__WFUNCTION__);

	bSucces = false;

	//Get the filename and path of the executable
	if (GetModuleFileName(NULL, cFilename, MAX_PATH)!=0) {

		//Remove the filename of the executable
		if (PathRemoveFileSpec(cFilename)) {

			LoadString(oGlobalInstances.hLanguage, IDS_CURRENTLANGUAGEDESC, cLanguage, sizeof(cLanguage));

			sHelpIndex = cFilename;
			sHelpIndex += L"\\help\\";
			sHelpIndex += cLanguage;			
			sHelpIndex += L"\\index.html";
		
			//Try to open the help index.
			if (ShellExecute(phWnd, L"open", sHelpIndex.c_str(), NULL, NULL, SW_SHOWNORMAL)>(HINSTANCE)32) {
		
				bSucces = true;// help file on local disc opened.
				}

			if (!bSucces) {

				//try to open the english help index
				sHelpIndex = cFilename;
				sHelpIndex += L"\\help\\english\\index.html";
			
				//Try to open the help index.
				if (ShellExecute(phWnd, L"open", sHelpIndex.c_str(), NULL, NULL, SW_SHOWNORMAL)>(HINSTANCE)32) {
			
					bSucces = true;// help file on local disc opened.
					}
				}
			}
		}

	if (!bSucces) {

		ShellExecute(phWnd, L"open", L"http://copyingmachinehelp.meusesoft.com", NULL, NULL, SW_SHOWNORMAL);
		}

	oTrace->EndTrace(__WFUNCTION__);
}

//Script functions
bool 
CCopyingMachineCore::Set(std::wstring psObjectname, sScriptExpression psNewValue) {

	bool bResult;
	CCopyDocument* oDocument;

	bResult = CScriptObject::Set(psObjectname, psNewValue);

	if (!bResult && QueryType(psObjectname) == eScriptObjectAttribute) { 

		//process the document functions
		if (psObjectname.find(L"document")==0) {

			oDocument = GetCurrentDocument();
			if (!oDocument) {

				sError = L"No document available";
				}
			else {

				if (psObjectname == L"documentname") {

					if (psNewValue.toString()) {

						oDocument->SetTitle(psNewValue.sResult);
						}
					else {

						sError = L"Value must be a string";
						}
					}
				}
			}
		}

	return bResult;
	}


sScriptExpression 
CCopyingMachineCore::Get(std::wstring psObjectname) {

	sScriptExpression oResult;

	oResult = CScriptObject::Get(psObjectname);

	if (oResult.cValueType == eScriptValueAny &&
		QueryType(psObjectname) == eScriptObjectAttribute) { 


		}

	return oResult;
	}

bool 
CCopyingMachineCore::Execute(std::wstring psObjectname, 
							 sScriptExpression &psValue) {

	bool bResult;


	bResult = false;

	bResult = CScriptObject::Execute(psObjectname, psValue);

	if (!bResult) {

		if (psObjectname==L"exit") {

			//PostMessage(hMainApplicationWindow, WM_COMMAND, MAKEWPARAM(IDM_EXIT, 0), 0);			
			bResult = true;
			}

		if (psObjectname==L"currentdocument") {

			psValue.cValueType = eScriptValuePointer;
			psValue.pPointer = (void*)GetCurrentDocument();
			bResult = true;
			}
		if (psObjectname==L"currentpage") {

			psValue.cValueType = eScriptValuePointer;
			psValue.pPointer = (void*)GetCurrentPage();
			bResult = true;
			}
		}

	return bResult;
	}


