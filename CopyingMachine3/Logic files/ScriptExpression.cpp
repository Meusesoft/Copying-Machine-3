#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include <string>
#include <algorithm>

//This function initializes the strcuture
void
sScriptExpression::init() {

	cValueType = eScriptValueAny;
	bResult = false;
	bConstant = false;
	iResult = 0;
	sResult = L"";
	fResult = 0.0f;
	pPointer = NULL;
	}

//This function returns true if the structure contains a number
bool 
sScriptExpression::isNumber() {
		
	return (cValueType==eScriptValueFloat || cValueType==eScriptValueInteger);
	}

//This function returns true if the structure contains an integer
bool 
sScriptExpression::isInteger() {
		
	return (cValueType==eScriptValueInteger);
	}

//This function returns true if the structure contains a float
bool 
sScriptExpression::isFloat() {
		
	return (cValueType==eScriptValueFloat);
	}

//This function returns true if the structure contains a boolean
bool 
sScriptExpression::isBoolean() {
		
	return (cValueType==eScriptValueBoolean);
	}

//This function returns true if the structure contains an object
bool 
sScriptExpression::isObject() {
		
	return (cValueType==eScriptValuePointer);
	}

//This function returns true if the structure contains a string
bool 
sScriptExpression::isString() {
		
	return (cValueType==eScriptValueString);
	}

//This function returns the numeric value the structure contains
float 
sScriptExpression::GetNumber() {

	switch (cValueType) {

		case eScriptValueInteger:
			return (float)iResult;
			break;

		case eScriptValueFloat:
		default:
			return fResult;
			break;
		}
	}

//This function converts the contained value to a string, if possible
bool 
sScriptExpression::toString() {
	
	bool bReturn;
	wchar_t* cNumericBuffer;

	if (bConstant) return false;

	bReturn = true;

	switch (cValueType) {

		case eScriptValueFloat:

			cNumericBuffer = (wchar_t*)malloc(80);

			swprintf_s(cNumericBuffer, 80, L"%f", fResult);
			sResult = cNumericBuffer;
			
			free (cNumericBuffer);
			break;

		case eScriptValueInteger:

			cNumericBuffer = (wchar_t*)malloc(80);

			swprintf_s(cNumericBuffer, 80, L"%d", iResult);
			sResult = cNumericBuffer;
			
			free (cNumericBuffer);
			break;

		case eScriptValueBoolean:

			sResult = bResult ? ScrYes : ScrNo;
			break;
		
		case eScriptValueString:

			break;

		default:

			bReturn = false;
			break;
		}

	
	if (bReturn) {
		//Conversion successful	
		iResult = 0;
		fResult = 0;
		bResult = false;
		cValueType = eScriptValueString;
		bResult = false;
		}	

	return bReturn;
}

//This function converts the contained value to a boolean, if possible
bool 
sScriptExpression::toBoolean() {
	
	bool bReturn;

	if (bConstant) return false;

	switch (cValueType) {

		case eScriptValueFloat:

			bResult = (fResult!=0.0f);
			break;

		case eScriptValueInteger:

			bResult = (iResult!=0);
			break;

		case eScriptValueString:

			bReturn = false;
			
			if (sResult == ScrYes) {
				bResult = true;
				bReturn = true;
				}
			if (sResult == ScrNo) {
				bResult = true;
				bReturn = true;
				}
			break;
		
		default:

			bReturn = false;
			break;
		}

	
	if (bReturn) {
		//Conversion successful	
		iResult = 0;
		fResult = 0;
		sResult = L"";
		}	

	return bReturn;
	}

//This function converts the contained value to an integer, if possible
bool 
sScriptExpression::toInteger() {

	bool bReturn;

	if (bConstant) return false;

	bReturn = false;

	if (isFloat()) {

		iResult = (int)fResult;
		fResult = 0;
		cValueType = eScriptValueInteger;

		bReturn = true;
		}

	return bReturn;
}

//This function converts the contained value to a float, if possible
bool 
sScriptExpression::toFloat() {

	bool bReturn;

	if (bConstant) return false;

	bReturn = false;

	if (isInteger()) {

		fResult = (float)iResult;
		iResult = 0;
		cValueType = eScriptValueFloat;

		bReturn = true;
		}

	return bReturn;
	}

//This function compares the current value with the given parameter/
// <0 : smaller
//  0 : equals
// >0 : greater
int  
sScriptExpression::Compare(sScriptExpression pcValue) {

	int iCompareResult;

	iCompareResult = 0;

	if (pcValue.cValueType == cValueType) {

		switch (cValueType) {

			case eScriptValueInteger:

				iCompareResult = iResult - pcValue.iResult;
				break;

			case eScriptValueBoolean:

				iCompareResult = (bResult == pcValue.bResult ? 0 : 1);
				break;

			case eScriptValueFloat:

				iCompareResult = (int)(fResult - pcValue.fResult);
				break;

			case eScriptValueString:

				iCompareResult = sResult.compare(pcValue.sResult);
				break;

			//Dunno what to do
			default:

				iCompareResult = -1;				
				break;
			};
		}

	return iCompareResult;
	}

