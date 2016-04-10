#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "rsa.h"
#include "shlobj.h"


CRegistration::CRegistration(sGlobalInstances pcGlobalInstances) {

	//Initialise global instances
	memcpy(&oGlobalInstances, &pcGlobalInstances, sizeof(sGlobalInstances));

	oTrace		= oGlobalInstances.oTrace;
	oRegistry	= oGlobalInstances.oRegistry;

	GetHash();
	
	//check the registration
	bRegistered = CheckRegistration();
	}

CRegistration::~CRegistration() {
	}

void 
CRegistration::Reminder(HWND phWnd) {

	CCopyingMachineCore* oCmCore;

	if (!bRegistered) {

		oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

		oCmCore->oDialogs->DlgReminder(phWnd);
		}
	}

//--------------------------------------------------------------------------------------------------
// Functies voor afhandelen registratie van gebruikers.
//--------------------------------------------------------------------------------------------------

bool 
CRegistration::CheckRegistration() {

   char* xmlstr;
   bool bReturn;
	int iChecksum;
	bReturn = false;

	xmlstr = GetRegistrationXML();

	if (xmlstr!=NULL) {

		iChecksum=0;

		for (int i=1; i<(int)strlen(xmlstr+1); i++) {
			iChecksum = (iChecksum + *(xmlstr+i));
			}

		iChecksum = (iChecksum & 0x07);
		iChecksum = (iChecksum << 5 ) & 0xFF;
		iChecksum = iChecksum | 0x20;

//		iChecksum = (iChecksum << 5)  & 0xFF;

		if (iChecksum!=*xmlstr) {

			wchar_t* wXML;
			long lXMLSize;
			
			lXMLSize = (long)(sizeof(wchar_t)*strlen(xmlstr+1) + 2);
			wXML = (wchar_t*)malloc(lXMLSize);
			SecureZeroMemory(wXML, lXMLSize);

			MultiByteToWideChar(CP_ACP, 0, xmlstr+1, (int)strlen(xmlstr+1), wXML, lXMLSize); 

			free(xmlstr);

			XNode oNode;
			WCHAR sValue[MAX_PATH];

			CXMLDocument* oXMLDocument;

			oXMLDocument = new CXMLDocument();
			oXMLDocument->LoadXML(wXML);

			oXMLDocument->SetCurrentNode(oXMLDocument->GetTopNode());
				
			oNode = oXMLDocument->SelectSingleNode(L"applicatie", eRoot);

			if (oNode!=NULL) {
				
				oXMLDocument->SetCurrentNode(oNode);
				if (oXMLDocument->GetAttribute(L"name", sValue, MAX_PATH)) {
					if (wcscmp(sValue, L"Copying Machine")==0) bReturn = true;
					}
				}

			oNode = oXMLDocument->SelectSingleNode(L"user", eRoot);

			if (oNode!=NULL) {
				
				oXMLDocument->SetCurrentNode(oNode);
				if (oXMLDocument->GetAttribute(L"name", sValue, MAX_PATH)) {
					sName = sValue;
					}
				if (oXMLDocument->GetAttribute(L"organisation", sValue, MAX_PATH)) {
					sOrganisation = sValue;
					}
				}

			free(wXML);
		}
	}

	return bReturn;
}

//This function retrieves the xml from the encrypted
//key file which contains the registration information
char* 
CRegistration::GetRegistrationXML() {

	R_RSA_PUBLIC_KEY publicKey;
	char PublicKeyTextForm[sizeof(publicKey)*2+100];
	char* memstr;
	char* xmlstr;
	char* pointer;
	char cHexByte[3];
	char cBinByte;
	unsigned char MemStr[sizeof(publicKey)];

	xmlstr=NULL;

	//dit is de key geplaatst in een array of chars. Daarna wordt de key eerst
	//omgezet naar binaire data in het geheugen, om vervolgens gekopieerd te
	//worden naar de variable publicKey
	strcpy_s(PublicKeyTextForm, sizeof(publicKey)*2+100, "0002000000000000000000000000000000000000000000000"
		"000000000000000000000000000000000000000000000000000000000000000000000000000"
		"00000000C184BEE6F8292804D93BC26422106F6735829FF7771C763C7806C6806F677F7735F"
		"71E6D68DF46C1D4CE7C2D4BB8B818335A7E5AC2DA444289A5446D5321C3ED00000000000000"
		"000000000000000000000000000000000000000000000000000000000000000000000000000"
		"000000000000000000000000000000000000000000000000000000000000000000000000000"
		"000000000000000000000000000000000000000000000000000000000000000000000000000"
		"00000000000010001");

	 //alloceren tijdelijke geheugen ruimte
	 if ((memstr = (char *) malloc(sizeof(publicKey))) != NULL) {

		pointer = memstr;
		for (int i=0; i<(int)(strlen(PublicKeyTextForm)/2); i++) {

			strncpy_s(cHexByte, 3, PublicKeyTextForm+(i*2), 2);

			MemStr[i] = (unsigned char)ConvertHexDoubleDigit(cHexByte);
			}

		memcpy(&publicKey, MemStr, sizeof(publicKey));

		//vrijgeven tijdelijke geheugen ruimte
		free(memstr);
		}


    //I've got the key, now lets get the data from the registration file
	FILE* hFileHandle;
	std::wstring sKeyFileName;

	oRegistry->ReadString(L"General", L"Key", L"", sKeyFileName);

	 _wfopen_s(&hFileHandle, sKeyFileName.c_str(), L"rt");

    if (hFileHandle!=NULL) {
      //omvang bepalen
      long curpos, length;

      curpos = ftell(hFileHandle);
      fseek(hFileHandle, 0L, SEEK_END);
      length = ftell(hFileHandle);
      fseek(hFileHandle, curpos, SEEK_SET);

	 	if ((memstr = (char *) malloc(length)) != NULL) {

      	if (fread(memstr, 1, length, hFileHandle) != length) {
         	//error, afsluiten
            free(memstr);
            fclose(hFileHandle);
            return NULL;
         	}
      	}

    	fclose(hFileHandle);

      int iRepeatCount;
      int iPointer;

    	unsigned char output[MAX_RSA_MODULUS_LEN + 1]; /* output block */
    	unsigned int outputLen; /* length of output block */
    	unsigned char input[MAX_RSA_MODULUS_LEN + 1]; /* input block */
    	//unsigned int inputLen; /* length of input block */

      //I've got the data in text mode, transform to binary mode
      pointer = memstr;

      for (int i=0; i<length/2; i++) {

			strncpy_s(cHexByte, 3, memstr+(i*2), 2);

			cBinByte = (char)ConvertHexDoubleDigit(cHexByte);

			*pointer = (char)cBinByte;
			pointer++;
      	}

      //I've got the data in binary mode, now decrypt
      iRepeatCount = length/128;
      iPointer = 0;
	  int iXmlStrSize;

	  iXmlStrSize = 21*(length/128)+1;

	 	if ((xmlstr = (char *) malloc(iXmlStrSize)) != NULL) {
        	ZeroMemory(xmlstr, iXmlStrSize);

         do {
         	ZeroMemory(&input, MAX_RSA_MODULUS_LEN+1);
         	ZeroMemory(&output, MAX_RSA_MODULUS_LEN+1);
         	memcpy(input, memstr+iPointer*64, 64);

            outputLen=0;
            if (RSAPublicDecrypt(output, &outputLen, input, 64, &publicKey)==ID_OK) {

               strcat_s(xmlstr, iXmlStrSize, (const char*)output);
        			}

            iPointer++;
            iRepeatCount--;
         	} while(iRepeatCount>0);
      	}
      else {
         //error, afsluiten
         free(memstr);
         return NULL;
      	}

      free(memstr);
		}


      return xmlstr;

}

int 
CRegistration::ConvertHexDoubleDigit(char* cInput) {

	int iReturn;

	iReturn = ConvertHexDigit(*cInput)*16 + ConvertHexDigit(*(cInput+1));

	return iReturn;
}

int 
CRegistration::ConvertHexDigit(char cInput) {

	int iReturn;

	iReturn = 0;

	switch (cInput) {
		case '0': {iReturn = 0; break;}
		case '1': {iReturn = 1; break;}
		case '2': {iReturn = 2; break;}
		case '3': {iReturn = 3; break;}
		case '4': {iReturn = 4; break;}
		case '5': {iReturn = 5; break;}
		case '6': {iReturn = 6; break;}
		case '7': {iReturn = 7; break;}
		case '8': {iReturn = 8; break;}
		case '9': {iReturn = 9; break;}
		case 'A': {iReturn = 10; break;}
		case 'B': {iReturn = 11; break;}
		case 'C': {iReturn = 12; break;}
		case 'D': {iReturn = 13; break;}
		case 'E': {iReturn = 14; break;}
		case 'F': {iReturn = 15; break;}
		}

	return iReturn;
}

//This function places the key by a text file
bool 
CRegistration::PlaceKeyText(std::wstring psKeyText) {

	std::wstring sCleanKeyText;
	wchar_t cFileName[MAX_PATH];
	wchar_t cReceivedTime[100];
	FILE* hFile;
	SYSTEMTIME sSysTime;

	oTrace->StartTrace(__WFUNCTION__);

	//remove unwanted characters from the string
	wchar_t cCharacter;
	sCleanKeyText.clear();
	for (int i=0; i<(int)psKeyText.length(); i++) {
	
		cCharacter = psKeyText.at(i);

		if ((cCharacter>=48 && cCharacter<=58) ||
			(cCharacter>=65 && cCharacter<=90) ||
			(cCharacter>=97 && cCharacter<=122)) {

			sCleanKeyText += cCharacter;
			}
		}

	//get the appdata folder, and create one if it doesn't exists
	SHGetSpecialFolderPath(NULL, cFileName, CSIDL_COMMON_APPDATA, TRUE);
	wcscat_s(cFileName, MAX_PATH, L"\\Meusesoft Copying Machine\\");
	CreateDirectory(cFileName, NULL);	
	
	//create a unique filename
	wcscat_s(cFileName, MAX_PATH, L"copymach_");

	GetSystemTime(&sSysTime);
	if (GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &sSysTime, L"HHmmss", cReceivedTime, sizeof(cReceivedTime))!=0) {
		wcscat_s(cFileName, MAX_PATH, cReceivedTime);
		}

	wcscat_s(cFileName, MAX_PATH, L".cmk");

	//write the content to the file
	if (sCleanKeyText.length()>0) {

		_wfopen_s(&hFile, cFileName, L"w");

		//the registration file isn't in unicode format. Convert it to ansistring.
		CW2A sAnsiCleanKeyText(sCleanKeyText.c_str());

		if (hFile!=NULL) {
		   fwrite(sAnsiCleanKeyText, strlen(sAnsiCleanKeyText), 1, hFile);

		   fclose (hFile);
		   }

		//pas register aan
		oRegistry->WriteString(L"General", L"Key", cFileName);
		}

	bRegistered = CheckRegistration();

	if (!bRegistered) {
	
		//delete invalid registration file
		DeleteFile(cFileName);
		}

	oTrace->EndTrace(__WFUNCTION__, bRegistered);

	return bRegistered;
	}

//This function places the key by a file
bool 
CRegistration::PlaceKeyFile(std::wstring psFilename) {

	wchar_t cFileName[MAX_PATH];
	wchar_t cReceivedTime[100];
	SYSTEMTIME sSysTime;

	oTrace->StartTrace(__WFUNCTION__);

	//get the appdata folder, and create one if it doesn't exists
	SHGetSpecialFolderPath(NULL, cFileName, CSIDL_COMMON_APPDATA, TRUE);
	wcscat_s(cFileName, MAX_PATH, L"\\Meusesoft Copying Machine\\");
	CreateDirectory(cFileName, NULL);	

	//create a unique filename
	wcscat_s(cFileName, MAX_PATH, L"copymach_");

	GetSystemTime(&sSysTime);
	if (GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &sSysTime, L"HHmmss", cReceivedTime, sizeof(cReceivedTime))!=0) {
		wcscat_s(cFileName, MAX_PATH, cReceivedTime);
		}

	wcscat_s(cFileName, MAX_PATH, L".cmk");

	//Copy the file to the appdata
	CopyFile(psFilename.c_str(), cFileName, FALSE);

	//Update registry entry
	oRegistry->WriteString(L"General", L"Key", cFileName);

	//Recheck registration
	bRegistered = CheckRegistration();

	if (!bRegistered) {
	
		//delete invalid registration file
		DeleteFile(cFileName);
		}

	oTrace->EndTrace(__WFUNCTION__, bRegistered);

	return bRegistered;
	}

//This function retrieves the hash from the language module
void  
CRegistration::GetHash() {

	wchar_t	cHash[MAX_PATH];
	HINSTANCE hModule;

	hModule = LoadLibrary(L"cm_english.dll");
	
	if (hModule) {

		LoadString(hModule, IDS_HASH, cHash, MAX_PATH);
		sHash = cHash;

		FreeLibrary(hModule);
		}

	#ifdef _DEBUG
		//This is for debugging only.
		CCopyingMachineCore* oCmCore;
		sExecutableInformation cExecutableInformation;
		oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		
		oCmCore->GetCopyingMachineVersion(cExecutableInformation);
		sHash = cExecutableInformation.sHashValue;
	#endif
}
