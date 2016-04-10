#pragma once
#include "thread.h"
#include "listview.h"
#include "filelist.h"


class CFileListview :
	public CListview, CThread
{
public:
	CFileListview(sGlobalInstances pInstances, std::wstring psWindowName);
	~CFileListview(void);

	HWND Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra);

	virtual bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);
	virtual void __fastcall Execute();

	virtual void SaveSettings();
	virtual void LoadSettings();

	void SetFolder(std::wstring psFolder, bool pbClearHistory=true);
	std::wstring GetFolder();
	void SetAllowedFiles(cAllowedFiles poAllowedFiles);

	bool FolderHistory();
	void DoFolderBack();
	void DoOpenFile(int piItem);

	void Localize(HINSTANCE);


protected:

	virtual void OnColumnClick(int piColumn);
	virtual void OnButtonDoubleClick(int piItem);

	CFileList* oFileList;

private:

	void DoUpdateFiles();
	void DoSetSortMark();

	void OnGetDisplayInfo(NMLVDISPINFO* pcDisplayInfo);

	vector <std::wstring> sFolderHistory;
	};
