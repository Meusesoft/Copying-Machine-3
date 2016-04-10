#pragma once

class CDlgPlaceKey :
	public CDialogTemplate
{
public:
	CDlgPlaceKey(HINSTANCE hInstance, HWND phParent, sGlobalInstances poGlobalInstances);
	~CDlgPlaceKey(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();

private:

	void	LoadKey();
	void	MessageKeyValid();
};
