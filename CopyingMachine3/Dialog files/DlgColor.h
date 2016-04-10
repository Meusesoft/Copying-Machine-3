#pragma once

class CDlgColor
{
public:
	CDlgColor(void);
	~CDlgColor(void);

	bool ExecuteColorDialog(sGlobalInstances pcInstances, HWND hParent);

	static UINT CALLBACK HookProc(HWND hDlg, UINT msg, WPARAM, LPARAM param2);
	
};
