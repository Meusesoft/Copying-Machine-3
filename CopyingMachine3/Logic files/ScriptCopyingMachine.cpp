#include "StdAfx.h"
#include "ScriptProcessor.h"
#include "CopyingMachineCore.h"

CScriptCopyingMachine::CScriptCopyingMachine(sGlobalInstances pcGlobalInstances) :
	CScript(pcGlobalInstances)
{
	RegisterChild((CScriptObject*)pcGlobalInstances.oCopyingMachineCore);

	Initialise();
	//init the variables.
	//oObjectModel = (CScriptObject*)pcGlobalInstances.oCopyingMachineCore;
}

CScriptCopyingMachine::~CScriptCopyingMachine()
{
	oTrace->StartTrace(__WFUNCTION__);

	oTrace->EndTrace(__WFUNCTION__);
	}

void 
CScriptCopyingMachine::Initialise() {

	std::wstring sInitialiseScript;
	bool bContinueExecuting, bEndOfScript;

	oTrace->StartTrace(__WFUNCTION__);

	CScript::Initialise();

	sInitialiseScript  = L"const ScanDocument 0\r\n";
	sInitialiseScript += L"const ScanPage 1\r\n";
	sInitialiseScript += L"const ScanEvenPages 2\r\n";
	sInitialiseScript += L"const ScanOddPages 3\r\n";
	sInitialiseScript += L"const AlignNW 0\r\n";
	sInitialiseScript += L"const AlignNC 1\r\n";
	sInitialiseScript += L"const AlignNE 2\r\n";
	sInitialiseScript += L"const AlignCW 3\r\n";
	sInitialiseScript += L"const AlignCC 4\r\n";
	sInitialiseScript += L"const AlignCE 5\r\n";
	sInitialiseScript += L"const AlignSW 6\r\n";
	sInitialiseScript += L"const AlignSC 7\r\n";
	sInitialiseScript += L"const AlignSE 8\r\n";

	Parse(sInitialiseScript);

	Execute(bContinueExecuting, bEndOfScript);

	delete oScript;
	CScript::Initialise();

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function adds the variables to an autocomplete list of keywords
void 
CScriptCopyingMachine::AddVariablesToAutoComplete(std::wstring psRoot, 
												  std::vector<std::wstring>& psElements) {

	mVariables::iterator VariablesIterator;
	std::wstring sVariableName;

   for( VariablesIterator = oVariables.begin(); VariablesIterator != oVariables.end(); ++VariablesIterator ) {

		sVariableName = VariablesIterator->first;

		if (sVariableName.find(psRoot)==0) psElements.push_back(sVariableName);
		}
	}

//This function returns the name of all variables. To be used in an editor so it can give these
//constants a color coding.
std::wstring 
CScriptCopyingMachine::GetVariablesToKeywords() {

	mVariables::iterator VariablesIterator;
	std::wstring sResult;

	sResult = L"";

   for( VariablesIterator = oVariables.begin(); VariablesIterator != oVariables.end(); ++VariablesIterator ) {

		sResult += VariablesIterator->first + L" ";
		}	

	return sResult;
	}



//Execute the current command
bool 
CScriptCopyingMachine::ExecuteCommand(bool &pbContinue) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);
	CCopyingMachineCore* oCMCore;


	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	bResult = false;

	if (oScriptPointer!=NULL) {

		switch (oScriptPointer->cStatement) {

			case eScriptBegin: 

				switch (oScriptPointer->cFunction) {

					case eScriptCopy:

						bResult = ExecuteCopy(oScriptPointer);
						pbContinue = false;
						break;

					case eScriptPrint:

						bResult = ExecutePrint(oScriptPointer);
						pbContinue = false;
						break;

					case eScriptScan:

						bResult = ExecuteScan(oScriptPointer);
						//pbContinue = !(oCMCore->oScanCore->ImagesAvailable()>0);
						pbContinue = true;
						break;

					case eScriptPage:

						bResult = ExecutePage(oScriptPointer);
						pbContinue = true;
						break;

					default:
						
						bResult = CScript::ExecuteCommand(pbContinue);
						break;
					}
				break;

			default:

				bResult = CScript::ExecuteCommand(pbContinue);
				break;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function executes the scan command
bool 
CScriptCopyingMachine::ExecuteScan(CScriptLine* poLine) {

	bool bResult;
	long iIndex;
	CCopyingMachineCore* oCMCore;
	CScanSettings* oScanSettings;

	oTrace->StartTrace(__WFUNCTION__);

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oScanSettings = oCMCore->oScanCore->GetDefaultScanSettings();
	oScanSettings->cAcquireType = eAcquireNewDocument;
	
	bResult = true;
	//MessageBox(NULL, L"Scan", L"Script", MB_OK);

	bool bInContext;
	sScriptExpression cValue;

	bInContext = true;
	
	do { 

		bInContext = GoToNextCommand();

		if (oScriptPointer->cStatement == eScriptEnd) bInContext = false;
		if (oScriptPointer->cStatement == eScriptSet) {

			cValue.init();

			EvaluateExpression(oScriptPointer->sExpression, cValue);

			switch (oScriptPointer->cAttribute) {
				
				case eScriptUserInterface:

					if (cValue.isBoolean()) {
						
						oScanSettings->SetBool(eShowInterface, cValue.bResult);
						}
					else {

						SetErrorMessage(L"Boolean expected for interface attribute");
						bResult = false;
						}
					break;

				case eScriptBrightness:

					if (cValue.isNumber()) {

						cValue.toInteger();
						oScanSettings->SetInt(eBrightnessValue, cValue.iResult);
						}
					else {

						SetErrorMessage(L"Numeric value expected for brightness attribute");
						bResult = false;
						}
					break;

				case eScriptContrast:

					if (cValue.isNumber()) {
						
						cValue.toInteger();
						oScanSettings->SetInt(eContrastValue, cValue.iResult);
						}
					else {

						SetErrorMessage(L"Numeric value expected for contrast attribute");
						bResult = false;
						}
					break;

				case eScriptResolution:

					if (cValue.isNumber()) {

						cValue.toInteger();
						oScanSettings->SetInt(eResolutionValue, cValue.iResult);
						}
					else {

						SetErrorMessage(L"Numeric value expected for resolution attribute");
						bResult = false;
						}
					break;

				case eScriptColor:

					if (cValue.isNumber()) {

						cValue.toInteger();
						switch (cValue.iResult) {

							case 0:	oScanSettings->SetInt(eColorDepthValue, 0); break;
							case 1:	oScanSettings->SetInt(eColorDepthValue, 1); break;
							default: oScanSettings->SetInt(eColorDepthValue, 2); break;

							}
						}
					else {

						SetErrorMessage(L"Numeric value expected for color attribute");
						bResult = false;
						}
					break;

				case eScriptScanType:

					if (cValue.isNumber()) {

						cValue.toInteger();
						oScanSettings->cAcquireType = (eAcquireType)cValue.iResult;
						}
					else {

						SetErrorMessage(L"Numeric value expected for scantype");
						bResult = false;
						}
					break;

				case eScriptCollection: 

					if (cValue.isString()) {

						iIndex=-1;

						if (!oCMCore->oDocumentCollections->GetDocumentCollectionIndex(cValue.sResult, iIndex)) {

							SetErrorMessage(L"Document collection does not exist");
							bResult = false;
							}
						
						cValue.iResult = iIndex;
						cValue.cValueType = eScriptValueInteger;
						}

					if (cValue.isNumber()) {

						cValue.toInteger();

						oScanSettings->iCollection = cValue.iResult;
						oScanSettings->cAcquireType = eAcquireNewDocumentToCollection;
						}
					else {

						SetErrorMessage(L"String value expected for collection attribute");
						bResult = false;
						}
					break;

				default:

					SetErrorMessage(L"Unknown scan attribute");
					bResult = false;
					break;
				}
			}
		} 
	while (bInContext);

	if (!bResult) MessageBox(NULL, sError.c_str(), L"Error", MB_OK);

	if (bResult) bResult = oCMCore->Acquire(oCMCore->hMainApplicationWindow, 
								  oScanSettings->cAcquireType, 
								  oScanSettings);
	if (bResult) oCMCore->TransferImages(oCMCore->hMainApplicationWindow);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function executes the print command
bool 
CScriptCopyingMachine::ExecutePrint(CScriptLine* poLine) {

	bool bResult;
	sOutputDevice cDevice;
	CCopyingMachineCore* oCMCore;
	CCopyDocument* poDocument;
	CPrintSettings* oTemporarySettings;

			
	oTrace->StartTrace(__WFUNCTION__);

	oCMCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	bResult = true;
	//MessageBox(NULL, L"Print", L"Script", MB_OK);

	oTemporarySettings = oCMCore->oPrintSettings->Copy();
	oTemporarySettings->bDeleteAfterPrint = false;
	oTemporarySettings->bNotifyChanges = false;
	oTemporarySettings->GetDevice(cDevice);

	bool bInContext;
	sScriptExpression cValue;

	bInContext = true;
	
	do { 

		bInContext = GoToNextCommand();

		if (oScriptPointer->cStatement == eScriptEnd) bInContext = false;
		if (oScriptPointer->cStatement == eScriptSet) {

			cValue.init();

			EvaluateExpression(oScriptPointer->sExpression, cValue);

			switch (oScriptPointer->cAttribute) {
				
				case eScriptUserInterface:

					if (cValue.isBoolean()) {
						
						oTemporarySettings->SetBool(ePrintShowInterface, cValue.bResult);
						}
					else {

						SetErrorMessage(L"Boolean expected for interface attribute");
						bResult = false;
						}
					break;

				case eScriptShrinkToFit:

					if (cValue.isBoolean()) {
						
						oTemporarySettings->SetBool(ePrintShrink, cValue.bResult);
						}
					else {

						SetErrorMessage(L"Boolean expected for shrink attribute");
						bResult = false;
						}
					break;

				case eScriptStretchToFit:

					if (cValue.isBoolean()) {
						
						oTemporarySettings->SetBool(ePrintStretch, cValue.bResult);
						}
					else {

						SetErrorMessage(L"Boolean expected for stretch attribute");
						bResult = false;
						}
					break;

				case eScriptMultiCopiesOnPage:

					if (cValue.isBoolean()) {
						
						oTemporarySettings->SetBool(ePrintMultipleCopiesOnPage, cValue.bResult);
						}
					else {

						SetErrorMessage(L"Boolean expected for MultiCopiesOnPage attribute");
						bResult = false;
						}
					break;

				case eScriptMultiPageOnPage:

					if (cValue.isBoolean()) {
						
						oTemporarySettings->SetBool(ePrintMultiPagesOnPage, cValue.bResult);
						}
					else {

						SetErrorMessage(L"Boolean expected for MultiPageOnPage attribute");
						bResult = false;
						}
					break;

				case eScriptMagnification:

					if (cValue.isNumber()) {

						cValue.toInteger();
						oTemporarySettings->SetInt(ePrintMagnification, cValue.iResult);
						}
					else {

						SetErrorMessage(L"Numeric value expected for magnification attribute");
						bResult = false;
						}
					break;

				case eScriptCopies:

					if (cValue.isNumber()) {

						cValue.toInteger();
						oTemporarySettings->SetInt(ePrintCopies, cValue.iResult);
						}
					else {

						SetErrorMessage(L"Numeric value expected for copies attribute");
						bResult = false;
						}
					break;

				case eScriptAlignment:

					if (cValue.isNumber()) {

						cValue.toInteger();
						oTemporarySettings->SetInt(ePrintAlignment, cValue.iResult);
						}
					else {

						SetErrorMessage(L"Numeric value expected for alignment attribute");
						bResult = false;
						}
					break;

				case eScriptDevice:


					if (cValue.isString()) {

						cDevice.sDescription = cValue.sResult;
						cDevice.cType = eOutputPrinter;

						if (cDevice.sDescription == L"E-mail") cDevice.cType = eOutputMail;
						if (cDevice.sDescription == L"File") cDevice.cType = eOutputFile;

						bResult = oTemporarySettings->SetDevice(cDevice);
						
						if (!bResult) {

							SetErrorMessage(L"Unknown print device");
							}						
						}
					else {

						SetErrorMessage(L"String expected for device attribute");
						bResult = false;
						}
					break;

				default:

					SetErrorMessage(L"Unknown print attribute");
					bResult = false;
					break;
				}
			}
		} 
	while (bInContext);

	if (!bResult) MessageBox(oCMCore->hMainApplicationWindow, sError.c_str(), L"Error", MB_OK);

	if (bResult) {

		poDocument = oCMCore->GetCurrentDocument();

		if (poDocument) {

			//Do the output
			switch (cDevice.cType) {

				case eOutputFile:

					oCMCore->SaveDocument(oCMCore->hMainApplicationWindow, poDocument);
					break;

				case eOutputMail:

					poDocument->MailTo();
					break;

				default: 

					CDlgProgressPrint* oDlgProgress;
					oDlgProgress = oCMCore->oDialogs->DlgProgressPrint(oCMCore->hMainApplicationWindow, poDocument, oTemporarySettings);

					delete oDlgProgress;
					break;
					}
				}
			else {

				SetErrorMessage(L"No document to print");
				bResult = false;
				}
			}

	delete oTemporarySettings;

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function executes a copy script command
bool 
CScriptCopyingMachine::ExecuteCopy(CScriptLine* poLine) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;
	MessageBox(NULL, L"Copy", L"Script", MB_OK);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function executes a page script command
bool 
CScriptCopyingMachine::ExecutePage(CScriptLine* poLine) {

	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = true;
	MessageBox(NULL, L"Page", L"Script", MB_OK);

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

