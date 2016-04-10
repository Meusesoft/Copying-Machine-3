#pragma once
#include "listview.h"
#include "bitmap.h"
#include <vector>
#include "CopyingMachineCore.h"

struct THUMBNAILITEM {

	TCHAR pszText[MAX_PATH+1];
	TCHAR pszFile[MAX_PATH+1];

	CBitmap* oImage;
	int		 iImagelistImage;
	DWORD	 dPageId;
	bool	 bNewThumbnail;
	DWORD	 dTimeLastUpdateRequest;
};

DWORD WINAPI ThreadCreateThumbnail(LPVOID lpParameter);
int CALLBACK PageCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

class CPageThumbnails :
	public CListview
{
public:
	CPageThumbnails(sGlobalInstances pInstances, std::wstring psWindowName);
	~CPageThumbnails(void);


	HWND Create(HWND phParent, HINSTANCE phInstance);
	bool AddItem(DWORD pdPageId, THUMBNAILITEM &pcItem);
	bool UpdateItem(DWORD pdPageId);
	bool DeleteItem(DWORD pdPageId);
	bool SelectItem(DWORD pdPageId);

	int CreateThumbnail(LPTSTR pszFile, int piImagelistIndex=-1);
	int CreateThumbnail(HGLOBAL phImage, int piImagelistIndex=-1);
	int CreateThumbnail(CBitmap* poBitmap, int piImagelistIndex=-1);
	void SetListviewItemImage(DWORD pdPageId, int piImagelistIndex);

	bool Clear();
	void RenumberItems();
	void Rearrange();

	virtual void Localize(HINSTANCE phLanguage);
	virtual bool WndProc(HWND hWnd, UINT message,
			WPARAM wParam, LPARAM lParam, 
			int &piReturn);
	virtual void OnSelectionChanged(int piIndex);

	//Variables for communicating with the thread
	bool bEndThread;
	std::vector<THUMBNAILITEM> vToBeProcessedThumbnails;
	CRITICAL_SECTION ThumbnailCriticalSection; 
	CCopyingMachineCore* oCore;

protected: 

	virtual bool DoContextMenu(int piItem, HMENU hContextMenu);
	virtual void RearrangeAfterDrag(int piNewItemIndex);

private:

	bool OnEventCoreNotification();

	int CreateThumbnail(LPTSTR pszFile, HIMAGELIST phImagelist, int piImagelistIndex=-1);
	int CreateThumbnail(HGLOBAL phImage, HIMAGELIST phImagelist, int piImagelistIndex=-1);
	int CreateThumbnail(Bitmap* poBitmap, HIMAGELIST phImagelist, int piImagelistIndex=-1);
	int CreateThumbnail(CBitmap* poBitmap, HIMAGELIST phImagelist, int piImagelistIndex=-1);

	int GetImage(LVITEM &pcItem);

	std::vector<THUMBNAILITEM> vThumbnails;
	HANDLE hThumbnailThread;

	sGlobalInstances oGlobalInstances;
};
