#pragma once
#include "Button.h"
#include "WindowScrollbar.h"
#include "CopyingMachineCore.h"

enum ePaneType {ePaneCopy, ePaneImage, ePaneDocument, ePaneActions, ePaneUndefined};

struct sPaneButton {

	int			iCommandId;
	int			iTextResourceId;
	int			iExtraSpace;
	bool		bEnabled;
	bool		bCollapsed;
	CButton*	oPaneButton;
	HWND		hStaticControl;
	float       fHeightMultiply;
	};

class CPaneBase: public CWindowBase
{
public:
	CPaneBase(sGlobalInstances poGlobalInstances, int piMenuCommand, std::wstring psWindowName);
	~CPaneBase(void);

	bool Create(HINSTANCE phInstance, HWND phParent);
	void Destroy();

	void Resize(int piX, int piY, int piWidth, int piHeight);

	void SetBackgroundImagePointer(CBitmap* poBitmap);
	void SetVisible(bool pbVisible);

	HWND AddPaneButton(int piCommand, int piCaption, int piImage, int piExtraSpace=0, float pfHeightMultiply=1.0f);
	HWND AddPaneButton(int piCommand, int piCaption, std::wstring psImage, int piExtraSpace=0, float pfHeightMultiply=1.0f);
	HWND AddPaneStatic(int piCaption, int piExtraSpace=0, float pfHeightMultiply=1.0f);
	void LocalizePaneButtons(HINSTANCE phLanguage);
	void PositionPaneButtons(RectF pcButtonsControlRect);

	HWND AddStaticControl(std::wstring psLabel);

	//HWND			hWnd;
	std::wstring	sTitle;
	std::wstring	sImage;
	ePaneType		cType;
	int				iMenuCommand;
	int				iPaneMinimumHeight;
	int				iTitle;

	void DoPaint();
	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);
	void DoDrawStaticControls(LPDRAWITEMSTRUCT oDrawItemInfo);
	void DoDrawCheckboxBackground(HWND phControl, HDC phDC);
	void DoDrawUnderlineHeaders(Graphics* poCanvas);
	void Initialize();

protected: 



	void OnResizePane();


	sGlobalInstances		oGlobalInstances;
	CCopyingMachineCore*	oCore;
	CTracer*				oTrace;

	CBitmap*				oDoubleBufferBitmap;
	SolidBrush*				oWhiteBrush;

	vector <sPaneButton>	oPaneButtons;
	vector <HWND>		    oToBeDeletedHWND;

	HFONT					hNormalFont;
	HFONT					hBoldFont;
	int						iFontHeight;

private:

};
