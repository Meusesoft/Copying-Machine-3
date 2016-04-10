#pragma once
#include "filelistview.h"

class CCopyFileListview :
	public CFileListview
{
public:
	CCopyFileListview(sGlobalInstances pInstances, std::wstring psWindowName);
	~CCopyFileListview(void);

	HWND Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra);

	std::wstring GetSelectedFolder();
	void Refresh();

protected:

	virtual bool DoContextMenu(HMENU hContextMenu);
	virtual bool DoContextMenu(int piItem, HMENU hContextMenu);

private:

};
