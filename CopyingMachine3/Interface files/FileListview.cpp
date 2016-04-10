#include "StdAfx.h"
#include "Listview.h"
#include "FileListview.h"
#include "CopyingMachineCore.h"

CFileListview::CFileListview(sGlobalInstances pInstances, std::wstring psWindowName) : 
	CListview(pInstances, psWindowName), CThread(true)
{

	oFileList = new CFileList(pInstances);

	Resume();
	}

CFileListview::~CFileListview(void)
{
	Terminate();
	WaitFor();
	delete oFileList;

	}

//The execution function for the thread
void __fastcall 
CFileListview::Execute() {

	while (!IsTerminating()) {

		Sleep(100);

		if (oFileList->GetUpdated()) {

			oFileList->Suspend();

			DoUpdateFiles();

			oFileList->Resume();
			}
		}

	//Signal that the thread is completed executing
	Completed();
}

//This function creates the filelistview its window
HWND 
CFileListview::Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra) {

	long lWindowStyle;
	
	//Create the listview
	CListview::Create(phParent, phInstance, pdStyleExtra | LVS_OWNERDATA | LVS_REPORT, (HMENU)1009);

	//Initialise the window
	lWindowStyle = GetWindowLong(hWnd, GWL_STYLE);
	//lWindowStyle |= WS_BORDER;
	SetWindowLong(hWnd, GWL_STYLE, lWindowStyle);

	//Add the columns
	SetView(LV_VIEW_DETAILS);
	InsertColumn(0, IDS_NAME, 200);
	InsertColumn(1, IDS_SIZE, 100, LVCFMT_RIGHT);
	InsertColumn(2, IDS_MODIFIED, 100);
	InsertColumn(3, IDS_TYPE, 100);

	//Add the imagelists
	SHFILEINFO  sfi;
	hSmallImagelist = (HIMAGELIST)SHGetFileInfo(TEXT("C:\\"), 0, &sfi,
		sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	hSmallImagelist = oGlobalInstances.hSmallFileIcons;
	bDestroySmallImagelist = false; //Don't destroy this handle after switching to easy mode. It will fail after trying to recreate it.

	//hNormalImagelist = (HIMAGELIST)SHGetFileInfo(TEXT("C:\\"), 0, &sfi,
	//	sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
	hNormalImagelist = oGlobalInstances.hNormalFileIcons;
	bDestroyNormalImagelist = false;

	ListView_SetImageList(hWnd, hSmallImagelist, LVSIL_SMALL);
	ListView_SetImageList(hWnd, hSmallImagelist, LVSIL_NORMAL);

	oFileList->Resume();

	//return the window handle
	return hWnd;
}

//This is the message handler for the listview
bool 
CFileListview::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {

	bool bReturn;
	LPNMHDR cNotificationHeader;
	LPNMLISTVIEW cNotificationListview;

	bReturn = false;

	if (!bReturn) {

		switch (message) {

			case WM_NOTIFY:

				cNotificationHeader = (LPNMHDR)lParam;
				
				if (cNotificationHeader->hwndFrom == hWnd) {

					cNotificationListview = (LPNMLISTVIEW)lParam;
							
					switch (cNotificationHeader->code) {

						case LVN_GETDISPINFO: //request to provide information to the virtual listview

							OnGetDisplayInfo((NMLVDISPINFO*)lParam);					
							break;
						}
					}
				break;
			}
		}

	if (!bReturn) {
		bReturn = CListview::WndProc(phWnd, message, wParam, lParam, piReturn);
		}
	
	return bReturn;
	}

//This function localize the columns of the listview to the current
//language
void 
CFileListview::Localize(HINSTANCE phLanguage) {

	oGlobalInstances.hLanguage = phLanguage;

	SetColumnTitle(0, IDS_NAME);
	SetColumnTitle(1, IDS_SIZE);
	SetColumnTitle(2, IDS_MODIFIED);
	SetColumnTitle(3, IDS_TYPE);
	}



//This function updates the listview so it represents the status
//of the filelist
void
CFileListview::DoUpdateFiles() {

	long lFiles = oFileList->GetFileCount();

	ListView_SetItemCountEx(hWnd, lFiles, 0);
}

//This function retrieves the requested informatie for the listview
void 
CFileListview::OnGetDisplayInfo(NMLVDISPINFO* pcDisplayInfo) {

	CFile* oFile;

	//if (ListView_IsItemVisible(hWnd, pcDisplayInfo->item.iItem)) {

		oFile = oFileList->GetFile(pcDisplayInfo->item.iItem);
		
		if (oFile) {

			oFile->GetFileInfo(oFileList->GetFolder());	

			//Process the image request
			if (pcDisplayInfo->item.mask & LVIF_IMAGE) {

				pcDisplayInfo->item.iImage = oFile->iImage;

				}	
				
			//Process the text request
			if (pcDisplayInfo->item.mask & LVIF_TEXT) {

				if (oFile!=NULL) {

					switch (pcDisplayInfo->item.iSubItem) {

						case 0: //item
							pcDisplayInfo->item.pszText = (LPWSTR)oFile->sName.c_str();
							break;

						case 1: //size
							
							pcDisplayInfo->item.pszText = (LPWSTR)oFile->sSize.c_str();
							break;

						case 2: //modification datetime
							
							pcDisplayInfo->item.pszText = (LPWSTR)oFile->sDateTime.c_str();
							break;

						case 3: //type
							pcDisplayInfo->item.pszText = (LPWSTR)oFile->sType.c_str();
							break;


						default:
							pcDisplayInfo->item.pszText = L"sub";
							break;
						}
					}
				}
			}
	//	}
	}

//This function is called when the column is clicked
void 
CFileListview::OnColumnClick(int piColumn) {

	switch (piColumn) {

		case 0: //name

			oFileList->SetSort(eSortName);
			break;

		case 1: //size

			oFileList->SetSort(eSortSize);
			break;

		case 2: //date

			oFileList->SetSort(eSortDate);
			break;

		case 3: //type

			oFileList->SetSort(eSortType);
			break;
		}

	DoSetSortMark();
	}

//This function will open the requested item
void 
CFileListview::DoOpenFile(int piItem) {

	CFile* oFile;
	std::wstring sFile;
	CCopyingMachineCore* oCore;

	oFile = oFileList->GetFile(piItem);

	sFile = oFileList->GetFolder();
	sFile += L"\\";
	sFile += oFile->sName;

	if (oFile->IsFolder()) {

		SetFolder(sFile, false);
		}
	else {

		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

		oCore->LoadDocument(hWnd, sFile);
		}
	}

//This function is called when an item is double clicked
void 
CFileListview::OnButtonDoubleClick(int piItem) {

	DoOpenFile(piItem);
	}


//Set the current folder in the listview
void 
CFileListview::SetFolder(std::wstring psFolder, 
						 bool pbClearHistory) {
	
	//Save the current folder to the history
	sFolderHistory.push_back(oFileList->GetFolder());

	//Set a new folder
	oFileList->SetFolder(psFolder);

	//Send a notification of the folder change
	CCopyingMachineCore* oCore;
	CCoreNotifications* oNotifications;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oNotifications = oCore->oNotifications;
	oNotifications->CommunicateEvent(eNotificationDocumentCollectionFolderSelect, 0);

	//Clear the history if requested
	if (pbClearHistory) {
		sFolderHistory.clear();
		}
	}

//Retrieve the current folder
std::wstring
CFileListview::GetFolder() {

	return oFileList->GetFolder();
	}

//Is folder history present
bool 
CFileListview::FolderHistory() {

	return sFolderHistory.size()>0;
}

//Go back a folder
void 
CFileListview::DoFolderBack() {

	if (FolderHistory()) {

		oFileList->SetFolder(sFolderHistory[sFolderHistory.size()-1]);
		sFolderHistory.erase(sFolderHistory.begin() + sFolderHistory.size() - 1);
		
		//Send a notification of the folder change
		CCopyingMachineCore* oCore;
		CCoreNotifications* oNotifications;

		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		oNotifications = oCore->oNotifications;
		oNotifications->CommunicateEvent(eNotificationDocumentCollectionFolderSelect, 0);
		}
	}

//Set the allowed files
void 
CFileListview::SetAllowedFiles(cAllowedFiles poAllowedFiles) {

	oFileList->SetAllowedFiles(poAllowedFiles);
	}

//Save settings
void
CFileListview::SaveSettings() {

	//Save the width of the columns
	oGlobalInstances.oRegistry->WriteInt(L"Window", L"FLCName", GetColumnWidth(0));
	oGlobalInstances.oRegistry->WriteInt(L"Window", L"FLCSize", GetColumnWidth(1));
	oGlobalInstances.oRegistry->WriteInt(L"Window", L"FLCModified", GetColumnWidth(2));
	oGlobalInstances.oRegistry->WriteInt(L"Window", L"FLCType", GetColumnWidth(3));

	oGlobalInstances.oRegistry->WriteInt(L"Window", L"FLCSort", (int)oFileList->GetSort());
	oGlobalInstances.oRegistry->WriteInt(L"Window", L"FLCReverse", oFileList->GetSortReverse() ? 1 : 0);

	CWindowBase::SaveSettings();
	}

//Load settings
void
CFileListview::LoadSettings() {

	SetColumnWidth(0, oGlobalInstances.oRegistry->ReadInt(L"Window", L"FLCName", 200));
	SetColumnWidth(1, oGlobalInstances.oRegistry->ReadInt(L"Window", L"FLCSize", 100));
	SetColumnWidth(2, oGlobalInstances.oRegistry->ReadInt(L"Window", L"FLCModified", 100));
	SetColumnWidth(3, oGlobalInstances.oRegistry->ReadInt(L"Window", L"FLCType", 100));

	oFileList->SetSort((eFileSort)oGlobalInstances.oRegistry->ReadInt(L"Window", L"FLCSort", (int)eSortName));
	oFileList->SetSortReverse(oGlobalInstances.oRegistry->ReadInt(L"Window", L"FLCReverse", 0)==1);

	DoSetSortMark();

	CWindowBase::LoadSettings();
	}

//This function sets a sort image in the column header
void 
CFileListview::DoSetSortMark() {

	HWND hFileHeader; 
	HDITEM cHeaderItem = {0};
	int iHeaderItems;
	eFileSort cFileSort;

	hFileHeader = ListView_GetHeader(hWnd);
	cFileSort = oFileList->GetSort();

	cHeaderItem.mask = HDI_FORMAT;
	iHeaderItems = Header_GetItemCount(hFileHeader);

	while (iHeaderItems>0) {

		iHeaderItems--;
	
		Header_GetItem(hFileHeader, iHeaderItems, &cHeaderItem);

		cHeaderItem.fmt &= ~(HDF_SORTDOWN|HDF_SORTUP);

		if ((iHeaderItems==0 && cFileSort==eSortName) ||
		    (iHeaderItems==1 && cFileSort==eSortSize) ||
		    (iHeaderItems==2 && cFileSort==eSortDate) ||
		    (iHeaderItems==3 && cFileSort==eSortType)) {

			cHeaderItem.fmt |= oFileList->GetSortReverse() ? HDF_SORTDOWN : HDF_SORTUP;
			}

		Header_SetItem(hFileHeader, iHeaderItems, &cHeaderItem);
		}
	}


