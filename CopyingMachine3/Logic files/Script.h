#pragma once

#include "ScriptExpression.h"
#include "ScriptLine.h"

//A class that holds the content of a maco
class CScript : public CScriptObject {
	public:
		CScript(sGlobalInstances pInstances);
		~CScript();

		void AddCommand(CScriptLine* poLine);
		virtual void Initialise();
		virtual bool Execute(bool &pbContinue, bool &pbEndOfScript);
		virtual bool ExecuteCommand(bool &pbContinue);
		bool GoToNextCommand();
		bool GoToLine(int piLine);

		bool Parse(std::wstring psScript);
		bool Parse(char* pCScript);

		int GetCommandCount();
		CScriptLine* GetCommand(int piIndex);

		CScriptLine* oScript;

		std::wstring	sError;

	protected:
		vector <CScriptLine*> oCommands;

		CScriptLine* oCursor; //Pointer to the position where new commands will be inserted
		CScriptLine* oScriptPointer; //Pointer to the command to be executed
		int iProgramCounter; //The index of the current command

		CTracer*	oTrace;
		sGlobalInstances oGlobalInstances;
		//CScriptObject* oObjectModel;

		void SetErrorMessage(std::wstring psError);
		bool ExecuteMessage(CScriptLine* poLine, bool &pbContinue);
		bool ExecuteLet(CScriptLine* poLine, bool &pbContinue);
		bool ExecuteConst(CScriptLine* poLine, bool &pbContinue);
		bool ExecuteFunction(CScriptLine* poLine, bool &pbContinue);
		bool ExecuteIf(CScriptLine* poLine, bool &pbContinue);
		bool ExecuteElse(CScriptLine* poLine, bool &pbContinue);
		bool ExecuteRepeat(CScriptLine* poLine, bool &pbContinue);
		bool ExecuteEnd(CScriptLine* poLine, bool &pbContinue);
		bool ExecuteUntil(CScriptLine* poLine, bool &pbContinue);

		bool EvaluateExpression(std::wstring sExpression, sScriptExpression& cValue);
							    
		virtual CScriptObject* GetChildObject(std::wstring psObjectname);

		//Variables and constants
		bool IsVariable(std::wstring psVariable);
		bool IsConstant(std::wstring psVariable);
		int IndexVariable(std::wstring psVariable);
		sScriptExpression GetVariable(std::wstring psVariable);
		bool SetVariable(std::wstring psVariable, sScriptExpression pcNewValue);

	private:

		bool EvaluateExpression(std::wstringstream& ssExpression,
								sScriptExpression& cValue,
								int piLevel=0);
							    
		bool ComputePartialExpression(sScriptExpression& cCurrentValue,
									  std::wstring psPartialExpression,
									  eScriptExpressionOperator cOperator);

		bool ComputeExpressionTerms(sScriptExpression& cCurrentValue,
									sScriptExpression& pcTerm,
									eScriptExpressionOperator cOperator);

		eScriptExpressionOperator GetOperator(std::wstringstream& ssExpression);

};

