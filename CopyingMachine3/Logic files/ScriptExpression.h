#pragma once

struct sScriptExpression {

	eScriptValueType cValueType;
	bool			 bResult;
	int				 iResult;
	std::wstring	 sResult;
	float			 fResult;
	bool			 bConstant;
	void*			 pPointer;

	void init ();
	bool isNumber();
	bool isInteger();
	bool isFloat();
	bool isBoolean();
	bool isString();
	bool isObject();
	float GetNumber();

	bool toString();
	bool toBoolean();
	bool toInteger();
	bool toFloat();

	int  Compare(sScriptExpression pcValue);
};


