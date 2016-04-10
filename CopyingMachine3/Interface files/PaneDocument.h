#pragma once
#include "PaneBase.h"
#include "PagePropertiesListview.h"

enum ePaneDocumentEdit {ePDResolutionX,
						ePDResolutionY,
						ePDPageSizeX,
						ePDPageSizeY};

struct sPaneDocumentEdit {

	HWND hWnd;
	std::wstring sValue;
	};

class CPaneDocument : public CPaneBase 
{
public:
	CPaneDocument(sGlobalInstances poGlobalInstances, int piMenuCommand);
	~CPaneDocument(void);

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
	bool OnCommand(int piCommand, WPARAM wParam, LPARAM lParam);

	void DoPositionControls();
	void DoEnableControls();
	void DoFillControls();
	//void DoDrawBackground();

	void DoProcessUpdate(int piControlId);
	void DoFillEditControl(ePaneDocumentEdit peEditControl);

	float GetFloatFromEditControl(HWND phEditControl);


	RectF	cSettingsControlRect;
	RectF	cButtonsControlRect;

	HWND hHeaderResolution;
	HWND hHeaderSize;
	HWND hHeaderDocument;
	HWND hHeaderPage;

	HWND hLabelPageMemorySize;
	HWND hLabelDocumentMemorySize;
	HWND hLabelDocumentPages;
	HWND hLabelPageSizePixels;

	HWND hTextPageSizePixels;
	HWND hTextDocumentPages;
	HWND hTextx;
	HWND hTextdpi;
	HWND hTextx2;
	HWND hTextPageMemorySize;
	HWND hTextDocumentMemorySize;
	HWND hEditResolutionX;
	HWND hEditResolutionY;
	HWND hEditPageSizeX;
	HWND hEditPageSizeY;
	HWND hEditMeasurement;

	CPagePropertiesListview* oPageProperties;

	sPaneDocumentEdit oPaneDocumentEdit[4];
};
