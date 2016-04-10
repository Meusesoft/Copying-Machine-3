#pragma once

DWORD WINAPI ThreadPrintDocument(LPVOID lpParameter);

struct sPrintDocumentWithSettings {

	int iPrintRequest;				//The number of the printrequest
	CCopyDocument*	oDocument;		//The printed document
	CPrintSettings*	oPrintSettings; //The print settings
	};

class CPrintCore :
	public CInterfaceBase
{
public:
	CPrintCore(sGlobalInstances pInstances);
	~CPrintCore(void);

	bool LoadOutputDevices();
	bool GetOutputDevice(int piIndex, sOutputDevice &oOutputDevice);
	int  GetOutputDeviceCount();
	bool FindOutputDevice(eOutputType pcType, std::wstring psDeviceName, sOutputDevice &oOutputDevice); 

	eActionResult PrintDocument(CCopyDocument* poDocument, 
								CPrintSettings* poPrintSettings);
	int PrintDocumentThread(CCopyDocument* poDocument, 
								CPrintSettings* poPrintSettings);
	RectF GetStandardPageImageArea(sOutputDevice poOutputDevice, CPrintSettings* poPrintSettings);

	//Thread 
	bool GetEndThread();
	void SetEndThread();
	
	CRITICAL_SECTION PrintCriticalSection; 
	HANDLE hPrintThread;
	vector <sPrintDocumentWithSettings*> oPrintQueue;

private:

	CRITICAL_SECTION EndThreadCriticalSection; 
	bool bEndThread;

	eActionResult ProcessDocument(Graphics* poCanvas, CCopyDocument* poDocument, 
								CPrintSettings* poPrintSettings);
	eActionResult ProcessPage(Graphics* poCanvas, CCopyDocument* poDocument, long plIndex, 
								CPrintSettings* poPrintSettings);
	eActionResult ProcessImage(Graphics* poCanvas, CCopyDocumentPage* poPage, 
								RectF prPageImageArea, float pfMagnificationAdjustment, CPrintSettings* poPrintSettings, 
								bool pbIgnoreMultipleCopiesOnPage = false);
	bool StartPrintThread();

	RectF CalculatePageImageArea(Graphics* poCanvas, long plIndex, float &pfMagnificationAdjustment, CPrintSettings* poPrintSettings);
	void TracePrinterProperties(HDC phdcPrint);
	HDC  GetDCFromOutputDevice(sOutputDevice poOutputDevice);

	long lCurrentPrinterDeviceEnum;
	vector<sOutputDevice> oOutputDevices;

	int iPrintRequestNumber;
	};
