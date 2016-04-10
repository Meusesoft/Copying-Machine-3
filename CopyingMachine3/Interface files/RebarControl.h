#pragma once

struct CBandInfo	{
	long lId;
	long lcx;
	DWORD bBreakLine;
	DWORD bGripper;
	DWORD bHidden;
    };

class CRebarControl
{
public:
	CRebarControl(sGlobalInstances pInstances);
	~CRebarControl(void);

	bool Create(HINSTANCE phInstance, HWND phParent);
	void Destroy();

	bool AddBand(HWND phChild, UINT pID);

	void LoadRebarSettings();
	void SaveRebarSettings();

	HWND hWnd;	//my window handle
	
private:

	CTracer* oTrace;
	CRegistry* oRegistry;

};
