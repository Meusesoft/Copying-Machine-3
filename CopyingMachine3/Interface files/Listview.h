#pragma once
#include "WindowBase.h"

enum eListviewWindowsVersion {eLVWindowsUnknown, eLVWindowsOlderThanXP, eLVWindowsXP, eLVWindowsVista, eLVWindowsNewerThanVista};

class CListview : public CWindowBase
{
public:
	CListview(sGlobalInstances pInstances, std::wstring psWindowName);
	~CListview(void);

	HWND Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra, HMENU phID = (HMENU)1008);
	void Destroy();

	int GetSelectedItem();
	bool SetSelectedItem(int piIndex);

	int ItemCount();

	virtual bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int &piReturn);
	virtual void OnSelectionChanged(int piIndex);

	int SetView(DWORD piView);
	int InsertColumn(int iCol, const LPLVCOLUMN pcol);
	int InsertColumn(int iCol, int piResource, int iWidth, int piFmt = LVCFMT_LEFT);
	int InsertColumn(int iCol, std::wstring psName, int iWidth, int piFmt = LVCFMT_LEFT);
	int GetColumnWidth(int iCol);
	void SetColumnWidth(int iCol, int piWidth);
	void SetColumnTitle(int iCol, int piResource);
	void SetColumnTitle(int iCol, std::wstring psName);
	
protected:

	virtual bool DoContextMenu(int piItem, HMENU hContextMenu);
	virtual bool DoContextMenu(HMENU hContextMenu);
	virtual void OnColumnClick(int piColumn);
	virtual void OnButtonDoubleClick(int piItem);

	virtual void OnBeginDrag(LPNMHDR pNmHdr);
	virtual void OnMoveDrag(int piX, int piY);
	virtual void OnEndDrag(int piX, int piY);
	virtual void RearrangeAfterDrag(int piNewItemIndex);


	HINSTANCE hInstance;
	HIMAGELIST hNormalImagelist;
	HIMAGELIST hSmallImagelist;
	HIMAGELIST hStateImagelist;
	HIMAGELIST hDragImageList;

	bool bDestroyNormalImagelist;
	bool bDestroySmallImagelist;
	bool bDestroyStateImagelist;
	bool bDestroyDragImageList;


	eListviewWindowsVersion	cWindowsVersion;

private:

	void OnRButtonClick();

	bool bDragging;
	HCURSOR hStandardCursor;
	HCURSOR hDragDropCursor;
	POINT   cPreviousPosition;

};
