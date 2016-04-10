#pragma once
#include "WindowBase.h"

class CMainFrame : public CWindowBase
{
public:
	CMainFrame(sGlobalInstances pInstances);
	~CMainFrame(void);

	HWND Create(HINSTANCE phInstance, int nCmdShow);
	void Destroy();

	bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);

	virtual void LoadSettings();
	virtual void SaveSettings();

	void ViewDocument(CCopyDocument* poDocument, DWORD pdPageId=0);
	void ViewPage(CCopyDocumentPage* poPage=NULL);

	int nCmdShow;

private:

	void CreateChildWindows();
	void LayoutChildWindows();
	bool OnEventCoreNotification();

	bool OnCommand(int piCommand);
	void DoCopyDocumentStep1(bool pbExit);
	void DoCopyDocumentStep2();
	void DoOutputDocument(CCopyDocument* poDocument, bool bShowPrinterSelectionDialog=false);
	void DoPrintDocument(CCopyDocument* poDocument, bool ShowInterface);
	void DoEnableControls();
	void DoEnableControlsPaste();
	void DoEnableControlsMagnification();
	void DoEnableControlsLanguage();
	void DoDeletePages();
	void DoUpdateTitle();
	void DoChangeColor(DWORD pdColor);
	void DoAcceptDropFiles(HDROP phDrop);
	void DoOpenHelp();
	void DoSwitchWindowVisibility(HWND phWnd1, HWND phWnd2);

	void AddPageToThumbnailsControl(CCopyDocumentPage* poPage);

	HINSTANCE hInstance;
	HINSTANCE hLanguage;
	HWND hListviewControl;

	CRebarControl*		oRebarControl;
	CToolbar*			oCommandBar;
	CToolbar*			oToolbar2;
	CMenuToolbar*		oMenuToolbar;
	CPageThumbnails*	oThumbnails;
	CSplitter*			oSplitterV;
	CSplitter*			oSplitterV2;
	CSplitter*			oSplitterH;
	CSplitter*			oSplitterH2;
	CSplitter*			oSplitterH3;
	CPageViewer*		oPageViewer;
	CDocumentList*		oDocumentList;
	CDocumentCollectionList* oDocumentCollectionList;
	CPane*				oPane;
	CMenuIcon*			oMenuIcon;
	CWindowContainer*	oDocumentCollectionListContainer;
	CWindowContainer*	oDocumentListContainer;
	CWindowContainer*   oThumbnailsContainer;
	CCopyFileListviewContainer*	oFileContainer;
	CWindowContainer*	oPageViewerContainer;

	sPageViewerSettings* oPageViewerSettings;

	CRegistry* oRegistry;
	CCopyingMachineCore* oCore;

	bool bCopyInProgress;
	bool bExitAfterCopy;

};
