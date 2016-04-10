#include "StdAfx.h"
#include "BitmapPCX.h"
#include "CopyingMachineCore.h"
#include "DlgSaveChanges.h"
#include "tiffio.h"

static int
checkcmap(int n, uint16* r, uint16* g, uint16* b)
{
   while (n-- > 0)
     if (*r++ >= 256 || *g++ >= 256 || *b++ >= 256)
       return (16);
   /* Assuming 8-bit colormap */
   return (8);
 }

#define CVT(x)          (((x) * 255L) / ((1L<<16)-1))

CCopyDocuments::CCopyDocuments(sGlobalInstances pInstances)
{
	//Set helper classes
	oTrace = pInstances.oTrace;	
	oNotification = (CCoreNotifications*)pInstances.oNotifications;

	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pInstances, sizeof(sGlobalInstances));

	//Init variables
	iNewDocumentCounter = 0;
}

CCopyDocuments::~CCopyDocuments(void)
{
	oTrace->StartTrace(__WFUNCTION__);

	//Clear all the pages
	CCopyDocument* oDocument;

	for (long lIndex=(long)oDocuments.size()-1; lIndex>=0; lIndex--) {

		oDocument = oDocuments[lIndex];
		delete oDocument;
		}

	oDocuments.clear();

	oTrace->EndTrace(__WFUNCTION__);
	}

//this function creates a new document
CCopyDocument* 
CCopyDocuments::CreateNewDocument() {

	CCopyDocument* oResult;
	wchar_t cTitle[50];


	oTrace->StartTrace(__WFUNCTION__);
	
	oResult = NULL;

	oResult = new CCopyDocument(oGlobalInstances);

	if (oResult!=NULL) {

		iNewDocumentCounter++;
		
		LoadStringW(oGlobalInstances.hLanguage, IDS_NEWDOCUMENTTITLE, cTitle, 50);
		swprintf_s(cTitle, 50, cTitle, iNewDocumentCounter);
		
		oResult->SetTitle(cTitle);
		oResult->iNewDocumentID = iNewDocumentCounter;

		oDocuments.push_back(oResult);
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)(oResult!=NULL));

	return oResult;
	}

//This function returns the number of documents
int	
CCopyDocuments::GetDocumentCount() {

	return (int)oDocuments.size();
	}

CCopyDocument* 
CCopyDocuments::GetDocument(long plIndex) {

	CCopyDocument* oResult;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oResult = NULL;

	if (plIndex>=0 && plIndex<GetDocumentCount()) {

		oResult = oDocuments[plIndex];
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)(oResult!=NULL), eAll);

	return oResult;
	}

CCopyDocument* 
CCopyDocuments::GetDocumentByID(int piDocumentID) {

	CCopyDocument* oResult;
	int iIndex;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oResult = NULL;
	iIndex = GetDocumentCount()-1;

	while (oResult==NULL && iIndex>=0) {

		if (oDocuments[iIndex]->GetDocumentID() == piDocumentID) {
			oResult = oDocuments[iIndex];
			}

		iIndex--;
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)(oResult!=NULL), eAll);

	return oResult;
	}

//This function returns the page instance of the give PageID (if
//it exists)
CCopyDocumentPage* 
CCopyDocuments::GetPageByID(DWORD pdPageID) {

	CCopyDocumentPage* oResult;
	int iIndex;

	oTrace->StartTrace(__WFUNCTION__);

	oResult = NULL;
	iIndex = GetDocumentCount();

	//Loop through all the documents, it isn't the best
	//method but it works fast enough.
	while (oResult==NULL && iIndex>0) {

		iIndex--;

		oResult = oDocuments[iIndex]->GetPageByID(pdPageID);
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)(oResult!=NULL));

	return oResult;
	}


//This function loads a new document, no load-dialog is shown.
eActionResult
CCopyDocuments::LoadDocument(std::wstring psFilename, CCopyDocument* poResultDocument) {

	CBitmap* oBitmap;
	CCopyDocument* oNewDocument;
	eActionResult eResult;
	sLoadFileResult oLoadResult;
	sLoadFilePageResult oPageResult;
	eCopyDocumentFiletype cFiletype;
	CBitmapPCX oPCX;

	oTrace->StartTrace(__WFUNCTION__);
	
	oNewDocument = NULL;

	eResult = eFileResultUnknownError;

	//test if the file exists
	FILE * fp;

	if ((_wfopen_s(&fp, psFilename.c_str(), L"rb"))!=0) {

		eResult = eFileResultNotFound;
		return eResult;
		}
	fclose(fp);

	//Open the file
	cFiletype = CCopyDocument::DetermineFiletype(psFilename);

	switch (cFiletype) {

		case ePCX:
			
			oPageResult.oBitmap = oPCX.LoadPCX(psFilename, oPageResult.hMemoryBitmap);
			
			oLoadResult.oPages.push_back(oPageResult);
			oLoadResult.eResult = oPCX.GetLastStatus();
			oLoadResult.eFiletype = ePCX;

			break;

		case ePDF:
		
			ShellExecute(NULL, L"open", psFilename.c_str(), NULL, NULL, SW_SHOWNORMAL);
			
			oLoadResult.eResult = eFileResultExternalFile;
			oLoadResult.eFiletype = ePDF;			
			break;

		case eTIFFCompressed:
		case eTIFFUncompressed:

			oLoadResult = LoadCompressedTiff(psFilename);

			break;

		default:

			oLoadResult.eResult = eFileResultUnknownError;

			IStream *oFileStream(0); 
			if (SUCCEEDED (SHCreateStreamOnFile(psFilename.c_str(), 
				STGM_READ | STGM_SHARE_DENY_NONE, 
				&oFileStream))) {

				ULARGE_INTEGER FileSize;
				if (SUCCEEDED (IStream_Size(oFileStream, &FileSize))) {

					IStream *oMemoryStream(0);
					oMemoryStream = SHCreateMemStream(NULL, FileSize.LowPart);

					if (oMemoryStream != NULL) {

						if (SUCCEEDED (oFileStream->CopyTo(oMemoryStream, FileSize, NULL, NULL))) {

							oPageResult.oBitmap = Bitmap::FromStream(oMemoryStream);

							oLoadResult.eResult = CBitmap::ConvertGDIPlusToFileResult(oPageResult.oBitmap->GetLastStatus());

							oFileStream->Release();

							oPageResult.hMemoryBitmap = NULL;

							oLoadResult.oPages.push_back(oPageResult);
							oLoadResult.eFiletype = cFiletype;
						}
					}
				}
			}

			break;
		}

	//If the filetype was already unknown and the attempt to open it returned
	//an unknown error, we can safely assume the filetype isn't supported.
	if (oLoadResult.eResult == eFileResultUnknownError && cFiletype == eUnknown) {
		
		oLoadResult.eResult = eFileResultNotSupported;
		}

	//Process the newly opened document
	if (oLoadResult.eResult==eOk) {

		if (oLoadResult.eFiletype != ePDF) {

			oNewDocument = new CCopyDocument(oGlobalInstances);
			oNewDocument->SetFilename(psFilename);
			oNewDocument->SetFiletype(oLoadResult.eFiletype);

			for (int iIndex=0; iIndex<(int)oLoadResult.oPages.size(); iIndex++) {

				oBitmap = new CBitmap(oLoadResult.oPages[iIndex].oBitmap, oLoadResult.oPages[iIndex].hMemoryBitmap);

				oNewDocument->InsertNewPage(oNewDocument->GetPageCount(), oBitmap);
				}

			//if succesful add the new document to the vector
			if (oLoadResult.eResult==eOk) {
				oDocuments.push_back(oNewDocument);
				}
			else {
				delete oNewDocument;
				oNewDocument = NULL;
				}
			}
		}
	else {
		
		//clear up bitmaps
		for (int iIndex=0; iIndex<(int)oLoadResult.oPages.size(); iIndex++) {

			delete oLoadResult.oPages[iIndex].oBitmap;
			GlobalFree(oLoadResult.oPages[iIndex].hMemoryBitmap);
			}
		}

	eResult = oLoadResult.eResult;

	oTrace->EndTrace(__WFUNCTION__, eResult);

	poResultDocument = oNewDocument;
	return eResult;
	}

//This function asks the user if he wants to save changes to
//a dirty document. This function is called prior to a termination
//of the application. Result = false when a save isn't succesful or
//the users pressed cancel.
bool 
CCopyDocuments::SaveChangesToDirtyDocuments(HWND phWnd) {

	int iResult;
	long lIndex;
	bool bContinue;
	bool bYesToAll;

	oTrace->StartTrace(__WFUNCTION__);
	
	iResult = IDYES;
	bContinue = true;
	bYesToAll = false;
	
	lIndex = oDocuments.size();

	while (lIndex>0 && bContinue) {

		lIndex--;	
		iResult = SaveChangesToDirtyDocument(phWnd, oDocuments[lIndex], !bYesToAll);

		bContinue = (iResult!=IDCANCEL && iResult!=IDNO2);
		bYesToAll = (iResult==IDYES2);
		}

	oTrace->EndTrace(__WFUNCTION__, iResult);

	return iResult != IDCANCEL;
	}

//This function asks the user if he wants to save changes to
//a dirty document. This function is called prior to a termination
//of the application. Result = false when a save isn't succesful or
//the users pressed cancel.
int 
CCopyDocuments::SaveChangesToDirtyDocument(HWND phWnd, CCopyDocument* poDocument, bool bShowSaveChangesDialog) {

	wchar_t cTemplate[MAX_PATH]; 
	wchar_t cText[MAX_PATH];
	eActionResult eResult;
	CCopyingMachineCore* oCore;
	int iResult;

	oTrace->StartTrace(__WFUNCTION__);

	iResult = IDCANCEL;

	if (poDocument!=NULL) {
		if (poDocument->GetDirty()) {

			LoadString(oGlobalInstances.hLanguage, IDS_SAVECHANGES, cTemplate, MAX_PATH);
			swprintf_s(cText, MAX_PATH, cTemplate, poDocument->GetTitle().c_str());
			
			if (bShowSaveChangesDialog) {
				oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
				iResult = oCore->oDialogs->DlgSaveChanges(phWnd, cText);
			}
			else
			{
				iResult = IDYES;
			}

			//iResult = MessageBox(phWnd, cText, L"Copying Machine", MB_YESNOCANCEL | MB_ICONQUESTION);

			if (iResult == IDYES || iResult == IDYES2) {
				
				eResult = poDocument->Save(phWnd);
				
				if (eResult != eOk) {

					oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
					oCore->oDialogs->DlgError(phWnd, eResult, 1, poDocument->GetFilename().c_str());
					
					iResult = IDCANCEL;
					}
				}

			if (iResult == IDCANCEL) {
			
				iResult = IDCANCEL;
				}
			}
		else
		{
			iResult = -1;
		}
	}

	oTrace->EndTrace(__WFUNCTION__, iResult);

	return iResult;
	}


//This function deletes the document (without saving) and removes it from the
//document vector
bool 
CCopyDocuments::CloseDocument(CCopyDocument* poDocument) {

	bool bResult;
	int iIndex;
	int iDocumentId;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;

	if (poDocument==NULL) bResult = false;

	if (bResult) {

		//Look for the given document in the documents vector
		bResult = false;
		iIndex = (int)oDocuments.size();

		while (!bResult && iIndex>0) {

			iIndex--;

			if (oDocuments[iIndex] == poDocument) {
				
				iDocumentId = oDocuments[iIndex]->GetDocumentID();
				
				//we got it, now remove and delete it
				oDocuments.erase(oDocuments.begin() + iIndex);

				delete poDocument;

				bResult = true;
				}
			}

		//Send a notification that a document has been changed
		oNotification->CommunicateEvent(eNotificationDocumentClosed, (void*)iDocumentId);
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function splits the given document. The pageid is the first page of the new document.
eActionResult 
CCopyDocuments::DoSplitDocument(CCopyDocument* poDocument, DWORD pdPageId, HWND phWnd) {

	eActionResult cResult;
	CCopyDocument* oNewDocument;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;

	if (poDocument!=NULL) {

		oNewDocument = CreateNewDocument();

		lIndex = poDocument->GetPageIndex(pdPageId);

		if (lIndex >=0) {
	
			while (lIndex < poDocument->GetPageCount()) {
				
				pdPageId = poDocument->GetPage(lIndex)->GetPageID();
				poDocument->DoMovePage(pdPageId, oNewDocument);
				}
			}

		cResult = eOk;

		if (oNewDocument->GetPageCount()==0) {

			//No pages added, something went wrong. Delete the created document.
			oDocuments.erase(oDocuments.begin() + (oDocuments.size()-10));

			delete oNewDocument;
			oNewDocument = NULL;
			
			cResult = eImageActionError;
			}

		if (cResult == eOk) {

			//Send a notification that a document has been changed
			oNotification->CommunicateEvent(eNotificationNewDocument, (void*)oNewDocument->GetDocumentID());
			}
		}

	oTrace->EndTrace(__WFUNCTION__, (cResult == eOk));

	return cResult;
	}



//This function loads a tiff file.
sLoadFileResult 
CCopyDocuments::LoadCompressedTiff(std::wstring psFilename) {

	Bitmap* oBitmap;
	long lImageWidth;
	long lImageHeight;
	unsigned short lTileWidth;
	float lResolutionX;
	float lResolutionY;
	unsigned short lPhotometricInterpretation;
	unsigned long lLineSize;
	unsigned short lBitsPerSample;
	long lRowsPerStrip;
	long lCurrentRow;
	long lSamplePerPixel;
	uint16 lCompression;
	long Align;
	long nrow;
	TIFF* TIFF_FileHandle;
	HGLOBAL hStrip;
	sLoadFileResult oResult;
	sLoadFilePageResult oPageResult;

	BYTE* buf;
	BYTE* lpBits;
	BYTE* pPallette;
	bool bContinueLoading;
	
	bContinueLoading = true;
	oResult.eResult = eOk;
	oResult.eFiletype = eTIFFUncompressed;
	
	//OpenFile
	TIFF_FileHandle = TIFFOpenW(psFilename.c_str(), "r");
	if (TIFF_FileHandle == NULL) {

		//terminating
        oResult.eResult = eFileResultNotFound;
		return oResult;
		}

	do {
      	
		//get the properties of the TIFF file
		TIFFGetField(TIFF_FileHandle, TIFFTAG_IMAGEWIDTH, &lImageWidth);
		TIFFGetField(TIFF_FileHandle, TIFFTAG_TILEWIDTH, &lTileWidth);
		TIFFGetField(TIFF_FileHandle, TIFFTAG_IMAGELENGTH, &lImageHeight);
		TIFFGetField(TIFF_FileHandle, TIFFTAG_PHOTOMETRIC, &lPhotometricInterpretation);
		TIFFGetField(TIFF_FileHandle, TIFFTAG_XRESOLUTION, &lResolutionX);
		TIFFGetField(TIFF_FileHandle, TIFFTAG_YRESOLUTION, &lResolutionY);
		TIFFGetField(TIFF_FileHandle, TIFFTAG_BITSPERSAMPLE, &lBitsPerSample);
		TIFFGetField(TIFF_FileHandle, TIFFTAG_ROWSPERSTRIP, &lRowsPerStrip);
		TIFFGetField(TIFF_FileHandle, TIFFTAG_COMPRESSION, &lCompression);

		lLineSize = TIFFScanlineSize(TIFF_FileHandle); //Number of byte in one line
		lSamplePerPixel = (lLineSize*8) / lImageWidth;

		DWORD lSizeImage = (DWORD)CBitmap::CalculateRowsize(lImageWidth, lSamplePerPixel) * lImageHeight;
		DWORD lColorsUsed = lSamplePerPixel > 8 ? 0 : (1 << lBitsPerSample);
		DWORD lMemorySize = sizeof(BITMAPINFOHEADER) + (int)(lColorsUsed * sizeof(RGBQUAD))+ lSizeImage;

		if (lCompression==COMPRESSION_PACKBITS || lCompression==COMPRESSION_JPEG) {

			oResult.eFiletype = eTIFFCompressed;
			}

		//Do the preparations to import the images
		hStrip = GlobalAlloc(GHND, TIFFStripSize(TIFF_FileHandle));
		buf = (BYTE*)GlobalLock(hStrip);

		if (!buf) {
			TIFFClose(TIFF_FileHandle);
            oResult.eResult = eGeneralOutOfMemory;
			return oResult;
			}

		//Alloc the memory space for this frame's image
		HGLOBAL DibHandle1 = ::GlobalAlloc(GMEM_MOVEABLE, lMemorySize);
	                                                                    
		LPBITMAPINFO DibHandle;
		DibHandle = (LPBITMAPINFO)::GlobalLock(DibHandle1);

		DibHandle->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		DibHandle->bmiHeader.biWidth = lImageWidth;
		DibHandle->bmiHeader.biHeight = lImageHeight;
		DibHandle->bmiHeader.biPlanes = 1;
		DibHandle->bmiHeader.biBitCount = (WORD)lSamplePerPixel;
		DibHandle->bmiHeader.biCompression = BI_RGB;
		DibHandle->bmiHeader.biSizeImage = 0;
		DibHandle->bmiHeader.biXPelsPerMeter = (LONG)((lResolutionX*100)/2.54); //pixels per meter
		DibHandle->bmiHeader.biYPelsPerMeter = (LONG)((lResolutionY*100)/2.54);
		DibHandle->bmiHeader.biClrUsed = lColorsUsed;
		DibHandle->bmiHeader.biClrImportant = 0;
		DibHandle->bmiHeader.biSizeImage = lSizeImage;

		//set the pointers
		pPallette = (BYTE*)DibHandle->bmiColors;
		lpBits = pPallette + (WORD)(DibHandle->bmiHeader.biClrUsed * 4);


		//Align = Number of byte to add at the end of each line of the DIB
		Align = 4 - (lLineSize % 4);
		if (Align == 4)	Align = 0;

		//pallette toevoegen als aanwezig
		int maxPallette;
		if (lPhotometricInterpretation == 0) {
			//white is zero
			maxPallette = 1 << lBitsPerSample;
			for (int i = 0; i < maxPallette; i++)
			{
			*pPallette=255-(BYTE)(255*i/(maxPallette-1));
			pPallette++;
			*pPallette=255-(BYTE)(255*i/(maxPallette-1));
			pPallette++;
			*pPallette=255-(BYTE)(255*i/(maxPallette-1));
			pPallette++;
			*pPallette=255-(BYTE)0;
			pPallette++;
			}
		}

		if (lPhotometricInterpretation == 1) {
		//black is zero
		maxPallette = 1 << lBitsPerSample;
		for (int i = 0; i < maxPallette; i++)
			{
			*pPallette=(BYTE)(255*i/(maxPallette-1));
			pPallette++;
			*pPallette=(BYTE)(255*i/(maxPallette-1));
			pPallette++;
			*pPallette=(BYTE)(255*i/(maxPallette-1));
			pPallette++;
			*pPallette=(BYTE)0;
			pPallette++;
			}
		}

      if (lPhotometricInterpretation == 3)
         {
         uint16* red;
         uint16* green;
         uint16* blue;
         int16 i;
         int   Palette16Bits;

         TIFFGetField(TIFF_FileHandle, TIFFTAG_COLORMAP, &red, &green, &blue);

         //Is the palette 16 or 8 bits ?
         if (checkcmap(1<<lBitsPerSample, red, green, blue) == 16)
         	Palette16Bits = TRUE;
         else
			 Palette16Bits = FALSE;

         //load the palette in the DIB
         for (i = 0; i < (1<<lBitsPerSample); i++) {
            if (Palette16Bits) {
              	*pPallette =(BYTE) CVT(red[i]);
               pPallette++;
               *pPallette = (BYTE) CVT(green[i]);
               pPallette++;
               *pPallette = (BYTE) CVT(blue[i]);
               pPallette++;
			   *pPallette=(BYTE)0;
				pPallette++;

               }
            else {
               *pPallette = (BYTE) red[i];
               pPallette++;
               *pPallette = (BYTE) green[i];
               pPallette++;
               *pPallette = (BYTE) blue[i];
               pPallette++;
				*pPallette=(BYTE)0;
				pPallette++;
               }
            }
         }

      //init afbeelding variabelen
      lCurrentRow = 0;
      lpBits+=(lLineSize+Align)*(lImageHeight-1);

      //Verwerk lijnen
      while (lCurrentRow < lImageHeight && bContinueLoading) {

         nrow = (lCurrentRow + lRowsPerStrip > lImageHeight ? lImageHeight - lCurrentRow : lRowsPerStrip);
         if (nrow==0) {nrow=1;}

         if (TIFFReadEncodedStrip(TIFF_FileHandle, TIFFComputeStrip(TIFF_FileHandle, lCurrentRow, 0), buf, nrow*lLineSize)==-1) {

			bContinueLoading = false;
			oResult.eResult = eFileResultUnknownError;
            }
         else {
            for (long l = 0; l < nrow; l++) {
               if (lSamplePerPixel  == 24) {
                  for (int i=0;i< (int) (lImageWidth);i++) {
                     lpBits[(i*lSamplePerPixel)/8+0]=buf[l*lLineSize+(i*lSamplePerPixel)/8+2];
                     lpBits[(i*lSamplePerPixel)/8+1]=buf[l*lLineSize+(i*lSamplePerPixel)/8+1];
                     lpBits[(i*lSamplePerPixel)/8+2]=buf[l*lLineSize+(i*lSamplePerPixel)/8+0];
                     }

                  lpBits-=(lLineSize+Align);
                  }
               else {
                  memcpy(lpBits, &buf[(int)(l*lLineSize)], lLineSize);

                  lpBits-=(WORD)(lLineSize+Align);
                  }
               }
            }

         lCurrentRow = lCurrentRow + lRowsPerStrip;
         }

		if (bContinueLoading) {	

			lpBits = (BYTE*)DibHandle->bmiColors + (WORD)(DibHandle->bmiHeader.biClrUsed * 4);
			oBitmap = new Bitmap((LPBITMAPINFO)DibHandle, (BYTE*)lpBits);
			oBitmap->SetResolution(lResolutionX, lResolutionY);

			//Add the page to the load result
			oPageResult.hMemoryBitmap = DibHandle1;
			oPageResult.oBitmap = oBitmap;

			oResult.oPages.push_back(oPageResult);
			}

		//free memory
		GlobalUnlock(DibHandle1);
		if (!bContinueLoading) GlobalFree(DibHandle1);
		GlobalUnlock(hStrip);
		GlobalFree(hStrip);

		} while (TIFFReadDirectory(TIFF_FileHandle) && bContinueLoading);

	//close
	TIFFClose(TIFF_FileHandle);

	//clear the bitmaps if an error occured
	if (!bContinueLoading) {

		for (int iIndex=0; iIndex<(int)oResult.oPages.size(); iIndex++) {

			delete oResult.oPages[iIndex].oBitmap;
			GlobalFree(oResult.oPages[iIndex].hMemoryBitmap);
			oResult.oPages.clear();
			}
		}

	//return the result
	return oResult;
	}
