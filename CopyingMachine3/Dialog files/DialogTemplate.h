#pragma once

class CDialogTemplate {

public:
	
	CDialogTemplate(HINSTANCE hInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances);
	~CDialogTemplate();

	int Execute();
	HWND Create();

	static INT_PTR CALLBACK DialogTemplateProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam); 

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam);
	virtual	void OnCloseOk();
	virtual void OnApply();

	virtual void Localize(HINSTANCE phLanguage=NULL);

	static void ShowBalloonTip(HINSTANCE phLanguage, HWND phControl, int piMessage, int piTitle);

	HWND hDlg;

protected:

	bool SetDlgItemFont(UINT uItem, HFONT hf);
	void LocalizeControl(int piControl, int piResource);
	void LocalizeWindowCaption(int piResource);

	int  GetIntegerFromEditControl(int piControl);
	std::wstring GetStringFromEditControl(int piControl);
	void ShowControl(int piControl, bool pbShow);
	void EnableControl(int piControl, bool pbEnable);

	HINSTANCE hInstance;
	HFONT hCaptionFont;
	CTracer* oTrace;
	void* oCore;

	sGlobalInstances oGlobalInstances;
	HWND hParent;

private:

	int iDialogResource;

};