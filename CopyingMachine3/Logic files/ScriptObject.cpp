#include "stdafx.h"
#include "CopyingMachineCore.h"
#include <cctype>
#include <string>
#include <algorithm>


CScriptObject::CScriptObject(std::wstring psName)
{
	sName = psName;

	//To lowercase
	transform(
		sName.begin(), sName.end(),
		sName.begin(),
		tolower); 

	RegisterAttribute(L"type", true);
	}

CScriptObject::~CScriptObject()
{

}

//This function retrieves the value of an attribute
sScriptExpression 
CScriptObject::Get(std::wstring psObjectname) {

	sScriptExpression oResult;
	int iPosition;
	std::wstring sChildName;
	std::wstring sNewObjectName;
	CScriptObject* oChild;

	oResult.init();

	//Find a dot
	iPosition = psObjectname.find_first_of(L".");

	if (iPosition>=0) {

		//there is a dot in the objectname, try to match the first
		//part to a child object
		sChildName = psObjectname.substr(0, iPosition);
		sNewObjectName = psObjectname.substr(iPosition+1);

		oChild = GetChildObject(sChildName);
		if (oChild) {
			
			oResult = oChild->Get(sNewObjectName);
			if (oResult.cValueType == eScriptValueAny) sError = oChild->sError;
			}
		}
	
	//return the type of this object
	if (psObjectname==L"type") {

		oResult.sResult = sName;
		oResult.cValueType = eScriptValueString;
		}

	return oResult;
	}

//This function executes the given object/function
bool 
CScriptObject::Execute(std::wstring psObjectname, sScriptExpression &poValue) {

	bool bResult;
	int iPosition;
	std::wstring sChildName;
	std::wstring sNewObjectName;
	CScriptObject* oChild;

	poValue.init();
	bResult = false;

	//Find a dot
	iPosition = psObjectname.find_first_of(L".");

	if (iPosition>=0) {

		//there is a dot in the objectname, try to match the first
		//part to a child object
		sChildName = psObjectname.substr(0, iPosition);
		sNewObjectName = psObjectname.substr(iPosition+1);

		oChild = GetChildObject(sChildName);
		if (oChild) {
			
			bResult = oChild->Execute(sNewObjectName, poValue);
			if (!bResult) sError = oChild->sError;
			}
		}

	return bResult;
	}

//This function sets the value of an attribute
bool 
CScriptObject::Set(std::wstring psObjectname, sScriptExpression psNewValue) {

	bool bResult;
	int iPosition;
	std::wstring sChildName;
	std::wstring sNewObjectName;
	CScriptObject* oChild;

	bResult = false;

	//Find a dot
	iPosition = psObjectname.find_first_of(L".");

	if (iPosition>=0) {

		//there is a dot in the objectname, try to match the first
		//part to a child object
		sChildName = psObjectname.substr(0, iPosition);
		sNewObjectName = psObjectname.substr(iPosition+1);

		oChild = GetChildObject(sChildName);
		if (oChild) {
			
			bResult = oChild->Set(sNewObjectName, psNewValue);
			}
		}

	return bResult;
	}

//This function retrieves what the given name is, an attribute or a function.
eScriptObjectType 
CScriptObject::GetType(std::wstring psObjectname) {

	eScriptObjectType cResult;
	int iPosition;
	std::wstring sChildName;
	std::wstring sNewObjectName;
	CScriptObject* oChild;

	cResult = eScriptObjectNone;

	//To lowercase
	transform(
		psObjectname.begin(), psObjectname.end(),
		psObjectname.begin(),
		tolower); 

	//Find a dot
	iPosition = psObjectname.find_first_of(L".");

	if (iPosition>=0) {

		//there is a dot in the objectname, try to match the first
		//part to a child object
		sChildName = psObjectname.substr(0, iPosition);
		sNewObjectName = psObjectname.substr(iPosition+1);

		oChild = GetChildObject(sChildName);
		if (oChild) {

			cResult = oChild->GetType(sNewObjectName);
			}
		}
	else {

		//no dot in the objectname. Try to match it to a function or attribute	
		cResult = QueryType(psObjectname);
		}

	return cResult;
	}

//This function test if the request object exists
bool 
CScriptObject::Test(std::wstring psObjectname) {

	return (GetType(psObjectname)!=eScriptObjectNone);
	}


//This function test if the request name is an attribute
bool 
CScriptObject::IsFunction(std::wstring psObjectname) {

	return (GetType(psObjectname)==eScriptObjectFunction);
	}

//This function test if the requested name is an attribute
bool 
CScriptObject::IsAttribute(std::wstring psObjectname) {

	return (GetType(psObjectname)==eScriptObjectAttribute);
	}

//This function (un)registers a child object
bool 
CScriptObject::RegisterChild(CScriptObject* poChild, bool pbRegister) {

	if (pbRegister) {

		oChildren.push_back(poChild);
		}
	else {


		}

	return true;
	}

//This function (un)registers an attribute of this object
bool 
CScriptObject::RegisterAttribute(std::wstring psAttribute, bool pbRegister) {

	if (pbRegister) {

		//To lowercase
		transform(
			psAttribute.begin(), psAttribute.end(),
			psAttribute.begin(),
			tolower); 

		oAttributes.push_back(psAttribute);
		}
	else {


		}

	return true;
	}

//This function (un)registers a function of this object
bool 
CScriptObject::RegisterFunction(std::wstring psFunction, bool pbRegister) {

	if (pbRegister) {

		//To lowercase
		transform(
			psFunction.begin(), psFunction.end(),
			psFunction.begin(),
			tolower); 

		oFunctions.push_back(psFunction);
		}
	else {


		}

	return true;
	}

//This function queries what the given name is, a child, an attribute
//or a function
eScriptObjectType 
CScriptObject::QueryType(std::wstring psObjectname) {

	eScriptObjectType cResult;
	long lChildren;
	long lAttributes;
	long lFunctions;	

	cResult = eScriptObjectNone;
	lChildren = (long)oChildren.size();

	//Is it a child of this object?
	while (cResult == eScriptObjectNone && lChildren > 0) {

		lChildren--;
		if (oChildren[lChildren]->sName == psObjectname) cResult = eScriptObjectChild;
		}

	//Is it an attribute of this object?
	lAttributes = (long)oAttributes.size();

	while (cResult == eScriptObjectNone && lAttributes > 0) {

		lAttributes--;
		if (oAttributes[lAttributes] == psObjectname) cResult = eScriptObjectAttribute;
		}

	//Is it a function of this object?
	lFunctions = (long)oFunctions.size();

	while (cResult == eScriptObjectNone && lFunctions > 0) {

		lFunctions--;
		if (oFunctions[lFunctions] == psObjectname) cResult = eScriptObjectFunction;
		}

	return cResult;
	}

//This function get the requested type
CScriptObject* 
CScriptObject::GetChildObject(std::wstring psObjectname) {

	CScriptObject* oResult;
	long lIndex;

	oResult = NULL;
	lIndex = (long)oChildren.size();

	while (lIndex > 0 && oResult==NULL) {
		
		lIndex--;

		if (oChildren[lIndex]->sName == psObjectname) 
			oResult = oChildren[lIndex];
		}


	return oResult;
	}

//This function appends the items of this script object to the autocomplete elements
bool 
CScriptObject::Autocomplete(std::wstring psRoot, std::vector<std::wstring>& psElements) {

	int iDotPosition;
	long lIndex;
	std::wstring sPartialRoot;
	std::wstring sElement;
	
	iDotPosition = psRoot.find(L".");

	if (iDotPosition==psRoot.npos) {

		//no dot in the root, just add the name of this object
		if (sName.find(psRoot)==0) psElements.push_back(sName);
		}
	else {

		sPartialRoot = psRoot.substr(0, iDotPosition);

		//check if the element before the dot is this object its name, 
		//and if so add its functions and attrbitues, or add its child elements
		if (sPartialRoot == sName) {

			sPartialRoot = psRoot.substr(iDotPosition+1, psRoot.length());
			iDotPosition = sPartialRoot.find(L".");

			//There is not dot anymore, add all functions and attributes
			if (iDotPosition==psRoot.npos) {

				//add all attributes
				lIndex = (long)oAttributes.size();

				while (lIndex > 0) {
					
					lIndex--;
					sElement = /*sName + L"." + */oAttributes[lIndex];
					if (oAttributes[lIndex].find(sPartialRoot)==0) psElements.push_back(oAttributes[lIndex]);
					}

				//add all functions
				lIndex = (long)oFunctions.size();

				while (lIndex > 0) {
					
					lIndex--;
					sElement = /*sName + L"." + */oFunctions[lIndex];
					if (oFunctions[lIndex].find(sPartialRoot)==0) psElements.push_back(sElement);
					}
				
				//add the children objects
				lIndex = (long)oChildren.size();

				while (lIndex > 0) {
					
					lIndex--;
					sElement = /*sName + L"." + */oChildren[lIndex]->sName;

					if (oChildren[lIndex]->sName.find(sPartialRoot)==0) psElements.push_back(sElement);
					}
				}
			else {

				//enumerate the children objects
				lIndex = (long)oChildren.size();

				while (lIndex > 0) {
					
					lIndex--;
					oChildren[lIndex]->Autocomplete(sPartialRoot, psElements);
					}
				}
			}
		}


	return true;
	}

