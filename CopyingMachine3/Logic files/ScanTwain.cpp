#include "StdAfx.h"
#include "CopyingMachineCore.h"

double P_Hoogte[] = {11.69, 8.26, 11, 14};
double P_Breedte[] = {8.26, 5.82, 8.5, 8.5};                                

CScanTwain::CScanTwain(sGlobalInstances pInstances, CScanSettings* poSettings) :
	CScanInterfaceBase(pInstances, poSettings) {

	oTrace->StartTrace(__WFUNCTION__);

	StandardResolution=0;
	ResolutionFromImage=-1;
	CalculatedResolutionFromImage=-1;

	//Identify Application
	appID.Id = 0;
	appID.Version.MajorNum = 1;
	appID.Version.MinorNum = 5;
	appID.Version.Language = TWLG_USA;
	appID.Version.Country  = TWCY_USA;
	strcpy_s(appID.Version.Info, sizeof(appID.Version.Info), "Copying Machine 3");
	strcpy_s(appID.ProductName, sizeof(appID.ProductName),  "copying.exe");

	appID.ProtocolMajor =    TWON_PROTOCOLMAJOR;
	appID.ProtocolMinor =    TWON_PROTOCOLMINOR;
	appID.SupportedGroups =  DG_IMAGE | DG_CONTROL;
	strcpy_s(appID.Manufacturer, sizeof(appID.Manufacturer), "Meusesoft");
	strcpy_s(appID.ProductFamily, sizeof(appID.ProductFamily), "Scan Utility");

	//Initialising DataSource ID
	strcpy_s(dsID.ProductName, sizeof(dsID.ProductName), "\0");
	dsID.Id=0;

	//Initialise variables
	TWDSMOpen=TWDSOpen=InitSet=false;
	
	/*
	WiaOpen = false;
	hWiaDLL = NULL;
	oWia = NULL;
	*/

	Chocolate=false;
	bPagesAvailable = false;
	oLastScanDevice = NULL;

	//Load Data Source Manager
	/*EnableDSM();
	Available=DSMLoaded();
	DisableDSM();*/

	//oRegistry->ReadString(L"General",L"DefaultScanner",L"-", oLastScanDevice->cScanningDevice);
	//oRegistry->ReadString(L"General",L"DefaultScannerID",L"-", oLastScanDevice->cScanningDeviceID);
	
	SetPaperWidth=1;
	bDefaultScannerAvailable = true;

	oTrace->EndTrace(__WFUNCTION__);
}

CScanTwain::~CScanTwain()
{
   oTrace->StartTrace(__WFUNCTION__);

  // DisableDS();
  // DisableDSM();

   oTrace->EndTrace(__WFUNCTION__);

	//delete oTrace;
	}

bool CScanTwain::CheckMessage() {

	return (bool)(oRegistry->ReadInt(L"Scan",L"bShowErrorMessage",1)==1);
	}

bool CScanTwain::ProcessTwainMessage(MSG &m) {

   TW_EVENT twEvent;
   TW_INT16 rc;
   rc=TWRC_NOTDSEVENT;

   if ((TWDSMOpen) && (TWDSOpen))
   	{
      twEvent.pEvent = (TW_MEMREF) &m;
      twEvent.TWMessage = MSG_NULL;
      rc=(*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_EVENT,
      						MSG_PROCESSEVENT, (TW_MEMREF) &twEvent);

      switch (twEvent.TWMessage)
      	{
         case MSG_XFERREADY:
         	{
            DoTransfer();
            break;
            }

         case MSG_CLOSEDSREQ:
         	{
            DisableDS();
            CloseDS();
            DisableDSM();

            //let the application know a cancel has happened
	 		PostMessage(oSettings->hWindow, WM_SCANIMAGESRECEIVED, 1, 0);
			
			AddErrorMessage(IDS_ACTIONCANCELLED, 0, true);
            break;
            }

         case MSG_NULL:
         	{
            break;
            }
         }
      }

   if (rc==TWRC_NOTDSEVENT) return false;
   return true;
}

CScanDevice* 
CScanTwain::GetFirstDevice() {

	TW_UINT16 twRC;
	TW_IDENTITY dirDS;
	CScanDevice* poDevice;

	poDevice = NULL;

	oTrace->StartTrace(__WFUNCTION__);
	
	//Enumerata through the twain devices and return the first.
	EnableDSM();

	twRC=(*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
							MSG_GETFIRST, &dirDS);

	if ((twRC==TWRC_SUCCESS) && (strncmp(dirDS.ProductName, "WIA-", 4)!=0)) {

		poDevice = new CScanDevice(oGlobalInstances);
		poDevice->cScanningDevice = CA2W(dirDS.ProductName);
		poDevice->cScanningDeviceID = CA2W(dirDS.ProductName);
		poDevice->cInterface = eTWAIN;
		}
	else {

		poDevice = GetNextDevice();
		}

	oTrace->EndTrace(__WFUNCTION__, poDevice!=NULL);

	return poDevice;
	}

CScanDevice* 
CScanTwain::GetNextDevice() {

	TW_UINT16 twRC;
	TW_IDENTITY dirDS;
	CScanDevice* poDevice;

	oTrace->StartTrace(__WFUNCTION__);
	
	poDevice = NULL;
	
	twRC=(*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
					MSG_GETNEXT, &dirDS);

	if ((twRC!=TWRC_ENDOFLIST) && (twRC!=TWRC_FAILURE) 
		 && (strncmp(dirDS.ProductName, "WIA-", 4)!=0)) {

		poDevice = new CScanDevice(oGlobalInstances);
		poDevice->cScanningDevice = CA2W(dirDS.ProductName);
		poDevice->cScanningDeviceID = CA2W(dirDS.ProductName);
		poDevice->cInterface = eTWAIN;
		}
	else {
		DisableDSM();
		}

	oTrace->EndTrace(__WFUNCTION__, poDevice!=NULL);

	return poDevice;
	}

/*void CScanTwain::CmSelectSource()
{
	oTrace->StartTrace(__WFUNCTION__);

   TW_UINT16 twRC;
   TW_IDENTITY NewDSIdentity;
   TW_IDENTITY dirDS;

	char cDefaultScanner[MAX_PATH];
	char cTempScannerName[MAX_PATH];
	ScanStruct sScanStruct;

	if (WiaAvailable()) {

		GetDefaultScannerID(cDefaultScanner, sizeof(cDefaultScanner));

		if (EnableWia()) {

			::ZeroMemory(&sScanStruct, sizeof(ScanStruct));
			sScanStruct.sSize = sizeof(ScanStruct);

			//is er een default scanner gekozen? Zo ja, deze plaatsen in de ScanStruct
			ReadRegistryString("General","DefaultScanner","-", cTempScannerName, sizeof(cTempScannerName));

			if (strcmp(cTempScannerName, "-")!=0) {
				strcpy(sScanStruct.cScanningDevice, cTempScannerName);

				ReadRegistryString("General","DefaultScannerID","-", cTempScannerName, sizeof(cTempScannerName));

				if (strcmp(cTempScannerName, "-")!=0) {
					strcpy(sScanStruct.cScanningDeviceID, cTempScannerName);
					}
				}
			else {
				strcpy(sScanStruct.cScanningDevice, cDefaultScanner+4);
				}

			//vraag dialoogvenster op.
			oWia->CreateChooseScannerDlg(sScanStruct);

			//voeg TWAIN devices toe aan dialoogvenster
			EnableDSM();

			twRC=(*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
									MSG_GETFIRST, &dirDS);

			if (twRC!=TWRC_FAILURE)
				{
				do {
					if (strncmp(dirDS.ProductName, "WIA-", 4)!=0) {
						oWia->AddTWAINToChooseScannerDlg(dirDS.ProductName);
						}

					twRC=(*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
									MSG_GETNEXT, &dirDS);
					} while ((twRC!=TWRC_ENDOFLIST) && (twRC!=TWRC_FAILURE));
				}

			DisableDSM();

         //Toon dialoog venster
			oWia->ExecuteChooseScannerDlg();

			DisableWia();
			InitSet=false;
			}
		}
	else {

		EnableDSM();

		// I will settle for the system default.  Shouldn't I get a highlight
		// on system default without this call?
		(*lpDSM_Entry)(&appID,
									  NULL,
									  DG_CONTROL,
									  DAT_IDENTITY,
									  MSG_GETDEFAULT,
									  (TW_MEMREF)&NewDSIdentity);

		 // This call performs one important function:
		 // - should cause SM to put up dialog box of available Source's
	    // - tells the SM which application, appID.id, is requesting, REQUIRED
	    // - returns the SM assigned NewDSIdentity.id field, you check if changed
	    //  (needed to talk to a particular Data Source)
	    // - be sure to test return code, failure indicates SM did not close !!
	    //
	    twRC = (*lpDSM_Entry)(&appID,
	                          NULL,
	                          DG_CONTROL,
	                          DAT_IDENTITY,
	                          MSG_USERSELECT,
	                          (TW_MEMREF)&NewDSIdentity);
	
		// Check if the user changed the Source and react as apporpriate.
	    // - TWRC_SUCCESS, log in new Source
	    // - TWRC_CANCEL,  keep the current Source
	    // - default,      check down the codes in a status message, display result
	    //

		if (twRC==TWRC_SUCCESS)
			{
			InitSet=false;
			dsID = NewDSIdentity;
			WriteRegistryString("General","DefaultScanner",NewDSIdentity.ProductName);

			if (strncmp(NewDSIdentity.ProductName, "WIA-", 4)==0) {
				WriteRegistryString("General","DefaultScanner",NewDSIdentity.ProductName+4);
				}

			WriteRegistryString("General","DefaultScannerID","-"); //Geen id, is geen standard WIA scanner
			}
		DisableDSM();
		}

	ReadRegistryString("General","DefaultScanner","-", oLastScanDevice.cScanningDevice, sizeof(oLastScanDevice.cScanningDevice));
	ReadRegistryString("General","DefaultScannerID","-", oLastScanDevice.cScanningDeviceID, sizeof(oLastScanDevice.cScanningDeviceID));

	oTrace->EndTrace(__WFUNCTION__);
}*/
/*
void CScanTwain::CmSelectSource()
{
	TW_IDENTITY dirDS[17];
	TW_UINT16     twRC;
	int n=0;

	EnableDSM();
	SelSourceStruct.DataSources.Clear();

	twRC=(*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
									MSG_GETFIRST, &dirDS[n]);

	if (twRC!=TWRC_FAILURE)
		{
		do
			{
			SelSourceStruct.DataSources.AddString(dirDS[n].ProductName);

			n++;
			twRC=(*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
									MSG_GETNEXT, &dirDS[n]);
			} while ((twRC!=TWRC_ENDOFLIST) && (twRC!=TWRC_FAILURE) && (n<16));

		// Get the selected source
		(*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
								MSG_GETDEFAULT, &dirDS[16]);

		SelSourceStruct.DataSources.SelectString(dirDS[16].ProductName);

		if (TSelSourceDialog(WindowHandler, SelSourceStruct, Resource).Execute()==IDOK)
			{
			dsID=dirDS[SelSourceStruct.DataSources.GetSelIndex()];
			InitSet=false;
			}
		}
	DisableDSM();
}
*/
/*bool
CScanTwain::WiaAvailable() {

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


bool
CScanTwain::UseWia() {

   oTrace->StartTrace(__WFUNCTION__);

	bool retval = false;

	//als de ID van de scanning device is ingevuld, en wia is bruikbaar dan gaan we die gebruiken. Anders via
	//de reguliere TWAIN interface

   if (WiaAvailable()) {

		if (strlen(oLastScanDevice.cScanningDeviceID)>0 && (strcmp(oLastScanDevice.cScanningDeviceID, "-")!=0)) {
			retval = true;
			}
	}

   oTrace->EndTrace(__WFUNCTION__, retval);

	return retval;
	}*/

//bool
//CScanTwain::PagesAvailable() {
//
//   oTrace->StartTrace(__WFUNCTION__);
//   oTrace->EndTrace(__WFUNCTION__, bPagesAvailable);
//
//   return bPagesAvailable;
//	}

void
CScanTwain::InitScannerSet()
{
   bool bUseTwain;

   oTrace->StartTrace(__WFUNCTION__);

   bUseTwain = true;

	if (!InitSet && oLastScanDevice!=NULL)
		{
			//bepaal welke init moet worden aangesproken. De init van TWAIN
			//of de init van WIA
			//gekozen source staat in dsID

			DeviceHasFeeder=false;
			bPagesAvailable = false;

         /*if (UseWia()) {

				if (EnableWia()) {

					//init structure voor WIA capabilities
					::ZeroMemory(&sWiaCapabilityStruct, sizeof(ScanStruct));
					sWiaCapabilityStruct.sSize = sizeof(ScanStruct);

					if (oWia) {

						//devicenaam bepalen huidige device
						strcpy(sWiaCapabilityStruct.cScanningDevice, oLastScanDevice.cScanningDevice);
						strcpy(sWiaCapabilityStruct.cScanningDeviceID, oLastScanDevice.cScanningDeviceID);

						//capabilities opvragen
						if (oWia->GetDeviceCapabilities(sWiaCapabilityStruct)) {

                     //settings kopieren naar intern instelling structure
                     //Resolutie
                     SettingStruct->Depth.PosResolution=sWiaCapabilityStruct.bResolution;

                     SettingStruct->Depth.ResolutionData.Clear();
                     if (sWiaCapabilityStruct.bResolution75) SettingStruct->Depth.ResolutionData.AddString("75");
                     if (sWiaCapabilityStruct.bResolution100) SettingStruct->Depth.ResolutionData.AddString("100");
                     if (sWiaCapabilityStruct.bResolution150) SettingStruct->Depth.ResolutionData.AddString("150");
                     if (sWiaCapabilityStruct.bResolution200) SettingStruct->Depth.ResolutionData.AddString("200");
                     if (sWiaCapabilityStruct.bResolution300) SettingStruct->Depth.ResolutionData.AddString("300");
                     if (sWiaCapabilityStruct.bResolution400) SettingStruct->Depth.ResolutionData.AddString("400");
                     if (sWiaCapabilityStruct.bResolution600) SettingStruct->Depth.ResolutionData.AddString("600");
                     if (sWiaCapabilityStruct.bResolution1200) SettingStruct->Depth.ResolutionData.AddString("1200");

                     //Brightness
                     SettingStruct->Balance.PosBrightness=sWiaCapabilityStruct.bBrightness;

                     SettingStruct->Balance.MinBright=sWiaCapabilityStruct.iBrightnessMin;
                     SettingStruct->Balance.MaxBright=sWiaCapabilityStruct.iBrightnessMax;

                     //Contrast
                     SettingStruct->Balance.PosContrast=sWiaCapabilityStruct.bContrast;

                     SettingStruct->Balance.MinContrast=sWiaCapabilityStruct.iContrastMin;
                     SettingStruct->Balance.MaxContrast=sWiaCapabilityStruct.iContrastMax;

                     //Color
                     SettingStruct->Depth.PosColor=sWiaCapabilityStruct.bColorDepth;

                     char BW[20], CLR[20], GRY[20];
                     Resource->LoadString(IDS_BLACKWHITENAME, BW, sizeof(BW));
                     Resource->LoadString(IDS_GRAYSCALENAME, GRY, sizeof(GRY));
                     Resource->LoadString(IDS_COLOURNAME, CLR, sizeof(CLR));
                     SettingStruct->Depth.ColorData.Clear();

                     if (sWiaCapabilityStruct.bBlackWhite) SettingStruct->Depth.ColorData.AddString(BW);
                     if (sWiaCapabilityStruct.bGray) SettingStruct->Depth.ColorData.AddString(GRY);
                     if (sWiaCapabilityStruct.bColor) SettingStruct->Depth.ColorData.AddString(CLR);

                     //Page
                     SettingStruct->Page.PosPage=sWiaCapabilityStruct.bPage;

                     //Orientation
                     SettingStruct->Page.PosOrientation=sWiaCapabilityStruct.bOrientation;


                     UIControllable=sWiaCapabilityStruct.bShowInterface;

                     //Wia uitschakelen en DLL vrijgeven: we got what we came for.
                     DisableWia();

                     bUseTwain = false;
							InitSet = true;
                     }
						}
					}
				}*/

			if (bUseTwain) {

				EnableDSM();
				OpenDS();


			if ((TWDSMOpen) && (TWDSOpen))
				{
				NegotiatePixelFlavor();            
				NegotiateDuplex();  

				oLastScanDevice->bResolution =	NegotiateResolution();
				oLastScanDevice->bBrightness =	NegotiateBrightness();
				oLastScanDevice->bContrast =	NegotiateContrast();
				oLastScanDevice->bColorDepth =	NegotiateColor();
				oLastScanDevice->bPage =		NegotiatePage();
				oLastScanDevice->bOrientation =	NegotiateOrientation();

				NegotiateUIControllable();

				CloseDS();
				DisableDSM();
				}

			InitSet=true;
         }
      }

   oTrace->EndTrace(__WFUNCTION__);
}


/*void CScanTwain::CmScannerSettings(int Page)
{
	oTrace->StartTrace(__WFUNCTION__);

   InitScannerSet();


   char Buffer[50];
   Resource->LoadString(IDS_SCANNERSETTING, Buffer, sizeof(Buffer));

  	TPropertySheet* ps = new TMyPropertySheet(WindowHandler, Buffer , Page, false, PSH_NOAPPLYNOW);

  	TTwainPageDialog* TPD= new TTwainPageDialog(ps, &SettingStruct->Page, Resource);
  	TTwainBalanceDialog* TBD= new TTwainBalanceDialog(ps, &SettingStruct->Balance, Resource);
  	TTwainDepthDialog* TDD= new TTwainDepthDialog(ps, &SettingStruct->Depth, Resource);

   ps->SelectPage(Page);
  	if (ps->Execute()==IDOK)
  		{
      TPD->WriteSettings(&SettingStruct->Page);
      TBD->WriteSettings(&SettingStruct->Balance);
      TDD->WriteSettings(&SettingStruct->Depth);
      }

   oTrace->EndTrace(__WFUNCTION__);
}*/

int CScanTwain::GetScannedDPI()
{
	oTrace->StartTrace(__WFUNCTION__);

	int iResolution=-1;

	oTrace->Add(L"ScanResolution", ScanResolution);
	oTrace->Add(L"CurrentResolution", CurrentResolution);
	oTrace->Add(L"ResolutionFromImage", ResolutionFromImage);
	oTrace->Add(L"CalculatedResolutionFromImage", CalculatedResolutionFromImage);

	if (ScanResolution == ResolutionFromImage) iResolution = ScanResolution;
	if (iResolution==-1 && ScanResolution == CurrentResolution) iResolution = CurrentResolution;
	if (iResolution==-1 && ResolutionFromImage == CurrentResolution) iResolution = CurrentResolution;
	if (iResolution==-1) iResolution = ScanResolution;

	if (iResolution==-1) {

		iResolution = ResolutionFromImage;
		if (iResolution==0 || iResolution<25 || iResolution>1500) iResolution=CalculatedResolutionFromImage;
		}

	oTrace->EndTrace(__WFUNCTION__, iResolution);

	return iResolution;
}

/*bool
CScanTwain::DetermineDefaultScanner() {

	//haal de default scanner op en bepaal of het een WIA is, zo niet open dan
   //het dialoogvenster om een WIA scanner uit te kiezen (mits WIA mogelijk is, en
   //mits er een WIA scanner geinstalleerd is).

   oTrace->StartTrace(__WFUNCTION__);

   bool bRetval = false;
	char cDefaultScanner[MAX_PATH];
	char cTempScannerName[MAX_PATH];
	ScanStruct sScanStruct;

	GetDefaultScannerID(cDefaultScanner, sizeof(cDefaultScanner));

	if (WiaAvailable()) {

		if (EnableWia()) {

			::ZeroMemory(&sScanStruct, sizeof(ScanStruct));
			sScanStruct.sSize = sizeof(ScanStruct);

			//is er een default scanner gekozen? Zo ja, deze plaatsen in de ScanStruct
			ReadRegistryString("General","DefaultScanner","-", cTempScannerName, sizeof(cTempScannerName));

			oTrace->StartTrace("DefaultWiaScanner");
			oTrace->EndTrace("DefaultWiaScanner", cTempScannerName);

			if (strcmp(cTempScannerName, "-")!=0) {
				strcpy(sScanStruct.cScanningDevice, cTempScannerName);

				ReadRegistryString("General","DefaultScannerID","-", cTempScannerName, sizeof(cTempScannerName));

				if (strcmp(cTempScannerName, "-")!=0) {
					strcpy(sScanStruct.cScanningDeviceID, cTempScannerName);
					}
				}
			else {
				strcpy(sScanStruct.cScanningDevice, cDefaultScanner+4);
				}

			oTrace->Add(L"sScanStruct.cScanningDevice: ", sScanStruct.cScanningDevice);
			oTrace->Add(L"sScanStruct.cScanningDeviceID: ", sScanStruct.cScanningDeviceID);

			//vraag dialoogvenster op.
			if (oWia->AskDefaultScanner(sScanStruct)) {
         	//er is een andere scanner gekozen, leeg cache.
				WriteRegistryString("Cache","ScannerName","leeg");
         	strcpy(oLastScanDevice.cScanningDevice, "");
         	strcpy(oLastScanDevice.cScanningDeviceID, "");
            bRetval = true;
            }

			DisableWia();
			}
		}

   oTrace->EndTrace(__WFUNCTION__, bRetval);

   return bRetval;
	}*/

bool
CScanTwain::GetDefaultScannerID(char* cDeviceID, int iMaxLen) {

   oTrace->StartTrace(__WFUNCTION__);

	//open en sluit de twain DS zodat de identity wordt ingevuld met de
   //default scanner vanuit de Twain interface.
	bool bOpenedDSM = false;
	TW_IDENTITY NewDSIdentity;
	TW_UINT16     twRC;

	//default to no default scanner
	strcpy_s(cDeviceID, iMaxLen, "???");
	bDefaultScannerAvailable = false;

   if (!TWDSMOpen) {
		bOpenedDSM = EnableDSM();
		}

	if (TWDSMOpen) {
		
		oTrace->StartTrace(L"twRC - MSG_GETDEFAULT");

		::ZeroMemory(&NewDSIdentity, sizeof(TW_IDENTITY));

		twRC =(*lpDSM_Entry)(&appID,    NULL,
	                          DG_CONTROL,
	                          DAT_IDENTITY,
	                          MSG_GETDEFAULT,
	                          (TW_MEMREF)&NewDSIdentity);
		oTrace->EndTrace(L"twRC - MSG_GETDEFAULT");

		if (twRC==TWRC_SUCCESS) {
			strncpy_s(cDeviceID, iMaxLen, NewDSIdentity.ProductName, iMaxLen);
      		bDefaultScannerAvailable=true;
      		}

   		if (bOpenedDSM) {
   			DisableDSM();
      		}
		}
      
	oTrace->EndTrace(__WFUNCTION__/*, cDeviceID*/);

	return (twRC==TWRC_SUCCESS);
}


/*bool
CScanTwain::EnableWia() {

   oTrace->StartTrace(__WFUNCTION__);

	bool retval = false;

	typedef WiaClass* (*ObjMaker)();

	ObjMaker pfuncMaker = 0;


	if (!WiaOpen) {
		//if (SUCCEEDED(CoInitialize(NULL))) {

		hWiaDLL = LoadLibrary("cm_wia.dll");  // load the dll
		if (hWiaDLL!=NULL) {pfuncMaker = (ObjMaker)(::GetProcAddress( hWiaDLL, "_MakeWiaClassInstance" ));}

		if (pfuncMaker==NULL || hWiaDLL==NULL ) {
			char* lpMsgBuf;

			::FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL
				);

			// Display the string.
			::MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

			// Free the buffer.
			LocalFree( lpMsgBuf );
			return false;
			}

		oWia = pfuncMaker();

		if (oWia!=NULL) {
			retval = true;
			WiaOpen = true;
         oWia->hResource = Resource->GetInstance();
			}
	  //	}
	}

   oTrace->EndTrace(__WFUNCTION__, retval);

   return retval;
}

bool CScanTwain::DisableWia() {

	oTrace->StartTrace(__WFUNCTION__);

	if (WiaOpen) {

		oWia->Release();
		oWia = NULL;
		if (!FreeLibrary(hWiaDLL)) {
			char* lpMsgBuf;

			::FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL
				);

			// Display the string.
			::MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

			// Free the buffer.
			LocalFree( lpMsgBuf );
			}
		//CoUninitialize();

		WiaOpen = false;
	}

	oTrace->EndTrace(__WFUNCTION__, WiaOpen);

  	return true;
}*/

bool CScanTwain::EnableDSM()
{
	TW_UINT16     twRC;
	WCHAR DSMName[MAX_PATH];

   oTrace->StartTrace(__WFUNCTION__);

   hDSMDLL = NULL;

	if (!TWDSMOpen) {

		wcscpy_s(DSMName, MAX_PATH, L"TWAIN_32.DLL");

		hDSMDLL = (HANDLE)::LoadLibrary(DSMName);

		if (hDSMDLL==NULL) {

			TCHAR szBuf[80]; 
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError(); 

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, NULL );

			wsprintf(szBuf, L"failed with error %d: %s", dw, lpMsgBuf); 
 
			MessageBox(NULL, szBuf, L"Error", MB_OK); 

			LocalFree(lpMsgBuf);
			}

		if (hDSMDLL != NULL &&
			(lpDSM_Entry = (DSMENTRYPROC) GetProcAddress((HINSTANCE__ *)hDSMDLL, "DSM_Entry"))) {
         
			 twRC = (*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_PARENT,
         							MSG_OPENDSM, (TW_MEMREF)&oSettings->hWindow);

			  if (twRC==TWRC_SUCCESS) TWDSMOpen=true;
         else
         	{
            oTrace->Add(L"Opening Data Source", twRC);
            }
        	}
      else {
   		oTrace->Add(L"No Twain_32 DLL found/no connection possible");
         ::MessageBox(NULL, L"No Twain_32.dll found, scanning is disabled", L"Error", MB_OK);
      	}
    	}

   oTrace->EndTrace(__WFUNCTION__, TWDSMOpen);

   return TWDSMOpen;
}

void CScanTwain::DisableDSM()
{
   TW_UINT16     twRC;

	oTrace->StartTrace(__WFUNCTION__);

	if ((TWDSMOpen) && (!TWDSOpen))
   	{
     	twRC = (*lpDSM_Entry)(&appID, NULL, DG_CONTROL,
      			DAT_PARENT, MSG_CLOSEDSM, &oSettings->hWindow);

      if (twRC == TWRC_SUCCESS)
			{
        	TWDSMOpen = false;

        	if (hDSMDLL)
     			{
		      FreeLibrary ((HINSTANCE)hDSMDLL);
		      hDSMDLL=NULL;
            dsID.Id = 0;
		      }
         }
      else
      	{
         oTrace->Add(L"Disabling Data Source Manager", twRC);
         }
      }

	oTrace->EndTrace(__WFUNCTION__);
}

bool CScanTwain::DSMLoaded()
{
	return TWDSMOpen;
}

bool CScanTwain::OpenDS()
{
   TW_UINT16     twRC;
   std::string	sLastScanDevice;

   oTrace->StartTrace(__WFUNCTION__);

  	if ((TWDSMOpen) && (!TWDSOpen) && oLastScanDevice!=NULL)
		{
		ZeroMemory(&dsID, sizeof(dsID));

		if (oLastScanDevice->cScanningDevice.length()>1) {
			//er is om een specifiek device gevraagd, probeer deze te openen
			
			size_t origsize = oLastScanDevice->cScanningDevice.length() + 1;
			size_t convertedChars = 0;
			wcstombs_s(&convertedChars, dsID.ProductName, origsize, oLastScanDevice->cScanningDevice.c_str(), _TRUNCATE);
			sLastScanDevice = dsID.ProductName;
			//strcpy(dsID.ProductName, oLastScanDevice.cScanningDevice.c_str());

			twRC = (*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
									MSG_GETFIRST, &dsID);

			if (twRC==TWRC_SUCCESS)
				{
				bool bFound = false;

				while (twRC==TWRC_SUCCESS && !bFound) {

					bFound = (sLastScanDevice.compare(dsID.ProductName)==0);

					if (!bFound) {
						twRC = (*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
									MSG_GETNEXT, &dsID);
//						::MessageBox(NULL, dsID.ProductName, oLastScanDevice.cScanningDevice, MB_OK);
						}
					else {
//						::MessageBox(NULL, "Searched for", oLastScanDevice.cScanningDevice, MB_OK);
//						::MessageBox(NULL, "Found", dsID.ProductName, MB_OK);
						}
					}
				}
			}

		twRC = (*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
									MSG_OPENDS, &dsID);


		if (twRC==TWRC_SUCCESS)
			{
			TWDSOpen=true;
         }
      else
      	{
		std::wstring sMessage;

		TranslateReturnStatus(twRC, sMessage);
		oTrace->Add(L"Opening Data Source", sMessage);
         TWDSOpen=false;
         }
      }

   oTrace->EndTrace(__WFUNCTION__, TWDSOpen);

   return TWDSOpen;
}

void CScanTwain::DisableDS()
{
	TW_UINT16     twRC;
	TW_USERINTERFACE  twUI;

   oTrace->StartTrace(__WFUNCTION__);

		//Disable Data Source
		twUI.hParent = oSettings->hWindow;
   	twUI.ShowUI  = TWON_DONTCARE8;
   	twRC = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_USERINTERFACE,
   								MSG_DISABLEDS, (TW_MEMREF)&twUI);

		if (twRC!=TWRC_SUCCESS) {
        	
			std::wstring sMessage;

			TranslateReturnStatus(twRC, sMessage);
			oTrace->Add(L"Disabling Data Source", sMessage);
	         TWDSMOpen=false;
		     }

   oTrace->EndTrace(__WFUNCTION__);
   }

void CScanTwain::CloseDS()
{
	TW_UINT16     twRC;

   oTrace->StartTrace(__WFUNCTION__);

   if (TWDSOpen)
   	{
		//Close Data Source
     	twRC = (*lpDSM_Entry)(&appID, NULL, DG_CONTROL, DAT_IDENTITY,
                            MSG_CLOSEDS, &dsID);

		if (twRC==TWRC_SUCCESS)
        	{
      	TWDSOpen=false;
        	}
      else
      	{
		std::wstring sMessage;

		TranslateReturnStatus(twRC, sMessage);
		oTrace->Add(L"Closing Data Source", sMessage);
		TWDSMOpen=false;
         }
      }

   oTrace->EndTrace(__WFUNCTION__);
}

//This function retrieves the device its capabilities and stores them in the poSettings
bool	
CScanTwain::GetDeviceCapabilities(CScanSettings* poSettings) {

	CScanDevice* oScanDevice;
	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	oScanDevice = poSettings->GetScanner();
	
	if (oScanDevice!=NULL) {

		if (!oScanDevice->bCapabilitiesRetrieved) {

			//the two settings instances aren't the same. We have to fill the poSettings with
			//the latest capabilities by detecting them again.

			oSettings = poSettings;
			InitSet = false;
			oLastScanDevice=oScanDevice;

			InitScannerSet();

			oScanDevice->bCapabilitiesRetrieved = InitSet;
			}
		}

	bResult = (oScanDevice!=NULL && InitSet);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This is the most important function, here is where it all begins; acquiring images!
eAcquireResult
CScanTwain::CmAcquire(CScanSettings* poSettings) {

	TW_UINT16     twRC;
	std::wstring sMessage;
	eAcquireResult eReturn; 
	bool bShowDeviceInterface;
	CScanDevice* oScanDevice;

	//Test if the input is valid
	if (poSettings==NULL) {

		oTrace->Add(L"poSettings", L"NULL");
		return eAcquireFail;
		}

	oScanDevice = poSettings->GetScanner();
	
	if (oScanDevice==NULL) {
		
		oTrace->Add(L"oScanDevice", L"NULL");
		return eAcquireFail;
		};
	
	oTrace->StartTrace(__WFUNCTION__);

	oTrace->Add(L"cScanningDevice", oScanDevice->cScanningDevice);
	oTrace->Add(L"cScanningDeviceID", oScanDevice->cScanningDeviceID);

	eReturn = eAcquireOk;
	oSettings = poSettings;
	oErrorMessages.clear();

	bShowDeviceInterface = oSettings->GetBool(eShowInterface) || oSettings->GetBool(eAlwaysShowInterface);
	bLastScanWithInterface = oSettings->GetBool(eShowInterface);

	if (bPagesAvailable) {

		//There are  pages ready to be acquired (i.e. in a feeder). Now
		//acquire one of those pages.

   		EnableDSM();
		OpenDS();

		TW_STATUS twStatus;
		(*lpDSM_Entry)(&appID, &dsID, DG_CONTROL,DAT_STATUS,MSG_GET, (TW_MEMREF)&twStatus);

		CloseDS();
		DisableDSM();

		if (twStatus.ConditionCode==TWCC_LOWMEMORY) {
			eReturn = eAcquireLowMemory;
			}
   		}
   else {

      if (oLastScanDevice!=oScanDevice) {

		// Another scanning device will be used for the next scan action, make sure
		// all the scanning settings will be reset before scanning commences

         InitSet = false;
         }

      oLastScanDevice = oScanDevice;

      if (!bShowDeviceInterface) {
         //if we are using the scanner's interface than we will not set
         //any settings beforehand. This may lead to problems with some
         //scanners.
	      InitScannerSet();
   		}

	  if (!TestMemory()) {
		  
		  AddErrorMessage(IDS_LOWMEMORY, 0, true);
		  }
	  else {

		//oImages leegmaken
   		//oImage.clear();

		bool UseTwain = true;

		/*if (UseWia()) {

			if (EnableWia()) {

				if (oWia) {

					UseTwain = false;

					oTrace->Add(L"Acquiring image through WIA");

                  //show user interface: we don not use this with WIA for now
                  //user can select use device interface that should work just fine.
                  //sWiaCapabilityStruct.bShowInterface = UserInterface;

						//settings instellen
						//Resolutie
						int UseResolution = SettingStruct->Depth.Resolution;
						if (UseResolution==0) UseResolution=300;

						sWiaCapabilityStruct.iResolution = ScanResolution = UseResolution;

						//Kleurdiepte
						sWiaCapabilityStruct.iColorDepth = -1; //default

						switch (SettingStruct->Depth.Color)
							{
							case 3: sWiaCapabilityStruct.iColorDepth=2; break;
							case 2: sWiaCapabilityStruct.iColorDepth=1; break;
							case 1: sWiaCapabilityStruct.iColorDepth=0; break;
							}

						//Brightness
						int Bereik=SettingStruct->Balance.MaxBright-SettingStruct->Balance.MinBright;
						int Value=SettingStruct->Balance.MinBright+(Bereik*(100+atoi(SettingStruct->Balance.Brightness)))/200;

						sWiaCapabilityStruct.iBrightnessValue = Value;

						//Contrast
						Bereik=SettingStruct->Balance.MaxContrast-SettingStruct->Balance.MinContrast;
						Value=SettingStruct->Balance.MinContrast+(Bereik*(100+atoi(SettingStruct->Balance.Contrast)))/200;

						sWiaCapabilityStruct.iContrastValue = Value;

						//Pagina omvang
						sWiaCapabilityStruct.iPage =  -1;
						switch (SettingStruct->Page.Page)
							{
							case 1:
								{
								sWiaCapabilityStruct.iPage =  0;  //a4
								break;
								}
							case 2:
								{
								sWiaCapabilityStruct.iPage =  1; //a5
								break;
								}
							case 3:
								{
								sWiaCapabilityStruct.iPage =  2; //letter
								break;
								}
							case 4:
								{
								sWiaCapabilityStruct.iPage =  3; //legal
								break;
								}
							}

						//Orientatie
						if (SettingStruct->Page.Portrait) sWiaCapabilityStruct.iOrientation = 0;
						if (SettingStruct->Page.Landscape) sWiaCapabilityStruct.iOrientation = 1;

						//Afbeelding binnen halen
						sWiaCapabilityStruct.sSize = sizeof(ScanStruct);
						int iAcquireResult = oWia->AcquireImages(WindowHandler->HWindow, sWiaCapabilityStruct);

						switch (iAcquireResult) {

							case 0: {//success

                        //images overhalen
                        while (oWia->ImagesAvailable()) {

									SetImageAcquired(oWia->GetImage().hImageMemoryHandle);
                           }

								//laat het weten dat er een image klaar staat
								PostMessage(WindowHandler->HWindow, WM_USER, 1, 0);
								break;
								}

							case -1: {//foutmelding
								// Canceled by user
								//oWia->ShowLastError();
								//UseTwain = true;
                        if (oWia->ImagesAvailable()) {
                        	while (oWia->ImagesAvailable()) {

										SetImageAcquired(oWia->GetImage().hImageMemoryHandle);
                           	}

                           //laat het weten dat er een (misschien) een image klaar staat
                           PostMessage(WindowHandler->HWindow, WM_USER, 1, 0);
                           }
								break;
								}

							case -2: {//probeer TWAIN
								UseTwain = true;
								break;
								}

							}

						WriteDebugStringToFile("End Acquiring image through WIA:\n");

						//Wia uitschakelen en DLL vrijgeven: we got what we came for.
						DisableWia();
					}
				}
         }*/


		if (UseTwain) {

			EnableDSM();
			OpenDS();

			if ((TWDSMOpen) && (TWDSOpen)) {

				//bepaal welke init moet worden aangesproken. De init van TWAIN
				//of de init van WIA
				//gekozen source staat in dsID

				if (!bShowDeviceInterface) {

					if (oLastScanDevice->bPage && oSettings->GetBool(ePageUse) && oSettings->GetInt(ePageValue)!=-1) {
						SetMeasure();
						SetPage();
						}

					if (oLastScanDevice->bOrientation && oSettings->GetBool(eOrientationUse)) {
						 SetOrientation();
						}

					if (oLastScanDevice->bBrightness, oSettings->GetBool(eBrightnessUse)) {
						SetBrightness();
						}
               
					if (oLastScanDevice->bContrast && oSettings->GetBool(eContrastUse)) {
						SetContrast();
						}

					if (oLastScanDevice->bColorDepth && oSettings->GetBool(eColorDepthUse)) {
						SetColor();
						}

					ScanResolution=-1;
					if (oLastScanDevice->bResolution && oSettings->GetBool(eResolutionUse)) {
						SetResolution();
						}
					}

    			//SetDuplex();

    			// Enable user interface
    			twUI.hParent = (TW_HANDLE)oSettings->hWindow;
    			twUI.ShowUI  = UIControllable ? bShowDeviceInterface : true;
    			twRC = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_USERINTERFACE,
                               MSG_ENABLEDS, (TW_MEMREF)&twUI);

				TranslateReturnStatus(twRC, sMessage);
    			oTrace->Add(L"Acquiring image", sMessage);

 				if (twRC==TWRC_FAILURE) {

					TW_STATUS twStatus;
					(*lpDSM_Entry)(&appID, &dsID, DG_CONTROL,DAT_STATUS,MSG_GET, (TW_MEMREF)&twStatus);

					if (twStatus.ConditionCode==TWCC_LOWMEMORY) {

                  		if (CheckMessage()) AddErrorMessage(IDS_LOWMEMORY, 0, true);
                  		}
               		}
				}
			else
				{
				oTrace->Add(L"Result Acquiring image", "Error: Data Source or Data Source Manager not open");
				}
			}
		}
	}

   

	if (GetCriticalErrorMessageCount()>0) eReturn = eAcquireFailSeeMessages;

	oTrace->EndTrace(__WFUNCTION__, (int)eReturn);

   	return eReturn;
	}

bool 
CScanTwain::TranslateReturnStatus(TW_UINT16 status, std::wstring &sErrorMessage) {

	TW_STATUS twStatus;
	bool bReturn=false;

	switch (status)
  		{
		case TWRC_SUCCESS: sErrorMessage = L"TWRC_SUCCESS!"; bReturn=true; break;
		case TWRC_FAILURE: sErrorMessage = L"TWRC_FAILURE"; break;
		case TWRC_CHECKSTATUS: sErrorMessage = L"TWRC_CHECKSTATUS"; break;
		case TWRC_CANCEL: sErrorMessage = L"TWRC_CANCEL"; break;
		case TWRC_DSEVENT: sErrorMessage = L"TWRC_DSEVENT"; break;
		case TWRC_NOTDSEVENT: sErrorMessage = L"TWRC_NOTDSEVENT"; break;
		case TWRC_XFERDONE: sErrorMessage = L"TWRC_XFERDONE"; break;
		case TWRC_ENDOFLIST: sErrorMessage = L"TWRC_ENDOFLIST"; break;
		case TWRC_INFONOTSUPPORTED: sErrorMessage = L"TWRC_INFONOTSUPPORTED"; break;
		case TWRC_DATANOTAVAILABLE: sErrorMessage = L"TWRC_DATANOTAVAILABLE"; break;
		
		}

      if (status == TWRC_FAILURE)
      	{
         (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL,DAT_STATUS,MSG_GET, (TW_MEMREF)&twStatus);

   		switch (twStatus.ConditionCode)
   			{
      		case TWCC_SUCCESS: sErrorMessage+=L"It worked!";
            	{
               bReturn=true;
               break;
               }
      		case TWCC_BUMMER: sErrorMessage+=L"Failure due to unknown causes"; break;
      		case TWCC_LOWMEMORY: sErrorMessage+=L"Not enough memory to perform operation"; break;
      		case TWCC_MAXCONNECTIONS: sErrorMessage+=L"DS is connected to max possible apps"; break;
      		case TWCC_OPERATIONERROR: sErrorMessage+=L"DS or DSM reported error, app shouldn't"; break;
      		case TWCC_BADCAP: sErrorMessage+=L"Unknown capability"; break;
      		case TWCC_BADPROTOCOL: sErrorMessage+=L"Unrecognized MSG DG DAT combination"; break;
      		case TWCC_BADVALUE: sErrorMessage+=L"Data parameter out of range"; break;
      		case TWCC_SEQERROR: sErrorMessage+=L"DG DAT MSG out of expected sequence"; break;
      		case TWCC_BADDEST: sErrorMessage+=L"Unknown destination App/Src in DSM_Entry"; break;
      		case TWCC_CAPUNSUPPORTED: sErrorMessage+=L"Capability not supported by source"; break;
      		case TWCC_CAPBADOPERATION: sErrorMessage+=L"Operation not supported by capability"; break;
      		case TWCC_CAPSEQERROR: sErrorMessage+=L"Capability has dependancy on other capability"; break;
   			default: sErrorMessage+=L"Unknown error status"; break;
      		}
         }

   return bReturn;

}

void CScanTwain::DoTransfer()
{
	 TW_PENDINGXFERS     twPendingXfer;
	 TW_UINT16           twRC;
	 TW_UINT16           twRC2;
	 HGLOBAL             hBitMap;
	 std::wstring sMessage;

    bool bError = false;

    oTrace->StartTrace(__WFUNCTION__);

	 // Do until there are no more pending transfers
	 // explicitly initialize the our flags

	 twPendingXfer.Count = 0;
	 do {

    		oTrace->Add(L"DoTransfer, loop");
		  // Initiate Native Transfer
    		oTrace->Add(L"DoTransfer, imagenativexfer");
			twRC = (*lpDSM_Entry)(&appID, &dsID,
								DG_IMAGE, DAT_IMAGENATIVEXFER,
								MSG_GET, (TW_MEMREF)&hBitMap);

			TranslateReturnStatus(twRC, sMessage);
			oTrace->Add(L"DoTransfer", sMessage);

		  switch (twRC) {

				case TWRC_XFERDONE:  // Session is in State 7
					{
    				oTrace->Add(L"DoTransfer, TWRC_XFERDONE");
					if (hBitMap!=NULL)
						{

						//Acknowledge the end of the transfer
						// and transition to state 6/5
						(*lpDSM_Entry)(&appID, &dsID, DG_CONTROL,
												DAT_PENDINGXFERS, MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

						RetrieveCurrentResolution();

						SetImageAcquired(hBitMap);

						// close the DSM and DS
						if (twPendingXfer.Count <= 0)
							{
							if (((LPBITMAPINFOHEADER)GlobalLock(hbm_acq))!=NULL)
								{
								DisableDS();
								GlobalUnlock(hbm_acq);
								bPagesAvailable = false;
								}
							}
                  else {
                     bPagesAvailable = true;
                  	}

						}
               else {
                  bError = true;
               	}

    				oTrace->Add(L"DoTransfer, TWRC_XFERDONE", twPendingXfer.Count);
					break;
					}

				// the user canceled or wants to rescan the image
				// something wrong, abort the transfer and delete the image
				// pass a null ptr back to App

				case TWRC_CANCEL:   // Session is in State 7
					{
    				oTrace->Add(L"DoTransfer, TWRC_CANCEL");
					 //Source (or User) Canceled Transfer
					 // transistion to state 6/5
					 (*lpDSM_Entry)(&appID,
												&dsID,
												DG_CONTROL,
												DAT_PENDINGXFERS,
												MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

					 // close the DSM and DS
					 if (twPendingXfer.Count <= 0)
						  DisableDS();

					bError = true;

					oTrace->Add(L"DoTransfer, TWRC_CANCEL", twPendingXfer.Count);
					 break;
					 }

				case TWRC_FAILURE:  //Session is in State 6
					 {
					 // Abort the image
					 // Enhancement: Check Condition Code and attempt recovery
                oTrace->Add(L"DoTransfer, TWRC_FAILURE");
					 twRC2 = (*lpDSM_Entry)(&appID,
												&dsID,
												DG_CONTROL,
												DAT_PENDINGXFERS,
												MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

					if (twRC2 != TWRC_SUCCESS)
						{
						TranslateReturnStatus(twRC2, sMessage);
						oTrace->Add(L"Aborting image transfer", sMessage);
						}

					 bError = true;

                // close the DSM and DS
					 if (twPendingXfer.Count <= 0)
						  DisableDS();

                oTrace->Add(L"DoTransfer, TWRC_FAILURE", twPendingXfer.Count);
					 break;
					 }

				default:    //Sources should never return any other RC
					 {
					 	// Abort the image
					 	// Enhancement: Check Condition Code and attempt recovery instead
					oTrace->Add(L"DoTransfer, default");

					twRC2 = (*lpDSM_Entry)(&appID,
												&dsID,
												DG_CONTROL,
												DAT_PENDINGXFERS,
												MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

					if (twRC2 != TWRC_SUCCESS) {
						TranslateReturnStatus(twRC2, sMessage);
						oTrace->Add(L"Aborting image transfer", sMessage);
					  	}

                // close the DSM and DS
                if (twPendingXfer.Count <= 0) DisableDS();

					bError = true;

					oTrace->Add(L"DoTransfer, default", twPendingXfer.Count);
					 break;
					 }
				}

    	oTrace->Add(L"DoTransfer, loop", twPendingXfer.Count);

	 } while (twPendingXfer.Count > 0 && !bError);

	 CloseDS();
	 DisableDSM();

	 if (bError) AddErrorMessage(IDS_ERROROCCURRED, 0, true);


	 PostMessage(oSettings->hWindow, WM_SCANIMAGESRECEIVED, bError ? 1 : 0, 0);

	oTrace->EndTrace(__WFUNCTION__);
    }

void CScanTwain::SetImageAcquired(HGLOBAL hImage) {

	oTrace->StartTrace(__WFUNCTION__);

	hbm_acq = hImage;

	BITMAPINFO far* Info;
	Info = (LPBITMAPINFO)::GlobalLock(hbm_acq);

	//Scan inverted
	bool Invert = (Info->bmiHeader.biClrUsed==2 && oSettings->GetBool(eColorInvert));

	if (Invert)
		{
		char* pColor;
		for (unsigned int i=0; i<Info->bmiHeader.biClrUsed; i++)
			{
			pColor=(char*)Info->bmiColors+i*sizeof(RGBQUAD);
			for (int n=0; n<3; n++)
				{
				*pColor^=255;
				pColor++;
				}
			}
		}

	//Read the resolution from the image;
	int r=0;
	ResolutionFromImage=-1;
	CalculatedResolutionFromImage=-1;
	if (Info->bmiHeader.biSize == sizeof(BITMAPINFOHEADER))
		{
		r = (int)(Info->bmiHeader.biXPelsPerMeter * 2.54 ) / 100;
		if (SetPaperWidth!=0)
			{
			CalculatedResolutionFromImage = (int)(Info->bmiHeader.biWidth / SetPaperWidth);
			}
		}
	::GlobalUnlock(hbm_acq);

	//Compensate for the translation from meter to inch.
	if (r>70 && r<80) r=75;
	if (r>95 && r<105) r=100;
	if (r>145 && r<155) r=150;
	if (r>195 && r<205) r=200;
	if (r>295 && r<305) r=300;
	if (r>395 && r<405) r=400;
	if (r>595 && r<605) r=600;
	ResolutionFromImage=r;

	cImageTransferStruct newImage;

	newImage.hImageHandle = hbm_acq;
	newImage.iDPIX = GetScannedDPI();
	newImage.iDPIY = GetScannedDPI();

	if (oImages.size()<oImages.max_size()) {
		oImages.push_back(newImage);
		}

	oTrace->EndTrace(__WFUNCTION__);
   }

bool
CScanTwain::NegotiateResolution()
{
	oTrace->StartTrace(__WFUNCTION__);

	bool retval=false;
	TW_CAPABILITY	cap;
	TW_INT16	rc;
	vector<int>	iResolutions;

	cap.Cap = ICAP_XRESOLUTION ;
	cap.ConType = TWON_DONTCARE16;
	cap.hContainer = NULL;
	

	//clear the resolution settings
	oLastScanDevice->bResolution75 = false;
	oLastScanDevice->bResolution100 = false;
	oLastScanDevice->bResolution150 = false;
	oLastScanDevice->bResolution200 = false;
	oLastScanDevice->bResolution300 = false;
	oLastScanDevice->bResolution400 = false;
	oLastScanDevice->bResolution600 = false;
	oLastScanDevice->bResolution1200 = false;

	if (oSettings->GetBool(eResolutionNegotiate))
		{       
		rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, MSG_GET, (TW_MEMREF)&cap);

		if (rc == TWRC_SUCCESS)
			{
			int firstReso=-1;
			oLastScanDevice->iResolutionContainerType=(int)cap.ConType;

			switch (cap.ConType)
				{
				case TWON_ONEVALUE:
					{
					oTrace->Add(L"ContainerType", "TWON_ONEVALUE");

					pTW_ONEVALUE pvalOneValue;
					pvalOneValue = (pTW_ONEVALUE)GlobalLock(cap.hContainer);

					int n=(TW_UINT16)GetItem(pvalOneValue->ItemType, pvalOneValue->Item);
					oLastScanDevice->iResolutionItemType = pvalOneValue->ItemType;					
					if (n==75 || n==100 || n==150 || n==200 || n==300 || n==400 || n==600 || n==1200) {

						iResolutions.push_back(n);
                  				retval=true;
               					}
					else {
						
						if (StandardResolution==-1) StandardResolution=n;
						}
      					break;
            				}

				case TWON_ENUMERATION:
					{
					oTrace->Add(L"ContainerType", "TWON_ENUMERATION");

					pTW_ENUMERATION	pvalEnum;
					pvalEnum = (pTW_ENUMERATION)GlobalLock(cap.hContainer);

            				TW_UINT32 i=0;
            				TW_UINT16 n;

					oLastScanDevice->iResolutionItemType = pvalEnum->ItemType;
            				//Add all possible reso's
          				while (i<pvalEnum->NumItems) {

               					n = (TW_UINT16)GetItemFromEnum(i, pvalEnum);
             					if (n==75 || n==100 || n==150 || n==200 || n==300 || n==400 || n==600 || n==1200) {

                     					if (firstReso==-1) firstReso=n;
											iResolutions.push_back(n);
                               				retval=true;
                  							}
									i++;
									}
               				if (pvalEnum->NumItems>0 && firstReso==-1) firstReso=n;

              				//Retrieving the current resolution
            				StandardResolution=(TW_UINT16)GetItemFromEnum(pvalEnum->CurrentIndex, pvalEnum);
							oTrace->Add(L"Current", StandardResolution);
 
					if (oSettings->GetInt(eResolutionValue)<=0) {

						for (long lIndex=0; lIndex<(long)iResolutions.size(); lIndex++) {

							if (iResolutions[lIndex]==StandardResolution) oSettings->SetInt(eResolutionValue, StandardResolution);
							}
						}
					
					if (oSettings->GetInt(eResolutionValue)<=0) {

						oSettings->SetInt(eResolutionValue, firstReso);
      					StandardResolution = firstReso;
   						}
 
					break;
					}

				case TWON_ARRAY:
					{
					oTrace->Add(L"ContainerType", "TWON_ARRAY");

					pTW_ARRAY pvalArray;
					pvalArray = (pTW_ARRAY)GlobalLock(cap.hContainer);

    				TW_UINT32 i=0;
    				TW_UINT16 n;

					oLastScanDevice->iResolutionItemType = pvalArray->ItemType;

            				//Add all possible reso's
          				while (i<pvalArray->NumItems) {

						n = (TW_UINT16)GetItemFromArray(i, pvalArray);
						if (n==75 || n==100 || n==150 || n==200 || n==300 || n==400 || n==600 || n==1200) {

							if (firstReso==-1) firstReso=n;
							iResolutions.push_back(n);
                     					retval=true;
							}
						i++;
						}

              				//Retrieving the current resolution
					StandardResolution=firstReso;
					oTrace->Add(L"Current", StandardResolution);

					if (oSettings->GetInt(eResolutionValue)<=0) {

						for (long lIndex=0; lIndex<(long)iResolutions.size(); lIndex++) {

							if (iResolutions[lIndex]==StandardResolution) oSettings->SetInt(eResolutionValue, StandardResolution);
							}
						}
					
					if (oSettings->GetInt(eResolutionValue)<=0) {

						oSettings->SetInt(eResolutionValue, firstReso);
                  		StandardResolution = firstReso;
               			}
					break;
					}

				case TWON_RANGE:
					{
					oTrace->Add(L"ContainerType", "TWON_RANGE");
					pTW_RANGE pvalRange;
					pvalRange = (pTW_RANGE)GlobalLock(cap.hContainer);

            				TW_UINT32 n=pvalRange->MinValue;

	            			//Workaround for Artec scanners and TW_RANGE bug.
   	         			if ((pvalRange->MaxValue<pvalRange->MinValue) || (pvalRange->StepSize<1)) {
         	      				pvalRange->MaxValue=400;
            	   				pvalRange->MinValue=1;
               					pvalRange->StepSize=1;
               					}

					oLastScanDevice->iResolutionItemType = pvalRange->ItemType;

					//Add all possible reso's
					while ((n<=pvalRange->MaxValue) && (n<=1200)) {
					
						if (n==75 || n==100 || n==150 || n==200 || n==300 || n==400 || n==600 || n==1200) {

							if (firstReso==-1) firstReso=n;
							iResolutions.push_back(n);
							retval=true;
							}
 						n=n+pvalRange->StepSize;
						}

					//Retrieving the current resolution
					StandardResolution=pvalRange->CurrentValue;
					oTrace->Add(L"CurrentResolution", (int)StandardResolution);
				
					if (oSettings->GetInt(eResolutionValue)<=0) {

						for (long lIndex=0; lIndex<(long)iResolutions.size(); lIndex++) {

							if (iResolutions[lIndex]==StandardResolution) oSettings->SetInt(eResolutionValue, StandardResolution);
							}
						}
					
					if (oSettings->GetInt(eResolutionValue)<=0) {

						oSettings->SetInt(eResolutionValue, firstReso);
                  		StandardResolution = firstReso;
               			}

               		break;
					}
				}

			
			//Convert the vector iResolutions to the booleans in the scan settings
			for (long lIndex=0; lIndex<(long)iResolutions.size(); lIndex++) {

				oTrace->Add(L"Resolution", iResolutions[lIndex]);

				switch (iResolutions[lIndex]) {

   					case 75:	oLastScanDevice->bResolution75 = true; break;
   					case 100:	oLastScanDevice->bResolution100 = true; break;
   					case 150:	oLastScanDevice->bResolution150 = true; break;
   					case 200:	oLastScanDevice->bResolution200 = true; break;
   					case 300:	oLastScanDevice->bResolution300 = true; break;
   					case 400:	oLastScanDevice->bResolution400 = true; break;
   					case 600:	oLastScanDevice->bResolution600 = true; break;
   					case 1200:	oLastScanDevice->bResolution1200 = true; break;
					}
				}

			GlobalUnlock(cap.hContainer);
			GlobalFree(cap.hContainer);
      		}
		}
	else
		{
   		oLastScanDevice->bResolution75 = true;
   		oLastScanDevice->bResolution100 = true;
   		oLastScanDevice->bResolution150 = true;
   		oLastScanDevice->bResolution200 = true;
   		oLastScanDevice->bResolution300 = true;
   		oLastScanDevice->bResolution400 = true;
   		oLastScanDevice->bResolution600 = true;
   		oLastScanDevice->bResolution1200 = true;
		oSettings->SetInt(eResolutionValue, 200);	
   		retval=true;
   		}

	oTrace->EndTrace(__WFUNCTION__, retval);

	return retval;
	}

bool CScanTwain::NegotiatePage() {

	bool retval=false;
	TW_CAPABILITY	cap;
	TW_INT16	rc;
	std::wstring sErrorMessage;

	oTrace->StartTrace(__WFUNCTION__);

	if (oSettings->GetBool(ePageNegotiate)) {

		cap.Cap = ICAP_FRAMES;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;

		rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, MSG_GET, (TW_MEMREF)&cap);

		retval = TranslateReturnStatus(rc, sErrorMessage);
		oTrace->Add(L"ICAP_FRAMES", sErrorMessage);
		}

	oTrace->EndTrace(__WFUNCTION__, retval);

	return retval;
	}

bool CScanTwain::RetrieveCurrentResolution()
{
   bool result=false;

   oTrace->StartTrace(__WFUNCTION__);

	TW_CAPABILITY	cap;
	TW_INT16			rc;

	cap.Cap = ICAP_XRESOLUTION ;
	cap.ConType = TWON_DONTCARE16;
	cap.hContainer = NULL;

   rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
					MSG_GETCURRENT, (TW_MEMREF)&cap);

	if (rc == TWRC_SUCCESS)
		{
		switch (cap.ConType)
			{
    		case TWON_ONEVALUE:
         	{
				pTW_ONEVALUE pvalOneValue;
				pvalOneValue = (pTW_ONEVALUE)GlobalLock(cap.hContainer);

				CurrentResolution = (TW_UINT16)GetItem(pvalOneValue->ItemType, pvalOneValue->Item);
				break;
            }

			case TWON_RANGE:
				{
				pTW_RANGE pvalRange;
				pvalRange = (pTW_RANGE)GlobalLock(cap.hContainer);

				CurrentResolution = pvalRange->CurrentValue;
	      		break;
				}

			case TWON_ENUMERATION:
				{
				pTW_ENUMERATION	pvalEnum;
				pvalEnum = (pTW_ENUMERATION)GlobalLock(cap.hContainer);

				CurrentResolution = pvalEnum->ItemList[(pvalEnum->CurrentIndex)*2];
				break;
				}

			case TWON_ARRAY:
				{
				pTW_ARRAY			pvalArray;
				pvalArray = (pTW_ARRAY)GlobalLock(cap.hContainer);

            if (pvalArray->NumItems>0)
            	{
               CurrentResolution = pvalArray->ItemList[0]; //There can be only one
               }
            break;
				}
         }
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
  		result=true;
      }
   else
   	{
      oTrace->Add(L"Error", "RetrieveCurrentResolution");
      }

   oTrace->EndTrace(__WFUNCTION__, result);

   return result;
}

bool CScanTwain::NegotiateOrientation()
{
	TW_CAPABILITY	cap;
	TW_INT16	rc;
	std::wstring 	sErrorMessage;
	bool result=false;

	oTrace->StartTrace(__WFUNCTION__);

	if (oSettings->GetBool(eOrientationNegotiate)) {

		cap.Cap = ICAP_ORIENTATION;
		cap.ConType = TWON_DONTCARE16;
		cap.hContainer = NULL;

		rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
					MSG_GET, (TW_MEMREF)&cap);

		
		result = TranslateReturnStatus(rc, sErrorMessage);
		oTrace->Add(L"ICAP_ORIENTATIPON", sErrorMessage);

		if (result) {

			oLastScanDevice->iOrientationContainerType = cap.ConType;
			switch (cap.ConType)
				{
				case TWON_ONEVALUE:
					{
					pTW_ONEVALUE pvalOneValue;
					pvalOneValue = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
					oLastScanDevice->iOrientationItemType = pvalOneValue->ItemType;
					break;
					}

				case TWON_RANGE:
					{
					pTW_RANGE pvalRange;
					pvalRange = (pTW_RANGE)GlobalLock(cap.hContainer);
					oLastScanDevice->iOrientationItemType = pvalRange->ItemType;
					break;
					}

				case TWON_ENUMERATION:
					{
					pTW_ENUMERATION	pvalEnum;
					pvalEnum = (pTW_ENUMERATION)GlobalLock(cap.hContainer);
					oLastScanDevice->iOrientationItemType = pvalEnum->ItemType;
					break;
					}

				case TWON_ARRAY:
					{
					pTW_ARRAY pvalArray;
					pvalArray = (pTW_ARRAY)GlobalLock(cap.hContainer);
					oLastScanDevice->iOrientationItemType = pvalArray->ItemType;
					break;
					}
				}
			GlobalUnlock(cap.hContainer);
			GlobalFree(cap.hContainer);
			}
		}
	else {

		result=true;
		}

   	oTrace->EndTrace(__WFUNCTION__, result);

   	return result;
	}

int CScanTwain::GetItem(TW_UINT16 Type, TW_UINT32 Value) {

	switch (Type) {

		case TWTY_INT8: return (TW_INT8)Value;
		case TWTY_INT16: return (TW_INT16)Value;
		case TWTY_INT32: return (TW_INT32)Value;
		case TWTY_UINT8: return (TW_UINT8)Value;
		case TWTY_UINT16: return (TW_UINT16)Value;
		case TWTY_UINT32: return (TW_UINT32)Value;
		case TWTY_FIX32:
			{
			pTW_FIX32 pTWFix32;
	 		pTWFix32 = (pTW_FIX32)&(Value);
			return (int)FIX32ToFloat(*pTWFix32);
			}
		}
	return -1;
	}

TW_UINT16 CScanTwain::SetItem(TW_UINT16 Type, pTW_UINT32 Item, int Value) {

	switch (Type) {

		case TWTY_INT8: *Item=(TW_INT8)Value; return TWTY_INT8;
		case TWTY_INT16: *Item=(TW_INT16)Value; return TWTY_INT32;
		case TWTY_INT32: *Item=(TW_INT32)Value; return TWTY_INT32;
		case TWTY_UINT8: *Item=(TW_UINT8)Value; return TWTY_UINT8;
		case TWTY_UINT16: *Item=(TW_UINT16)Value; return TWTY_UINT16;
		case TWTY_UINT32: *Item=(TW_UINT32)Value; return TWTY_UINT32;
		case TWTY_BOOL: *Item=(TW_BOOL)Value; return TWTY_BOOL;
		case TWTY_FIX32:
			{
        		 *(pTW_FIX32)Item=FloatToFix32((float)Value);
         		return TWTY_FIX32;
        		}
     		 default: *Item=(TW_INT32)Value; return TWTY_INT32;
      		}	
	}

bool CScanTwain::NegotiateBrightness()
{
	bool bResult=false;
	std::wstring sErrorMessage;

	oTrace->StartTrace(__WFUNCTION__);

	if (oSettings->GetBool(eBrightnessNegotiate)) {

		TW_CAPABILITY	cap;
		TW_INT16	rc;

		cap.Cap = ICAP_BRIGHTNESS;
		cap.ConType = TWON_RANGE;
		cap.hContainer = NULL;

		rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
					MSG_GET, (TW_MEMREF)&cap);

		bResult = TranslateReturnStatus(rc, sErrorMessage);
		oTrace->Add(L"ICAP_BRIGHTNESS", sErrorMessage);

		if (rc == TWRC_SUCCESS) {

			oLastScanDevice->iBrightnessContainerType = cap.ConType;
			switch (cap.ConType) {

				case TWON_RANGE:
					{
					int MinValue, MaxValue, StepSize, CurrentValue;
					pTW_RANGE pvalRange;
					pvalRange = (pTW_RANGE)GlobalLock(cap.hContainer);

					oLastScanDevice->iBrightnessItemType = (TW_UINT16)pvalRange->ItemType;

					MinValue = GetItem(oLastScanDevice->iBrightnessItemType, pvalRange->MinValue);
					MaxValue = GetItem(oLastScanDevice->iBrightnessItemType, pvalRange->MaxValue);
               		StepSize = GetItem(oLastScanDevice->iBrightnessItemType, pvalRange->StepSize);
					CurrentValue = GetItem(oLastScanDevice->iBrightnessItemType, pvalRange->CurrentValue);

					//Workaround Artec scanner bug.
					if (MaxValue<MinValue)
						{
						MaxValue=1000;
						MinValue=0;
                  				StepSize=1;
						CurrentValue=500;
						}

					oLastScanDevice->iBrightnessMin = MinValue;
					oLastScanDevice->iBrightnessMax = MaxValue;
					oLastScanDevice->iBrightnessStep = StepSize;

					oTrace->Add(L"MinValue", MinValue);
					oTrace->Add(L"MaxValue", MaxValue);
					oTrace->Add(L"StepSize", StepSize);
					oTrace->Add(L"CurrentValue", CurrentValue);

					GlobalUnlock(cap.hContainer);
					GlobalFree(cap.hContainer);

					bResult=true;
               		break;
					}
				default:
					{
					oTrace->Add(L"Error", "Not a RANGE structure returned");
					bResult=false;
               		break;
					}
				}
			}
		}

	oLastScanDevice->bBrightness = bResult;	//it is possible to set the brightness	

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
}

bool CScanTwain::NegotiateContrast()
{
	bool bResult=false;
	std::wstring sErrorMessage;

	oTrace->StartTrace(__WFUNCTION__);

	if (oSettings->GetBool(eContrastNegotiate)) {

		TW_CAPABILITY	cap;
		TW_INT16	rc;

 		cap.Cap = ICAP_CONTRAST;
		cap.ConType = TWON_RANGE;
		cap.hContainer = NULL;
		
		rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                  	MSG_GET, (TW_MEMREF)&cap);

		bResult = TranslateReturnStatus(rc, sErrorMessage);
		oTrace->Add(L"ICAP_CONTRAST", sErrorMessage);

      		if (rc == TWRC_SUCCESS) {

         		oLastScanDevice->iContrastContainerType = cap.ConType;

         		switch (cap.ConType) {
            
				case TWON_RANGE: {

       				int MinValue, MaxValue, CurrentValue, StepSize;
       				pTW_RANGE pvalRange;
       				pvalRange = (pTW_RANGE)GlobalLock(cap.hContainer);

       				oLastScanDevice->iContrastItemType = (TW_UINT16)pvalRange->ItemType;

       				MinValue = GetItem(oLastScanDevice->iContrastItemType, pvalRange->MinValue);
       				MaxValue = GetItem(oLastScanDevice->iContrastItemType, pvalRange->MaxValue);
       				StepSize = GetItem(oLastScanDevice->iContrastItemType, pvalRange->StepSize);
       				CurrentValue = GetItem(oLastScanDevice->iContrastItemType, pvalRange->CurrentValue);

       				//Workaround Artec scanner bug.
       				if (MaxValue<MinValue) {

						MaxValue=1000;
						MinValue=0;
						StepSize=1;
						CurrentValue=500;
						}

					oLastScanDevice->iContrastMin = MinValue;
					oLastScanDevice->iContrastMax = MaxValue;
					oLastScanDevice->iContrastStep = StepSize;

					GlobalUnlock(cap.hContainer);
					GlobalFree(cap.hContainer);

					bResult=true;
               		break;
               		}
				default:
					{
					oTrace->Add(L"Error", "Not a RANGE structure returned");
					bResult=false;
               		break;
					}
            	}
         	}	
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

/*bool CScanTwain::NegotiateMeasure()
{
	TW_CAPABILITY	cap;
	TW_INT16			rc;

	cap.Cap = ICAP_UNITS  ;			//Fill in capability of interest
	cap.ConType = TWON_ENUMERATION;
	cap.hContainer = NULL;

	rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
					MSG_GET, (TW_MEMREF)&cap);

   ScannerSetStruct.Measure.Clear();

	char DebugString[512];
  	strcpy(DebugString, "   NegotiateMeasure - ");

	if (rc == TWRC_SUCCESS)
		{
		switch (cap.ConType)
			{
			case TWON_ENUMERATION:
				{
				pTW_ENUMERATION	pvalEnum;
				TW_UINT16			valueU16;
				TW_UINT32			index;

				pvalEnum = (pTW_ENUMERATION)GlobalLock(cap.hContainer);

				for (index = 0; index < pvalEnum->NumItems; index++)
					{
					if (pvalEnum->ItemType == TWTY_UINT16)
						{
						valueU16 = ((TW_UINT16)(pvalEnum->ItemList[index*2]));
                  if (valueU16==TWUN_INCHES)
                  	{
                     ScannerSetStruct.Measure.AddString("inch");
                     strcat(DebugString, "inch, ");
                     }
                  if (valueU16==TWUN_CENTIMETERS)
                  	{
                     ScannerSetStruct.Measure.AddString("cm");
                     strcat(DebugString, "cm, ");
                     }
						}
					}
            char str[25];
   			ReadRegistryString("ScannerSettings", "Measure","x",str,sizeof(str));
            if (strcmp(str,"x")==0)
            	{
               ScannerSetStruct.Measure.Select(pvalEnum->CurrentIndex);
            	}
            else
            	{
               ScannerSetStruct.Measure.SelectString(str);
               }

				GlobalUnlock(cap.hContainer);
				}
			break;
			}
 		GlobalFree(cap.hContainer);
      strcat(DebugString, "\n");
      WriteDebugStringToFile(DebugString);
      return true;
		}
   else
   	{
      WriteDebugStringToFile(DebugString);
      WriteErrorMessageToFile(rc);
      return false;
		}
}
*/
bool CScanTwain::NegotiateColor()
{
	TW_CAPABILITY	cap;
	TW_INT16 	rc;
	std::wstring sErrorMessage;
	bool retval=false;

	oTrace->StartTrace(__WFUNCTION__);

	oLastScanDevice->bBlackWhite = false;
	oLastScanDevice->bGray = false;
	oLastScanDevice->bColor = false;

	if (oSettings->GetBool(eColorDepthNegotiate))
		{
		cap.Cap = ICAP_PIXELTYPE  ;			//Fill in capability of interest
		cap.ConType = TWON_ENUMERATION;
		cap.hContainer = NULL;

		rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
					MSG_GET, (TW_MEMREF)&cap);

		retval = TranslateReturnStatus(rc, sErrorMessage);
		oTrace->Add(L"ICAP_PIXELTYPE", sErrorMessage);

		if (rc == TWRC_SUCCESS) {
         		
			oLastScanDevice->iColorDepthContainerType = cap.ConType;

			switch (cap.ConType) {
			
				case TWON_ENUMERATION: {

					pTW_ENUMERATION	pvalEnum;
					TW_UINT16	valueU16;
					TW_UINT32	index;

					pvalEnum = (pTW_ENUMERATION)GlobalLock(cap.hContainer);
            
					oLastScanDevice->iColorDepthItemType = pvalEnum->ItemType;

					for (index = 0; index < pvalEnum->NumItems; index++) {

						if (pvalEnum->ItemType == TWTY_UINT16) {
						
							valueU16 = ((TW_UINT16)(pvalEnum->ItemList[index*2]));
                  
							switch (valueU16) {

								case TWPT_BW:

									oLastScanDevice->bBlackWhite = true;
									oTrace->Add(L"ColorDepth", "BlackWhite");
									if (pvalEnum->DefaultIndex==index && oSettings->GetInt(eColorDepthValue)==-1) oSettings->SetInt(eColorDepthValue, 0);
									break;

								case TWPT_GRAY:

									oLastScanDevice->bGray = true;
									oTrace->Add(L"ColorDepth", "Gray");
                     				if (pvalEnum->DefaultIndex==index && oSettings->GetInt(eColorDepthValue)==-1) oSettings->SetInt(eColorDepthValue, 1);
									break;

								case TWPT_RGB:

									oLastScanDevice->bColor = true;
									oTrace->Add(L"ColorDepth", "RGB");
                     				if (pvalEnum->DefaultIndex==index && oSettings->GetInt(eColorDepthValue)==-1) oSettings->SetInt(eColorDepthValue, 2);
									break;
								}
							}
						}
					break;
					}
				}
			GlobalUnlock(cap.hContainer);
 			GlobalFree(cap.hContainer);
			}
		}
	else {

		oLastScanDevice->bBlackWhite = true;
		oLastScanDevice->bGray = true;
		oLastScanDevice->bColor = true;
		oSettings->SetInt(eColorDepthValue, 0);

		retval=true;
   		}

	oLastScanDevice->bColorDepth = retval;

   	oTrace->EndTrace(__WFUNCTION__, retval);

	return retval;
}

bool CScanTwain::NegotiateUIControllable()
{
/*	TW_CAPABILITY	cap;
	TW_INT16			rc;

	char DebugString[512];
	strcpy(DebugString, "   NegotiateUIControllable - ");

	cap.Cap = CAP_UICONTROLLABLE;
	cap.ConType = TWON_ONEVALUE;
	cap.hContainer = NULL;

	rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
					MSG_GET, (TW_MEMREF)&cap);

	if (rc == TWRC_SUCCESS)
		{
		switch (cap.ConType)
			{
			case TWON_ONEVALUE:
				{
				pTW_ONEVALUE		pvalOneValue;

				pvalOneValue = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
				if (pvalOneValue->ItemType == TWTY_BOOL)
					{
					UIControllable = (TW_BOOL)pvalOneValue->Item;
					}
				GlobalUnlock(cap.hContainer);
				break;
				}
			}

		GlobalFree(cap.hContainer);
		}
	else
		{
		strcat(DebugString, "Error, assume ");
		UIControllable=true;
		}
	strcat(DebugString, UIControllable ? "True, " : "False, ");
	WriteDebugStringToFile(DebugString);
	WriteErrorMessageToFile(rc);  */

	UIControllable=true;
	return true;
}

bool CScanTwain::NegotiatePixelFlavor()
{
	TW_CAPABILITY	cap;
	TW_INT16	rc;
	std::wstring	sErrorMessage;
	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	cap.Cap = ICAP_PIXELFLAVOR ;
	cap.ConType = TWON_DONTCARE16;
	cap.hContainer = NULL;

	rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
					MSG_GET, (TW_MEMREF)&cap);

	bResult = TranslateReturnStatus(rc, sErrorMessage);
	oTrace->Add(L"ICAP_PIXELFLAVOR", sErrorMessage);

	if (bResult)
		{
		switch (cap.ConType)
			{
			case TWON_ONEVALUE:
				{
				pTW_ONEVALUE pval;

				pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
            	Chocolate = (pval->Item == TWPF_CHOCOLATE);
				
				oTrace->Add(L"TWON_ONEVALUE", Chocolate ? L"Chocolate" : L"Vanilla");            
				break;
				}

         	case TWON_ENUMERATION:
				{
				pTW_ENUMERATION	pval;

				pval = (pTW_ENUMERATION)GlobalLock(cap.hContainer);
      			Chocolate=(GetItemFromEnum(pval->CurrentIndex, pval)!=0);

				oTrace->Add(L"TWON_ENUMERATION", Chocolate ? L"Chocolate" : L"Vanilla");            
            	break;
            	}

			default:
				{
 				oTrace->Add(L"Error", L"Not a TW_ONEVALUE or TW_ENUMERATION returned");
				bResult = false;
            	break;
				}
			}	//End Switch Statement
 		
		GlobalUnlock(cap.hContainer);
		GlobalFree(cap.hContainer);
		}


	if (!bResult) {
		
		Chocolate = true;
		oTrace->Add(L"Error", "Assuming Chocolate PixelFlavor");
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

bool CScanTwain::NegotiateFeeder()
{
/*	TW_CAPABILITY	cap;
	TW_INT16			rc;

	char DebugString[512];
	strcpy(DebugString, "   NegotiateFeeder - ");

   bFeeder = false;

	cap.Cap = CAP_FEEDERENABLED ;
	cap.ConType = TWON_ONEVALUE;
	cap.hContainer = NULL;

	rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
					MSG_GET, (TW_MEMREF)&cap);

	if (rc == TWRC_SUCCESS)
		{
		switch (cap.ConType)
			{
			case TWON_ONEVALUE:
				{
				pTW_ONEVALUE		pval;

           	strcat(DebugString, "TWON_BOOL: ");

				pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);

            if (pval->ItemType == TWTY_BOOL) {
            	bFeeder = (TW_BOOL)pval->Item;
            	}

            strcat(DebugString, bFeeder ? "Feeder available\n" : "Feeder not available\n");

				GlobalUnlock(cap.hContainer);
				break;
				}

			default:
				{
            strcat(DebugString, "Error, not a TW_BOOL returned\n");

            break;
				}
			}	//End Switch Statement
 		GlobalFree(cap.hContainer);
		}
   else
   	{
      strcat(DebugString, "Error, call was not successful\n");
		}

   WriteDebugStringToFile(DebugString);*/
   return true;   
   }

bool
CScanTwain::NegotiateDuplex() {

	TW_CAPABILITY	cap;
	TW_INT16	rc;
   	TW_UINT16	twReturnValue;
	std::wstring	sErrorMessage;

	oTrace->StartTrace(__WFUNCTION__);

	bDuplex = false;

	cap.Cap = CAP_DUPLEX;
	cap.ConType = TWON_ONEVALUE;
	cap.hContainer = NULL;

	rc = (*lpDSM_Entry)(&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
					MSG_GET, (TW_MEMREF)&cap);

	TranslateReturnStatus(rc, sErrorMessage);
	oTrace->Add(L"CAP_DUPLEX", sErrorMessage);

	if (rc == TWRC_SUCCESS)
		{
		switch (cap.ConType)
			{
			case TWON_ONEVALUE:
				{
				pTW_ONEVALUE pval;

				pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);

    			if (pval->ItemType == TWTY_UINT16) {

    				twReturnValue = (TW_UINT16)pval->Item;

       				switch (twReturnValue) {

          				case TWDX_1PASSDUPLEX:
          				case TWDX_2PASSDUPLEX:
          					bDuplex = true;
             					break;

          				default:
          					bDuplex = false;
             					break;
       					}
    				}

				oTrace->Add(L"Duplex available", bDuplex ? L"Yes" : L"No");

				GlobalUnlock(cap.hContainer);
				break;
				}

			default:
				{
				oTrace->Add(L"Error", "Not a TW_UINT16 returned");

				break;
				}
			}	//End Switch Statement
 		
		GlobalFree(cap.hContainer);
		}

	oTrace->EndTrace(__WFUNCTION__, true);

  	return true;
	}


void
CScanTwain::SetResolution() {

	bool bResult;

	if (oLastScanDevice->iResolutionContainerType==TWON_ONEVALUE) return; // 

   	oTrace->StartTrace(__WFUNCTION__);

	
	bResult = SetResolution(ICAP_XRESOLUTION);

	//Setting the resolution failed, notify the user

	if (!bResult) {

		if (CheckMessage()) AddErrorMessage(IDS_TWRESOLUTION, 0, false);
		}

   	if (bResult) {
      
		SetResolution(ICAP_YRESOLUTION);
		ScanResolution = oSettings->GetInt(eResolutionValue);
		}

	oTrace->Add(L"ScanResolution", ScanResolution);

   	oTrace->EndTrace(__WFUNCTION__);
	}


bool
CScanTwain::SetResolution(int piTwainCapacity)
{
	TW_CAPABILITY   cap;
	TW_INT16        status;
	std::wstring	sErrorMessage;
	bool result=false;

	if (oLastScanDevice->iResolutionContainerType==TWON_ONEVALUE) return true; // We can't change it anyway

   	oTrace->StartTrace(__WFUNCTION__);

	//Try TWON_ONEVALUE
	int UseResolution = (TW_UINT8)oSettings->GetInt(eResolutionValue);
	if (UseResolution==0) UseResolution=300;

	if (!result)
		{                      
		cap.Cap = piTwainCapacity;
		cap.ConType = TWON_ONEVALUE;

		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
		if (cap.hContainer!=NULL)
			{
			pTW_ONEVALUE    pval;
			pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
			pval->ItemType = SetItem (oLastScanDevice->iResolutionItemType, &pval->Item, UseResolution);

			GlobalUnlock(cap.hContainer);

			status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
											  MSG_SET, (TW_MEMREF)&cap);

			GlobalFree((HANDLE)cap.hContainer);

			result = TranslateReturnStatus(status, sErrorMessage);
			oTrace->Add(L"ICAP_XRESOLUTION - TWON_ONEVALUE", sErrorMessage);
			}
		}

	//Try TWON_RANGE
	if (!result)
		{
		cap.Cap = piTwainCapacity;
   		cap.ConType = TWON_RANGE;

		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_RANGE));
   		
		if (cap.hContainer!=NULL)
   			{
   			pTW_RANGE    pval;
   			pval = (pTW_RANGE)GlobalLock(cap.hContainer);

   			pval->ItemType = SetItem (oLastScanDevice->iResolutionItemType, &pval->CurrentValue, UseResolution);
   			pval->ItemType = SetItem (oLastScanDevice->iResolutionItemType, &pval->MinValue, UseResolution);
   			pval->ItemType = SetItem (oLastScanDevice->iResolutionItemType, &pval->MaxValue, UseResolution);
  			pval->StepSize = 0;
  			pval->ItemType = oLastScanDevice->iResolutionItemType;
   		
			GlobalUnlock(cap.hContainer);

   			status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                                   MSG_SET, (TW_MEMREF)&cap);

   			GlobalFree((HANDLE)cap.hContainer);

			result = TranslateReturnStatus(status, sErrorMessage);
			oTrace->Add(L"ICAP_XRESOLUTION - TWON_RANGE", sErrorMessage);
      			}
      		}

	// Try TWON_ENUMERATION
	if (!result)
		{
		cap.Cap = piTwainCapacity;
   		cap.ConType = TWON_ENUMERATION;

		cap.hContainer = GlobalAlloc(GHND, (sizeof(TW_ENUMERATION) + sizeof(TW_UINT16) * (1)));
   		
		if (cap.hContainer!=NULL)
   			{
			pTW_ENUMERATION	pval;

			pval = (pTW_ENUMERATION)GlobalLock(cap.hContainer);

			pval->NumItems = 1;		//Number of Items in ItemList
			pval->ItemType = TWTY_UINT16;
			pval->ItemList[0] = UseResolution;
			pval->CurrentIndex=0;

			GlobalUnlock(cap.hContainer);

			status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&cap);

			GlobalFree((HANDLE)cap.hContainer);

			result = TranslateReturnStatus(status, sErrorMessage);
			oTrace->Add(L"ICAP_XRESOLUTION - TWON_ENUMERATION", sErrorMessage);
      			}
      		}

	oTrace->EndTrace(__WFUNCTION__, result);

	return result;
	}


void 
CScanTwain::SetColor() {

	TW_CAPABILITY   cap;
	pTW_ONEVALUE    pval;
	TW_INT16        status;
	std::wstring	sErrorMessage;

	//SetCompression();

	if (oLastScanDevice->iColorDepthContainerType==TWON_ONEVALUE) return; // We can't change it anyway

   	oTrace->StartTrace(__WFUNCTION__);

	cap.Cap = ICAP_PIXELTYPE;
	cap.ConType = TWON_ONEVALUE;

	cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));

	if (cap.hContainer!=NULL) {

		pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);

		pval->ItemType = oLastScanDevice->iColorDepthItemType;

		switch (oSettings->GetInt(eColorDepthValue)) {

			case 2:    	
				SetItem(oLastScanDevice->iColorDepthItemType, &pval->Item, TWPT_RGB);
				break;

			case 1:
				SetItem(oLastScanDevice->iColorDepthItemType, &pval->Item, TWPT_GRAY);
    			break;
           
			default:
    			SetItem(oLastScanDevice->iColorDepthItemType, &pval->Item, TWPT_BW);
				break;
			}

		GlobalUnlock(cap.hContainer);

  		status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                                   MSG_SET, (TW_MEMREF)&cap);

		TranslateReturnStatus(status, sErrorMessage);
		oTrace->Add(L"ICAP_PIXELTYPE - TWON_ONEVALUE", sErrorMessage);

  		GlobalFree((HANDLE)cap.hContainer);


  		if (status != TWRC_SUCCESS) {

     			if (CheckMessage()) AddErrorMessage(IDS_TWCOLOUR, (int)status, false);
        		}
     		}

	oTrace->EndTrace(__WFUNCTION__, status == TWRC_SUCCESS);
	}

void
CScanTwain::SetCompression()
{
/*	TW_CAPABILITY   cap;
	pTW_ONEVALUE    pval;
	TW_INT16        status;

   oTrace->StartTrace("SetCompression");

	cap.Cap = ICAP_COMPRESSION;
	cap.ConType = TWON_ONEVALUE;

	cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
	if (cap.hContainer!=NULL)
		{
		char DebugString[512];
		strcpy(DebugString, "   SetCompression:");

		pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
		pval->ItemType = TWTY_UINT16;

      SetItem(TWTY_UINT16, &pval->Item, TWCP_NONE);

		GlobalUnlock(cap.hContainer);

  		status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                                   MSG_SET, (TW_MEMREF)&cap);

  		GlobalFree((HANDLE)cap.hContainer);

      WriteDebugStringToFile(DebugString);
      WriteErrorMessageToFile(status);

  		if (status != TWRC_SUCCESS)
  			{
     		if (CheckMessage()) oErrorMessages.push_back(IDS_TWCOLOUR);
        	}
     	}

   oTrace->EndTrace("SetCompression, result=", status == TWRC_SUCCESS);*/
}

//-----------------------------------------------------------------------------

void
CScanTwain::SetMeasure() {

	TW_CAPABILITY   cap;
	pTW_ONEVALUE    pval;
	TW_INT16        status;
	std::wstring 	sErrorMessage;

   	oTrace->StartTrace(__WFUNCTION__);

	cap.Cap = ICAP_UNITS;
	cap.ConType = TWON_ONEVALUE;

	cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));

	if (cap.hContainer!=NULL)
  		{
  		pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);

  		pval->ItemType = TWTY_UINT8;
     		pval->Item = TWUN_INCHES;
  		
		GlobalUnlock(cap.hContainer);

  		status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                                   MSG_SET, (TW_MEMREF)&cap);

  		GlobalFree((HANDLE)cap.hContainer);

		TranslateReturnStatus(status, sErrorMessage);
		oTrace->Add(L"ICAP_UNITS - TWON_ONEVALUE", sErrorMessage);	
		}

   	oTrace->EndTrace(__WFUNCTION__, status == TWRC_SUCCESS);
	}

//-----------------------------------------------------------------------------

void
CScanTwain::SetDuplex() {

	TW_CAPABILITY   cap;
	TW_INT16        status;
	bool		result=false;
	std::wstring 	sErrorMessage;

	oTrace->StartTrace(__WFUNCTION__);

	if (bDuplex) {

		cap.Cap = CAP_DUPLEXENABLED;
		cap.ConType = TWON_ONEVALUE;

		cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));

		if (cap.hContainer!=NULL) {

			pTW_ONEVALUE    pval;
			pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
			pval->ItemType = SetItem(TWTY_BOOL, &pval->Item, TRUE);

			GlobalUnlock(cap.hContainer);

			status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                                      MSG_SET, (TW_MEMREF)&cap);

			GlobalFree((HANDLE)cap.hContainer);

			result = TranslateReturnStatus(status, sErrorMessage);
			oTrace->Add(L"CAP_DUPLEXENABLED - TWON_ONEVALUE", sErrorMessage);	
			}
		}
	
	oTrace->EndTrace(__WFUNCTION__, result);
	}

//-----------------------------------------------------------------------------

void
CScanTwain::SetOrientation() {

	TW_CAPABILITY   cap;
	pTW_ONEVALUE    pval;
	TW_INT16        status;
	bool bResult = false;
	std::wstring	sErrorMessage;

	if (oLastScanDevice->iOrientationContainerType==TWON_ONEVALUE) return; // We can't change it anyway

   	oTrace->StartTrace(__WFUNCTION__);

	cap.Cap = ICAP_ORIENTATION;
	cap.ConType = TWON_ONEVALUE;

	cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));

	if (cap.hContainer!=NULL) {

		pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
		pval->ItemType = oLastScanDevice->iOrientationItemType;

		oTrace->Add(L"Orientation", oSettings->GetInt(eOrientationValue)==0 ? L"Portrait" : L"Landscape");

		switch (oSettings->GetInt(eOrientationValue)) {

			case 1: //landscape
		
     			SetItem(oLastScanDevice->iOrientationItemType, &pval->Item, TWOR_LANDSCAPE);
				break;

			default: //default = portrait
  				
				SetItem(oLastScanDevice->iOrientationItemType, &pval->Item, TWOR_PORTRAIT);
				break;
         		}

		GlobalUnlock(cap.hContainer);


		status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                                   MSG_SET, (TW_MEMREF)&cap);

		bResult = TranslateReturnStatus(status, sErrorMessage);
		oTrace->Add(L"ICAP_ORIENTATION - TWON_ONEVALUE", sErrorMessage);	

  		GlobalFree((HANDLE)cap.hContainer);


  		if (!bResult) {

     			if (CheckMessage()) AddErrorMessage(IDS_TWORIENTATION, (int)status, false);
        		}
     		}
	
	oTrace->EndTrace(__WFUNCTION__, bResult);
	}

//-----------------------------------------------------------------------------

void
CScanTwain::SetBrightness() {

	TW_CAPABILITY	cap;
	TW_INT16	rc;
	pTW_ONEVALUE    pval;
	bool bResult = false;
	std::wstring sErrorMessage;

	if (oLastScanDevice->iBrightnessContainerType==TWON_ONEVALUE) return; // We can't change it anyway

   	oTrace->StartTrace(__WFUNCTION__);

	cap.Cap = ICAP_BRIGHTNESS;
	cap.ConType = TWON_ONEVALUE;

	cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));

	if (cap.hContainer!=NULL)
		{
		pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
		pval->ItemType = oLastScanDevice->iBrightnessItemType;

		oTrace->Add(L"Value%", (int)oSettings->GetInt(eBrightnessValue));

		int Bereik=oLastScanDevice->iBrightnessMax-oLastScanDevice->iBrightnessMin;
		int Value=oLastScanDevice->iBrightnessMin+(Bereik*(100+oSettings->GetInt(eBrightnessValue)))/200;
		int Modulus = (Value - oLastScanDevice->iBrightnessMin) % oLastScanDevice->iBrightnessStep;

		if (Modulus!=0) {
			
			//we zitten niet mooi op een stepsize
			int Factor;
			Factor = (Value - oLastScanDevice->iBrightnessMin) / oLastScanDevice->iBrightnessStep;

			if (Modulus>oLastScanDevice->iBrightnessStep/2) {
				Factor ++;
         		}

			Value = oLastScanDevice->iBrightnessMin + Factor * oLastScanDevice->iBrightnessStep;
			}

		oTrace->Add(L"Value", Value);


		SetItem(oLastScanDevice->iBrightnessItemType, &pval->Item, Value);

		GlobalUnlock(cap.hContainer);


		rc = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                                   MSG_SET, (TW_MEMREF)&cap);

		bResult = TranslateReturnStatus(rc, sErrorMessage);
		oTrace->Add(L"ICAP_BRIGHTNESS - TWON_ONEVALUE", sErrorMessage);	

		GlobalFree((HANDLE)cap.hContainer);

  		if (!bResult) {

			if (CheckMessage()) AddErrorMessage(IDS_TWBRIGHTNESS, (int)rc, false);
        		}
     		}

	oTrace->EndTrace(__WFUNCTION__, bResult);
	}

void 
CScanTwain::SetContrast() {

	TW_CAPABILITY	cap;
	TW_INT16	rc;
	pTW_ONEVALUE    pval;
	bool bResult = false;
	std::wstring sErrorMessage;

	if (oLastScanDevice->iContrastContainerType==TWON_ONEVALUE) return; // We can't change it anyway

   	oTrace->StartTrace(__WFUNCTION__);

	cap.Cap = ICAP_CONTRAST;
	cap.ConType = TWON_ONEVALUE;

	cap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
	
	if (cap.hContainer!=NULL)
		{
		pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);

		pval->ItemType = oLastScanDevice->iContrastItemType;

		oTrace->Add(L"Value%", (int)oSettings->GetInt(eContrastValue));

		int Bereik=oLastScanDevice->iContrastMax-oLastScanDevice->iContrastMin;
		int Value=oLastScanDevice->iContrastMin+(Bereik*(100+oSettings->GetInt(eContrastValue)))/200;	
		int Modulus = (Value - oLastScanDevice->iContrastMin) % oLastScanDevice->iContrastStep;

      		if (Modulus!=0) {
			//we zitten niet mooi op een stepsize
			int Factor;
			Factor = (Value - oLastScanDevice->iContrastMin) / oLastScanDevice->iContrastStep;

			if (Modulus>oLastScanDevice->iContrastStep/2) {
				Factor ++;
         		}

			Value = oLastScanDevice->iContrastMin + Factor * oLastScanDevice->iContrastStep;
			}

		oTrace->Add(L"Value", Value);

		SetItem(oLastScanDevice->iContrastItemType, &pval->Item, Value);

  		GlobalUnlock(cap.hContainer);

  		rc = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                                   MSG_SET, (TW_MEMREF)&cap);

 		bResult = TranslateReturnStatus(rc, sErrorMessage);
		oTrace->Add(L"ICAP_CONTRAST - TWON_ONEVALUE", sErrorMessage);	

 		GlobalFree((HANDLE)cap.hContainer);

  		if (!bResult) {

  			if (CheckMessage()) AddErrorMessage(IDS_TWCONTRAST, (int)rc, false);
			}
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);
	}

void CScanTwain::SetPage()
{
	TW_CAPABILITY   cap;
	TW_IMAGELAYOUT  layout;
	pTW_ONEVALUE    pval;
	TW_INT16        status;
	bool bRetval = false;
	std::wstring	sErrorMessage;
	
	oTrace->StartTrace(__WFUNCTION__);

	// Eerst vaste pagina definieren als dit nodig is.
	cap.hContainer = (TW_HANDLE)GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
	if (cap.hContainer!=NULL)
		{
		cap.Cap = ICAP_SUPPORTEDSIZES;
		cap.ConType = TWON_ONEVALUE;

		pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
		pval->ItemType = TWTY_UINT16;
	
		switch (oSettings->GetInt(ePageValue))
			{
			case 0: {
				pval->Item=(TW_UINT16)TWSS_A4LETTER;
				SetPaperWidth=(float)8.27;
				oTrace->Add(L"Value", "A4");
				break;
				}
			case 1:	{
				pval->Item=(TW_UINT16)TWSS_A5;
				SetPaperWidth=(float)5.83;
				oTrace->Add(L"Value", "A5");
				break;
				}
			case 2:	{
				pval->Item=(TW_UINT16)TWSS_USLETTER;
				SetPaperWidth=8.5;
				oTrace->Add(L"Value", "Letter");
				break;
				}
			case 3:	{
				pval->Item=(TW_UINT16)TWSS_USLEGAL;
				SetPaperWidth=8.5;
				oTrace->Add(L"Value", "Legal");
				break;
				}

         		default: {
				oTrace->Add(L"Value", "Default");
				break;
				}
			}

		GlobalUnlock(cap.hContainer);

		status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&cap);

 		bRetval = TranslateReturnStatus(status, sErrorMessage);
		oTrace->Add(L"ICAP_SUPPORTEDSIZES - TWON_ONEVALUE", sErrorMessage);	

		GlobalFree((HANDLE)cap.hContainer);
		}

	if (!bRetval) {
		
		cap.Cap = ICAP_UNITS;
		cap.ConType = TWON_ONEVALUE;

		// alloc the container
		cap.hContainer = (TW_HANDLE)GlobalAlloc(GHND, sizeof(TW_ONEVALUE));
		
		if (cap.hContainer!=NULL) {

			oTrace->Add(L"Error", "No success, try user defined");

			pval = (pTW_ONEVALUE)GlobalLock(cap.hContainer);
			pval->ItemType = TWTY_UINT16;
			pval->Item = TWUN_INCHES;

			GlobalUnlock(cap.hContainer);

         		status = (*lpDSM_Entry) (&appID, &dsID, DG_CONTROL, DAT_CAPABILITY,
                                      MSG_SET, (TW_MEMREF)&cap);

 			bRetval = TranslateReturnStatus(status, sErrorMessage);
			oTrace->Add(L"ICAP_UNITS - TWON_ONEVALUE", sErrorMessage);	

        		GlobalFree((HANDLE)cap.hContainer);
			}

		(*lpDSM_Entry) (&appID, &dsID, DG_IMAGE, DAT_IMAGELAYOUT,
                                      MSG_GET, (TW_MEMREF)&layout);

		layout.Frame.Right=FloatToFix32((float)P_Breedte[oSettings->GetInt(ePageValue)]);
		layout.Frame.Bottom=FloatToFix32((float)P_Hoogte[oSettings->GetInt(ePageValue)]);

		layout.Frame.Left=FloatToFix32(0);
		layout.Frame.Top=FloatToFix32(0);
		layout.PageNumber=0;
		layout.FrameNumber=0;
		layout.DocumentNumber=0;

		status=(*lpDSM_Entry) (&appID, &dsID, DG_IMAGE, DAT_IMAGELAYOUT,
                                      MSG_SET, (TW_MEMREF)&layout);

 
 		bRetval = TranslateReturnStatus(status, sErrorMessage);
		oTrace->Add(L"DAT_IMAGELAYOUT", sErrorMessage);	
		}

	oTrace->EndTrace(__WFUNCTION__, bRetval);
	}



int 
CScanTwain::GetItemFromEnum(int Index, pTW_ENUMERATION pvalEnum) {

	switch (pvalEnum->ItemType)
		{
		case TWTY_INT8: return (int)pvalEnum->ItemList[Index*sizeof TWTY_INT8];
		case TWTY_INT16: return (int)pvalEnum->ItemList[Index*sizeof TWTY_INT16];
		case TWTY_INT32: return (int)pvalEnum->ItemList[Index*sizeof TWTY_INT32];
		case TWTY_UINT8: return (int)pvalEnum->ItemList[Index*sizeof TWTY_UINT8];
		case TWTY_UINT16: return (int)pvalEnum->ItemList[Index*sizeof TWTY_UINT16];
		case TWTY_UINT32: return (int)pvalEnum->ItemList[Index*sizeof TWTY_UINT32];
		case TWTY_BOOL: return (int)pvalEnum->ItemList[Index*sizeof TWTY_BOOL];
		case TWTY_FIX32:
			{
			pTW_FIX32 pTWFix32;
	 		pTWFix32 = (pTW_FIX32)&(pvalEnum->ItemList[Index*sizeof TWTY_FIX32]);
			return (int)FIX32ToFloat(*pTWFix32);
			}
		}
	return -1;
	}

int 
CScanTwain::GetItemFromArray(int Index, pTW_ARRAY pvalArray) {

	switch (pvalArray->ItemType)
		{
		case TWTY_INT8: return (int)pvalArray->ItemList[Index*sizeof TWTY_INT8];
		case TWTY_INT16: return (int)pvalArray->ItemList[Index*sizeof TWTY_INT16];
		case TWTY_INT32: return (int)pvalArray->ItemList[Index*sizeof TWTY_INT32];
		case TWTY_UINT8: return (int)pvalArray->ItemList[Index*sizeof TWTY_UINT8];
		case TWTY_UINT16: return (int)pvalArray->ItemList[Index*sizeof TWTY_UINT16];
		case TWTY_UINT32: return (int)pvalArray->ItemList[Index*sizeof TWTY_UINT32];
		case TWTY_BOOL: return (int)pvalArray->ItemList[Index*sizeof TWTY_BOOL];
		case TWTY_FIX32:
			{
			pTW_FIX32 pTWFix32;
	 		pTWFix32 = (pTW_FIX32)&(pvalArray->ItemList[Index*sizeof TWTY_FIX32]);
         		return (int)FIX32ToFloat(*pTWFix32);
         		}
      		}
   	return -1;
	}

float 
CScanTwain::FIX32ToFloat (TW_FIX32 fix32) {

	float 	floater;

	floater = (float)fix32.Whole + (float)fix32.Frac / (float)65536.0;
	return floater;
	}

TW_FIX32 
CScanTwain::FloatToFix32 (float floater) {

	TW_FIX32 Fix32_value;
	TW_INT32 value = (TW_INT32) (floater * 65536.0 + 0.5);
	Fix32_value.Whole = (TW_INT16)( value >> 16);
	Fix32_value.Frac = (TW_UINT16)( value & 0x0000ffffL);
	return (Fix32_value);
	}


bool
CScanTwain::TestMemory() {

	int bpp;
	long w,h;
	float pw,ph;
	int resolution;
	bool bResult;

	bResult = true;

	oTrace->StartTrace(__WFUNCTION__);

	if (CheckMessage()) {

		if (oLastScanDevice->bResolution)
			{
			resolution=oSettings->GetInt(eResolutionValue);
			}
		else
			{
			resolution=1;
			}                     

		int index=oSettings->GetInt(ePageValue);
		ph=(float)P_Hoogte[index];
		pw=(float)P_Breedte[index];

		w=(long)(pw*resolution);
		h=(long)(ph*resolution);

		if (oLastScanDevice->bColorDepth)
			{
			bpp=8;
			if (oSettings->GetInt(eColorDepthValue)==0) bpp=1;
			if (oSettings->GetInt(eColorDepthValue)==2) bpp=24;
			}

		MEMORYSTATUS mStat;
		mStat.dwLength=sizeof(mStat);
		GlobalMemoryStatus(&mStat);
		DWORD dibSize=(DWORD)(w*h*bpp*1.5/8);
		int memFree = (int)mStat.dwAvailVirtual-dibSize;
		int r=IDYES;

		if (memFree<1000000) {
			bResult = false;
			}
		}

   oTrace->EndTrace(__WFUNCTION__, bResult);

   return bResult;
	}