#pragma once

class CDlgPreferencesScannerImage :
	public CDialogTemplate
{
public:
	CDlgPreferencesScannerImage(HINSTANCE hInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances);
	~CDlgPreferencesScannerImage(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void OnApply();

	virtual void Localize(HINSTANCE phLanguage=NULL);
private:

	void UpdateEditControl(int pIdSlider);
	void UpdateSliderControl(int pIdEdit);
	void DoEnableControls();
	void DoUpdateImage(LPDRAWITEMSTRUCT pcDrawItem);
	void DoFillColorDepth(HWND hColorBox);
	void DoFillResolution(HWND hResolutionBox);
};
