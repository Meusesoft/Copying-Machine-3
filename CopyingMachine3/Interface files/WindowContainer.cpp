#include "StdAfx.h"
#include "WindowContainer.h"
#include "Bitmap.h"

CWindowContainer::CWindowContainer(sGlobalInstances pInstances, 
								   int piTitle,
								   std::wstring psWindowName) :
	CWindowBase(pInstances, psWindowName)
{
	hContainedWnd			= NULL;
	hWCButtonsDefault       = NULL;
	hWCButtonsHot			= NULL;
	hTooltip				= NULL;
	hInstance				= NULL;
	oConnectedSplitter		= NULL;
	hFixedWnd				= NULL;
	iTitle					= piTitle;
	bContainedWindowVisible = true;
	bContainerVisible		= true;
	iHeightWindow			= 0;
}

CWindowContainer::~CWindowContainer(void)
{
	Destroy();

	if (hWCButtonsDefault)	ImageList_Destroy(hWCButtonsDefault);
	if (hWCButtonsHot)		ImageList_Destroy(hWCButtonsHot);
	if (hTooltip)			DestroyWindow(hTooltip);
}

HWND CWindowContainer::Create(HWND phParent, HINSTANCE phInstance, int piButtons) {

	oTrace->StartTrace(__WFUNCTION__);
	
	hInstance = phInstance;
	
	DWORD dwStyle = WS_TABSTOP | 
					WS_CHILD |
					WS_VISIBLE |
					WS_CLIPCHILDREN | 
					WS_BORDER |
					WS_CLIPSIBLINGS;

	hWnd = CreateWindowEx(0,						// ex style
						 L"MeusesoftContainer",     // class name - defined in commctrl.h
						 NULL,                      // window text
						 dwStyle,                   // style
						 0,                         // x position
						 30,                        // y position
						 20,                       // width
						 20,                       // height
						 phParent,					// parent
						 (HMENU)1100,				// ID
						 hInstance,                 // instance
						 NULL);                     // no extra data
	
	//Localize
	Localize(oGlobalInstances.hLanguage);
	
	//Create the tooltip window
	DoTooltipCreate();

	//Create the button imagelists
	hWCButtonsDefault = ImageList_Create(
			16, 16,					//size of images
			ILC_COLOR32,			//flags
			16,						//initial size
			2);						//numbers of images to grow when needed

	hWCButtonsHot = ImageList_Create(
			16, 16,					//size of images
			ILC_COLOR32,			//flags
			16,						//initial size
			2);						//numbers of images to grow when needed

	//Add a hide button
	sWindowContainerButton cButton;

	if (piButtons & WC_BUTTONCLOSE) {

		cButton.cRect = Rect(0,0,0,0);
		cButton.hCommandWindow = hWnd;
		cButton.iCommand = IDC_WINDOWCONTAINERCLOSE;
		cButton.iTooltip = IDS_CLOSE;
		cButton.sImage = L"PNG_TOOLWCCLOSE";
		cButton.bImageHot = false;
		cButton.bVisible = true;
		AddButton(cButton);
		}

	if (piButtons & WC_BUTTONHIDE) {

		cButton.cRect = Rect(0,0,0,0);
		cButton.hCommandWindow = hWnd;
		cButton.iCommand = IDC_WINDOWCONTAINERTOGGLE;
		cButton.iTooltip = IDS_HIDE;
		cButton.sImage = L"PNG_TOOLARROWUP";
		cButton.bImageHot = false;
		cButton.bVisible = true;
		AddButton(cButton);
		}

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
	}

//This function is called when a language change is being processed
void 
CWindowContainer::Localize(HINSTANCE phLanguage) {

	oGlobalInstances.hLanguage = phLanguage;

	if (iTitle!=-1) {
		
		wchar_t* cText;

		//Alloc text buffer
		cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

		//Update title
		LoadString(phLanguage, iTitle, cText, 400);
		sTitle = cText;

		//Free memory
		free(cText);

		//Redraw the title
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

//This function sets the title of this container
void 
CWindowContainer::SetTitle(int piId) {

	iTitle = piId;

	Localize(oGlobalInstances.hLanguage);

	InvalidateRect(hWnd, NULL, FALSE);
	}

//This function sets the title of this container
void 
CWindowContainer::SetTitle(std::wstring psTitle) {

	iTitle = -1;
	sTitle = psTitle;

	InvalidateRect(hWnd, NULL, FALSE);
}

//This function sets the contained window
void 
CWindowContainer::SetContainedWindow(HWND phContainedWnd) {

	hContainedWnd = phContainedWnd;

	LayoutChildWindow();
	}

//This functions saves the settings to the registry
void
CWindowContainer::SaveSettings() {

	std::wstring sKey;
	CRegistry* oRegistry;

	oRegistry = oGlobalInstances.oRegistry;

	sKey = sWindowName;
	sKey += L"Visible";
	oRegistry->WriteInt(L"Window", sKey, bContainedWindowVisible ? 1 : 0);
	sKey = sWindowName;
	sKey += L"MemorizedHeight";	
	oRegistry->WriteInt(L"Window", sKey, iHeightWindow);

	CWindowBase::SaveSettings();
	}

//This functions loads the settings to the registry
void
CWindowContainer::LoadSettings() {

	std::wstring sKey;
	CRegistry* oRegistry;

	oRegistry = oGlobalInstances.oRegistry;
	sKey = sWindowName;
	sKey += L"Visible";
	bContainedWindowVisible = (oRegistry->ReadInt(L"Window", sKey, 1)==1);
	sKey = sWindowName;
	sKey += L"MemorizedHeight";	
	iHeightWindow = oRegistry->ReadInt(L"Window", sKey, 200);

	if (oWindowContainerButtons.size()>0) {
		
		sWindowContainerButton cButton;

		GetButton(IDC_WINDOWCONTAINERTOGGLE, cButton);

		cButton.iTooltip = bContainedWindowVisible ? IDS_HIDE : IDS_SHOW;
		cButton.sImage = bContainedWindowVisible ? L"PNG_TOOLARROWUP" : L"PNG_TOOLARROWDOWN";

		UpdateButton(IDC_WINDOWCONTAINERTOGGLE, cButton);
		}
	
	CWindowBase::LoadSettings();
	}

//This is the message handler for this class
bool CWindowContainer::WndProc(HWND phWnd, UINT message, 
						WPARAM wParam, LPARAM lParam, 
						int &piReturn) {

	bool bReturn;

	bReturn = false;

	//Handle the message since the base class didn't 
	switch (message) {

		case WM_SIZE:
			if (phWnd==this->hWnd) {
				LayoutChildWindow();	
				bReturn = true;
				}
			break;

		case WM_ERASEBKGND:
			if (phWnd == this->hWnd) {
				bReturn = true;
				piReturn = 0;
				}
			break;

		case WM_PAINT:

			if (phWnd == this->hWnd) {

				DoPaint();
				bReturn = true;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:

			if (phWnd == this->hWnd) {
				
				OnButtonDown(LOWORD(lParam), HIWORD(lParam));

				bReturn = true;
				}
			break;

		case WM_NOTIFY:

			if (phWnd == this->hWnd) {

				bReturn = OnNotify(wParam, lParam);
				}
			break;

		case WM_COMMAND:

			if (phWnd == this->hWnd) {

				//Process the command of the user
				OnCommand(LOWORD(wParam));
				
				bReturn = true;
				}
			break;


		default:

			break;
	
		}

	//Process the base wndproc 
	if (!bReturn) {

		bReturn = CWindowBase::WndProc(phWnd, message, wParam, lParam, piReturn);
		}

	return bReturn;
	}

//this function layouts the child windows after an event which
//causes the window or one of its child to change its size.
void 
CWindowContainer::LayoutChildWindow() {

	RECT cClientRect;
	sWindowContainerButton cButton;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	GetClientRect(hWnd, &cClientRect);

	if (oWindowContainerButtons.size()>0 && (cClientRect.bottom - 22 > 0)) {
		bContainedWindowVisible = true;

		GetButton(IDC_WINDOWCONTAINERTOGGLE, cButton);

		cButton.iTooltip = bContainedWindowVisible ? IDS_HIDE : IDS_SHOW;

		UpdateButton(IDC_WINDOWCONTAINERTOGGLE, cButton);
		}

	int iHeightPaneActions = 0;
	if (hFixedWnd != NULL) 
	{
		iHeightPaneActions = 230;

		MoveWindow(hFixedWnd,
			0,
			cClientRect.bottom-iHeightPaneActions,
			cClientRect.right,
			iHeightPaneActions,
			TRUE);
	}

	MoveWindow(hContainedWnd,
				0, 20, 
				cClientRect.right, cClientRect.bottom-20-iHeightPaneActions, 
				TRUE);

	InvalidateRect(hWnd, NULL, TRUE);

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//Paints the container title
void 
CWindowContainer::DoPaint() {

	PAINTSTRUCT cPaintStruct;
	HDC hDC;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	//Start drawing
	SecureZeroMemory(&cPaintStruct, sizeof(cPaintStruct));
	hDC = BeginPaint(hWnd, &cPaintStruct);

	DoPaint(hDC);

	//End drawing
	EndPaint(hWnd, &cPaintStruct);

	oTrace->EndTrace(__WFUNCTION__, eAll);
}


void 
CWindowContainer::DoPaint(HDC hDC) {

	Graphics* oCanvas;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	//Create a canvas for drawing to the screen
	oCanvas = new Graphics(hDC);

	//Draw the title of this container
	RECT		cWindowRect;
	GetWindowRect(hWnd, &cWindowRect);

	cWindowRect.right = cWindowRect.right - cWindowRect.left;
	cWindowRect.bottom = 20;
	cWindowRect.left = 0;
	cWindowRect.top = 0;

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	HBRUSH hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

//	FillRect(hDC, &cWindowRect, hBrush);

	RectF cBackgroundRect;

		cBackgroundRect.X = 0;
		cBackgroundRect.Y = 0;
		cBackgroundRect.Width = (REAL)cWindowRect.right;
		cBackgroundRect.Height = 20;

	LinearGradientBrush* oBackgroundBrush = new LinearGradientBrush( 
							cBackgroundRect, 
							Color(255, (BYTE)(defBackgroundR*1.3), (BYTE)(defBackgroundR*1.3), (BYTE)(defBackgroundR*1.3)), 
							Color(255, (BYTE)(defBackgroundR), (BYTE)(defBackgroundR), (BYTE)(defBackgroundR)), 
							(REAL)90.0, 
							false); 
	//SolidBrush* oBackgroundBrush = new SolidBrush(Color((BYTE)(defBackgroundR*1.3), (BYTE)(defBackgroundR*1.3), (BYTE)(defBackgroundR*1.3))); 
	
	oCanvas->FillRectangle(oBackgroundBrush, cBackgroundRect);

	delete oBackgroundBrush;

	cWindowRect.top += 2;

	//Draw the buttons of the container
	HIMAGELIST hImagelist;
	Rect		cWCButtonRect;
	GetWindowRect(hWnd, &cWindowRect);
	long lIndex;

	sWindowContainerButton cWCButton;

	cWCButtonRect.X      = cWindowRect.right - cWindowRect.left - 20;
	cWCButtonRect.Y      = 2;
	cWCButtonRect.Width  = 16;
	cWCButtonRect.Height = 17;

	lIndex = 0;

	while (lIndex<(long)oWindowContainerButtons.size()) {
	
		oWindowContainerButtons[lIndex].cRect = Rect(0,0,0,0);

		if (oWindowContainerButtons[lIndex].bVisible) {

			//Add the switch button to the vector of switch buttons
			oWindowContainerButtons[lIndex].cRect = cWCButtonRect;
			DoTooltipUpdateRect(lIndex, cWCButtonRect);

			//oPaneSwitchButtons.push_back(cPaneSwitchButton);

			//Draw the image
			hImagelist = oWindowContainerButtons[lIndex].bImageHot ?
				hWCButtonsHot : hWCButtonsDefault;

			ImageList_Draw(hImagelist, lIndex, hDC, 
						cWCButtonRect.X, 
						cWCButtonRect.Y, 
						ILD_TRANSPARENT);

			//Adjust the window rect
			cWCButtonRect.X -= 20;
			cWindowRect.right -=20;
			}
	
		lIndex++;
		}

	cWindowRect.right -= 5;

	//Draw the title
	RECT cTitleRect;
	GetWindowRect(hWnd, &cTitleRect);

	cTitleRect.right = cWindowRect.right - cWindowRect.left;
	cTitleRect.bottom = 20;
	cTitleRect.left = 5;
	cTitleRect.top = 2;
	
	SelectObject(hDC, hFont);
	SetTextColor(hDC, RGB(255,255,255));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, sTitle.c_str(), -1, &cTitleRect, 
		DT_PATH_ELLIPSIS | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

	delete oCanvas;

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function processes the WM_COMMAND message
bool 
CWindowContainer::OnCommand(int piCommand) {

	bool bResult;

	bResult = false;
	
	switch (piCommand) {

		case IDC_WINDOWCONTAINERTOGGLE:

			DoToggleContainedWindow();
			bResult = true;
			break;

		case IDC_WINDOWCONTAINERCLOSE:

			Close();
			SendMessage(GetParent(hWnd), WM_SIZE, 0, 0);
			bResult = true;
			break;
		}

	return bResult;
	}

//This function processes the button down event
void
CWindowContainer::OnButtonDown(int piX, int piY) {

	bool bFound;
	long lIndex;

	oTrace->StartTrace(__WFUNCTION__);

	bFound = false;
	lIndex = oWindowContainerButtons.size();

	while (lIndex>0 && !bFound) {

		lIndex--;

		if (oWindowContainerButtons[lIndex].cRect.Contains(piX, piY)) {

			//Send a command to change the visible pane
			SendMessage(oWindowContainerButtons[lIndex].hCommandWindow, 
				WM_COMMAND, 
				oWindowContainerButtons[lIndex].iCommand, 0);

			bFound = true;
			}
		}

	oTrace->EndTrace(__WFUNCTION__);
	}


//This function handles the WM_NOTIFY message
bool 
CWindowContainer::OnNotify(WPARAM pwParam, LPARAM plParam) {

	bool bResult;
	LPNMHDR cNmhdr;
	LPNMTTDISPINFO cNmDispInfo;
	
	bResult = false;
	cNmhdr = (LPNMHDR)plParam;
		
	switch (cNmhdr->code) {
		
		case TTN_GETDISPINFO:

			cNmDispInfo = (LPNMTTDISPINFO)plParam;

			if ((int)oWindowContainerButtons.size() > (int)cNmDispInfo->lParam) {

				wchar_t* cText;

				//Alloc text buffer
				cText = (wchar_t*)malloc(sizeof(wchar_t) * 400);

				//Update title
				LoadString(oGlobalInstances.hLanguage, oWindowContainerButtons[(int)cNmDispInfo->lParam].iTooltip, cText, 400);
				sTooltip = cText;

				//Free memory
				free(cText);

				cNmDispInfo->lpszText = (LPTSTR)sTooltip.c_str();
				}

			bResult = true;
			break;
		}

	return bResult;

}

//This function hides or shows the contained window
void 
CWindowContainer::DoToggleContainedWindow() {

	sWindowContainerButton cButton;

	bContainedWindowVisible = !bContainedWindowVisible;
		
	GetButton(IDC_WINDOWCONTAINERTOGGLE, cButton);

	cButton.iTooltip = bContainedWindowVisible ? IDS_HIDE : IDS_SHOW;
	cButton.sImage = bContainedWindowVisible ? L"PNG_TOOLARROWUP" : L"PNG_TOOLARROWDOWN";

	UpdateButton(IDC_WINDOWCONTAINERTOGGLE, cButton);

	if (!bContainedWindowVisible) {

		iHeightWindow = oConnectedSplitter->GetSiblingSize();
		oConnectedSplitter->SetSiblingSize(22);
		}
	else {

		oConnectedSplitter->SetSiblingSize(iHeightWindow);
		}

	SendMessage(GetParent(hWnd), WM_SIZE, 0, 0);
}


//Create a tooltip control
void 
CWindowContainer::DoTooltipCreate ()
{
    /* CREATE A TOOLTIP WINDOW */
    hTooltip = CreateWindowEx(WS_EX_TOPMOST,
        TOOLTIPS_CLASS,
        NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hWnd,
        NULL,
        oGlobalInstances.hInstance,
        NULL
        );

    SetWindowPos(hTooltip,
        HWND_TOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    /* GET COORDINATES OF THE MAIN CLIENT AREA */
//    GetClientRect (hWnd, &rect);
}

//This function adds a tooltip
void 
CWindowContainer::DoTooltipAdd(UINT_PTR piId, Rect pcRect) {

    TOOLINFO ti;
 
	//Initialize the structure of the tooltip
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hWnd;
    ti.hinst = oGlobalInstances.hInstance;
    ti.uId = piId;
    ti.lpszText		= LPSTR_TEXTCALLBACK;
    ti.rect.left	= pcRect.X;
    ti.rect.top		= pcRect.Y;
    ti.rect.right	= pcRect.X + pcRect.Width;
    ti.rect.bottom	= pcRect.Y + pcRect.Height;
	ti.lParam		= (LPARAM)piId;

	//Add the tool to the tooltip window
	SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
	}

//This function localizes (updates the text) of the give tooltip
void 
CWindowContainer::DoTooltipUpdateRect(UINT_PTR piId, Rect pcRect) {

    TOOLINFO ti;
 
	//Initialize the structure of the tooltip
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hWnd;
    ti.uId = piId;
	ti.lpszText = NULL;

	SendMessage(hTooltip, TTM_GETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &ti);

    ti.rect.left	= pcRect.X;
    ti.rect.top		= pcRect.Y;
    ti.rect.right	= pcRect.X + pcRect.Width;
    ti.rect.bottom	= pcRect.Y + pcRect.Height;

	ti.lpszText = LPSTR_TEXTCALLBACK;

	SendMessage(hTooltip, TTM_SETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &ti); 
	}

//This function adds a button to the WCButtons vector, and als
//create a tooltip for the button.
void 
CWindowContainer::AddButton(sWindowContainerButton pcButton) {

	Bitmap* oBitmap;

	//Add the icon to the imagelist
	oBitmap = CBitmap::LoadFromResource(pcButton.sImage.c_str(), L"PNG", hInstance);

	CBitmap::AddToImagelist(oBitmap, hWCButtonsDefault, false, true);
	CBitmap::AddToImagelist(oBitmap, hWCButtonsHot,		false, false);

	delete oBitmap;

	//Add a tooltip
	DoTooltipAdd(oWindowContainerButtons.size(), Rect(0, 0, 0, 0));
	
	oWindowContainerButtons.push_back(pcButton);
}

//Update the button
void 
CWindowContainer::UpdateButton(int piCommand, 
							   sWindowContainerButton pcButton) {

	long lIndex;

	lIndex = oWindowContainerButtons.size();

	while (lIndex>0) {

		lIndex--;

		if (oWindowContainerButtons[lIndex].iCommand == piCommand) { 

			oWindowContainerButtons[lIndex].bImageHot = pcButton.bImageHot;
			oWindowContainerButtons[lIndex].bVisible = pcButton.bVisible;
			oWindowContainerButtons[lIndex].iTooltip = pcButton.iTooltip;
			oWindowContainerButtons[lIndex].iCommand = pcButton.iCommand;
			oWindowContainerButtons[lIndex].hCommandWindow = pcButton.hCommandWindow;

			if (oWindowContainerButtons[lIndex].sImage != pcButton.sImage.c_str()) {

				oWindowContainerButtons[lIndex].sImage = pcButton.sImage.c_str();
				
				//Add the icon to the imagelist
				Bitmap* oBitmap = CBitmap::LoadFromResource(pcButton.sImage.c_str(), L"PNG", hInstance);

				CBitmap::AddToImagelist(oBitmap, hWCButtonsDefault, false, true, lIndex);
				CBitmap::AddToImagelist(oBitmap, hWCButtonsHot,		false, false, lIndex);
		
				delete oBitmap;
				}
			}
		}	
	}

//Get the button
void 
CWindowContainer::GetButton(int piCommand, 
							sWindowContainerButton &pcButton) {

	long lIndex;

	lIndex = oWindowContainerButtons.size();

	while (lIndex>0) {

		lIndex--;

		if (oWindowContainerButtons[lIndex].iCommand == piCommand) { 

			pcButton.bImageHot		= oWindowContainerButtons[lIndex].bImageHot;
			pcButton.bVisible		= oWindowContainerButtons[lIndex].bVisible;
			pcButton.iTooltip		= oWindowContainerButtons[lIndex].iTooltip;
			pcButton.iCommand		= oWindowContainerButtons[lIndex].iCommand;
			pcButton.hCommandWindow = oWindowContainerButtons[lIndex].hCommandWindow;
			pcButton.sImage			= oWindowContainerButtons[lIndex].sImage;
			}
		}	
	}

//This function will return true if the container is visible (not closed)
bool
CWindowContainer::IsVisible() {

	return bContainerVisible;
	}

//This function will hide this window(container);
void 
CWindowContainer::Close() {

	bContainerVisible = false;
	ShowWindow(hWnd, SW_HIDE);
	SendMessage(GetParent(hWnd), WM_SIZE, 0, 0);
	}


//This function will make the contained window visible if it is hidden
//and/or closed
void 
CWindowContainer::MakeVisible() {

	if (!bContainerVisible) {

		bContainerVisible = true;
		ShowWindow(hWnd, SW_SHOW);
		SendMessage(GetParent(hWnd), WM_SIZE, 0, 0);
		}
	
	if (!bContainedWindowVisible) {

		DoToggleContainedWindow();
		}
	}


