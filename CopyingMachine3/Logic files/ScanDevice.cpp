#include "StdAfx.h"
#include "CopyingMachineCore.h"

CScanDevice::CScanDevice(sGlobalInstances pInstances)
{
	//Set helper classes
	oTrace = pInstances.oTrace;
	oRegistry = pInstances.oRegistry;

	bCapabilitiesRetrieved = false;

	Initialize();
}

CScanDevice::~CScanDevice(void)
{
	oTrace->StartTrace(__WFUNCTION__);
	oTrace->EndTrace(__WFUNCTION__);

}

//Initializes the scan device capabilities
void 
CScanDevice::Initialize() {

	if (!bCapabilitiesRetrieved) {

		bResolution = false;
		bResolution75 = false;
		bResolution150 = false;
		bResolution200 = false;
		bResolution300 = false;
		bResolution400 = false;
		bResolution600 = false;
		bResolution1200 = false;
		bColorDepth = false;
		bBlackWhite = false;
		bGray = false;
		bColor = false;
		bFeederPresent = false;
		bPage = false;
		bOrientation = false;
		bBrightness = false;
		bContrast = false;
		}
	}
