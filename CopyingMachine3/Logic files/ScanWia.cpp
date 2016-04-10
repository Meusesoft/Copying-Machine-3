#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "ScanWiaDataCallback.h"

CScanWia::CScanWia(sGlobalInstances pcGlobalInstances, CScanSettings* poSettings):
CScanInterfaceBase(pcGlobalInstances, poSettings) 
{
    oWiaDeviceManager = NULL;
    oWiaFirstChildItem = NULL;
    oWiaRootItem = NULL;
    oWiaEnumItem = NULL;

	hScanThread = NULL;

	//Initialize COM
     //CoInitialize(NULL);

 	}

void 
CScanWia::GetDeviceManager() {

   //Create WIA Device Manager
	HRESULT hr;
	
	hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER,
                          IID_IWiaDevMgr,(void**)&oWiaDeviceManager);
    if (!SUCCEEDED(hr))
       {
       ShowResult(hr);
       oWiaDeviceManager = NULL;
       }
	}

void 
CScanWia::ReleaseDeviceManager() {


   if (oWiaDeviceManager!=NULL) {
      ((IWiaDevMgr*)oWiaDeviceManager)->Release();
	  oWiaDeviceManager = NULL;
      }
	}


CScanWia::~CScanWia(void)
{

   if (oWiaFirstChildItem!=NULL) {
      ((IWiaItem*)oWiaFirstChildItem)->Release();
      }

   if (oWiaEnumItem!=NULL) {
        ((IWiaItem*)oWiaEnumItem)->Release();
        }

   if (oWiaRootItem!=NULL) {
      ((IWiaItem*)oWiaRootItem)->Release();
      }

	ReleaseDeviceManager();

	//Clean up the thread
	if (hScanThread!=NULL) {

		SetEndThread();
		
		//Wait for the thread to finish.
		WaitForSingleObject(hScanThread, INFINITE);
	
		CloseHandle(hScanThread);

		DeleteCriticalSection(&ScanCriticalSection);
		DeleteCriticalSection(&EndThreadCriticalSection);
		}
   //CoUninitialize();
	}

//-----------------------------------------------------------------------------
// Public Functions
//-----------------------------------------------------------------------------

bool
CScanWia::GetDeviceCapabilities(CScanSettings* poSettings) {

     bool retval = false;
     HRESULT hr;
	 CScanDevice* oScanDevice;

     oTrace->StartTrace(__WFUNCTION__);

	 oScanDevice = poSettings->GetScanner();

	 if (oScanDevice!=NULL) {

		 retval = true; 
	 
		 if (!oScanDevice->bCapabilitiesRetrieved) {

			 retval = false;

			GetDeviceManager();

			 if (oWiaDeviceManager!=NULL) {
				//Er moet een Wia Device Manager zijn in de eerste plaats
				//om uberhaupt iets te kunnen doen

				if (GetDeviceID(poSettings)) {

					if (GetRootItem(poSettings)) {
					//we hebben nu pointers naar de juiste objecten:
					//we kunnen nu de eigenschappen van de scanner
					//gaan ophalen

					if (GetNextItem()) {

						PROPSPEC    propspec[8];
						PROPVARIANT propvar[8];
						IWiaPropertyStorage *pIWiaPropStg = NULL;
						ULONG flags[8];

						hr = ((IWiaItem*)oWiaFirstChildItem)->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);

						oTrace->Add(L"GetNextItem", SUCCEEDED(hr));

						if (SUCCEEDED(hr)) {

						   //resolutie (assumption: xres = yres)
							propspec[0].ulKind = PRSPEC_PROPID;
							propspec[0].propid = WIA_IPS_XRES;
							flags[0] = WIA_PROP_LIST;

							//brightness
							propspec[1].ulKind = PRSPEC_PROPID;
							propspec[1].propid = WIA_IPS_BRIGHTNESS;
							flags[1] = WIA_PROP_RANGE;

							//contrast
							propspec[2].ulKind = PRSPEC_PROPID;
							propspec[2].propid = WIA_IPS_CONTRAST;
							flags[2] = WIA_PROP_RANGE;

							//datatype (kleurdiepte)
							propspec[3].ulKind = PRSPEC_PROPID;
							propspec[3].propid = WIA_IPA_DATATYPE;
							flags[3] = WIA_PROP_LIST;

							//orientatie (kleurdiepte)
							propspec[4].ulKind = PRSPEC_PROPID;
							propspec[4].propid = WIA_IPS_ROTATION;
							flags[4] = WIA_PROP_LIST;

							//datatype (kleurdiepte)
							propspec[5].ulKind = PRSPEC_PROPID;
							propspec[5].propid = WIA_IPS_XEXTENT;
							flags[5] = WIA_PROP_RANGE;

							//datatype (kleurdiepte)
							propspec[6].ulKind = PRSPEC_PROPID;
							propspec[6].propid = WIA_IPS_YEXTENT;
							flags[6] = WIA_PROP_RANGE;
					
							hr = pIWiaPropStg->GetPropertyAttributes(7, propspec, flags, propvar);
							oTrace->Add(L"GetPropertyAttributes", SUCCEEDED(hr));

							if (SUCCEEDED(hr)) {

								retval = true;

								//resolutie: werkt altijd = mandatory for all scanners
								oScanDevice->bResolution = true;

								oScanDevice->iResolutionNominaal = propvar[0].caul.pElems[WIA_LIST_NOM];
								for (unsigned int i=0; i<propvar[0].caul.pElems[WIA_LIST_COUNT]; i++) {

									switch (propvar[0].caul.pElems[WIA_LIST_VALUES+i]) {

										case 75:   {oScanDevice->bResolution75   = true; break;}
										case 100:  {oScanDevice->bResolution100  = true; break;}
										case 150:  {oScanDevice->bResolution150  = true; break;}
										case 200:  {oScanDevice->bResolution200  = true; break;}
										case 300:  {oScanDevice->bResolution300  = true; break;}
										case 400:  {oScanDevice->bResolution400  = true; break;}
										case 600:  {oScanDevice->bResolution600  = true; break;}
										case 1200: {oScanDevice->bResolution1200 = true; break;}

										}
									}

								//page: mandatory for all scanners
								oScanDevice->bPage = true;
								oScanDevice->iPageWidth = propvar[5].caul.pElems[WIA_RANGE_MAX];
								oScanDevice->iPageHeight = propvar[6].caul.pElems[WIA_RANGE_MAX];

								//brightness:  mandatory for all scanners
								oScanDevice->bBrightness = true;
								oScanDevice->iBrightnessMin = propvar[1].caul.pElems[WIA_RANGE_MIN];
								oScanDevice->iBrightnessMax = propvar[1].caul.pElems[WIA_RANGE_MAX];
								oScanDevice->iBrightnessStep = propvar[1].caul.pElems[WIA_RANGE_STEP];
								//poSettings->SetInt(eBrightnessValue, propvar[1].caul.pElems[WIA_RANGE_NOM]);

								//contrast: mandatory for all scanners
								oScanDevice->bContrast = true;
								oScanDevice->iContrastMin = propvar[2].caul.pElems[WIA_RANGE_MIN];
								oScanDevice->iContrastMax = propvar[2].caul.pElems[WIA_RANGE_MAX];
								oScanDevice->iContrastStep = propvar[2].caul.pElems[WIA_RANGE_STEP];
								//poSettings->SetInt(eContrastValue, propvar[2].caul.pElems[WIA_RANGE_NOM]);

								//color depth
								if (propvar[3].iVal>0) {

									oScanDevice->bColorDepth = true;

										for (unsigned int i=0; i<propvar[3].caul.pElems[WIA_LIST_COUNT]; i++) {

											switch (propvar[3].caul.pElems[WIA_LIST_VALUES+i]) {

												case WIA_DATA_THRESHOLD: {oScanDevice->bBlackWhite = true; break;}

												case WIA_DATA_GRAYSCALE: {oScanDevice->bGray = true; break;}

												case WIA_DATA_COLOR: {oScanDevice->bColor = true; break;}
												}
											}
										}

									//orientatie
									if (propvar[4].iVal>0) {

										oScanDevice->bOrientation = true;

										}

									//Get the document handling capabilities
									PROPSPEC propspec1[1];
									PROPVARIANT propvar1[1];
									IWiaPropertyStorage *pIWiaPropStg;

									hr = ((IWiaItem*)oWiaRootItem)->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);

									propspec1[0].ulKind = PRSPEC_PROPID;
									propspec1[0].propid = WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES;

									hr = pIWiaPropStg->ReadMultiple(1,
												propspec1,
												propvar1);

									oTrace->Add(L"WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES", SUCCEEDED(hr));

									if (SUCCEEDED(hr)) {

										bool bFeeder = (propvar1[0].intVal & FEED);
										oScanDevice->bFeederPresent = bFeeder;
										oTrace->Add(L"Feeder", bFeeder);
										}
									}
								else {
									//geen properties
									ShowResult(hr);
									}

								pIWiaPropStg->Release();
								}
							else {
								//geen property interface
								ShowResult(hr);
								}


							//instances weer vrij geven
							if (oWiaFirstChildItem!=NULL) {
								((IWiaItem*)oWiaFirstChildItem)->Release();
								oWiaFirstChildItem=NULL;
								}
							}

						if (oWiaEnumItem!=NULL) {
							((IWiaItem*)oWiaEnumItem)->Release();
							oWiaEnumItem=NULL;
							}
						}

					if (oWiaRootItem!=NULL) {

						((IWiaItem*)oWiaRootItem)->Release();
						oWiaRootItem=NULL;
						}
					}
				}

			oScanDevice->bCapabilitiesRetrieved = retval;

			ReleaseDeviceManager();
			}
		}

     oTrace->EndTrace(__WFUNCTION__, retval);

     return retval;
}

//-----------------------------------------------------------------------------

bool
CScanWia::AskDefaultScanner(CScanSettings* poSettings) {

  /*HRESULT hr;
  IWiaPropertyStorage* pIWiaPropStg;
  IEnumWIA_DEV_INFO* oWiaEnumeratorDeviceInfo;
  ULONG ulFetched;
  bool bFound = false;
  bool bWiaAlreadyDefault = false;
  bool bRetValue;
  TListItem* oListItem;

  bRetValue = false;

  oTrace->StartTrace(__WFUNCTION__);

  //if (frmSelectScanner==NULL) {
  //   //Application->Initialize();
  //   //Application->CreateForm(__classid(TfrmSelectScanner),&frmSelectScanner);
  //   }


   //vul de lijst met WIA Scanning Devices
   if (oWiaDeviceManager!=NULL) {

        //Enumerating Devices, op zoek naar het scanning devices
        hr = ((IWiaDevMgr*)oWiaDeviceManager)->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL,&oWiaEnumeratorDeviceInfo);
        if (SUCCEEDED(hr))
           {
           // Call Reset on Enumerator
           hr = oWiaEnumeratorDeviceInfo->Reset();

           if (SUCCEEDED(hr))
              {
              do
                {
                // Enumerate requesting 1 Device on each Next call
                hr = oWiaEnumeratorDeviceInfo->Next(1,&pIWiaPropStg,&ulFetched);

                if (hr == S_OK)
                   {
                   PROPSPEC        PropSpec[3];
                   PROPVARIANT     PropVar[3];

                   memset(PropVar,0,sizeof(PropVar));

                   PropSpec[0].ulKind = PRSPEC_PROPID;
                   PropSpec[0].propid = WIA_DIP_DEV_ID;

                   PropSpec[1].ulKind = PRSPEC_PROPID;
                   PropSpec[1].propid = WIA_DIP_DEV_NAME;

                   PropSpec[2].ulKind = PRSPEC_PROPID;
                   PropSpec[2].propid = WIA_DIP_DEV_TYPE;

                   hr = pIWiaPropStg->ReadMultiple(sizeof(PropSpec)/sizeof(PROPSPEC),
                                PropSpec,
                                PropVar);

                   if (hr == S_OK) {
                      //we hebben een device, is het degene die we willen?

                      char converter[250];
                      char returnvalue[10];


                      if (GET_STIDEVICE_TYPE(PropVar[2].lVal) == StiDeviceTypeScanner) //)
                          {
                          ZeroMemory(converter, 250);
                          wcstombs(converter, PropVar[1].bstrVal, 250);

                          oListItem = frmSelectScanner->lvScannerList->Items->Add();
                          oListItem->Caption = converter;
                          oListItem->ImageIndex = 0;
                          oListItem->SubItems->Add("WIA");

                          oWiaTrace->Add(L"DeviceFound:", converter);

                          //vergelijke tekenreeksen
                          if (strcmp(converter, sCommStruct.cScanningDevice)==0) {
                             bWiaAlreadyDefault=true;
                             }

                          wcstombs(converter, PropVar[0].bstrVal, 250);

                          frmSelectScanner->oScannerIDs->Add(converter);

                          //vergelijk ID's
                          if (strcmp(converter, sCommStruct.cScanningDeviceID)==0) {
                             bWiaAlreadyDefault=true;
                             }

                          //Er zijn WIA scanners gevonden
                          bFound = true;
                          }

                      }
                     pIWiaPropStg->Release();
                   }
                 } while (hr == S_OK && !bWiaAlreadyDefault);
              }

           oWiaEnumeratorDeviceInfo->Release();
           oWiaEnumeratorDeviceInfo = NULL;
           }
        else {
            ShowResult(hr);
            }

      if (bFound && !bWiaAlreadyDefault) {
         //toon de dialoogvenster
      
         frmSelectScanner->hResource = hResource;
         bRetValue = (frmSelectScanner->ShowModal() == mrOk);
         }
      }

   delete frmSelectScanner;
   frmSelectScanner = NULL;

   oTrace->EndTrace(__WFUNCTION__);

   return bRetValue;*/

	return true;
}

//-----------------------------------------------------------------------------

bool
CScanWia::CheckExistingID(CScanSettings* poSettings) {

     bool retval = false; //false = bestaat niet!

     oTrace->StartTrace(__WFUNCTION__);

	 GetDeviceManager();

     if (oWiaDeviceManager!=NULL) {
        //Er moet een Wia Device Manager zijn in de eerste plaats
        //om uberhaupt iets te kunnen doen

        if (GetDeviceID(poSettings)) {
           retval = true;
           }
        }

	 ReleaseDeviceManager();

     oTrace->EndTrace(__WFUNCTION__, retval);

     return retval;
     }

//-----------------------------------------------------------------------------
int
CScanWia::GetBrightness(CScanSettings* poSettings)
{
     oTrace->StartTrace(__WFUNCTION__);

	 CScanDevice* oScanDevice;

	oScanDevice = poSettings->GetScanner();
	
	int Bereik=oScanDevice->iBrightnessMax-oScanDevice->iBrightnessMin;
	int Value=oScanDevice->iBrightnessMin+(Bereik*(100+poSettings->GetInt(eBrightnessValue)))/200;
	int Modulus = (Value - oScanDevice->iBrightnessMin) % oScanDevice->iBrightnessStep;

	if (Modulus!=0) {
		
		//we zitten niet mooi op een stepsize
		int Factor;
		Factor = (Value - oScanDevice->iBrightnessMin) / oScanDevice->iBrightnessStep;

		if (Modulus>oScanDevice->iBrightnessStep/2) {
			Factor ++;
     		}

		Value = oScanDevice->iBrightnessMin + Factor * oScanDevice->iBrightnessStep;
		}

     oTrace->EndTrace(__WFUNCTION__, Value);

	 return Value;
}
//-----------------------------------------------------------------------------
int
CScanWia::GetContrast(CScanSettings* poSettings)
{
     oTrace->StartTrace(__WFUNCTION__);

	 CScanDevice* oScanDevice;

	oScanDevice = poSettings->GetScanner();
	
	int Bereik=oScanDevice->iContrastMax-oScanDevice->iContrastMin;
	int Value=oScanDevice->iContrastMin+(Bereik*(100+poSettings->GetInt(eContrastValue)))/200;
	int Modulus = (Value - oScanDevice->iContrastMin) % oScanDevice->iContrastStep;

	if (Modulus!=0) {
		
		//we zitten niet mooi op een stepsize
		int Factor;
		Factor = (Value - oScanDevice->iContrastMin) / oScanDevice->iContrastStep;

		if (Modulus>oScanDevice->iContrastStep/2) {
			Factor ++;
     		}

		Value = oScanDevice->iContrastMin + Factor * oScanDevice->iContrastStep;
		}

     oTrace->EndTrace(__WFUNCTION__, Value);

	 return Value;
}

//-----------------------------------------------------------------------------

eAcquireResult
CScanWia::AcquireImages(CScanSettings* poSettings) {

     eAcquireResult retval = eAcquireOk;
	 CScanDevice* oScanDevice;
     HRESULT hr;
     bool bFeeder;
     bool bDuplex;

     oTrace->StartTrace(__WFUNCTION__);

     bFeeder = false;
     bDuplex = false;

	 oScanDevice = poSettings->GetScanner();

	 if (oScanDevice!=NULL) {
	 
		 GetDeviceManager();

		if (oWiaDeviceManager!=NULL ) {
			//Er moet een Wia Device Manager zijn in de eerste plaats
			//om uberhaupt iets te kunnen doen

			if (GetDeviceID(poSettings)) {

				if (GetRootItem(poSettings)) {

				   if (poSettings->GetBool(eAlwaysShowInterface) || poSettings->GetBool(eShowInterface)) {

						long lItemCount;
						long lItemCounter;
						IWiaItem** oWiaItems;

						hr = ((IWiaItem*)oWiaRootItem)->DeviceDlg((HWND)poSettings->hWindow, WIA_DEVICE_DIALOG_SINGLE_IMAGE,
															  WIA_INTENT_NONE, &lItemCount,
															  &oWiaItems);

						if (SUCCEEDED(hr)) {

					IWiaPropertyStorage *pIWiaPropStg;

					hr = ((IWiaItem*)oWiaRootItem)->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);

					//acquire all queued images from the source device
							lItemCounter = 0;
							while (lItemCounter < lItemCount && retval==0) {

								retval = AcquireWiaItem(pIWiaPropStg, oWiaItems[lItemCounter], poSettings);
								lItemCounter++;
								}
							}
						else {
							retval = eAcquireFail;
							}
						}
				   else {

					PROPSPEC propspec1[1];
					PROPVARIANT propvar1[1];
					IWiaPropertyStorage *pIWiaPropStg;

					hr = ((IWiaItem*)oWiaRootItem)->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);

					oTrace->Add(L"PropertyStorage", SUCCEEDED(hr));

					if (SUCCEEDED(hr)) {

						propspec1[0].ulKind = PRSPEC_PROPID;
						propspec1[0].propid = WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES;

						hr = pIWiaPropStg->ReadMultiple(1,
									propspec1,
									propvar1);

						oTrace->Add(L"WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES", SUCCEEDED(hr));

						if (SUCCEEDED(hr)) {

							/*if (propvar1[0].intVal & DUP) {
								bDuplex = true;
								}*/

							if (propvar1[0].intVal & FEED && propvar1[0].intVal & FLAT) {

								oTrace->Add(L"Choose Document Handling");

									//er moet een keuze worden gemaakt: welke invoer!!!

									/*if (frmChooseDocumentHandling==NULL) {

										Application->CreateForm(__classid(TfrmChooseDocumentHandling),&frmChooseDocumentHandling);
										}

									frmChooseDocumentHandling->hResource = hResource;
									frmChooseDocumentHandling->ParentWindow = (HWND)poProgressForm->Handle;

									//disable the progress form, because else the threads will be
									//messed up when the user cancels the operation while this
									//dialog is shown.
									poProgressForm->Enabled = false;

									int iModalResult = frmChooseDocumentHandling->ShowModal();

									//enabld the progress form again.
									poProgressForm->Enabled = true;

									oWiaTrace->EndTrace("Choose Document Handling: iModalResult", iModalResult);

									delete frmChooseDocumentHandling;
									frmChooseDocumentHandling = NULL;*/

									//int iModalResult = ::MessageBox(NULL, L"Use the feeder?", L"Copying Machine 3", MB_YESNO);

									if (poSettings->GetBool(eUseFeeder))
									{

									/*switch (iModalResult) {

										case IDYES: {*/

											bFeeder = true;

											//Document Feeder
											propspec1[0].ulKind = PRSPEC_PROPID;
											propspec1[0].propid = WIA_DPS_DOCUMENT_HANDLING_SELECT ;
											propvar1[0].vt   = VT_I4;
											propvar1[0].lVal = FEEDER;

											/*if (bDuplex) {
												propvar1[0].lVal |= DUPLEX;
												} */

											hr = pIWiaPropStg->WriteMultiple(1, propspec1, propvar1, 2);

											if (!SUCCEEDED(hr)) {
												bFeeder = false;
												}
											//break;
											}
									else
									{
										//case IDNO: {


											//Flatbed
											propspec1[0].ulKind = PRSPEC_PROPID;
											propspec1[0].propid = WIA_DPS_DOCUMENT_HANDLING_SELECT ;
											propvar1[0].vt   = VT_I4;
											propvar1[0].lVal = FLATBED;

											pIWiaPropStg->WriteMultiple(1, propspec1, propvar1, 2);

											//break;
											}

										/*default: {

											//Cancel
											retval = eAcquireFail;
											break;
											}
										}*/
									}
								}
							}
						if (pIWiaPropStg!=NULL) {
							pIWiaPropStg->Release();
							pIWiaPropStg=NULL;
							}

						GetNextItem();

						while (oWiaFirstChildItem!=NULL && retval==eAcquireOk) {

							// IWiaPropertyStorage *pIWiaPropStg;

							hr = ((IWiaItem*)oWiaFirstChildItem)->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);

							oTrace->Add(L"Query PropertyStorage", SUCCEEDED(hr));

							if (SUCCEEDED(hr)) {
								int nProperties = 0;
								LONG lVal;
								PROPSPEC    propspec[10];
								PROPVARIANT propvar[10];


								//set the choices of the user
								propspec[nProperties].ulKind = PRSPEC_PROPID;
								propspec[nProperties].propid = WIA_IPA_TYMED;
								propvar[nProperties].vt   = VT_I4;
								propvar[nProperties].lVal = TYMED_FILE;
								//propvar[nProperties].lVal = TYMED_CALLBACK;
								nProperties++;

								//resolution
								if (oScanDevice->bResolution && poSettings->GetInt(eResolutionValue)!=-1) {
									propspec[nProperties].ulKind = PRSPEC_PROPID;
									propspec[nProperties].propid = WIA_IPS_XRES;
									propvar[nProperties].vt   = VT_I4;
									propvar[nProperties].lVal = poSettings->GetInt(eResolutionValue);
									nProperties++;
									propspec[nProperties].ulKind = PRSPEC_PROPID;
									propspec[nProperties].propid = WIA_IPS_YRES;
									propvar[nProperties].vt   = VT_I4;
									propvar[nProperties].lVal = poSettings->GetInt(eResolutionValue);
									nProperties++;

									oTrace->Add(L"SetResolution", (int)poSettings->GetInt(eResolutionValue));
									}

								//kleurdiepte
								if (oScanDevice->bColorDepth && poSettings->GetInt(eColorDepthValue)!=-1) {
									propspec[nProperties].ulKind = PRSPEC_PROPID;
									propspec[nProperties].propid = WIA_IPA_DATATYPE;
									propvar[nProperties].vt   = VT_I4;

									lVal = WIA_DATA_THRESHOLD;

									switch (poSettings->GetInt(eColorDepthValue)) {
										case 1: {
											lVal = WIA_DATA_GRAYSCALE;
											break;
											}

										case 2: {
											lVal = WIA_DATA_COLOR;
											break;
											}
										}

									propvar[nProperties].lVal = lVal;

									oTrace->Add(L"SetColor", (int)poSettings->GetInt(eColorDepthValue));

									nProperties++;
									}

								//Helderheid
								if (oScanDevice->bBrightness) {
									propspec[nProperties].ulKind = PRSPEC_PROPID;
									propspec[nProperties].propid = WIA_IPS_BRIGHTNESS;
									propvar[nProperties].vt   = VT_I4;
									propvar[nProperties].lVal = GetBrightness(poSettings);

									oTrace->Add(L"SetBrightness", (int)propvar[nProperties].lVal);

									nProperties++;
									}

								//Contrast
								if (oScanDevice->bContrast) {
									propspec[nProperties].ulKind = PRSPEC_PROPID;
									propspec[nProperties].propid = WIA_IPS_CONTRAST;
									propvar[nProperties].vt   = VT_I4;
									propvar[nProperties].lVal = GetContrast(poSettings);

									oTrace->Add(L"SetContrast", (int)propvar[nProperties].lVal);

									nProperties++;
									}

								//Orientatie
								if (oScanDevice->bOrientation) {
									propspec[nProperties].ulKind = PRSPEC_PROPID;
									propspec[nProperties].propid = WIA_IPS_ROTATION;
									propvar[nProperties].vt   = VT_I4;

									switch (poSettings->GetInt(eOrientationValue)) {
										case 0:
											propvar[nProperties].lVal = PORTRAIT;
											break;
										case 1:
											propvar[nProperties].lVal = LANSCAPE;
											break;
											}
									oTrace->Add(L"SetOrientatie", (int)poSettings->GetInt(eOrientationValue));

									nProperties++;
									}

							  //Pagina omvang
							  //float P_Hoogte[] = {11.69, 8.26, 11, 14};
							  //float P_Breedte[] = {8.26, 5.82, 8.5, 8.5};

							 /* if (poSettings->bPage && poSettings->iPage>=0 && poSettings->bResolution) {
								  propspec[nProperties].ulKind = PRSPEC_PROPID;
								  propspec[nProperties].propid = WIA_IPS_XEXTENT;
								  propvar[nProperties].vt   = VT_I4;
								  //inches omrekenen naar pixels
								  propvar[nProperties].lVal = (int)P_Breedte[poSettings->iPage] * poSettings->iResolution;

								  nProperties++;


								  propspec[nProperties].ulKind = PRSPEC_PROPID;
								  propspec[nProperties].propid = WIA_IPS_YEXTENT;
								  propvar[nProperties].vt   = VT_I4;
								  //inches omrekenen naar pixels
								  propvar[nProperties].lVal = (int)P_Hoogte[poSettings->iPage] * poSettings->iResolution;

								  nProperties++;
								 }
							   */

							 hr = pIWiaPropStg->WriteMultiple(nProperties, propspec, propvar, 2);

							 oTrace->Add(L"Properties WriteMultiple", SUCCEEDED(hr));
							
							 retval = eAcquireFail;

							 if (!SUCCEEDED(hr)) {

								//Show the error message
								ShowResult(hr);
							 }
							 else {

								//de settings zijn ingesteld: scannuh!
								retval = AcquireWiaItem(pIWiaPropStg, oWiaFirstChildItem, poSettings);

								//instances weer vrij geven
								((IWiaItem*)oWiaFirstChildItem)->Release();
								oWiaFirstChildItem=NULL;

								if (retval==eAcquireOk) {

									//nog geen fout tegen gekomen: volgende pagina ophalen

									if (!GetNextItem()) {
										oWiaFirstChildItem=NULL;
										}

									if (oWiaFirstChildItem==NULL && bFeeder) {
										((IWiaItem*)oWiaRootItem)->Release();
										oWiaRootItem=NULL;

										if (GetRootItem(poSettings)) {

											hr = ((IWiaItem*)oWiaRootItem)->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);

											if (SUCCEEDED(hr)) {

												if (bFeeder) {

													bFeeder = true;

													//set the choices of the user
													propspec1[0].ulKind = PRSPEC_PROPID;
													propspec1[0].propid = WIA_DPS_DOCUMENT_HANDLING_SELECT ;
													propvar1[0].vt   = VT_I4;
													propvar1[0].lVal = FEEDER;

													hr = pIWiaPropStg->WriteMultiple(1, propspec1, propvar1, 2);

													if (!SUCCEEDED(hr)) {
														bFeeder = false;
														}
													}
												else {

													//set the choices of the user
													propspec1[0].ulKind = PRSPEC_PROPID;
													propspec1[0].propid = WIA_DPS_DOCUMENT_HANDLING_SELECT ;
													propvar1[0].vt   = VT_I4;
													propvar1[0].lVal = FLATBED;

													pIWiaPropStg->WriteMultiple(1, propspec1, propvar1, 2);
													}
												}

											if (!GetNextItem()) {
												oWiaFirstChildItem=NULL;
												}
											}
										}
									}
								}
							}

						if (pIWiaPropStg!=NULL) {
							//pIWiaPropStg->Release();
							pIWiaPropStg=NULL;
							}
						}

					if (oWiaEnumItem!=NULL) {
						((IWiaItem*)oWiaEnumItem)->Release();
						oWiaEnumItem=NULL;
						}
					}
					((IWiaItem*)oWiaRootItem)->Release();
					oWiaRootItem=NULL;
					}
				}
			else {
				//WIA device niet gevonden: geef probeer TWAIN terug
				retval = eAcquireFailTryTWAIN;
				}
			}
		ReleaseDeviceManager();
		}
	else {

		//Geen scan device in settings; return fail
		retval = eAcquireFailInvalidArgument;
		}

    oTrace->EndTrace(L"AcquireImages", (bool)(retval==eAcquireOk || retval==eAcquireFeederEmpty));

    return retval;
    }

//-----------------------------------------------------------------------------
HRESULT 
ReadPropertyLong(
    IWiaPropertyStorage *pWiaPropertyStorage, 
    const PROPSPEC      *pPropSpec, 
    LONG                *plResult
)
{
    PROPVARIANT PropVariant;

    HRESULT hr = pWiaPropertyStorage->ReadMultiple(
        1, 
        pPropSpec, 
        &PropVariant
    );

    // Generally, the return value should be checked against S_FALSE.
    // If ReadMultiple returns S_FALSE, it means the property name or ID
    // had valid syntax, but it didn't exist in this property set, so
    // no properties were retrieved, and each PROPVARIANT structure is set 
    // to VT_EMPTY. But the following switch statement will handle this case
    // and return E_FAIL. So the caller of ReadPropertyLong does not need
    // to check for S_FALSE explicitly.

    if (SUCCEEDED(hr))
    {
        switch (PropVariant.vt)
        {
            case VT_I1:
            {
                *plResult = (LONG) PropVariant.cVal;

                hr = S_OK;

                break;
            }

            case VT_UI1:
            {
                *plResult = (LONG) PropVariant.bVal;

                hr = S_OK;

                break;
            }

            case VT_I2:
            {
                *plResult = (LONG) PropVariant.iVal;

                hr = S_OK;

                break;
            }

            case VT_UI2:
            {
                *plResult = (LONG) PropVariant.uiVal;

                hr = S_OK;

                break;
            }

            case VT_I4:
            {
                *plResult = (LONG) PropVariant.lVal;

                hr = S_OK;

                break;
            }

            case VT_UI4:
            {
                *plResult = (LONG) PropVariant.ulVal;

                hr = S_OK;

                break;
            }

            case VT_INT:
            {
                *plResult = (LONG) PropVariant.intVal;

                hr = S_OK;

                break;
            }

            case VT_UINT:
            {
                *plResult = (LONG) PropVariant.uintVal;

                hr = S_OK;

                break;
            }

            case VT_R4:
            {
                *plResult = (LONG) (PropVariant.fltVal + 0.5);

                hr = S_OK;

                break;
            }

            case VT_R8:
            {
                *plResult = (LONG) (PropVariant.dblVal + 0.5);

                hr = S_OK;

                break;
            }

            default:
            {
                hr = E_FAIL;

                break;
            }
        }
    }

    PropVariantClear(&PropVariant);

    return hr;
}

	
	eAcquireResult
CScanWia::AcquireWiaItem(IWiaPropertyStorage *pIWiaPropStg, void* poWiaItem, CScanSettings* poSettings) {

    //this function acquires an image from a WiaItem
    oTrace->StartTrace(__WFUNCTION__);

    HRESULT hr;
    cImageTransferStruct sResult;
    eAcquireResult eRetval;

    eRetval = eAcquireFail;   //-2 is system error, -1 cancel, 0 OK

    IWiaDataCallback *pIWiaDataCallback = NULL;
    IWiaDataTransfer *pIWiadataTransfer = NULL;
    
	CScanWiaDataCallback* oWiaDataCallback;

    //Create the scanning interface (IWiaDataTransfer)
    hr = ((IWiaItem*)poWiaItem)->QueryInterface(IID_IWiaDataTransfer, (void **)&pIWiadataTransfer);
    oTrace->Add(L"Get scanning interface", SUCCEEDED(hr));

    if (SUCCEEDED(hr)) {

        //Create the callback class for receiving data (CScanWiaDataCallback)
        oWiaDataCallback = new CScanWiaDataCallback();
        oTrace->Add(L"Get CScanWiaDataCallback", oWiaDataCallback!=NULL);

        if (oWiaDataCallback) {

            //Pass arguments to the callback class
			oWiaDataCallback->hProgressBar = hProgressBar;
			oWiaDataCallback->oScanWia = (void*)this;

            //Create the CScanWiaDataCallback interface
            
			hr = oWiaDataCallback->QueryInterface(IID_IWiaDataCallback,(void **)&pIWiaDataCallback);
            oTrace->Add(L"Get CScanWiaDataCallback interface", SUCCEEDED(hr));

			if (SUCCEEDED(hr)) {

 			// Read the transfer buffer size from the device, default to 64K
				PROPSPEC specBufferSize;

				specBufferSize.ulKind = PRSPEC_PROPID;
				specBufferSize.propid = WIA_IPA_BUFFER_SIZE;

				LONG nBufferSize;

				hr = ReadPropertyLong(
					pIWiaPropStg, 
					&specBufferSize, 
					&nBufferSize
				);

				if (FAILED(hr))
				{
					nBufferSize = 64 * 1024;
				}

				// Set storage medium
				//
				STGMEDIUM StgMedium = {0};
				StgMedium.tymed = TYMED_FILE;

				hr = pIWiadataTransfer->idtGetData( &StgMedium, pIWiaDataCallback );

/*
				//Initiate the data transfer block
                byte* DataBuffer = (byte*)malloc(2 * nBufferSize);
				
				WIA_DATA_TRANSFER_INFO  wiaDataTransInfo;
                SecureZeroMemory(&wiaDataTransInfo, sizeof(WIA_DATA_TRANSFER_INFO));
                wiaDataTransInfo.ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
				wiaDataTransInfo.bDoubleBuffer = TRUE;
                wiaDataTransInfo.ulBufferSize = 2 * nBufferSize;// don: 524288;//262144; // calculate, or determine buffer size
				wiaDataTransInfo.ulSection = (ULONG)DataBuffer;

                //Execute transfer call
                hr = pIWiadataTransfer->idtGetBandedData(&wiaDataTransInfo, pIWiaDataCallback);*/

                oTrace->Add(L"Transfer data", SUCCEEDED(hr));

				switch (hr)
				{
					case S_OK:
						{

						 oTrace->Add(L"Start ReceivingImage");

						 SecureZeroMemory(&sResult, sizeof(sResult));
						 //sResult.hImageHandle = oWiaDataCallback->hImage;

						 //Read the file and store the memory handle
						IStream *oFileStream(0); 
						if (SUCCEEDED (SHCreateStreamOnFile(StgMedium.lpszFileName, 
							STGM_READ, 
							&oFileStream)) ) {

							Bitmap* temp = new Bitmap(oFileStream);

							sResult.hImageHandle = CBitmap::ConvertToHandle(temp);

							delete temp;

							oFileStream->Release();
						}


						if (oImages.size()<oImages.max_size() && sResult.hImageHandle!=NULL) {

							/*BYTE* pbBuf = (BYTE*)GlobalLock(sResult.hImageHandle);
							VerticalFlip(pbBuf);

							GlobalUnlock(sResult.hImageHandle);*/

							sResult.iDPIX = GetResolutionFromImage(sResult.hImageHandle);
							sResult.iDPIY = GetResolutionFromImage(sResult.hImageHandle);

							oImages.push_back(sResult);

							eRetval = eAcquireOk;
							}

						oTrace->Add(L"End ReceivingImage");
						}

					break;

					case WIA_ERROR_PAPER_EMPTY:

						eRetval = eAcquireFeederEmpty; //The feeder is empty.

						break;

					default:

						eRetval = eAcquireFail; //User cancelled on us.

						break;

				}

				//Release the memory associated with the stgmedium and the temporary file
				ReleaseStgMedium(&StgMedium);
            
				//Release pIWiadataTransfer interface
				pIWiadataTransfer->Release();
	
//				free(DataBuffer);
				}


			//Do not delete it yourself, still references open to it. 
			//If deleted CoUnitialize will give a heap error.
			//delete oWiaDataCallback; 
            } 

      // Release IWiadataTransfer Interface
      pIWiadataTransfer->Release();
      }

	if (eRetval==eAcquireFail) {
		if (!bEndThread) {
			//An error occurred: show it
			ShowLastError();
			AddErrorMessage(IDS_ERROROCCURRED, (int)eAcquireFail, true); //TO DO, change into error while scanning
			}
		else {

			AddErrorMessage(IDS_ACTIONCANCELLED, 0, true); 
			}
		}

    oTrace->EndTrace(__WFUNCTION__);

    return eRetval;
    }

//-----------------------------------------------------------------------------

bool
CScanWia::GetRootItem(CScanSettings* poSettings) {

     bool retval = false;
     HRESULT hr;
     oWiaFirstChildItem = NULL;
	 CScanDevice* oScanDevice;

     oTrace->StartTrace(__WFUNCTION__);

	 oScanDevice = poSettings->GetScanner();

     if (oWiaDeviceManager!=NULL && oScanDevice!=NULL) {

		 oWiaRootItem = NULL;

		 BSTR sDevice;

		 sDevice = SysAllocString(oScanDevice->cScanningDeviceID.c_str());

        hr = ((IWiaDevMgr*)oWiaDeviceManager)->CreateDevice(sDevice, (IWiaItem**)&oWiaRootItem);

		SysFreeString(sDevice);

        oTrace->Add(L"CreateDevice", SUCCEEDED(hr));

        if (SUCCEEDED(hr)) {

             oWiaEnumItem = NULL;
             retval = true;
           }
        else {

            oWiaRootItem = NULL;
            ShowResult(hr);
            }
        }

     //SysFreeString(Converter);

     oTrace->EndTrace(__WFUNCTION__, retval);
     return retval;

}

//-----------------------------------------------------------------------------

bool
CScanWia::GetNextItem() {

     HRESULT hr;
     oWiaFirstChildItem = NULL;
     IWiaItem *pIWiaItem = NULL;
     LONG ItemType;
     bool retval = false;
     ULONG plFetched;

     oTrace->StartTrace(__WFUNCTION__);

     if (oWiaDeviceManager!=NULL) {

        if (oWiaEnumItem==NULL) {

            oWiaEnumItem = NULL;

 	        hr = ((IWiaItem*)oWiaRootItem)->EnumChildItems((IEnumWiaItem**)&oWiaEnumItem);

	        if (!SUCCEEDED(hr)) {

                oWiaEnumItem=NULL;
                }
            }


        if (oWiaEnumItem!=NULL) {

            if (oWiaFirstChildItem!=NULL) {
                ((IWiaItem*)oWiaFirstChildItem)->Release();
                oWiaFirstChildItem=NULL;
                }

            hr = ((IEnumWiaItem*)oWiaEnumItem)->Next(1, &pIWiaItem, &plFetched);

            if (SUCCEEDED(hr) && pIWiaItem!=NULL) {

                pIWiaItem->GetItemType(&ItemType);

                oTrace->Add(L"GetItemType=", ItemType);

                // is het een image transfer geval?
                if ((ItemType || WiaItemTypeTransfer) ||
                   (ItemType || WiaItemTypeImage)) {

                   oWiaFirstChildItem = pIWiaItem;
                   retval = true;
                   }
                else {
                    pIWiaItem->Release();
                    oWiaFirstChildItem = NULL;
                    }
                }
            }
        }

     oTrace->EndTrace(__WFUNCTION__, retval);
     return retval;
}

//-----------------------------------------------------------------------------

bool
CScanWia::GetDeviceID(CScanSettings* poSettings) {

     bool retval = false;
     bool found = false;
     HRESULT hr;
     IEnumWIA_DEV_INFO* oWiaEnumeratorDeviceInfo;
     IWiaPropertyStorage* pIWiaPropStg;
     ULONG ulFetched;
	 CScanDevice* oScanDevice;

     oTrace->StartTrace(__WFUNCTION__);

	 oScanDevice = poSettings->GetScanner();

     if (oWiaDeviceManager!=NULL && oScanDevice!=NULL) {
        
		 //Er moet een Wia Device Manager zijn in de eerste plaats
        //om uberhaupt iets te kunnen doen

        //Enumerating Devices, op zoek naar het gezochte scanning device

        hr = ((IWiaDevMgr*)oWiaDeviceManager)->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL,&oWiaEnumeratorDeviceInfo);

        oTrace->Add(L"GetEnumerator", SUCCEEDED(hr));

        if (SUCCEEDED(hr))
            {
            // Call Reset on Enumerator
            hr = oWiaEnumeratorDeviceInfo->Reset();

            oTrace->Add(L"Reset Enumerator", SUCCEEDED(hr));

            if (SUCCEEDED(hr))
              {
              do
                {
                // Enumerate requesting 1 Device on each Next call
                hr = oWiaEnumeratorDeviceInfo->Next(1,&pIWiaPropStg,&ulFetched);

                oTrace->Add(L"Get next device", SUCCEEDED(hr));

                if (hr == S_OK)
                   {
                   // Do something with the device

                   PROPSPEC        PropSpec[6];
                   PROPVARIANT     PropVar[6];

                   memset(PropVar,0,sizeof(PropVar));

                   PropSpec[0].ulKind = PRSPEC_PROPID;
                   PropSpec[0].propid = WIA_DIP_DEV_ID;

                   PropSpec[1].ulKind = PRSPEC_PROPID;
                   PropSpec[1].propid = WIA_DIP_DEV_NAME;

                   //paginaomvang
                   PropSpec[2].ulKind = PRSPEC_PROPID;
                   PropSpec[2].propid = WIA_DPS_HORIZONTAL_BED_SIZE;

                   PropSpec[3].ulKind = PRSPEC_PROPID;
                   PropSpec[3].propid = WIA_DPS_PAGE_HEIGHT;

                   //document handling properties
                   PropSpec[4].ulKind = PRSPEC_PROPID;
                   PropSpec[4].propid = WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES;

                   PropSpec[5].ulKind = PRSPEC_PROPID;
                   PropSpec[5].propid = WIA_DPS_DOCUMENT_HANDLING_STATUS;


                    hr = pIWiaPropStg->ReadMultiple(sizeof(PropSpec)/sizeof(PROPSPEC),
                                PropSpec,
                                PropVar);

                    oTrace->Add(L"Read Properties", hr == S_OK);


                    if (hr == S_OK) {
                      //we hebben een device, is het degene die we willen?

					  std::wstring sDeviceMinusPrefix;

                     oTrace->Add(L"ScanningDevice", oScanDevice->cScanningDevice);
                     oTrace->Add(L"ScanningDeviceID", oScanDevice->cScanningDeviceID);
                     oTrace->Add(L"DeviceReturned", (wchar_t*)PropVar[1].bstrVal);
                     oTrace->Add(L"DeviceReturned", (wchar_t*)PropVar[0].bstrVal);

					 if (oScanDevice->cScanningDeviceID==PropVar[0].bstrVal || oScanDevice->cScanningDevice==PropVar[1].bstrVal) {
                         
						 found = retval = true;

//                         poSettings->cScanDevice.cScanningDevice = PropVar[1].bstrVal;
//                         poSettings->cScanDevice.cScanningDeviceID = PropVar[0].bstrVal;
						 oScanDevice->iPageHeight = PropVar[2].intVal;
                         oScanDevice->iPageWidth = PropVar[3].intVal;
                         }

                      oTrace->Add(L"CompareDevices", found);
                      }


                   //Free Prop Varient
                   FreePropVariantArray(sizeof(PropSpec)/sizeof(PROPSPEC),PropVar);

                   pIWiaPropStg->Release();
                   }

              }
           while (hr == S_OK && !found);

           if (!found) {
              retval=false;
              }
           }
        else {
             //EnumDevice.reset failed
             ShowResult(hr);
             retval = false;
             }

        oWiaEnumeratorDeviceInfo->Release();
        oWiaEnumeratorDeviceInfo = NULL;
        }
        else{
             //Create Enumerator failed
             ShowResult(hr);
             retval = false;
             }
        }

     oTrace->EndTrace(__WFUNCTION__, retval);

     return retval;
}

//-----------------------------------------------------------------------------

void
CScanWia::ShowResult(HRESULT phr) {

	std::wstring sMessage;
    bool bShowError;

    bShowError = true;

    oTrace->StartTrace(__WFUNCTION__);

    switch (phr) {

        case WIA_ERROR_DEVICE_LOCKED: {

            sMessage = L"The scanner head is locked.";
            break;
            }

        case WIA_ERROR_EXCEPTION_IN_DRIVER: {

            sMessage = L"The device driver threw an exception.";
            break;
            }

        case WIA_ERROR_GENERAL_ERROR: {

            sMessage = L"An unknown error has occurred with the WIA device.";
            break;
            }

        case WIA_ERROR_INCORRECT_HARDWARE_SETTING: {

            sMessage = L"There is an incorrect setting on the WIA device.";
            break;
            }

        case WIA_ERROR_INVALID_DRIVER_RESPONSE: {

            sMessage = L"The response from the driver is invalid.";
            break;
            }

        case WIA_ERROR_PAPER_JAM: {

            sMessage = L"Paper is jammed in the scanner's document feeder.";
            break;
            }

        case WIA_ERROR_PAPER_EMPTY: {

            sMessage = L"The user requested a scan and there are no documents left in the document feeder.";
            break;
            }

        case WIA_ERROR_PAPER_PROBLEM: {

            sMessage = L"An unspecified problem occurred with the scanner's document feeder.";
            break;
            }

        case WIA_ERROR_OFFLINE: {

            sMessage = L"The WIA device is not online.";
            break;
            }

        case WIA_ERROR_BUSY: {

            sMessage = L"The WIA device is busy.";
            break;
            }

        case WIA_ERROR_WARMING_UP: {

            sMessage = L"The WIA device is warming up.";
            break;
            }

        case WIA_ERROR_USER_INTERVENTION: {

            sMessage = L"An unspecified error has occurred with the WIA device that requires user intervention. The user should ensure that the device is turned on, online, and any cables are properly connected.";
            break;
            }

        case WIA_ERROR_ITEM_DELETED: {

            sMessage = L"The WIA device was deleted. It can no longer be accessed.";
            break;
            }

        case WIA_S_NO_DEVICE_AVAILABLE: {

            sMessage = L"No WIA device of the selected type is available.";
            break;
            }

        case WIA_ERROR_DEVICE_COMMUNICATION: {

            sMessage = L"An unspecified error occurred during an attempted communication with the WIA device.";
            break;
            }

        case WIA_ERROR_INVALID_COMMAND: {

            sMessage = L"The device does not support this command.";
            break;
            }

		case E_INVALIDARG: {

			sMessage = L"The device does not support the requested (combination of) settings";
			break;
			}

        default: {

            sMessage = L"The last operation return an unknown result.";
            bShowError = false;
            break;
            }
        }

    // Display the string.
    if (bShowError) {
        MessageBox( NULL, sMessage.c_str(), L"Copying Machine 3", MB_OK|MB_ICONINFORMATION );
        }
       
    oTrace->EndTrace(__WFUNCTION__, sMessage);
    }
//-----------------------------------------------------------------------------
void
CScanWia::ShowLastError() {

    oTrace->StartTrace(__WFUNCTION__);

    TCHAR* lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );

    // Display the string.
    MessageBox( NULL, lpMsgBuf, L"Copying Machine", MB_OK|MB_ICONINFORMATION );

    // Free the buffer.
    LocalFree( lpMsgBuf );

    oTrace->EndTrace(__WFUNCTION__);
}

//-----------------------------------------------------------------------------

VOID
CScanWia::VerticalFlip(BYTE *pBuf)
{
    LONG                lHeight;
    LONG                lWidth;
    BITMAPINFOHEADER    *pbmih;
    PBYTE               pTop;
    PBYTE               pBottom;

     oTrace->StartTrace(__WFUNCTION__);

    pbmih = (BITMAPINFOHEADER*) pBuf;

    //flip alleen als het nodig is: biHeight<0
    if (pbmih->biHeight > 0) {
		oTrace->EndTrace(__WFUNCTION__);
        return;
    }

    if (pbmih->biCompression != BI_RGB) {
        ::MessageBox(NULL, L"Unknown image type. Cannot perform the vertical flip", L"Copying Machine 3", MB_OK|MB_ICONEXCLAMATION);
		oTrace->EndTrace(__WFUNCTION__);
        return;
		}

	 pTop = pBuf + pbmih->biSize + ((pbmih->biClrUsed) * sizeof(RGBQUAD));
    lWidth = ((pbmih->biWidth * pbmih->biBitCount + 31) / 32) * 4;
    pbmih->biHeight = abs(pbmih->biHeight);
    lHeight = pbmih->biHeight;

    PBYTE pTempBuffer = (PBYTE)LocalAlloc(LPTR, lWidth);

    if (pTempBuffer) {
        LONG  index;
        pBottom = pTop + (lHeight-1) * lWidth;
        for (index = 0;index < (lHeight/2);index++) {

            memcpy(pTempBuffer, pTop, lWidth);
            memcpy(pTop, pBottom, lWidth);
            memcpy(pBottom,pTempBuffer, lWidth);

            pTop    += lWidth;
            pBottom -= lWidth;
        }
        LocalFree(pTempBuffer);
    }

    oTrace->EndTrace(__WFUNCTION__);
}

//-----------------------------------------------------------------------------
//This function checks if Wia is available in this windows version.
bool
CScanWia::WiaAvailable() {

	oTrace->StartTrace(__WFUNCTION__);

	bool retval = false;

	//eerste versie van windows bepalen, WIA is beperkt beschikbaar
	OSVERSIONINFO lpVersionInformation;

	lpVersionInformation.dwOSVersionInfoSize = sizeof(lpVersionInformation);

	if (GetVersionEx(&lpVersionInformation)!=0) {

		//WIA is alleen beschikbaar vanaf winME en winXP, dus geldt niet voor
		//win95, win98, winNT3, winNT4 en win2000.

		if (lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32s ||
			(lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && lpVersionInformation.dwMajorVersion<=4 && lpVersionInformation.dwMinorVersion<90) ||
			(lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT	&& lpVersionInformation.dwMajorVersion<=5 && lpVersionInformation.dwMinorVersion<1)) {
				//Windows Image Acquisition is not supported in these platform versions
				//Win32s, Win95, Win98, WinNT3 en WinNT4
				retval = false;
				}
		else {
			retval = true;
			}
		}

   	oTrace->EndTrace(__WFUNCTION__, retval);

	return retval;
	}


//This functions checks if Wia is available in this Windows version and if a scanner is set. If one of the two
//fails it will return false
bool
CScanWia::WiaUsable(CScanSettings* poSettings) {

	CScanDevice* oScanDevice;

	oTrace->StartTrace(__WFUNCTION__);

	bool retval = false;

	oScanDevice = poSettings->GetScanner();
	
	if (oScanDevice!=NULL) {

		//als de ID van de scanning device is ingevuld, en wia is bruikbaar dan gaan we die gebruiken. Anders via
		//de reguliere TWAIN interface

		if (WiaAvailable()) {

			if (oScanDevice->cScanningDeviceID.length()>0 && oScanDevice->cScanningDeviceID!=L"-") {
				retval = true;
				}
		}
	}

	oTrace->EndTrace(__WFUNCTION__, retval);

	return retval;
	}

CScanDevice* 
CScanWia::GetFirstDevice() {

	HRESULT hr;
	IWiaPropertyStorage* pIWiaPropStg;
	IEnumWIA_DEV_INFO* oWiaEnumeratorDeviceInfo;
	ULONG ulFetched;
	CScanDevice* oNewScanDevice;
	CScanDevice* poDevice;

	oTrace->StartTrace(__WFUNCTION__);

	oScanDevices.clear();	
	iScanDeviceIndex=0;

	GetDeviceManager();

	//vul de lijst met WIA Scanning Devices
	if (oWiaDeviceManager!=NULL) {

        //Enumerating Devices, op zoek naar het scanning devices
        hr = ((IWiaDevMgr*)oWiaDeviceManager)->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL,&oWiaEnumeratorDeviceInfo);
        if (SUCCEEDED(hr))
           {
           // Call Reset on Enumerator
           hr = oWiaEnumeratorDeviceInfo->Reset();

           if (SUCCEEDED(hr))
              {
              do
                {
                // Enumerate requesting 1 Device on each Next call
                hr = oWiaEnumeratorDeviceInfo->Next(1,&pIWiaPropStg,&ulFetched);

                if (hr == S_OK)
                   {
                   PROPSPEC        PropSpec[3];
                   PROPVARIANT     PropVar[3];

                   memset(PropVar,0,sizeof(PropVar));

                   PropSpec[0].ulKind = PRSPEC_PROPID;
                   PropSpec[0].propid = WIA_DIP_DEV_ID;

                   PropSpec[1].ulKind = PRSPEC_PROPID;
                   PropSpec[1].propid = WIA_DIP_DEV_NAME;

                   PropSpec[2].ulKind = PRSPEC_PROPID;
                   PropSpec[2].propid = WIA_DIP_DEV_TYPE;

                   hr = pIWiaPropStg->ReadMultiple(sizeof(PropSpec)/sizeof(PROPSPEC),
                                PropSpec,
                                PropVar);

                   if (hr == S_OK) {
                      //we hebben een device, is het degene die we willen?


                      if (GET_STIDEVICE_TYPE(PropVar[2].lVal) == StiDeviceTypeScanner) 
                          {
							oNewScanDevice = new CScanDevice(oGlobalInstances);
							oNewScanDevice->cInterface = eWIA;
							oNewScanDevice->cScanningDevice = PropVar[1].bstrVal;
							oNewScanDevice->cScanningDeviceID = PropVar[0].bstrVal;
							oScanDevices.push_back(oNewScanDevice);

							oTrace->Add(L"DeviceID", oNewScanDevice->cScanningDeviceID);
							}

                      }
                     pIWiaPropStg->Release();
                   }
                 } while (hr == S_OK);
              }

           oWiaEnumeratorDeviceInfo->Release();
           oWiaEnumeratorDeviceInfo = NULL;

		   ReleaseDeviceManager();
           }
        else {
            ShowResult(hr);
            }

      }

	poDevice = GetNextDevice();   	

   	oTrace->EndTrace(__WFUNCTION__, poDevice!=NULL);

	return poDevice;
	}


//This function return the next device in the ScanDevices vector. Until it has reached the end of the vector
//it will return true with the poDevice filled with the properties of the scan device.
CScanDevice* 
CScanWia::GetNextDevice() {

	CScanDevice* poDevice;

	poDevice = NULL;

	if (iScanDeviceIndex < (int)oScanDevices.size()) {

		poDevice = oScanDevices[iScanDeviceIndex];
		iScanDeviceIndex++;
		}

	return poDevice;
	}

//This is the main function acquiring images form a WIA device
eAcquireResult
CScanWia::AcquireImagesThread(CScanSettings* poSettings) {

	bEndThread = !InitializeCriticalSectionAndSpinCount(&EndThreadCriticalSection, 0x80000400);
	bEndThread = !InitializeCriticalSectionAndSpinCount(&ScanCriticalSection, 0x80000400);

	//Create a thread for scanning images. Only if the critical section
	//is initialized
	if (!bEndThread) {

		oSettings = poSettings;
		hProgressBar = oSettings->hProgressBar;

		hScanThread = CreateThread(
			NULL,				// default security attributes
			0,					// use default stack size  
			&ThreadScanWia,        // thread function 
			this,				// argument to thread function 
			0,					// use default creation flags 
			NULL);				// returns the thread identifier 
 
			}

	return (hScanThread!=NULL ? eAcquireOk : eAcquireFail); 
	}

bool 
CScanWia::GetEndThread() {

	bool bResult;

	EnterCriticalSection(&EndThreadCriticalSection); 

	bResult = bEndThread;

    LeaveCriticalSection(&EndThreadCriticalSection);

	return bResult;
	}

void 
CScanWia::SetEndThread() {

	EnterCriticalSection(&EndThreadCriticalSection); 

	bEndThread = true;

    LeaveCriticalSection(&EndThreadCriticalSection); 
	}

DWORD 
WINAPI ThreadScanWia(LPVOID lpParameter) {

	CScanWia* oScanWia;
	eAcquireResult cResult;

	oScanWia = (CScanWia*)lpParameter;

	CoInitialize(NULL);

	EnterCriticalSection(&oScanWia->ScanCriticalSection); 

	cResult = oScanWia->AcquireImages(oScanWia->oSettings);

    LeaveCriticalSection(&oScanWia->ScanCriticalSection); 

	CoUninitialize();

	PostMessage(oScanWia->oSettings->hWindow, WM_SCANIMAGESRECEIVED, cResult==eAcquireOk || cResult==eAcquireFeederEmpty ? 0 : 1, 0);

	return 0;
	}
