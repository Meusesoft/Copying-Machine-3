#pragma once
#include "PaneBase.h"
#include "Button.h"

class CPaneImage : public CPaneBase
{
public:
	CPaneImage(sGlobalInstances poGlobalInstances, int piMenuCommand);
	~CPaneImage(void);

	bool Create(HINSTANCE phInstance, HWND phParent);
	void Destroy();
	void Initialize();
	void Localize(HINSTANCE phLanguage);
	void UpdateControls();

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);
	
protected: 

	virtual void DoRepositionControls();

private:

	bool OnEventCoreNotification();

	void DoPositionControls();
	void DoEnableControls();

	RectF					cSettingsControlRect;
	RectF					cButtonsControlRect;

};
