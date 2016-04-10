#pragma once
#include "bitmap.h"
#include <vector>
#include "CopyingMachineCore.h"
#include "WindowBase.h"

class CDocumentList : public CWindowBase {

public:
	CDocumentList(sGlobalInstances pInstances,
					std::wstring psWindowName);
	~CDocumentList(void);

	HWND Create(HWND phParent, HINSTANCE phInstance);
	void Destroy();

	virtual bool WndProc(HWND hWnd, UINT message,
			WPARAM wParam, LPARAM lParam, 
			int &piReturn);

	bool AddItem(int piDocumentID);
	bool DeleteItem(int piDocumentID);
	void SyncDocumentSelection();
	void Localize(HINSTANCE phLanguage);

protected: 

	virtual void OnSelectionChanged(int piIndex);
	virtual void OnRButtonClick();
	virtual void DoContextMenu(HTREEITEM phItem);
	virtual void DoContextMenu();

private:

	bool CheckItemInList(int piDocumentID);

	CCopyingMachineCore* oCore;
	HINSTANCE hInstance;
	HIMAGELIST hNormalImagelist;
	HIMAGELIST hSmallImagelist;
	HIMAGELIST hStateImagelist;
};
