#pragma once

//Generic keywords
#define ScrBegin			L"begin"
#define ScrEnd				L"end"
#define ScrSet				L"set"
#define ScrLet				L"let"
#define ScrYes				L"yes"
#define ScrNo				L"no"
#define ScrComment			L"rem"
#define ScrCommentShort		L"'"
#define ScrMessage			L"message"
#define ScrExit				L"exit"
#define ScrConst			L"const"
#define ScrIf				L"if"
#define ScrElse				L"else"
#define ScrRepeat			L"repeat"
#define ScrWhile			L"while"
#define ScrUntil			L"until"

//Copying Machine specific
#define ScrScan				L"scan"
#define ScrCopy				L"copy"
#define ScrPrint			L"print"
#define ScrBrightness		L"brightness"
#define ScrContrast			L"contrast"
#define ScrColor			L"color"
#define ScrInterface		L"interface"
#define ScrResolution		L"resolution"
#define ScrCollection		L"collection"
#define ScrScanType			L"scantype"
#define ScrDevice			L"device"
#define ScrCopies			L"copies"
#define ScrMultiPage		L"multipage"
#define ScrShrinkToFit		L"shrinktofit"
#define ScrStretchToFit		L"stretchtofit"
#define ScrMultiPageOnPage  L"multipageonpage"
#define ScrMultiCopiesOnPage L"multicopiesonpage"
#define ScrMagnification    L"magnification"
#define ScrAlignment		L"alignment"
#define ScrClose			L"close"

enum eScriptKeyword {eScriptUnknown, eScriptEmpty,
					 eScriptBegin, eScriptEnd, eScriptSet, eScriptComment, eScriptMessage, eScriptExit, eScriptOpen, eScriptLet, eScriptConst, eScriptFunction, eScriptElse, eScriptRepeat, eScriptWhile, eScriptUntil, //statements
					 eScriptUserInterface, eScriptDevice, //attributes general
					 eScriptContrast, eScriptBrightness, eScriptColor, eScriptResolution, eScriptCollection, eScriptScanType, //attributes scan
					 eScriptCopies, eScriptShrinkToFit, eScriptStretchToFit, eScriptMultiPageOnPage, eScriptMultiCopiesOnPage, eScriptAlignment, eScriptMagnification, eScriptDeleteAfterPrint, //attributes print
					 eScriptScan, eScriptPrint, eScriptCopy, eScriptIf, eScriptPage, eScriptDocument}; //actions

enum eScriptKeywordType {eScriptKeywordStatement, eScriptKeywordAttribute, eScriptKeywordAction, eScriptKeywordConst};

enum eScriptValueType {	eScriptValueBoolean, eScriptValueInteger, 
						eScriptValueFloat, eScriptValueString,
						eScriptValueAny, eScriptValuePointer};

enum eScriptExpressionOperator {eScriptPlus, eScriptMinus, 
								eScriptDivide, eScriptMultiply,
								eScriptConcatenate, 
								eScriptLarger, eScriptSmaller,
								eScriptLargerOrSame, eScriptSmallerOrSame,
								eScriptEquals, eScriptNotEqual,
								eScriptNone};

enum eScriptObjectType {eScriptObjectNone, eScriptObjectChild, 
						eScriptObjectAttribute, eScriptObjectFunction};

struct KeywordStringCombination {

	std::wstring sString;
	eScriptKeyword cKeyword;
	eScriptKeywordType eType;
	};

struct FunctionAttributeCombination {

	eScriptKeyword cFunction;
	eScriptKeyword cAttribute;
	};

const FunctionAttributeCombination aFunctionAttributes[] = {

	{eScriptScan, eScriptBrightness},
	{eScriptScan, eScriptContrast},
	{eScriptScan, eScriptUserInterface},
	{eScriptScan, eScriptResolution},
	{eScriptScan, eScriptColor},
	{eScriptScan, eScriptCollection},
	{eScriptScan, eScriptScanType},
	{eScriptScan, eScriptDevice},
	{eScriptPrint, eScriptDevice},
	{eScriptPrint, eScriptUserInterface},
	{eScriptPrint, eScriptCopies},
	{eScriptPrint, eScriptShrinkToFit},
	{eScriptPrint, eScriptStretchToFit},
	{eScriptPrint, eScriptMultiPageOnPage},
	{eScriptPrint, eScriptMultiCopiesOnPage},
	{eScriptPrint, eScriptAlignment},
	{eScriptPrint, eScriptMagnification},
	{eScriptPrint, eScriptDeleteAfterPrint},
};

const KeywordStringCombination aKeywordStrings[] = {

	{L"",					eScriptEmpty,				eScriptKeywordStatement},
	{ScrBegin,				eScriptBegin,				eScriptKeywordStatement},
	{ScrEnd,				eScriptEnd,					eScriptKeywordStatement},
	{ScrExit,				eScriptExit,				eScriptKeywordStatement},
	{ScrSet,				eScriptSet,					eScriptKeywordStatement},
	{ScrLet,				eScriptLet,					eScriptKeywordStatement},
	{ScrConst,				eScriptConst,				eScriptKeywordStatement},
	{ScrComment,			eScriptComment,				eScriptKeywordStatement},
	{ScrCommentShort,		eScriptComment,				eScriptKeywordStatement},
	{ScrContrast,			eScriptContrast,			eScriptKeywordAttribute},
	{ScrScanType,			eScriptScanType,			eScriptKeywordAttribute},
	{ScrInterface,			eScriptUserInterface,		eScriptKeywordAttribute},
	{ScrBrightness,			eScriptBrightness,			eScriptKeywordAttribute},
	{ScrResolution,			eScriptResolution,			eScriptKeywordAttribute},
	{ScrCollection,			eScriptCollection,			eScriptKeywordAttribute},
	{ScrColor,				eScriptColor,				eScriptKeywordAttribute},
	{ScrDevice,				eScriptDevice,				eScriptKeywordAttribute},
	{ScrCopies,				eScriptCopies,				eScriptKeywordAttribute},
	{ScrShrinkToFit,		eScriptShrinkToFit,			eScriptKeywordAttribute},
	{ScrStretchToFit,		eScriptStretchToFit,		eScriptKeywordAttribute},
	{ScrMultiPageOnPage,	eScriptMultiPageOnPage,		eScriptKeywordAttribute},
	{ScrMultiCopiesOnPage,	eScriptMultiCopiesOnPage,	eScriptKeywordAttribute},
	{ScrAlignment,			eScriptAlignment,			eScriptKeywordAttribute},
	{ScrMagnification,		eScriptMagnification,		eScriptKeywordAttribute},
	{ScrScan,				eScriptScan,				eScriptKeywordAction},
	{ScrPrint,				eScriptPrint,				eScriptKeywordAction},
	{ScrIf,					eScriptIf,					eScriptKeywordAction},
	{ScrElse,				eScriptElse,				eScriptKeywordStatement},
	{ScrRepeat,				eScriptRepeat,				eScriptKeywordStatement},
	{ScrWhile,				eScriptWhile,				eScriptKeywordAction},
	{ScrUntil,				eScriptUntil,				eScriptKeywordStatement},
	{ScrMessage,			eScriptMessage,				eScriptKeywordStatement}
	};