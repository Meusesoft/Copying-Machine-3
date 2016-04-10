#pragma once

enum eButtonImageAlignment {eImageLeft, eImageRight, eImageTop, eImageBottom, eImageCenterNoText};

class CButton
{
public:
	CButton(CTracer* poTrace);
	~CButton(void);

	HWND Create(HWND phParent, HINSTANCE phInstance, int piCommand, 
				int piX, int piY, int piWidth, int piHeight,
				DWORD pdStyleExtra);

	void SetHandle(HWND phWnd, HINSTANCE phInstance);
	void SetText(LPCTSTR lpString);
	void SetImage(int piImage, eButtonImageAlignment pcImageAlignment = eImageLeft, int piImageSize = 16);
	void SetImage(std::wstring psImage, eButtonImageAlignment pcImageAlignment = eImageLeft, int piImageSize = 16);

	bool WndProc(HWND hWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn);
	HWND hWnd;

protected:

	void Destroy();

private:

	void			DoDrawButtonFace(LPNMCUSTOMDRAW pcNMButton);
	bool		    bInherited;
	HIMAGELIST		CreateImagelist(int piImageSize = 16);
	
	eButtonImageAlignment cImageAlignment;
	HIMAGELIST		hDefaultImagelist;
	HIMAGELIST		hDisabledImagelist;
	HINSTANCE		hInstance;
	CTracer*		oTrace;
};
