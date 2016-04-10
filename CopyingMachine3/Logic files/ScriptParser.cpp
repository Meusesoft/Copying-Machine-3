#include "StdAfx.h"

#include "CopyingMachineCore.h"
#include "ScriptProcessorKeywords.h"
#include "ScriptParser.h"
#include <cctype>
#include <string>
#include <algorithm>

 
CScriptParser::CScriptParser(sGlobalInstances pcGlobalInstances, CScript* poScript)
{
	oTrace = pcGlobalInstances.oTrace;

	oTrace->StartTrace(__WFUNCTION__);

	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pcGlobalInstances, sizeof(sGlobalInstances));

	//initialise variables
	oScriptBeingProcessed = poScript;


	oTrace->EndTrace(__WFUNCTION__);
}

CScriptParser::~CScriptParser()
{
	oTrace->StartTrace(__WFUNCTION__);

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function start the processing of the macro
bool
CScriptParser::Parse(std::wstring psScript) {

	std::wstring sLine;
	//std::wstring sScriptHeader;
	std::wstringstream ssScript;
	bool bReturn;

	oTrace->StartTrace(__WFUNCTION__);

	bReturn = true;
	sErrorMessages.clear();

	//sScriptHeader = L"const ScanDocument 0\n\rconst ScanPage 1\n\rconst ScanEvenPages 2\n\rconst ScanOddPages 3\n\r";
	//psScript = sScriptHeader + psScript;	

	//place the string into a stream
	ssScript << psScript;

	//create a script object
	//oScriptBeingProcessed = new CScriptCopyingMachine(oGlobalInstances);

	//extract the lines and process them
	iCurrentLine = 1;

	while (std::getline(ssScript, sLine)) {

		if (sLine.size() > 0) {
			bReturn = (bReturn && ParseLine(sLine));
			}

		iCurrentLine++;
		}

	//if error message then present them
	if (sErrorMessages.size()>0) {

		std::wstring sMessage;

		for (int iIndex=0; iIndex<(int)sErrorMessages.size(); iIndex++) {

			sMessage += sErrorMessages[iIndex];
			sMessage += L"\n\r";
			}

		::MessageBox(NULL, sMessage.c_str(), L"Error", MB_OK | MB_ICONERROR);

		bReturn = false;
		}

	oTrace->EndTrace(__WFUNCTION__, bReturn);

	return bReturn;
}

//This function start the processing of the macro
bool
CScriptParser::Parse(char* pcScript) {

	std::string sScript;
	std::wstring wsScript;
	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	sScript = pcScript;

	wsScript.assign(sScript.begin(), sScript.end());

	bResult = Parse(wsScript);

	free((void*)pcScript);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
}

//This function processes a line of the macro
bool
CScriptParser::ParseLine(std::wstring psLine) {

	CScriptLine* oLine;
	CScriptLine* oContextLine;
	eScriptKeyword cKeyword;
	std::wstring sExpression;
	std::wstring sVariableName;
	bool bReturn;

	oTrace->StartTrace(__WFUNCTION__);

	bReturn = true;

	oLine = new CScriptLine(oTrace);

	cKeyword = GetKeyword(psLine);

	if (cKeyword!=eScriptEmpty) {
	
		bReturn = (bReturn && oLine->AddStatement(cKeyword));
		if (!bReturn) AddErrorMessage(oLine->sError, iCurrentLine);

		//add the line
		oLine->iLine = iCurrentLine;
		oScriptBeingProcessed->AddCommand(oLine);

		//process the remainder of the line
		switch (oLine->cStatement) {

			case eScriptRepeat: 
			case eScriptBegin: {

				cKeyword = GetKeyword(psLine);
				bReturn = oLine->AddFunction(cKeyword);
				oScriptContext.push_back(oLine);
				if (!bReturn && oLine->cStatement!=eScriptRepeat) AddErrorMessage(oLine->sError, iCurrentLine);
				bReturn = GetExpression(psLine, sExpression, !(oLine->cStatement==eScriptRepeat && oLine->cFunction==eScriptUnknown));
				if (bReturn) oLine->sExpression = sExpression;
				if (!bReturn && cKeyword==eScriptIf) {
					AddErrorMessage(L"An 'if' statement requires a valid expression", iCurrentLine);
					}
				if (!bReturn && cKeyword==eScriptWhile) AddErrorMessage(L"An 'while' statement requires a valid expression", iCurrentLine);
				break;
				}

			case eScriptEnd: {

				if (oScriptContext.size()>0) {
					//if this is the end of an if statement, set the jump
					oContextLine = oScriptContext.back();
					oLine->AddFunction(oContextLine->cFunction);

					if (oContextLine->cFunction == eScriptIf || oContextLine->cFunction == eScriptElse) {

						oContextLine->iJumpToLine = iCurrentLine;
						}

					if (oContextLine->cStatement == eScriptRepeat) {

						if (oContextLine->cFunction == eScriptWhile) {

							oLine->iJumpToLine = oContextLine->iLine;
							oContextLine->iJumpToLine = oLine->iLine;
							}
						else {

							AddErrorMessage(L"A 'repeat while' must be paired with an 'until' statement expected instead of 'end'.", iCurrentLine);
							}
						}
					}

				oScriptContext.pop_back();
				break;
				}

			case eScriptUntil: {

				if (oScriptContext.size()>0) {
					//if this is the end of an if statement, set the jump
					oContextLine = oScriptContext.back();

					if (oContextLine->cStatement == eScriptRepeat) {

						if (oContextLine->cFunction == eScriptWhile) {

							AddErrorMessage(L"A 'repeat while' must be paired with an 'end' statement expected instead of 'until'.", iCurrentLine);
							}
						else {
							oLine->iJumpToLine = oContextLine->iLine;
							bReturn = GetExpression(psLine, sExpression);
							if (bReturn) oLine->sExpression = sExpression;
							if (!bReturn) AddErrorMessage(L"An 'until' statement required a valid expression.", iCurrentLine);
							}
						}
					}

				oScriptContext.pop_back();
				break;
				}

			case eScriptElse: {

				oLine->AddFunction(cKeyword);
				
				bReturn = false;
				if (oScriptContext.size()>0) {
					//if this is the end of an if statement, set the jump if it hasn't been set by a preceding else
					oContextLine = oScriptContext.back();

					if (oContextLine->cFunction == eScriptIf) {

						oContextLine->iJumpToLine = iCurrentLine;
						bReturn = true;
						}
					
					oScriptContext.pop_back();
					oScriptContext.push_back(oLine);
					}

				if (!bReturn) {

					AddErrorMessage(L"An 'else' statement is found without a preceding if.", iCurrentLine);
					}

				break;
				}

			case eScriptMessage: {

				bReturn = GetExpression(psLine, sExpression);
				if (bReturn) oLine->sExpression = sExpression;
				
				break;
				}

			case eScriptSet: {

				cKeyword = GetKeyword(psLine);
				bReturn = oLine->AddAttribute(cKeyword, oScriptContext.size()>0 ? oScriptContext.back()->cFunction : eScriptUnknown);
				if (!bReturn) AddErrorMessage(oLine->sError, iCurrentLine);

				bReturn = GetExpression(psLine, sExpression);
				if (bReturn) oLine->sExpression = sExpression;
				
				break;
				}

			case eScriptFunction:

				oLine->sExpression = psLine;
				break;

			case eScriptConst:
			case eScriptLet: {

				sVariableName = GetKeywordString(psLine);
				if (sVariableName.length()>0) oLine->sVariable = sVariableName;

				//remove spaces/tabs and the = sign 
				while (psLine.find_first_of(L" \t=")==0) {
					psLine.erase(psLine.begin());
					}

				bReturn = GetExpression(psLine, sExpression);
				if (bReturn) oLine->sExpression = sExpression;
				
				break;
				}
			}
		}
	else {

		//this line was empty
		delete oLine;
		bReturn = true;
		}

	oTrace->EndTrace(__WFUNCTION__);

	return bReturn;
}

//This function extracts a key from the current line
eScriptKeyword 
CScriptParser::GetKeyword(std::wstring& psLine) {

	eScriptKeyword cResult;
	std::wstring sSubstring;
	std::wstring sError;
	int iKeywords;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	cResult = eScriptUnknown;

	sSubstring = GetKeywordString(psLine);

	//enumerate through the keywords
	iKeywords = sizeof(aKeywordStrings) / sizeof(KeywordStringCombination);

	while (iKeywords > 0 && cResult==eScriptUnknown) {

		iKeywords--;
		if (aKeywordStrings[iKeywords].sString == sSubstring) {

			cResult = aKeywordStrings[iKeywords].cKeyword;
			}
		}

	//if keyword unknown then it may be an variable name that is set,
	//check if it is in the format of 'variable = expression'.
	if (cResult==eScriptUnknown) {

		//remove and space in front of string
		while (psLine.find_first_of(L" \t")==0) {
			psLine.erase(psLine.begin());
			}
		
		if (psLine.find_first_of(L"=")==0 
			&& psLine.length()>1
			&& psLine.find_first_not_of(L"=")==1) {

			cResult = eScriptLet;
			psLine = sSubstring + psLine;
			}
		}

	//if keyword is still unknown, that it might be a function from an object
	//check if that is the case
	if (cResult==eScriptUnknown) {

		//remove and space in front of string
		while (psLine.find_first_of(L" \t")==0) {
			psLine.erase(psLine.begin());
			}
		
		if (CheckObjectFunction(sSubstring + psLine)) {

			cResult = eScriptFunction;
			psLine = sSubstring + psLine;
			}
		}

	//if keyword is still unknown, that it might be an attribute from an object
	//check if that is the case
	if (cResult==eScriptUnknown) {

		//remove and space in front of string
		while (psLine.find_first_of(L" \t")==0) {
			psLine.erase(psLine.begin());
			}
		
		if (CheckObjectAttribute(sSubstring + psLine)) {

			cResult = eScriptLet;
			psLine = sSubstring + psLine;
			}
		}

	//if keyword unknown then set an error
	if (cResult==eScriptUnknown) {

		sError = L"Unknown keyword '";
		sError += sSubstring;
		sError += L"'";

		AddErrorMessage(sError, iCurrentLine);
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	
	return cResult;
	}

//This function checks if the given line is a function of an script object
bool 
CScriptParser::CheckObjectFunction(std::wstring psLine) {

	bool bResult;
	std::wstring sSubstring;
	int iPosition;

	bResult = false;
	iPosition = psLine.find_first_of(L"(");
	
	sSubstring = psLine.substr(0, iPosition);

	iPosition = psLine.find_first_of(L".");

	if (iPosition>0 && sSubstring.length()>0) bResult = true; // we assume it is a function. Not really sure though.

//	bResult = oScriptBeingProcessed->IsFunction(sSubstring);	

	return bResult;
	}

//This function checks if the given line is a function of an script object
bool 
CScriptParser::CheckObjectAttribute(std::wstring psLine) {

	bool bResult;
	std::wstring sSubstring;
	int iPosition;

	bResult = false;
	iPosition = psLine.find_first_of(L" =");
	
	sSubstring = psLine.substr(0, iPosition);

	bResult = oScriptBeingProcessed->IsAttribute(sSubstring);	

	return bResult;
	}

//This function extracts a key from the current line
std::wstring 
CScriptParser::GetKeywordString(std::wstring& psLine) {

	std::wstring sResult;
	int iPosition;
	std::wstring sError;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	sResult = L"";

	//remove and space in front of string
	while (psLine.find_first_of(L" \t")==0) {
		psLine.erase(psLine.begin());
		}

	//grab the keyword string
	if (psLine.find(ScrCommentShort)==0) {

		iPosition = 1;
		sResult = psLine.substr(0, 1);
		}
	else {
	
		iPosition = psLine.find_first_not_of(L"abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.");
		sResult = psLine.substr(0, iPosition);
		}

	//Remove the extracted substring from the line
	if (iPosition>0) {
		psLine.erase(psLine.begin(), psLine.begin()+iPosition);
		}
	else {
		psLine.clear();
		};

	//To lowercase
	transform(
		sResult.begin(), sResult.end(),
		sResult.begin(),
		tolower); 

	oTrace->EndTrace(__WFUNCTION__, eAll);
	
	return sResult;
	}

//This function retrieves the string of an expression
bool 
CScriptParser::GetExpression(std::wstring& psLine, std::wstring& psExpression, bool pbExpressionRequired) {

	bool bResult;
	bool bInQuote;
	bool bEndOfLine;
	int	 iPointer;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	bResult = true;

	//remove tabs, space and = in front of string
	while (psLine.find_first_of(L" =\t")==0) {
		psLine.erase(psLine.begin());
		}

	//find the end of the expression
	bInQuote = false;
	iPointer = 0;
	bEndOfLine = (iPointer >= psLine.length());

	while (!bEndOfLine) {

		if (psLine.substr(iPointer, 1)==L"\"") {

			bInQuote = !bInQuote;
			}

		if (psLine.substr(iPointer, 1)==L"'" && !bInQuote) {

			bEndOfLine = true;
			iPointer--;
			}

		if (iPointer >= psLine.length()) bEndOfLine=true;
		iPointer++;
		}

	//missing expression
	if (iPointer==0 && pbExpressionRequired) {

		bResult = false;
		AddErrorMessage(L"Missing expression", iCurrentLine);
		}

	//unterminated string
	if (bInQuote) {

		bResult = false;
		AddErrorMessage(L"Unterminated string expression", iCurrentLine);
		}

	if (bResult) {

		psExpression = psLine.substr(0, iPointer);
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return bResult;
	}

//This function adds an error message to the message queue
void
CScriptParser::AddErrorMessage(std::wstring psError, int piLine) {

	wchar_t* cMessage;
	std::wstring sMessage;
	int iMessageLength;

	oTrace->StartTrace(__WFUNCTION__);

	psError.append(L" in line %d");

	iMessageLength = psError.length() + 10;

	cMessage = (wchar_t*)malloc(iMessageLength*2);

	swprintf_s(cMessage, iMessageLength, psError.c_str(), piLine);
	sMessage = cMessage;

	free (cMessage);

	sErrorMessages.push_back(sMessage);

	oTrace->EndTrace(__WFUNCTION__);
	}
