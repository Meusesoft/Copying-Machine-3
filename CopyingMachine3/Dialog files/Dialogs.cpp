#include "StdAfx.h"
#include "Dialogs.h"
#include "DlgMacroEditor.h"
#include "DlgPrint.h"

#include <stdarg.h>

sTranslateMessage cTranslator[] = {
	{eFileResultInvalidParameter,	IDS_ERRORPROCESSINGFILE},
	{eFileResultNotFound,			IDS_ERRORFILENOTFOUND},
	{eFileResultNotSupported,		IDS_ERRORINVALIDFORMAT},
	{eFileResultUnknownError,		IDS_ERRORUNKNOWN},
	{eFileResultCouldNotCreate,		IDS_ERRORCOULDNOTCREATE},
	{eFileResultAccessDenied,		IDS_ERRORACCESSDENIED},
	{eGeneralOutOfMemory,			IDS_ERROROUTOFMEMORY},
	{ePrintError,					IDS_PRINTERROR},
	{ePrintErrorCheckMessages,		IDS_PRINTERRORMESSAGE}};

CDialogs::CDialogs(sGlobalInstances pcGlobalInstances)
{
	//set the global instances
	cGlobalInstances.hInstance = pcGlobalInstances.hInstance;
	cGlobalInstances.oTrace = pcGlobalInstances.oTrace;
	cGlobalInstances.oNotifications = pcGlobalInstances.oNotifications;
	cGlobalInstances.oRegistry = pcGlobalInstances.oRegistry;
	cGlobalInstances.oCopyingMachineCore = pcGlobalInstances.oCopyingMachineCore;
	cGlobalInstances.hLanguage = pcGlobalInstances.hLanguage;
}

CDialogs::~CDialogs(void)
{
}

//Show the progress dialog for acquireing images
CDlgProgressAcquire* 
CDialogs::DlgProgressAcquire(HWND phWnd, void* poScanSettings) {

	CDlgProgressAcquire* oDlgProgress;

	oDlgProgress = new CDlgProgressAcquire(phWnd, poScanSettings, cGlobalInstances);
	oDlgProgress->Execute();

	return oDlgProgress;
	}

//Show the progress dialog for print a document
CDlgProgressPrint* 
CDialogs::DlgProgressPrint(HWND phWnd, void* poDocument, void* poPrintSettings) {

	CDlgProgressPrint* oDlgProgress;

	oDlgProgress = new CDlgProgressPrint(phWnd, poDocument, poPrintSettings, cGlobalInstances);
	oDlgProgress->Execute();

	return oDlgProgress;
	}

//Show the About Dialog
void 
CDialogs::DlgAbout(HWND phWnd) {

	CDlgAbout* oDlgAbout;

	oDlgAbout = new CDlgAbout(cGlobalInstances.hInstance, phWnd, cGlobalInstances);
	
	oDlgAbout->Execute();

	delete oDlgAbout;

	}

//Show the Macro Editor Dialog
void 
CDialogs::DlgMacroEditor(HWND phWnd) {

	CDlgMacroEditor* oDlgMacroEditor;

	oDlgMacroEditor = new CDlgMacroEditor(cGlobalInstances.hInstance, phWnd, cGlobalInstances);
	
	oDlgMacroEditor->Execute();

	delete oDlgMacroEditor;

	}

//Show the Place key Dialog
void 
CDialogs::DlgPlaceKey(HWND phWnd) {

	CDlgPlaceKey* oDlgPlaceKey;

	oDlgPlaceKey = new CDlgPlaceKey(cGlobalInstances.hInstance, phWnd, cGlobalInstances);
	
	oDlgPlaceKey->Execute();

	delete oDlgPlaceKey;

	}

//Show the Reminder Dialog
void 
CDialogs::DlgReminder(HWND phWnd) {

	CDlgReminder* oDlgReminder;

	oDlgReminder = new CDlgReminder(cGlobalInstances.hInstance, phWnd, cGlobalInstances);
	
	oDlgReminder->Execute();

	delete oDlgReminder;

	}

//Show the Select Source Dialog
void 
CDialogs::DlgSelectSource(HWND phWnd) {

	CDlgPreferences* oDlgPreferences;

	oDlgPreferences = new CDlgPreferences(cGlobalInstances.hInstance, phWnd, eScannerDevice, cGlobalInstances);
	
	oDlgPreferences->Execute();

	delete oDlgPreferences;

	//CDlgSelectSource*	oDlgSelectSource;

	//oDlgSelectSource = new CDlgSelectSource(cGlobalInstances.hInstance, phWnd, IDD_SELECTSOURCE, cGlobalInstances);
	//
	//oDlgSelectSource->Execute();

	//delete oDlgSelectSource;
	}

//Show the Prefrences Dialog
void 
CDialogs::DlgPreferences(HWND phWnd, ePreferences pcPreferences) {

	CDlgPreferences* oDlgPreferences;

	oDlgPreferences = new CDlgPreferences(cGlobalInstances.hInstance, phWnd, pcPreferences, cGlobalInstances);
	
	oDlgPreferences->Execute();

	delete oDlgPreferences;
	}

//Show the recognize text dialog
void 
CDialogs::DlgRecognizeText(HWND pHwnd, DWORD pdPageId) {

	CDlgRecognizeText* oDlgRecognizeText;

	oDlgRecognizeText = new CDlgRecognizeText(cGlobalInstances.hInstance, pHwnd, pdPageId, cGlobalInstances);
	
	oDlgRecognizeText->Execute();

	delete oDlgRecognizeText;
	}

//Show the print dialog
bool 
CDialogs::DlgPrint(HWND pHwnd, void* poPrintSettings) {

	CDlgPrint* oDlgPrint;
	bool bResult;

	oDlgPrint = new CDlgPrint();

	bResult = oDlgPrint->ExecutePrintDialog(cGlobalInstances, pHwnd, poPrintSettings);

	delete oDlgPrint;

	return bResult;
	}

//Open file dialog
bool 
CDialogs::DlgOpenFile(HWND pHwnd, wchar_t* pcFileFilters, 
					  int &piDefaultFilter, wchar_t* pcFilename) {

	CDlgOpenSave* oDlgOpenSave;
	bool bResult;

	oDlgOpenSave = new CDlgOpenSave();

	bResult = oDlgOpenSave->ExecuteOpenDialog(cGlobalInstances.hLanguage, pHwnd, pcFileFilters, piDefaultFilter, pcFilename);

	delete oDlgOpenSave;

	return bResult;
	}

//Open save dialog
bool 
CDialogs::DlgSaveFile(HWND pHwnd, wchar_t* pcFileFilters, 
					  int &piDefaultFilter, wchar_t* pcFilename) {


	CDlgOpenSave* oDlgOpenSave;
	bool bResult;

	oDlgOpenSave = new CDlgOpenSave();

	bResult = oDlgOpenSave->ExecuteSaveDialog(cGlobalInstances.hLanguage, pHwnd, pcFileFilters, piDefaultFilter, pcFilename);

	delete oDlgOpenSave;

	return bResult;
	}

//Open color dialog
bool 
CDialogs::DlgColor(HWND pHwnd) {

	CDlgColor* oDlgColor;
	bool bResult;

	oDlgColor = new CDlgColor();

	bResult = oDlgColor->ExecuteColorDialog(cGlobalInstances, pHwnd);

	delete oDlgColor;

	return bResult;
	}

//Open color dialog
int 
CDialogs::DlgSaveChanges(HWND pHwnd, std::wstring psMessage) {

	CDlgSaveChanges* oDlgSaveChanges;
	int iResult;

	oDlgSaveChanges = new CDlgSaveChanges(cGlobalInstances.hInstance, pHwnd, psMessage, cGlobalInstances);

	iResult = oDlgSaveChanges->Execute();

	delete oDlgSaveChanges;

	return iResult;
	}


//Open the Unknown Event StillImage Dialog
int 
CDialogs::DlgUnknownEvent(HWND pHwnd, std::wstring psEvent) {

	CDlgUnknownEvent* oDlgUnknownEvent;
	int iResult;

	oDlgUnknownEvent = new CDlgUnknownEvent(cGlobalInstances, pHwnd, psEvent);

	oDlgUnknownEvent->Execute();
	iResult = oDlgUnknownEvent->iDialogResult;

	delete oDlgUnknownEvent;

	return iResult;	
	}


int	 
CDialogs::DlgError(HWND pHwnd, eActionResult peResult, int piParameters, ...) {

	wchar_t cCaption[101];
	wchar_t* cText;
	wchar_t* cParameter1;
	int cParameter2;
	int iTextSize;
	int iResult;
	wchar_t* cTemplateText;

	iResult = IDOK;

	if (peResult!=eCancelled && peResult!=eOk) {

		cText = (wchar_t*)malloc(sizeof(wchar_t) * 256);

		LoadStringW(cGlobalInstances.hLanguage, IDS_CAPTION, cCaption, 100);
		LoadStringW(cGlobalInstances.hLanguage, ConvertResultString(peResult), cText, 256);

		//do a sprintf
		if (piParameters>0) {

			va_list marker;
			va_start(marker, piParameters);
			
			cTemplateText = cText;

			cParameter1 = va_arg(marker, wchar_t*);
			if (piParameters==2) cParameter2 = va_arg(marker, int);

			iTextSize = (int)wcslen(cParameter1) + (int)wcslen(cTemplateText) + 20;
			if (piParameters==2) iTextSize+= 20;

			cText = (wchar_t*)malloc(sizeof(wchar_t) * iTextSize);

			if (piParameters==1) swprintf_s(cText, iTextSize, cTemplateText, cParameter1);
			if (piParameters==2) swprintf_s(cText, iTextSize, cTemplateText, cParameter1, cParameter2);

			free (cTemplateText);

			va_end( marker );              
			}

		//show the message box
		iResult = MessageBox(pHwnd, cText, cCaption, MB_OK | MB_ICONERROR);

		//clean up and return
		free(cText);
		}

	return iResult;
	}

int 
CDialogs::ConvertResultString(eActionResult peResult) {

	int iResource;
	int iSize;

	iSize = sizeof(cTranslator)/sizeof(sTranslateMessage);
	iResource = IDS_ERRORUNKNOWN;

	for (int iIndex=0; iIndex<iSize && iResource == IDS_ERRORUNKNOWN; iIndex++) {

		if (cTranslator[iIndex].eResult == peResult) {

			iResource = cTranslator[iIndex].iResource;
			}
		}

	return iResource;
}
