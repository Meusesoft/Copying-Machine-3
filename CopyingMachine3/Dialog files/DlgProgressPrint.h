#pragma once
#include "ScanStructures.h"


class CDlgProgressPrint :
	public CDialogTemplate
{
public:
	CDlgProgressPrint(HWND phParent, void* poDocument, void* poPrintSettings, sGlobalInstances poGlobalInstances);
	~CDlgProgressPrint(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();

private:

	void ShowEnableControl(int piCommand, bool pbEnabled);
	void SetProgress(int piValue);
	bool OnEventCoreNotification();

	void* oPrintSettings;
	void* oDocument;
	int iPrintTicket;
};
