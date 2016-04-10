#include "StdAfx.h"

#include "CopyingMachineCore.h"
#include <cctype>
#include <string>
#include <algorithm>

CScriptLine::CScriptLine(CTracer* poTrace)
{
	oTrace = poTrace;

	oTrace->StartTrace(__WFUNCTION__);

	cStatement = eScriptUnknown;
	iJumpToLine = 0;

	oTrace->EndTrace(__WFUNCTION__);
	}

CScriptLine::~CScriptLine()
{
	oTrace->StartTrace(__WFUNCTION__);


	oTrace->EndTrace(__WFUNCTION__);
	}

bool 
CScriptLine::AddStatement(eScriptKeyword pcKeyword) {

	bool bResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	switch (pcKeyword) {

		case eScriptBegin:
		case eScriptEnd:
		case eScriptSet:
		case eScriptComment:
		case eScriptMessage:
		case eScriptLet:
		case eScriptConst:
		case eScriptElse:
		case eScriptExit:
		case eScriptFunction:
		case eScriptRepeat:
		case eScriptUntil:
		
			cStatement = pcKeyword;
			break;

		default:

			cStatement = eScriptUnknown;
			break;
		};

	bResult = (cStatement != eScriptUnknown);

	if (!bResult) {

		sError = L"Unknown command";
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}


//This function adds a function to the line
bool 
CScriptLine::AddFunction(eScriptKeyword pcKeyword) {

	bool bResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	switch (pcKeyword) {

		case eScriptPrint:
		case eScriptPage:
		case eScriptDocument:
		case eScriptScan:
		case eScriptCopy:
		case eScriptIf:
		case eScriptElse:
		case eScriptWhile:
		
			cFunction = pcKeyword;
			break;

		default:

			cFunction = eScriptUnknown;
			break;
		};

	bResult = (cFunction != eScriptUnknown || cStatement==eScriptRepeat);

	if (!bResult) {

		sError = L"Unknown function '";
		sError += GetKeywordString(pcKeyword);
		sError += L"'";
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function adds a function to the line
bool 
CScriptLine::AddAttribute(eScriptKeyword pcKeyword, eScriptKeyword pcContext) {

	bool bResult;
	int iCombinations;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	iCombinations = sizeof(aFunctionAttributes) / sizeof(FunctionAttributeCombination);

	while (iCombinations > 0 && !bResult) {

		iCombinations--;
		bResult = ((aFunctionAttributes[iCombinations].cFunction  == pcContext &&
					aFunctionAttributes[iCombinations].cAttribute == pcKeyword));
		}

	if (bResult) {

		cAttribute = pcKeyword;
		}
	else {

		sError = L"Unknown attribute '";
		sError += GetKeywordString(pcKeyword);
		sError += L"' for function '";
		sError += GetKeywordString(pcContext);
		sError += L"'";
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function translate the keyword back to a string
std::wstring 
CScriptLine::GetKeywordString(eScriptKeyword pcKeyword) {

	std::wstring sResult;
	int iKeywords;

	sResult = L"";

	//enumerate through the keywords
	iKeywords = sizeof(aKeywordStrings) / sizeof(KeywordStringCombination);

	while (iKeywords > 0 && sResult==L"") {

		iKeywords--;
		if (aKeywordStrings[iKeywords].cKeyword == pcKeyword) {

			sResult = aKeywordStrings[iKeywords].sString;
			}
		}

	return sResult;
	}


