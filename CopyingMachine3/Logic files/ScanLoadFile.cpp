#include "StdAfx.h"
#include "CopyingMachineCore.h"

CScanLoadFile::CScanLoadFile(sGlobalInstances pcGlobalInstances, CScanSettings* poSettings):
CScanInterfaceBase(pcGlobalInstances, poSettings) 
{


}

CScanLoadFile::~CScanLoadFile(void)
{
}

//-----------------------------------------------------------------------------
// Public Functions
//-----------------------------------------------------------------------------

eAcquireResult
CScanLoadFile::AcquireImages(CScanSettings* poSettings) {

	eAcquireResult retval = eAcquireFail;

	oTrace->StartTrace(__WFUNCTION__);

    oTrace->EndTrace(__WFUNCTION__, (bool)(retval==eAcquireOk));

    return retval;
    }

