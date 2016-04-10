#include "stdafx.h"
#include "PDFObject.h"
#include "PDF.h"
#include "Bitmap.h"
#include "Wincrypt.h"

//---------------------------------------------------------------------------
CPDF::CPDF() {

    CPDFObject* oPagesPDFObject;
    CPDFObject* oCatalogPDFObject;
    CPDFObject* oOutlinePDFObject;
    CPDFObject* oMetadataPDFObject;
	CPDFObject* oInfoPDFObject;
	CPDFObject* oOutputIntentPDFObject;

    lJPEGCompression = 25;

    lNextObjectNumber = 1;

    //add required objects (Catalog and Pages)
    oCatalogPDFObject = new CPDFObject(lNextObjectNumber, 0, eCatalog);
    oPDFObjects.push_back(oCatalogPDFObject);
    lNextObjectNumber++;

    oPagesPDFObject = new CPDFObject(lNextObjectNumber, 0, ePages);
    oPDFObjects.push_back(oPagesPDFObject);
    lNextObjectNumber++;

    oOutlinePDFObject = new CPDFObject(lNextObjectNumber, 0 , eOutline);
    oPDFObjects.push_back(oOutlinePDFObject);
    lNextObjectNumber++;

	oOutputIntentPDFObject = new CPDFObject(lNextObjectNumber, 0, eOutputIntent);

	HMODULE hModule = GetModuleHandle(NULL);

    HRSRC myResource = ::FindResource(NULL, L"ICC_RGB", RT_RCDATA);
    HGLOBAL myResourceData = ::LoadResource(NULL, myResource);


	//HGLOBAL hMem = LoadResource(hModule, hRes);
	LPVOID lpResource = LockResource(myResourceData);
	DWORD size = SizeofResource(hModule, myResource);

	oOutputIntentPDFObject->AddCompressedMemoryBlock(lpResource, size);
//	oOutputIntentPDFObject->AddMemoryBlock(lpResource, size);

	/*std::string ICCContent;
	std::string Number;
    char cNumber[20];
	byte* lpPointer = (byte*)lpResource;
	for (int i=0; i<size; i++)
	{
        sprintf_s(cNumber, sizeof(cNumber), "%02X ", lpPointer[i]);
		Number = cNumber;
		ICCContent += Number;
	}
	//ICCContent += ">";
	oOutputIntentPDFObject->AddContent(ICCContent);*/

	UnlockResource(lpResource);
	FreeResource(myResourceData);



	oPDFObjects.push_back(oOutputIntentPDFObject);
	lICCResourceId = lNextObjectNumber;
	lNextObjectNumber++;


	TIME_ZONE_INFORMATION cTimeZoneInformation;
    long lBiasMinutes;
    long lBiasHours;
	std::string sBias;
	
	//Get current date and time
                //Get date offset from UTC.
                GetTimeZoneInformation(&cTimeZoneInformation);

				SYSTEMTIME cSystemTime;

				GetSystemTime(&cSystemTime);
				char cDateTime[20];

				sprintf_s(cDateTime, sizeof(cDateTime), "%04d-%02d-%02dT%02d:%02d:%02d", //"yyyy-mm-dd-Thh:nn:ss"
							cSystemTime.wYear,
							cSystemTime.wMonth,
							cSystemTime.wDay,
							cSystemTime.wHour,
							cSystemTime.wMinute,
							cSystemTime.wSecond);

				std::string sDate = cDateTime;

                //place the bias from the UTC in the date string
                lBiasMinutes = cTimeZoneInformation.Bias % 60;
                lBiasHours =   cTimeZoneInformation.Bias / 60;

				char cBias[30];
				sprintf_s(cBias, sizeof(cBias), "%+03d:%02d", lBiasHours, lBiasMinutes);
                sDate += cBias;

	oMetadataPDFObject = new CPDFObject(lNextObjectNumber, 0 , eMetadata);
    oMetadataPDFObject->AddCreator("Meusesoft Copying Machine");
    oMetadataPDFObject->AddProducer("Meusesoft Copying Machine");
	oMetadataPDFObject->AddCreationDate(sDate);
	oPDFObjects.push_back(oMetadataPDFObject);
    lNextObjectNumber++;

					sprintf_s(cDateTime, sizeof(cDateTime), "%04d%02d%02d%02d%02d%02d", //"yyyymmddhhnnss"
							cSystemTime.wYear,
							cSystemTime.wMonth,
							cSystemTime.wDay,
							cSystemTime.wHour,
							cSystemTime.wMinute,
							cSystemTime.wSecond);

				sDate = cDateTime;

                //place the bias from the UTC in the date string
                lBiasMinutes = cTimeZoneInformation.Bias % 60;
                lBiasHours =   cTimeZoneInformation.Bias / 60;

				cBias[30];
				sprintf_s(cBias, sizeof(cBias), "%+03d'%02d'", lBiasHours, lBiasMinutes);
                sDate += cBias;
	
	oInfoPDFObject = new CPDFObject(lNextObjectNumber, 0 , eInfo);
    oInfoPDFObject->AddCreator("Meusesoft Copying Machine");
    oInfoPDFObject->AddProducer("Meusesoft Copying Machine");
	oInfoPDFObject->AddCreationDate(sDate);

    oPDFObjects.push_back(oInfoPDFObject);
    lNextObjectNumber++;

    //place the pages object in the child lists of the catalog
    oCatalogPDFObject->AddChild(oPagesPDFObject);
    oCatalogPDFObject->AddChild(oOutlinePDFObject);
    oCatalogPDFObject->AddChild(oMetadataPDFObject);
    oCatalogPDFObject->AddChild(oOutputIntentPDFObject);

    //hold the pagesobject in a global variable for further use
    oPagesObject = (void*)oPagesPDFObject;
    oRootObject = (void*)oCatalogPDFObject;
    oInfoObject = (void*)oInfoPDFObject;
    }
//---------------------------------------------------------------------------
CPDF::~CPDF() {

    while (oPDFObjects.size()>0) {

        delete oPDFObjects[0];
        oPDFObjects.erase(oPDFObjects.begin());
        }
   }
//---------------------------------------------------------------------------
long
CPDF::AddPage() {

    CPDFObject* oNewPDFObject;
    CPDFObject* oParent;
    long lObjectId;

    lObjectId = lNextObjectNumber;

    oParent = (CPDFObject*)oPagesObject;

    //create new instance of PDF object, this instance is
    //a page
    oNewPDFObject = new CPDFObject(lObjectId, oParent->lObjectId, ePage);
    oPDFObjects.push_back(oNewPDFObject);
    lNextObjectNumber++;

    //add the page to the pages object as a new page
    oParent->AddChild(oNewPDFObject);

    //return Id of added page
    return lObjectId;
    }
//---------------------------------------------------------------------------
bool 
CPDF::AddTextToPage(long plPageId, std::string psText, RectF pcTextBox) {

	//Add text to a page object
    CPDFObject* oPDFObject;
    CPDFObject* oFont;
    CPDFObject* oContents;
    CPDFObject* oResource;
    CPDFObject* oTempPDFObject;

    std::string sContent;
	bool bResult;

    oPDFObject = NULL;
	bResult = false;

    //get the object of the given page id.
    for (unsigned long lIndex=0; lIndex<oPDFObjects.size() && 
		oPDFObject==NULL; lIndex++) {

        oTempPDFObject = (CPDFObject*)oPDFObjects[lIndex];

        if (oTempPDFObject->lObjectId == plPageId) {

            oPDFObject = oTempPDFObject;
            }
        }

    //add the text to the page (if page is found)
    if (oPDFObject!=NULL) {

 		oResource = oPDFObject->GetChildObject(eText);

		if (oResource!=NULL) {
	        //create resource instance
			oResource = new CPDFObject(lNextObjectNumber, 0, eText);
			oPDFObjects.push_back(oResource);
			oPDFObject->AddChild(oResource);

			oPDFObject->lResourceId = oPDFObject->lObjectId;
			lNextObjectNumber++;
			}


		oFont = oPDFObject->GetChildObject(eFont);

		if (oFont==NULL) {
			//create font instance
			oFont = new CPDFObject(lNextObjectNumber, 0, eFont);
			oPDFObjects.push_back(oFont);
			oPDFObject->AddChild(oFont);
			oPDFObject->lFontId = oFont->lObjectId;

			lNextObjectNumber++; 
			}

		oContents = oPDFObject->GetChildObject(eContents);

		if (oContents==NULL) {
			//create contents instance
			oContents = new CPDFObject(lNextObjectNumber, 0, eContents);
			oPDFObjects.push_back(oContents);
			oPDFObject->AddChild(oContents);
			oPDFObject->lContentId = oContents->lObjectId;
			
			lNextObjectNumber++; 
			}

		//Set the textbox of the given rectangle
		RectF cTextbox;

		cTextbox.X		= /*oPDFObject->GetWidth() - */(pcTextBox.X * oPDFObject->GetContentScale() + (float)oPDFObject->GetContentBox().X);
		cTextbox.Y		= oPDFObject->GetHeight() - (pcTextBox.Y * oPDFObject->GetContentScale() + pcTextBox.Height * oPDFObject->GetContentScale() + oPDFObject->GetContentBox().Y);
		cTextbox.Width	= pcTextBox.Width * oPDFObject->GetContentScale();
		cTextbox.Height = pcTextBox.Height * oPDFObject->GetContentScale();

		bResult = oContents->AddText(psText, cTextbox);
        }

	return bResult;
}
//------------------------------------------------------------------
bool
CPDF::AddBitmapToPage(long plPageId, Bitmap* poBitmap) {

    CPDFObject* oPDFObject;
    CPDFObject* oContents;
    CPDFObject* oResource;
    CPDFObject* oTempPDFObject;

    std::string sContent;
    long lWidth;
    long lHeight;
    long lPageWidth;
    long lPageHeight;
    float fImage;
    float fPage;
	bool bResult;

    oPDFObject = NULL;
	bResult = false;

    //get the object of the given page id.
    for (unsigned long lIndex=0; lIndex<oPDFObjects.size() && oPDFObject==NULL; lIndex++) {

        oTempPDFObject = (CPDFObject*)oPDFObjects[lIndex];

        if (oTempPDFObject->lObjectId == plPageId) {

            oPDFObject = oTempPDFObject;
            }
        }

    //add the bitmap to the page (if page is found)
    if (oPDFObject!=NULL) {

        //create resource instace
        oResource = new CPDFObject(lNextObjectNumber, 0, eImage);
        oResource->lJPEGCompression = lJPEGCompression;
		oResource->lICCResourceId = lICCResourceId;
        oPDFObjects.push_back(oResource);
		oPDFObject->AddChild(oResource);

        bResult = oResource->AddBitmapStream(poBitmap);

        oPDFObject->lResourceId = lNextObjectNumber;
        lNextObjectNumber++;

		oContents = oPDFObject->GetChildObject(eContents);

		if (oContents==NULL) {
			//create contents instance
			oContents = new CPDFObject(lNextObjectNumber, 0, eContents);
			oPDFObjects.push_back(oContents);
			oPDFObject->AddChild(oContents);
			}

        lWidth = poBitmap->GetWidth();
        lHeight = poBitmap->GetHeight();

        lPageWidth = oPDFObject->GetWidth() - 20;
        lPageHeight = oPDFObject->GetHeight() - 20;

        fImage = (float)lWidth / (float)lHeight;
        fPage = (float)lPageWidth / (float)lPageHeight;

        if (fPage > fImage) {

            //page wider than image
			oPDFObject->SetContentScale((float)lPageHeight / (float)lHeight);
            lWidth = (long)(lPageHeight * fImage);
            lHeight = lPageHeight;
            }
        else {

            //page higher than image
			oPDFObject->SetContentScale((float)lPageWidth / (float)lWidth);
            lWidth = lPageWidth;
            lHeight = (long)(lPageWidth / fImage);
            }

        char cCommand[250];
		
		sContent = "q\r";

		sprintf_s(cCommand, sizeof(cCommand), "%d 0 0 %d %d %d cm\r\n", lWidth, lHeight, 10, 10 + (lPageHeight - lHeight));
		sContent += cCommand;

		sContent += "/Image1 Do\r\nQ\r\n";
        oContents->AddContent(sContent);

        oPDFObject->lContentId = oContents->lObjectId;
        lNextObjectNumber++; 

		//Set the content box of the page to the size of the image
		oPDFObject->SetContentBox(Rect(10, 10, lWidth, lHeight));
        }

	return bResult;
    }
//---------------------------------------------------------------------------
bool
CPDF::AddHBitmapToPage(long plPageId, HANDLE phBitmap) {

    bool bResult;
	Bitmap* oBitmap;

	bResult = false;

    //convert the handle to a VCL TBitmap instance
    oBitmap = ConvertDIBtoBitmap(phBitmap);

    if (oBitmap!=NULL) {

        //Add the bitmap instance to the page
        bResult = AddBitmapToPage(plPageId, oBitmap);

        //delete the bitmap instance
        delete oBitmap;
        }

	return bResult;
    }
//---------------------------------------------------------------------------
eActionResult
CPDF::WriteToFile(std::wstring psFilename) {

    char cNumber[10];
    __int64 lPositionCrossReference;
    CPDFObject* oPDFObject;
	eActionResult eResult;

	eResult = eOk;

    try {

        //create file
		eResult = eFileResultCouldNotCreate;
		oFile = new std::ofstream(psFilename.c_str(), ios::out | ios::binary);

        //write file header
		oFile->write("%PDF-1.4\r", 9); //first line, identifier
        oFile->write("%‚„œ”\n", 6); //to be sure it is identified as binary file

        //write objects
        for (unsigned long lIndex=0; lIndex<oPDFObjects.size(); lIndex++) {

            oPDFObject = (CPDFObject*)oPDFObjects[lIndex];

            oPDFObject->WriteToFile(oFile);
            }

        //write cross reference
        lPositionCrossReference = oFile->tellp();
        WriteCrossReferenceToFile();

        //write file trailer
        oFile->write("trailer\r<<\r/Size ", 17);
        sprintf_s(cNumber, sizeof(cNumber), "%d", lNextObjectNumber);
        oFile->write(cNumber, (streamsize)strlen(cNumber));

        //root node
        oFile->write("\r/Root ", 7);
        oPDFObject = (CPDFObject*)oRootObject;

        sprintf_s(cNumber, sizeof(cNumber), "%d", oPDFObject->lObjectId);
        oFile->write(cNumber, (streamsize)strlen(cNumber));

        oFile->write(" 0 R\r", 5);

		//ID
		std::wstring sID = CreateID(psFilename);
		

		int len;
		int slength = (int)sID.length() + 1;
		len = WideCharToMultiByte(CP_ACP, 0, sID.c_str(), slength, 0, 0, 0, 0); 
		std::string r(len, '\0');
		WideCharToMultiByte(CP_ACP, 0, sID.c_str(), slength, &r[0], len, 0, 0); 



		oFile->write("/ID[<",5);
		oFile->write(r.c_str(), r.length()-1);
		oFile->write("><",2);
		oFile->write(r.c_str(), r.length()-1);
		oFile->write(">]\r",3);
		
		//Info node
        oFile->write("/Info ", 6);
        oPDFObject = (CPDFObject*)oInfoObject;

        sprintf_s(cNumber, sizeof(cNumber), "%d", oPDFObject->lObjectId);
        oFile->write(cNumber, (streamsize)strlen(cNumber));

        oFile->write(" 0 R\r>>\r", 8);

        //cross reference
        oFile->write("startxref\r", 10); //pointer to crossreference section

        sprintf_s(cNumber, sizeof(cNumber), "%d", lPositionCrossReference);
        oFile->write(cNumber, (streamsize)strlen(cNumber));
        oFile->write("\r", 1);

        oFile->write("%%EOF", 5); //end of file

		eResult = eOk;

        //close file
        delete oFile;
        }
    catch (...) {

        }

	return eResult;
    }
//---------------------------------------------------------------------------
void
CPDF::WriteCrossReferenceToFile() {

    char cNumber[10];
    char cLine[30];
    std::string sLine;
    CPDFObject* oPDFobject;

    //this function write the cross reference table into the pdf file
    oFile->write("xref\r", 5);

    oFile->write("0 ", 2);
    sprintf_s(cNumber, sizeof(cNumber), "%d", oPDFObjects.size()+1);
    oFile->write(cNumber, (streamsize)strlen(cNumber));
    oFile->write("\r", 1);

    oFile->write("0000000000 65535 f\r\n", 20);

    for (unsigned long lIndex=0; lIndex<oPDFObjects.size(); lIndex++) {

        oPDFobject = (CPDFObject*)oPDFObjects[lIndex];

        sprintf_s(cLine, sizeof(cLine), "%010d 00000 n\r\n", oPDFobject->lFilePosition);

        oFile->write(cLine, (streamsize)strlen(cLine));
        }
    }
//---------------------------------------------------------------------------
Bitmap*
CPDF::ConvertDIBtoBitmap(HANDLE phDIB) {

	Bitmap* oBitmap;

	oBitmap = NULL;

	LPBITMAPINFO pBitmapInfo;
	void* pBitmapData;
	
	pBitmapInfo = (LPBITMAPINFO)GlobalLock(phDIB);
	pBitmapData = ((BYTE*)pBitmapInfo->bmiColors + pBitmapInfo->bmiHeader.biClrUsed*sizeof(RGBQUAD));

	oBitmap= new Bitmap(pBitmapInfo, pBitmapData);

	GlobalUnlock(phDIB);
		
	return oBitmap;
	}
//---------------------------------------------------------------------------
std::wstring
CPDF::CreateID(std::wstring psFilename)
{
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
	wchar_t cHashByte[16];
    TIME_ZONE_INFORMATION cTimeZoneInformation;

	std::wstring Result = L"";


	//Create ID string, containing time
	GetTimeZoneInformation(&cTimeZoneInformation);

	SYSTEMTIME cSystemTime;

	GetSystemTime(&cSystemTime);
	wchar_t cDateTime[20];

	swprintf_s(cDateTime, 20, L"%04d-%02d-%02dT%02d:%02d:%02d", //"yyyy-mm-dd-Thh:nn:ss"
				cSystemTime.wYear,
				cSystemTime.wMonth,
				cSystemTime.wDay,
				cSystemTime.wHour,
				cSystemTime.wMinute,
				cSystemTime.wSecond);

	std::wstring sID = cDateTime;
	sID += psFilename;




	//Create a MD5 hash
	if(CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL,0)) {

		if(CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash)) {

			//Hash the string
			bSuccess = (CryptHashData(hHash, (BYTE*)sID.c_str(), sID.length(), 0)==TRUE);

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


								for (long lIndex=0; lIndex<(long)dwHashLen; lIndex++) {
		
									swprintf_s(cHashByte, 16, L"%X", pbHash[lIndex]);
									Result += cHashByte;		
									}

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

	return Result;
}


