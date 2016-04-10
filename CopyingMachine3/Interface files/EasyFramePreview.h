
class CEasyFramePreview
{
public:
	CEasyFramePreview(sGlobalInstances pInstances);
	~CEasyFramePreview(void);

	HWND Create(HINSTANCE phInstance, HWND phWnd);
	void Destroy();

	static BOOL CALLBACK DialogTemplateProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam); 
	bool OnMessage(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam); 

	void OnInitDialog();
	void SetPreviewDocument(int piDocumentID, CPrintSettings* poPrintSettings);

	HWND hWnd;
	int nCmdShow;

private:

	bool OnCommand(int piCommand, WPARAM wParam, LPARAM lParam);
	bool OnEventCoreNotification();

	void DoEnableControls();
	void DoFillControls();
	void DoLocalize();
	void DoUpdatePreview(LPDRAWITEMSTRUCT pcDrawItem);
	void DoCreatePreview();
	void DoClearPreview();

	void CreateDoubleBuffer();
	void DoubleBufferDrawBackground();
	void DoubleBufferDrawPage();

	void LocalizeControl(int piControl, int piResource);
	void LocalizeWindowCaption(int piResource);
	int  GetIntegerFromEditControl(int piControl);

	//Variables for the preview
	int		 iDocumentID;
	CPrintSettings* oPrintPreviewSettings;
	int		 iPrintTicket;
	CBitmap* oDoubleBufferBitmap;
	CBitmap* oPreviewBitmap;
	Rect	 cPageRect;
	float	 fPreviewDpiX, fPreviewDpiY;
	

	//Global instances
	sGlobalInstances oGlobalInstances;
	HINSTANCE hInstance;
	HWND	  hParent;
	CTracer* oTrace;
	CRegistry* oRegistry;
	CCopyingMachineCore* oCore;
};
