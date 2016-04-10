#pragma once

DWORD WINAPI ThreadScanWia(LPVOID lpParameter);

class CScanWia : 
	public CScanInterfaceBase
{
public:
	CScanWia(sGlobalInstances pInstances, CScanSettings* poSettings);
	~CScanWia(void);

	void Release() {delete this;};
	void ShowLastError();
	void ShowResult(HRESULT phr);

	void GetDeviceManager();
	void ReleaseDeviceManager();

	//functies voor het uitvoeren van acties
	bool AskDefaultScanner(CScanSettings* poSettings);
	bool GetDeviceCapabilities(CScanSettings* poSettings);
	int GetBrightness(CScanSettings* poSettings);
	int GetContrast(CScanSettings* poSettings);
	eAcquireResult AcquireImages(CScanSettings* poSettings);
	eAcquireResult AcquireImagesThread(CScanSettings* poSettings);

	//Controle functies
	bool CheckExistingID(CScanSettings* poSettings);
	bool WiaUsable(CScanSettings* poSettings);
	bool WiaAvailable();

	//Enumarator functies voor scan devices
	CScanDevice* GetFirstDevice();
	CScanDevice* GetNextDevice();

	bool GetEndThread();
	void SetEndThread();
	

	//Thread 
	CRITICAL_SECTION ScanCriticalSection; 
	HWND hProgressBar;				//Progress bar on progress form
	HANDLE hScanThread;

private:

	CRITICAL_SECTION EndThreadCriticalSection; 
	bool bEndThread;


	void*  oWiaDeviceManager;  //Pointer naar WIA Device Manager
	void*  oWiaRootItem;       //Pointer naar root item WIA Device
	void*  oWiaFirstChildItem; //Pointer naar eerste item in WIA Device
	void*  oWiaEnumItem;       //Pointer naar enumerator voor child items

	bool GetDeviceID(CScanSettings* poSettings);
	bool GetRootItem(CScanSettings* poSettings);
	bool GetNextItem();
	void VerticalFlip(BYTE *pBuf);
	eAcquireResult AcquireWiaItem(IWiaPropertyStorage *pIWiaPropStg, void* poWiaItem, CScanSettings* poSettings);

	//-----------------------------------------------------------------
	std::vector<CScanDevice*> oScanDevices;
	int iScanDeviceIndex; 

	void* oDlgProgress;
	HWND hProgressDlg;				//Progress dialog
	};
