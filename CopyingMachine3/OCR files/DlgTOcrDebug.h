#pragma once

class CDlgTOcrDebug :
	public CDialogTemplate
{
public:
	CDlgTOcrDebug(HINSTANCE hInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances);
	~CDlgTOcrDebug(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();

private:

	
};
