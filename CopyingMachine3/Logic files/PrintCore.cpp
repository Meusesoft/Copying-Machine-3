#include "StdAfx.h"
#include "CopyingMachineCore.h"

CPrintCore::CPrintCore(sGlobalInstances pInstances):
	CInterfaceBase(pInstances)
{
	//Set helper classes
	LoadOutputDevices();

	bEndThread = false;
	hPrintThread = NULL;
	iPrintRequestNumber = 0;
}

CPrintCore::~CPrintCore(void)
{
	oOutputDevices.clear();
	
	//Clean up the thread
	if (hPrintThread!=NULL) {
		
		SetEndThread();

		//Wait for the thread to finish.
		WaitForSingleObject(hPrintThread, INFINITE);
	
		CloseHandle(hPrintThread);

		DeleteCriticalSection(&PrintCriticalSection);
		DeleteCriticalSection(&EndThreadCriticalSection);
		}
}

//This function fills the outputdevices vector with all known output devices
bool 
CPrintCore::LoadOutputDevices() {

	PRINTER_INFO_4* oPrintDevicesEnum;
	DWORD dBytes;
	DWORD dDevicesCount;
	sOutputDevice oOutputDevice;
	sPrinterDevice oPrintDevice;
	wchar_t cItemText[128];

	oTrace->StartTrace(__WFUNCTION__);

	oOutputDevices.clear();

	//Add File
	LoadStringW(oGlobalInstances.hLanguage, IDS_FILE, cItemText, 128);
	oOutputDevice.cType = eOutputFile;
	oOutputDevice.sDescription = cItemText;
	oOutputDevices.push_back(oOutputDevice);

	//Add E-mail
	LoadStringW(oGlobalInstances.hLanguage, IDS_EMAIL, cItemText, 128);
	oOutputDevice.cType = eOutputMail;
	oOutputDevice.sDescription = cItemText;
	oOutputDevices.push_back(oOutputDevice);

	//Add PDF
	oOutputDevice.cType = eOutputPDF;
	oOutputDevice.sDescription = L"Adobe PDF";
	oOutputDevices.push_back(oOutputDevice);

	//Add Printers
	EnumPrinters(
			PRINTER_ENUM_NAME,			// printer object types
			NULL,						// name of printer object
			4,							// information level
			0,							// printer information buffer
			0,							// size of printer information buffer
			&dBytes,					// bytes received or required
			&dDevicesCount				// number of printers enumerated
			);

	oPrintDevicesEnum = (PRINTER_INFO_4*)malloc(dBytes);

	EnumPrinters(
			PRINTER_ENUM_NAME,			// printer object types
			NULL,						// name of printer object
			4,							// information level
			(LPBYTE)oPrintDevicesEnum,	// printer information buffer
			dBytes,							// size of printer information buffer
			&dBytes,					// bytes received or required
			&dDevicesCount				// number of printers enumerated
			);

	for (long lIndex=0; lIndex<(long)dDevicesCount; lIndex++) {

		oOutputDevice.cType = eOutputPrinter;
		oOutputDevice.oPrinterDevice.sName = oPrintDevicesEnum[lIndex].pPrinterName;
		oOutputDevice.sDescription = oPrintDevicesEnum[lIndex].pPrinterName;

		oOutputDevices.push_back(oOutputDevice);
		}

	//free memory
	free (oPrintDevicesEnum);

	oTrace->EndTrace(__WFUNCTION__);

	return true;
}

//This function returns the requested outputdevice. It returns false if the requested
//index is invalid
bool 
CPrintCore::GetOutputDevice(int piIndex, sOutputDevice &oOutputDevice) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	if (piIndex >= 0 && piIndex < (int)oOutputDevices.size()) {

		oOutputDevice = oOutputDevices[piIndex];
		bResult = true;
		}
	
	oTrace->EndTrace(__WFUNCTION__);

	return bResult;
	}

//This function returns the number of detected outputdevices
int  
CPrintCore::GetOutputDeviceCount() {

	return (int)oOutputDevices.size();
	}

//This function finds the an OutputDevice based on the give attributes
bool 
CPrintCore::FindOutputDevice(eOutputType pcType, std::wstring psDeviceName, 
							 sOutputDevice &oOutputDevice) {
	
	bool bResult;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__);

	oTrace->Add(L"DeviceName", psDeviceName);

	bResult = false;
	lIndex = GetOutputDeviceCount()-1;

	while (!bResult && lIndex>=0) {

		oTrace->Add(L"Device", oOutputDevices[lIndex].oPrinterDevice.sName);

		if (oOutputDevices[lIndex].cType == pcType) {
	
			bResult = true;

			if (pcType==eOutputPrinter) {

				bResult = (psDeviceName == oOutputDevices[lIndex].oPrinterDevice.sName);
				}

			//we found a match, return it.
			if (bResult) {

				oOutputDevice = oOutputDevices[lIndex];
				}
			}

		lIndex--;
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function does the printing of a document. It is the base
//function for the complete process; from showing the printing dialog 
//to the actual printing of the document.
eActionResult 
CPrintCore::PrintDocument(CCopyDocument* poDocument, 
						  CPrintSettings* poPrintSettings) {

	eActionResult cResult;
	CCopyingMachineCore* oCmCore;
	Graphics*			 oCanvas;
	
	oTrace->StartTrace(__WFUNCTION__);

	cResult = eOk;
	oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oCanvas = NULL;
	
	//Initialise oPrintData 
	sOutputDevice oOutputDevice;
	poPrintSettings->GetDevice(oOutputDevice);

	poPrintSettings->oPrintData.sDevice = oOutputDevice.oPrinterDevice.sName;
	poPrintSettings->oPrintData.hDC = NULL;
	poPrintSettings->oPrintData.bCollate = true;
	poPrintSettings->oPrintData.iCopies = poPrintSettings->GetInt(ePrintCopies);
	if (poPrintSettings->oPrintData.bInitializePageRange) {
		for (long lIndex=0; lIndex<poDocument->GetPageCount(); lIndex++) {
			poPrintSettings->oPrintData.lPage.push_back(lIndex);
			}
		}

	//Show print dialog for last changes to the settings
	if (poPrintSettings->GetBool(ePrintShowInterface) || poPrintSettings->GetBool(ePrintAlwaysShowInterface)) {

		if (!oCmCore->oDialogs->DlgPrint(poPrintSettings->hWindow, poPrintSettings)) {

			cResult = eCancelled;
			}
		else {
			
			//the user selected a printer
			oOutputDevice.sDescription = poPrintSettings->oPrintData.sDevice;
			oOutputDevice.cType = eOutputPrinter;
			}

		oTrace->Add(L"DlgPrint", (int)cResult);
		}

	if (cResult==eOk) {

		//Start printing
		HDC hdcPrint;
		hdcPrint = NULL;

		oTrace->Add(L"oOutputDevice", oOutputDevice.sDescription);
		oTrace->Add(L"oOutputDeviceType", (int)oOutputDevice.cType);

		switch (oOutputDevice.cType) {

			case eOutputBitmap:

				oCanvas = new Graphics(oOutputDevice.oBitmap->GetBitmap());
				
				oTrace->Add(L"eOutputBitmap", (bool)(oCanvas!=NULL));
				break;

			case eOutputPrinter:

				hdcPrint = poPrintSettings->oPrintData.hDC;
				if (hdcPrint == NULL) {
					// Get a device context for the printer.
					hdcPrint = GetDCFromOutputDevice(oOutputDevice);
					oTrace->Add(L"GetContextDevice", (bool)(hdcPrint!=NULL));
					}
				if (hdcPrint!=NULL) oCanvas = new Graphics(hdcPrint);
				
				oTrace->Add(L"eOutputPrinter", (bool)(oCanvas!=NULL));
				break;
			}

		if (oCanvas!=NULL) {

			if (hdcPrint!=NULL) TracePrinterProperties(hdcPrint);
			poPrintSettings->Trace();
			
			//Process document
			cResult = ProcessDocument(oCanvas, poDocument, poPrintSettings);

			delete oCanvas;
			DeleteDC(hdcPrint);
			}
		else {

			//Retrieve the latest error
			cResult = ePrintError;		
			}
		}

	//If an error occurred process it
	if (cResult!=eOk && cResult!=eCancelled) {

		cErrorMessage oErrorMessage;
		wchar_t		  sErrorMessage[MAX_PATH+2];

		if (GetErrorMessageCount()>0) {
			
			oErrorMessage = GetLastErrorMessage(true);
			
			LoadString(oGlobalInstances.hLanguage, oErrorMessage.iMessageID, sErrorMessage, MAX_PATH);
			};

		oCmCore->oDialogs->DlgError(poPrintSettings->hWindow, cResult, 2, sErrorMessage, oErrorMessage.iStatus);
		}

	//Autodelete the print settings when requested
	if (poPrintSettings->bDeleteAfterPrint) delete poPrintSettings;

	oTrace->EndTrace(__WFUNCTION__, cResult);

	return cResult;
	}

//This function prints the document to the outputdevice
eActionResult 
CPrintCore::ProcessDocument(Graphics* poCanvas, CCopyDocument* poDocument, 
							CPrintSettings* poPrintSettings) {

	eActionResult cResult;
	std::wstring sDocName;
	DOCINFO docInfo;
	HDC		hdcPrint;
	long	lPage;
	long	lTotalPages;
	
	oTrace->StartTrace(__WFUNCTION__);

	cResult = eOk;
	hdcPrint = NULL;

	//Set the document info for printing
	ZeroMemory(&docInfo, sizeof(docInfo));
	docInfo.cbSize = sizeof(docInfo);
	sDocName = poDocument->GetTitle().c_str();
	docInfo.lpszDocName = sDocName.c_str();
	poPrintSettings->oPrintData.bInPage = false;

	//Start a document, only if it is a valid HDC
	hdcPrint = poCanvas->GetHDC();
	if (hdcPrint!=NULL) {
		StartDoc(hdcPrint, &docInfo);
		poCanvas->ReleaseHDC(hdcPrint);
		}
	
	if (poPrintSettings->hProgressBar!=NULL) {

		SendMessage(poPrintSettings->hProgressBar, PBM_SETPOS, 0, 0);
		}

	//Set the number of copies (collated or non collated)
	long lNumberCopiesCollated = 1;
	long lNumberCopiesNonCollated = 1;
	long lPrintPageIndex;
	if (poPrintSettings->oPrintData.bCollate) {
		lNumberCopiesCollated = poPrintSettings->oPrintData.iCopies;
		}
	else {
		lNumberCopiesNonCollated = poPrintSettings->oPrintData.iCopies;
		}	
	oTrace->Add(L"lNumberCopiesCollated", lNumberCopiesCollated);
	oTrace->Add(L"lNumberCopiesNonCollated", lNumberCopiesNonCollated);
	
	lPage = 0;
	lTotalPages = (long)poPrintSettings->oPrintData.lPage.size() * lNumberCopiesCollated * lNumberCopiesNonCollated;

	if (poPrintSettings->GetBool(ePrintMultiPagesOnPage) && lNumberCopiesNonCollated>1) {
		//Make sure the number of pages is a multiply of 4. This is necessary
		//for the NonCollated copies with MultiplePagesOnPage setting enabled.
		long lNumberDummyPagesToAdd = 4 - (long)poPrintSettings->oPrintData.lPage.size()%4;

		for (long lIndex=0; lIndex<lNumberDummyPagesToAdd; lIndex++) {

			poPrintSettings->oPrintData.lPage.push_back(-1);
			}
		}

	lPage = 0;
	lTotalPages = (long)poPrintSettings->oPrintData.lPage.size() * lNumberCopiesCollated * lNumberCopiesNonCollated;

	//Loop through the collated copies
	for (long lCopyCollated=0; lCopyCollated < lNumberCopiesCollated && !GetEndThread() && cResult==eOk; lCopyCollated++) {
	
		//loop through the pages of the document
		for (long lIndex=0; lIndex<(long)poPrintSettings->oPrintData.lPage.size() && cResult==eOk  && !GetEndThread(); lIndex++) {
			
			//Loop through non collated copies
			for (long lCopyNonCollated=0; lCopyNonCollated < lNumberCopiesNonCollated && !GetEndThread() && cResult==eOk; lCopyNonCollated++) {
			
				oTrace->Add(L"lIndex", lIndex);
		
				lPrintPageIndex = lIndex;

				if (poPrintSettings->GetBool(ePrintMultiPagesOnPage) && lNumberCopiesNonCollated>1) {
					
					//MultiPagesOnPage and CopyNonCollated enabled. Transform the pageindex so
					//that the physical pages will contain the same info. 
					//(A B C D - A B C D - E F - E F instead of A A B B - C C D D - E E F - F)
					
					lPrintPageIndex = ((lIndex * lNumberCopiesNonCollated) + lCopyNonCollated); //number of pages printed
					long lPagesPrinted	= lPrintPageIndex / 4;
					long lGroup			= lPagesPrinted / lNumberCopiesNonCollated; 
					long lPageInGroup	= lPrintPageIndex % 4;

					lPrintPageIndex		= lGroup * 4 + lPageInGroup;
					}

				//Print a page
				cResult = ProcessPage(poCanvas, poDocument, lPrintPageIndex, poPrintSettings);
				lPage++;

				//Update the progress bar
				if (poPrintSettings->hProgressBar!=NULL) {

					SendMessage(poPrintSettings->hProgressBar, PBM_SETPOS, lPage>lTotalPages ? 100 : (lPage * 100) / lTotalPages, 0);
					}

				//Update the status text
				if (poPrintSettings->hStatusWindow!=NULL) {

					wchar_t cStatusTemplateText[MAX_PATH+2];
					wchar_t cStatusText[MAX_PATH+2];

					LoadString(oGlobalInstances.hLanguage, IDS_PRINTSTATUSSTRING, cStatusTemplateText, MAX_PATH);

					wsprintf(cStatusText, cStatusTemplateText, 
						lPage>lTotalPages ? lTotalPages : lPage,											//Printed page
						lTotalPages,									//Number of pages to be printed
						poPrintSettings->oPrintData.sDevice.c_str());	//Printing device

					SetWindowText(poPrintSettings->hStatusWindow, cStatusText);
					}
				}
			}
		}

	//Close up
	hdcPrint = poCanvas->GetHDC();
	if (hdcPrint!=NULL) {

		if (poPrintSettings->oPrintData.bInPage) EndPage(hdcPrint);
		EndDoc(hdcPrint);

		poCanvas->ReleaseHDC(hdcPrint);
		}


	//Clear progress bar and status text
	if (poPrintSettings->hProgressBar!=NULL) {
		SendMessage(poPrintSettings->hProgressBar, PBM_SETPOS, 0, 0);
		}
	if (poPrintSettings->hStatusWindow!=NULL) {
		SetWindowText(poPrintSettings->hStatusWindow, L"");
		}

	oTrace->EndTrace(__WFUNCTION__, cResult);

	return cResult;
	}

//This function fills the page on the outputdevice with images from
//the document
eActionResult 
CPrintCore::ProcessPage(Graphics* poCanvas, CCopyDocument* poDocument, long plIndex, 
						CPrintSettings* poPrintSettings) {

	RectF rPageImageArea;
	float fMagnificationAdjustment;
	CCopyDocumentPage *oPage;
	HDC	  hdcPrint;

	eActionResult cResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	cResult = eOk;
	hdcPrint = NULL;
	//Calculate the area on which the image will be placed
	rPageImageArea = CalculatePageImageArea(poCanvas, plIndex, fMagnificationAdjustment, poPrintSettings);

	//Start page
	if (plIndex%4==0 || !poPrintSettings->GetBool(ePrintMultiPagesOnPage)) {
		
		hdcPrint = poCanvas->GetHDC();
		if (hdcPrint!=NULL) {
			StartPage(hdcPrint); //only send a startpage when a physical page is started
			poCanvas->ReleaseHDC(hdcPrint);
			}
		
		poPrintSettings->oPrintData.bInPage = true;
		}

	//Print the image
	oPage = poDocument->GetPage(poPrintSettings->oPrintData.lPage[plIndex]);

	if (oPage) cResult = ProcessImage(poCanvas, oPage, rPageImageArea, fMagnificationAdjustment, poPrintSettings);

	//End page
	if (plIndex%4==3 || !poPrintSettings->GetBool(ePrintMultiPagesOnPage)
		|| plIndex==(long)poPrintSettings->oPrintData.lPage.size()-1) {
		
		hdcPrint = poCanvas->GetHDC();
		if (hdcPrint!=NULL) {
			EndPage(hdcPrint); //only send a startpage when a physical page is started
			poCanvas->ReleaseHDC(hdcPrint);
			}
		
		poPrintSettings->oPrintData.bInPage = false;
		}

	oTrace->EndTrace(__WFUNCTION__, cResult);

	return cResult;
	}							
		
//This function process an image and draws it on the page
eActionResult 
CPrintCore::ProcessImage(Graphics* poPageCanvas, CCopyDocumentPage* poPage, 
						RectF prPageImageArea, float pfMagnificationAdjustment, CPrintSettings* poPrintSettings, 
						bool pbIgnoreMultipleCopiesOnPage) {

	eActionResult		cResult;
	ePrintAlignmentType cPrintAlignment; //Alignment of the image on the page

	float fMagnification;		//the level of magnification

	RectF rImageArea;			//the source rectangle
	RectF rImagePrintArea;		//the destination rectangle, magnified and positioned
	RectF rPageArea;			//the printable area
	RectF rPageAreaPortion;		//A portion of the printable area

	Bitmap* oPageImage;
	
	oTrace->StartTrace(__WFUNCTION__);
	oTrace->Add(L"pbIgnoreMultipleCopiesOnPage", pbIgnoreMultipleCopiesOnPage);
	oTrace->Add(L"ResolutionX", poPage->GetResolutionX());
	oTrace->Add(L"ResolutionY", poPage->GetResolutionY());

	cResult = eOk;

	//Initialise variables
	oPageImage = poPage->GetImage()->GetBitmap();
	oTrace->Add(L"ImageWidth", (int)oPageImage->GetWidth());
	oTrace->Add(L"ImageHeight", (int)oPageImage->GetHeight());

	fMagnification = pfMagnificationAdjustment * (float)poPrintSettings->GetInt(ePrintMagnification) / 100.0f;

	rPageArea = prPageImageArea;
	rImageArea = RectF(0, 0, (REAL)oPageImage->GetWidth() / poPage->GetResolutionX(), (REAL)oPageImage->GetHeight() / poPage->GetResolutionY());
	rImagePrintArea = RectF(rPageArea.X, rPageArea.Y, rImageArea.Width * fMagnification, rImageArea.Height * fMagnification);

	//Are we trying to fit as many copies as possible on one page?
	if (!pbIgnoreMultipleCopiesOnPage && poPrintSettings->GetBool(ePrintMultipleCopiesOnPage)) {

		//Yes we are, print as much as possible
		
		long lNumberCopiesX;
		long lNumberCopiesY;

		lNumberCopiesX	= max(1, (long)(rPageArea.Width / rImagePrintArea.Width));
		lNumberCopiesY	= max(1, (long)(rPageArea.Height / rImagePrintArea.Height));
		
		rPageAreaPortion = RectF(0, 0, rPageArea.Width / lNumberCopiesX, rPageArea.Height / lNumberCopiesY);
		
		for (long lXIndex = 0; lXIndex<lNumberCopiesX && cResult==eOk; lXIndex++) {

			for (long lYIndex = 0; lYIndex<lNumberCopiesY && cResult==eOk; lYIndex++) {

				oTrace->Add(L"CopyOnPage", (lXIndex * lNumberCopiesY) + lYIndex);
				
				rPageAreaPortion.X = rPageArea.X + (float)lXIndex * rPageAreaPortion.Width;
				rPageAreaPortion.Y = rPageArea.Y + (float)lYIndex * rPageAreaPortion.Height;
				
				cResult = ProcessImage(poPageCanvas, poPage, rPageAreaPortion, pfMagnificationAdjustment, poPrintSettings, true);
				}
			}
		}
	else {

		//No, Print one image on the give page area.
		
		//Apply stretching/shrinking. Does the image fit into the page?
		if (rPageArea.Width >= rImagePrintArea.Width && 
			rPageArea.Height >= rImagePrintArea.Height) {

			//The image could fit into the page area entirely. 
			//Are we going to stretch it?
			if (poPrintSettings->GetBool(ePrintStretch)) {

				fMagnification = max(rPageArea.Width / rImagePrintArea.Width,
									rPageArea.Height / rImagePrintArea.Height);

				rImagePrintArea = RectF(rPageArea.X, rPageArea.Y, rImagePrintArea.Width * fMagnification, rImagePrintArea.Height * fMagnification);
				}
			}
		else {

			//The image couldn't fit onto the page entirely. Are we going to shrink it?
			if (poPrintSettings->GetBool(ePrintShrink)) {

				fMagnification = min(rPageArea.Width / rImagePrintArea.Width,
									rPageArea.Height / rImagePrintArea.Height);

				rImagePrintArea = RectF(rPageArea.X, rPageArea.Y, rImagePrintArea.Width * fMagnification, rImagePrintArea.Height * fMagnification);
				}
			}

		//Apply alignment.
		cPrintAlignment = (ePrintAlignmentType)poPrintSettings->GetInt(ePrintAlignment);

		switch (cPrintAlignment) {

			//west
			case eAlignNorthWest:
			case eAlignCenterWest:
			case eAlignSouthWest:
				
				//do nothing, by default image aligned to the left/west
				break;

			//center
			case eAlignNorthCenter:
			case eAlignCenterCenter:
			case eAlignSouthCenter:

				rImagePrintArea.Offset((rPageArea.Width - rImagePrintArea.Width) / 2, 0);
				break;


			//east
			case eAlignNorthEast:
			case eAlignCenterEast:
			case eAlignSouthEast:

				rImagePrintArea.Offset(rPageArea.Width - rImagePrintArea.Width, 0);
				break;
			}

		switch (cPrintAlignment) {

			//north
			case eAlignNorthWest:
			case eAlignNorthCenter:
			case eAlignNorthEast:
				
				//do nothing, by default image aligned to the left/west
				break;

			//center
			case eAlignCenterWest:
			case eAlignCenterCenter:
			case eAlignCenterEast:

				rImagePrintArea.Offset(0, (rPageArea.Height - rImagePrintArea.Height) / 2);
				break;


			//south
			case eAlignSouthWest:
			case eAlignSouthCenter:
			case eAlignSouthEast:

				rImagePrintArea.Offset(0, rPageArea.Height - rImagePrintArea.Height);
				break;
			}


		//Draw the image onto the page
		//int iDCState;
		Status cStatus;

		//iDCState = SaveDC(phdcPrint);

		//Graphics* oPageCanvas = new Graphics(phdcPrint);
		GraphicsState  iState;
		iState = poPageCanvas->Save();

		//Convert Area rectangles to pixel format
		rImageArea = RectF(rImageArea.X * poPage->GetResolutionX(), rImageArea.Y * poPage->GetResolutionY(),
						   rImageArea.Width * poPage->GetResolutionX(), rImageArea.Height * poPage->GetResolutionY());

		poPageCanvas->SetPageUnit(UnitInch); //Set the unit of the canvas
		poPageCanvas->SetClip(rPageArea);	//Set the clipping rectangle
		poPageCanvas->SetInterpolationMode(InterpolationModeHighQualityBicubic); //Interpolation mode set to the highest quality

		cStatus = poPageCanvas->DrawImage(oPageImage, (RectF)rImagePrintArea, 
						rImageArea.X, rImageArea.Y, rImageArea.Width, rImageArea.Height, 
						(Unit)UnitPixel);

		if (cStatus!=Ok) {
			cResult=ePrintErrorCheckMessages;
			AddErrorMessage(IDS_ERRORUNKNOWN, (int)cStatus, false);
			oTrace->Add(L"Status DrawImage", (int)cStatus);
			}

		//delete oPageCanvas;

		//RestoreDC(phdcPrint, iDCState);

		//Check on registration and hash function
		CCopyingMachineCore* oCmCore;
		oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

		sExecutableInformation cExecutableInformation;
		oCmCore->GetCopyingMachineVersion(cExecutableInformation);

		if (!oCmCore->oRegistration->bRegistered/* ||
			wcscmp(cExecutableInformation.sHashValue.c_str(), oCmCore->oRegistration->sHash.c_str())!=0*/) {
			
			Gdiplus::Font*	oFont;
			PointF	cPoint;
			StringFormat* oStringFormat;
			wchar_t cUnregistered[50];

			cPoint.X = rImageArea.X;
			cPoint.Y = rImageArea.Y;
			LoadString(oGlobalInstances.hLanguage, IDS_UNREGISTERED, cUnregistered, 50);

			oStringFormat = new StringFormat();
			oStringFormat->SetAlignment(StringAlignmentCenter);
			oStringFormat->SetLineAlignment(StringAlignmentCenter);
			oFont = new Gdiplus::Font(L"Arial", 80 * pfMagnificationAdjustment, FontStyleRegular, UnitPoint, NULL);
			SolidBrush oBrush(Color(128, 0, 0, 0));		
			cStatus = poPageCanvas->DrawString(cUnregistered, -1, oFont, rPageArea, oStringFormat, &oBrush);

			delete oFont;
			delete oStringFormat;
			}

		poPageCanvas->Restore(iState);
		}

	oTrace->EndTrace(__WFUNCTION__, cResult);

	return cResult;
	}

//This function saves the properties of the printer into the
//trace file
void 
CPrintCore::TracePrinterProperties(HDC phdcPrint)
{
	int iValue;

	oTrace->StartTrace(__WFUNCTION__);
	
	oTrace->Add(L"PHYSICALWIDTH",	GetDeviceCaps(phdcPrint, PHYSICALWIDTH));
	oTrace->Add(L"PHYSICALHEIGHT",	GetDeviceCaps(phdcPrint, PHYSICALHEIGHT));
	oTrace->Add(L"PHYSICALOFFSETX",	GetDeviceCaps(phdcPrint, PHYSICALOFFSETX));
	oTrace->Add(L"PHYSICALOFFSETY",	GetDeviceCaps(phdcPrint, PHYSICALOFFSETY));
	oTrace->Add(L"LOGPIXELSX",		GetDeviceCaps(phdcPrint, LOGPIXELSX));
	oTrace->Add(L"LOGPIXELSY",		GetDeviceCaps(phdcPrint, LOGPIXELSY));
	oTrace->Add(L"HORZSIZE",		GetDeviceCaps(phdcPrint, HORZSIZE));
	oTrace->Add(L"VERTSIZE",		GetDeviceCaps(phdcPrint, VERTSIZE));
	oTrace->Add(L"BITSPIXEL",		GetDeviceCaps(phdcPrint, BITSPIXEL));
	oTrace->Add(L"PLANES",			GetDeviceCaps(phdcPrint, PLANES));
	oTrace->Add(L"NUMCOLORS",		GetDeviceCaps(phdcPrint, NUMCOLORS));
	oTrace->Add(L"SIZEPALETTE",		GetDeviceCaps(phdcPrint, SIZEPALETTE));

	iValue = GetDeviceCaps(phdcPrint, RASTERCAPS);

	oTrace->Add(L"RC_BANDING",		(iValue&RC_BANDING)!=0);
	oTrace->Add(L"RC_BITMAP64",		(iValue&RC_BITMAP64)!=0);
	oTrace->Add(L"RC_DI_BITMAP",		(iValue&RC_DI_BITMAP)!=0);
	oTrace->Add(L"RC_DIBTODEV",		(iValue&RC_DIBTODEV)!=0);
	oTrace->Add(L"RC_STRETCHDIB",	(iValue&RC_STRETCHDIB)!=0);
	oTrace->Add(L"RC_STRETCHBLT",	(iValue&RC_STRETCHBLT)!=0);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function gets a DC from the output device
HDC  
CPrintCore::GetDCFromOutputDevice(sOutputDevice poOutputDevice) {

	HDC hDC;

	hDC = CreateDC(NULL, poOutputDevice.oPrinterDevice.sName.c_str(), NULL, NULL);

	if (hDC==NULL) oTrace->Add(L"GetDCFromOutputDeviceError", (int)GetLastError());

	return hDC;
	}



//This function calculates the are on the page onto an image can be drawn
RectF 
CPrintCore::GetStandardPageImageArea(sOutputDevice oOutputDevice, CPrintSettings* poPrintSettings) {

	Graphics*		oDummy;
	RectF			cResult;	
	HDC				hdcPrint;
	float			fMagnification;

	hdcPrint = GetDCFromOutputDevice(oOutputDevice);
	
	if (hdcPrint!=NULL) {
	
		oDummy = new Graphics(hdcPrint);

		fMagnification = 1.0f;
		cResult = CalculatePageImageArea(oDummy, 0, fMagnification, poPrintSettings);

		delete oDummy;
		}
	else {

		//no DC, return a standard A4

		cResult = RectF(0, 0, 8.3f, 11.7f);
		}

	return cResult;
	}


//This function calculates the area on the page onto the image can
//be drawn. Result is a rectangle. The rectangle its unit is INCH
RectF 
CPrintCore::CalculatePageImageArea(Graphics* poCanvas, long plIndex, float &pfMagnificationAdjustment, 
								   CPrintSettings* poPrintSettings) {

	float fLeftMargin, fTopMargin;
	float fSmallPageWidth, fSmallPageHeight;
	float fPageMargin;
	RectF rPageArea;
	RectF rPagePrintableArea;
	RectF rPageImageArea;
	HDC	  hdcPrint;
	sOutputDevice oOutputDevice;

	pfMagnificationAdjustment = 1.0f;

	poPrintSettings->GetDevice(oOutputDevice);

	switch (oOutputDevice.cType) {

		case eOutputPrinter:

			hdcPrint = poCanvas->GetHDC();

			//Calculate the printable area. The result is a rect in inches. The margins of the output device are taken
			//into account.
			float fDpiX;
			float fDpiY;

			fDpiX			= (float)GetDeviceCaps(hdcPrint, LOGPIXELSX);
			fDpiY			= (float)GetDeviceCaps(hdcPrint, LOGPIXELSY);
			rPageArea		= RectF(0, 0, (REAL)GetDeviceCaps(hdcPrint, PHYSICALWIDTH), (REAL)GetDeviceCaps(hdcPrint, PHYSICALHEIGHT));
			
			fLeftMargin		= (float)GetDeviceCaps(hdcPrint, PHYSICALOFFSETX);
			fTopMargin		= (float)GetDeviceCaps(hdcPrint, PHYSICALOFFSETY);

			
			rPagePrintableArea = RectF(fLeftMargin / fDpiX, fTopMargin / fDpiY,
									   (REAL)GetDeviceCaps(hdcPrint, HORZRES) / fDpiX, 
									   (REAL)GetDeviceCaps(hdcPrint, VERTRES) / fDpiY);

			//Set the area on the page on which the image will be placed
			rPageImageArea = rPagePrintableArea;
		
			poCanvas->ReleaseHDC(hdcPrint);
			break;

		case eOutputBitmap:

			Bitmap*	  oPageBitmap;

			oPageBitmap = oOutputDevice.oBitmap->GetBitmap();

			fDpiX = oPageBitmap->GetHorizontalResolution();
			fDpiY = oPageBitmap->GetVerticalResolution();

			rPageImageArea.Width = (REAL)oPageBitmap->GetWidth();
			rPageImageArea.Height = (REAL)oPageBitmap->GetHeight();
			
			rPageImageArea.Width = rPageImageArea.Width / fDpiX;
			rPageImageArea.Height = rPageImageArea.Height / fDpiY;
			break;
		}

	//Calculate page size, start position, and small page size.
	rPagePrintableArea = rPageImageArea;
	fPageMargin = 0.2f;
	fSmallPageWidth = (rPagePrintableArea.Width - fPageMargin) / 2 ;
	fSmallPageHeight = (rPagePrintableArea.Height - fPageMargin) / 2;

	//Position image on multipage page
	if (poPrintSettings->GetBool(ePrintMultiPagesOnPage)) {

		//Determine the adjustment to the magnification
		pfMagnificationAdjustment = min(fSmallPageWidth / rPagePrintableArea.Width, 
										fSmallPageHeight / rPagePrintableArea.Height);
					
		//Adjust the printable area
		switch (plIndex % 4) {

			case 0: // top left

				rPageImageArea = RectF(rPagePrintableArea.X, rPagePrintableArea.Y,
									   fSmallPageWidth, fSmallPageHeight);
				break;

			case 1: // top right

				rPageImageArea = RectF(rPagePrintableArea.X + rPagePrintableArea.Width - fSmallPageWidth, rPagePrintableArea.Y,
									   fSmallPageWidth, fSmallPageHeight);
				break;

			case 2: // bottom left

				rPageImageArea = RectF(rPagePrintableArea.X, rPagePrintableArea.Y + rPagePrintableArea.Height - fSmallPageHeight,
									   fSmallPageWidth, fSmallPageHeight);
				break;

			case 3: // bottom right

				rPageImageArea = RectF(rPagePrintableArea.X + rPagePrintableArea.Width - fSmallPageWidth, rPagePrintableArea.Y + rPagePrintableArea.Height - fSmallPageHeight,
									   fSmallPageWidth, fSmallPageHeight);
				break;
			}
		}

	return rPageImageArea;
	}

//This is the main function for printing a document by thread
bool
CPrintCore::StartPrintThread() {

	oTrace->StartTrace(__WFUNCTION__);
	
	bEndThread = !InitializeCriticalSectionAndSpinCount(&EndThreadCriticalSection, 0x80000400);
	bEndThread = bEndThread || !InitializeCriticalSectionAndSpinCount(&PrintCriticalSection, 0x80000400);

	//Create a thread for printing the document. Only if the critical section
	//is initialized
	if (!bEndThread) {

		hPrintThread = CreateThread(
			NULL,					// default security attributes
			0,						// use default stack size  
			&ThreadPrintDocument,   // thread function 
			oGlobalInstances.oCopyingMachineCore,	// argument to thread function 
			0,						// use default creation flags 
			NULL);					// returns the thread identifier 
 
			}

	oTrace->EndTrace(__WFUNCTION__,(bool)(hPrintThread!=NULL));

	return (bool)(hPrintThread!=NULL); 
	}

//This is the main function for printing a document by thread
int
CPrintCore::PrintDocumentThread(CCopyDocument* poDocument, 
								CPrintSettings* poPrintSettings) {

	sPrintDocumentWithSettings* cPrintDocument;
									
	oTrace->StartTrace(__WFUNCTION__);

	cPrintDocument = NULL;

	if (bEndThread && hPrintThread!=NULL) {

		//Wait for the thread to finish.
		WaitForSingleObject(hPrintThread, INFINITE);
		
		hPrintThread = NULL;
		}

	if (hPrintThread==NULL) {

		StartPrintThread();
		}

	if (hPrintThread!=NULL) {
	
		EnterCriticalSection(&PrintCriticalSection); 

		iPrintRequestNumber++;

		cPrintDocument = new sPrintDocumentWithSettings;
		cPrintDocument->oDocument = poDocument;
		cPrintDocument->oPrintSettings = poPrintSettings;
		cPrintDocument->iPrintRequest = iPrintRequestNumber;

		oPrintQueue.push_back(cPrintDocument);

		LeaveCriticalSection(&PrintCriticalSection);
		}

	oTrace->EndTrace(__WFUNCTION__, (cPrintDocument==NULL) ? -1 : iPrintRequestNumber);

	return cPrintDocument==NULL ? -1 : iPrintRequestNumber;
	}

//This function returns true if an end thread is signalled
bool 
CPrintCore::GetEndThread() {

	bool bResult;

	EnterCriticalSection(&EndThreadCriticalSection); 

	bResult = bEndThread;

    LeaveCriticalSection(&EndThreadCriticalSection);

	return bResult;
	}

//This function set the EndThread signal to true
void 
CPrintCore::SetEndThread() {

	EnterCriticalSection(&EndThreadCriticalSection); 

	bEndThread = true;

    LeaveCriticalSection(&EndThreadCriticalSection); 
	}

//This is the main processing function for printing document
DWORD 
WINAPI ThreadPrintDocument(LPVOID lpParameter) {

	sPrintDocumentWithSettings* cPrintDocument;	
	CPrintCore* oPrintCore;
	CCopyingMachineCore* oCore;
	eActionResult cResult;

	oCore = (CCopyingMachineCore*)lpParameter;
	oPrintCore = oCore->oPrintCore;

	while (!oPrintCore->GetEndThread()) {

		Sleep(500);

		EnterCriticalSection(&oPrintCore->PrintCriticalSection); 

		if (oPrintCore->oPrintQueue.size()>0) {

			Sleep(500);

			cPrintDocument = oPrintCore->oPrintQueue[0];
			
			LeaveCriticalSection(&oPrintCore->PrintCriticalSection);

			cResult = oPrintCore->PrintDocument(cPrintDocument->oDocument, cPrintDocument->oPrintSettings);

			//let the application know we are done.
			oCore->oNotifications->CommunicateEvent(eNotificationDocumentPrintingDone, cPrintDocument); 

			EnterCriticalSection(&oPrintCore->PrintCriticalSection); 

			oPrintCore->oPrintQueue.erase(oPrintCore->oPrintQueue.begin());
			}
		
		LeaveCriticalSection(&oPrintCore->PrintCriticalSection);
		}

	return 0;
	}
