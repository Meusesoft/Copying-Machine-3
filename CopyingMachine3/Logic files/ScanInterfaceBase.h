#pragma once
#include "ScanSettings.h"
#include "ScanStructures.h"

class CScanInterfaceBase : 
	public CInterfaceBase {

public:
	CScanInterfaceBase(sGlobalInstances pInstances, CScanSettings* poSettings);
	~CScanInterfaceBase();

	//cErrorMessage	GetLastErrorMessage(bool pbClearAll = false);
	//int				GetErrorMessageCount();
	//int				GetCriticalErrorMessageCount();

	int ImagesAvailable();
	cImageTransferStruct GetImage();

	CScanSettings* oSettings;

protected:

	//void AddErrorMessage(int piResourceID, bool bCritical);


	std::vector<cImageTransferStruct> oImages;
	int GetResolutionFromImage(HGLOBAL phHandle);
	//std::vector<cErrorMessage> oErrorMessages;

	//CTracer*	oTrace;
	//CRegistry*	oRegistry;
	//sGlobalInstances oGlobalInstances;

private:

	};

