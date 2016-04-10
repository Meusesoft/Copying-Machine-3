#pragma once

class CScanCore : public CScriptObject
{
public:
	CScanCore(sGlobalInstances pInstances, CScanSettings* poSettings); 
	~CScanCore(void);

	bool ProcessMessage(MSG &message);

	bool Acquire(HWND phWnd);
	bool Acquire(HWND phWnd, CScanSettings* poScanSettings);
	eAcquireResult Acquire(CScanSettings* poScanSettings);

	cErrorMessage GetLastErrorMessage(bool pbClearAll = false);
	int GetErrorMessageCount();
	void ShowErrorMessage(HWND phWnd);


	cImageTransferStruct GetImage();
	int ImagesAvailable();
	void ClearImages();
	eAcquireType GetLastAcquisitionType();

	bool SetCurrentScanner(CScanDevice* pcScanDevice);
	CScanDevice* GetCurrentScanner();

	CScanDevice* GetFirstDevice(bool pbReset=false);
	CScanDevice* GetNextDevice();
	CScanDevice* GetDevice(eScanInterface pcInterface, std::wstring psDevice, std::wstring psDeviceId);

	bool GetEndThread();
	void SetEndThread();

	void GetDeviceCapabilities();
	eScanInterface GetCurrentInterface();

	CScanSettings* GetDefaultScanSettings();
	CScanSettings* GetLastScanSettings();

private:

	sGlobalInstances oGlobalInstances;
	CTracer* oTrace;
	CCoreNotifications* oNotifications;
	CScanSettings*	oDefaultScanSettings;
	CScanSettings*	oLastScanSettings;
	CScanWia*		oScanWia;
	CScanTwain*		oScanTwain;
	CScanLoadFile*	oScanLoadFile;

	long lCurrentDevice;
	eScanInterface cCurrentAcquisitionInterface;

	std::vector <CScanDevice*> oScanDevices;
	int iScanDeviceIndex;
};
