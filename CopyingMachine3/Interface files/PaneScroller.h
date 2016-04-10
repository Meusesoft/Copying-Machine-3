#pragma once
#include "WindowScrollbar.h"
#include "PaneBase.h"
#include "PaneActions.h"

class CPaneScroller: public CWindowScrollbar
{
public:

	CPaneScroller(sGlobalInstances poGlobalInstances);
	~CPaneScroller();

	bool Create(HINSTANCE phInstance, HWND phParent);
	virtual void LayoutChildWindow();

	void SetVisiblePane(CPaneBase* poCurrentPane);
	CPaneBase* GetVisiblePane();

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);

protected:

	virtual bool OnSize();

private:

	void DoCheckScrollbars();
	void OnEventScrollbar(UINT message, WPARAM wParam, LPARAM lParam);
	bool OnEventCoreNotification();

	CPaneBase*	oCurrentPane;
};
