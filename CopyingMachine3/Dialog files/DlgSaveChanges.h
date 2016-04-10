#pragma once

class CDlgSaveChanges :
	public CDialogTemplate
{
public:
	CDlgSaveChanges(HINSTANCE hInstance, HWND phParent, std::wstring psMessage, sGlobalInstances poGlobalInstances);
	~CDlgSaveChanges(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();

private:

	std::wstring sMessage;
};
