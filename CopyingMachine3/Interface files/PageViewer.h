#pragma once
#include "CopyingMachineCore.h"
#include "PagePreview.h"
#include "WindowScrollbar.h"
#include "WindowBase.h"

enum ePageViewType {ePVTImage, ePVTPrintPreview};
enum eHotSpotType {eHSTNone, eHSTSelection, 
				   eHSTSelectionTop, eHSTSelectionLeft, 
				   eHSTSelectionBottom, eHSTSelectionRight, 
				   eHSTSelectionNorthWest, eHSTSelectionSouthEast,
				   eHSTSelectionNorthEast, eHSTSelectionSouthWest};

enum eMouseState {eMouseStateStandard, 
				  eMouseStateHotSpot, 
				  eMouseStateSelectionMove,
				  eMouseStateColorPicker};

struct sMouseState {

	eMouseState cMouseState;
	eHotSpotType cType;
	PointF cMouseClickPoint;
	};

struct sPageViewerHotSpot {

	RectF cHotSpotRect;
	eHotSpotType eType;
	};

struct sPageViewerSettings {

	DWORD			dPageID;			//To which page does it belong
	float			fMagnification;		//Magnification of the document (percentage of original document)
	bool			bSizeToFit;			//Magnification is set to fit the screen
	float			fPosX;				//Position X for drawing 
	float			fPosY;				//Position Y for drawing 
};

struct sPageViewerPosition {

	POINT cImagePoint;
	POINT cClientPoint;
};

struct sCachedImageRequest {

	Bitmap* oImage;
	float	 fMagnification;
	DWORD	 dRequestedOnTick;
	};

DWORD WINAPI ThreadCreateCachedImage(LPVOID lpParameter);

class CPageViewer : public CPagePreview, public CWindowScrollbar
{
public:
	CPageViewer(sGlobalInstances pInstances, std::wstring psWindowName);
	~CPageViewer(void);

	HWND Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra);
	void Destroy();
	
	bool WndProc(HWND hWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn);
	
	void ProcessUpdatedSettings();
	void NewPage();

	float GetMagnification();
	void SetMagnification(float pfMagnification);
	void SetMagnificationToSelection();
	void SelectionAll();
	void SelectionClear();
	void SelectionSwap();
	void SelectContent();
	void SetSizeToFit();
	bool GetSizeToFit();
	void SetColorPicker();
	void ToggleRecognitionLayer();
	bool GetRecogntionLayerVisible();

	void Localize(HINSTANCE phLanguage);
	void SetPageViewType(ePageViewType pcNewType);
	ePageViewType GetPageViewType();

	Bitmap* CreateCachedBitmap(Bitmap* poBitmap, float pfMagnification, Graphics* poCanvas);

	sPageViewerPosition GetImagePosition(POINT pcClientPosition);
	void SetImagePosition(sPageViewerPosition pcPosition);
	void Redraw();
	

	//Variables for communicating with the thread
	bool bEndThread;
	std::vector<sCachedImageRequest> vToBeProcessed;
	CRITICAL_SECTION ThreadCriticalSection;
	HANDLE hCachedBitmapThread;
	Bitmap* oNewCachedBitmap;
	float   fMagnificationNewCachedBitmap;

	//Position in image of last context menu
	PointF cImagePositionContextMenu;

protected:

	HINSTANCE hInstance;
	HWND hSibling;

	CCopyingMachineCore* oCore;
	CRegistry* oRegistry;
	CCoreNotifications* oNotifications;

private:

	void DrawClient();
	void DrawImage(Graphics* oCanvas);
	void DrawPrintPreview(Graphics* oCanvas);
	void DrawSelection(Graphics* oCanvas, RectF cDestinationRect);
	void DrawRecognitionLayer(Graphics* oCanvas);
	void RequestCachedBitmap(Bitmap* poBitmap, Graphics* poCanvas);
	void ClearCachedBitmap();
	void CreateClientBitmap(Graphics* poCanvas);
	void ClearClientBitmap();

	bool OnEventMouse(UINT message, WPARAM wParam, LPARAM lParam);
	bool OnEventMouseWheel(WPARAM wParam, LPARAM lParam);
	bool OnEventCoreNotification();
	void OnEventScrollbar(UINT message, WPARAM wParam, LPARAM lParam);
	
	void DoSelectionContextMenu(int piX, int piY);
	void DoSelectionCommit();
	
	void ClearMouseState();
	void SetMouseStateMoveHotspot(eHotSpotType peType);
	void SetMouseState(sMouseState &pcState);

	void SetHotSpot(RectF pcHotSpot, eHotSpotType peType, bool pbInFront = true);
	void RemoveHotSpot(eHotSpotType peType);
	void UpdateHotSpotPosition(eHotSpotType peType, int iX, int iY);
	eHotSpotType MouseOverHotSpot(int piX, int piY);
	vector <sPageViewerHotSpot> oHotSpots;
	HCURSOR GetCursorType(eHotSpotType peType);

	bool   ClientPositionInImage(float pfX, float pfY);
	PointF ConvertClientToImage(float pfX, float pfY);
	PointF ConvertImageToClient(float pfX, float pfY);


	sPageViewerSettings* oSettings;
	sPageViewerSettings* oMySettings; //settings created for empty viewer
	sMouseState			 oMouseState; //state of the mouse, what are we doing?

	Bitmap* oCachedBitmap;
	float fMagnificationCachedBitmap;

	HCURSOR		hCursorArrow;
	HCURSOR	    hCursorSizeAll;
	HCURSOR	    hCursorSizeNS;
	HCURSOR	    hCursorSizeWE;
	HCURSOR	    hCursorSizeNWSE;
	HCURSOR	    hCursorSizeNESW;
	HCURSOR	    hCursorHand;

	ePageViewType cPageViewType;

	sPageViewerSettings* GetSettings(DWORD pdPageID);
	sPageViewerSettings* CreateSettings(DWORD pdPageID);
	bool SetSettings(sPageViewerSettings* cSettings);
	bool ClearSettings(DWORD pdPageID);
	bool ClearAllSettings();
	bool bRecognitionLayerVisible;

	vector<sPageViewerSettings*> oPageSettings;

	HIMAGELIST  hSelectionGripImages;
	vector <Bitmap*> oSelectionGripImages;
	HWND hWndTooltip;
};
