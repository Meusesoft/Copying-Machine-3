#pragma once
#include "ScriptProcessorKeywords.h"
#include "ScriptExpression.h"
#include "ScriptObject.h"
#include "Script.h"
#include "ScriptCopyingMachine.h"

const COLORREF black = RGB(0,0,0);
const COLORREF white = RGB(0xff,0xff,0xff);

static HACCEL	g_hAccelerator;
static WNDPROC	g_ScintillaDefaultProc;

class CDlgMacroEditor :
	public CDialogTemplate
{
public:
	CDlgMacroEditor(HINSTANCE hInstance, HWND phParent, sGlobalInstances poGlobalInstances);
	~CDlgMacroEditor(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();

	HWND    hEditor;   //Handle to the editor window

	LRESULT SendEditor(UINT Msg, WPARAM wParam=0, LPARAM lParam=0) {
		return ::SendMessage(hEditor, Msg, wParam, lParam);
		}

	//These functions are used to prevent Scintilla to place a space when autocompletion
	//is activited by ctrl-space.
	static LRESULT CALLBACK ScintillaStatic_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
		
		CDlgMacroEditor *oScriptEditor = (CDlgMacroEditor *)(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (oScriptEditor)
			return (oScriptEditor->ScintillaNew_Proc(hwnd, Message, wParam, lParam));
		else
			return ::DefWindowProc(hwnd, Message, wParam, lParam);
	};

	LRESULT ScintillaNew_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	WNDPROC  g_CallWindowProc;

private:

	void DoShowControls();

	void OnExecute();
	void OnExport();
	void OnOpen();
	void OnNotify(LPARAM lParam);
	void OnCharAdded(char ch);

	void SetAStyle(int style, COLORREF fore, COLORREF back=white, int size=-1, const char *face=0);
	bool DoAutoComplete();
	
	int GetCaretInLine();
	std::wstring GetLine(int piLine = -1);
	int GetAutocompleteWords(std::wstring pwsRoot, std::wstring &pwsAutocompleteWords);

	HMODULE hSciLexer; //Handle to the SCI Lexer DLL
	bool    bAutoCompleteIsActive;
	std::wstring GetKeywords(eScriptKeywordType peType = eScriptKeywordStatement);
	CScriptCopyingMachine* oScript;
	};
