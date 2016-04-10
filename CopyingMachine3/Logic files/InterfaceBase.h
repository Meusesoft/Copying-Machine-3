#pragma once
#include "ScanStructures.h"

class CInterfaceBase {

public:
	CInterfaceBase(sGlobalInstances pInstances);
	~CInterfaceBase();

	cErrorMessage	GetLastErrorMessage(bool pbClearAll = false);
	int				GetErrorMessageCount();
	int				GetCriticalErrorMessageCount();

protected:

	void AddErrorMessage(int piResourceID, int piStatus, bool bCritical);

	std::vector<cErrorMessage> oErrorMessages;

	CTracer*	oTrace;
	CRegistry*	oRegistry;
	sGlobalInstances oGlobalInstances;

private:

	};

