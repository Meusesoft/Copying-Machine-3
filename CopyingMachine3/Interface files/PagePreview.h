#pragma once

class CPagePreview
{
public:
	CPagePreview(sGlobalInstances pInstances);
	~CPagePreview(void);

	void DoSubscribeNotifications(HWND pHwnd);
	void SetPreviewDocument(int piDocumentID, CPrintSettings* poPrintSettings);
	void Redraw(HWND phWnd);

protected:

	bool OnEventCoreNotification(HWND phWnd, sCoreNotification pcNotification);

	void DoUpdatePreview(LPDRAWITEMSTRUCT pcDrawItem);
	void DoCreatePreview();
	void DoClearPreview();

	void CreateDoubleBuffer(HWND phWnd);
	void DoubleBufferDrawBackground();
	void DoubleBufferDrawPage();

	//Variables for the preview
	int		 iDocumentID;
	CPrintSettings* oPrintPreviewSettings;
	int		 iPrintTicket;
	CBitmap* oPreviewBitmap;
	Bitmap* oClientBitmap; //a bitmap for double buffering the client area
	Rect	 cPageRect;
	float	 fPreviewDpiX, fPreviewDpiY;
	

	//Global instances
	sGlobalInstances	oGlobalInstances;
	HINSTANCE			hInstance;
	CTracer*			oTrace;
	CRegistry*			oRegistry;
	CCopyingMachineCore* oCore;
};
