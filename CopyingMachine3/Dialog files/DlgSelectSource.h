#pragma once
#include "ScanStructures.h"
#include "ScanDevice.h"

class CDlgSelectSource :
	public CDialogTemplate
{
public:
	CDlgSelectSource(HINSTANCE hInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances);
	~CDlgSelectSource(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();

private:

	vector <CScanDevice*> oScanningDevices;

	HIMAGELIST hImagelist;
	
};
