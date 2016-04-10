#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"
#include "..\OCR files\TOcr.h"
#include "..\OCR files\DlgTOcrDebug.h"


CDlgRecognizeText::CDlgRecognizeText(HINSTANCE phInstance, HWND phParent, DWORD pdPageId, sGlobalInstances poGlobalInstances) :
CDialogTemplate(phInstance, phParent, 
					#ifdef DEBUG 
					IDD_OCRDEBUG
					#endif 
					#ifndef DEBUG
					IDD_OCR
					#endif
					, poGlobalInstances) {

	dPageId = pdPageId;
	iOCRTicket = -1;
	hImage = NULL;
	}

CDlgRecognizeText::~CDlgRecognizeText(void)
{
	CCopyingMachineCore* oCore;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	//Unsubscribe to notifications
	oCore->oNotifications->UnsubscribeAll(hDlg);

	//free the allocated memory for the shown image
	if (hImage != NULL) {

		GlobalFree(hImage);
		}
}

//This function is called when the dialog is initialised
void 
CDlgRecognizeText::OnInitDialog() {

	CCopyingMachineCore* oCore;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	//Subscribe to notifications
	oCore->oNotifications->Subscribe(hDlg, eNotificationPageOCRDone);

	//Localize the controls of the dialog
	Localize();

	//set the font of the caption
	SetDlgItemFont(IDC_OCRCAPTION, hCaptionFont);

	//Enable/disable the controls
	EnableControls();

	//Initialise/fill the controls
	DoFillLanguageControl();

	}

BOOL 
CDlgRecognizeText::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_OCRCAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}
             if ((HWND) lParam == GetDlgItem(hDlg, IDC_POWERBYLABEL)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}
			break;

		case WM_DRAWITEM:

			DrawPictureControl(lParam);
			break;
		
		case WM_CORENOTIFICATION:

			OnEventCoreNotification();

			bReturn=true;
			break;
		};	


	return bReturn;
	}

BOOL 
CDlgRecognizeText::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	switch (iCommand) {

		case IDC_START:
		case IDC_DOOCR:

			DoOcr(dPageId);			
			break;

		case IDC_TOCLIPBOARD:

			DoToClipboard();
			break;

		//case IDC_CANCELOCR:

		//	DoCancelOcr();
		//	break;
		}

	return FALSE;
	}

//This function handles the core event notifications
bool 
CDlgRecognizeText::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;
	CCopyingMachineCore* oCore;

	bReturn = false;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	while (oCore->oNotifications->GetNotification(hDlg, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationPageOCRDone:
				
				sOCRPage* oOCRPage;

				oOCRPage = (sOCRPage*)cNotification.pData;

				if (iOCRTicket == oOCRPage->iRequest) {

					//Process the results
					iOCRTicket = -1;

					HWND hEditControl;

					hEditControl = GetDlgItem(hDlg, IDC_OCRTEXT);
					SendMessage(hEditControl, WM_SETTEXT, NULL, (LPARAM)oOCRPage->sOutput.c_str());


				#ifdef DEBUG
					//add the trace information
					hEditControl = GetDlgItem(hDlg, IDC_OCRTRACE);
					SendMessage(hEditControl, WM_SETTEXT, NULL, (LPARAM)oOCRPage->oOcrSettings.sTrace.c_str());

					//free the allocated memory for the shown image
					if (hImage != NULL) {

						GlobalFree(hImage);
						}

					hImage =oOCRPage->oOcrSettings.hIntermediateImage;
					
					//repaint the image control
					InvalidateRect(GetDlgItem(hDlg, IDC_PICTURE), NULL, true);
				#endif

					delete oOCRPage;
					
					EnableControls();
					}
				break;
			}
		}

	return bReturn;
	}

//This function handles the OK event
void
CDlgRecognizeText::OnCloseOk() {

	#ifdef DEBUG
	oGlobalInstances.oRegistry->WriteInt(L"TOcr", L"SaveXML",	SendDlgItemMessage(hDlg, IDC_CHECK_SAVEXML,		BM_GETCHECK, 0, 0)==BST_CHECKED ? 1 : 0);
	oGlobalInstances.oRegistry->WriteInt(L"TOcr", L"SaveImage",	SendDlgItemMessage(hDlg, IDC_CHECK_SAVEIMAGE,	BM_GETCHECK, 0, 0)==BST_CHECKED ? 1 : 0);
	oGlobalInstances.oRegistry->WriteInt(L"TOcr", L"SaveSplit",	SendDlgItemMessage(hDlg, IDC_CHECK_SAVESPLIT,	BM_GETCHECK, 0, 0)==BST_CHECKED ? 1 : 0);
	oGlobalInstances.oRegistry->WriteInt(L"TOcr", L"SaveResults", SendDlgItemMessage(hDlg, IDC_CHECK_SAVERESULTS, BM_GETCHECK, 0, 0)==BST_CHECKED ? 1 : 0);
	oGlobalInstances.oRegistry->WriteInt(L"TOcr", L"OcrType",	SendDlgItemMessage(hDlg, IDC_COMBOOCRTYPE, CB_GETCURSEL, 0, 0));
	#endif
}

//This function handles the OK event
void
CDlgRecognizeText::Localize() {

	LocalizeWindowCaption(IDS_RECOGNIZETEXT);

	LocalizeControl(IDC_TOPIC1,			IDS_SETTINGS);
	LocalizeControl(IDC_TOPIC2,			IDS_RESULT);
	LocalizeControl(IDC_TOPIC3,			IDS_LANGUAGE);
	//LocalizeControl(IDC_CANCELOCR,		IDS_CANCEL);
	LocalizeControl(IDC_DOOCR,			IDS_START);
	LocalizeControl(IDC_TOCLIPBOARD,	IDS_COPYTOCLIPBOARD);
	LocalizeControl(IDOK,				IDS_OK);
	LocalizeControl(IDC_OCRCAPTION,		IDS_RECOGNIZETEXT);
	LocalizeControl(IDC_POWERBYLABEL,	IDS_OCRPOWEREDBY);
	}

//This function starts the OCR
void
CDlgRecognizeText::DoOcr(DWORD pdPageId) {

	CCopyingMachineCore* oCmCore;
	CCopyDocumentPage* oPage;
	cOcrSettings oOcrSettings;

	int iLanguage;

	if (iOCRTicket==-1) {

		oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

		oCmCore->oOCRCore->hMainWindow = hDlg;

		oPage = oCmCore->oDocuments->GetPageByID(pdPageId);
		iLanguage = GetSelectedLanguage();
		
		#ifdef DEBUG
		oOcrSettings.bSaveXML = (SendDlgItemMessage(hDlg, IDC_CHECK_SAVEXML,		BM_GETCHECK, 0, 0)==BST_CHECKED);
		oOcrSettings.bSaveSplits = (SendDlgItemMessage(hDlg, IDC_CHECK_SAVESPLIT,		BM_GETCHECK, 0, 0)==BST_CHECKED);
		oOcrSettings.bSaveResults = (SendDlgItemMessage(hDlg, IDC_CHECK_SAVERESULTS,		BM_GETCHECK, 0, 0)==BST_CHECKED);
		oOcrSettings.bSaveImage = (SendDlgItemMessage(hDlg, IDC_CHECK_SAVEIMAGE,		BM_GETCHECK, 0, 0)==BST_CHECKED);
		oOcrSettings.iCompareMethod = SendDlgItemMessage(hDlg, IDC_COMBOOCRTYPE, CB_GETCURSEL, 0, 0);
		#endif

		iOCRTicket = oCmCore->oOCRCore->OCRPageThread(oPage, oOcrSettings, iLanguage);
		}

	EnableControls();
	}

//This function 'tries' to cancel the ocr operation
void 
CDlgRecognizeText::DoCancelOcr() {

	CCopyingMachineCore* oCmCore;

	if (iOCRTicket!=-1) {

		oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

		//Not possible to stop the thread of the Tesseract DLL. It is not built for multiple threads
		//running at the same time. Maybe a later version, or every thread needs to load the dll
		//itself. For now this function is disabled.
		}

	EnableControls();
	}

//This function copies the contents of the result field to the clipboard
void 
CDlgRecognizeText::DoToClipboard() {

	oTrace->StartTrace(__WFUNCTION__);

	long lTextLength;
	HWND hEditControl;
	HGLOBAL hMem;
	void*   pMem;

	//Determine the length of the text in the control
	hEditControl = GetDlgItem(hDlg, IDC_OCRTEXT);
	lTextLength = SendMessage(hEditControl, WM_GETTEXTLENGTH, NULL, NULL);

	//Allocate memory and get the text from the control
	hMem = GlobalAlloc(GMEM_MOVEABLE, (lTextLength + 1) * sizeof(TCHAR));
	pMem = GlobalLock(hMem);

	SendMessage(hEditControl, WM_GETTEXT, (WPARAM)lTextLength+1, (LPARAM)pMem);

	GlobalUnlock(hMem);

	//Copy the text to the clipboard
	if (OpenClipboard(hDlg)) {

		SetClipboardData(CF_UNICODETEXT, hMem);

		CloseClipboard();
		}
	else {

		//Free the memory to prevent a memory leak
		GlobalFree(hMem);
		}

	oTrace->EndTrace(__WFUNCTION__);
	}


//This function enables and disables the dialog its controls according to the state of the OCR
void 
CDlgRecognizeText::EnableControls() {

	//EnableControl(IDC_CANCELOCR,	iOCRTicket!=-1);
	EnableControl(IDC_DOOCR,		iOCRTicket==-1);
	EnableControl(IDOK,				iOCRTicket==-1);

	long lTextLength;
	HWND hEditControl;

	hEditControl = GetDlgItem(hDlg, IDC_OCRTEXT);
	lTextLength = SendMessage(hEditControl, WM_GETTEXTLENGTH, NULL, NULL);

	EnableControl(IDC_OCRTEXT,		lTextLength>0);
	EnableControl(IDC_TOCLIPBOARD,	lTextLength>0);

	#ifdef DEBUG
	EnableControl(IDC_DOOCR,				iOCRTicket==-1);
	EnableControl(IDC_COMBOOCRTYPE,			iOCRTicket==-1);
	EnableControl(IDC_CHECK_SAVEIMAGE,		iOCRTicket==-1);
	EnableControl(IDC_CHECK_SAVESPLIT,		iOCRTicket==-1);
	EnableControl(IDC_CHECK_SAVEXML,		iOCRTicket==-1);
	EnableControl(IDC_CHECK_SAVERESULTS,	iOCRTicket==-1);
	#endif
	}

//This function fills the language combobox
void 
CDlgRecognizeText::DoFillLanguageControl() {

	CCopyingMachineCore* oCmCore;
	wchar_t		cText[100];
	int			iItemIndex;
	LANGID		lLanguageId;
	int			iInsertIndex;
	int			iSelectedItem;
	int			iCurrentSetting;
	vector <std::wstring> sLanguages;
	CRegistry*  oRegistry;

	oCmCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oRegistry = oGlobalInstances.oRegistry;

	//Clear the combo box
	SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_RESETCONTENT, 0, 0);

	iCurrentSetting = (LANGID)oRegistry->ReadInt(L"General", L"LanguageOCR", 0);
	iSelectedItem = 0;

	//Fill the combobox with the initial choice, local regional setting
	LoadString(oGlobalInstances.hLanguage, IDS_LOCALLANGUAGE, cText, 100);
	iItemIndex=(int)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_ADDSTRING, 0, (LPARAM)cText);
	if (iItemIndex!=CB_ERR) {
		SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)0);
		}

	//Fill the combobox, loop through the available languages
	for (long lIndex=0; lIndex<oCmCore->GetLanguageCount(); lIndex++) {

		lLanguageId = oCmCore->GetLanguage(lIndex);

		VerLanguageName(lLanguageId, cText, 100);
		
		iInsertIndex = -1;

		for (long lIndex=0; lIndex<(long)sLanguages.size() && iInsertIndex==-1; lIndex++) {
			
			if (wcscmp(cText, sLanguages[lIndex].c_str())<0) iInsertIndex=lIndex+1;
			}

		iItemIndex=(int)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_INSERTSTRING, (WPARAM)iInsertIndex, (LPARAM)cText);
		sLanguages.insert(sLanguages.begin(), cText);
		if (iItemIndex!=CB_ERR) {
			SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)lLanguageId);
			}
		}

	//determine which item to select
	long lIndex = (long)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_GETCOUNT, 0, 0);

	if (lIndex!=CB_ERR) {

		while (lIndex>0) {

			lIndex--;

			if (SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_GETITEMDATA, lIndex, 0)==iCurrentSetting) {

				iSelectedItem = lIndex;
				}			
			}
		}

	//Select an entry
	SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_SETCURSEL, (WPARAM)iSelectedItem, 0);

	#ifdef DEBUG

		//fill the ocr type combobox
		std::wstring sValue;
		
		sValue= L"All";
		SendDlgItemMessage(hDlg, IDC_COMBOOCRTYPE, CB_ADDSTRING, NULL, (LPARAM)sValue.c_str());
		sValue= L"Group";
		SendDlgItemMessage(hDlg, IDC_COMBOOCRTYPE, CB_ADDSTRING, NULL, (LPARAM)sValue.c_str());
		sValue= L"Tree";
		SendDlgItemMessage(hDlg, IDC_COMBOOCRTYPE, CB_ADDSTRING, NULL, (LPARAM)sValue.c_str());
		SendDlgItemMessage(hDlg, IDC_COMBOOCRTYPE, CB_SETCURSEL, (WPARAM)oGlobalInstances.oRegistry->ReadInt(L"TOcr", L"OcrType", 0), NULL);
	
		//fill the checkboxes
		SendDlgItemMessage(hDlg, IDC_CHECK_SAVEIMAGE, BM_SETCHECK, oGlobalInstances.oRegistry->ReadInt(L"TOcr", L"SaveImage", 0) == 0 ? BST_UNCHECKED : BST_CHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SAVERESULTS, BM_SETCHECK, oGlobalInstances.oRegistry->ReadInt(L"TOcr", L"SaveResults", 0) == 0 ? BST_UNCHECKED : BST_CHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SAVESPLIT, BM_SETCHECK, oGlobalInstances.oRegistry->ReadInt(L"TOcr", L"SaveSplit", 0) == 0 ? BST_UNCHECKED : BST_CHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SAVEXML, BM_SETCHECK, oGlobalInstances.oRegistry->ReadInt(L"TOcr", L"SaveXML", 0) == 0 ? BST_UNCHECKED : BST_CHECKED, 0);

	#endif
}

//This function reads out the selected language in the
//language combobox
int 
CDlgRecognizeText::GetSelectedLanguage() {

	//Set the language settings
	int iLanguage;
	int iItem;
	CRegistry* oRegistry;
		
	oRegistry = oGlobalInstances.oRegistry;
	iLanguage = 0;
	iItem = (int)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_GETCURSEL, 0, 0);
	
	if (iItem!=CB_ERR) {

		iLanguage = (int)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_GETITEMDATA, iItem, 0);
		
		if (iLanguage!=CB_ERR) {
			
			oRegistry->WriteInt(L"General", L"LanguageOCR", iLanguage);
			}
		}

	return iLanguage;
	}

//Draw the contents of the picture control
void 
CDlgRecognizeText::DrawPictureControl(LPARAM plParam) {

	LPDRAWITEMSTRUCT oDrawItem;

	oDrawItem = (LPDRAWITEMSTRUCT)plParam;

	if (hImage!=NULL) {

		Graphics* oCanvas;
		CBitmap*   oBitmap;

		oCanvas = new Graphics(oDrawItem->hDC);

	   // Create a SolidBrush object.
	   SolidBrush blackBrush(Color(255, 0, 0, 0));

	   // Create a RectF object.
	   RectF fillRect(1.0f, 2.5f, 100.3f, 100.9f);

	   // Fill the rectangle.
	    oCanvas->FillRectangle(&blackBrush, fillRect);
		oBitmap = CBitmap::GetFromHandle(hImage);



		
		oCanvas->DrawImage(oBitmap->GetBitmap(), (REAL)0, (REAL)0, (REAL)oBitmap->GetBitmap()->GetWidth(), (REAL)oBitmap->GetBitmap()->GetHeight());

	CBitmap* oTestBitmap;
	oTestBitmap = CBitmap::GetFromHandle(hImage);

	CLSID pngClsid;
	CCopyDocument::GetEncoderClsid(L"image/bmp", &pngClsid);
	Status iResult;

	EncoderParameters* pEncoderParameters = (EncoderParameters*)
	malloc(sizeof(EncoderParameters) + 2 * sizeof(EncoderParameter));

	// An EncoderParameters object has an array of
	// EncoderParameter objects. In this case, there is only
	// one EncoderParameter object in the array.
	pEncoderParameters->Count = 0;

	iResult = oTestBitmap->GetBitmap()->Save(L"c:\\TOcrImages\\test.bmp", &pngClsid, pEncoderParameters);

	if (iResult!=Ok) {
	iResult = Ok;
	}

	free(pEncoderParameters);

	//delete oBitmap;
		delete oCanvas;
		}
	}







