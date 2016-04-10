#pragma once
#include "ScanStructures.h"


class CDlgPreferencesScannerDevice :
	public CDialogTemplate
{
public:
	CDlgPreferencesScannerDevice(HINSTANCE hInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances);
	~CDlgPreferencesScannerDevice(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void OnApply();

	virtual void Localize(HINSTANCE phLanguage=NULL);
private:

	vector <CScanDevice*> oScanningDevices;

	HIMAGELIST hImagelist;
	bool bInitialized;
	
};
