#pragma once
#include "ScanStructures.h"
#include "ScanDevice.h"

class CScanSettings : public CScriptObject
{
public:
	CScanSettings(sGlobalInstances pInstances);
	~CScanSettings(void);

	void Load();
	void Save();
	void Copy(CScanSettings* poSettings);
	void Initialize();

	bool SetScanner(CScanDevice* poScanDevice);
	CScanDevice* GetScanner();

	void SetInt(eScanSetting pSetting, unsigned int piValue);
	unsigned int GetInt(eScanSetting pSetting);
	void SetBool(eScanSetting pSetting, bool piValue, bool pbNotify = true);
	bool GetBool(eScanSetting pSetting);

	eAcquireType cAcquireType;		//chosen acquisition type (see eAcquireType)
	int iCollection;				//collection to save document to
	std::wstring sFolder;			//folder in collection to save document to
	bool bSendNotifications;		//If true notifications of changes will be send


	//Parent window
	HWND hWindow;

	//Progress bar
	HWND hProgressBar;

	//overload functions from ScriptObject
	virtual sScriptExpression Get(std::wstring psObjectname);
	virtual bool Set(std::wstring psObjectname, sScriptExpression psNewValue);

private:
	
	//These are scansettings set by the user. These are declared private so all
	//other classes need to use the set and get methods. These methods contain
	//extra functionality for messaging the changes.

	//toon userinterface
	bool bShowInterface;			//Toon user inteface van scanner voor scannen J/N
	bool bAlwaysShowInterface;		//Toon altijd de user interface
	bool bShowErrorMessage;			//Show error messages while scanning Y/N
	bool bAskForNextPage;			//Ask to scan for another page after acquisition
	bool bUseFeeder;				//Use the document feeder
	bool bAssumeDPI;				//When invalid DPI detected, assume a DPI or give a warning
	int  iAssumedDPI;				//The assumed DPI when an invalid DPI is detected

	bool bBrightnessNegotiate; 		//gaan we onderhandelen over de helderheid?
	bool bBrightnessUse;			//gaan we de helderheid gebruiken?
	int iBrightnessValue;	   		//gewenste brightness waarde (-100 t/m +100)

	bool bContrastNegotiate;   		//gaan we onderhandelen over het contrast?
	bool bContrastUse;		   		//gaan we het contrast gebruiken?
	int iContrastValue;	       		//gewenste contrast waarde (-100 t/m +100)

	bool bPageNegotiate;	   		//gaan we onderhandelen over de pagina omvang? J/N
	bool bPageUse;					//gaan we de pagina omvang gebruiken? J/N
	unsigned int iPage;		   		//welke pagina omvang: -1=default, 0=a4, 1=a5, 2=Letter, 3=Legal

	bool bOrientationNegotiate;		//gaan we onderhandelen over de orientatie?
	bool bOrientationUse;			//gaan we onderhandelen over de orientatie?
	unsigned int iOrientation; 		//gewenste orientatie: 0 = Portrait, 1=Landscape

	bool bColorDepthNegotiate;		//gaan we onderhandelen over de kleurdiepte J/N
	bool bColorDepthUse;			//gaan we de kleurdiepte gebruiken J/N
	unsigned int iColorDepth;		//gekozen kleur: -1=default; 0=BW; 1=Grijs; 2=Kleur
	bool bInvertColor;				//Inverteer de kleur na scannen?

	bool bResolutionNegotiate;		//gaan we onderhandelen over de resolutie J/N
	bool bResolutionUse;			//gaan we de resolutie gebruiken J/N
	unsigned int iResolution;  		//gekozen resolutie in dpi : -1=default

	void DoChangedSetting();
	
	CTracer* oTrace;
	CRegistry* oRegistry;
	CCoreNotifications* oNotifications;
	void* oCore;

	CScanDevice*	oScanDevice;
};
