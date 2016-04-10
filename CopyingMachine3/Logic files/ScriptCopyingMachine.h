#pragma once
#include "Script.h"

//A class that holds the functionality of the script object for
//this specific application
class CScriptCopyingMachine : public CScript {

	public:
		CScriptCopyingMachine(sGlobalInstances pInstances);
		~CScriptCopyingMachine();

		bool ExecuteCommand(bool &pbContinue);
		
		void AddVariablesToAutoComplete(std::wstring psRoot, std::vector<std::wstring>& psElements);
		std::wstring GetVariablesToKeywords(); 

		virtual void Initialise();

	private:

		bool ExecuteScan(CScriptLine* poLine);
		bool ExecutePrint(CScriptLine* poLine);
		bool ExecuteCopy(CScriptLine* poLine);
		bool ExecutePage(CScriptLine* poLine);
		};

