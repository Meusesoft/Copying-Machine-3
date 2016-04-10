#include "StdAfx.h"
#include "Trace.h"


//#include <shlobj.h>
//#include <shlwapi.h>
//#include <io.h>

CRegistry* CTracer::oDialogRegistry;

CTraceNode::CTraceNode(CTraceNode* poParent) {

	oParent = poParent;

	sFunction=L"";
	sResult = L"";
	sClass = L"";
	eType = eTraceNodeType::eTraceFunction;
	}

CTraceNode::~CTraceNode(void) {

	for (long lIndex=(long)oChilds.size()-1; lIndex>=0; lIndex--) {

		delete oChilds[lIndex];
		}
	}

void CTraceNode::Write(FILE* poFile, bool pbEndNode) {

	if (poFile==NULL) return;
	
	switch (eType) {
		
		case eTraceFunction:
			
			if (!pbEndNode) {
				fwprintf_s(poFile, L"<%s ", sClass.c_str());
				fwprintf_s(poFile, L"function=\"%s\">", sFunction.c_str());
				}
			else {
				if (sResult.length()>0) {
					fwprintf_s(poFile, L"<Result function=\"%s\"  value=\"%s\"/>", sFunction.c_str(), sResult.c_str());
					}
				fwprintf_s(poFile, L"</%s>", sClass.c_str());
				}
				
			break;

		case eTraceVariable:
			fwprintf_s(poFile, L"<Variable ");
			fwprintf_s(poFile, L"function=\"%s\" result=\"%s\"/>", sFunction.c_str(), sResult.c_str());
			break;
		};		
	}

//void CTraceNode::Write(FILE* poFile, bool pbEndNode) {
//
//	if (poFile==NULL) return;
//	
//	if (oChilds.size()>0) {
//
//		switch (eType) {
//		
//			case eTraceFunction:
//				fprintf_s(poFile, "<%s ", sClass.c_str());
//				break;
//
//			case eTraceVariable:
//				fprintf_s(poFile, "<Variable ");
//				break;
//
//			};		
//		
//		fprintf_s(poFile, "function=\"%s\" result=\"%s\">", sFunction.c_str(), sResult.c_str());
//
//		for (long lIndex=0; lIndex<(long)oChilds.size(); lIndex++) {
//
//			oChilds[lIndex]->Write(poFile);
//			}
//		
//		switch (eType) {
//		
//			case eTraceFunction:
//				fprintf_s(poFile, "</%s>", sClass.c_str());
//				break;
//
//			case eTraceVariable:
//				fprintf_s(poFile, "</Variable>");
//				break;
//
//			};
//		}
//	else {
//
//		switch (eType) {
//		
//			case eTraceFunction:
//				fprintf_s(poFile, "<%s ", sClass.c_str());
//				break;
//
//			case eTraceVariable:
//				fprintf_s(poFile, "<Variable ");
//				break;
//
//			}
//
//		fprintf_s(poFile, "name=\"%s\" result=\"%s\"/>", sFunction.c_str(), sResult.c_str());
//		}
//	}

CTraceNode* CTraceNode::AddNode(std::wstring psFunction) {
	
	CTraceNode *oNewNode;

	oNewNode = new CTraceNode(this);
	oNewNode->SetFunction(psFunction);

	oChilds.push_back(oNewNode);

	return oNewNode;
	}

void CTraceNode::SetResult(std::wstring psResult) {
	
	sResult = psResult;
}

void CTraceNode::SetFunction(std::wstring psFunction) {

	long lColonPosition;

	lColonPosition = (long)psFunction.find(L"::", 0);

	sClass = psFunction.substr(0, lColonPosition);
	sFunction = psFunction.substr(lColonPosition+2, psFunction.length()-2-lColonPosition);

	if (lColonPosition == psFunction.npos) sFunction = psFunction;
}

//-------------------------------------------------------------------------------------


CTracer::CTracer(CRegistry* poRegistry) {

	oRegistry = poRegistry;
	 
	bTrace = (bool)(oRegistry->ReadInt(L"Debug", L"DoTrace", 0)!=0);
	cTraceLevel = (eTraceLevel)oRegistry->ReadInt(L"Debug", L"Level", 1);
	bAlwaysTrace = false;
    //bTrace = false;

	oNode = NULL;
}

//-----------------------------------------------------------------------------

CTracer::~CTracer() {

	if (bAlwaysTrace || bTrace) {
		
		if (File!=NULL) {
    	
			if (oNode!=NULL) {
				oNode->Write(File);
				delete oNode;
				}

			fprintf_s(File, "</Debug>");
			fclose(File);
			}
		}
    }
//-----------------------------------------------------------------------------

void
CTracer::InitTracing(HINSTANCE phInstance) {

	WCHAR cText[MAX_PATH];
	WCHAR cMessage[MAX_PATH];

	if (bAlwaysTrace || bTrace) {

		bTrace = (bool)(DialogBoxParam(phInstance, MAKEINTRESOURCE(IDD_INITTRACE), NULL, TraceDialogProc, (LPARAM)oRegistry)!=0); 

		oRegistry->WriteInt(L"Debug", L"DoTrace", bTrace ? 1 : 0);

		if (bAlwaysTrace || bTrace) {
		
			MakeTraceFileName();
			
			if (_wfopen_s(&File, cTraceFileName.c_str(), L"w+")==0) {
	    	
				fprintf_s(File, "<Debug>");

				VersionInfo(phInstance);
				}
			else {

				//error opening file
				_wcserror_s(cMessage, MAX_PATH, errno);
				swprintf_s(cText, MAX_PATH, L"An error occurred while creating trace file '%s'.\n\n%s", cTraceFileName.c_str(), cMessage);

				MessageBox(NULL, cText, L"Error Copying Machine", MB_OK | MB_ICONERROR);
				}

			//if (MessageBox(NULL, L"Tracing is enabled. Are you sure you want to continue tracing?", L"Copying Machine", MB_YESNO)==IDNO) {

			//	oRegistry->WriteInt(L"Debug", L"DoTrace", 0);
			//	}
			}
		}
	}

//-----------------------------------------------------------------------------

void
CTracer::StartTrace(const wchar_t* cFunction, eTraceLevel eLevel) {

	if (cTraceLevel>=eLevel) {
		if (bAlwaysTrace || bTrace) {

			CTraceNode* oNewNode;

			if (oNode==NULL) {

				oNewNode = new CTraceNode(oNode);
				oNewNode->SetFunction(cFunction);

				oNode = oNewNode;
				oNewNode->Write(File);
				}
			else {
			
				oNode = oNode->AddNode(cFunction);
				oNode->Write(File);
				}
			}
        }
    }

//-----------------------------------------------------------------------------

void
CTracer::Add(const wchar_t* cFunction, int iReturn, eTraceLevel eLevel) {

	wchar_t cNumber[20];
	CTraceNode* oNewNode;

	if (cTraceLevel>=eLevel) {
		if (bAlwaysTrace || bTrace) {

			_itow_s(iReturn, cNumber, 20, 10);
			
			if (oNode==NULL) {

				oNewNode = new CTraceNode(oNode);
				oNewNode->SetFunction(cFunction);

				oNode = oNewNode;
				}
			else {
			
				oNewNode = oNode->AddNode(cFunction);
				}

			oNewNode->eType = eTraceNodeType::eTraceVariable;
			oNewNode->SetResult(cNumber);
			oNewNode->Write(File);
			}
		}
   }

//-----------------------------------------------------------------------------

void
CTracer::Add(const wchar_t* cFunction, bool bReturn, eTraceLevel eLevel) {

	CTraceNode* oNewNode;

	if (cTraceLevel>=eLevel) {
		if (bAlwaysTrace || bTrace) {

			if (oNode==NULL) {

				oNewNode = new CTraceNode(oNode);
				oNewNode->SetFunction(cFunction);
				

				oNode = oNewNode;
				}
			else {
			
				oNewNode = oNode->AddNode(cFunction);
				}

			oNewNode->eType = eTraceNodeType::eTraceVariable;
			oNewNode->SetResult(bReturn ? L"true" : L"false");
			oNewNode->Write(File);
			}
		}
   }

//-----------------------------------------------------------------------------
void	
CTracer::Add(const wchar_t* cFunction, const wchar_t* psReturn, eTraceLevel eLevel) {

	std::wstring sConvert;

	sConvert = psReturn;

	Add(cFunction, sConvert, eLevel);
	}

//-----------------------------------------------------------------------------
void
CTracer::Add(const wchar_t* cFunction, std::wstring cReturn, eTraceLevel eLevel) {

	CTraceNode* oNewNode;
	std::wstring sResult;

	if (cTraceLevel>=eLevel) {
		if (bAlwaysTrace || bTrace) {

			if (oNode==NULL) {

				oNewNode = new CTraceNode(oNode);
				oNewNode->SetFunction(cFunction);

				oNode = oNewNode;
				}
			else {
			
				oNewNode = oNode->AddNode(cFunction);
				}

			oNewNode->eType = eTraceNodeType::eTraceVariable;
			oNewNode->SetResult(cReturn);
			oNewNode->Write(File);
			}
		}
    }

//-----------------------------------------------------------------------------

void
CTracer::Add(const wchar_t* cFunction, eTraceLevel eLevel) {

	CTraceNode* oNewNode;

	if (cTraceLevel>=eLevel) {
		if (bAlwaysTrace || bTrace) {

			if (oNode==NULL) {

				oNewNode = new CTraceNode(oNode);
				oNewNode->SetFunction(cFunction);

				oNode = oNewNode;
				}
			else {
			
				oNewNode = oNode->AddNode(cFunction);
				}

			oNewNode->eType = eTraceNodeType::eTraceVariable;
			oNewNode->Write(File);
			}
		}
	}
//-----------------------------------------------------------------------------

void
CTracer::EndTrace(const wchar_t* cFunction, int iReturn, eTraceLevel eLevel) {

    wchar_t cNumber[20];

	if (cTraceLevel>=eLevel) {
		if (bAlwaysTrace || bTrace) {

			_itow_s(iReturn, cNumber, 20, 10);

			oNode->SetResult(cNumber);
			oNode->Write(File, true);
		
			if (oNode->oParent != NULL) {

				oNode = oNode->oParent;
				}
			else {

				delete oNode;

				oNode = NULL;
				}
			}
		}
   }

//-----------------------------------------------------------------------------

void
CTracer::EndTrace(const wchar_t* cFunction, bool bReturn, eTraceLevel eLevel) {

	if (cTraceLevel>=eLevel) {
		if (bAlwaysTrace || bTrace) {

			oNode->SetResult(bReturn ? L"true" : L"false");
			oNode->Write(File, true);
		
			if (oNode->oParent != NULL) {

				oNode = oNode->oParent;
				}
			else {

				delete oNode;

				oNode = NULL;
				}
			}
		}
   }

//-----------------------------------------------------------------------------
void	
CTracer::EndTrace(const wchar_t* cFunction, const wchar_t* psReturn, eTraceLevel eLevel) {

	std::wstring sConvert;

	sConvert = psReturn;

	EndTrace(cFunction, sConvert, eLevel);
}

//-----------------------------------------------------------------------------

void
CTracer::EndTrace(const wchar_t* cFunction, std::wstring cReturn, eTraceLevel eLevel) {

	if (cTraceLevel>=eLevel) {
		if (bAlwaysTrace || bTrace) {

			oNode->SetResult(cReturn);
			oNode->Write(File, true);
			
			if (oNode->oParent != NULL) {

				oNode = oNode->oParent;
				}
			else {

				delete oNode;

				oNode = NULL;
				}
			}
		}
    }

//-----------------------------------------------------------------------------

void
CTracer::EndTrace(const wchar_t* cFunction, eTraceLevel eLevel) {

	if (cTraceLevel>=eLevel) {
		if (bAlwaysTrace || bTrace) {

			oNode->Write(File, true);

			if (oNode->oParent != NULL) {

				oNode = oNode->oParent;
				}
			else {

				delete oNode;

				oNode = NULL;
				}
     		}
		}
	}
//-----------------------------------------------------------------------------

INT_PTR CALLBACK CTracer::TraceDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;
	WCHAR cItem[MAX_PATH];
	std::wstring sPath;
	BROWSEINFO cInfo;
	LPCITEMIDLIST pidl;
	int iTraceLevel;

	bReturn = FALSE;

	switch (uMsg) {

		case WM_INITDIALOG: {

			oDialogRegistry = (CRegistry*)lParam;

			//fill combobox
			wcscpy_s(cItem, MAX_PATH, L"Minimum");
			SendDlgItemMessage(hwndDlg, IDC_TRACELEVEL, CB_ADDSTRING, 0, (LPARAM)cItem);
			wcscpy_s(cItem, MAX_PATH, L"Normal (standard)");
			SendDlgItemMessage(hwndDlg, IDC_TRACELEVEL, CB_ADDSTRING, 0, (LPARAM)cItem);
			wcscpy_s(cItem, MAX_PATH, L"All");
			SendDlgItemMessage(hwndDlg, IDC_TRACELEVEL, CB_ADDSTRING, 0, (LPARAM)cItem);
			wcscpy_s(cItem, MAX_PATH, L"Extreme (not recommended)");
			SendDlgItemMessage(hwndDlg, IDC_TRACELEVEL, CB_ADDSTRING, 0, (LPARAM)cItem);

			iTraceLevel = oDialogRegistry->ReadInt(L"Debug", L"Level", 1);
			SendDlgItemMessage(hwndDlg, IDC_TRACELEVEL, CB_SETCURSEL, iTraceLevel, (LPARAM)0);

			if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, 
				NULL, 0, cItem))) {

					wcscpy_s(cItem, MAX_PATH, L"c:\\");
					}

			oDialogRegistry->ReadString(L"Debug", L"Path", cItem, sPath);

			SendDlgItemMessage(hwndDlg, IDC_EDITTRACEFILE, WM_SETTEXT, 0, (LPARAM)sPath.c_str());
			
			bReturn = TRUE;
			break;
			}
		case WM_COMMAND: {

			switch (LOWORD(wParam)) {

				case IDYES: {

					//Save settings
					SendDlgItemMessage(hwndDlg, IDC_EDITTRACEFILE, WM_GETTEXT, sizeof(cItem), (LPARAM)cItem);
					sPath = cItem;
					oDialogRegistry->WriteString(L"Debug", L"Path", sPath);

					iTraceLevel = (int)SendDlgItemMessage(hwndDlg, IDC_TRACELEVEL, CB_GETCURSEL, 0, (LPARAM)0);
					if (iTraceLevel!=CB_ERR) oDialogRegistry->WriteInt(L"Debug", L"Level", iTraceLevel);

					//Close dialog
					EndDialog(hwndDlg, true);
					bReturn = TRUE;
					break;
					}
				case IDNO: {

					EndDialog(hwndDlg, false);
					bReturn = TRUE;
					break;
					}

				case IDC_BROWSE: {

		
					ZeroMemory(&cInfo, sizeof(cInfo));
					cInfo.hwndOwner = hwndDlg;
					cInfo.pidlRoot = NULL;
					cInfo.pszDisplayName = cItem;
					cInfo.lpszTitle = L"Select folder";
					cInfo.ulFlags = BIF_NEWDIALOGSTYLE;

					pidl = SHBrowseForFolder(&cInfo);
					if (pidl!=NULL) {

						SHGetPathFromIDList(pidl, cItem);
						SendDlgItemMessage(hwndDlg, IDC_EDITTRACEFILE, WM_SETTEXT, 0, (LPARAM)cItem);
						}
					}
				}

			break;
			}
		}

	return bReturn;
	}

//this is the container function for adding version info of the OS
//and the application to the trace file

void CTracer::VersionInfo(HINSTANCE phInstance) {

	StartTrace(__WFUNCTION__, eTraceLevel::eMinimum);

	//Add windows version
	AddWindowsInfo();
	AddProductVersion(phInstance);

	EndTrace(__WFUNCTION__, eTraceLevel::eMinimum);
}

//this function adds version info of the OS to the trace file

void CTracer::AddWindowsInfo() {

   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfo;

   // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
   // If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	bOsVersionInfo = GetVersionEx ((OSVERSIONINFO *) &osvi);
	if (!bOsVersionInfo) bOsVersionInfo = GetVersionEx ((OSVERSIONINFO *) &osvi);

	if (bOsVersionInfo) {
		
		Add(L"WindowsPlatformWin32NT", (bool)(osvi.dwPlatformId==VER_PLATFORM_WIN32_NT), eTraceLevel::eMinimum);			
		Add(L"WindowsMajorVersion", (int)osvi.dwMajorVersion, eTraceLevel::eMinimum);			
		Add(L"WindowsMinorVersion", (int)osvi.dwMinorVersion, eTraceLevel::eMinimum);
		Add(L"ServicePack", osvi.szCSDVersion, eTraceLevel::eMinimum);
		}

	SYSTEM_INFO sysinfo;

	GetSystemInfo( &sysinfo );
	
	Add(L"NumberPhysicalProcessors", (int)sysinfo.dwNumberOfProcessors);
	}
	

//This function add the version info of the application to the trace file

void CTracer::AddProductVersion(HINSTANCE phInstance) {

	DWORD dwVerInfoSize;
	DWORD dwHnd;
	void* pBuffer;
	VS_FIXEDFILEINFO *pFixedInfo; // pointer to fixed file info structure
	UINT    uVersionLen;   // Current length of full version string
	TCHAR szExeName[MAX_PATH];
	wchar_t szProductVersion[50];
	
	if (phInstance==NULL) return;
	
	GetModuleFileName(phInstance, szExeName, sizeof (szExeName));
	dwVerInfoSize = GetFileVersionInfoSize(szExeName, &dwHnd);
	
	if (dwVerInfoSize) {
		  
		pBuffer = malloc(dwVerInfoSize);
		if (pBuffer == NULL) return;

		GetFileVersionInfo(szExeName, dwHnd, dwVerInfoSize, pBuffer);
		// get the fixed file info (language-independend) 
		VerQueryValue(pBuffer,_T("\\"),(void**)&pFixedInfo,(UINT *)&uVersionLen);
		  
		  
		swprintf_s(szProductVersion, 50, L"%u,%u,%u,%u", HIWORD (pFixedInfo->dwProductVersionMS),
						  LOWORD (pFixedInfo->dwProductVersionMS),
						  HIWORD (pFixedInfo->dwProductVersionLS),
						  LOWORD (pFixedInfo->dwProductVersionLS));
		  
		Add(L"CopyingMachine", szProductVersion, eTraceLevel::eMinimum);
		}
	}

//this function generates a unique name for the tracefile. It will have a
//format like 'copyingmachine_yyyymmdd_nnn.log'. The location will be
//the trace folder as chosen by the user or the default 'My Documents'.

void CTracer::MakeTraceFileName() {

	std::wstring cTracePath;

	WCHAR sFilename[MAX_PATH];
	WCHAR sTracePath[MAX_PATH];
	WCHAR sDate[10];
	long lIterator;
	intptr_t hFile;
	struct _wfinddata64i32_t c_file;
	struct tm today;
	bool bContinue;
	
	oRegistry->ReadString(L"Debug", L"Path", L"c:\\", cTracePath);

	lIterator = 1;
	bContinue = false;

	//get the current date in the yyyymmdd format
	time_t ltime;
	time(&ltime);
	localtime_s(&today, &ltime);
 	wcsftime(sDate, 10, L"%Y%m%d", &today );

	//keep trying to find a unique filename
	do {

		swprintf_s(sFilename, MAX_PATH, L"copyingmachine_trace_%s_%03d.xml", sDate, lIterator);

		wcscpy_s(sTracePath, MAX_PATH, cTracePath.c_str());
		PathAppend(sTracePath, sFilename);

		hFile = _wfindfirst(sTracePath, &c_file);
		bContinue = (hFile==-1);
		_findclose( hFile );

		lIterator++;

		} while (!bContinue);

	cTraceFileName = sTracePath;
	}
