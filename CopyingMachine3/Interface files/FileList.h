#pragma once
#include "thread.h"
#include <list>

enum eFileSort {eSortName, eSortSize, eSortDate, eSortType};

struct cAllowedFiles {

	std::wstring sFileTypes;
	bool bFolders;
	bool bHidden;
	};	

//The file structure
class CFile {

public:

	CFile();
	~CFile();

	bool Compare(CFile* poCompareWith, eFileSort cFileSort);
	bool IsFolder();
	bool IsHidden();
	void FormatFileSize();
	void FormatFileDateTime();
	void GetFileInfo(std::wstring psFolder);
	std::wstring GetExtension();

	std::wstring sName;
	std::wstring sType;
	std::wstring sSize;
	std::wstring sDateTime;
	DWORDLONG dSize;
	DWORDLONG dModifiedDateTime;
	DWORD     dAttributes;
	int iImage;
	bool bRefresh;		//true if the file is found during a synchronization

private:

	bool bFileInfoPresent;

	int CompareName(CFile* poCompareWith);

	NUMBERFMT cNumberFormat;
	TCHAR     sThousand[5];
	};

//The filelist class

class CFileList : 
	public CThread
{
public:
	CFileList(sGlobalInstances pInstances);
	~CFileList(void);

	virtual void __fastcall Execute();

	void SetFolder(std::wstring psFolder);
	std::wstring GetFolder();
	void SetRefresh(bool pbValue = true);

	void SetAllowedFiles(cAllowedFiles poAllowedFiles);
	bool IsAllowed(CFile* poFile);

	void SetSort(eFileSort pcFileSort);
	eFileSort GetSort();
	bool GetSortReverse();
	void SetSortReverse(bool pbSortReverse);

	void Clear();
	void Synchronize();
	void SetSynchronizationInProgress(bool pbValue);
	bool GetSynchronizationInProgress();
	bool GetUpdated();
	int GetFileCount();
	CFile* GetFile(int piIndex);

protected:

	bool AddFileToList(CFile* poFile);



private:

	void ResetIterator();
	bool GetRefresh();
	CFile* FindFile(std::wstring psName);

	void DoSort();
	void DoClearRefreshFlag();
	void DoRemoveUnrefreshedFiles();

	sGlobalInstances oGlobalInstances;
	std::wstring sFolder;
	bool bSynchronizeInProgress;
	bool bUpdated;
	bool bRefresh;

	eFileSort cFileSort;
	cAllowedFiles oAllowedFiles;
	bool bSortReverse;

	std::list <CFile*> oFiles;
	std::list <CFile*>::const_iterator FileIterator;
	int iFileIteratorIndex;
};
