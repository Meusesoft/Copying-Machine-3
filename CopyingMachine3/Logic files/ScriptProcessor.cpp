#include "StdAfx.h"
#include "ScriptProcessorKeywords.h"
#include "ScriptExpression.h"
#include "ScriptObject.h"
#include "Script.h"
#include "CopyingMachineCore.h"
#include <cctype>
#include <string>
#include <algorithm>

 
CScriptProcessor::CScriptProcessor(sGlobalInstances pcGlobalInstances)
{
	oTrace = pcGlobalInstances.oTrace;

	oTrace->StartTrace(__WFUNCTION__);

	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pcGlobalInstances, sizeof(sGlobalInstances));

	//initialise variables
	oScriptBeingExecuted = NULL;

	oTrace->EndTrace(__WFUNCTION__);
}

CScriptProcessor::~CScriptProcessor()
{
   oTrace->StartTrace(__WFUNCTION__);

   //Clear all scripts
   while (oScriptQueue.size()>0) {

	   delete oScriptQueue[0];
	   oScriptQueue.erase(oScriptQueue.begin());
	   }

   oTrace->EndTrace(__WFUNCTION__);
	}


//This function adds an error message to the message queue
void
CScriptProcessor::AddErrorMessage(std::wstring psError, int piLine) {

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

//This function parses the script and add it to the queue
bool 
CScriptProcessor::Process(std::wstring psScript) {

	bool bResult;
	CScript* oScript;

	oTrace->StartTrace(__WFUNCTION__);

	//Parse the script
	oScript = new CScriptCopyingMachine(oGlobalInstances);
	
	bResult = oScript->Parse(psScript);

	//Add the script to the queue
	if (bResult) {

		oScriptQueue.push_back(oScript);
		}
	else {

		delete oScript;
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function parses the script and add it to the queue
bool 
CScriptProcessor::Process(char* pcScript) {

	bool bResult;
	CScript* oScript;

	oTrace->StartTrace(__WFUNCTION__);

	//Parse the script
	oScript = new CScriptCopyingMachine(oGlobalInstances);

	bResult = oScript->Parse(pcScript);

	//Add the script to the queue
	if (bResult) {

		oScriptQueue.push_back(oScript);
		}
	else {

		delete oScript;
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}


//This function executes the next command in the script
void 
CScriptProcessor::ExecuteNextScriptLine() {

	oTrace->StartTrace(__WFUNCTION__);
	CScript* oScript;

	//If there isn't a script being executed, get the first script in 
	//the queue if present.
	while ((oScriptBeingExecuted==NULL) && (oScriptQueue.size()>0)) {

		if (oScriptQueue.size() > 0) {

			oScriptBeingExecuted = oScriptQueue[0];
			}

		oScript = (CScript*)oScriptBeingExecuted;

		if (!oScript->GoToNextCommand()) {

			//End of script is reached. Delete it
			oScript = (CScript*)oScriptQueue[0];
			delete oScript;
			oScriptQueue.erase(oScriptQueue.begin());

			oScriptBeingExecuted = NULL;
			}
		}

	bool bContinueExecuting;
	bool bEndOfScript;

	bContinueExecuting = true;
	bEndOfScript = false;

	if (oScriptBeingExecuted!=NULL) {

		oScript = (CScript*)oScriptBeingExecuted;
		if (!oScript->Execute(bContinueExecuting, bEndOfScript)) {

			//Error message...
			MessageBox(NULL, oScript->sError.c_str(), L"Error", MB_OK);
			}

		if (bEndOfScript) {

			//End of script is reached. Delete it
			oScript = (CScript*)oScriptQueue[0];
			delete oScript;
			oScriptQueue.erase(oScriptQueue.begin());

			oScriptBeingExecuted = NULL;
			}
		}

	oTrace->EndTrace(__WFUNCTION__);
	}
