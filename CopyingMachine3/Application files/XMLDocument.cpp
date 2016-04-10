#include "StdAfx.h"
#include "XMLDocument.h"

#import "msxml6.dll"
using namespace MSXML2;

CXMLDocument::CXMLDocument(void)
{
	try {

		oXMLDocument.CreateInstance(__uuidof(MSXML2::DOMDocument60));
		}
	catch(_com_error &e) {
			
		MessageBox(NULL, e.ErrorMessage(), L"XMLError", MB_OK);
		}
}

CXMLDocument::~CXMLDocument(void)
{
}

bool CXMLDocument::Save(WCHAR* cFile) {

	VARIANT vFilename;
	HRESULT HR;
	
	try {
	
		if (oXMLDocument==NULL) return false;

		VariantInit(&vFilename);
		V_VT(&vFilename) = VT_BSTR;
		V_BSTR(&vFilename) = SysAllocString(cFile);
	
		HR = oXMLDocument->save(vFilename);
	
		VariantClear(&vFilename);
		}
	catch(_com_error &e) {
			
		HR=e.Error();
		}

	return (HR==S_OK);	
	}

bool CXMLDocument::Load(WCHAR* cFile) {

	VARIANT vFilename;
	HRESULT HR;
	
	try {
	
		if (oXMLDocument==NULL) {
	
		
			if (S_OK!=oXMLDocument.CreateInstance(__uuidof(MSXML2::DOMDocument60))) return false;
			}

		VariantInit(&vFilename);
		V_VT(&vFilename) = VT_BSTR;
		V_BSTR(&vFilename) = SysAllocString(cFile);
	
		HR=oXMLDocument->load(vFilename);
	
		VariantClear(&vFilename);
		}
	catch(_com_error &e) {
			
		HR=e.Error();
		}

	return (HR==S_OK);	
	}

bool CXMLDocument::LoadXML(WCHAR* cXML) {

	HRESULT HR;
	
	try {
	
		if (oXMLDocument==NULL) {
	
			if (S_OK!=oXMLDocument.CreateInstance(__uuidof(MSXML2::DOMDocument60))) return false;
			}

		HR = oXMLDocument->loadXML(cXML);
		}
	catch(_com_error &e) {
			
		HR=e.Error();
		}

	return (HR==S_OK);	
	}


//This function returns the topnode

MSXML2::IXMLDOMNodePtr CXMLDocument::GetTopNode() {

	return oXMLDocument->GetfirstChild();
	}

//This function sets the current node

bool CXMLDocument::SetCurrentNode(WCHAR* cName) {

	oCurrentNode = SelectSingleNode(cName, eRoot);

	return oCurrentNode!=NULL;
}

bool CXMLDocument::SetCurrentNode(MSXML2::IXMLDOMNodePtr poNode) {

	if (poNode==NULL) return false;

	oCurrentNode = poNode;

	return true;
}


//This function select the single node with the given name and attribute

MSXML2::IXMLDOMNodePtr CXMLDocument::SelectSingleNode(WCHAR* pcName, WCHAR* pcAttribute, eSearchStartPosition pePosition) {

	WCHAR sQueryStatement[MAX_PATH];

	//fabricate the XPath search statement
	swprintf_s(sQueryStatement, MAX_PATH, L"%s[@%s]", pcName, pcAttribute);	

	return SelectSingleNode(sQueryStatement, pePosition);
	}

//This function select the single node with the given name, or the give XPATH
//querystring

MSXML2::IXMLDOMNodePtr CXMLDocument::SelectSingleNode(WCHAR* pcName, eSearchStartPosition pePosition) {

	MSXML2::IXMLDOMNodePtr		oStartNode;
	MSXML2::IXMLDOMNodePtr		oResultNode;
	BSTR						sQueryString;

	oResultNode = NULL;

	//set the start position node
	switch (pePosition) {

		case eRoot:

			oStartNode = oXMLDocument->GetfirstChild();
			break;

		default: // = eCurrent

			oStartNode = oCurrentNode;
			break;
			}	


	sQueryString = SysAllocString(pcName);

	//Do the action
	try {
		oResultNode = oStartNode->selectSingleNode(sQueryString);
		}
	catch(_com_error /*&e*/) {
			
		//MessageBox(NULL, e.ErrorMessage(), L"XMLError", MB_OK);
		oResultNode = NULL;
		}

	//clean up
	SysFreeString(sQueryString);

	//return value
	return oResultNode;
	}

//this function adds a new node to the document

MSXML2::IXMLDOMNodePtr CXMLDocument::AddNode(WCHAR* pcName) {

	MSXML2::IXMLDOMNodePtr oNewNode;
	MSXML2::IXMLDOMNodePtr oParentNode;
	VARIANT v;
	BSTR sName;

	oNewNode = NULL;

	oParentNode = oCurrentNode;
	if (oCurrentNode==NULL) {
		oParentNode = oXMLDocument;
		}

	sName = SysAllocString(pcName);

	//Do the action
	try {

		v.vt = VT_I4;
		V_I4(&v) = 1; // NODE_ELEMENT

		oNewNode = oXMLDocument->createElement(sName);
		
		oParentNode->appendChild(oNewNode);
		}
	catch(_com_error &e) {
			
		MessageBox(NULL, e.ErrorMessage(), L"XMLError", MB_OK);
		oNewNode = NULL;
		}

	SysFreeString(sName);

	return oNewNode;
}

bool CXMLDocument::SetAttribute(WCHAR* pcName, WCHAR* pcValue) {

	MSXML2::IXMLDOMAttributePtr pAttribute;
	VARIANT v;
	BSTR sName;
	BSTR sValue;
	bool bReturn;

	if (oCurrentNode==NULL) return false;

	bReturn = true;

	sName = SysAllocString(pcName);
	sValue = SysAllocString(pcValue);

	//Do the action
	try {

		v.vt = VT_BSTR;
		v.bstrVal = sValue;

		pAttribute = oXMLDocument->createAttribute(sName);
		pAttribute->put_nodeTypedValue(v);

		oCurrentNode->attributes->setNamedItem((MSXML2::IXMLDOMNode*)pAttribute);
		}
	catch(_com_error &e) {
			
		MessageBox(NULL, e.ErrorMessage(), L"XMLError", MB_OK);
		bReturn = false;
		}

	SysFreeString(sName);
	SysFreeString(sValue);

	return bReturn;
	}

bool CXMLDocument::GetAttribute(WCHAR* pcName, WCHAR* pcValue, long plValueSize) {

	MSXML2::IXMLDOMNodePtr pAttribute;
	VARIANT v;
	BSTR sName;

	if (oCurrentNode==NULL) return false;

	try {
		//get the attribute
		sName = SysAllocString(pcName);

		pAttribute = oCurrentNode->attributes->getNamedItem(sName);

		SysFreeString(sName);

		if (pAttribute==NULL) return false; // no attribute with this name was found

		//retrieve the value of the attribute
		v.vt = VT_BSTR;

		pAttribute->get_nodeTypedValue(&v);

		//convert the BSTR to the WHCAR*
		swprintf_s(pcValue, plValueSize, L"%s", v.bstrVal); 
		}
	catch(_com_error &e) {
			
		MessageBox(NULL, e.ErrorMessage(), L"XML Error", MB_OK);
		return false;
		}

	return true;
	}
