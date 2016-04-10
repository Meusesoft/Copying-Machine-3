#pragma once
#include "DialogTemplate.h"
#include "DlgProgressAcquire.h"
#include "DlgProgressPrint.h"
#include "DlgSelectSource.h"
#include "DlgOpenSave.h"
#include "DlgAbout.h"
#include "DlgPlaceKey.h"
#include "DlgReminder.h"
#include "DlgPreferencesGeneral.h"
#include "DlgPreferencesScannerImage.h"
#include "DlgPreferencesScannerPage.h"
#include "DlgPreferencesScannerDevice.h"
#include "DlgPreferencesScannerAdvanced.h"
#include "DlgPreferencesTrace.h"
#include "DlgPreferencesPrinterImageSize.h"
#include "DlgPreferencesPrinterCopiesAlignment.h"
#include "DlgPreferencesPrinterDevice.h"
#include "DlgPreferencesAutosave.h"
#include "DlgPreferences.h"
#include "DlgColor.h"
#include "DlgUnknownEvent.h"
#include "DlgRecognizeText.h"
#include "DlgAutoSaveDescription.h"
#include "DlgSaveChanges.h"

struct sTranslateMessage {

	eActionResult eResult;
	int			  iResource;
	};

class CDialogs
{
public:
	CDialogs(sGlobalInstances cGlobalInstances);
	~CDialogs(void);

	void DlgAbout(HWND pHwnd);
	void DlgMacroEditor(HWND pHwnd);
	void DlgPlaceKey(HWND pHwnd);
	void DlgReminder(HWND pHwnd);
	void DlgSelectSource(HWND pHwnd);
	void DlgPreferences(HWND pHwnd, ePreferences pcPreferences);
	bool DlgOpenFile(HWND pHwnd, wchar_t* pcFileFilters, int &piDefaultFilter, wchar_t* pcFilename);
	bool DlgSaveFile(HWND pHwnd, wchar_t* pcFileFilters, int &piDefaultFilter, wchar_t* pcFilename);
	int DlgUnknownEvent(HWND pHwnd, std::wstring psEvent);
	bool DlgColor(HWND pHwnd);
	bool DlgPrint(HWND pHwnd, void* poPrintSettings);
	void DlgRecognizeText(HWND pHwnd, DWORD pdPageId);
	int	 DlgError(HWND pHwnd, eActionResult peResult, int piParameters, ...);
	int	 DlgSaveChanges(HWND pHwnd, std::wstring psMessage);
	CDlgProgressAcquire* DlgProgressAcquire(HWND pHwnd, void* poScanSettings);
	CDlgProgressPrint*	 DlgProgressPrint(HWND pHwnd, void* poDocument, void* poPrintSettings);


private:

	int ConvertResultString(eActionResult peResult);

	sGlobalInstances cGlobalInstances;
};

