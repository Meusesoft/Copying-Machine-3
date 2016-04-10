#pragma once
#include "WindowContainer.h"
#include "CopyFileListview.h"

class CCopyFileListviewContainer : public CWindowContainer
{
public:
	CCopyFileListviewContainer(sGlobalInstances poGlobalInstances, 
				    int piTitle,
					std::wstring sWindowName);
	~CCopyFileListviewContainer(void);

	virtual HWND Create(HWND phParent, HINSTANCE phInstance, int piButtons=0);
	virtual void Localize(HINSTANCE phLanguage);

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);

	CCopyFileListview*	oFileListview;

protected: 

	virtual bool OnCommand(int piCommand);
	bool OnEventCoreNotification();


private:


};
