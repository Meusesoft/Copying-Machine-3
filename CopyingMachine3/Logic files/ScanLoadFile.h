#pragma once

class CScanLoadFile : 
	public CScanInterfaceBase
{
public:
	CScanLoadFile(sGlobalInstances pInstances, CScanSettings* poSettings);
	~CScanLoadFile(void);

	eAcquireResult AcquireImages(CScanSettings* poSettings);


private:

	CTracer* oTrace;
	};
