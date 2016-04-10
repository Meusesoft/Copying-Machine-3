#pragma once
#include "Toolbar.h"
#include "SharedDeviceSelection.h"

class CCopyToolbar : public CToolbar, CSharedDeviceSelection
{
public:
	CCopyToolbar(sGlobalInstances poGlobalInstances);
	~CCopyToolbar(void);

	bool Create(HINSTANCE phInstance, HWND phParent, int pID);
	void Destroy();
	void Initialize();
	void Localize(HINSTANCE phLanguage);
	
	void UpdateCurrentScanDevice();
	void UpdateCurrentPrintDevice();

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);

protected: 


private:

	bool OnEventCoreNotification();

	void DoPositionComboBoxes();

	sGlobalInstances oGlobalInstances;
	CCopyingMachineCore* oCore;
};
