#pragma once
#include "SharedPreferencesSelection.h"
#include "SharedDeviceSelection.h"
#include "PaneBase.h"

class CPaneCopy : public CSharedPreferencesSelection, 
				  public CSharedDeviceSelection, 
				  public CPaneBase
{
public:
	CPaneCopy(sGlobalInstances poGlobalInstances, int piMenuCommand);
	~CPaneCopy(void);

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

	HWND hHeaderAcquireFrom;
	HWND hHeaderCopyTo;
	HWND hHeaderResolution;
	HWND hHeaderColor;
	HWND hHeaderCopies;
	HWND hHeaderBrightness;
	HWND hHeaderContrast;

	HWND hButtonPrint;
	HWND hButtonCopy;
	HWND hButtonAcquire;
	HWND hScanControls;
	HWND hScanButtons;
};

