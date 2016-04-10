#include "StdAfx.h"
#include "CopyingMachineCore.h"

CInterfaceBase::CInterfaceBase(sGlobalInstances pcGlobalInstances)
{
	oTrace = pcGlobalInstances.oTrace;

	oTrace->StartTrace(__WFUNCTION__);

	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pcGlobalInstances, sizeof(sGlobalInstances));

	oRegistry = pcGlobalInstances.oRegistry;

	oTrace->EndTrace(__WFUNCTION__);
}

CInterfaceBase::~CInterfaceBase()
{
   oTrace->StartTrace(__WFUNCTION__);

   oTrace->EndTrace(__WFUNCTION__);
	}

void 
CInterfaceBase::AddErrorMessage(int piResourceID, int piStatus, bool pbCritical) {

	cErrorMessage cNewMessage;

	cNewMessage.bCritical	= pbCritical;
	cNewMessage.iStatus		= piStatus;
	cNewMessage.iMessageID	= piResourceID;

	oErrorMessages.push_back(cNewMessage);
	}
	
//This function returns the number of error messages
int 
CInterfaceBase::GetErrorMessageCount() {

	int iResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	iResult = (int)oErrorMessages.size();

	oTrace->EndTrace(__WFUNCTION__, iResult);

	return iResult;
	}

//This function returns the number of critical error message
int				
CInterfaceBase::GetCriticalErrorMessageCount() {

	int iResult;

	oTrace->StartTrace(__WFUNCTION__);

	iResult = 0;

	for (long lIndex=(long)oErrorMessages.size()-1; lIndex>=0; lIndex--) {
		
		if (oErrorMessages[lIndex].bCritical) iResult++;

		}

	oTrace->EndTrace(__WFUNCTION__, iResult);

	return iResult;
	}


//This function retrieves the last error message from the messages. If ClearAll is true then
//all messages in the vector will be deleted. Default for ClearAll is false.
cErrorMessage 
CInterfaceBase::GetLastErrorMessage(bool pbClearAll) {

	cErrorMessage cResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	cResult.bCritical = false;
	cResult.iMessageID = IDS_UNKNOWN;

	if (oErrorMessages.size()>0) {

		cResult = oErrorMessages[oErrorMessages.size()-1];
		oErrorMessages.erase(oErrorMessages.begin() + oErrorMessages.size()-1);
		}

	if (pbClearAll) oErrorMessages.clear();

	oTrace->EndTrace(__WFUNCTION__, cResult.iMessageID);

	return cResult;
	}

