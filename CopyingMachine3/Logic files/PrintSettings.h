#pragma once
#include "PrintStructures.h"

struct sPrintSettingsData {

	bool bShowInterface;						//Toon user inteface van scanner voor scannen J/N
	bool bAlwaysShowInterface;					//Toon altijd de user inteface van scanner voor scannen J/N
	
	bool bMultiPagesOnPage;						//4 pages on one page
	bool bMultipleCopiesOnPage;					//Multiple copies on one page	

	bool bShrinkToFit;							//Shrink the image to fit on the page
	bool bStretchToPage;						//Stretch the image to cover the entire page
	bool bPortrait;								//Print in portrait rotation


	int iCopies;								//Number of copies 1 - 99
	int iMagnification;							//Magnification percentage 0 - 999%
	int	iAlignment;								//One value of the Print Alignment enum
	int iMarginLeft;							//Page margins
	int iMarginRight;
	int iMarginTop;
	int iMarginBottom;
	int iPageType;								//Page type, A4, A5, Letter etc.
	};

struct sPrintData {

	vector <long> lPage;
	HDC hDC;
	int iCopies;
	bool bCollate;
	std::wstring sDevice;
	bool bInPage;								//Is a StartPage called and do we still need a EndPage
	bool bInitializePageRange;					//Fill the pages to print automatically before printing?
	};

class CPrintSettings
{
public:
	CPrintSettings(sGlobalInstances pInstances);
	~CPrintSettings(void);

	void Load();
	void Save();
	void Trace();
	CPrintSettings* Copy();
	void CopyData(sPrintSettingsData& poData);

	bool SetDevice(sOutputDevice &pcOutputDevice);
	bool GetDevice(sOutputDevice &pcOutputDevice);

	void SetBool(ePrintSetting pSetting, bool piValue, bool pbNotify = true);
	bool GetBool(ePrintSetting pSetting);
	void SetInt(ePrintSetting pSetting, unsigned int piValue);
	unsigned int GetInt(ePrintSetting pSetting);
	
	//Parent window
	HWND hWindow;

	//Progress bar
	HWND hProgressBar;

	//Status windows
	HWND hStatusWindow;

	//Data set by the PrintDialog
	sPrintData oPrintData;

	
	bool bDeleteAfterPrint; //Autodelete after a print action, default = false
	bool bNotifyChanges;    //Notify about changes to the settings, default = true

private:

	eOutputType			cOutputType;			//The type of outputdevice; mail, file, printer
	std::wstring		sOutputDeviceName;		//The name of the output device
	CBitmap*				oOutputBitmap;

	sPrintSettingsData oData;

	void DoChangedSetting();

	CTracer* oTrace;
	CRegistry* oRegistry;
	CCoreNotifications* oNotifications;

	sGlobalInstances	oGlobalInstances;
};
