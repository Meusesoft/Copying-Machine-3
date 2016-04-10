#pragma once

class CFiller
{
public:
	CFiller(CTracer* poTrace);
	~CFiller(void);

	HWND Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra);
	void Destroy();

	bool WndProc(HWND hWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn);
	HWND hWnd;

protected:

private:

	HINSTANCE hInstance;

	CTracer* oTrace;
};
