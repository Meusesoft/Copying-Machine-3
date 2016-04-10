#include "StdAfx.h"
#include "Dialogs.h"
#include "DlgMacroEditor.h"
#include "CopyingMachineCore.h"
#include <cctype>
#include <string>
#include <algorithm>

#define SCI_NAMESPACE

static const int MARGIN_SCRIPT_FOLD_INDEX = 1;

#include "Scintilla.h"
#include "Scilexer.h"

CDlgMacroEditor::CDlgMacroEditor(HINSTANCE phInstance, HWND phParent, sGlobalInstances poGlobalInstances) :
				  CDialogTemplate(phInstance, phParent, IDD_MACROEDITOR, poGlobalInstances) {

	hSciLexer = LoadLibrary(L"SciLexer.DLL");
	if (hSciLexer == NULL) {

		MessageBox(phParent,
		L"The Scintilla DLL could not be loaded.",
		L"Error loading Scintilla",
		MB_OK | MB_ICONERROR);
		}

	oScript = new CScriptCopyingMachine(poGlobalInstances);

	bAutoCompleteIsActive = false;
	}

CDlgMacroEditor::~CDlgMacroEditor(void)
{
	FreeLibrary(hSciLexer);
	delete oScript;
}

//This function is called when the dialog is initialised
void 
CDlgMacroEditor::OnInitDialog() {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oCore;

	//set the font of the caption
	SetDlgItemFont(IDC_ABOUTCAPTION, hCaptionFont);

	//Get the handle to the editor
	hEditor = GetDlgItem(hDlg, IDC_SCINTILLA);

	//Set the properties

	SendEditor(SCI_SETCODEPAGE, SC_CP_UTF8);
	
	SendEditor(SCI_SETLEXER, SCLEX_VBSCRIPT);
	SendEditor(SCI_SETSTYLEBITS, 7);
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"0");

	SendEditor(SCI_SETMARGINWIDTHN, MARGIN_SCRIPT_FOLD_INDEX, 0);
	SendEditor(SCI_SETMARGINTYPEN,  MARGIN_SCRIPT_FOLD_INDEX, SC_MARGIN_SYMBOL);
	SendEditor(SCI_SETMARGINMASKN, MARGIN_SCRIPT_FOLD_INDEX, SC_MASK_FOLDERS);
	SendEditor(SCI_SETMARGINWIDTHN, MARGIN_SCRIPT_FOLD_INDEX, 20);

	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);

	SendEditor(SCI_SETFOLDFLAGS, 16, 0); // 16  	Draw line below if not expanded

	SendEditor(SCI_SETMARGINSENSITIVEN, MARGIN_SCRIPT_FOLD_INDEX, 1);

	const COLORREF red = RGB(0xFF, 0, 0);
	const COLORREF offWhite = RGB(0xFF, 0xFB, 0xF0);
	const COLORREF darkGreen = RGB(0, 0x80, 0);
	const COLORREF darkBlue = RGB(0, 0, 0x80);
	const COLORREF darkGray = RGB(0x80, 0x80, 0x80);

	int pixelWidth = 30;//int((willBeShowed)?(8 + _maxNbDigit * execute(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"8")):0);
    SendEditor(SCI_SETMARGINWIDTHN, 0, pixelWidth);
	SendEditor(SCI_SETMARGINWIDTHN, 1, 20);
	SendEditor(SCI_SETCARETLINEVISIBLE, false);
	//SendEditor(SCI_SETCARETLINEBACK, RGB(0xD0, 0xD0, 0xD0));
	SendEditor(SCI_SETSELFORE, true, RGB(0xFF, 0xFF, 0xFF));
	SendEditor(SCI_SETSELBACK, true, RGB(0x33, 0x99, 0xFF));

	std::wstring wsKeyWords;
	std::string sKeyWords;

	wsKeyWords = GetKeywords(eScriptKeywordStatement);
	sKeyWords.assign(wsKeyWords.begin(), wsKeyWords.end());
	SendEditor(SCI_SETKEYWORDS, 0, 
		reinterpret_cast<LPARAM>(sKeyWords.c_str()));

	wsKeyWords = GetKeywords(eScriptKeywordAction);
	sKeyWords.assign(wsKeyWords.begin(), wsKeyWords.end());
	SendEditor(SCI_SETKEYWORDS, 1, 
		reinterpret_cast<LPARAM>(sKeyWords.c_str()));

	wsKeyWords = GetKeywords(eScriptKeywordAttribute);
	sKeyWords.assign(wsKeyWords.begin(), wsKeyWords.end());
	SendEditor(SCI_SETKEYWORDS, 2, 
		reinterpret_cast<LPARAM>(sKeyWords.c_str()));

	wsKeyWords = oScript->GetVariablesToKeywords();
	sKeyWords.assign(wsKeyWords.begin(), wsKeyWords.end());
	SendEditor(SCI_SETKEYWORDS, 3, 
		reinterpret_cast<LPARAM>(sKeyWords.c_str()));

	// Set up the global default style. These attributes are used wherever no explicit choices are made.
	//SetAStyle(STYLE_DEFAULT, black, white, 11, "Verdana");
	SendEditor(SCI_STYLECLEARALL);	// Copies global style to all others


	// Hypertext default is used for all the document's text
	SetAStyle(SCE_H_DEFAULT, black, white, 11, "Times New Roman");
	
	// Unknown tags and attributes are highlighed in red. 
	// If a tag is actually OK, it should be added in lower case to the htmlKeyWords string.
	SetAStyle(SCE_B_COMMENT, darkGreen);
	SetAStyle(SCE_B_DEFAULT, black);
	SetAStyle(SCE_B_KEYWORD, darkBlue);
	SetAStyle(SCE_B_KEYWORD2, darkGray);
	SetAStyle(SCE_B_KEYWORD3, darkGray);
	SetAStyle(SCE_B_KEYWORD4, red);
	SetAStyle(SCE_B_STRING, red);
	SetAStyle(SCE_B_NUMBER, red);

	//Settings for autocompletion
	SendEditor(SCI_AUTOCSETFILLUPS, 0, (LPARAM)" =.()");
	//SendEditor(SCI_ASSIGNCMDKEY, 65, SCI_TAB);

	//Workaround for an issue with set text during init dialog. 
	//It automatically select the complete text and this cannot
	//be changed with a SCI_SETSEL or something. Weird.
	PostMessage(hDlg, WM_COMMAND, IDC_EVENTS, 0);

	Localize();

	DoShowControls();

	//Set the new WndProc
	SetWindowLongPtr(hEditor, GWLP_USERDATA, reinterpret_cast<LONG>(this));
	g_CallWindowProc = (WNDPROC)GetWindowLongPtr(hEditor, GWLP_WNDPROC);
	g_ScintillaDefaultProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hEditor, GWLP_WNDPROC, reinterpret_cast<LONG>(ScintillaStatic_Proc)));
	}

//This function is called before the Scintilla WndProce is called. It checks whether a 
//ctrl-space is pressed. If so, it will make sure the wm_char won't reach Scintilla. This
//prevents the space to be added to the text and to prevent the autocompletion window to
//close immediately.
LRESULT 
CDlgMacroEditor::ScintillaNew_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) 
{
	SHORT nVirtKey;

	switch (Message)
	{
		case WM_CHAR :
			
			//check if it is a ctrl+space. If so, stop it.

			if (wParam==32)  {
				
				nVirtKey = GetKeyState(VK_CONTROL);
				
				if (nVirtKey & 0x8000) {

					DoAutoComplete();

					return 0;
					}
				}

			break;
	}
	return CallWindowProc(g_CallWindowProc, hwnd, Message, wParam, lParam);
}

//This function sets a Scintilla style color
void 
CDlgMacroEditor::SetAStyle(int style, COLORREF fore, COLORREF back, int size, const char *face) {
	SendEditor(SCI_STYLESETFORE, style, fore);
	SendEditor(SCI_STYLESETBACK, style, back);
	if (size >= 1)
		SendEditor(SCI_STYLESETSIZE, style, size);
	if (face) 
		SendEditor(SCI_STYLESETFONT, style, reinterpret_cast<LPARAM>(face));
}

//This function handles the OnMessag event
BOOL 
CDlgMacroEditor::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_ABOUTCAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}
             if ((HWND) lParam == GetDlgItem(hDlg, IDC_COPYINGMACHINEICON)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}

			break;


		case WM_NOTIFY:

			OnNotify(lParam);

			break; //WM NOTIFY

		};	


	return bReturn;
	}

//This function handles the scintilla on notify
void
CDlgMacroEditor::OnNotify(LPARAM lParam) {

	Scintilla::SCNotification* notification = (Scintilla::SCNotification*)lParam;

		const int modifiers = notification->modifiers;
		const int position = notification->position;
		const int margin = notification->margin;
		const int line_number = SendEditor(SCI_LINEFROMPOSITION, position, 0);

		switch (notification->nmhdr.code) {

		case SCN_MARGINCLICK:


			switch (margin) {
				case MARGIN_SCRIPT_FOLD_INDEX:
					{
					SendEditor(SCI_TOGGLEFOLD, line_number, 0);
					}
				break; //margin
				}

			break; //SCN_MARGINCLICK

		case SCN_CHARADDED:

			OnCharAdded(static_cast<char>(notification->ch));

			break; //SCN_CHARADDED
		}
	}


BOOL 
CDlgMacroEditor::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {

	CCopyingMachineCore* oCMCore;

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	switch (iCommand) {

		case IDC_EXECUTESCRIPT: 
			
			OnExecute();
			break;

		case IDC_SCRIPTEXPORT: 
			
			OnExport();
			break;

		case IDC_SCRIPTOPEN: 
			
			OnOpen();
			break;

		case IDC_AUTOCOMPLETE:

			DoAutoComplete();
			break;

		case IDC_EVENTS:

			std::string sScript;
			std::wstring wsScript;

			oGlobalInstances.oRegistry->ReadString(L"Editor", L"Text", L"'This is an empty script", wsScript); 
			sScript.assign(wsScript.begin(), wsScript.end());

			SendEditor(SCI_SETTEXT, 0, (LPARAM)sScript.c_str());
		break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgMacroEditor::OnCloseOk() {

	//Grab the macro and store it in memory
	long lTextLength;
	char* cText;

	lTextLength = SendEditor(SCI_GETLENGTH);

	cText = (char*)malloc(lTextLength+1);
	SendEditor(SCI_GETTEXT, lTextLength+1, (LPARAM)cText);

	std::string sScript;
	std::wstring wsScript;

	sScript = cText;

	wsScript.assign(sScript.begin(), sScript.end());

	oGlobalInstances.oRegistry->WriteString(L"Editor", L"Text", wsScript); 
	}

//This function handles the execution of the script
void 
CDlgMacroEditor::OnExecute() {



	//Grab the macro and process it
	long lTextLength;
	char* cText;

	lTextLength = SendEditor(SCI_GETLENGTH);

	cText = (char*)malloc(lTextLength+1);
	SendEditor(SCI_GETTEXT, lTextLength+1, (LPARAM)cText);

	CCopyingMachineCore* oCMCore;
	oCMCore = (CCopyingMachineCore*)oCore;

	if (oCMCore->oScriptProcessor->Process(cText)) {

		//Start the execution of the script
		oCMCore->oScriptProcessor->ExecuteNextScriptLine(); 
		}
	}

//This function exports the currently selected script to
//a text file
void 
CDlgMacroEditor::OnExport() {

	CCopyingMachineCore* oCore;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	
	wchar_t cFilename[MAX_PATH];
	int	    iDefaultFilter;
		std::wofstream ssFile;

	iDefaultFilter = 0;
	SecureZeroMemory(cFilename, MAX_PATH * sizeof(wchar_t));

	if (oCore->oDialogs->DlgSaveFile(hDlg, L"Scripts (*.script)\0*.script\0\0", 
									iDefaultFilter, 
									cFilename)) {

	//Get the script
		long lTextLength;
		char* cText;
		std::string sScript;
		std::wstring wsScript;
		std::wstring wsFilename;

		lTextLength = SendEditor(SCI_GETLENGTH);

		cText = (char*)malloc(lTextLength+1);
		SendEditor(SCI_GETTEXT, lTextLength+1, (LPARAM)cText);

		//Convert it to wstring
		sScript = cText;
		wsScript.assign(sScript.begin(), sScript.end());

		//Save the contents of the script to a file
		wsFilename = cFilename;
		if (wsFilename.rfind(L".script")==wsFilename.npos) {

			wsFilename += L".script";
			}


		ssFile.open(wsFilename.c_str());

		if (ssFile.is_open()) {

			ssFile << wsScript;

			ssFile.close();
			}
		else {
			
			//give error message
			MessageBox(hDlg, L"An error occurred while opening the file.", L"Error", MB_ICONERROR);
			}

		}	
	}

//This function opens a new script into the editor
void 
CDlgMacroEditor::OnOpen() {

	CCopyingMachineCore* oCore;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	
	wchar_t cFilename[MAX_PATH];
	int	    iDefaultFilter;
	std::wifstream ssFile;
	std::wstring wsScript;
	std::string sScript;
	std::wstring sLine;

	iDefaultFilter = 0;
	SecureZeroMemory(cFilename, MAX_PATH * sizeof(wchar_t));

	if (oCore->oDialogs->DlgOpenFile(hDlg, L"Scripts (*.script)\0*.script\0\0", 
									iDefaultFilter, 
									cFilename)) {

		//Open the file and read it contents
		ssFile.open(cFilename);

		if (ssFile.is_open()) {

			while (ssFile.good()) {
				
				std::getline(ssFile, sLine);
				wsScript += sLine;
				}

			ssFile.close();

			//Set the text in the editor
			sScript.assign(wsScript.begin(), wsScript.end());
			SendEditor(SCI_SETTEXT, 0, (LPARAM)sScript.c_str());

			}
		else {
			
			//give error message
			MessageBox(hDlg, L"An error occurred while opening the file.", L"Error", MB_ICONERROR);
			}
		}
	}

//This function handles the OK event
void
CDlgMacroEditor::Localize() {

	CCopyingMachineCore* oCMCore;
//	HWND				 hControl;
//	wchar_t				 cText[100];
//	wchar_t				 cTemplate[100];
	sExecutableInformation	 cVersionInformation;
	
	oCMCore = (CCopyingMachineCore*)oCore;

	LocalizeWindowCaption(IDS_SCRIPTEDITOR);

	LocalizeControl(IDC_TOPIC1,			IDS_SCRIPTS);
	LocalizeControl(IDC_EXECUTESCRIPT,	IDS_EXECUTE);
	LocalizeControl(IDC_SCRIPTOPEN,		IDS_OPEN);
	LocalizeControl(IDC_SCRIPTEXPORT,	IDS_SAVE);
	}

//This functions shows or hides controls
void
CDlgMacroEditor::DoShowControls() {

	CCopyingMachineCore* oCMCore;
	oCMCore = (CCopyingMachineCore*)oCore;

	}

void 
CDlgMacroEditor::OnCharAdded(char ch) {

	if (SendEditor(SCI_AUTOCACTIVE)) {

		DoAutoComplete();
		}
	else {
		
		if (ch==0x2e && bAutoCompleteIsActive) {

			PostMessage(hDlg, WM_COMMAND, IDC_AUTOCOMPLETE, 0);
			}
		bAutoCompleteIsActive = false;
		}
	}

//This function starts an autocomplete. It populates the autocomplete window and show it.
bool 
CDlgMacroEditor::DoAutoComplete() {

	std::string sAutocompleteWords;
	std::wstring wsAutocompleteWords;
	std::wstring wsLine;
	std::wstring wsWord;
	int iCurrentPosition;
	int iStartPositionWord;
	bool bInWord;
	
	wsLine = GetLine();
	iCurrentPosition = GetCaretInLine();
	iStartPositionWord = iCurrentPosition;
	bInWord = true;

	while (iStartPositionWord > 0  && bInWord) {
	
		iStartPositionWord--;

		wsWord = wsLine.substr(iStartPositionWord, 1);

		bInWord = (wsWord.find_first_not_of(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.")==wsWord.npos);
		
		if (!bInWord) iStartPositionWord++;
		}

	wsWord = wsLine.substr(iStartPositionWord, iCurrentPosition - iStartPositionWord);

	int iAutomcompleteWords = GetAutocompleteWords(wsWord, wsAutocompleteWords);
	sAutocompleteWords.assign(wsAutocompleteWords.begin(), wsAutocompleteWords.end());

	int iLastDotPosition = wsWord.find_last_of(L".");
	if (iLastDotPosition == wsWord.npos) {
		iLastDotPosition = 0;
		}
	else {
		iLastDotPosition++;
		}

	int iWordLength = iCurrentPosition - iStartPositionWord - iLastDotPosition;


	//Show the autocomplete dialog
	SendEditor(SCI_AUTOCSHOW, 
				iWordLength, 
				(LPARAM)sAutocompleteWords.c_str());

	bAutoCompleteIsActive = true;


	return true;
	}

//This function compiles a list of autocomplete words.
int 
CDlgMacroEditor::GetAutocompleteWords(std::wstring pwsRoot, std::wstring &pwsAutocompleteWords) {

	CCopyingMachineCore* oCore;
	std::wstring wsSwapValue;
	vector <std::wstring> wsWordsArray;
	bool bNotDone;
	int iIndex;
	int iFinalDotPosition;

	//root to lower case
	transform(
		pwsRoot.begin(), pwsRoot.end(),
		pwsRoot.begin(),
		tolower); 

	//fill the array with words
	oScript->AddVariablesToAutoComplete(pwsRoot, wsWordsArray);
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oCore->Autocomplete(pwsRoot, wsWordsArray);

	iFinalDotPosition = pwsRoot.find_last_of(L".");
	if (iFinalDotPosition==pwsRoot.npos) {

		int iKeywords = sizeof(aKeywordStrings) / sizeof(KeywordStringCombination);

		while (iKeywords > 0) {

			iKeywords--;

			if (aKeywordStrings[iKeywords].sString.find(pwsRoot)==0) {

				wsWordsArray.push_back(aKeywordStrings[iKeywords].sString);
				}
			}
		}
	else {

		pwsRoot = pwsRoot.substr(iFinalDotPosition+1, pwsRoot.length()-1);
		}

	//check the array if the words start with the root
	//iIndex = (int)wsWordsArray.size();

	//while (iIndex > 0) {
	//	
	//	iIndex--;

	//	if (wsWordsArray[iIndex].find(pwsRoot)!=0) wsWordsArray.erase(wsWordsArray.begin() + iIndex);
	//	}
	
	//sort the array
	bNotDone = true;
	
	while (bNotDone) {

		bNotDone = false;
		
		iIndex = (int)wsWordsArray.size();

		while (iIndex > 1) {
			
			iIndex--;

			if (wsWordsArray[iIndex-1].compare(wsWordsArray[iIndex])>0) {
	
				wsSwapValue = wsWordsArray[iIndex];
				wsWordsArray[iIndex] = wsWordsArray[iIndex-1];
				wsWordsArray[iIndex-1] = wsSwapValue;
				bNotDone = true;
				}
			}
		}

	//construct the string with all the autocompletion words as scintilla wants it
	iIndex = (int)wsWordsArray.size();

	while (iIndex > 0) {
		
		iIndex--;
		
		pwsAutocompleteWords = wsWordsArray[iIndex] + L" " + pwsAutocompleteWords;
		}
	
	pwsAutocompleteWords = pwsAutocompleteWords.substr(0, pwsAutocompleteWords.length()-1);
	
	return (int)wsWordsArray.size();
	}



//This function returns the position of the caret in the current line
int 
CDlgMacroEditor::GetCaretInLine() {

	int caret = SendEditor(SCI_GETCURRENTPOS);
	int line = SendEditor(SCI_LINEFROMPOSITION, caret);
	int lineStart = SendEditor(SCI_POSITIONFROMLINE, line);
	
	return caret - lineStart;
	}

//This function returns the current line as a std::wstring
std::wstring
CDlgMacroEditor::GetLine(int piLine) {
	
	int len;
	char* cTextLine;
	std::string sLine;
	std::wstring wsLine;
	
	// Get needed buffer size
	if (piLine < 0) {
		len = SendEditor(SCI_GETCURLINE, 0);
		} 
	else {
		len = SendEditor(SCI_GETLINE, piLine);
		}

	// Allocate buffer
	cTextLine = new char[len];

	// Get the line
	if (piLine < 0) {
		SendEditor(SCI_GETCURLINE, len, (LPARAM)cTextLine);
		} 
	else {
		SendEditor(SCI_GETLINE, piLine, (LPARAM)cTextLine);
		}

	//convert to std::wstring
	sLine = cTextLine;
	wsLine.assign(sLine.begin(), sLine.end());

	//free resources
	delete[] cTextLine;

	return wsLine;
	}

//This function constructs a list of script keywords, so Scintilla can give it a nice color
std::wstring 
CDlgMacroEditor::GetKeywords(eScriptKeywordType peType) {

	std::wstring wsResult;

	int iKeywords = sizeof(aKeywordStrings) / sizeof(KeywordStringCombination);

	while (iKeywords > 0) {

		iKeywords--;

		if (aKeywordStrings[iKeywords].eType == peType) {

			wsResult += aKeywordStrings[iKeywords].sString;
			wsResult += L" ";
			}
		}

	return wsResult;
	}


