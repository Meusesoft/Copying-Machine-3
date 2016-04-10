#pragma once
#include "WindowBase.h"

enum eSplitterType {eSplitterDirect, eSplitterPreview};
enum eSplitterHV {eSplitterH, eSplitterVRight, eSplitterVLeft};
enum eSplitterMode {eSplitterUserSize, eSplitterDesiredSize, eSplitterHidden};

class CSplitter : public CWindowBase
{
public:
	CSplitter(sGlobalInstances pInstances, std::wstring psWindowName);
	~CSplitter(void);

	HWND Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra, 
				eSplitterHV pcSplitterHV, HWND phSizingSibling);

	virtual bool WndProc(HWND hWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn);

	virtual void SaveSettings();
	virtual void LoadSettings();

	void SetSiblingSize(int piSize);
	int	 GetSiblingSize();
	void SetMinimalSiblingSize(int piSize);
	void SetDesiredSiblingSize(int piSize);
	
	void SetMode(eSplitterMode = eSplitterUserSize);
	void ToggleMode();
	eSplitterMode GetMode();

protected:

private:

	HINSTANCE hInstance;
	HWND hSibling;

	bool bInCapture;
//	bool bHidden;
	HWND hPreviousCapture;

	int iStartSize;
	int iSize;
	int iDefiniteSize;
	int iMinimalSize;
	int iDesiredSize;

	eSplitterType cSplitterType;
	eSplitterHV	  cSplitterHV;
	eSplitterMode cSplitterMode;

	void DrawSplitterPreview(RECT &pcRect);
};
