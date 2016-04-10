#pragma once

class CDlgAbout :
	public CDialogTemplate
{
public:
	CDlgAbout(HINSTANCE hInstance, HWND phParent, sGlobalInstances poGlobalInstances);
	~CDlgAbout(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();

private:

	void DoShowControls();
};
