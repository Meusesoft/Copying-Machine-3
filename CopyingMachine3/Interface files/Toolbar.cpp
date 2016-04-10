#include "StdAfx.h"
#include "Toolbar.h"
#include "bitmap.h"

CToolbar::CToolbar(sGlobalInstances poGlobalInstances)
{
	hWnd = NULL;
	hWndParent = NULL;
	hDefaultImagelist = NULL;
	hHotImagelist = NULL;
	hDisabledImagelist = NULL;
	dwStyleExtra = 0;

	//Copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &poGlobalInstances, sizeof(sGlobalInstances));

	oTrace = oGlobalInstances.oTrace;
}

CToolbar::~CToolbar(void)
{
	Destroy();
}

bool CToolbar::Create(HINSTANCE phInstance, HWND phParent, int pID) {

	if (hWnd!=NULL) return false;

	oTrace->StartTrace(__WFUNCTION__);

	hInstance = phInstance;
	hWndParent = phParent;

	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_COMPOSITED | TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DOUBLEBUFFER,
						  TOOLBARCLASSNAME, NULL,
						  dwStyleExtra | WS_CHILD | WS_VISIBLE |
						  TBSTYLE_FLAT | TBSTYLE_TRANSPARENT |
						  CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_TOP,
						  0, 0, 0, 0,
						  phParent,
						  (HMENU)pID,
						  NULL,
						  NULL);

	if (hWnd!=NULL) SendMessage(hWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	//hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_COMPOSITED | TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DOUBLEBUFFER,
	//					  TOOLBARCLASSNAME, NULL,
	//					  dwStyleExtra | WS_CHILD | WS_VISIBLE |
	//					  TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT |
	//					  CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_TOP ,
	//					  0, 0, 0, 0,
	//					  phParent,
	//					  (HMENU)pID,
	//					  NULL,
	//					  NULL);

	//if (hWnd!=NULL) {
	//	
	//	SendMessage(hWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	//	SendMessage(hWnd, TB_SETMAXTEXTROWS,0,0);
	//	}

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return (hWnd!=NULL);
	}

//This function will destroy all handles to controls and imagelists
void CToolbar::Destroy() {

	oTrace->StartTrace(__WFUNCTION__);

	if (hWnd!=NULL) DestroyWindow(hWnd);
	
	DestroyImagelist(hDefaultImagelist);
	DestroyImagelist(hDisabledImagelist);
	DestroyImagelist(hHotImagelist);

	oTrace->EndTrace(__WFUNCTION__);
}

//This function will add buttons
bool 
CToolbar::AddButtons(sToolbarButton pButtons[], long plNumberButtons) {

	TBBUTTON cNewButton;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	SecureZeroMemory(&cNewButton, sizeof(cNewButton));

	//This is necessary because now the text and icon will be centered in
	//the toolbar. Without adding a test string the text is aligned at the
	//bottom of the toolbar
	int iTextIndex = SendMessage(hWnd, TB_ADDSTRING, (WPARAM)0, (LPARAM)L"Hallo\0\0\0\0");

	//Add the buttons to the toolbar
	for (long lIndex=0; lIndex<plNumberButtons; lIndex++) {

		cNewButton.iBitmap   = pButtons[lIndex].iBitmap;
		cNewButton.fsState   = pButtons[lIndex].fsState;
		cNewButton.fsStyle   = pButtons[lIndex].fsStyle;
		cNewButton.idCommand = pButtons[lIndex].iCommand;
		cNewButton.iString   = pButtons[lIndex].iString;

		SendMessage(hWnd, TB_ADDBUTTONS, 1, (LPARAM)&cNewButton);
		}

	SendMessage(hWnd, TB_AUTOSIZE, 0, 0);
	
	//This is a fix for not completely drawing disabled texts on the
	//toolbar. For now we just set the buttons to a very large size. It
	//doesn't seem te have much effect. The height however determines
	//the height of the toolbar
	SendMessage(hWnd, TB_SETBUTTONSIZE, 0, (LPARAM)MAKELONG(250,45));
	
	lNumberButtons = SendMessage(hWnd, TB_BUTTONCOUNT, 0, 0);

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return true;
}	

	
//This function will set the default images. 
bool CToolbar::SetDefaultImages(HINSTANCE phInstance, int iImages[], long lNumberImages) {

	oTrace->StartTrace(__WFUNCTION__);

	if (hDefaultImagelist!=NULL) {
		DestroyImagelist(hDefaultImagelist);
		}

	if (!FillImagelist(phInstance, hDefaultImagelist, iImages, lNumberImages, false, true)) {
		
		oTrace->EndTrace(__WFUNCTION__, false);
		return false;
		}

	if (hWnd!=NULL) SendMessage(hWnd, TB_SETIMAGELIST, 0, (LPARAM)hDefaultImagelist);

	oTrace->EndTrace(__WFUNCTION__, true);

	return true;
}

//This function will set the hot images. 
bool CToolbar::SetHotImages(HINSTANCE phInstance, int iImages[], long lNumberImages) {

	oTrace->StartTrace(__WFUNCTION__);

	if (hHotImagelist!=NULL) {
		DestroyImagelist(hHotImagelist);
		}

	if (!FillImagelist(phInstance, hHotImagelist, iImages, lNumberImages, false, false)) {
		
		oTrace->EndTrace(__WFUNCTION__, false);
		return false;
		}

	if (hWnd!=NULL) SendMessage(hWnd, TB_SETHOTIMAGELIST, 0, (LPARAM)hHotImagelist);

	oTrace->EndTrace(__WFUNCTION__, true);

	return true;
}

//This function will set the disabled images. 
bool CToolbar::SetDisabledImages(HINSTANCE phInstance, int iImages[], long lNumberImages) {

	oTrace->StartTrace(__WFUNCTION__);

	if (hDisabledImagelist!=NULL) {
		DestroyImagelist(hDisabledImagelist);
		}

	if (!FillImagelist(phInstance, hDisabledImagelist, iImages, lNumberImages, true, false)) {
		
		oTrace->EndTrace(__WFUNCTION__, false);
		return false;
		}

	if (hWnd!=NULL) SendMessage(hWnd, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)hDisabledImagelist);

	oTrace->EndTrace(__WFUNCTION__, true);

	return true;
}

//This function will set the default images. 
bool CToolbar::SetDefaultImages(HINSTANCE phInstance, std::wstring psImages[], long lNumberImages) {

	oTrace->StartTrace(__WFUNCTION__);

	if (hDefaultImagelist!=NULL) {
		DestroyImagelist(hDefaultImagelist);
		}

	if (!FillImagelist(phInstance, hDefaultImagelist, psImages, lNumberImages, false, true)) {
		
		oTrace->EndTrace(__WFUNCTION__, false);
		return false;
		}

	if (hWnd!=NULL) SendMessage(hWnd, TB_SETIMAGELIST, 0, (LPARAM)hDefaultImagelist);

	oTrace->EndTrace(__WFUNCTION__, true);

	return true;
}

//This function will set the hot images. 
bool CToolbar::SetHotImages(HINSTANCE phInstance, std::wstring psImages[], long lNumberImages) {

	oTrace->StartTrace(__WFUNCTION__);

	if (hHotImagelist!=NULL) {
		DestroyImagelist(hHotImagelist);
		}

	if (!FillImagelist(phInstance, hHotImagelist, psImages, lNumberImages, false, false)) {
		
		oTrace->EndTrace(__WFUNCTION__, false);
		return false;
		}

	if (hWnd!=NULL) SendMessage(hWnd, TB_SETHOTIMAGELIST, 0, (LPARAM)hHotImagelist);

	oTrace->EndTrace(__WFUNCTION__, true);

	return true;
}

//This function will set the disabled images. 
bool CToolbar::SetDisabledImages(HINSTANCE phInstance, std::wstring psImages[], long lNumberImages) {

	oTrace->StartTrace(__WFUNCTION__);

	if (hDisabledImagelist!=NULL) {
		DestroyImagelist(hDisabledImagelist);
		}

	if (!FillImagelist(phInstance, hDisabledImagelist, psImages, lNumberImages, true, false)) {
		
		oTrace->EndTrace(__WFUNCTION__, false);
		return false;
		}

	if (hWnd!=NULL) SendMessage(hWnd, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)hDisabledImagelist);

	oTrace->EndTrace(__WFUNCTION__, true);

	return true;
}

//this private function will create (if it does not exist) and fill the imagelist
bool CToolbar::FillImagelist(HINSTANCE phInstance, HIMAGELIST &phImageList, int iImages[], long plNumberImages, bool pbDisabled, bool pbDefault) {

	oTrace->StartTrace(__WFUNCTION__);

	Bitmap* oBitmap;
	Bitmap* oDestinationBitmap;
	long lX, lY;
	long lValue;
	long lAlpha;
	Color oColor;

	if (phImageList==NULL) {
		phImageList = CreateImagelist();
		}

	if (phImageList==NULL) {
		
		oTrace->EndTrace(__WFUNCTION__, false);

		return false;
		}

	for (long lIndex = 0; lIndex<plNumberImages; lIndex++) {

		oBitmap = new Bitmap((HINSTANCE)phInstance, MAKEINTRESOURCE(iImages[lIndex]));
		//oBitmap = new Bitmap(L"D:\\Projects\\CopyingMachine3\\CopyingMachine3\\Resource files\\arrow_undo.png");
		//oBitmap = new Bitmap((HINSTANCE)phInstance, MAKEINTRESOURCE(IDB_PNG1));
		//oBitmap = CBitmap::LoadFromResource(L"IDB_TOOLUNDO", L"PNG", phInstance);
		//oBitmap = CBitmap::LoadFromResource(L"IDB_TOOLUNDO", RT_BITMAP, phInstance);

		if (oBitmap) {

			lX = oBitmap->GetWidth();
			lY = oBitmap->GetHeight();
			oDestinationBitmap = new Bitmap(lX, lY, PixelFormat32bppARGB);		

			while (lX>0) {

				lX--;
				lY = oBitmap->GetHeight();

				while (lY>0) {

					lY--;

					oBitmap->GetPixel(lX, lY, &oColor);

					if (oColor.GetValue() != Color::MakeARGB(255,255,0,255)) {

						if (pbDefault || pbDisabled) {

							lValue = (oColor.GetR() + oColor.GetB() + oColor.GetG())/3;
							lAlpha = oColor.GetA();
							
							oColor.SetValue(Color::MakeARGB((BYTE)lAlpha,(BYTE)lValue,(BYTE)lValue,(BYTE)lValue));
							}

						if (pbDisabled) {

							oColor.SetValue(Color::MakeARGB((BYTE)(lAlpha/2),(BYTE)lValue,(BYTE)lValue,(BYTE)lValue));
							}
						}
					else {

						oColor = Color::MakeARGB(0,0,0,0);
						}
							
					oDestinationBitmap->SetPixel(lX, lY, oColor);
					}
				}

			//ImageList_Add(phImageList, (HBITMAP)hResultBitmap, 0);
			//ImageList_AddMasked(phImageList, (HBITMAP)hResultBitmap, (COLORREF)RGB(255, 0 ,255));

			CBitmap::AddToImagelist(oDestinationBitmap, phImageList);

			delete oDestinationBitmap;
			delete oBitmap;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, true);

	return true;
}

//this private function will create (if it does not exist) and fill the imagelist
bool CToolbar::FillImagelist(HINSTANCE phInstance, HIMAGELIST &phImageList, std::wstring sImages[], long plNumberImages, bool pbDisabled, bool pbDefault) {

	oTrace->StartTrace(__WFUNCTION__);

	Bitmap* oBitmap;

	if (phImageList==NULL) {
		phImageList = CreateImagelist();
		}

	if (phImageList==NULL) {
		
		oTrace->EndTrace(__WFUNCTION__, false);

		return false;
		}

	for (long lIndex = 0; lIndex<plNumberImages; lIndex++) {

		oBitmap = CBitmap::LoadFromResource(sImages[lIndex].c_str(), L"PNG", phInstance);

		CBitmap::AddToImagelist(oBitmap, phImageList, pbDisabled, pbDefault);

		delete oBitmap;
		}

	oTrace->EndTrace(__WFUNCTION__, true);

	return true;
}

//this private function will create an imagelist
HIMAGELIST CToolbar::CreateImagelist() {

	HIMAGELIST hReturn;
	
	oTrace->StartTrace(__WFUNCTION__);

	hReturn = ImageList_Create(
			16, 16,					//size of images
			/*ILC_MASK |*/ ILC_COLOR32,	//flags
			16,						//initial size
			2);						//numbers of images to grow when needed
	
	oTrace->EndTrace(__WFUNCTION__, (int)hReturn);

	return hReturn;
	}

//this private function will destroy an imagelist
void 
CToolbar::DestroyImagelist(HIMAGELIST phImagelist) {

	oTrace->StartTrace(__WFUNCTION__);

	if (phImagelist!=NULL) {
		
		ImageList_Destroy(phImagelist);		
		}

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function enables/disables the given button
void 
CToolbar::EnableButton(int piCommand, bool pbEnable) {

	SendMessage(hWnd, TB_ENABLEBUTTON, (WPARAM)piCommand, (LPARAM)MAKELONG(pbEnable ? TRUE : FALSE, 0));
	}

//This function localizes the tooltips of the toolbar. The command descriptions
//from the main menu will be used to fill the tooltip
void 
CToolbar::Localize(HMENU phMenu) {

//	return;

	long lIndex;
	TBBUTTONINFO cButtonInfo;
	TBBUTTON cButton;
	wchar_t cCommandText[MAX_PATH];
	std::wstring sText;

	cButtonInfo.cbSize = sizeof(cButtonInfo);
	cButtonInfo.dwMask = TBIF_BYINDEX | TBIF_TEXT;

	lIndex = SendMessage(hWnd, TB_BUTTONCOUNT, 0, 0);

	while (lIndex>0) {

		lIndex--;

		if (SendMessage(hWnd, TB_GETBUTTON, (WPARAM)lIndex, (LPARAM)&cButton)) {

			GetMenuString(phMenu, 
						  cButton.idCommand,
						  cCommandText,
						  MAX_PATH,
						  MF_BYCOMMAND);

			sText = cCommandText;

			if (sText.find(L"...")!=-1) {

				cCommandText[sText.find(L"...")]=0x00;
				}
			if (sText.find(L"\t")!=-1) {

				cCommandText[sText.find(L"\t")]=0x00;
				}

			cButtonInfo.pszText = cCommandText;
			
			SendMessage(hWnd, TB_SETBUTTONINFO, (WPARAM)lIndex, (LPARAM)&cButtonInfo);
			}	
		}
	SendMessage(hWnd, TB_AUTOSIZE, 0, 0);

}

