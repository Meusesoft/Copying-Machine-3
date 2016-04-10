#pragma once

enum ePreferences { eAllPreferences, eGeneral, eAutoSave, ePrinter, ePrinterPage, ePrinterCopiesAlignment,
					ePrinterMagnificationClipStretch, ePrinterDevice,
					eScanner, eScannerPage, eScannerImage, eScannerDevice, eScannerAdvanced, 
					eTrace};

class CDlgPreferences :
	public CDialogTemplate
{
public:
	CDlgPreferences(HINSTANCE hInstance, HWND phParent, ePreferences pcPreferences, sGlobalInstances poGlobalInstances);
	~CDlgPreferences(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void OnApply();
	virtual void Localize(HINSTANCE);

private:

	void ShowPreferences(ePreferences pePage);
	void FillTreeView();
	void InitializeTreeView();
	bool OnEventCoreNotification();

	HTREEITEM AddTreeItem(HTREEITEM phParent, int piIcon, int piResource, ePreferences pePage);

	CDlgPreferencesGeneral					*oPreferencesGeneral;
	CDlgPreferencesAutoSave					*oPreferencesAutoSave;
	CDlgPreferencesScannerImage				*oPreferencesScannerImage;
	CDlgPreferencesScannerPage				*oPreferencesScannerPage;
	CDlgPreferencesScannerAdvanced			*oPreferencesScannerAdvanced;
	CDlgPreferencesScannerDevice			*oPreferencesScannerDevice;
	CDlgPreferencesTrace					*oPreferencesTrace;
	CDlgPreferencesPrinterCopiesAlignment	*oPreferencesPrinterCopiesAlignment;
	CDlgPreferencesPrinterImageSize			*oPreferencesPrinterImageSize;
	CDlgPreferencesPrinterDevice			*oPreferencesPrinterDevice;

	HWND hVisiblePage;
	HIMAGELIST hImagelist;

	ePreferences cPreferences;
	
	vector <CDialogTemplate*> oPreferencePages;
};
