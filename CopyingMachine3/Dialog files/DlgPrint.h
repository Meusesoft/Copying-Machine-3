#pragma once

class CDlgPrint
{
public:
	CDlgPrint(void);
	~CDlgPrint(void);

	bool ExecutePrintDialog(sGlobalInstances pcInstances, HWND hParent, void* poPrintSettings);

	static UINT CALLBACK HookProc(HWND hDlg, UINT msg, WPARAM, LPARAM param2);
	
};
