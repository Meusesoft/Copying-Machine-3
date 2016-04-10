#pragma once

class CDlgPreferencesScannerPage :
	public CDialogTemplate
{
public:
	CDlgPreferencesScannerPage(HINSTANCE hInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances);
	~CDlgPreferencesScannerPage(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void OnApply();

	virtual void Localize(HINSTANCE phLanguage=NULL);
private:

	void DoEnableControls();
	void DoUpdateImage();
};
