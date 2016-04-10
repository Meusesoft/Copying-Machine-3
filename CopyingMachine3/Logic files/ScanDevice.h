#pragma once
#include "ScanStructures.h"

class CScanDevice
{
public:
	CScanDevice(sGlobalInstances pInstances);
	~CScanDevice(void);

	void Initialize();

	//name scanning device
	std::wstring cScanningDevice;

	//id scanning device
	std::wstring cScanningDeviceID; 

	//interface of the scanning device
	eScanInterface cInterface;

	bool bCapabilitiesRetrieved;

	//feeder
	bool bFeederPresent;			//Is er een document feeder aanwezig?

	//brightness
	bool bBrightness;				//kan de brightness gezet worden J/N
	int iBrightnessContainerType;	//het type container voor TWAIN communicatie
	int iBrightnessItemType;		//het type variable voor TWAIN communicatie
	int iBrightnessMin;	       		//minimale brightness waarde
	int iBrightnessMax;        		//maximale brightness waarde
	int iBrightnessStep;       		//maximale brightness waarde

	//contrast
	bool bContrast;			   		//kan het contrast ingesteld worden J/N
	int iContrastContainerType;		//het type container voor TWAIN communicatie
	int iContrastItemType;			//het type variable voor TWAIN communicatie
	int iContrastMin;		   		//minimale contrast waarde
	int iContrastMax;		   		//maximale contrast waarde
	int iContrastStep;		   		//maximale contrast waarde

	//page size
	bool bPage;						//kan de pagina ingesteld worden J/N
	unsigned int iPageHeight;
	unsigned int iPageWidth;

	//orientation
	bool bOrientation;		   		//kan de orientatie van de pagina ingesteld worden J/N
	int iOrientationContainerType;	//het type container voor TWAIN communicatie
	int iOrientationItemType;		//het type variable voor TWAIN communicatie

	//color depth
	bool bColorDepth;				//kan de kleurdiepte ingesteld worden J/N
	int iColorDepthContainerType;	//het type container voor TWAIN communicatie
	int iColorDepthItemType;		//het type variable voor TWAIN communicatie
	bool bBlackWhite;				//is zwart wit mogelijk? J/N
	bool bGray;						//is grijs mogelijk? J/N
	bool bColor;					//is kleur mogelijk? J/N

	//resolution
	bool bResolution;				//kan de resolutie worden ingesteld J/N
	int iResolutionContainerType;	//het type container voor TWAIN communicatie
	int iResolutionItemType;		//het type variable voor TWAIN communicatie
	bool bResolution75;				//is 75 dpi mogelijk? J/N
	bool bResolution100;			//is 100 dpi mogelijk? J/N
	bool bResolution150;			//is 150 dpi mogelijk? J/N
	bool bResolution200;			//is 200 dpi mogelijk? J/N
	bool bResolution300;			//is 300 dpi mogelijk? J/N
	bool bResolution400;			//is 400 dpi mogelijk? J/N
	bool bResolution600;			//is 600 dpi mogelijk? J/N
	bool bResolution1200;			//is 1200 dpi mogelijk? J/N

	unsigned int iResolutionNominaal;

private:

	CTracer* oTrace;
	CRegistry* oRegistry;


};
