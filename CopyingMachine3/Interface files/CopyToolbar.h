#pragma once
#include "Toolbar.h"

class CCopyToolbar : public CToolbar
{
public:
	CCopyToolbar(sGlobalInstances poGlobalInstances);
	~CCopyToolbar(void);

	bool Create(HINSTANCE phInstance, HWND phParent, int pID);
	void Destroy();
	void Initialize();
	
	void UpdateCurrentScanDevice();
	void UpdateCurrentPrintDevice();

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);

protected: 


private:

	vector <sScanDevice> oScanningDevices;

	bool OnEventCoreNotification();

	void DoFillScanSelector();
	void DoFillPrintSelector();
	void DoApplySettings();
	void SelectOutputDevice(sOutputDevice poOutputDevice);

	sGlobalInstances oGlobalInstances;
	CCopyingMachineCore* oCore;

	HWND hScanComboBox;
	HWND hPrintComboBox;
	

};
