#pragma once
#include "WindowBase.h"

class CWindowScrollbar : public CWindowBase {

public:
	CWindowScrollbar(sGlobalInstances pInstances, std::wstring psWindowName);
	~CWindowScrollbar(void);

	virtual bool WndProc(HWND hWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn);

	int  GetVerticalPosition();
	int  SetVerticalPosition(int piVertPos);	

protected: 

	void InitializeScrollInfo();
	virtual void OnEventScrollbar(UINT message, WPARAM wParam, LPARAM lParam);
	virtual bool OnSize();
	void UpdateScrollbar(DWORD pdType, SCROLLINFO &pcNewScrollInfo);

	virtual void DoRepositionControls();
	void SetContentSize(int piWidth, int piHeight);
	int  GetHorizontalPosition();


	SCROLLINFO cSBIVertical;
	SCROLLINFO cSBIHorizontal;

private:

	void DoUpdateScrollbar(DWORD pdType);

	int iContentWidth;
	int iContentHeight;

};
