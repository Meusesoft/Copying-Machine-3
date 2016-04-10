#include "StdAfx.h"
#include "FileList.h"

bool 
FileSortName(CFile* lhs, CFile* rhs)
{
	return lhs->Compare(rhs, eSortName);
}

bool 
FileSortSize(CFile* lhs, CFile* rhs)
{
	return lhs->Compare(rhs, eSortSize);
}

bool 
FileSortDate(CFile* lhs, CFile* rhs)
{
	return lhs->Compare(rhs, eSortDate);
}

bool 
FileSortType(CFile* lhs, CFile* rhs)
{
	return lhs->Compare(rhs, eSortType);
}

//Class CFile

CFile::CFile() {

	//Clean up the structure
	SecureZeroMemory(&cNumberFormat, sizeof(NUMBERFMT));

	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, sThousand, 4);
	cNumberFormat.NumDigits = 0;
	cNumberFormat.lpDecimalSep = L"";
	cNumberFormat.lpThousandSep = sThousand;
	cNumberFormat.Grouping = 3;
	cNumberFormat.LeadingZero = 0;
	cNumberFormat.NegativeOrder = 1;

	bFileInfoPresent = false;
	}

CFile::~CFile() {
	}

bool 
CFile::Compare(CFile* poCompareWith, eFileSort cFileSort) {

	bool bResult;

	if (IsFolder() && !poCompareWith->IsFolder()) return true;
	if (!IsFolder() && poCompareWith->IsFolder()) return false;

	bResult = false;

	switch (cFileSort) {

		case eSortName:

			bResult = (CompareName(poCompareWith) < 0);
			break;	

		case eSortSize:

			if (dSize == poCompareWith->dSize) {
				bResult = (CompareName(poCompareWith) < 0);
				}
			else {
				bResult = (dSize < poCompareWith->dSize);
				}
			break;	

		case eSortType:

			if (sType == poCompareWith->sType) {
				bResult = (CompareName(poCompareWith) < 0);
				}
			else {
				bResult = (sType < poCompareWith->sType);
				}
			break;	

		case eSortDate:

			if (dModifiedDateTime == poCompareWith->dModifiedDateTime) {
				bResult = (CompareName(poCompareWith) < 0);
				}
			else {
				bResult = (dModifiedDateTime < poCompareWith->dModifiedDateTime);
				}
			break;	
		}

	return bResult;
	}

bool 
CFile::IsFolder() {

	return ((dAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
	}

bool
CFile::IsHidden() {

	return ((dAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)) != 0);
	}

//This function returns the extension of the file
std::wstring 
CFile::GetExtension() {

	std::wstring sResult;
	std::wstring::size_type iIndex;

	sResult = L"";

	//find the last dot in the filename
	iIndex = sName.find_last_of(L".");

	//get the extension
	if (iIndex != std::wstring::npos) {

		sResult = sName.substr(iIndex, sName.size() - iIndex);
		}

	//to lowercase
	std::wstring::iterator  p = sResult.begin(); 

    while (p != sResult.end()) { 
        
		*p = tolower(*p);
        ++p;
		}

	return sResult;
	}

int
CFile::CompareName(CFile* poCompareWith) {
 
	std::wstring::iterator  p = sName.begin(); 
    std::wstring::iterator p2 = poCompareWith->sName.begin();

    while (p != sName.end() && p2 != poCompareWith->sName.end()) { 
        if (toupper(*p) != toupper(*p2)) 
            return (toupper(*p) < toupper(*p2)) ? -1 : 1; 
        ++p;
		++p2; 
		}

    return poCompareWith->sName.size() - sName.size(); 
	}

//This function formats the filesize according to the local settings
//in the windows file system
void 
CFile::FormatFileSize() {

	TCHAR cNumber[50];
	TCHAR cFormattedNumber[50];
	
	sSize = L"";

	if (!IsFolder()) {

		swprintf_s(cNumber, 50, L"%d", dSize / 1024);

		GetNumberFormat(LOCALE_USER_DEFAULT, 0, cNumber, &cNumberFormat, 
						cFormattedNumber, 50);

		sSize = cFormattedNumber;
		sSize += L" kB";
		}
	}

//This function formats the file date/time to the
//local format of the user
void 
CFile::FormatFileDateTime() {

	FILETIME   cFileDateTime;
	SYSTEMTIME cSystemDateTime;
	TCHAR cDateTimeString[128];

	sDateTime = L"";
	cFileDateTime.dwLowDateTime = dModifiedDateTime & 0xFFFFFFFF;
	cFileDateTime.dwHighDateTime = dModifiedDateTime >> 32 ;

	//Convert filetime to systemtime
	FileTimeToSystemTime(&cFileDateTime, &cSystemDateTime);

	//Process date
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &cSystemDateTime, NULL, cDateTimeString, 128);

	sDateTime = cDateTimeString;

	//Process time
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &cSystemDateTime, NULL, cDateTimeString, 128);
	
	sDateTime += L" ";
	sDateTime += cDateTimeString;
	}	

//This function retrieves information about the file in the sFile instance
void 
CFile::GetFileInfo(std::wstring psFolder) {

	SHFILEINFO cSHFileInfo;
	std::wstring sFile;

	if (!bFileInfoPresent) {

		sFile = psFolder;
		sFile += L"\\";
		sFile += sName;

		SHGetFileInfo(sFile.c_str(),
			dAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NORMAL),
			&cSHFileInfo,
			sizeof(SHFILEINFO),
			SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_TYPENAME);

		iImage = cSHFileInfo.iIcon;
		sType = cSHFileInfo.szTypeName;
		
		bFileInfoPresent = true;
		}
	}


//Class CFileList

CFileList::CFileList(sGlobalInstances pInstances) :
	CThread(true)
{
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &pInstances, sizeof(sGlobalInstances));
	
	bUpdated = false;
	sFolder = L".";	

	oAllowedFiles.sFileTypes = L"";
	oAllowedFiles.bFolders = true;
	oAllowedFiles.bHidden = false;

	ResetIterator();

	cFileSort = eSortName;
	bSortReverse = false;
}

CFileList::~CFileList(void)
{
	Terminate();
	WaitFor();
	Clear();
}

//The execution function for the thread
void __fastcall 
CFileList::Execute() {

	while (!IsTerminating()) {

		if (!GetSynchronizationInProgress() || GetRefresh()) {

			Synchronize();
			}
		else {
	
			Sleep(500);
			}
		}
		
	Completed();
	}

//This function sets the folder for the filelist
void 
CFileList::SetFolder(std::wstring psFolder) {

	Clear();

	sFolder = psFolder;
	}

//Retrieve the current folder
std::wstring
CFileList::GetFolder() {

	return sFolder;
	}

//Refresh the current filelist
void
CFileList::SetRefresh(bool pbValue ) {

	EnterCriticalSection();

	bRefresh = pbValue;

	LeaveCriticalSection();
	}

//Refresh the current filelist
bool
CFileList::GetRefresh() {

	bool bResult;

	EnterCriticalSection();

	bResult = bRefresh;

	LeaveCriticalSection();

	return bResult;
	}


//This function sets the information for the allowed files
void 
CFileList::SetAllowedFiles(cAllowedFiles pcAllowedFiles) {

	Clear();

	oAllowedFiles.bFolders = pcAllowedFiles.bFolders;
	oAllowedFiles.bHidden = pcAllowedFiles.bHidden;
	oAllowedFiles.sFileTypes = pcAllowedFiles.sFileTypes;
	}

//This function checks if the given file is allowed according to the
//settings. 
bool 
CFileList::IsAllowed(CFile* poFile) {

	bool bResult;
	std::wstring sExtension;

	bResult = true;
	
	//Is it a valid file/directory? Not . or ..?
	if (poFile->sName == L"." || poFile->sName == L"..") bResult = false;

	//Check the hidden en folder attributes
	if (!oAllowedFiles.bFolders) {

		bResult = (bResult && !poFile->IsFolder());
		}
	if (!oAllowedFiles.bHidden) {
		
		bResult = (bResult && !poFile->IsHidden());
		}
	//Check the extension
	sExtension = poFile->GetExtension();
	if (sExtension.size()>0) {
		sExtension += L"|";
		if (oAllowedFiles.sFileTypes.find(sExtension)==std::wstring::npos) {
			bResult = false;
			}
		}

	return bResult;
	}


//This function clears the memory of the filelist and cancels
//the synchronization in the process
void
CFileList::Clear() {

	EnterCriticalSection();

	SetSynchronizationInProgress(false);

	//Delete all instances
	CFile* oFile;

	if (!oFiles.empty()) {
		
		FileIterator = oFiles.begin();
		iFileIteratorIndex = 0;

		do {
			oFile = *FileIterator;
			delete oFile;
			FileIterator++;
			}
		while (FileIterator!=oFiles.end());
		}

	//clear the list
	oFiles.clear();

	//reset the iterator
	FileIterator = oFiles.begin();
	iFileIteratorIndex = 0;

	LeaveCriticalSection();	
	}

//This function synchronizes the contents of the files in memory with
//the folder on the drive
void
CFileList::Synchronize() {

	CFile* oFile;
	WIN32_FIND_DATA cFindData;
	std::wstring sSearchString;
	HANDLE hFindHandle;

	SetSynchronizationInProgress(true);
	SetRefresh(false);

	sSearchString = sFolder;
	sSearchString += L"\\*.*";

	//Set all the refresh flags of the files to false
	DoClearRefreshFlag();

	//loop through all the files in the selected folder
	hFindHandle = FindFirstFile((LPCWSTR)sSearchString.c_str(), &cFindData);

	if (hFindHandle!=INVALID_HANDLE_VALUE) {

		do {

			oFile = new CFile();

			oFile->sName		= cFindData.cFileName;
			oFile->dAttributes	= cFindData.dwFileAttributes;
			oFile->bRefresh		= false;  
				
			if (AddFileToList(oFile)) {
				
				oFile->bRefresh = true;
				oFile->dSize = cFindData.nFileSizeHigh;
				oFile->dSize = oFile->dSize << 32;
				oFile->dSize += cFindData.nFileSizeLow;
				oFile->FormatFileSize();
				
				oFile->dModifiedDateTime = cFindData.ftLastWriteTime.dwHighDateTime;
				oFile->dModifiedDateTime = oFile->dModifiedDateTime << 32;
				oFile->dModifiedDateTime +=	cFindData.ftLastWriteTime.dwLowDateTime;
				oFile->FormatFileDateTime();		
				
				EnterCriticalSection();

				ResetIterator();

				bUpdated = true;

				LeaveCriticalSection();
				}
			else {

				delete oFile;
				}

		} while (FindNextFile(hFindHandle, &cFindData) && GetSynchronizationInProgress());

		FindClose(hFindHandle);
		}	

	if (GetSynchronizationInProgress()) {
		
		DoRemoveUnrefreshedFiles();
		DoSort();
		}
	}

//This function adds a file to the filelist. It checks
//if the file can be added (visible or valid file) and
//if the file isn't already present in the filelist (check on the name)
bool
CFileList::AddFileToList(CFile* poFile) {

	bool bResult;
	CFile* oFile;

	bResult = true;
	
	bResult = IsAllowed(poFile);

	//Add the file
	if (bResult) {

		oFile = FindFile(poFile->sName); 

		if (oFile==NULL) {

			EnterCriticalSection();

			oFiles.push_back(poFile);

			//reset the iterator
			ResetIterator();

			LeaveCriticalSection();
			}
		else {

			//File already existed in the file list, and it 
			//should stay.
			oFile->bRefresh = true;

			bResult = false;
			}
		}

	return bResult;
	}

//This functions set the synchronization in progress flag
void
CFileList::SetSynchronizationInProgress(bool pbValue) {

	EnterCriticalSection();

	bSynchronizeInProgress = pbValue;

	LeaveCriticalSection();
}

//This functions set the synchronization in progress flag
bool
CFileList::GetSynchronizationInProgress() {

	bool bResult;

	EnterCriticalSection();

	bResult = bSynchronizeInProgress;

	LeaveCriticalSection();

	return bResult;
}

//This function returns true if the updated signal is set and it
//also reset the signal to false
bool 
CFileList::GetUpdated() {

	bool bResult;
	
	EnterCriticalSection();

	bResult = bUpdated;

	bUpdated = false;

	LeaveCriticalSection();

	return bResult;
}

//This function returns the number of items in the filelist
int 
CFileList::GetFileCount() {

	return (int)oFiles.size();
}

//This function returns the request file
CFile* 
CFileList::GetFile(int piIndex) {

	CFile* oResult;

	oResult = NULL;

	if (piIndex>=0 && piIndex<(int)oFiles.size()) {

		while (piIndex != iFileIteratorIndex) {

			if (piIndex > iFileIteratorIndex) {

				FileIterator++;
				iFileIteratorIndex++;
				}
			else {

				FileIterator--;
				iFileIteratorIndex--;
				}
			}

		oResult = *FileIterator;
		}

	return oResult;
	}

//This function reset the file iterator
void 
CFileList::ResetIterator() {

	FileIterator = oFiles.begin();
	iFileIteratorIndex = 0;
	}

//This function updates the sort type of the filelist
void
CFileList::SetSort(eFileSort pcFileSort) {

	if (cFileSort == pcFileSort) {

		bSortReverse = !bSortReverse;
		}
	else {
		
		bSortReverse = false;
		cFileSort = pcFileSort;
		}

	DoSort();
	}

//Return the current sort type
eFileSort 
CFileList::GetSort() {

	return cFileSort;
	}

//Return the current sort reverse
bool 
CFileList::GetSortReverse() {

	return bSortReverse;
	}

//Set the sort reverse
void 
CFileList::SetSortReverse(bool pbSortReverse) {

	bSortReverse = pbSortReverse;
	}

//This function sorts the file list
void
CFileList::DoSort() {

	EnterCriticalSection();

	switch (cFileSort) {

		case eSortName:

			oFiles.sort(FileSortName);
			break;
		
		case eSortSize:

			oFiles.sort(FileSortSize);
			break;

		case eSortDate:

			oFiles.sort(FileSortDate);
			break;

		case eSortType:

			oFiles.sort(FileSortType);
			break;
		}

	if (bSortReverse) oFiles.reverse();

	ResetIterator();

	bUpdated = true;

	LeaveCriticalSection();
	}

//This function clears the refresh flag of all
//the files in the filelist
void 
CFileList::DoClearRefreshFlag() {

	CFile* oFile;

	ResetIterator();

	while (FileIterator != oFiles.end()) {

		oFile = *FileIterator;
		oFile->bRefresh = false;
		
		FileIterator++;
		}

	ResetIterator();
	}

//Find the file in the filelist
CFile* 
CFileList::FindFile(std::wstring psName) {

	CFile* oResult;

	oResult = NULL;

	ResetIterator();

	while (FileIterator != oFiles.end() && oResult==NULL) {

		oResult = *FileIterator;
		
		if (oResult->sName != psName) {

			oResult = NULL;
			}

		FileIterator++;
		}

	ResetIterator();

	return oResult;
	}

//Remove unrefreshed files
void 
CFileList::DoRemoveUnrefreshedFiles() {

	CFile* oFile;

	ResetIterator();

	while (FileIterator != oFiles.end()) {

		oFile = *FileIterator;
		
		if (!oFile->bRefresh) {

			delete oFile;
			FileIterator = oFiles.erase(FileIterator);
			}

		if (FileIterator != oFiles.end()) ++FileIterator;
		}

	ResetIterator();
	}

