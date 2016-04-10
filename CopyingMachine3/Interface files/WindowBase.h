#pragma once

class CWindowBase {

public:
	CWindowBase(sGlobalInstances oGlobalInstances,
					std::wstring sWindowName);
	~CWindowBase();

	virtual void Destroy();

	virtual bool WndProc(HWND hWnd, UINT message,
			WPARAM wParam, LPARAM lParam, 
			int &piReturn);

	virtual void LayoutChildWindow();
	virtual void SaveSettings();
	virtual void LoadSettings();

	//Child window operations
	void AddChildWindow(CWindowBase* poChild);
	bool RemoveChildWindow(CWindowBase* poChild);
	int  ChildWindowCount();
	CWindowBase* GetChildWindow(int piIndex);

	HWND hWnd;

protected: 

	virtual bool OnCommand(int piCommand);
	virtual bool OnSize();

	std::wstring sWindowName;
	CTracer* oTrace;
	sGlobalInstances oGlobalInstances;

private:

	std::vector <CWindowBase*> oChildWindows;

	HINSTANCE hInstance;
};
