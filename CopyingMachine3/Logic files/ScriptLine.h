#pragma once
#include "ScriptProcessorKeywords.h"
#include "ScriptExpression.h"

// A class that holds the contents of a line;
class CScriptLine {

	public:
		CScriptLine(CTracer* poTrace);
		~CScriptLine();

		//void AddCommand(CScriptLine* poLine);
		bool AddStatement(eScriptKeyword pcKeyword);
		bool AddFunction(eScriptKeyword pcKeyword);
		bool AddAttribute(eScriptKeyword pcKeyword, eScriptKeyword pcContext);

		eScriptKeyword	cStatement;
		eScriptKeyword	cAttribute;
		eScriptKeyword  cFunction;
		std::wstring    sVariable;
		std::wstring	sExpression;

		CScriptLine*	oParent;

		int				iLine;
		int				iJumpToLine; //used in case of an 'if' or 'loop' statement
		std::wstring	sError;

	private:

		std::wstring GetKeywordString(eScriptKeyword pcKeyword);

		CTracer*	oTrace;
		};