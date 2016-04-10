#pragma once
#include "Toolbar.h"
#include "SharedPreferencesSelection.h"

class CPreferencesToolbar : public CToolbar, CSharedPreferencesSelection
{
public:
	CPreferencesToolbar(sGlobalInstances poGlobalInstances);
	~CPreferencesToolbar(void);

	bool Create(HINSTANCE phInstance, HWND phParent, int pID);
	void Destroy();
	void Initialize();
	void Localize(HINSTANCE phLanguage);
	void UpdateControls();

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);

protected: 


private:

	bool OnEventCoreNotification();

	void DoPositionControls();
	void DoEnableControls();

	sGlobalInstances oGlobalInstances;
	CCopyingMachineCore* oCore;
};
