#include "StdAfx.h"

#include "CopyingMachineCore.h"
#include "ScriptProcessorKeywords.h"
#include "ScriptParser.h"
#include <cctype>
#include <string>
#include <algorithm>

CScript::CScript(sGlobalInstances pcGlobalInstances) :
	CScriptObject(L"Script")
{
	oTrace = pcGlobalInstances.oTrace;

	oTrace->StartTrace(__WFUNCTION__);

	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pcGlobalInstances, sizeof(sGlobalInstances));

	Initialise();

	oTrace->EndTrace(__WFUNCTION__);
}

CScript::~CScript()
{
	oTrace->StartTrace(__WFUNCTION__);

	delete oScript;

	while (oCommands.size()>0) {

		delete oCommands[0];
		oCommands.erase(oCommands.begin());
		}

	oTrace->EndTrace(__WFUNCTION__);
	}



void 
CScript::Initialise() {

	oTrace->StartTrace(__WFUNCTION__);

	//init the variables.
	oScript = new CScriptLine(oTrace);
	oScript->oParent = NULL;
	//oObjectModel = NULL;
	oCursor = oScript;
	oScriptPointer = NULL;

	//clear the commands
	while (oCommands.size()>0) {

		delete oCommands[0];
		oCommands.erase(oCommands.begin());
		}

	iProgramCounter = -1;

	oTrace->EndTrace(__WFUNCTION__);
	}

//The function below take a text and parse them as input for this script
bool 
CScript::Parse(std::wstring psScript) {

	CScriptParser* oParser;
	bool bResult;

	oParser = new CScriptParser(oGlobalInstances, this);
		
	bResult = oParser->Parse(psScript);

	delete oParser;

	return bResult;
	}


bool 
CScript::Parse(char* pcScript) {

	CScriptParser* oParser;
	bool bResult;

	oParser = new CScriptParser(oGlobalInstances, this);
		
	bResult = oParser->Parse(pcScript);

	delete oParser;

	return bResult;
	}

void 
CScript::AddCommand(CScriptLine* poLine) {

	oTrace->StartTrace(__WFUNCTION__);

	//set the parent
	poLine->oParent = oCursor;

	oCommands.push_back(poLine);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function returns the number of commands
int 
CScript::GetCommandCount() {

	int iResult;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	iResult = (int)oCommands.size();

	oTrace->EndTrace(__WFUNCTION__, iResult, eAll);

	return iResult;
}

//This function returns the requested command if it exists
CScriptLine* 
CScript::GetCommand(int piIndex) {

	CScriptLine* oResult;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	oResult = NULL;

	if (((int)oCommands.size() > piIndex) && (piIndex>=0)) {

		oResult = oCommands[piIndex];
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return oResult;
}

//Execute the script. The result is true if the execution of
//the script was successful. The parameter pbContinue is set to false
//by this function if the script should not immediately continue processing
//commands. 
bool 
CScript::Execute(bool &pbContinue, bool &pbEndOfScript) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;

	if (oScriptPointer==NULL) pbEndOfScript = !GoToNextCommand();

	//Execute the next command in the script
	while (pbContinue && bResult && !pbEndOfScript) {

		bResult = ExecuteCommand(pbContinue);
		pbEndOfScript = !GoToNextCommand();
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}
//Execute the current command. The result is true if the execution of
//the command was successful. The parameter pbContinue is set to false
//by this function if the script should not immediately continue processing
//commands. 
bool 
CScript::ExecuteCommand(bool &pbContinue) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;


	if (oScriptPointer!=NULL) {

		switch (oScriptPointer->cStatement) {

			case eScriptMessage:

				bResult = ExecuteMessage(oScriptPointer, pbContinue);
				break;

			case eScriptLet:

				bResult = ExecuteLet(oScriptPointer, pbContinue);
				break;

			case eScriptConst:

				bResult = ExecuteConst(oScriptPointer, pbContinue);
				break;

			case eScriptFunction:

				bResult = ExecuteFunction(oScriptPointer, pbContinue);
				break;

			case eScriptEnd:

				bResult = ExecuteEnd(oScriptPointer, pbContinue);
				break;

			case eScriptBegin:

				switch (oScriptPointer->cFunction) {

					case eScriptIf:
					
						bResult = ExecuteIf(oScriptPointer, pbContinue);
						break;
					};

				break;

			case eScriptElse:

				bResult = ExecuteElse(oScriptPointer, pbContinue);
				break;

			case eScriptRepeat:

				bResult = ExecuteRepeat(oScriptPointer, pbContinue);
				break;

			case eScriptUntil:

				bResult = ExecuteUntil(oScriptPointer, pbContinue);
				break;

			default:

				bResult = false; //unknown statement
				pbContinue = false;
				break;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function executes a message script command
bool 
CScript::ExecuteEnd(CScriptLine* poLine, bool &pbContinue) {

	bool bResult = true;
	sScriptExpression cValue;

	oTrace->StartTrace(__WFUNCTION__);

	if (poLine->cFunction == eScriptWhile) {

		pbContinue = GoToLine(poLine->iJumpToLine);
		iProgramCounter--;		
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function executes a message script command
bool 
CScript::ExecuteMessage(CScriptLine* poLine, bool &pbContinue) {

	bool bResult;
	sScriptExpression cValue;

	oTrace->StartTrace(__WFUNCTION__);

	cValue.init();
	bResult = EvaluateExpression(poLine->sExpression, cValue);
	
	if (bResult) {

		cValue.toString();
		MessageBox(NULL, cValue.sResult.c_str(), L"Message", MB_OK);

		pbContinue = true;
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function executes the IF statement
bool 
CScript::ExecuteIf(CScriptLine* poLine, bool &pbContinue) {

	bool bResult;
	sScriptExpression cValue;

	oTrace->StartTrace(__WFUNCTION__);

	cValue.init();
	bResult = EvaluateExpression(poLine->sExpression, cValue);
	
	if (bResult) {

		if (cValue.isBoolean()) {

			pbContinue = true;

			if (!cValue.bResult) {

				//Jump to the end or the else statement
				pbContinue = GoToLine(poLine->iJumpToLine);
				//pbContinue = GoToNextCommand();
				}
			}
		else {

			SetErrorMessage(L"Invalid expression, a boolean expected");
			pbContinue = false;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);	
	
	return bResult;	
}

//This function executes the IF statement
bool 
CScript::ExecuteRepeat(CScriptLine* poLine, bool &pbContinue) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;

	if (poLine->cFunction == eScriptWhile) {

		bResult = ExecuteIf(poLine, pbContinue);
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);	
	
	return bResult;	
}

//This function executes the IF statement
bool 
CScript::ExecuteUntil(CScriptLine* poLine, bool &pbContinue) {

	bool bResult;
	sScriptExpression cValue;

	oTrace->StartTrace(__WFUNCTION__);

	cValue.init();
	bResult = EvaluateExpression(poLine->sExpression, cValue);
	
	if (bResult) {

		if (cValue.isBoolean()) {

			if (!cValue.bResult) {

				//Jump to the end or the else statement
				pbContinue = GoToLine(poLine->iJumpToLine);
				//pbContinue = GoToNextCommand();
				}
			}
		else {

			SetErrorMessage(L"Invalid expression, a boolean expected");
			pbContinue = false;
			}

		pbContinue = true;
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);	
	
	return bResult;	
}

//This function executes the else statement
bool 
CScript::ExecuteElse(CScriptLine* poLine, bool &pbContinue) {

	oTrace->StartTrace(__WFUNCTION__);

	//Jump to the end
	pbContinue = GoToLine(poLine->iJumpToLine);
//	pbContinue = GoToNextCommand();

	oTrace->EndTrace(__WFUNCTION__, true);	

	return true;
	}

//This function executes a let script command
bool 
CScript::ExecuteLet(CScriptLine* poLine, bool &pbContinue) {

	bool bResult;
	sScriptExpression cValue;

	oTrace->StartTrace(__WFUNCTION__);

	cValue.init();
	bResult = EvaluateExpression(poLine->sExpression, cValue);
	
	if (bResult) {

		pbContinue = SetVariable(poLine->sVariable, cValue);
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function executes a const script command
bool 
CScript::ExecuteConst(CScriptLine* poLine, bool &pbContinue) {

	bool bResult;
	sScriptExpression cValue;

	oTrace->StartTrace(__WFUNCTION__);

	cValue.init();
	bResult = EvaluateExpression(poLine->sExpression, cValue);
	
	if (bResult) {

		cValue.bConstant = true;

		pbContinue = SetVariable(poLine->sVariable, cValue);
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function executes the give function
bool 
CScript::ExecuteFunction(CScriptLine* poLine, bool &pbContinue) {

	bool bResult;
	sScriptExpression cValue;
	std::wstring sFunction;
	int iPosition;

	oTrace->StartTrace(__WFUNCTION__);

	//initialise
	cValue.init();
	iPosition = poLine->sExpression.find(L"(");
	sFunction = poLine->sExpression.substr(0, iPosition);

	bResult = CScriptObject::Execute(sFunction, cValue);
	if (!bResult) SetErrorMessage(CScriptObject::sError);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}



//This function evaluates an expression and tries to calculate/determine
//the value
bool 
CScript::EvaluateExpression(std::wstring sExpression,
							sScriptExpression& pcValue) {

	std::wstringstream ssExpression;
	bool bResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	oTrace->Add(L"Expression", sExpression);

	//Initialise 
	ssExpression << sExpression;

	//perform the evaluation
	bResult = EvaluateExpression(ssExpression, pcValue);

	switch (pcValue.cValueType) {

		case eScriptValueFloat:
			oTrace->Add(L"Result", (int)pcValue.fResult);
			break;
		case eScriptValueInteger:
			oTrace->Add(L"Result", pcValue.iResult);
			break;
		case eScriptValuePointer:
			oTrace->Add(L"Result", (int)pcValue.pPointer);
			break;
		case eScriptValueBoolean:
			oTrace->Add(L"Result", (bool)pcValue.bResult);
			break;
		case eScriptValueString:
			oTrace->Add(L"Result", pcValue.sResult);
			break;
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function retrieves a part of the expression and tries to
//evaluate it to a value.
bool 
CScript::EvaluateExpression(std::wstringstream& ssExpression,
								sScriptExpression& pcValue,
								int piLevel) {
	std::wstring sPartExpression;
	sScriptExpression cValue;
	wchar_t cCharacter;
	bool bResult;
	bool bDone;
	bool bInQuotes;
	eScriptExpressionOperator cOperator;
	
	oTrace->StartTrace(__WFUNCTION__);

	//Initialise 
	bResult = true;
	bDone = false;
	bInQuotes = false;
	cOperator = eScriptNone;

	//Iterate through the stream
	while (bResult && !bDone && ssExpression.get(cCharacter)) {

		switch (cCharacter) {

			case 0x0028: // = (

				if (GetType(sPartExpression) == eScriptObjectFunction) {

					//execute the function
					cValue.init();
					bResult = CScriptObject::Execute(sPartExpression, cValue);
					sPartExpression = L"";		
					while (cCharacter!=0x0029) {
						ssExpression.get(cCharacter);
						}
					}
				else {
					//the ( didn't mean the arguments of a function, so evaluate wat is in between.
					cValue.init();
					bResult = EvaluateExpression(ssExpression, cValue);
					}
				if (bResult) bResult = ComputeExpressionTerms(pcValue, cValue, cOperator);
				cOperator = eScriptNone;

				break;

			case 0x0029: // = )
				bDone = true;
				break;

			case 0x0020: // = space
				if (bInQuotes) sPartExpression += cCharacter;
				break;

			case 0x0022: // = quote

				do {

					if (cCharacter > 0x0019) sPartExpression += cCharacter;
					if (cCharacter == 0x0022) bInQuotes = !bInQuotes;				

					} while (bInQuotes && ssExpression.get(cCharacter));

				if (bInQuotes) {

					bResult = false;
					SetErrorMessage(L"Unterminated string");
					}

				break;

			case 0x002B: // = + operator
			case 0x002D: // = - operator
			case 0x002A: // = * operator
			case 0x002F: // = / operator
			case 0x003C: // = < operator
			case 0x003D: // = = operator
			case 0x003E: // = > operator
			case 0x0021: // = ! operator

				bResult = ComputePartialExpression(pcValue, sPartExpression, cOperator);	
				ssExpression.putback(cCharacter);
				cOperator = GetOperator(ssExpression);
				sPartExpression = L"";		
				break;

			default:

				if (cCharacter > 0x0019) sPartExpression += cCharacter;
				break;
			}
		}

	//Compute the (last part of the) expression
	if (bResult) {
		
		bResult = ComputePartialExpression(pcValue, sPartExpression, cOperator);	
		}


	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function gets the operator in the expression. 
eScriptExpressionOperator 
CScript::GetOperator(std::wstringstream& ssExpression) {

	wchar_t cCharacter;
	std::wstring sOperator;
	eScriptExpressionOperator cResult;

	oTrace->StartTrace(__WFUNCTION__);

	ssExpression.get(cCharacter);
	sOperator = cCharacter;

	if (ssExpression.good()) {

		cCharacter = ssExpression.peek();

		if (cCharacter == 0x003D) {

			ssExpression.get(cCharacter);
			sOperator += cCharacter;
			}
		}

	if (sOperator == L"+") cResult = eScriptPlus;
	if (sOperator == L"-") cResult = eScriptMinus;
	if (sOperator == L"*") cResult = eScriptMultiply;
	if (sOperator == L"/") cResult = eScriptDivide;
	if (sOperator == L"==") cResult = eScriptEquals;
	if (sOperator == L"!=") cResult = eScriptNotEqual;
	if (sOperator == L">") cResult = eScriptLarger;
	if (sOperator == L">=") cResult = eScriptLargerOrSame;
	if (sOperator == L"<") cResult = eScriptSmaller;
	if (sOperator == L"<=") cResult = eScriptSmallerOrSame;

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
}


//This function computes the new value.
bool 
CScript::ComputePartialExpression(sScriptExpression& pcValue,
									std::wstring psPartialExpression,
									eScriptExpressionOperator cOperator) {

	bool bResult;
	int iPosition;
	sScriptExpression cTerm;
	
	oTrace->StartTrace(__WFUNCTION__);

	//Initialise 
	bResult = true;
	sError = L"";
	cTerm.init();

	//Is it a boolean?
	if (psPartialExpression == ScrYes) {

		cTerm.cValueType = eScriptValueBoolean;
		cTerm.bResult = true;
		}

	if (psPartialExpression == ScrNo) {

		cTerm.cValueType = eScriptValueBoolean;
		cTerm.bResult = false;
		}

	//Is it a number?
	if (cTerm.cValueType == eScriptValueAny) {

		iPosition = psPartialExpression.find_first_not_of(L"01234567890.");
		if (iPosition<0) {
			
			iPosition = psPartialExpression.find_first_of(L"."); 
			if (iPosition>=0) {
				cTerm.cValueType = eScriptValueFloat;
				cTerm.fResult = (float)_wtof(psPartialExpression.c_str());
				}
			else {
				cTerm.cValueType = eScriptValueInteger;
				cTerm.iResult = _wtoi(psPartialExpression.c_str());
				}
			}
		}	
	
	//Is it a string?
	if (cTerm.cValueType == eScriptValueAny) {
		
		iPosition = psPartialExpression.find_first_of(L"\""); 
		
		if (iPosition==0) {

			cTerm.cValueType = eScriptValueString;
			cTerm.sResult = psPartialExpression;

			cTerm.sResult.erase(cTerm.sResult.begin()); //remove the quotes
			cTerm.sResult.erase(cTerm.sResult.begin() + cTerm.sResult.length()-1);
			}
		}

	//is it a variable?
	if (cTerm.cValueType == eScriptValueAny) {

		if (IsVariable(psPartialExpression)) {
			
			cTerm = GetVariable(psPartialExpression);
			}
		}

	//Is it a reference to an object?
	if (cTerm.cValueType == eScriptValueAny) {

		eScriptObjectType cType;
		
		cType = GetType(psPartialExpression);

		if (cType==eScriptObjectFunction || cType==eScriptObjectAttribute) {

			//To lowercase
			transform(
				psPartialExpression.begin(), psPartialExpression.end(),
				psPartialExpression.begin(),
				tolower); 

			if (cType==eScriptObjectAttribute) {
				cTerm = Get(psPartialExpression);
				}
			else {
				CScriptObject::Execute(psPartialExpression, cTerm);
				}
			
			if (cTerm.cValueType == eScriptValueAny) sError = CScriptObject::sError; 
			}
		}


	if (cTerm.cValueType != eScriptValueAny) {

		//Do the computation
		bResult = ComputeExpressionTerms(pcValue, cTerm, cOperator);
		}
	else {

		bResult = false;
		if (sError==L"") SetErrorMessage(L"Syntax error");
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function computes the result of two terms by applying an operator
bool 
CScript::ComputeExpressionTerms(sScriptExpression& pcValue,
								sScriptExpression& cTerm,
								eScriptExpressionOperator cOperator) {

	bool bResult;
	int iCompareResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	//Initialise 
	bResult = true;

	//No operator and no script value set yet... this is the first term
	//Just make the two the same
	switch (cOperator) {

		case eScriptNone:

			if (pcValue.cValueType == eScriptValueAny) {

				pcValue.bResult   = cTerm.bResult;
				pcValue.fResult   = cTerm.fResult;
				pcValue.iResult   = cTerm.iResult;
				pcValue.sResult   = cTerm.sResult;
				pcValue.pPointer   = cTerm.pPointer;
				pcValue.cValueType = cTerm.cValueType;
				}
			break;

		case eScriptEquals:
		case eScriptNotEqual:
		case eScriptLarger:
		case eScriptLargerOrSame:
		case eScriptSmallerOrSame:
		case eScriptSmaller:
		case eScriptPlus:

			if (pcValue.cValueType != cTerm.cValueType) {
	
				//If numbers are of different type, convert it to float
				if (pcValue.isNumber() && cTerm.isNumber()) {

					switch (pcValue.cValueType) {

						case eScriptValueFloat:
							cTerm.toFloat();
							break;
						case eScriptValueInteger:
							cTerm.toInteger();
							break;
						}
					}
					

				//If operating on number/boolean and string
				if (pcValue.isString() || cTerm.isString()) {

					pcValue.toString();
					cTerm.toString();
					}
				}
			
			//Do the operation
			if (cOperator == eScriptPlus) {
				
				if ((pcValue.cValueType == cTerm.cValueType) && 
					(pcValue.isNumber() || pcValue.isString())) {

					pcValue.iResult += cTerm.iResult;
					pcValue.fResult += cTerm.fResult;
					pcValue.sResult += cTerm.sResult;
					}
				else {

					bResult = false;
					SetErrorMessage(L"Invalid expression (two terms cannot be added)");
					}
				}
			else {

				if (pcValue.cValueType == cTerm.cValueType) {

					iCompareResult = pcValue.Compare(cTerm);

					switch (cOperator) {

						case eScriptEquals:
							pcValue.bResult = (iCompareResult==0);							
							break;
						case eScriptNotEqual:
							pcValue.bResult = (iCompareResult!=0);							
							break;
						case eScriptLarger:
							pcValue.bResult = (iCompareResult>0);							
							break;
						case eScriptLargerOrSame:
							pcValue.bResult = (iCompareResult>=0);							
							break;
						case eScriptSmaller:
							pcValue.bResult = (iCompareResult<0);							
							break;
						case eScriptSmallerOrSame:
							pcValue.bResult = (iCompareResult<=0);							
							break;
						};

						pcValue.cValueType = eScriptValueBoolean;
					}
				else {

					bResult = false;
					SetErrorMessage(L"Invalid expression (two terms of a different type cannot be compared)");
					}
				}

			break;

		case eScriptMinus:
		case eScriptMultiply:
		case eScriptDivide:

			if (pcValue.cValueType != cTerm.cValueType) {
	
				//If numbers are of different type, convert it to float
				if (pcValue.isNumber() && cTerm.isNumber()) {

					switch (pcValue.cValueType) {

						case eScriptValueFloat:
							cTerm.toFloat();
							break;
						case eScriptValueInteger:
							cTerm.toInteger();
							break;
						}
					}
				}

			if ((pcValue.cValueType == cTerm.cValueType)
				&& pcValue.isNumber()) {

				switch (cOperator) {

					case eScriptMinus:

						pcValue.iResult -= cTerm.iResult;
						pcValue.fResult -= cTerm.fResult;
						break;
					case eScriptMultiply:

						pcValue.iResult = pcValue.iResult * cTerm.iResult;
						pcValue.fResult = pcValue.fResult * cTerm.fResult;
						break;
					case eScriptDivide:

						pcValue.iResult = pcValue.iResult / cTerm.iResult;
						pcValue.fResult = pcValue.fResult / cTerm.fResult;
						break;
					}
				}
			else {

				bResult = false;
				switch (cOperator) {

					case eScriptMinus:
						SetErrorMessage(L"Invalid expression (two terms cannot be substracted)");
						break;
					case eScriptMultiply:
						SetErrorMessage(L"Invalid expression (two terms cannot be multiplied)");
						break;
					case eScriptDivide:
						SetErrorMessage(L"Invalid expression (two terms cannot be divided)");
						break;
					}
				}

			break;
		}
	
	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}


void
CScript::SetErrorMessage(std::wstring psError) {

	wchar_t* cMessage;
	int iMessageLength;
	int iLine;

	oTrace->StartTrace(__WFUNCTION__);

	psError.append(L" in line %d");
	iLine = oScriptPointer->iLine;

	iMessageLength = psError.length() + 10;

	cMessage = (wchar_t*)malloc(iMessageLength*2);

	swprintf_s(cMessage, iMessageLength, psError.c_str(), iLine);
	sError = cMessage;

	free (cMessage);

	oTrace->EndTrace(__WFUNCTION__);
	}

//Step to the next command in the script. The result will be false if the
//script is at then end
bool 
CScript::GoToNextCommand() {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;

	iProgramCounter++;
	oScriptPointer = NULL;

	if (iProgramCounter < (int)oCommands.size()) {

		oScriptPointer = oCommands[iProgramCounter];	
		}

	//Is it a comment, then skip it
	if (oScriptPointer) {
		if (oScriptPointer->cStatement == eScriptComment) GoToNextCommand();
		}

	//Check if we have a new script line
	bResult = ((oScriptPointer!=NULL) && (oScriptPointer->cStatement != eScriptExit));

	oTrace->EndTrace(__WFUNCTION__, bResult);
	
	return bResult;
	}

//Go to the next line.
bool
CScript::GoToLine(int piLine) {

	bool bResult;
	int  iNewProgramCounter;
	int  iDelta;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;
	iNewProgramCounter = iProgramCounter;
	
	if (piLine != oScriptPointer->iLine) {
	
		bResult = false;
		iDelta = (oScriptPointer->iLine > piLine ? -1 : 1);
		
		while (!bResult && 
				iNewProgramCounter>=0 && 
				iNewProgramCounter<(int)oCommands.size()) {
				
				iNewProgramCounter += iDelta;
				if (oCommands[iNewProgramCounter]->iLine == piLine) {

					bResult = true;
				}
			}
		}


	if (bResult) {

		iProgramCounter = iNewProgramCounter - 1;
		bResult = GoToNextCommand();
		}
	else {

		SetErrorMessage(L"Next command not found");
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);
	
	return bResult;
	}


//This function get the requested type
CScriptObject* 
CScript::GetChildObject(std::wstring psObjectname) {

	CScriptObject* oResult;
	long lIndex;

	oResult = NULL;
	lIndex = (long)oChildren.size();

	sScriptExpression cVariable;

	if (IsVariable(psObjectname)) {

		cVariable = GetVariable(psObjectname);
		if (cVariable.isObject()) {

			//typecast the pointer to a scriptobject
			oResult = (CScriptObject*)cVariable.pPointer;
			}
		}
	
	while (lIndex > 0 && oResult==NULL) {
		
		lIndex--;

		if (oChildren[lIndex]->sName == psObjectname) 
			oResult = oChildren[lIndex];
		}


	return oResult;
	}

//This function test if the given string is an existing variable
bool 
CScript::IsVariable(std::wstring psVariable) {

	//To lowercase
	transform(
		psVariable.begin(), psVariable.end(),
		psVariable.begin(),
		tolower); 

	//Test if the variable its name already exists
	mVariables::iterator VariablesIterator;

	VariablesIterator = oVariables.find(psVariable);

	return (VariablesIterator != oVariables.end());
	}

//This function test if the given string is an existing variable
bool 
CScript::IsConstant(std::wstring psVariable) {

	bool bResult;
	sScriptExpression cValue;

	bResult = false;

	//To lowercase
	transform(
		psVariable.begin(), psVariable.end(),
		psVariable.begin(),
		tolower); 

	//Test if the variable its name already exists
	mVariables::iterator VariablesIterator;

	VariablesIterator = oVariables.find(psVariable);

	if (VariablesIterator != oVariables.end()) {

		cValue = GetVariable(psVariable);
		bResult = cValue.bConstant;
		}

	return bResult;
	}

//This function test if the given string is an existing variable, and if
//so it will return the index in the oVariables vector
int 
CScript::IndexVariable(std::wstring psVariable) {

	int iResult;

	//iResult = (int)oVariables.size();

	return iResult;
	}

//This function retrieves the value of the requested variable
sScriptExpression 
CScript::GetVariable(std::wstring psVariable) {

	sScriptExpression cResult;

	//To lowercase
	transform(
		psVariable.begin(), psVariable.end(),
		psVariable.begin(),
		tolower); 

	//Find the variable and get its value
	cResult.init();
	
	if (oVariables.find(psVariable)!=oVariables.end()) {

		cResult = oVariables[psVariable];
		}

	return cResult;
	}

//This function updates or sets the value of an existing or new variable
bool 
CScript::SetVariable(std::wstring psVariable, sScriptExpression pcNewValue) {

	bool bResult;

	bResult = false;

	//If it is a variable, update the value
	if (IsVariable(psVariable)) {

		if (!IsConstant(psVariable)) {

			oVariables[psVariable] = pcNewValue;
			bResult = true;
			}
		else {

			SetErrorMessage(L"Cannot change the value of a constant");
			}
		}
	else {

		//It isn't a known variable, check if it is a object
		if (Test(psVariable)) {

			if (Set(psVariable, pcNewValue)) bResult = true;
			}
		else {

			//Not a known variable, create one
			oVariables[psVariable] = pcNewValue;
			bResult = true;
			}
		}

	return bResult;
	}


