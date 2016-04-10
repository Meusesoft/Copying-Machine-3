#pragma once
#include "WindowBase.h"
#include "Splitter.h"

#define WC_BUTTONHIDE		1
#define WC_BUTTONCLOSE  	2

struct sWindowContainerButton {

	Rect cRect;
	int  iIndex;
	int  iCommand;
	HWND hCommandWindow;
	int  iTooltip;
	std::wstring sImage;
	bool bImageHot;
	bool bVisible;
	};

class CWindowContainer : public CWindowBase {

public:
	CWindowContainer(sGlobalInstances pInstances, 
					int piTitle,
					std::wstring sWindowName);
	~CWindowContainer(void);

	virtual HWND Create(HWND phParent, HINSTANCE phInstance, int piButtons=0);

	virtual bool WndProc(HWND hWnd, UINT message,
			WPARAM wParam, LPARAM lParam, 
			int &piReturn);

	virtual void Localize(HINSTANCE phLanguage);

	void SetContainedWindow(HWND phContainedWnd);
	void LayoutChildWindow();
	void MakeVisible();
	void Close();
	bool IsVisible();
	
	virtual void LoadSettings();
	virtual void SaveSettings();

	void AddButton(sWindowContainerButton pcButton);
	void UpdateButton(int piCommand, sWindowContainerButton pcButton);
	void GetButton(int piCommand, sWindowContainerButton &pcButton);

	HWND hContainedWnd;
	CSplitter* oConnectedSplitter;
	HWND hFixedWnd;

protected: 

	virtual void OnButtonDown(int piX, int piY);
	virtual bool OnNotify(WPARAM, LPARAM);
	virtual bool OnCommand(int piCommand);

	void DoPaint();
	void DoPaint(HDC phDC);
	void SetTitle(int piId);
	void SetTitle(std::wstring psTitle);

	std::vector <sWindowContainerButton> oWindowContainerButtons;
	HIMAGELIST hWCButtonsDefault;
	HIMAGELIST hWCButtonsHot;
	
private:

	void DoToggleContainedWindow();
	
	HINSTANCE hInstance;
	int iTitle;
	bool bContainedWindowVisible;
	bool bContainerVisible;

	int iHeightWindow; //the height of the window is memorize when the contained window is hidden
	
	std::wstring sTitle;

	//Tooltip
	void DoTooltipCreate();
	void DoTooltipAdd(UINT_PTR piId, Rect pcRect);
	void DoTooltipUpdateRect(UINT_PTR piId, Rect pcRect);
	
	std::wstring sTooltip;
	HWND hTooltip;
};
