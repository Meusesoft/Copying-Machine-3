#pragma once
#include "SharedPreferencesSelection.h"
#include "SharedDeviceSelection.h"
#include "PaneBase.h"

class CPaneActions : public CPaneBase
{
public:
	CPaneActions(sGlobalInstances poGlobalInstances, int piMenuCommand);
	~CPaneActions(void);

	bool Create(HINSTANCE phInstance, HWND phParent);
	void Destroy();
	void Initialize();
	void Localize(HINSTANCE phLanguage);
	void UpdateControls();
	void SetBackgroundImagePointer(CBitmap* poBitmap);

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);
	
protected: 
	
	virtual void DoRepositionControls();

private:

	bool OnEventCoreNotification();

	void DoPositionControls();
	void DoEnableControls();
	//void DoDrawBackground();

	RectF					cSettingsControlRect;
	RectF					cButtonsControlRect;


	HWND hButtonPrint;
	HWND hButtonCopy;
	HWND hButtonAcquire;	
};

