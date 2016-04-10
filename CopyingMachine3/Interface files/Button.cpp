#include "StdAfx.h"
#include "Button.h"
#include "Bitmap.h"

CButton::CButton(CTracer* poTrace)
{
	hWnd = NULL;
	oTrace = poTrace;

	hDefaultImagelist = NULL;
	hDisabledImagelist = NULL;
	cImageAlignment = eImageLeft;
	bInherited = true;
}

CButton::~CButton(void)
{
	Destroy();
}

void 
CButton::SetHandle(HWND phWnd, HINSTANCE phInstance) {

	oTrace->StartTrace(__WFUNCTION__);

	hWnd = phWnd;
	hInstance = phInstance;

	bInherited = true;
	
	oTrace->EndTrace(__WFUNCTION__);
	}

//Create the window for the button
HWND 
CButton::Create(HWND phParent, HINSTANCE phInstance, int piCommand, 
				int piX, int piY, int piWidth, int piHeight,
				DWORD pdStyleExtra) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	hInstance = phInstance;

	DWORD dwStyle = WS_CHILD |
					WS_VISIBLE |
					WS_TABSTOP | 
					pdStyleExtra;
            
	hWnd = CreateWindowEx(0,						// ex style
						 L"BUTTON",					// class name
						 L"",						// window text
						 dwStyle,                   // style
						 piX,                       // x position
						 piY,                       // y position
						 piWidth,                   // width
						 piHeight,                  // height
						 phParent,					// parent
						 (HMENU)piCommand,			// ID
						 hInstance,                 // instance
						 NULL);                     // no extra data

	//We created the handle ourself, so we need to clean it up
	bInherited = false;

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL), eAll);

	return hWnd;
	}

//Destroy the window handle of the button
void 
CButton::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	if (hDefaultImagelist)  ImageList_Destroy(hDefaultImagelist);
	if (hDisabledImagelist) ImageList_Destroy(hDisabledImagelist);

	if (hWnd!=NULL && !bInherited) DestroyWindow(hWnd);

	oTrace->EndTrace(__WFUNCTION__);
	}

//this function is the message handler for the button
bool 
CButton::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	bool bReturn;

	if (hWnd == NULL) return false;

	oTrace->StartTrace(__WFUNCTION__, eExtreme);

	bReturn = false;
	
	switch (message) {

		case WM_NOTIFY:

			LPNMCUSTOMDRAW cNMButton;

			cNMButton = (LPNMCUSTOMDRAW)lParam;

			if (cNMButton->hdr.hwndFrom == hWnd) {

				if (cNMButton->dwDrawStage == CDDS_PREPAINT) {

					DoDrawButtonFace(cNMButton);

					piReturn = CDRF_SKIPDEFAULT;
					bReturn = true;
					}
				}

			break;
		}

	oTrace->EndTrace(__WFUNCTION__, eExtreme);

	return bReturn;
	}

//Draw the face of the button (text and image)
void
CButton::DoDrawButtonFace(LPNMCUSTOMDRAW pcNMButton) {

	wchar_t		cButtonText[50];
	RECT		cImageRect;
	RECT		cTextRect;
	RECT		cCalcTextRect;
	DWORD		dTextAlignment;
	IMAGEINFO	cImageInfo;
	HIMAGELIST	hImagelist;
	COLORREF	cColor;
	COLORREF	cDisabledColor;

	oTrace->StartTrace(__WFUNCTION__, eExtreme);

	//Get the text of the button
	GetWindowText(hWnd, cButtonText, 49);

	//Get the rectangles
	cImageRect = pcNMButton->rc;
	cTextRect  = pcNMButton->rc;

	dTextAlignment = DT_CENTER | DT_VCENTER;

	//Draw the image
	if (hDefaultImagelist!=NULL) {
	
		if (ImageList_GetImageCount(hDefaultImagelist)>0) {

			//Calculate the size of the text
			cCalcTextRect.left		= 0;
			cCalcTextRect.top		= 0;
			cCalcTextRect.right		= cTextRect.right - cTextRect.left;
			cCalcTextRect.bottom	= cTextRect.bottom - cTextRect.top;

			DrawTextW(pcNMButton->hdc, cButtonText, -1, &cCalcTextRect, DT_CALCRECT | DT_SINGLELINE | DT_TOP | DT_LEFT); 

			//Adjust rectangles
			switch (cImageAlignment) {

				case eImageLeft:

					//a quarter of the space is for the image.
					cImageRect.right -= (LONG)((cImageRect.right - cImageRect.left) * 0.75);
					cTextRect.left    = cImageRect.right;
					dTextAlignment	  = DT_LEFT | DT_VCENTER;
					break;

				case eImageRight:
					//a quarter of the space is for the image.
					cImageRect.left += (LONG)((cImageRect.right - cImageRect.left) * 0.75);
					cTextRect.right  = cImageRect.left;
					dTextAlignment   = DT_RIGHT | DT_VCENTER;
					break;

				case eImageTop:

					cImageRect.bottom -= cCalcTextRect.bottom * 2;
					cTextRect.top = cImageRect.bottom;
					dTextAlignment   = DT_CENTER | DT_TOP;
					break;

				case eImageBottom:

					cImageRect.top += cCalcTextRect.bottom * 2;
					cTextRect.bottom = cImageRect.top;
					dTextAlignment   = DT_CENTER | DT_BOTTOM;
					break;

				case eImageCenterNoText:

					//no change to the rectangles. Standard is center in button.
					break;
				}
			
			//Do the actual drawing
			hImagelist = hDefaultImagelist;

			if (hDefaultImagelist!=NULL && !IsWindowEnabled(hWnd)) hImagelist = hDisabledImagelist;

			ImageList_GetImageInfo(hImagelist, 0, &cImageInfo);

			ImageList_Draw(hImagelist, 0, pcNMButton->hdc, 
						cImageRect.left + ((cImageRect.right - cImageRect.left - cImageInfo.rcImage.right) / 2), 
						cImageRect.top + ((cImageRect.bottom - cImageRect.top - cImageInfo.rcImage.bottom) / 2), 
						ILD_TRANSPARENT);
			}
		}

	//Draw the text
	if (cImageAlignment!=eImageCenterNoText) {

		cColor = GetTextColor(pcNMButton->hdc);

		if (!IsWindowEnabled(hWnd)) {

			cDisabledColor = RGB((255 - GetRValue(cColor))/3,
							(255 - GetGValue(cColor))/3,
							(255 - GetBValue(cColor))/3);
			SetTextColor(pcNMButton->hdc, cDisabledColor);
			}

		SetBkMode(pcNMButton->hdc, TRANSPARENT);
		DrawTextW(pcNMButton->hdc, cButtonText, -1, &cTextRect, DT_SINGLELINE | dTextAlignment); 
		
		SetTextColor(pcNMButton->hdc, cColor);
		}

	oTrace->StartTrace(__WFUNCTION__, eExtreme);
	}


//Destroy the window handle of the button
void 
CButton::SetText(LPCTSTR lpString) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	if (hWnd!=NULL) SetWindowText(hWnd, lpString);

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//Set the image of the button. It adds the image to the default imagelist
void 
CButton::SetImage(int piImage, eButtonImageAlignment pcImageAlignment, int piImageSize) {

	HBITMAP hBitmap;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	cImageAlignment = pcImageAlignment;

	if (hDefaultImagelist==NULL) {
		hDefaultImagelist = CreateImagelist(piImageSize);
		}

	//Clear all previous images
	ImageList_RemoveAll(hDefaultImagelist);

	hBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(piImage));			
	ImageList_AddMasked(hDefaultImagelist, (HBITMAP)hBitmap, (COLORREF)RGB(255, 0 ,255));

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//Set the image of the button. It adds the image to the default imagelist
void 
CButton::SetImage(std::wstring psImage, eButtonImageAlignment pcImageAlignment, int piImageSize) {

	Bitmap* oBitmap;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	cImageAlignment = pcImageAlignment;

	if (hDefaultImagelist==NULL)	hDefaultImagelist = CreateImagelist(piImageSize);
	if (hDisabledImagelist==NULL)	hDisabledImagelist = CreateImagelist(piImageSize);

	//Clear all previous images
	ImageList_RemoveAll(hDefaultImagelist);
	ImageList_RemoveAll(hDisabledImagelist);

	oBitmap = CBitmap::LoadFromResource(psImage.c_str(), L"PNG", hInstance);

	if (oBitmap) {

		CBitmap::AddToImagelist(oBitmap, hDefaultImagelist);
		CBitmap::AddToImagelist(oBitmap, hDisabledImagelist, true);
		}

	delete oBitmap;

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//this private function will create an imagelist
HIMAGELIST 
CButton::CreateImagelist(int piImageSize) {

	HIMAGELIST hReturn;
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	hReturn = ImageList_Create(
			piImageSize, piImageSize,	//size of images
			ILC_MASK | ILC_COLOR32,		//flags
			1,							//initial size
			1);							//numbers of images to grow when needed
	
	oTrace->EndTrace(__WFUNCTION__, (int)hReturn, eAll);

	return hReturn;
	}


