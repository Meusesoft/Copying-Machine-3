#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "math.h"

CScanInterfaceBase::CScanInterfaceBase(sGlobalInstances pcGlobalInstances, CScanSettings* poSettings) :
	CInterfaceBase(pcGlobalInstances)
{
	oSettings = NULL;

	oTrace->StartTrace(__WFUNCTION__);

	oSettings = poSettings;

	oTrace->EndTrace(__WFUNCTION__);
}

CScanInterfaceBase::~CScanInterfaceBase()
{
   oTrace->StartTrace(__WFUNCTION__);

   oTrace->EndTrace(__WFUNCTION__);
	}

//This function returns the number of images available
int
CScanInterfaceBase::ImagesAvailable() {

    return ((int)oImages.size());
    }

//This function retrieves an image from the vector
cImageTransferStruct
CScanInterfaceBase::GetImage() {

    oTrace->StartTrace(__WFUNCTION__);

	cImageTransferStruct sRetval;

    sRetval.hImageHandle = NULL;

	if (oImages.size()>0) {

        sRetval.sImageFileName = oImages[0].sImageFileName;
        sRetval.hImageHandle = oImages[0].hImageHandle;
		sRetval.iDPIX = oImages[0].iDPIX;
		sRetval.iDPIY = oImages[0].iDPIY;

		//verwijder de image
		oImages.erase(oImages.begin());
      }

    oTrace->EndTrace(__WFUNCTION__, (bool)(oImages.size()>0));

    return sRetval;
    }

//This function reads the horizontal resolution from the scanned image
int 
CScanInterfaceBase::GetResolutionFromImage(HGLOBAL phHandle) {

	int iResult;
	BITMAPINFO far* Info;

	Info = (LPBITMAPINFO)::GlobalLock(phHandle);
	iResult = 0;

	//Read the resolution from the image;
	if (Info->bmiHeader.biSize == sizeof(BITMAPINFOHEADER))
		{
		iResult = (int)(((Info->bmiHeader.biXPelsPerMeter * 2.54 ) / 100) + 0.5f);
		}
	::GlobalUnlock(phHandle);

	return iResult;
	}

