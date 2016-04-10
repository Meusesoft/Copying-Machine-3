#pragma once
#include "toolbar.h"

class CMenuToolbar :
	public CToolbar
{
public:
	CMenuToolbar(CTracer* poTrace);
	~CMenuToolbar(void);

	void Attach(HINSTANCE phResource, LPCTSTR lpMenuName);

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);

protected:

	//Hook for processing menu message
	static LRESULT CALLBACK MenuMessageFilter(int nCode, WPARAM wParam, LPARAM lParam);

	// Process menu message
	BOOL OnMenuMessage(MSG* pMsg);

private:

	HMENU hMenu;
	HMENU hSubMenu;
	void OpenSubMenu(long plIndex);
	long OpenSubMenu(UINT nChar);
	void PressButton(long plIndex, bool pbPressed);
	void CloseMenuWithKey();

	POINT cLastMousePosition;
	long  lMenuCount;
	long  lMenuOpen;
	long  lNextMenuOpen;
	long  lButtonPressed;
	bool  bMenuItemIsPopup;
	bool  bInSubmenu;
	bool  bKeyboardAction;
	HWND  hPreviousFocus;


};
