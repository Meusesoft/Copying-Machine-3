#include "EasyFramePreferences.h"
#include "EasyFramePreview.h"
#include "SharedDeviceSelection.h"
#include "Button.h"
#include "MenuIcon.h"

enum eToggleParameter {eToggle, eShow, eHide};

class CEasyFrame : public CSharedDeviceSelection
{
public:
	CEasyFrame(sGlobalInstances pInstances);
	~CEasyFrame(void);

	HWND Create(HINSTANCE phInstance, HWND phWnd);
	void Destroy();

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);

	void LoadSettings();
	void SaveSettings();

	HWND hWnd;
	int nCmdShow;

private:

	bool OnEventCoreNotification();
	bool MoveEasyFramePreferences();
	void ShowEasyFramePreferences(eToggleParameter pcToggle = eToggle);
	bool MoveEasyFramePreview();
	void ShowEasyFramePreview(eToggleParameter pcToggle = eToggle);
	void SetProgress(int piValue);
	void ShowViewMenu();

	void DoFillControls();
	void DoApplySettings();
	void DoLocalize();
	void DoAcquireImages();
	void DoOutputImages();
	void DoEnableControls();
	void DoPrintCurrentDocument();
	void DoValidatePositionEasyFrame(LPWINDOWPOS pcWindowPosition);

	bool GetCurrentDesktopSize(HWND phWindow, LPRECT pcDesktopRect);
	int  GetCurrentOutputDevice();

	void LocalizeControl(int piControl, int piResource);
	bool bCopyInProgress;

	sGlobalInstances oGlobalInstances;
	HINSTANCE hInstance;
	HINSTANCE hLanguage;
	HWND hListviewControl;
	HMENU	  hViewMenu;

	int iPrintTicket;

	bool bExitAfterCopy;

	CRegistry* oRegistry;
	CCopyingMachineCore* oCore;
	CEasyFramePreferences*	oEasyFramePreferences;
	CEasyFramePreview*		oEasyFramePreview;
	CButton*				oCopyButton;
	CMenuIcon*				oMenuIcon;
	};
