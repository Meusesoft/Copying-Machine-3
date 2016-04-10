#pragma once

#include "MathVector.h"
#include "ScriptProcessor.h"
#include "DocumentCollections.h"
#include "Bitmap.h"
#include "CoreNotifications.h"
#include "Registration.h"
#include "PrintSettings.h"
#include "ScanDevice.h"
#include "ScanSettings.h"
#include "RecognizedText.h"
#include "CopyDocumentPage.h"
#include "CopyDocument.h"
#include "CopyDocuments.h"
#include "CopyPageUndoRedo.h"
#include "InterfaceBase.h"
#include "PrintCore.h"
#include "OCRCore.h"
#include "ScanInterfaceBase.h"
#include "ScanTwain.h"
#include "ScanWia.h"
#include "ScanLoadFile.h"
#include "ScanCore.h"
#include "Dialogs.h"

struct sExecutableInformation {

	DWORD			dVersion;
	std::wstring	sLegalCopyright;
	std::wstring	sHashValue;
	};

class CCopyingMachineCore : 
	public CScriptObject
{
public:
	CCopyingMachineCore(sGlobalInstances pInstances);
	~CCopyingMachineCore(void);

	CCoreNotifications*		oNotifications;
	CCopyDocuments*			oDocuments;
	CCopyPageUndoRedo*		oUndoRedo;
	CPrintSettings*			oPrintSettings;
	CPrintCore*				oPrintCore;
	CScanSettings*			oScanSettings;
	CScanCore*				oScanCore;
	COCRCore*				oOCRCore;
	CDialogs*				oDialogs;
	CRegistration*			oRegistration;
	CScriptProcessor*		oScriptProcessor;
	CDocumentCollections*	oDocumentCollections;

	bool ProcessMessage(MSG &message);

	bool Acquire(HWND phWnd, eAcquireType pcAcquireType = eAcquireNewDocument, CScanSettings* poScanSettings = NULL);
	bool TransferImages(HWND phWnd);

	bool LoadDocument(HWND phWnd);
	bool LoadDocument(HWND phWnd, std::wstring psFilename);
	bool SaveDocument(HWND phWnd, CCopyDocument* poDocument);

	bool CloseDocument(HWND phWnd, CCopyDocument* poDocument);
	bool SaveChangesToDirtyDocuments(HWND phWnd);

	bool PasteImage(HWND phWnd, int piCommand);
	bool PasteImageCheck();

	CCopyDocument* GetCurrentDocument();
	CCopyDocumentPage* GetCurrentPage();
	bool SetCurrentPage(long plIndex);
	bool SetCurrentPageByID(DWORD pdPageId);
	bool SetCurrentDocument(long plIndex);
	bool SetCurrentDocumentByID(long plIndex);
	void MemorizeCurrentDocument();
	void RestoreMemorizedDocument();

	eActionResult DoPageImageAction(ePageImageAction pcAction, DWORD dActionData, HWND phWnd=NULL);
	bool CheckPageImageAction(ePageImageAction pcAction, DWORD dActionData);

	void CommandlineInput(std::wstring psOptions);
	void CommandlineOptionAdd(std::wstring psOption);
	void CommandlineOptionsProcess(HWND phWnd, bool pbEasyFrame);

	int  CreateDocumentID();

	//Language functions
	HINSTANCE	LoadLanguage();
	int			GetLanguageCount();
	LANGID		GetLanguage(int piIndex);
	LANGID		GetPreferredLanguage();
	void		SetPreferredLanguage(LANGID plLanguage);
	
	//This is the main window
	HWND		hMainApplicationWindow;

	//Help functions
	void OpenHelp(HWND phWnd);

	//Script function
	virtual bool Execute(std::wstring psObjectname, 
						 sScriptExpression &psValue);
	virtual bool Set(std::wstring psObjectname, sScriptExpression psNewValue);
	virtual sScriptExpression Get(std::wstring psObjectname);

	//Version functions
	void GetCopyingMachineVersion(sExecutableInformation &pcVersionInformation);

	sGlobalInstances oGlobalInstances;

private:

	void DoGetCopyingMachineVersion(sExecutableInformation &pcVersionInformation);
	void DoHashExecutable();

	vector <std::wstring> oCommandlineOptions;

	CTracer* oTrace;
	CRegistry* oRegistry;

	long lCurrentDocument;
	long lCurrentDocumentMemorized;
	long lCurrentPage;
	int iDocumentIDCounter;

	vector <LANGID> cLanguages;
	sExecutableInformation cExecutableInformation;
	;
};
