#pragma once
#include "Script.h"


//The class of the parser for scripts. This class contains
//the code that translate a text into a Script class instance.
class CScriptParser {

	public:
		CScriptParser(sGlobalInstances pInstances, CScript* poScript);
		~CScriptParser();

		bool Parse(std::wstring psScript);
		bool Parse(char* pCScript);
		bool ParseLine(std::wstring psScriptLine);

		vector<std::wstring> sErrorMessages;

	private:

		void AddErrorMessage(std::wstring psError, int piLine);
		eScriptKeyword GetKeyword(std::wstring& psLine);
		std::wstring GetKeywordString(std::wstring& psLine);
		bool GetExpression(std::wstring& psLine, std::wstring& psExpression, bool pbExpressionRequired = true);
		bool CheckObjectFunction(std::wstring psLine);
		bool CheckObjectAttribute(std::wstring psLine);

		CTracer*	oTrace;
		CRegistry*	oRegistry;
		sGlobalInstances oGlobalInstances;

		CScript* oScriptBeingProcessed;

		vector <CScriptLine*> oScriptContext;

		int iCurrentLine;
		};

