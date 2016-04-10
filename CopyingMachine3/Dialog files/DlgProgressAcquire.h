#pragma once
#include "ScanStructures.h"


class CDlgProgressAcquire :
	public CDialogTemplate
{
public:
	CDlgProgressAcquire(HWND phParent, void* poScanSettings, sGlobalInstances poGlobalInstances);
	~CDlgProgressAcquire(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();

private:

	void LocalizeStatusBeforeAcquire();
	void LocalizeStatusAfterSuccessfulAcquire();
	void LocalizeStatusAfterUnsuccessfulAcquire();
	void LocalizeMessageCountdown();
	void ShowEnableControl(int piCommand, bool pbEnabled);
	void DoFillControls();
	void DoFillAutoSaveComboBox();
	void DoApplySettings();
	void DoEnableControls();

	void SetProgress(int piValue);


	void* oScanSettings;
	long lCountdownTimer;
};
