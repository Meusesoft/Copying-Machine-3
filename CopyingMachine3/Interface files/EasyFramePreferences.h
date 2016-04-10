#pragma once

#include "SharedPreferencesSelection.h"

class CEasyFramePreferences : public CSharedPreferencesSelection
{
public:
	CEasyFramePreferences(sGlobalInstances pInstances);
	~CEasyFramePreferences(void);

	HWND Create(HINSTANCE phInstance, HWND phWnd);
	void Destroy();

	static BOOL CALLBACK DialogTemplateProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam); 
	bool	OnMessage(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam); 
	void	SetEnableControl(bool pbEnable);

	void OnInitDialog();

	HWND hWnd;
	int nCmdShow;

private:

	bool OnCommand(int piCommand, WPARAM wParam, LPARAM lParam);
	bool OnEventCoreNotification();

	void DoFillControls();
	void DoEnableControls();
	void DoLocalize();

	void LocalizeControl(int piControl, int piResource);
	void LocalizeWindowCaption(int piResource);

	sGlobalInstances oGlobalInstances;
	HINSTANCE hInstance;
	HWND hListviewControl;

	CTracer* oTrace;
	CRegistry* oRegistry;
	CCopyingMachineCore* oCore;
};
