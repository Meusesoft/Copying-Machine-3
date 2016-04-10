#include "StdAfx.h"
#include "CopyingMachineCore.h"

CScanCore::CScanCore(sGlobalInstances pInstances, 
					 CScanSettings* poScanSettings) :
	CScriptObject(L"Scan")
{
	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pInstances, sizeof(sGlobalInstances));

	//set helper classes
	oTrace = pInstances.oTrace;
	oNotifications = (CCoreNotifications*)pInstances.oNotifications;
	oDefaultScanSettings = poScanSettings;
	oLastScanSettings = NULL;

	//init devices
	CScanDevice* oFileDevice;
	WCHAR	cFile[50];

	lCurrentDevice=0;

	LoadString(oGlobalInstances.hLanguage, IDS_FILE, cFile, 50);

	oFileDevice = new CScanDevice(oGlobalInstances);
	oFileDevice->cInterface = eInputFile;
	oFileDevice->cScanningDevice = cFile;
	oFileDevice->cScanningDeviceID = cFile;
	oScanDevices.push_back(oFileDevice);

	//create instances of logic classes
	oScanTwain = new CScanTwain(pInstances, oDefaultScanSettings);
	oScanWia = new CScanWia(pInstances, oDefaultScanSettings);
	oScanLoadFile = new CScanLoadFile(pInstances, oDefaultScanSettings);
}

CScanCore::~CScanCore(void)
{
	//oScanSettings->Save();
	ClearImages();
	delete oScanTwain;
	delete oScanWia;
	delete oScanLoadFile;
	if (oLastScanSettings!=NULL) delete oLastScanSettings;

	//Clear the devices
	long lIndex = oScanDevices.size();

	while (lIndex>0) {
		
		lIndex--;

		delete oScanDevices[lIndex];
		}
}

bool 
CScanCore::ProcessMessage(MSG &pMessage) {

	bool bReturn;

	bReturn = false;

	if (oScanTwain)	bReturn = oScanTwain->ProcessTwainMessage(pMessage);

	return bReturn;
	}

//This function returns the number of error message in the queue of
//the current acquisition interface
int 
CScanCore::GetErrorMessageCount() {

	int iResult;

	oTrace->StartTrace(__WFUNCTION__);

	iResult = 0;

	switch (cCurrentAcquisitionInterface) {

		case eWIA:

			iResult = oScanWia->GetErrorMessageCount();
			break;

		case eTWAIN:
	
			iResult = oScanTwain->GetErrorMessageCount();
			break;
		}
	
	oTrace->EndTrace(__WFUNCTION__, iResult);

	return iResult;
	}


//This function retrieves the last message from the scan interface. The result is a
//stringtable resource identifier.
cErrorMessage 
CScanCore::GetLastErrorMessage(bool pbClearAll) {

	cErrorMessage cResult;

	oTrace->StartTrace(__WFUNCTION__);

	switch (cCurrentAcquisitionInterface) {

		case eWIA:

			cResult = oScanWia->GetLastErrorMessage(pbClearAll);
			break;

		case eTWAIN:
	
			cResult = oScanTwain->GetLastErrorMessage(pbClearAll);
			break;
		}
	
	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}


//This function starts the acquire process. It initiates the scan dialog.
bool 
CScanCore::Acquire(HWND phWnd, CScanSettings* poScanSettings) {

	eAcquireResult eResult;
	bool bResult;
	CScanDevice* oScanDevice;
	CDlgProgressAcquire* oDlgProgress;
	CCopyingMachineCore* oCore;


	oTrace->StartTrace(__WFUNCTION__);

	eResult = eAcquireFailTryTWAIN;

	poScanSettings->hWindow = phWnd;
	
	oScanDevice = poScanSettings->GetScanner();

	if (oScanDevice!=NULL) {

		SetCurrentScanner(oScanDevice);

		eResult = eAcquireOk;
		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		
 		oDlgProgress = oCore->oDialogs->DlgProgressAcquire(phWnd, poScanSettings);

		delete oDlgProgress;
		}

	bResult = (eResult==eAcquireOk);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;	
	}

	
//This function starts the acquire process. It uses oDefaultScanSettings for its settings.
bool
CScanCore::Acquire(HWND phWnd) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = Acquire(phWnd, oDefaultScanSettings);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;	
	}

//This function performs the actual scanning by initiating the WIA or the TWAIN scan
eAcquireResult
CScanCore::Acquire(CScanSettings* poScanSettings) {

	eAcquireResult eResult;
	CScanDevice* oSelectedDevice;
	CCopyingMachineCore* oCore;
	sDocumentCollection cSelectedCollection;

	oTrace->StartTrace(__WFUNCTION__);

	eResult = eAcquireFailTryTWAIN;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	
	oSelectedDevice = poScanSettings->GetScanner();

	if (oSelectedDevice!=NULL) {

		SetCurrentScanner(oSelectedDevice);
		if (oLastScanSettings!=NULL) delete oLastScanSettings;
		oLastScanSettings = poScanSettings;

		//If scanning to a selection, use the colordepth and resolution
		//of the selected collection
		if (poScanSettings->cAcquireType == eAcquireNewDocumentToCollection) {

			oCore->oDocumentCollections->GetDocumentCollection(oLastScanSettings->iCollection, cSelectedCollection);

			oLastScanSettings->SetInt(eColorDepthValue, cSelectedCollection.iColorDepth);
			oLastScanSettings->SetInt(eResolutionValue, cSelectedCollection.iResolution);
			}
		
		//Acquire a file
		if (oSelectedDevice->cInterface == eInputFile) {

			eResult = (oCore->LoadDocument(oLastScanSettings->hWindow) ? eAcquireOk : eAcquireCancelled);
			cCurrentAcquisitionInterface = eInputFile;
			oCore->oNotifications->CommunicateEvent(eNotificationAcquisitionPageDone, (void*)eAcquireOk);
			}	
		
		//Acquire from a WIA device
		if (oScanWia->WiaUsable(oLastScanSettings) && oSelectedDevice->cInterface==eWIA) {

			cCurrentAcquisitionInterface = eWIA;
			eResult = oScanWia->AcquireImagesThread(oLastScanSettings);
			}

		//Acquire from a TWAIN device
		if (eResult == eAcquireFailTryTWAIN) {

			cCurrentAcquisitionInterface = eTWAIN;
			eResult = oScanTwain->CmAcquire(oLastScanSettings);
			}
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)(eResult==eAcquireOk));

	return eResult;
	}
	

//This function sets the current scanner. Based on the device name or device id.
bool 
CScanCore::SetCurrentScanner(CScanDevice* poScanDevice) {

	bool bFound;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__);

	if (poScanDevice!=NULL) {

		oTrace->Add(L"psDevice", poScanDevice->cScanningDeviceID);

		//Test if the selected scanner is present. If not use the File (it is the best alternative).
		bFound = false;
		lIndex = (long)oScanDevices.size();

		while (!bFound && lIndex>0) {

			lIndex--;

			if (poScanDevice->cScanningDeviceID == oScanDevices[lIndex]->cScanningDeviceID) {

				bFound = true;
				lCurrentDevice = lIndex;
				}
			}
		}

	//Set the current device to the FileInput if non was found
	if (!bFound && oScanDevices.size()>0) {
		
		lCurrentDevice = 0;
		}

	oTrace->EndTrace(__WFUNCTION__);

	return true;
}

//this function returns a scansettings instance with a copy of the default scan settings
CScanSettings* 
CScanCore::GetDefaultScanSettings() {

	CScanSettings* oResult;

	oResult = new CScanSettings(oGlobalInstances);
	oResult->Copy(oDefaultScanSettings);

	return oResult;
	}

//this function returns a scansettings instance with a copy of the last scan settings
CScanSettings* 
CScanCore::GetLastScanSettings() {

	return oLastScanSettings;
	}

//this function fills the give sScanDevice structure with the name and ID of the current
//scanner
CScanDevice* 
CScanCore::GetCurrentScanner() {

	CScanDevice* poScanDevice;
	
	oTrace->StartTrace(__WFUNCTION__);

	poScanDevice = NULL;

	if (lCurrentDevice<(long)oScanDevices.size() && lCurrentDevice>=0) {

		poScanDevice = oScanDevices[lCurrentDevice];
		}

	oTrace->EndTrace(__WFUNCTION__);

	return poScanDevice;
}

eScanInterface 
CScanCore::GetCurrentInterface() {

	return cCurrentAcquisitionInterface;
	}


//Retrieve an image from the TWAIN or WIA scan class
cImageTransferStruct 
CScanCore::GetImage() {

	cImageTransferStruct cResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	cResult.hImageHandle = NULL;

	if (ImagesAvailable()>0) {

		if (oScanTwain->ImagesAvailable()) {

			cResult = oScanTwain->GetImage();
			}
		if (oScanLoadFile->ImagesAvailable()) {

			cResult = oScanLoadFile->GetImage();
		}
		if (oScanWia->ImagesAvailable()) {

			cResult = oScanWia->GetImage();
		}
	}

	oTrace->EndTrace(__WFUNCTION__ ,(bool)(cResult.hImageHandle != NULL));

	return cResult;
	}

void
CScanCore::ClearImages() {

	cImageTransferStruct cResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	cResult.hImageHandle = NULL;

	while (oScanTwain->ImagesAvailable()) {

		cResult = oScanTwain->GetImage();
		free(cResult.hImageHandle);
		}
	while (oScanLoadFile->ImagesAvailable()) {

		cResult = oScanLoadFile->GetImage();
		free(cResult.hImageHandle);
		}
	while (oScanWia->ImagesAvailable()) {

		cResult = oScanWia->GetImage();
		free(cResult.hImageHandle);
		}

	oTrace->EndTrace(__WFUNCTION__ );
	}

//Get the last acquisition type
eAcquireType 
CScanCore::GetLastAcquisitionType() {

	eAcquireType cResult;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = oLastScanSettings->cAcquireType;

	oTrace->EndTrace(__WFUNCTION__ );

	return cResult;
	}


//The number of images available to be transferred from the 
//TWAIN or WIA class
int 
CScanCore::ImagesAvailable() {

	int iResult;

	oTrace->StartTrace(__WFUNCTION__);

	iResult = oScanTwain->ImagesAvailable();
	iResult += oScanWia->ImagesAvailable();
	iResult += oScanLoadFile->ImagesAvailable();

	oTrace->EndTrace(__WFUNCTION__, iResult);

	return iResult;
}

//This functions get the requested device from the list of scan device. If the device
//is not found it will return the fileinput device;
CScanDevice* 
CScanCore::GetDevice(eScanInterface pcInterface, 
					 std::wstring psDevice, 
					 std::wstring psDeviceId) {

	bool bFound;
	CScanDevice* oDevice;
							 
	oTrace->StartTrace(__WFUNCTION__);

	bFound = false;
	oDevice = GetFirstDevice();

	//loop through all the device and see if the requested is present
	while (oDevice!=NULL && !bFound) {

		if (oDevice->cInterface == pcInterface &&
			oDevice->cScanningDevice == psDevice &&
			oDevice->cScanningDeviceID == psDeviceId) {

			bFound = true;
			}
		else {
			oDevice = GetNextDevice();
			}
		}

	if (!bFound) {
		//no device found, return the file input device
		oDevice = oScanDevices[0];
		}

	oTrace->EndTrace(__WFUNCTION__, oDevice!=NULL);

	return oDevice;
}

//This functions return the first of the scan devices. It first retrieves all the scanning device
//from the WIA en TWAIN interfaces and stores them in the internal vector.
CScanDevice* 
CScanCore::GetFirstDevice(bool pbReset) {

	CScanDevice* oNewScanDevice;

	iScanDeviceIndex = 0;

	if (pbReset || oScanDevices.size()==1) {

		//oScanDevices.clear();

		//fill the first device, an input from a file
		//LoadString(oGlobalInstances.hLanguage, IDS_FILE, cFile, 50);
		//
		//cNewScanDevice.cInterface = eInputFile;
		//cNewScanDevice.cScanningDevice = cFile;
		//cNewScanDevice.cScanningDeviceID = cFile;
		//oScanDevices.push_back(cNewScanDevice);

		//fill the vector with Wia scan device
		if (oScanWia->WiaAvailable()) {

			oNewScanDevice = oScanWia->GetFirstDevice();

			while (oNewScanDevice!=NULL) {

				oScanDevices.push_back(oNewScanDevice);
				oNewScanDevice = oScanWia->GetNextDevice();
				}
			}

		//fill the vector with Twain scan devices
		oNewScanDevice = oScanTwain->GetFirstDevice();

		while (oNewScanDevice!=NULL) {

			oScanDevices.push_back(oNewScanDevice);
			oNewScanDevice = oScanTwain->GetNextDevice();
			}
		}
	
	return GetNextDevice();
}

//This function retrieves the next device from the scandevices vector
CScanDevice*  
CScanCore::GetNextDevice() {

	CScanDevice* oResult;

	oResult = NULL;

	if (iScanDeviceIndex < (int)oScanDevices.size()) {

		oResult = oScanDevices[iScanDeviceIndex];
		iScanDeviceIndex++;
		}

	return oResult;
}

bool 
CScanCore::GetEndThread() {

	return oScanWia->GetEndThread();
	}

void 
CScanCore::SetEndThread() {

	if (cCurrentAcquisitionInterface==eWIA) oScanWia->SetEndThread();
	}

//This functioes retrieves the device its possibilities
void
CScanCore::GetDeviceCapabilities() {

	CScanDevice* oCurrentDevice;

	oTrace->StartTrace(__WFUNCTION__);

	oCurrentDevice = GetCurrentScanner();

	if (oCurrentDevice!=NULL) {

		switch (oCurrentDevice->cInterface) {

			case eWIA:

				oScanWia->GetDeviceCapabilities(oDefaultScanSettings);
				break;

			case eTWAIN:

				oScanTwain->GetDeviceCapabilities(oDefaultScanSettings);
				break;		
			}
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function shows all the error messages in one messagebox
void 
CScanCore::ShowErrorMessage(HWND phWnd) {

	if (GetErrorMessageCount()>0) {

		oTrace->StartTrace(__WFUNCTION__);
		oTrace->Add(L"ErrorMessageCount", GetErrorMessageCount());

		//Read out the error messages
		wchar_t* cMessage;
		wchar_t* cNewMessage;
		wchar_t* cTemplate;

		int iSize;
		cErrorMessage sErrorMessage;

		iSize = sizeof(wchar_t) * 128 * GetErrorMessageCount();

		cNewMessage = (wchar_t*)malloc(sizeof(wchar_t)*128+2);
		cMessage = (wchar_t*)malloc(iSize+2);
		cTemplate = (wchar_t*)malloc(iSize+2);

		SecureZeroMemory(cMessage, 10);

		for (long lIndex=GetErrorMessageCount(); lIndex>0; lIndex--) {

			sErrorMessage = GetLastErrorMessage();
			LoadStringW(oGlobalInstances.hInstance, sErrorMessage.iMessageID, cNewMessage, 128);
			
			swprintf(cTemplate, iSize, L"%s\n%s", cMessage, cNewMessage);
			memcpy(cMessage, cTemplate, iSize);
			}

		MessageBox(phWnd, cTemplate, L"Copying Machine Error", MB_ICONERROR | MB_OK);

		free(cTemplate);
		free(cMessage);
		free(cNewMessage);
	
		oTrace->EndTrace(__WFUNCTION__);
		}
}

