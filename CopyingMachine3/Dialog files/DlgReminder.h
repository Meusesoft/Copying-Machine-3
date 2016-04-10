#pragma once

class CDlgReminder :
	public CDialogTemplate
{
public:
	CDlgReminder(HINSTANCE hInstance, HWND phParent, sGlobalInstances poGlobalInstances);
	~CDlgReminder(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();

private:

	HWND hProgressControl;
	long lCountDown;
};
