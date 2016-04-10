#pragma once

DWORD WINAPI ThreadOCRPage(LPVOID lpParameter);

class CDlgRecognizeText :
	public CDialogTemplate
{
public:
	CDlgRecognizeText(HINSTANCE hInstance, HWND phParent, DWORD pdPageId, sGlobalInstances poGlobalInstances);
	~CDlgRecognizeText(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();


private:

	bool OnEventCoreNotification();
	void EnableControls();

	void DoOcr(DWORD pdPageId);
	void DoCancelOcr();
	void DoToClipboard();
	void DoFillLanguageControl();
	int GetSelectedLanguage();
	void DrawPictureControl(LPARAM plParam);

	DWORD dPageId;
	int iOCRTicket;
	HGLOBAL hImage;
};
