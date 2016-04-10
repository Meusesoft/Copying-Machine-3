#pragma once
#include "ScriptProcessorKeywords.h"
#include "ScriptExpression.h"
#include "ScriptObject.h"
#include "Script.h"
#include "ScriptCopyingMachine.h"

//The class of the Script Processor itself
class CScriptProcessor {

	public:
		CScriptProcessor(sGlobalInstances pInstances);
		~CScriptProcessor();

		bool Process(std::wstring psScript);
		bool Process(char* pCScript);

		vector<std::wstring> sErrorMessages;

		void ExecuteNextScriptLine();

	private:

		void AddErrorMessage(std::wstring psError, int piLine);

		CTracer*	oTrace;
		CRegistry*	oRegistry;
		sGlobalInstances oGlobalInstances;

		void* oScriptBeingExecuted;

		vector <void*> oScriptQueue;
		};
