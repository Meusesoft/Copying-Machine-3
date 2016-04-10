#pragma once

struct sToolbarButton {
	
	int			iBitmap;
	int			iCommand;
	BYTE		fsState; 
    BYTE		fsStyle; 
    DWORD_PTR   dwData; 
    INT_PTR		iString; 
	};

class CToolbar
{
public:
	CToolbar(sGlobalInstances poGlobalInstances);
	~CToolbar(void);

	bool Create(HINSTANCE phInstance, HWND phParent, int pID);
	void Destroy();

	bool AddButtons(sToolbarButton pButtons[], long plNumberButtons);
	bool SetDefaultImages(HINSTANCE phInstance, int iImages[], long lNumberImages);
	bool SetHotImages(HINSTANCE phInstance, int iImages[], long lNumberImages);
	bool SetDisabledImages(HINSTANCE phInstance, int iImages[], long lNumberImages);
	bool SetDefaultImages(HINSTANCE phInstance, std::wstring sImages[], long lNumberImages);
	bool SetHotImages(HINSTANCE phInstance, std::wstring sImages[], long lNumberImages);
	bool SetDisabledImages(HINSTANCE phInstance, std::wstring sImages[], long lNumberImages);

	void EnableButton(int piCommand, bool pbEnable);

	void Localize(HMENU phMenu);

	HWND hWnd;	//my window handle
	HIMAGELIST hDefaultImagelist;
	HIMAGELIST hHotImagelist;
	HIMAGELIST hDisabledImagelist;

protected: 

	HINSTANCE hInstance;
	DWORD dwStyleExtra;
	long lNumberButtons;
	CTracer* oTrace;
	HWND	 hWndParent;


private:

	sGlobalInstances		oGlobalInstances;

	HIMAGELIST CreateImagelist();
	void DestroyImagelist(HIMAGELIST phImagelist);
	bool FillImagelist(HINSTANCE phInstance, HIMAGELIST &phImageList, int iImages[], long plNumberImages, bool bDisabled=false, bool bDefault=false);
	bool FillImagelist(HINSTANCE phInstance, HIMAGELIST &phImageList, std::wstring sImages[], long plNumberImages, bool bDisabled=false, bool bDefault=false);
};
