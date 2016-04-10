#pragma once

//#include "wiaclass.h" 
#include "TWAIN.h"
#include "ScanSettings.h"
#include "ScanStructures.h"


class CScanTwain:
	public CScanInterfaceBase {

public:
	CScanTwain(sGlobalInstances pInstances, CScanSettings* poSettings);
	~CScanTwain();

	bool	DSMLoaded();
	eAcquireResult	CmAcquire(CScanSettings* poSettings);
	bool	GetDeviceCapabilities(CScanSettings* poSettings);

	void	CmSelectSource();
	void	CmScannerSettings(int Page=0);
	bool	ProcessTwainMessage(MSG &m);
	void	DisableDSM();
	int		GetScannedDPI();
	bool	TestMemory();
	
	bool	InitSet;
	void	InitScannerSet();
	bool	Available;
	bool	bDefaultScannerAvailable;
	CScanDevice* oLastScanDevice;
	bool	DetermineDefaultScanner();
	bool	GetDefaultScannerID(char* cDeviceID, int iMaxLen);
	//bool	PagesAvailable();

	bool bLastScanWithInterface;

	CScanDevice* GetFirstDevice();
	CScanDevice* GetNextDevice();

protected:
	bool	EnableDSM();

	bool	OpenDS();
	void	CloseDS();

	void	DisableDS();

	void	DoTransfer();
	bool	IsPicture();

	bool	NegotiateCapabilities();
	bool	NegotiateResolution();
	bool	NegotiateBrightness();
	bool	NegotiateContrast();
	bool 	NegotiateMeasure();
	bool 	NegotiateColor();
	bool 	NegotiateOrientation();
	bool	NegotiatePixelFlavor();
	bool	NegotiateUIControllable();
	bool	NegotiatePage();
	bool	NegotiateFeeder();
	bool	NegotiateDuplex();

	void	SetResolution();
	void	SetColor();
	void 	SetCompression();
	void	SetBrightness();
	void 	SetContrast();
	void	SetMeasure();
	void	SetPage();
	void	SetOrientation();
	void 	SetDuplex();

	bool	CheckMessage();
	bool	DeviceHasFeeder;
	bool	RetrieveCurrentResolution();
	void	SetCap(int what, TW_UINT16 number);                                                     
	float	FIX32ToFloat (TW_FIX32    fix32);
	TW_FIX32	FloatToFix32 (float floater);
	int		GetItemFromEnum(int Index, pTW_ENUMERATION);
	int		GetItemFromArray(int Index, pTW_ARRAY);
	int		GetItem(TW_UINT16, TW_UINT32);
	TW_UINT16	SetItem(TW_UINT16 , pTW_UINT32 , int );

private:
	bool TranslateReturnStatus(TW_UINT16 status, std::wstring &sErrorMessage);
	bool SetResolution(int piTwainCapacity);

	void SetImageAcquired(HGLOBAL hImage);

	/*
	//start functies voor WIA integratie
	bool UseWia();
	bool WiaAvailable();
	bool EnableWia();
	bool DisableWia();

	bool				WiaOpen;
	WiaClass*			oWia;
	HINSTANCE			hWiaDLL;
	ScanStruct 			sWiaCapabilityStruct;
	ImagesStruct		sWiaImagesStruct;
	//einde functies voor WIA integratie
	*/

	TW_USERINTERFACE	twUI;
	bool				TWDSMOpen,TWDSOpen;

	DSMENTRYPROC 	lpDSM_Entry;
	TW_IDENTITY 	appID, dsID;
	HANDLE          hbm_acq;
	HANDLE 			hDSMDLL;
	bool 			UIControllable;

	int StandardResolution;

	int ScanResolution;
	int ResolutionFromImage;
	int CalculatedResolutionFromImage;
	float SetPaperWidth;
	int CurrentResolution;

	bool Chocolate;
	bool bFeeder;
	bool bDuplex;
	bool bPagesAvailable;

};

