#pragma once

struct sMenuCommandIcon {

	int iMenuCommand;
	int iMenuIcon;
	std::wstring sMenuIcon;
	int iImagelistIndex;
};

struct sMenuCommandEnabled {
	
	int iMenuCommand;
	bool bEnabled;
	bool bChecked;
	};

enum eWindowsVersion {eWindowsUnknown, eWindowsOlderThanXP, eWindowsXP, eWindowsVista, eWindowsNewerThanVista};

class CMenuIcon
{
public:
	CMenuIcon(HINSTANCE phInstance);
	~CMenuIcon(void);

	void ProcessMenu(HMENU phMenu);
	void AddMenuCommandIcon(int piMenuCommand, int piMenuIcon, COLORREF pcTransparentColor);
	void AddMenuCommandIcon(int piMenuCommand, std::wstring psPNGImage);
	void ReplaceMenuCommandIcon(int piMenuCommand, HBITMAP phBitmap);
	void SetEnableMenuCommand(int piMenuCommand, bool pbEnabled);
	bool GetEnableMenuCommand(int piMenuCommand);
	void SetCheckMenuCommand(int piMenuCommand, bool pbEnabled);
	bool GetCheckMenuCommand(int piMenuCommand);

	bool WndProc(HWND hWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn);
protected:

private:

	long GetIcon(int puMenuCommand);

	HINSTANCE hInstance;

	eWindowsVersion				cWindowsVersion;
	vector <sMenuCommandIcon>	oMenuCommandIcons; //The known combinations of commands and icons
	vector <sMenuCommandEnabled> oMenuCommandEnabled; //Is the command enabled?
	vector <HBITMAP>			oMenuIcons; //WindowsVista+ : list of menu icons
	HIMAGELIST					hMenuIcons; //WindowsXP: list of menu icons
};
