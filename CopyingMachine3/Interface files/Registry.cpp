/*********************************************************
* CRegistry class
* 
* class for writing and reading to and from the registry
* or something else for storing preferences and settings
*
*********************************************************/

#include "stdafx.h"
#include <atlbase.h>

CRegistry::CRegistry() {

	wchar_t cXMLFile[MAX_PATH];

	eType = eXMLDocument;
	eType = eWindowsRegistry;

	if (eType==eXMLDocument) {

		//get the appdata folder, and create one if it doesn't exists
		SHGetSpecialFolderPath(NULL, cXMLFile, CSIDL_COMMON_APPDATA, TRUE);
		wcscat_s(cXMLFile, MAX_PATH, L"\\Meusesoft Copying Machine\\");
		CreateDirectory(cXMLFile, NULL);
		wcscat_s(cXMLFile, MAX_PATH, _T(SettingsFile));

		//Create an XML document
		oXMLDocument = new CXMLDocument();

		//Load the XML document
		oXMLDocument->Load(cXMLFile);

		//Is it a valid document? Check if it has a settings node
		if (oXMLDocument->SelectSingleNode(_T(QueryTopNode), eRoot)==NULL) {

			oXMLDocument->AddNode(_T(TopNodeName));
			}
		}
	}

CRegistry::~CRegistry() {

	wchar_t cXMLFile[MAX_PATH];

	if (eType==eXMLDocument) {

		//get the appdata folder, and create one if it doesn't exists
		SHGetSpecialFolderPath(NULL, cXMLFile, CSIDL_COMMON_APPDATA, TRUE);
		wcscat_s(cXMLFile, MAX_PATH, L"\\Meusesoft Copying Machine\\");
		CreateDirectory(cXMLFile, NULL);
		wcscat_s(cXMLFile, MAX_PATH, _T(SettingsFile));

		//Save the document (and all the settings)
		oXMLDocument->Save(cXMLFile);

		//Clear up
		delete oXMLDocument;
		}
	}

//---------------------------------------------------------------------------------
int CRegistry::ReadInt(std::wstring pKey, std::wstring pEntry, int piDefaultValue) {

	int iReturn;

	switch (eType) {

		case eXMLDocument: 

			iReturn = ReadIntXML(pKey, pEntry, piDefaultValue);
			break;

		default:

			iReturn = ReadIntWindows(HKEY_CURRENT_USER, pKey, pEntry, piDefaultValue);
			break;
		}

	return iReturn;
	}

int 
CRegistry::ReadIntLocalMachine(std::wstring pKey, std::wstring pEntry, int piDefaultValue) {

	return ReadIntWindows(HKEY_LOCAL_MACHINE, pKey, pEntry, piDefaultValue);
	}

int 
CRegistry::ReadIntWindows(HKEY phKey, std::wstring pKey, std::wstring pEntry, int piDefaultValue) {

	//read an integer from the registry. If the entry doesn't exist use the
	//value in piDefaultValue
	long lResult;
	int  iReturn;
	DWORD dValue;
	DWORD dValueSize;
	HKEY hHandle;
	std::wstring sKey;

	sKey = _T(RegistryBase);
	sKey += pKey;	
	
	iReturn = piDefaultValue;

	lResult = ::RegOpenKeyEx(phKey, sKey.c_str(), 0, KEY_READ, &hHandle);
	if (lResult==ERROR_SUCCESS) {

		dValueSize = sizeof(DWORD);
		
		lResult = ::RegQueryValueEx(hHandle, pEntry.c_str(), NULL, NULL, (LPBYTE)&dValue, &dValueSize);

		if (lResult==ERROR_SUCCESS) {

			iReturn = dValue;
		}

		::RegCloseKey(hHandle);
	}

	return iReturn;
}

int 
CRegistry::ReadIntXML(std::wstring pKey, std::wstring pEntry, int piDefaultValue) {

	XNode oNode;
	WCHAR sNumber[20];
	int iReturn;

	oXMLDocument->SetCurrentNode(oXMLDocument->GetTopNode());

	//Key node
	oNode = oXMLDocument->SelectSingleNode((WCHAR*)pKey.c_str(), eRoot);

	if (oNode==NULL) return piDefaultValue;

	oXMLDocument->SetCurrentNode(oNode);

	//Entry node
	oNode = oXMLDocument->SelectSingleNode((WCHAR*)pEntry.c_str(), eCurrent);

	if (oNode==NULL) {
		
		return piDefaultValue;
		}

	oXMLDocument->SetCurrentNode(oNode);

	//Get Value
	if (oXMLDocument->GetAttribute(L"Value", sNumber, 20)) {

		iReturn = _wtoi(sNumber);
		}
	else {
		
		iReturn = piDefaultValue;
		}
	
	return iReturn;
	}
//---------------------------------------------------------------------------------

void 
CRegistry::WriteInt(std::wstring pKey, std::wstring pEntry, int piValue) {

	switch (eType) {

		case eXMLDocument: 

			WriteIntXML(pKey, pEntry, piValue);
			break;

		default:

			WriteIntWindows(HKEY_CURRENT_USER, pKey, pEntry, piValue);
			break;
		}
	}

void 
CRegistry::WriteIntLocalMachine(std::wstring pKey, std::wstring pEntry, int piValue) {

	WriteIntWindows(HKEY_LOCAL_MACHINE, pKey, pEntry, piValue);
	}

void 
CRegistry::WriteIntWindows(HKEY phKey, std::wstring pKey, std::wstring pEntry, int piValue) {

	//write an integer to the registry. If the entry doesn't exist, create it
	long lResult;
	DWORD dValue;
	HKEY hHandle;
	std::wstring sKey;

	sKey = _T(RegistryBase);
	sKey += pKey;	
	
	lResult = ::RegCreateKeyEx(phKey, sKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hHandle, NULL);
	
	if (lResult==ERROR_SUCCESS) {

		dValue = piValue;
		
		::RegSetValueEx(hHandle, pEntry.c_str(), NULL, REG_DWORD, (LPBYTE)&dValue, sizeof(DWORD));
		
		if (lResult!=ERROR_SUCCESS) {

			//TRACE0("Failed to write registry (integer)\n");
		}

		::RegCloseKey(hHandle);
	}
	else {

		//TRACE0("Failed to open registry key (write integer)\n");
	}
}
	
void 
CRegistry::WriteIntXML(std::wstring pKey, std::wstring pEntry, int piValue) {

	XNode oNode;
	WCHAR sNumber[20];

	oXMLDocument->SetCurrentNode(oXMLDocument->GetTopNode());

	oNode = oXMLDocument->SelectSingleNode((WCHAR*)pKey.c_str(), eRoot);

	//Key node
	if (oNode==NULL) {

		oNode = oXMLDocument->AddNode((WCHAR*)pKey.c_str());
		}
	
	oXMLDocument->SetCurrentNode(oNode);

	//Entry Node
	oNode = oXMLDocument->SelectSingleNode((WCHAR*)pEntry.c_str(), eCurrent);

	if (oNode==NULL) {

		oNode = oXMLDocument->AddNode((WCHAR*)pEntry.c_str());
		}
	oXMLDocument->SetCurrentNode(oNode);

	//Add value
	swprintf_s(sNumber, 20, L"%d", piValue); 

	oXMLDocument->SetAttribute(L"Value", sNumber);
}
//---------------------------------------------------------------------------------

void CRegistry::WriteString(std::wstring pKey, std::wstring pEntry, std::wstring psValue) {

	switch (eType) {

		case eXMLDocument: 

			WriteStringXML(pKey, pEntry, psValue);
			break;

		default:

			WriteStringWindows(pKey, pEntry, psValue);
			break;
		}
}

void CRegistry::WriteStringWindows(std::wstring pKey, std::wstring pEntry, std::wstring psValue) {

	//write a string to the registry. If the entry doesn't exist, create it
	long lResult;
	std::wstring sKey;
	std::wstring sReturn;
	HKEY hHandle;

	sKey = _T(RegistryBase);
	sKey += pKey;	
	
	lResult = ::RegCreateKeyEx(HKEY_CURRENT_USER, sKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hHandle, NULL);
	
	if (lResult==ERROR_SUCCESS) {

		//sValue = psValue.c_str();

		string sValue(psValue.begin(), psValue.end()); 
		sValue.assign(psValue.begin(), psValue.end()); 

		::RegSetValueEx(hHandle, pEntry.c_str(), NULL, REG_SZ, (BYTE*)psValue.c_str(), (DWORD)(psValue.length() + 1)*2);

	
		if (lResult!=ERROR_SUCCESS) {

			//TRACE0("Failed to write registry (string)\n");
		}

		::RegCloseKey(hHandle);
	}
	else {

		//TRACE0("Failed to open registry key (write string)\n");
	}
}

void CRegistry::WriteStringXML(std::wstring pKey, std::wstring pEntry, std::wstring psValue) {

	XNode oNode;

	oXMLDocument->SetCurrentNode(oXMLDocument->GetTopNode());

	oNode = oXMLDocument->SelectSingleNode((WCHAR*)pKey.c_str(), eRoot);

	//Key node
	if (oNode==NULL) {

		oNode = oXMLDocument->AddNode((WCHAR*)pKey.c_str());
		}
	
	oXMLDocument->SetCurrentNode(oNode);

	//Entry Node
	oNode = oXMLDocument->SelectSingleNode((WCHAR*)pEntry.c_str(), eCurrent);

	if (oNode==NULL) {

		oNode = oXMLDocument->AddNode((WCHAR*)pEntry.c_str());
		}
	oXMLDocument->SetCurrentNode(oNode);

	//Add value

	oXMLDocument->SetAttribute(L"Value", (WCHAR*)psValue.c_str());
}
//---------------------------------------------------------------------------------
void CRegistry::ReadString(std::wstring pKey, std::wstring pEntry, std::wstring psDefaultValue, std::wstring &psValue) {

	switch (eType) {

		case eXMLDocument: 

			ReadStringXML(pKey, pEntry, psDefaultValue, psValue);
			break;

		default:

			ReadStringWindows(pKey, pEntry, psDefaultValue, psValue);
			break;
		}
}

void CRegistry::ReadStringWindows(std::wstring pKey, std::wstring pEntry, std::wstring psDefaultValue, std::wstring &psValue) {

	//read a string from the registry. If the entry doesn't exist use the
	//value in piDefaultValue
	long lResult;
	std::wstring sKey;
	LPBYTE sValue;
	HKEY hHandle;
	DWORD dType, dCount;

	sKey = _T(RegistryBase);
	sKey += pKey;	
	
	psValue = psDefaultValue;

	lResult = ::RegOpenKeyEx(HKEY_CURRENT_USER, sKey.c_str(), 0, KEY_READ, &hHandle);
	if (lResult==ERROR_SUCCESS) {

 
		lResult = RegQueryValueEx(hHandle, pEntry.c_str(), NULL, &dType, NULL, &dCount);
		
		if (lResult == ERROR_SUCCESS && dType == REG_SZ) {
			
			sValue = (LPBYTE)malloc(dCount + 1);
			
			RegQueryValueEx(hHandle, pEntry.c_str(), NULL, &dType, sValue, &dCount);

			if (lResult == ERROR_SUCCESS) {

				psValue = (TCHAR*)sValue;
				}
			
			free (sValue);
		}

	::RegCloseKey(hHandle);
	}

	//return psValue;
}

void CRegistry::ReadStringXML(std::wstring pKey, std::wstring pEntry, std::wstring psDefaultValue, std::wstring &psValue) {

	XNode oNode;
	WCHAR sValue[MAX_PATH];

	oXMLDocument->SetCurrentNode(oXMLDocument->GetTopNode());

	//Key node
	oNode = oXMLDocument->SelectSingleNode((WCHAR*)pKey.c_str(), eRoot);

	if (oNode==NULL) {
		
		psValue = psDefaultValue;
		return;
		}

	oXMLDocument->SetCurrentNode(oNode);

	//Entry node
	oNode = oXMLDocument->SelectSingleNode((WCHAR*)pEntry.c_str(), eCurrent);

	if (oNode==NULL) {
		
		psValue = psDefaultValue;
		return;
		}

	oXMLDocument->SetCurrentNode(oNode);

	//Get value
	if (oXMLDocument->GetAttribute(L"Value", sValue, MAX_PATH)) {

		psValue = sValue;
		}
	else {
		psValue = psDefaultValue;
		}
	}
//---------------------------------------------------------------------------------
void 
CRegistry::ReadClass(std::wstring pKey, std::wstring pEntry, std::wstring psDefaultValue, 
					 std::wstring &psValue) {

	//read a string from the registry. If the entry doesn't exist use the
	//value in piDefaultValue
	long lResult;
	std::wstring sKey;
	LPBYTE sValue;
	HKEY hHandle;
	DWORD dType, dCount;

	sKey = pKey;
	
	psValue = psDefaultValue;

	lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, sKey.c_str(), 0, KEY_READ, &hHandle);
	if (lResult==ERROR_SUCCESS) {

 
		lResult = RegQueryValueEx(hHandle, pEntry.c_str(), NULL, &dType, NULL, &dCount);
		
		if (lResult == ERROR_SUCCESS && dType == REG_SZ) {
			
			sValue = (LPBYTE)malloc(dCount + 1);
			
			RegQueryValueEx(hHandle, pEntry.c_str(), NULL, &dType, sValue, &dCount);

			if (lResult == ERROR_SUCCESS) {

				psValue = (TCHAR*)sValue;
				}
			
			free (sValue);
		}

	::RegCloseKey(hHandle);
	}

	//return psValue;
	}

//---------------------------------------------------------------------------------
void 
CRegistry::WriteClass(std::wstring pKey, std::wstring psValue) {

	//write a string to the registry. If the entry doesn't exist, create it
	long lResult;
	std::wstring sKey;
	std::wstring sReturn;
	HKEY hHandle;

	sKey = pKey;	

	lResult = ::RegCreateKeyEx(HKEY_CLASSES_ROOT, sKey.c_str(), 0, L"REG_SZ", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hHandle, NULL);
	
	if (lResult==ERROR_SUCCESS) {

		string sValue(psValue.begin(), psValue.end()); 
		sValue.assign(psValue.begin(), psValue.end()); 

		::RegSetValueEx(hHandle, L"", NULL, REG_SZ, (BYTE*)psValue.c_str(), (DWORD)(psValue.length() + 1)*2);

	
		if (lResult!=ERROR_SUCCESS) {

			MsgBox;
			//TRACE0("Failed to write registry (string)\n");
		}

		::RegCloseKey(hHandle);
	}
	else {

		//TRACE0("Failed to open registry key (write string)\n");
	}

	if (lResult!=ERROR_SUCCESS) {

    TCHAR szBuf[280]; 
    LPVOID lpMsgBuf;
    DWORD dw = lResult; 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    wsprintf(szBuf, 
        L"failed with error %d: %s", 
        dw, lpMsgBuf); 
 
    MessageBox(NULL, szBuf, L"Error", MB_OK); 

    LocalFree(lpMsgBuf);

	}
}

//---------------------------------------------------------------------------------
void 
CRegistry::NukeClass(std::wstring pKey) {




}
