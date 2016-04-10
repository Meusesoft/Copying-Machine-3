#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "shlobj.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
 
CDocumentCollections::CDocumentCollections(sGlobalInstances pcGlobalInstances)
{
	oTrace = pcGlobalInstances.oTrace;

	oTrace->StartTrace(__WFUNCTION__);

	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pcGlobalInstances, sizeof(sGlobalInstances));

	oRegistry = pcGlobalInstances.oRegistry;
	
	bNotifications = false;

	//load the DocumentCollections from the registry;
	Load();

	bNotifications = true;

	oTrace->EndTrace(__WFUNCTION__);
}

CDocumentCollections::~CDocumentCollections()
{
   oTrace->StartTrace(__WFUNCTION__);

   oTrace->EndTrace(__WFUNCTION__);
	}

//This function enables/disables the notifications
void
CDocumentCollections::EnableNotifications(bool pbEnable) {

	bNotifications = pbEnable;
	}

//This function returns the number of DocumentCollections
int 
CDocumentCollections::GetDocumentCollectionCount() {

	return (int)oDocumentCollections.size();
	}

bool
CDocumentCollections::GetDocumentCollectionIndex(std::wstring psSearch, long& piIndex) {

	bool bResult;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	lIndex = (long)oDocumentCollections.size();

	while (lIndex>0 && !bResult) {

		lIndex--;
		if (oDocumentCollections[lIndex].sName == psSearch) {

			piIndex = lIndex;
			bResult = true;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}


//This function gets the requested document type
bool 
CDocumentCollections::GetDocumentCollection(long plIndex, sDocumentCollection &pcType) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	if (plIndex>=0 && plIndex<(long)oDocumentCollections.size()) {

		pcType.sName = oDocumentCollections[plIndex].sName;
		pcType.sLocation = oDocumentCollections[plIndex].sLocation;
		pcType.sFilenameTemplate = oDocumentCollections[plIndex].sFilenameTemplate;
		pcType.cFileType = oDocumentCollections[plIndex].cFileType;
		pcType.iResolution = oDocumentCollections[plIndex].iResolution;
		pcType.iColorDepth = oDocumentCollections[plIndex].iColorDepth;

		bResult = true;
		}
	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function sets/updates a document type.
bool 
CDocumentCollections::SetDocumentCollection(long plIndex, sDocumentCollection pcType) {

	bool bResult;
	CCopyingMachineCore* oCore;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	if (plIndex>=0 && plIndex<(long)oDocumentCollections.size()) {

		if (plIndex!=0) oDocumentCollections[plIndex].sName = pcType.sName;
		oDocumentCollections[plIndex].sLocation = pcType.sLocation;
		oDocumentCollections[plIndex].sFilenameTemplate = pcType.sFilenameTemplate;
		oDocumentCollections[plIndex].cFileType = pcType.cFileType;
		oDocumentCollections[plIndex].iColorDepth = pcType.iColorDepth;
		oDocumentCollections[plIndex].iResolution = pcType.iResolution;

		bResult = true;

		if (bNotifications) {
			oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
			oCore->oNotifications->CommunicateEvent(eNotificationDocumentCollectionUpdate, 0);
			}
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function adds a document type to the vector.
int 
CDocumentCollections::AddDocumentCollection(sDocumentCollection pcType) {

	bool bResult;
	CCopyingMachineCore* oCore;

	bResult = true;

	oTrace->StartTrace(__WFUNCTION__);

	oDocumentCollections.push_back(pcType);

	if (bNotifications) {
		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		oCore->oNotifications->CommunicateEvent(eNotificationDocumentCollectionUpdate, 0);
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return (int)oDocumentCollections.size()-1;
	}

//This function deletes a document type from the vector. The default document type cannot
//be deleted
bool 
CDocumentCollections::DeleteDocumentCollection(long plIndex) {

	bool bResult;
	CCopyingMachineCore* oCore;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	if (plIndex>0 && plIndex<(long)oDocumentCollections.size()) {

		oDocumentCollections.erase(oDocumentCollections.begin() + plIndex);

		if (bNotifications) {
			oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
			oCore->oNotifications->CommunicateEvent(eNotificationDocumentCollectionUpdate, 0);
			}

		bResult = true;
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function returns true if the item its name can be edited
bool 
CDocumentCollections::CheckNameEditable(long plIndex) {

	return (plIndex>0);
	}

//This function returns true if the item its name can be deleted
bool 
CDocumentCollections::CheckDocumentTypeDeletable(long plIndex) {

	return (plIndex>0);
	}

//This function loads the document types from the registry/settings file
void 
CDocumentCollections::Load() {

	sDocumentCollection oDocumentCollection;
	wchar_t sTypeName[255];
	wchar_t cFolder[MAX_PATH];
	std::wstring sKey;
	std::wstring sEntry;
	long lDocumentTypeCount;
	CCopyingMachineCore* oCore;

	oTrace->StartTrace(__WFUNCTION__);

	//clear the vector
	oDocumentCollections.clear();

	//Initialise variables
	sKey = _T("DocumentCollections");
	SHGetSpecialFolderPath(NULL, cFolder, CSIDL_PERSONAL, TRUE);
	lDocumentTypeCount = oRegistry->ReadInt(sKey, _T("Count"), 0);
	
	if (lDocumentTypeCount>0) {

		//Read the information from the registry

		for (long lIndex=0; lIndex<lDocumentTypeCount; lIndex++) {


			swprintf(sTypeName, 25, L"%s%i", L"DC", lIndex);
			sEntry = sTypeName;

			oRegistry->ReadString(sKey, sEntry + L"Name", L"Name", oDocumentCollection.sName);
			oRegistry->ReadString(sKey, sEntry + L"Format",L"Document %counter%", oDocumentCollection.sFilenameTemplate);
			oRegistry->ReadString(sKey, sEntry + L"Location", cFolder, oDocumentCollection.sLocation);
			oDocumentCollection.cFileType = (eCopyDocumentFiletype)oRegistry->ReadInt(sKey, sEntry + L"FileType", (int)eTIFFUncompressed);
			oDocumentCollection.iColorDepth = oRegistry->ReadInt(sKey, sEntry + L"ColorDepth", 1);
			oDocumentCollection.iResolution = oRegistry->ReadInt(sKey, sEntry + L"Resolution", 200);

			if (lIndex==0) {
				
				LoadString(oGlobalInstances.hLanguage, IDS_DEFAULT, sTypeName, 254);
				oDocumentCollection.sName = sTypeName;
				}

			oDocumentCollections.push_back(oDocumentCollection);
			}	
		}
	else {

		//add the initial default documenttype
		LoadString(oGlobalInstances.hLanguage, IDS_DEFAULT, sTypeName, 254);
		oDocumentCollection.sName = sTypeName;
		oDocumentCollection.sFilenameTemplate = L"Document %counter%";

		oDocumentCollection.sLocation = cFolder;
		oDocumentCollection.iColorDepth = 2;
		oDocumentCollection.iResolution = 200;
		oDocumentCollection.cFileType = (int)eTIFFUncompressed;


		oDocumentCollections.push_back(oDocumentCollection);
		}

	if (bNotifications) {
		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		oCore->oNotifications->CommunicateEvent(eNotificationDocumentCollectionUpdate, 0);
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function saves the DocumentCollections to the registry / settings file
void 
CDocumentCollections::Save() {

	wchar_t sTypeName[25];
	std::wstring sKey;
	std::wstring sEntry;
	long lDocumentTypeCount;

	oTrace->StartTrace(__WFUNCTION__);

	//Initialise the variables
	sKey = _T("DocumentCollections");
	lDocumentTypeCount = (long)oDocumentCollections.size();
			
	oRegistry->WriteInt(sKey, _T("Count"), lDocumentTypeCount);

	//Iterate through the document types
	for (long lIndex=0; lIndex<lDocumentTypeCount; lIndex++) {

		swprintf(sTypeName, 25, _T("%s%i"), _T("DC"), lIndex);
		sEntry = sTypeName;

		oRegistry->WriteString(sKey, sEntry + _T("Name"),		oDocumentCollections[lIndex].sName);
		oRegistry->WriteString(sKey, sEntry + _T("Format"),		oDocumentCollections[lIndex].sFilenameTemplate);
		oRegistry->WriteString(sKey, sEntry + _T("Location"),	oDocumentCollections[lIndex].sLocation);
		oRegistry->WriteInt(   sKey, sEntry + _T("FileType"),	oDocumentCollections[lIndex].cFileType);	
		oRegistry->WriteInt(   sKey, sEntry + _T("ColorDepth"),	oDocumentCollections[lIndex].iColorDepth);	
		oRegistry->WriteInt(   sKey, sEntry + _T("Resolution"),	oDocumentCollections[lIndex].iResolution);	

	}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function sets the description
void 
CDocumentCollections::SetDescription(std::wstring psDescription) {

	sDescription = psDescription;
}

//This function creates a filename from the given document type
std::wstring 
CDocumentCollections::CreateFilename(long plDocumentType, long plDocumentId, 
									  std::wstring psNewLocation) {

	std::wstring sFilename;
	std::wstring sDocumentId;
	int iCounter;
	std::wstring sNewFilename;
	sDocumentCollection oDocumentCollection;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__);

	lIndex = plDocumentType;
	if (lIndex<0) lIndex=0;
	if (lIndex>=(long)oDocumentCollections.size()) lIndex = oDocumentCollections.size()-1;

	if (GetDocumentCollection(plDocumentType, oDocumentCollection)) {

		tm cTimeStruct;
		time_t cTime;

		time(&cTime);
		localtime_s(&cTimeStruct, &cTime);

		sFilename = oDocumentCollection.sFilenameTemplate;

		//build the filename by replace the tags in the template
		sFilename = DoReplace(sFilename, L"%name%",		sDescription);
		sFilename = DoReplace(sFilename, L"%type%",		oDocumentCollection.sName);
		sFilename = DoReplace(sFilename, L"%counter%",	itos(plDocumentId));
		sFilename = DoReplace(sFilename, L"%year%",		itos(1900 + cTimeStruct.tm_year), 2);
		sFilename = DoReplace(sFilename, L"%month%",	itos(1 + cTimeStruct.tm_mon), 2);
		sFilename = DoReplace(sFilename, L"%day%",		itos(cTimeStruct.tm_mday), 2);
		sFilename = DoReplace(sFilename, L"%hour%",		itos(cTimeStruct.tm_hour), 2);
		sFilename = DoReplace(sFilename, L"%minutes%",	itos(cTimeStruct.tm_min), 2);
		sFilename = DoReplace(sFilename, L"%seconds%",	itos(cTimeStruct.tm_sec), 2);
		sFilename = DoReplace(sFilename, L"%%",			L"%");

		if (!ValidateFilename(sFilename)) sFilename = RemoveReservedCharactersFromFilename(sFilename);

		sNewFilename = sFilename;
		if (psNewLocation==L"") {
			sFilename = oDocumentCollections[lIndex].sLocation;
			}
		else {
			sFilename = psNewLocation;
			}			
		sFilename += L"\\";
		sFilename += sNewFilename;

		//check if the file already exists, and if so add a counter
		sNewFilename = sFilename;
		iCounter = 1;
		sNewFilename += CCopyDocument::GetFiletypeExtension((eCopyDocumentFiletype)oDocumentCollection.cFileType);
		sFilename += L" %counter%";
				
		while (PathFileExists(sNewFilename.c_str()) && iCounter<1000) {

			sNewFilename = DoReplace(sFilename, L"%counter%", itos(iCounter), 3);
			sNewFilename += CCopyDocument::GetFiletypeExtension((eCopyDocumentFiletype)oDocumentCollection.cFileType);
			iCounter++;
			}
	
		sFilename = sNewFilename;
		}


	oTrace->EndTrace(__WFUNCTION__, sFilename);

	return sFilename;
}

//This function replaces a part of the string
std::wstring 
CDocumentCollections::DoReplace(std::wstring psSource, 
					 std::wstring psFind, 
					 std::wstring psReplace,
					 int iMinimumLength) {

	std::wstring sResult;

	//initialise variables
	sResult = psSource;

	//add zero to the replacement string so it match the requested minimum length
	while ((int)psReplace.size() < iMinimumLength) {

		psReplace = L"0" + psReplace;
		}

	//replace all occurrences 
	while (sResult.find(psFind)!=std::wstring::npos) {

		sResult.replace(sResult.find(psFind), psFind.size(), psReplace);
		}

	return sResult;
	}

//This function converts the given number to a string
std::wstring 
CDocumentCollections::itos(int i, base b)	{
		std::wstring mask = L"0123456789ABCDEF";
		
		b = b > 1 && b <= 16 ? b : 10;
		
		std::wstring s = L"";

		return (s += i < b ? L"" : itos(i / b, b)) + mask[i % b];
	}

//This function checks if the filename doesn't contain any reserved characters
bool 
CDocumentCollections::ValidateFilename(std::wstring psFilename) {

	bool bResult;

	bResult = true;
	std::wstring sReservedCharacters;

	sReservedCharacters = L"/\"*:<>?\\|";

	bResult = (psFilename.find_first_of(sReservedCharacters, 0) == std::string::npos );

	return bResult;
	}

//This function removes the reserved characters from the filename
std::wstring 
CDocumentCollections::RemoveReservedCharactersFromFilename(std::wstring psFilename) {

	std::wstring sResult;
	std::wstring sReservedCharacters;

	//initialise
	sResult = psFilename;
	sReservedCharacters = L"\"*:<>?\\|";

	//replace all reserved characters 
	while (sResult.find_first_of(sReservedCharacters)!=std::wstring::npos) {

		sResult.replace(sResult.find_first_of(sReservedCharacters), 1, L"");
		}

	return sResult;
	}

//This function shows a dialog if a description is necessary for the
//current document type
bool
CDocumentCollections::GetDescriptionFromUser(HWND phWnd, long plDocumentType, long plDocumentID) {

	CDlgAutoSaveDescription* oDialog;
	sDocumentCollection oDocumentCollection;
	int iDialogResult;
	bool bResult;

	//Determine the currently selected document type
	bResult = false;

	if (GetDocumentCollection(plDocumentType, oDocumentCollection)) {

		bResult = true;

		//check if the filename template contains %name%, and if so
		//show the description dialog
		if (oDocumentCollection.sFilenameTemplate.find(L"%name%")!=std::wstring::npos) {

			bResult = false;

			//Show the description dialog to the user
			oDialog = new CDlgAutoSaveDescription(oGlobalInstances.hInstance,
												  phWnd,
												  oGlobalInstances);

			oDialog->SetDocumentCollection(plDocumentType);
			oDialog->SetDocumentId(plDocumentID);

			iDialogResult = oDialog->Execute();

			if (iDialogResult==IDOK) {

				SetDescription(oDialog->GetDescription());
				bResult = true;
				}
			}
		}
	
	return bResult;	
	}

LRESULT CALLBACK EditValidateFilename (HWND hwndd, UINT message,
									   WPARAM wParam, LPARAM lParam) {

	sEditControlValidation* oEditControlValidation;
	oEditControlValidation = (sEditControlValidation*)GetWindowLong(hwndd, GWLP_USERDATA);
											   
	switch ( message) {

		case WM_CHAR:
			
			switch (wParam) {

				// \"*:<>?\\|

				case _T('/'):
				case _T('\"'):
				case _T('*'):
				case _T(':'):
				case _T('>'):
				case _T('<'):
				case _T('?'):
				case _T('\\'):
				case _T('|'):

					//Give feedback to the user that some characters
					//are not allowed
					sGlobalInstances* cGlobalInstances;
					cGlobalInstances = oEditControlValidation->oGlobalInstances;
					
					CDialogTemplate::ShowBalloonTip( cGlobalInstances->hLanguage, 
									hwndd,
									IDS_CHARACTERSNOTALLOWED,
									IDS_ERROR);
					
					return 0;
					
					break;
					}
			break;

		case WM_PASTE:

			//check if the text on the clipboard doesn't contain
			//illegal characters
			if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) return 0;  
			if (!OpenClipboard(hwndd)) return 0; 

			HANDLE hClipboardData;
			LPTSTR cText;
			std::wstring sText;
			int iTextLength;

			hClipboardData = GetClipboardData(CF_UNICODETEXT);
				
			if (hClipboardData!=NULL) {

				cText = (LPTSTR)GlobalLock(hClipboardData);
				if (cText!=NULL) {

					sText = cText;

					if (!CDocumentCollections::ValidateFilename(sText)) {

						//Give feedback to the user that some characters
						//are not allowed
						sGlobalInstances* cGlobalInstances;
						cGlobalInstances = (sGlobalInstances*)GetWindowLong(hwndd, GWLP_USERDATA);
						
						CDialogTemplate::ShowBalloonTip( cGlobalInstances->hLanguage, 
										hwndd,
										IDS_CHARACTERSNOTALLOWED,
										IDS_ERROR);

						//Update the contents of the clipboard. Remove all
						//the unwanted characters from the text on the
						//clipboard. Windows Explorer does the same when
						//a user pastes a text into a filename.
						sText = CDocumentCollections::RemoveReservedCharactersFromFilename(sText);

						EmptyClipboard();

						iTextLength = sText.length();
						hClipboardData = GlobalAlloc(GMEM_MOVEABLE, 
							(iTextLength+1) * sizeof(TCHAR)); 
        
						if (hClipboardData!=NULL) {
 
							cText = (LPTSTR)GlobalLock(hClipboardData);
							wcscpy_s(cText, (iTextLength+1), sText.c_str());
							GlobalUnlock(hClipboardData); 
 
							// Place the handle on the clipboard. 
							SetClipboardData(CF_UNICODETEXT, hClipboardData); 
							}
						}
					}
				}

			CloseClipboard();

			break;
			}

	return CallWindowProc ((WNDPROC)oEditControlValidation->lOldProc, hwndd, 
							message, wParam, lParam);
	}



