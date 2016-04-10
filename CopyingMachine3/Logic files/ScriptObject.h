#pragma once

#include <map>

typedef map<std::wstring, sScriptExpression> mVariables;

class CScriptObject {

public:
	CScriptObject(std::wstring psName);
	~CScriptObject();

	virtual sScriptExpression Get(std::wstring psObjectname);
	virtual bool Execute(std::wstring psObjectname, sScriptExpression &psValue);
	virtual bool Set(std::wstring psObjectname, sScriptExpression psNewValue);

	virtual eScriptObjectType GetType(std::wstring psObjectname);
	virtual bool Test(std::wstring psObjectname);
	virtual bool IsFunction(std::wstring psObjectname);
	virtual bool IsAttribute(std::wstring psObjectname);
	virtual bool Autocomplete(std::wstring psRoot, std::vector<std::wstring>& psElements);
	bool RegisterChild(CScriptObject* poChild, bool bRegister = true);
	bool RegisterAttribute(std::wstring psAttribute, bool bRegister = true);
	bool RegisterFunction(std::wstring psFunction, bool bRegister = true);
	
	
	//std::wstring Attributes();
	//std::wstring Children();

	std::wstring sName;
	std::wstring sError;

protected:

	mVariables oVariables;
	eScriptObjectType QueryType(std::wstring psObjectname);
	virtual CScriptObject* GetChildObject(std::wstring psObjectname);
	//eScriptKeyword GetKeyword(std::wstring psValue);

	vector <CScriptObject*> oChildren;
	vector <std::wstring> oAttributes;
	vector <std::wstring> oFunctions;

private:


	};

