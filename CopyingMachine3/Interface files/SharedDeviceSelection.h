#pragma once
#include "Toolbar.h"

class CSharedDeviceSelection
{
public:
	CSharedDeviceSelection(sGlobalInstances poGlobalInstances);
	~CSharedDeviceSelection(void);

	void UpdateCurrentScanDevice();
	void UpdateCurrentPrintDevice();

protected: 

	bool OnCommand(int piCommand, WPARAM wParam, LPARAM lParam);

	vector <CScanDevice*> oScanningDevices;

	void DoFillScanSelector();
	void DoFillPrintSelector();
	void DoApplySettings();
	void DoApplySettings(int piCommand);
	void SelectOutputDevice(sOutputDevice poOutputDevice);

	sGlobalInstances oGlobalInstances;
	CCopyingMachineCore* oCore;
	CTracer* oTrace;

	HWND hScanComboBox;
	HWND hPrintComboBox;

private:

	};
