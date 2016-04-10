#pragma once

enum eScanInterface {eWIA, eTWAIN, eInputFile};

enum eAcquireResult {eAcquireOk, eAcquireLowMemory, eAcquireFail, eAcquireFeederEmpty, eAcquireFailInvalidArgument, eAcquireFailSeeMessages, eAcquireFailTryTWAIN, eAcquireCancelled};

enum eScanSetting {eBrightnessNegotiate, eBrightnessUse, eBrightnessValue,
		eContrastNegotiate, eContrastUse, eContrastValue, ePageNegotiate,
		ePageUse, ePageValue, eOrientationNegotiate, eOrientationUse, eOrientationValue,
		eColorDepthNegotiate, eColorDepthUse, eColorDepthValue, eColorInvert, 
		eResolutionNegotiate, eResolutionUse, eResolutionValue,
		eShowErrorMessage, eAlwaysShowInterface, eAskScanNextPage, eInvertBlackWhite,
		eAssumeDPI, eShowInterface, eAcquireTypeSetting,eUseFeeder};

enum eAcquireType { eAcquireNewDocument = 0, 
					eAcquireNewPage = 1, 
					eAcquireNewPageAndInsert = 2, 
					eAcquireOddPages = 3, 
					eAcquireEvenPages = 4, 
					eAcquireNewDocumentToCollection = 5};

struct ImageCharacteristics {

	UINT imageWidth;		//Breedte van afbeelding in pixels
	UINT imageHeight;		//Hoogte van afbeelding in pixels
	UINT imageResolution;	//Resolutie van afbeelding in DPI
	UINT imageBitsPixel;	//Bits per pixel van afbeelding (1, 8, of 24)
	UINT imageTileWidth;    //Omvang van een tile
	bool imagePalette;		//Heeft de afbeelding een pallette

	HGLOBAL hImageHandle;		//Geheugenhandle naar afbeelding
	LPBITMAPINFO hImagePointer;	//Pointer start van afbeelding in geheugen
	};

struct cImageTransferStruct {

	HGLOBAL hImageHandle;
	int	  iDPIX;
	int	  iDPIY;
	std::wstring sImageFileName;
	};

struct cErrorMessage {

	bool bCritical;
	int  iStatus;
	int  iMessageID;
};
