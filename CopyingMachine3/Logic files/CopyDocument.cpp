#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "BitmapPCX.h"
#include "PDFObject.h"
#include "PDF.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <cctype>
#include "tiffio.h"
#include "MapiInterface.h"

CHAR wide_to_narrow(WCHAR w)
{
// simple typecast
// works because UNICODE incorporates ASCII into itself
return CHAR(w);
}


CCopyDocument::CCopyDocument(sGlobalInstances pInstances) : 
	CScriptObject(L"CopyDocument")
{
	//Set helper classes
	CCopyingMachineCore* oCore;

	oTrace = pInstances.oTrace;

	oCore = (CCopyingMachineCore*)pInstances.oCopyingMachineCore;

	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pInstances, sizeof(sGlobalInstances));

	//Init variables
	iDocumentID = oCore->CreateDocumentID();
	iNewDocumentID = iDocumentID;
	iPageIDCounter = 0;
	bDirty = false;
	cForceFileType = eAny;

	cFiletype = eUnknown;

	RegisterAttribute(L"name");
	RegisterFunction(L"save");
	RegisterFunction(L"saveas");
	RegisterFunction(L"mailto");
	RegisterFunction(L"getpagecount");
}

CCopyDocument::~CCopyDocument(void)
{
	oTrace->StartTrace(__WFUNCTION__);

	//Clear all the pages
	CCopyDocumentPage* oPage;

	for (long lIndex=(long)oPages.size()-1; lIndex>=0; lIndex--) {

		oPage = oPages[lIndex];
		delete oPage;
		}

	oPages.clear();

	oTrace->EndTrace(__WFUNCTION__);
	}

//Return the document ID
int 
CCopyDocument::GetDocumentID() {

	return iDocumentID;
	}

//This function determines of one of its pages are dirty, and if so returns true
bool 
CCopyDocument::GetDirty() {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bResult = bDirty;

	for (int iIndex=0; iIndex<(int)oPages.size() && !bResult; iIndex++) {

		bResult = oPages[iIndex]->GetDirty();
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)bResult, eAll);

	return bResult;
	}

//This function set the dirty flag of the document to true
void 
CCopyDocument::SetDirty() {

	bDirty = true;
	}

//This functions creates and inserts a new page.
CCopyDocumentPage* 
CCopyDocument::InsertNewPage(long plIndex, HGLOBAL phImage) {

	CCopyDocumentPage* oNewPage;

	oTrace->StartTrace(__WFUNCTION__);

	iPageIDCounter++;

	oNewPage = new CCopyDocumentPage(oGlobalInstances, iDocumentID, iPageIDCounter);
	oNewPage->SetImage(phImage);

	if (plIndex > (long)oPages.size()) plIndex = (long)oPages.size();
	
	oPages.insert(oPages.begin() + plIndex, oNewPage);

	oTrace->EndTrace(__WFUNCTION__);

	return oNewPage;
}

CCopyDocumentPage* 
CCopyDocument::InsertNewPage(long plIndex, Bitmap* poImage) {

	CCopyDocumentPage* oNewPage;

	oTrace->StartTrace(__WFUNCTION__);

	iPageIDCounter++;

	oNewPage = new CCopyDocumentPage(oGlobalInstances, iDocumentID, iPageIDCounter);
	oNewPage->SetImage(poImage);

	if (plIndex > (long)oPages.size()) plIndex = (long)oPages.size();

	oPages.insert(oPages.begin() + plIndex, oNewPage);

	oTrace->EndTrace(__WFUNCTION__);

	return oNewPage;
}

CCopyDocumentPage* 
CCopyDocument::InsertNewPage(long plIndex, CBitmap* poImage) {

	CCopyDocumentPage* oNewPage;

	oTrace->StartTrace(__WFUNCTION__);

	iPageIDCounter++;

	oNewPage = new CCopyDocumentPage(oGlobalInstances, iDocumentID, iPageIDCounter);
	oNewPage->SetImage(poImage);

	if (plIndex > (long)oPages.size()) plIndex = (long)oPages.size();

	oPages.insert(oPages.begin() + plIndex, oNewPage);

	oTrace->EndTrace(__WFUNCTION__);

	return oNewPage;
}

CCopyDocumentPage* 
CCopyDocument::InsertNewPage(long plIndex, CCopyDocumentPage* poPage) {

	oTrace->StartTrace(__WFUNCTION__);

	iPageIDCounter++;

	poPage->SetPageID(iDocumentID, iPageIDCounter);

	if (plIndex > (long)oPages.size()) plIndex = (long)oPages.size();

	oPages.insert(oPages.begin() + plIndex, poPage);

	oTrace->EndTrace(__WFUNCTION__);

	return poPage;
	}


//This funtions returns the position of the page within 
//this document. It returns -1 if the page isn't part of the document
long 
CCopyDocument::GetPageIndex(CCopyDocumentPage* poPage) {

	long lResult = -1;

	if (poPage!=NULL) lResult = GetPageIndex(poPage->GetPageID());

	return lResult;
}


long 
CCopyDocument::GetPageIndex(DWORD dPageId) {

	long lResult;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	lResult = -1;
	lIndex = oPages.size();

	while (lResult==-1 && lIndex>0) {

		lIndex--;

		if (oPages[lIndex]->GetPageID() == dPageId) lResult = lIndex;
		}

	oTrace->EndTrace(__WFUNCTION__, lResult, eAll);

	return lResult;
}

//This function gets a page from the document. The index of the page
//should be in the range 0 - GetPageCount()-1;
CCopyDocumentPage* 
CCopyDocument::GetPage(long plIndex) {

	CCopyDocumentPage* oPage;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oPage = NULL;
	
	if (plIndex < (long)oPages.size() && plIndex>=0) {

		oPage = oPages[plIndex];
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)(oPage!=NULL), eAll);

	return oPage;
	}

//This function gets a page from the document. 
CCopyDocumentPage* 
CCopyDocument::GetPageByID(DWORD dPageId) {

	CCopyDocumentPage* oPage;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oPage = NULL;
	lIndex = oPages.size();

	while (lIndex>0 && oPage==NULL ) {

		lIndex--;

		oPage = oPages[lIndex];
		if (oPage->GetPageID() != dPageId) oPage = NULL;
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)(oPage!=NULL), eAll);

	return oPage;
	}

//this function returns the number of pages within the document
int 
CCopyDocument::GetPageCount() {

	return (int)oPages.size();
	}

//these functions get and sets the filename
void 
CCopyDocument::SetFilename(std::wstring psFilename) {

	sFilename = psFilename;

	SetTitle(ConvertFilenameToTitle());
	if (GetFiletype()==eUnknown) SetFiletype(DetermineFiletype(sFilename));
	}

std::wstring 
CCopyDocument::GetFilename() {

	return sFilename;
	}

//these functions get and sets the title
void 
CCopyDocument::SetTitle(std::wstring psTitle) {

	if (sTitle != psTitle) {
		//Send a notification about the change
		SendNotification();
		}

	sTitle = psTitle;
}

std::wstring 
CCopyDocument::GetTitle() {

	return sTitle;
	}

//this function returns only the document name, so it is the
//filename without the path and the extension.
std::wstring 
CCopyDocument::ConvertFilenameToTitle() {

	wchar_t cBuffer[MAX_PATH];
	wchar_t* cFile;
	std::wstring sResult;
	int iPositionExtension;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	sResult = L"No name";

	if (GetFullPathName(sFilename.c_str(), MAX_PATH, cBuffer, &cFile)>0) {

		sResult = cFile;
		iPositionExtension = (int)sResult.length()-4;
		std::wstring sLowerResult = sResult;
		std::transform(sLowerResult.begin(), sLowerResult.end(), sLowerResult.begin(), ::tolower);

		if ((sLowerResult.rfind(L".jpg")==iPositionExtension) ||
			(sLowerResult.rfind(L".pcx")==iPositionExtension) ||
			(sLowerResult.rfind(L".bmp")==iPositionExtension) ||
			(sLowerResult.rfind(L".png")==iPositionExtension) ||
			(sLowerResult.rfind(L".pfd")==iPositionExtension) ||
			(sLowerResult.rfind(L".tif")==iPositionExtension)) {

			if (iPositionExtension>0) sResult.resize(sResult.length()-4);
			}

		iPositionExtension = (int)sResult.length()-5;

		if ((sLowerResult.rfind(L".jpeg")==iPositionExtension) ||
			(sLowerResult.rfind(L".tiff")==iPositionExtension)) {

			if (iPositionExtension>0) sResult.resize(sResult.length()-5);
			}
		}

	oTrace->EndTrace(__WFUNCTION__, sResult, eAll);

	return sResult;
	}
	
//Determine the filetype based on the file extension
eCopyDocumentFiletype 
CCopyDocument::DetermineFiletype(std::wstring psFilename) {

	eCopyDocumentFiletype eResult;
	int iPositionExtension;

	eResult = eUnknown;
	std::transform(psFilename.begin(), psFilename.end(), psFilename.begin(), (int(*)(int)) tolower);
	iPositionExtension = (int)psFilename.length()-4;

	if (psFilename.rfind(L".jpg")==iPositionExtension) eResult = eJPG;
	if (psFilename.rfind(L".png")==iPositionExtension) eResult = ePNG;
	if (psFilename.rfind(L".pcx")==iPositionExtension) eResult = ePCX;
	if (psFilename.rfind(L".bmp")==iPositionExtension) eResult = eBMP;
	if (psFilename.rfind(L".pdf")==iPositionExtension) eResult = ePDF;
	if (psFilename.rfind(L".tif")==iPositionExtension) eResult = eTIFFUncompressed;

	iPositionExtension = (int)psFilename.length()-5;

	if (psFilename.rfind(L".jpeg")==iPositionExtension) eResult = eJPG;
	if (psFilename.rfind(L".tiff")==iPositionExtension) eResult = eTIFFUncompressed;
	
	return eResult;
	}

//This functions adds an extension to the filename if there isn't an extension or
//if the extension doesn't match the filetype. Return is false if the extension
//is added.
bool
CCopyDocument::AddExtension() {

	bool bResult;
	int iPositionExtension;
	std::wstring sTempFilename;
	eCopyDocumentFiletype cFiletype;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;
	cFiletype = GetFiletype();

	sTempFilename = sFilename;
	std::transform(sTempFilename.begin(), sTempFilename.end(), sTempFilename.begin(), (int(*)(int)) tolower);
	iPositionExtension = (int)sTempFilename.length()-4;

	if (sTempFilename.rfind(L".jpg")==iPositionExtension && cFiletype==eJPG) bResult = true;
	if (sTempFilename.rfind(L".png")==iPositionExtension && cFiletype==ePNG) bResult = true;
	if (sTempFilename.rfind(L".pcx")==iPositionExtension && cFiletype==ePCX) bResult = true;
	if (sTempFilename.rfind(L".bmp")==iPositionExtension && cFiletype==eBMP) bResult = true;
	if (sTempFilename.rfind(L".pdf")==iPositionExtension && cFiletype==ePDF) bResult = true;
	if (sTempFilename.rfind(L".tif")==iPositionExtension && (cFiletype==eTIFFCompressed || cFiletype==eTIFFUncompressed)) bResult = true;

	iPositionExtension = (int)sTempFilename.length()-5;

	if (sTempFilename.rfind(L".jpeg")==iPositionExtension && cFiletype==eJPG) bResult = true;
	if (sTempFilename.rfind(L".tiff")==iPositionExtension && (cFiletype==eTIFFCompressed || cFiletype==eTIFFUncompressed)) bResult = true;

	if (!bResult) {

		sFilename += GetFiletypeExtension(cFiletype);
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

std::wstring 
CCopyDocument::GetFiletypeExtension(eCopyDocumentFiletype pcFiletype) {
	
	std::wstring sResult;

	switch (pcFiletype) {

		case eJPG:
			sResult =L".jpg";
			break;

		case ePNG:
			sResult =L".png";
			break;

		case ePDF:
			sResult =L".pdf";
			break;

		case eBMP:
			sResult =L".bmp";
			break;

		case ePCX:
			sResult =L".pcx";
			break;

		default:
			sResult =L".tiff";
			break;
		}

	return sResult;
	}


//these functions get and sets the filetype
eCopyDocumentFiletype 
CCopyDocument::GetFiletype() {

	return cFiletype;
	}

bool 
CCopyDocument::SetFiletype(eCopyDocumentFiletype pcFiletype) {

	bool bResult = true;
	
	//Before settings the filetype check if the filetype is valid in combination with a multipage document
	if (GetPageCount()>1 && pcFiletype!=eTIFFCompressed && pcFiletype!=eTIFFUncompressed && pcFiletype!=ePDF) {
		
		bResult = false; 
		}
	else {

		//Set the filetype
		cFiletype = pcFiletype;
		}

	return bResult;
	}

//This function force a file type in the save as file dialog
bool 
CCopyDocument::ForceFiletype(eCopyDocumentFiletype pcFiletype) {

	bool bResult = true;
	
	//Before settings the filetype check if the filetype is valid in combination with a multipage document
	if (GetPageCount()>1 && pcFiletype!=eTIFFCompressed && pcFiletype!=eTIFFUncompressed && pcFiletype!=ePDF) {
		
		bResult = false; 
		}
	else {

		//Set the filetype
		cForceFileType = pcFiletype;
		}

	return bResult;
	}

//This function returns the valid filetype given the document its characteristics
bool 
CCopyDocument::GetValidFiletypes(sCopyDocumentValidFileTypes &pcValidFiletypes) {

	pcValidFiletypes.eValidFiletypes.clear();

	switch (cForceFileType) {

		case eAny:
	
			pcValidFiletypes.eValidFiletypes.push_back(ePDF);

			if (GetPageCount()==1) {

				if (GetPage(0)->GetImage()->GetBitmap()->GetPixelFormat()==PixelFormat1bppIndexed) {
					pcValidFiletypes.eValidFiletypes.push_back(ePCX);
					}
				else {
					pcValidFiletypes.eValidFiletypes.push_back(eJPG);
					}
				
				pcValidFiletypes.eValidFiletypes.push_back(ePNG);
				}

			pcValidFiletypes.eValidFiletypes.push_back(eTIFFCompressed);
			pcValidFiletypes.eValidFiletypes.push_back(eTIFFUncompressed);

			if (GetPageCount()==1) {

				pcValidFiletypes.eValidFiletypes.push_back(eBMP);	
				}

			break;

		default:

			pcValidFiletypes.eValidFiletypes.push_back(cForceFileType);

			break;
	}

	return true;
}

eActionResult 
CCopyDocument::SaveAs(HWND phWnd) {

	wchar_t cFilename[MAX_PATH];
	std::wstring sFileFilters;
	int iDefaultFilter;
	eCopyDocumentFiletype cDefaultFilterType;
	eCopyDocumentFiletype cFiletype;
	sCopyDocumentValidFileTypes cValidFiletypes;
	eActionResult eResult;
	bool bResult;
	CCopyingMachineCore* oCore;
	CRegistry* oRegistry;

	oTrace->StartTrace(__WFUNCTION__);

	eResult = eCancelled;
	oRegistry = (CRegistry*)oGlobalInstances.oRegistry;

	SecureZeroMemory(cFilename, sizeof(cFilename));

	//Ask the document what filetypes are valid
	GetValidFiletypes(cValidFiletypes);

	//Determine the defailt filefilter
	cDefaultFilterType = GetFiletype();
	if (cDefaultFilterType == eUnknown) {
		

	if (oRegistry->ReadInt(L"General", L"TIFFcompression", 0)==0) {
		
		cDefaultFilterType = eTIFFUncompressed;
		}
	else {

		cDefaultFilterType = eTIFFCompressed;
		}
		//cDefaultFilterType = (eCopyDocumentFiletype)oRegistry->ReadInt(L"General", L"FilterSave", eUnknown);
		}
	//Build the filefilter
	iDefaultFilter = 0;
	
	for (int iIndex=0; iIndex<(int)cValidFiletypes.eValidFiletypes.size(); iIndex++) {

		//add description to string of filefilters
		switch (cValidFiletypes.eValidFiletypes[iIndex]) {

			case ePDF: sFileFilters += L"Adobe PDF (*.pdf)|*.pdf|"; break;
			case eJPG: sFileFilters += L"JPEG Compliant (*.jpg, *.jpeg)|*.jpg;*.jpeg|"; break;
			case eTIFFCompressed: sFileFilters += L"Tagged Image File Format compressed (*.tif, *.tiff)|*.tif;*.tiff|"; break;
			case eTIFFUncompressed: sFileFilters += L"Tagged Image File Format (*.tif, *.tiff)|*.tif;*.tiff|"; break;
			case eBMP: sFileFilters += L"Windows Bitmap (*.bmp)|*.bmp|"; break;
			case ePCX: sFileFilters += L"PC Paintbrush (*.pcx)|*.pcx|"; break;
			case ePNG: sFileFilters += L"Portable Network Graphics (*.png)|*.png|"; break;
			}

		//set the default filter, default is eTIFFCompressed
		if (cValidFiletypes.eValidFiletypes[iIndex] == cDefaultFilterType || 
			(cValidFiletypes.eValidFiletypes[iIndex] == eTIFFCompressed && (cDefaultFilterType==eUnknown || iDefaultFilter==0))) {

			iDefaultFilter = iIndex+1;
			}
		}

	//Convert the filefilter from wstring to wchar_t
	sFileFilters += L"|";

	wchar_t *cFileFilters;
	int iFileFilterLength;

	iFileFilterLength = (int)sFileFilters.length();

	cFileFilters = new wchar_t[iFileFilterLength+1];
	wcscpy_s(cFileFilters, iFileFilterLength+1, sFileFilters.c_str());

	//replace | by \0
	wchar_t* cPos;

	cPos = wcsrchr(cFileFilters, 124);
	
	while (cPos!=NULL) {
		
		cPos[0] = 0;
		
		cPos = wcsrchr(cFileFilters, 124);
		}

	//Copy filename from the document
	wcscpy_s(cFilename, MAX_PATH, ConvertFilenameToTitle().c_str());
	if (wcslen(cFilename)==0) {

		wcscpy_s(cFilename, MAX_PATH, GetTitle().c_str());
		}

	//Show the file save dialog
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	bResult = oCore->oDialogs->DlgSaveFile(phWnd, cFileFilters, iDefaultFilter, cFilename);

	delete []cFileFilters;

	//if result is positive, (try to) save the document
	if (bResult) {

		//oRegistry->WriteInt(L"General", L"FilterSaveMulti", (int)cValidFiletypes.eValidFiletypes[iDefaultFilter-1]);
		cFiletype = cValidFiletypes.eValidFiletypes[iDefaultFilter-1];

		SetFilename(cFilename);
		SetFiletype(cFiletype);
		eResult = Save(phWnd);

		if (eResult!=eOk) {

			oCore->oDialogs->DlgError(phWnd, eResult, 1, cFilename);
			}

		if (eResult==eOk) {

			//set the pages to non dirty
			for (int iIndex=0; iIndex<GetPageCount(); iIndex++) {

				GetPage(iIndex)->SetDirty(false);
				}
			bDirty = false;
			}
		}	

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;
	}

//This functions saves the document
eActionResult 
CCopyDocument::Save(HWND phWnd) {

	eActionResult eResult;

	oTrace->StartTrace(__WFUNCTION__);

	if (cFiletype == eUnknown ||
		(cFiletype != cForceFileType && cForceFileType != eAny)) {

		eResult = SaveAs(phWnd);
		}
	else {

		AddExtension();
		
		eResult = eOk;
		
		switch (GetFiletype()) {

			case ePCX: eResult = SavePCX(); break;
			case eJPG: eResult = SaveJPG(); break;
			case ePDF: eResult = SavePDF(); break;
			case eBMP: eResult = SaveBMP(); break;
			case ePNG: eResult = SaveBMP(); break;
			case eTIFFUncompressed: eResult = SaveTiff(); break; 
			case eTIFFCompressed: eResult = SaveCompressedTiff(); break; 
			default: eResult = SaveCompressedTiff(); break;
			}

		if (eResult==eOk) {

			//set the pages to non dirty
			for (int iIndex=0; iIndex<GetPageCount(); iIndex++) {

				GetPage(iIndex)->SetDirty(false);
				}
			bDirty = false;			
			}
		}

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;
	}

//This function process an autosave request
eActionResult 
CCopyDocument::AutoSaveSetDocumentName(HWND phWnd) {

	eActionResult cResult;
	CDocumentCollections* oAutoSave;
	std::wstring sAutoSaveFilename;
	int iSelectedItem;
	CCopyingMachineCore* oCore;
	CScanSettings* oLastScanSettings;
	sDocumentCollection cASDocumentType;

	cResult = eOk;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	if ((oGlobalInstances.oRegistry->ReadInt(L"General", L"AutoSave", 0)==1) ||
		(oCore->oScanCore->GetLastAcquisitionType() == eAcquireNewDocumentToCollection)){
	
		oTrace->StartTrace(__WFUNCTION__);

		if (oCore->oScanCore->GetLastAcquisitionType() == eAcquireNewDocumentToCollection) {
			
			iSelectedItem = 0;
			oLastScanSettings = oCore->oScanCore->GetLastScanSettings();

			if (oLastScanSettings) {

				iSelectedItem = oLastScanSettings->iCollection;
				}
			}
		else {

			iSelectedItem = oGlobalInstances.oRegistry->ReadInt(L"General", L"AutoSaveDocumentType", 0);
			}

		//Initialise
		oAutoSave = new CDocumentCollections(oGlobalInstances);

		if (oAutoSave->GetDescriptionFromUser(phWnd, iSelectedItem, iNewDocumentID)) {
		
			sAutoSaveFilename = oAutoSave->CreateFilename(iSelectedItem, iDocumentID, oCore->oScanSettings->sFolder);
			SetFilename(sAutoSaveFilename);
			oAutoSave->GetDocumentCollection(iSelectedItem, cASDocumentType);
			SetFiletype((eCopyDocumentFiletype)cASDocumentType.cFileType);
			AddExtension();
			}
		else {

			cResult = eCancelled;
			}

		//Clean up
		delete oAutoSave;

		oTrace->EndTrace(__WFUNCTION__, cResult==eOk);
		}

	return cResult;
}

//This function process an autosave request
eActionResult 
CCopyDocument::AutoSave(HWND phWnd) {

	eActionResult cResult;

	cResult = eOk;

	if (oGlobalInstances.oRegistry->ReadInt(L"General", L"AutoSave", 0)==1) {
	
		oTrace->StartTrace(__WFUNCTION__);

		if (!SetFiletype(GetFiletype())) {
			
			SetFiletype(eTIFFUncompressed);
			AddExtension();
			}

		//Process the image
		cResult = Save(phWnd);

		//Send a notification about the change
		SendNotification();

		oTrace->EndTrace(__WFUNCTION__, cResult==eOk);
		}

	return cResult;
}

//This function sends a notification 
void 
CCopyDocument::SendNotification(eCoreNotification peNotification) {

	CCopyingMachineCore* oCore;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oCore->oNotifications->CommunicateEvent(peNotification, (void*)iDocumentID);
}


//Save the document as a tiff file
eActionResult 
CCopyDocument::SaveTiff() {

	Status cResult;
	eActionResult eResult;
	Image* oSaveObject;
	Image* oPageImage;

	oTrace->StartTrace(__WFUNCTION__);

	eResult = eOk;

	//make a copy of the first page. It will serve as the image which
	//will be saved to the filesystem. We make a copy to not disturb/alter
	//the image in memory.
	GetPage(0)->SetMetadata(); //set the metadata in the image
	oPageImage = GetPage(0)->GetImage()->GetBitmap();
	oSaveObject = oPageImage->Clone();
	
	CLSID TiffClsid;

	ULONG	parameterValue;
	ULONG	compressionValue;

	EncoderParameters* pEncoderParameters = (EncoderParameters*)
		malloc(sizeof(EncoderParameters) + 2 * sizeof(EncoderParameter));

	// An EncoderParameters object has an array of
	// EncoderParameter objects. In this case, there is only
	// one EncoderParameter object in the array.
	pEncoderParameters->Count = 2;

	// Initialize the one EncoderParameter object.
	pEncoderParameters->Parameter[0].Guid = EncoderSaveFlag;
	pEncoderParameters->Parameter[0].Type = EncoderParameterValueTypeLong;
	pEncoderParameters->Parameter[0].NumberOfValues = 1;
	pEncoderParameters->Parameter[0].Value = &parameterValue;

	pEncoderParameters->Parameter[1].Guid = EncoderCompression;
	pEncoderParameters->Parameter[1].Type = EncoderParameterValueTypeLong;
	pEncoderParameters->Parameter[1].NumberOfValues = 1;
	pEncoderParameters->Parameter[1].Value = &compressionValue;

	GetEncoderClsid(L"image/tiff", &TiffClsid);

	parameterValue = EncoderValueMultiFrame;
	compressionValue = oPageImage->GetPixelFormat() == PixelFormat1bppIndexed ? 
		EncoderValueCompressionCCITT4 : EncoderValueCompressionLZW;
	if (GetFiletype()==eTIFFUncompressed) compressionValue = EncoderValueCompressionNone;

	cResult = oSaveObject->Save(GetFilename().c_str(), &TiffClsid, pEncoderParameters);

	if (cResult==Ok) {

		for (int iIndex=1; iIndex<GetPageCount() && cResult==Ok; iIndex++) {

			parameterValue = EncoderValueFrameDimensionPage;
			GetPage(iIndex)->SetMetadata(); //set the metadata in the image
			oPageImage = GetPage(iIndex)->GetImage()->GetBitmap();
			compressionValue = oPageImage->GetPixelFormat() == PixelFormat1bppIndexed ? 
				EncoderValueCompressionCCITT4 : EncoderValueCompressionLZW;
			if (GetFiletype()==eTIFFUncompressed) compressionValue = EncoderValueCompressionNone;

			cResult = oSaveObject->SaveAdd(oPageImage, pEncoderParameters);
			}

		// Close the multiframe file.
		parameterValue = EncoderValueFlush;
		cResult = oSaveObject->SaveAdd(pEncoderParameters);
		}

	if (cResult!=Ok) eResult = eFileResultCouldNotCreate;

	free(pEncoderParameters);
	delete oSaveObject;

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;
	}

//Save the document as a compressed tiff file
eActionResult
CCopyDocument::SaveCompressedTiff() {

	TIFF* TIFF_FileHandle;
	eActionResult eResult;
	int iRowIndex;
	long lRowSize;
	long lPhotoMetric;
	long lBitsPerPixel;
	long lImageWidth;
	long lImageHeight;
	BYTE* pRowBuffer;
	BYTE* memScanLine;
	HGLOBAL memHandle;
	Bitmap* oBitmap;

	oTrace->StartTrace(__WFUNCTION__);

	eResult = eOk;
	
	//Open file
	TIFF_FileHandle = TIFFOpenW(GetFilename().c_str(), "w");
	if (TIFF_FileHandle == NULL) {
		//terminating
		oTrace->EndTrace(__WFUNCTION__, eFileResultCouldNotCreate);
		return eFileResultNotFound;
		}

	TIFFWriteBufferSetup(TIFF_FileHandle, NULL, 131072);

	for (int iPageIndex=0; (iPageIndex<GetPageCount() && (eResult == eOk)); iPageIndex++) {

		oBitmap = GetPage(iPageIndex)->GetImage()->GetBitmap();
		
		TIFFSetField(TIFF_FileHandle, TIFFTAG_IMAGEWIDTH, oBitmap->GetWidth());
		TIFFSetField(TIFF_FileHandle, TIFFTAG_IMAGELENGTH, oBitmap->GetHeight());
		TIFFSetField(TIFF_FileHandle, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
		TIFFSetField(TIFF_FileHandle, TIFFTAG_PAGENUMBER, 0, GetPageCount());
		TIFFSetField(TIFF_FileHandle, TIFFTAG_XRESOLUTION, (float)oBitmap->GetHorizontalResolution());
		TIFFSetField(TIFF_FileHandle, TIFFTAG_YRESOLUTION, (float)oBitmap->GetVerticalResolution());
		TIFFSetField(TIFF_FileHandle, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(TIFF_FileHandle, TIFFTAG_ROWSPERSTRIP, 1);
		TIFFSetField(TIFF_FileHandle, TIFFTAG_PLANARCONFIG, 1);

		//get the palette and determine if the first color is black or white
		if (oBitmap->GetPixelFormat() == PixelFormat1bppIndexed ||
			oBitmap->GetPixelFormat() == PixelFormat8bppIndexed) {

			lPhotoMetric = PHOTOMETRIC_MINISBLACK;
			long lPaletteSize = oBitmap->GetPaletteSize();
			ColorPalette* oPalette = (ColorPalette*)malloc(lPaletteSize);
			if (oBitmap->GetPalette(oPalette, lPaletteSize)==Ok) {

				if ((oPalette->Entries[0] & 0xFFFFFF) == 0xFFFFFF) {

					lPhotoMetric = PHOTOMETRIC_MINISWHITE; //white = 0
					}
				}

			free (oPalette);
			}
		
		//lock the bits of the bitmap and get the bitmapdata info
		Rect rect(0, 0, oBitmap->GetWidth(), oBitmap->GetHeight());
		BitmapData* oBitmapData = new BitmapData;
		oBitmap->LockBits(&rect, ImageLockModeRead, oBitmap->GetPixelFormat(), oBitmapData);

		PixelFormat pixelFormat;

		pixelFormat = oBitmap->GetPixelFormat();

		switch (pixelFormat)  {

         case PixelFormat1bppIndexed: {

            lBitsPerPixel = 1;

            TIFFSetField(TIFF_FileHandle, TIFFTAG_BITSPERSAMPLE, 1);
            TIFFSetField(TIFF_FileHandle, TIFFTAG_SAMPLESPERPIXEL, 1); 
			TIFFSetField(TIFF_FileHandle, TIFFTAG_COMPRESSION, GetFiletype()==eTIFFCompressed ?  COMPRESSION_PACKBITS : COMPRESSION_NONE);
            TIFFSetField(TIFF_FileHandle, TIFFTAG_PHOTOMETRIC, lPhotoMetric);
            
       		pRowBuffer = (BYTE*)oBitmapData->Scan0;
            break;
            }


         case PixelFormat8bppIndexed: {

            lBitsPerPixel = 8;

            TIFFSetField(TIFF_FileHandle, TIFFTAG_BITSPERSAMPLE, 8);
            TIFFSetField(TIFF_FileHandle, TIFFTAG_SAMPLESPERPIXEL, 1);
			if (GetFiletype()==eTIFFCompressed) {

				TIFFSetField(TIFF_FileHandle, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
      			TIFFSetField(TIFF_FileHandle, TIFFTAG_JPEGQUALITY, 50);
				TIFFSetField(TIFF_FileHandle, TIFFTAG_ROWSPERSTRIP, 8);
				}
			else {

				TIFFSetField(TIFF_FileHandle, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
				}

            TIFFSetField(TIFF_FileHandle, TIFFTAG_PHOTOMETRIC, lPhotoMetric);

       		pRowBuffer = (BYTE*)oBitmapData->Scan0;
			break;
            }

         case PixelFormat24bppRGB: {
            //24 bits colors

            lBitsPerPixel = 24;

            TIFFSetField(TIFF_FileHandle, TIFFTAG_BITSPERSAMPLE, 8);
            TIFFSetField(TIFF_FileHandle, TIFFTAG_SAMPLESPERPIXEL, 3);
            TIFFSetField(TIFF_FileHandle, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			if (GetFiletype()==eTIFFCompressed) {

				TIFFSetField(TIFF_FileHandle, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
      			TIFFSetField(TIFF_FileHandle, TIFFTAG_JPEGQUALITY, 50);
				TIFFSetField(TIFF_FileHandle, TIFFTAG_ROWSPERSTRIP, 8);
				}
			else {

				TIFFSetField(TIFF_FileHandle, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
				}

       		pRowBuffer = (BYTE*)oBitmapData->Scan0;
            break;
            }
	
		 default: {

			//unsupported pixelformat, error it
			 eResult = eFileResultNotSupported;
			 break;
			}
      }
		
	if (eResult == eOk) {

		//write the lines
		lImageWidth = oBitmap->GetWidth();
		lImageHeight = oBitmap->GetHeight();
		lRowSize = CBitmap::CalculateRowsize(lImageWidth, lBitsPerPixel);

		memHandle = ::GlobalAlloc(GMEM_MOVEABLE, lRowSize);
		memScanLine = (BYTE*)::GlobalLock(memHandle);

		iRowIndex=0;

		do {
			//kopieer de data naar de scanline buffer
			//memcpy(memScanLine, pRowBuffer, lRowSize);

			if (lBitsPerPixel==24) {
				//het is een 24 bit kleur image. De kleuren omgooien voor de tiff.
				for (int j=0;j<lImageWidth;j++) {
					memScanLine[j*3+0]=pRowBuffer[j*3+2];
					memScanLine[j*3+1]=pRowBuffer[j*3+1];
					memScanLine[j*3+2]=pRowBuffer[j*3+0];
					}
				}
			else {
				memcpy(memScanLine, pRowBuffer, lRowSize);
			}

			 if (TIFFWriteScanline(TIFF_FileHandle, memScanLine, iRowIndex, 0) < 0) {
				//terminating
				eResult = eFileResultUnknownError;
				}

			 iRowIndex++;
			 pRowBuffer += oBitmapData->Stride;
			 }
		  while ((iRowIndex<lImageHeight) && (eResult==eOk));

		::GlobalUnlock(memHandle);
		::GlobalFree(memHandle);

		TIFFWriteDirectory (TIFF_FileHandle);
		}

	//Clear up and return the results
	oBitmap->UnlockBits(oBitmapData);
	delete oBitmapData;
	}

	TIFFClose(TIFF_FileHandle);

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;
	}


//Save the document as a jpeg file
eActionResult 
CCopyDocument::SaveJPG() {

	eActionResult eResult;
	int iResult;
	Image* oSaveObject;
	Image* oPageImage;

	oTrace->StartTrace(__WFUNCTION__);

	//make a copy of the first page. It will serve as the image which
	//will be saved to the filesystem. We make a copy to not disturb/alter
	//the image in memory.
	GetPage(0)->SetMetadata(); //set the metadata in the image
	oPageImage = GetPage(0)->GetImage()->GetBitmap();
	oSaveObject = oPageImage->Clone();
	
	CLSID TiffClsid;

	ULONG	compressionValue;

	EncoderParameters* pEncoderParameters = (EncoderParameters*)
		malloc(sizeof(EncoderParameters) + 2 * sizeof(EncoderParameter));

   // An EncoderParameters object has an array of
   // EncoderParameter objects. In this case, there is only
   // one EncoderParameter object in the array.
   pEncoderParameters->Count = 1;

   // Initialize the one EncoderParameter object.
   pEncoderParameters->Parameter[0].Guid = EncoderQuality;
   pEncoderParameters->Parameter[0].Type = EncoderParameterValueTypeLong;
   pEncoderParameters->Parameter[0].NumberOfValues = 1;
   pEncoderParameters->Parameter[0].Value = &compressionValue;

   GetEncoderClsid(L"image/jpeg", &TiffClsid);

	compressionValue = oGlobalInstances.oRegistry->ReadInt(L"General", L"JPEGcompression", 75);

	iResult = oSaveObject->Save(GetFilename().c_str(), &TiffClsid, pEncoderParameters);

	free(pEncoderParameters);
	delete oSaveObject;

	eResult = CBitmap::ConvertGDIPlusToFileResult(iResult);

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;
	}

//Save the document as a windows bitmap file
eActionResult 
CCopyDocument::SaveBMP() {

	eActionResult eResult;
	int iResult;
	Image* oSaveObject;
	Image* oPageImage;

	oTrace->StartTrace(__WFUNCTION__);

	eResult = eOk;

	//make a copy of the first page. It will serve as the image which
	//will be saved to the filesystem. We make a copy to not disturb/alter
	//the image in memory.
	GetPage(0)->SetMetadata(); //set the metadata in the image
	oPageImage = GetPage(0)->GetImage()->GetBitmap();
	oSaveObject = oPageImage->Clone();
	
	CLSID ImageClsid;

	switch (GetFiletype()) {
		
		case ePNG: 
			GetEncoderClsid(L"image/png", &ImageClsid);
			break;

		case eBMP:
			if (GetEncoderClsid(L"image/bmp", &ImageClsid)<0)
			{
			//	MessageBox(NULL, L"ok", L"ok", MB_OK);
			};
			break;
		}

	iResult = oSaveObject->Save(GetFilename().c_str(), &ImageClsid, NULL); //pEncoderParameters not necessary for PNG and BMP

	delete oSaveObject;

	eResult = CBitmap::ConvertGDIPlusToFileResult(iResult);

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;
	}

//Save the document as a pcx file
eActionResult 
CCopyDocument::SavePCX() {

	eActionResult eResult;
	int iResult;
	CBitmapPCX oPCX;

	oTrace->StartTrace(__WFUNCTION__);

	eResult = eOk;

	if (!oPCX.SavePCX(GetFilename(), GetPage(0)->GetImage()->GetBitmap())) {

		iResult = oPCX.GetLastStatus();
		eResult = CBitmap::ConvertGDIPlusToFileResult(iResult);
		}

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;
	}

//Save the document as a PDF file
eActionResult 
CCopyDocument::SavePDF() {

	return SavePDF(GetFilename());
	}

//Save the document as a PDF file
eActionResult 
CCopyDocument::SavePDF(std::wstring psFilename) {

	bool bResult;
	eActionResult eResult;
	long lPageId;
	long lCount;
	CPDF* oPDFDocument;
	CCopyDocumentPage* oPage;
	CRecognitionLayer* oRecognitionLayer;
	CRecognizedText* oText;
	sRecognizedCharacter cCharacter;
	sRecognizedCharacter cLookForwardCharacter;
	std::wstring sText;
	std::string sConvertedText;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;
	eResult = eOk;

	//Create the PDF document instance
	oPDFDocument = new CPDF();
	oPDFDocument->lJPEGCompression = 50;

	//Add the pages to the document
	//lPageId = oPDFDocument->AddPage();
	//bResult = oPDFDocument->AddTextToPage(lPageId, "Hello World");

	for (long lIndex=0; lIndex<GetPageCount() && bResult; lIndex++) {
	
		lPageId = oPDFDocument->AddPage();
		
		oPage = GetPage(lIndex);

		if (oPage!=NULL) {
			
			bResult = oPDFDocument->AddBitmapToPage(lPageId, oPage->GetImage()->GetBitmap());

			oRecognitionLayer = oPage->GetRecognitionLayer();
			lCount = oRecognitionLayer->GetRecognizedTextCount();
			
			std::string sDestination;
			
			while (lCount>0 && bResult) {
				
				lCount--;

				oText = oRecognitionLayer->GetRecognizedText(lCount);

				long lChar = 0;
				bool bWord = false;
				long lNumberChar = oText->GetCharacterCount();
				sDestination = "";
				RectF cSentenceRect;

				cSentenceRect = RectF(0, 0, 0, 0);


				while (lChar<lNumberChar && bResult) {

					cCharacter = oText->GetCharacterStruct(lChar);

					for (long lSpaces = 0; lSpaces<cCharacter.iSpaces; lSpaces++) {

						//sDestination+=" ";
						//bWord = true;
						}

					sDestination += wide_to_narrow(oText->GetCharacter(lChar));

					if (cSentenceRect.Width == 0) {
						
						cSentenceRect.X = (REAL)cCharacter.cBoundingBox.X;
						cSentenceRect.Y = (REAL)cCharacter.cBoundingBox.Y;
						}
					else {

						cSentenceRect.X = min(cSentenceRect.X, cCharacter.cBoundingBox.X);
						cSentenceRect.Y = min(cSentenceRect.Y, cCharacter.cBoundingBox.Y);
						}

					cSentenceRect.Height = max(cSentenceRect.Height, cCharacter.cBoundingBox.Height);
					cSentenceRect.Width = max(cSentenceRect.Width, cCharacter.cBoundingBox.Width + cCharacter.cBoundingBox.X - cSentenceRect.X);

					lChar++;

					if (lChar<lNumberChar) {
						
						cLookForwardCharacter = oText->GetCharacterStruct(lChar);
						if (cLookForwardCharacter.iSpaces>0) bWord = true;
						}
					
					if ((cCharacter.iEndOfLines>0 || lChar==lNumberChar || bWord) && cSentenceRect.Width>0) {

						cSentenceRect.X += oText->GetBoundingBox().X;
						cSentenceRect.Y += oText->GetBoundingBox().Y;
						
						bResult = bResult & oPDFDocument->AddTextToPage(lPageId, sDestination, cSentenceRect);
						
						sDestination = "";
						bWord = false;
						cSentenceRect = RectF(0, 0, 0, 0);
						}
				}

				//sText = oText->GetText();

				//sDestination="";
				//for (long lChar=0; lChar<(long)sText.length(); lChar++) {

				//	sDestination += wide_to_narrow(sText[lChar]);
				//	}

				//
				//
				//bResult = bResult & oPDFDocument->AddTextToPage(lPageId, sDestination, oText->GetBoundingBox());
				}
			}
		}

	//Save the document to a file
	if (bResult) oPDFDocument->WriteToFile(psFilename);	

	if (!bResult) eResult = eFileResultUnknownError;

	//Clean up and return
	delete oPDFDocument;

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;
	}

int 
CCopyDocument::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

//This function mails the current document as an attachment
eActionResult
CCopyDocument::MailTo() {

	eActionResult eResult;
	CMapiInterface* oMapi;
	wchar_t	 cTempFilename[MAX_PATH];
	wchar_t	 cTempPath[MAX_PATH];
	std::wstring sTempFilename;
	std::wstring sDocumentName;

	oTrace->StartTrace(__WFUNCTION__);

	eResult = eFileResultUnknownError;

	//save the current file as a pdf for e-mailing
	::GetTempPath(MAX_PATH, cTempPath);
	if (::GetTempFileName(cTempPath, L"doc", 0, cTempFilename)!=0) {

		sTempFilename = cTempFilename;

		sDocumentName = GetTitle();
		sDocumentName += L".pdf";

		SavePDF(sTempFilename);

		//open the mapi interface and send the file
		oMapi = new CMapiInterface();

		oMapi->SendFile(sDocumentName, sTempFilename);

		delete oMapi;

		//delete the temporary file
		::DeleteFile(sTempFilename.c_str());

		eResult = eOk;
		}

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;	
	}

//This function is the start of making a copy of a document on a printer
eActionResult
CCopyDocument::Print(CPrintSettings* poPrintSettings) {

	eActionResult eResult;
	CCopyingMachineCore* oCore;


	oTrace->StartTrace(__WFUNCTION__);

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	if (poPrintSettings==NULL) {
		poPrintSettings = oCore->oPrintSettings;
		}

	eResult = oCore->oPrintCore->PrintDocument(this, poPrintSettings);

	oTrace->EndTrace(__WFUNCTION__, eResult);

	return eResult;
	}

//This function is the start of making a copy of a document on a printer
int
CCopyDocument::PrintThread(CPrintSettings* poPrintSettings) {

	int iResult;
	CCopyingMachineCore* oCore;


	oTrace->StartTrace(__WFUNCTION__);

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	if (poPrintSettings==NULL) {
		poPrintSettings = oCore->oPrintSettings;
		}

	iResult = oCore->oPrintCore->PrintDocumentThread(this, poPrintSettings);

	oTrace->EndTrace(__WFUNCTION__, iResult);

	return iResult;
	}

//This function retrieves the memory size of the current document
DWORD 
CCopyDocument::GetMemorySize() {

	DWORD dResult;
	
	dResult = 0;

	for (long lIndex=0; lIndex<(long)oPages.size(); lIndex++) {

		dResult += oPages[lIndex]->GetMemorySize();
		}

	return dResult;
}

//This function delete the requested page	
eActionResult 
CCopyDocument::DoDeletePage(DWORD pdData, HWND phWnd) {

	eActionResult cResult;
	CCopyingMachineCore* oCore;
	int iDeletedIndex;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;
	iDeletedIndex = -1;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	if (CheckPageImageAction(ePageDelete, pdData)) {

		for (int iIndex=oPages.size()-1; iIndex>=0 && cResult!=eOk; iIndex--) {

			if (oPages[iIndex]->GetPageID() == pdData) {

				delete oPages[iIndex];
				oPages.erase(oPages.begin() + iIndex);
				iDeletedIndex = iIndex;

				oCore->oNotifications->CommunicateEvent(eNotificationPageDeleted, (void*)pdData);
				SetDirty();
				cResult = eOk;
				}
			}

		if (iDeletedIndex>=0) {

			iDeletedIndex = max(iDeletedIndex, (int)oPages.size()-1);

			oCore->SetCurrentPage(iDeletedIndex);
			}
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}
	
//This function copies the given page
eActionResult 
CCopyDocument::DoCopyPage(DWORD pdData, HWND phWnd) {

	eActionResult cResult;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;

	if (CheckPageImageAction(ePageCopy, pdData)) {

		for (int iIndex=oPages.size()-1; iIndex>=0 && cResult!=eOk; iIndex--) {

			if (oPages[iIndex]->GetPageID() == pdData) {

				cResult = oPages[iIndex]->GetImage()->CopyToClipboard(phWnd);
				}
			}
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}

//This function cuts the give page, in fact is it a combined copy and delete
//action
eActionResult 
CCopyDocument::DoCutPage(DWORD pdData, HWND phWnd) {

	eActionResult cResult;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;

	if (CheckPageImageAction(ePageCut, pdData)) {

		cResult = DoCopyPage(pdData, phWnd);
		if (cResult==eOk) cResult = DoDeletePage(pdData, phWnd);
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}

//Move the given page to a new position, it returns the new position 
//of the previous page on that position
int 
CCopyDocument::DoMovePage(DWORD pdData, int iNewPosition) {

	int iResult;
	CCopyDocumentPage* oNewPage;
	CCopyDocumentPage* oOldPage;
	
	oTrace->StartTrace(__WFUNCTION__);

	iResult = -1;

	oNewPage = GetPageByID(pdData);
	
	
	if (iNewPosition >= GetPageCount()) {

		//Move pages to the last position
		if (oNewPage!=NULL) {

			oPages.erase(oPages.begin() + GetPageIndex(oNewPage));
			oPages.push_back(oNewPage);

			iResult = GetPageCount();
			}
		}
	else {

		oOldPage = GetPage(iNewPosition);

		if (oOldPage!=NULL && oNewPage!=NULL) {

			oPages.erase(oPages.begin() + GetPageIndex(oNewPage));
			oPages.insert(oPages.begin() + GetPageIndex(oOldPage), oNewPage);

			iResult = GetPageIndex(oOldPage);
			}
		}

	oTrace->EndTrace(__WFUNCTION__);

	return iResult;
	}

//Move the given page to another document.
int 
CCopyDocument::DoMovePage(DWORD pdData, CCopyDocument* poDocument) {

	int iResult;
	CCopyDocumentPage* oPage;
	CCopyingMachineCore* oCore;
	
	oTrace->StartTrace(__WFUNCTION__);

	iResult = -1;

	oPage = GetPageByID(pdData);

	if (oPage!=NULL && poDocument!=NULL) {

		oPages.erase(oPages.begin() + GetPageIndex(oPage));
		poDocument->InsertNewPage(poDocument->GetPageCount(), oPage);
		oPage->SetDirty(true);

		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		oCore->oNotifications->CommunicateEvent(eNotificationPageDeleted, (void*)pdData);
		}

	oTrace->EndTrace(__WFUNCTION__);

	return iResult;
	}

//This function checks if the current action is possible.	
bool 
CCopyDocument::CheckPageImageAction(ePageImageAction pcAction, DWORD pdData) {
	
	bool bResult;
	CCopyingMachineCore* oCore;
	CCopyDocument* oDocument;
	CCopyDocumentPage* oPage;

	oTrace->StartTrace(__WFUNCTION__,eAll);

	bResult = false;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	switch (pcAction) {

		case ePageDelete:
		case ePageCut: // a cut/delete is possible if it isn't the last page

			oDocument = oCore->GetCurrentDocument();

			if (oDocument!=NULL) {

				bResult = (oDocument->GetPageCount()>1);
				}		

			break;

		case ePageSplit: //a split is possible if the selected page isn't the first page

			oDocument = oCore->GetCurrentDocument();
			oPage = oCore->GetCurrentPage();

			if (oDocument!=NULL && oPage!=NULL) {

				bResult = (oDocument->GetPageIndex(oPage)>0);
				}		

			break;

		case ePageCopy: // a copy is always possible

			bResult = true;
			break;
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	
	return bResult;
	}

//Script functions
bool 
CCopyDocument::Set(std::wstring psObjectname, sScriptExpression psNewValue) {

	bool bResult;

	bResult = CScriptObject::Set(psObjectname, psNewValue);

	if (!bResult && QueryType(psObjectname) == eScriptObjectAttribute) { 

		//process the document attributes
		if (psObjectname == L"name") {

			bResult = psNewValue.toString();

			if (bResult) {

				SetTitle(psNewValue.sResult);
				}
			}
		}

	return bResult;
	}


sScriptExpression 
CCopyDocument::Get(std::wstring psObjectname) {

	sScriptExpression oResult;

	oResult = CScriptObject::Get(psObjectname);

	if (oResult.cValueType == eScriptValueAny &&
		QueryType(psObjectname) == eScriptObjectAttribute) { 

		//process the document attributes
		if (psObjectname == L"name") {

			oResult.sResult = GetTitle();
			oResult.cValueType = eScriptValueString;
			}
		}

	return oResult;
	}

bool 
CCopyDocument::Execute(std::wstring psObjectname, 
							 sScriptExpression &psValue) {

	bool bResult;
	CCopyingMachineCore* oCore;

	bResult = false;

	bResult = CScriptObject::Execute(psObjectname, psValue);
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	if (!bResult) {
		
		if (psObjectname == L"save") {

			psValue.iResult = (int)Save(oCore->hMainApplicationWindow);
			psValue.cValueType = eScriptValueInteger;
			}
		if (psObjectname == L"saveas") {

			psValue.iResult = (int)SaveAs(oCore->hMainApplicationWindow);
			psValue.cValueType = eScriptValueInteger;
			}
		if (psObjectname == L"mailto") {

			psValue.iResult = (int)MailTo();
			psValue.cValueType = eScriptValueInteger;
			}
		if (psObjectname == L"pagecount") {

			psValue.iResult = (int)GetPageCount();
			psValue.cValueType = eScriptValueInteger;
			}

		}

	return bResult;
	}



