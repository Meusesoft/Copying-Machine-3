#pragma once

class CDlgUnknownEvent :
	public CDialogTemplate
{
public:
	CDlgUnknownEvent(sGlobalInstances poGlobalInstances, HWND phParent, std::wstring psEvent);
	~CDlgUnknownEvent(void);

	int iDialogResult;

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();

private:

	std::wstring sEvent;
};
