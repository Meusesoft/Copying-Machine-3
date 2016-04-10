#include "StdAfx.h"
#include "CopyFileListview.h"
#include "CopyingMachineCore.h"

CCopyFileListview::CCopyFileListview(sGlobalInstances pInstances, 
									std::wstring psWindowName) : 
	CFileListview(pInstances, psWindowName)
{
	//Set the allowed file types
	cAllowedFiles oAllowedFiles;
	oAllowedFiles.bFolders = true;
	oAllowedFiles.bHidden = false;
	oAllowedFiles.sFileTypes = L".jpg|.jpeg|.tiff|.tif|.bmp|.pcx|.png|.pdf|.gif|";
	SetAllowedFiles(oAllowedFiles);
}

CCopyFileListview::~CCopyFileListview(void)
{

}


//This function creates the filelistview its window
HWND 
CCopyFileListview::Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra) {

	//Create the listview
	CFileListview::Create(phParent, phInstance, pdStyleExtra);

	//return the window handle
	return hWnd;
	}

//Show the context menu
bool 
CCopyFileListview::DoContextMenu(int piItem, HMENU hContextMenu) {

	wchar_t sMenuItem[100];
	bool bResult;
	CFile* oFile;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	//The contents of the menu is dependent of the existence of a selection and where the cursor is hovering
	if (ListView_GetSelectedCount(hWnd)==1) {

		oFile = oFileList->GetFile(piItem);

		if (oFile->IsFolder()) {

			LoadString(oGlobalInstances.hLanguage, IDS_NEWDOCUMENT, sMenuItem, 99);
			AppendMenu(hContextMenu, MF_STRING, IDC_ACQUIRETOCOLLECTIONFOLDER, sMenuItem);
			LoadString(oGlobalInstances.hLanguage, IDS_EXPLORE, sMenuItem, 99);
			AppendMenu(hContextMenu, MF_STRING, IDC_EXPLOREFOLDER, sMenuItem);

			bResult = true;
			}

		if (!oFile->IsFolder()) {

			LoadString(oGlobalInstances.hLanguage, IDS_OPEN, sMenuItem, 99);
			AppendMenu(hContextMenu, MF_STRING, IDC_OPENFILEFROMCOLLECTION, sMenuItem);

			bResult = true;
			}
		}

	oTrace->EndTrace(__WFUNCTION__);

	return bResult;
	}

//Refresh the current view
void
CCopyFileListview::Refresh() {

	oFileList->SetRefresh(true);
	}


//Show the context menu
bool 
CCopyFileListview::DoContextMenu(HMENU hContextMenu) {

	wchar_t sMenuItem[100];
	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;

	LoadString(oGlobalInstances.hLanguage, IDS_NEWDOCUMENT, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, IDC_ACQUIRETOCOLLECTIONFOLDER, sMenuItem);
	LoadString(oGlobalInstances.hLanguage, IDS_EXPLORE, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, IDC_EXPLORE, sMenuItem);
	AppendMenu(hContextMenu, MF_SEPARATOR, 0, 0);
	LoadString(oGlobalInstances.hLanguage, IDS_REFRESH, sMenuItem, 99);
	AppendMenu(hContextMenu, MF_STRING, IDC_REFRESHFILELIST, sMenuItem);

	oTrace->EndTrace(__WFUNCTION__);

	return bResult;
	}

//This function returns the selected folder
std::wstring 
CCopyFileListview::GetSelectedFolder() {

	std::wstring sResult;
	int iItem;
	CFile* oFile;

	sResult = GetFolder();

	iItem = GetSelectedItem();

	if (iItem!=-1) {

		oFile = oFileList->GetFile(iItem);

		if (oFile->IsFolder()) {

			sResult += L"\\";
			sResult += oFile->sName;
			}
		}

	return sResult;
	}




