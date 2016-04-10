#pragma once

class CDlgOpenSave
{
public:
	CDlgOpenSave(void);
	~CDlgOpenSave(void);

	bool ExecuteOpenDialog(HMODULE phLanguage, HWND hParent, wchar_t* pcFileFilters, int &piDefaultFilter, wchar_t* pcFilename);
	bool ExecuteSaveDialog(HMODULE phLanguage, HWND hParent, wchar_t* pcFileFilters, int &piDefaultFilter, wchar_t* pcFilename);

	static UINT CALLBACK HookProc(HWND hDlg, UINT msg, WPARAM, LPARAM param2);

};
