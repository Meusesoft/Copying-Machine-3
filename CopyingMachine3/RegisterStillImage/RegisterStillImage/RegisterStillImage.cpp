// RegisterStillImage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	int iResult;
	bool bRegister;
	std::wstring sOption;
	std::wstring sExecutable;
	
	//initialise variables
	iResult = 0;
	bRegister = true;
	sExecutable = L"";

	//process the commandline, at least an executable its location is necesary
	while (argc > 1) {

		argc--;

		sOption = argv[argc];

		if (sOption == L"/register") bRegister = true;
		if (sOption == L"/unregister") bRegister = false;
		if (sOption.find(L".exe")!=std::string::npos) {
			sExecutable = sOption;
			}
		}
	
	if (bRegister && sExecutable.length()==0) {

		iResult = 1;

		std::wcout << L"Copying Machine - Missing parameter\n";
		}

	if (iResult==0) {

		std::wcout << L"Copying Machine - Register with StillImage: ";

		//Register with Still Image
		wchar_t wAppPath[MAX_PATH];

		HRESULT hRes;
		IStillImage * g_StillImage;

		//eerste versie van windows bepalen, still image is beperkt beschikbaar
		OSVERSIONINFO lpVersionInformation;

		lpVersionInformation.dwOSVersionInfoSize = sizeof(lpVersionInformation);

		if (GetVersionEx(&lpVersionInformation)!=0) {

		 //StillImage is only available since win98 en win2000, so exclude win95 en winNT3 en winNT4.

		 if (lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32s ||
			(lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && lpVersionInformation.dwMajorVersion<=4 && lpVersionInformation.dwMinorVersion<10) ||
			(lpVersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT	&& lpVersionInformation.dwMajorVersion<5)) {
			   //still image is not supported on these platform versions

				std::wcout << L"StillImage not available\n";
				iResult = 1;
			   }
		 else {
			//(Un)registering and interfacing with StillImage.
			//Has to run as administrator in Windows Vista and beyond (User Access Control);

			GetModuleFileName(NULL, wAppPath, MAX_PATH);

			hRes = StiCreateInstance(GetModuleHandle(NULL), STI_VERSION, &g_StillImage,NULL);

			if (SUCCEEDED(hRes)) {

				if (bRegister) {
					hRes = g_StillImage->RegisterLaunchApplication(L"Copying Machine", (LPWSTR)sExecutable.c_str());
					}
				else {
					hRes = g_StillImage->UnregisterLaunchApplication(L"Copying Machine");
					}

			   if (!SUCCEEDED(hRes)) {

				  std::wcout << L"Error\n";
				  iResult = 1;
				  }
			   else {

				  std::wcout << L"Success\n";
				   }
				}
			 }
		  }
		}

	//Return the result;
	return iResult;
	}

