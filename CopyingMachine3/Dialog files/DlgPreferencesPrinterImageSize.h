#pragma once

class CDlgPreferencesPrinterImageSize :
	public CDialogTemplate
{
public:
	CDlgPreferencesPrinterImageSize(HINSTANCE hInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances);
	~CDlgPreferencesPrinterImageSize(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void OnApply();

	virtual void Localize(HINSTANCE phLanguage=NULL);

private:

	void DoEnableControls();
	void DoUpdateImages();
};
