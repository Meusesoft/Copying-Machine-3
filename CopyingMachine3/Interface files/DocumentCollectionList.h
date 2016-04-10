#pragma once
#include "bitmap.h"
#include <vector>
#include "CopyingMachineCore.h"
#include "WindowBase.h"

class CDocumentCollectionList : public CWindowBase {

public:
	CDocumentCollectionList(sGlobalInstances pInstances, 
							std::wstring psWindowName);
	~CDocumentCollectionList(void);

	HWND Create(HWND phParent, HINSTANCE phInstance);
	virtual void Destroy();

	virtual bool WndProc(HWND hWnd, UINT message,
			WPARAM wParam, LPARAM lParam, 
			int &piReturn);

	void SyncDocumentCollections();
	void Localize(HINSTANCE phLanguage);
	int  GetSelectedCollection();

	HWND hWnd;

protected: 

	virtual void OnSelectionChanged(int piIndex);
	virtual void OnRButtonClick();
	virtual void DoContextMenu(HTREEITEM phItem);

private:

	bool CheckItemInList(int piDocumentID);

	CCopyingMachineCore* oCore;
	HINSTANCE hInstance;
	HIMAGELIST hNormalImagelist;
	HIMAGELIST hSmallImagelist;
	HIMAGELIST hStateImagelist;

};
