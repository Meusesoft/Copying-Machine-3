#pragma once
#include "..\OCR files\TOcr.h"

DWORD WINAPI ThreadOCRPage(LPVOID lpParameter);

struct sOCRPage {

	int iRequest;					//The number of the OCR request
	CCopyDocumentPage*	oPage;		//The page to OCR
	eActionResult cResult;			//The result of the process
	std::wstring sOutput;			//The result of the OCR as wide string
	LANGID cLanguage;				//The language used in the OCR process. An unknown LANGID will be translated to LANG_ENGLISH	

	#ifdef DEBUG
		cOcrSettings oOcrSettings;	
	#endif
};

class COCRCore :
	public CInterfaceBase
{
public:
	COCRCore(sGlobalInstances pInstances);
	~COCRCore(void);

	eActionResult OCRPage(sOCRPage* poOCRPage);
	int OCRPageThread(CCopyDocumentPage* poPage, cOcrSettings poOcrSettings, LANGID pcLanguage = LANG_ENGLISH); 

	//Thread 
	bool GetEndThread();
	void SetEndThread();
	
	CRITICAL_SECTION ThreadCriticalSection; 
	HANDLE hThread;
	HWND   hMainWindow;
	vector <sOCRPage*> oOCRQueue;

private:

	eActionResult OCRPageTOcr(sOCRPage* poOCRPage);
	eActionResult OCRPageTesseract(sOCRPage* poOCRPage);

	CRITICAL_SECTION EndThreadCriticalSection; 
	bool bEndThread;

	bool StartOCRThread();

	int iRequestNumber;

	TOcr* oOcr;
	};
