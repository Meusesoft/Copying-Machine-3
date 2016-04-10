#pragma once

#import "msxml6.dll"
using namespace MSXML2;

#define XNode MSXML2::IXMLDOMNodePtr
#define XDocument MSXML2::IXMLDOMDocumentPtr

enum eSearchStartPosition {eRoot, eCurrent};

class CXMLDocument
{
public:

	CXMLDocument(void);
	~CXMLDocument(void);

	bool Load(WCHAR* cFile);
	bool LoadXML(WCHAR* cXML);
	bool Save(WCHAR* cFile);

	MSXML2::IXMLDOMNodePtr AddNode(WCHAR* cName);
	MSXML2::IXMLDOMNodePtr GetTopNode();
	MSXML2::IXMLDOMNodePtr SelectSingleNode(WCHAR* cName, WCHAR* cAttribute, eSearchStartPosition ePosition);
	MSXML2::IXMLDOMNodePtr SelectSingleNode(WCHAR* cName, eSearchStartPosition ePosition);

	bool SetCurrentNode(WCHAR* cName);
	bool SetCurrentNode(MSXML2::IXMLDOMNodePtr poNode);
	bool RemoveNode(MSXML2::IXMLDOMNodePtr poNode);
	
	bool SetAttribute(WCHAR* cName, WCHAR* cValue);
	bool GetAttribute(WCHAR* cName, WCHAR* cValue, long plValueSize);

private:

	MSXML2::IXMLDOMDocumentPtr 	oXMLDocument;	
	MSXML2::IXMLDOMNodePtr		oTopNode;
	MSXML2::IXMLDOMNodePtr		oCurrentNode;
};
