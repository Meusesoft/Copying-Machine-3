#pragma once

class CDlgPreferencesGeneral :
	public CDialogTemplate
{
public:
	CDlgPreferencesGeneral(HINSTANCE hInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances);
	~CDlgPreferencesGeneral(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void OnApply();

	virtual void Localize(HINSTANCE phLanguage=NULL);

private:

	void DoValidateControls();
	void DoRemoveAssociation(wstring psExtension);
	void DoAssociate(wstring psExtension);
	bool DoTestAssociation(wstring psExtension);
	void DoFillControls();
	void DoFillLanguageControl();
	void DoRegisterStillImage(bool pbRegister);

	bool bInitialised;
};
