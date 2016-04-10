#pragma once
#include "PaneCopy.h"
#include "PaneActions.h"
#include "PaneImage.h"
#include "PaneDocument.h"
#include "WindowContainer.h"
#include "PaneScroller.h"

struct sPaneSwitchButton {

	Rect cRect;
	int  iIndex;
	};

class CPane : public CWindowContainer
{
public:
	CPane(sGlobalInstances poGlobalInstances);
	~CPane(void);

	bool Create(HINSTANCE phInstance, HWND phParent, int pID);
	void Destroy();
	void Localize(HINSTANCE phLanguage);

	void SetVisiblePane(CPaneBase* poCurrentPane);
	CPaneBase* GetVisiblePane();

	void Resize(int piX, int piY, int piWidth, int piHeight);
	void ResizeVisiblePane();

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);

	CPaneCopy*				oPaneCopy;
	CPaneImage*				oPaneImage;
	CPaneDocument*			oPaneDocument;
	CPaneActions*			oPaneActions;

protected: 

	void OnButtonDown(int piX, int piY);
	bool OnSize();

private:

	void DoSelectPaneMenu();
	void DoSetTitle();
	void DoPaint();

	void OnResizePane();


	CCopyingMachineCore*	oCore;

	std::vector <CPaneBase*> oPanes;
	CPaneScroller* 			 oPaneScroller;
	CPaneBase*				 oCurrentPane;
	bool					 bFirstSizeEvent;

	//std::vector <sPaneSwitchButton> oPaneSwitchButtons;

};
