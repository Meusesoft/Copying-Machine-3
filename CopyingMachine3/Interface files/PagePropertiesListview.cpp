#include "StdAfx.h"
#include "PagePropertiesListview.h"
#include "CopyingMachineCore.h"

CPagePropertiesListview::CPagePropertiesListview(sGlobalInstances pInstances, 
									std::wstring psWindowName) : 
	CListview(pInstances, psWindowName)
{
	oCurrentPage = NULL;
}

CPagePropertiesListview::~CPagePropertiesListview(void)
{

}


//This function creates the pagepropertieslistview its window
HWND 
CPagePropertiesListview::Create(HWND phParent, HINSTANCE phInstance, DWORD pdStyleExtra) {

	//Create the listview
	CListview::Create(phParent, phInstance, pdStyleExtra);

	//Add the columns
	SetView(LV_VIEW_DETAILS);
	InsertColumn(0, IDS_PROPERTY, 80);
	InsertColumn(1, IDS_VALUE, 90);

	//return the window handle
	return hWnd;
	}

//Localize
void 
CPagePropertiesListview::Localize(HINSTANCE phLanguage) {

	oGlobalInstances.hLanguage = phLanguage;

	ListView_DeleteAllItems(hWnd);
	SetProperties(oCurrentPage);

	SetColumnTitle(0, IDS_PROPERTY);
	SetColumnTitle(1, IDS_VALUE);
	}

//Clear the page listview
void 
CPagePropertiesListview::ClearProperties() {

	ListView_DeleteAllItems(hWnd);
	oCurrentPage = NULL;
	}

//Set the properties listview
void 
CPagePropertiesListview::SetProperties(CCopyDocumentPage* poPage) {

	int iIndex, iCount;

	if (poPage!=NULL) {

		oCurrentPage = poPage;
		oPageProperties = poPage->GetAllProperties();

		PROPID cProperties[] = {PropertyTagEquipMake,
			PropertyTagEquipModel,
			PropertyTagSoftwareUsed,
			PropertyTagCopyright,
			PropertyTagCompression,
			PropertyTagExifShutterSpeed,
			PropertyTagExifFNumber,
			PropertyTagExifAperture,
			PropertyTagExifBrightness,
			PropertyTagExifFlash,
			PropertyTagExifFocalLength,
			PropertyTagExifISOSpeed,
			PropertyTagExifMeteringMode,
			PropertyTagExifLightSource,
			PropertyTagExifExposureProg,
			PropertyTagExifSensingMethod,
			PropertyTagDateTime,
			PropertyTagExifDTOrig,
			PropertyTagExifDTDigitized};

		iCount = sizeof(cProperties) / sizeof(PROPID);

		for (iIndex = 0; iIndex < iCount; iIndex++) {

			DoAddProperty(cProperties[iIndex]);
			}

		//Only in debugging mode show all the properties of the image
		#ifdef DEBUG
		for (iIndex = 0; iIndex < oPageProperties.iMetadataCount; iIndex++) {

			DoAddProperty(oPageProperties.pMetadata[iIndex].id);
			}
		#endif
		}

	//Hide the listview if it is empty
	ShowWindow(hWnd, ItemCount()==0 ? SW_HIDE : SW_NORMAL);
	}

//This function (tries to) add the requested property to the listview
void 
CPagePropertiesListview::DoAddProperty(PROPID piPropertyId) {

	PropertyItem oPropertyItem;
	int iIndex;

	iIndex = oPageProperties.iMetadataCount;

	while (iIndex>0) {

		iIndex--;

		oPropertyItem = oPageProperties.pMetadata[iIndex];

		if (oPropertyItem.id == piPropertyId) DoProcessProperty(oPropertyItem);
		}
	}
	
//This function processes the given propertyitem and will create a listview
//item for it.

void 
CPagePropertiesListview::DoProcessProperty(PropertyItem cProperty) {

	int iItemIndex;
	LVITEM cItem;
	wchar_t* cItemLabel;

	SecureZeroMemory(&cItem, sizeof(LVITEM));
	cItemLabel = (wchar_t*)malloc(sizeof(wchar_t)*(MAX_PATH+1));
	
	cItem.mask = LVIF_TEXT;
	cItem.pszText = cItemLabel;

	if (DoProcessPropertyLabel(cProperty, cItem)) {

		cItem.mask = LVIF_TEXT;
		cItem.iItem = ListView_GetItemCount(hWnd);
		cItem.iSubItem = 0;

		iItemIndex = ListView_InsertItem(hWnd, &cItem);

		//Process the value / subitem
		SecureZeroMemory(&cItem, sizeof(LVITEM));

		cItem.mask = LVIF_TEXT;
		cItem.iItem = iItemIndex;
		cItem.iSubItem = 1;

		DoProcessPropertyValue(cProperty, cItem);
		}

	if (cItemLabel!=NULL) free(cItemLabel);
	}
	
//This function process the value of the given propertyitem. It will add the
//value directly into the subitem of the given listview item.
void 
CPagePropertiesListview::DoProcessPropertyValue(PropertyItem cProperty, LVITEM &pcItem) {

	wchar_t* cItemBuffer;
	wchar_t cItemText[128];
	int iItemBufferSize;
	long* pItemLong;
	BYTE* pItemByte;
	INT32* pItemSLong;
		
	cItemBuffer = NULL;
	cItemBuffer = (wchar_t*)malloc(sizeof(wchar_t)*129);
	pcItem.pszText = cItemBuffer;

	switch (cProperty.type) {

		case PropertyTagTypeASCII:

			iItemBufferSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)cProperty.value, (int)strlen((LPCSTR)cProperty.value), cItemText, 0); 
			cItemBuffer = (wchar_t*)malloc(sizeof(wchar_t)*(iItemBufferSize+1));
			SecureZeroMemory(cItemBuffer, (sizeof(wchar_t)*(iItemBufferSize+1)));
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)cProperty.value, (int)strlen((LPCSTR)cProperty.value), cItemBuffer, iItemBufferSize); 
			pcItem.pszText = cItemBuffer;
			break;

		case PropertyTagTypeLong:

			pItemLong = (long*)cProperty.value;
			swprintf_s(cItemBuffer, 128, L"%d", pItemLong[0]);
			break;

		case PropertyTagTypeSLONG:

			pItemSLong = (INT32*)cProperty.value;
			swprintf_s(cItemBuffer, 128, L"%d", pItemSLong[0]);
			break;

		case PropertyTagTypeShort:

			DoProcessPropertyValueShort(cProperty, pcItem);
			break;

		case PropertyTagTypeByte:

			pItemByte = (BYTE*)cProperty.value;
			swprintf_s(cItemBuffer, 128, L"%d", pItemByte[0]);
			break;

		case PropertyTagTypeRational:

			DoProcessPropertyValueRational(cProperty, pcItem);
			break;

		case PropertyTagTypeSRational:

			DoProcessPropertyValueSRational(cProperty, pcItem);
			break;

		default:

			pcItem.pszText = L"...";
			break;
		}

	ListView_SetItem(hWnd, &pcItem);

	if (cItemBuffer!=NULL) free(cItemBuffer);
	}

//

void
CPagePropertiesListview::DoProcessPropertyValueShort(PropertyItem cProperty, LVITEM &pcItem) {

	short* pItemShort;

	pItemShort = (short*)cProperty.value;

	switch (cProperty.id) {

		case PropertyTagExifFlash:

			switch (*pItemShort) {

				case (0x0000): pcItem.pszText = L"Flash did not fire"; break;
				case (0x0001): pcItem.pszText = L"Flash fired"; break;
				case (0x0005): pcItem.pszText = L"Strobe return light not detected"; break;
				case (0x0007): pcItem.pszText = L"Strobe return light detected"; break;
				case (0x0009): pcItem.pszText = L"Flash fired, compulsory flash mode"; break;
				case (0x000D): pcItem.pszText = L"Flash fired, compulsory flash mode, return light not detected"; break;
				case (0x000F): pcItem.pszText = L"Flash fired, compulsory flash mode, return light detected"; break;
				case (0x0010): pcItem.pszText = L"Flash did not fire, compulsory flash mode"; break;
				case (0x0018): pcItem.pszText = L"Flash did not fire, auto mode"; break;
				case (0x0019): pcItem.pszText = L"Flash fired, auto mode"; break;
				case (0x001D): pcItem.pszText = L"Flash fired, auto mode, return light not detected"; break;
				case (0x001F): pcItem.pszText = L"Flash fired, auto mode, return light detected"; break;
				case (0x0020): pcItem.pszText = L"No flash function"; break;
				case (0x0041): pcItem.pszText = L"Flash fired, red-eye reduction mode"; break;
				case (0x0045): pcItem.pszText = L"Flash fired, red-eye reduction mode, return light not detected"; break;
				case (0x0047): pcItem.pszText = L"Flash fired, red-eye reduction mode, return light detected"; break;
				case (0x0049): pcItem.pszText = L"Flash fired, compulsory flash mode, red-eye reduction mode"; break;
				case (0x004D): pcItem.pszText = L"Flash fired, compulsory flash mode, red-eye reduction mode, return light not detected"; break;
				case (0x004F): pcItem.pszText = L"Flash fired, compulsory flash mode, red-eye reduction mode, return light detected"; break;
				case (0x0059): pcItem.pszText = L"Flash fired, auto mode, red-eye reduction mode"; break;
				case (0x005D): pcItem.pszText = L"Flash fired, auto mode, return light not detected, red-eye reduction mode"; break;
				case (0x005F): pcItem.pszText = L"Flash fired, auto mode, return light detected, red-eye reduction mode"; break;
				default: pcItem.pszText = L"Unknown"; break;
				}
			break;

		case PropertyTagExifExposureProg:

			switch (*pItemShort) {

				case 0: pcItem.pszText = L"Not defined"; break;
				case 1: pcItem.pszText = L"Manual"; break;
				case 2: pcItem.pszText = L"Normal program"; break;
				case 3: pcItem.pszText = L"Aperture priority"; break;
				case 4: pcItem.pszText = L"Shutter priority"; break;
				case 5: pcItem.pszText = L"Creative program"; break;
				case 6: pcItem.pszText = L"Action program"; break;
				case 7: pcItem.pszText = L"Portrait mode"; break;
				case 8: pcItem.pszText = L"Landscape mode"; break;
				default: pcItem.pszText = L"Unknown"; break;
				}
			break;

		case PropertyTagExifLightSource:

			switch (*pItemShort) {

				case 0: pcItem.pszText = L"Unknown"; break;
				case 1: pcItem.pszText = L"Daylight"; break;
				case 2: pcItem.pszText = L"Flourescent"; break;
				case 3: pcItem.pszText = L"Tungsten"; break;
				case 17: pcItem.pszText = L"Standard Light A"; break;
				case 18: pcItem.pszText = L"Standard Light B"; break;
				case 19: pcItem.pszText = L"Standard Light C"; break;
				case 20: pcItem.pszText = L"D55"; break;
				case 21: pcItem.pszText = L"D65"; break;
				case 22: pcItem.pszText = L"D75"; break;
				default: pcItem.pszText = L"Unknown"; break;
				}
			break;

		case PropertyTagExifMeteringMode:

			switch (*pItemShort) {

				case 0: pcItem.pszText = L"Unknown"; break;
				case 1: pcItem.pszText = L"Average"; break;
				case 2: pcItem.pszText = L"Center Weighted Average"; break;
				case 3: pcItem.pszText = L"Spot"; break;
				case 4: pcItem.pszText = L"Multispot"; break;
				case 5: pcItem.pszText = L"Pattern"; break;
				case 6: pcItem.pszText = L"Partial"; break;
				default: pcItem.pszText = L"Unknown"; break;
				}
			break;


		case PropertyTagExifSensingMethod:

			switch (*pItemShort) {

				case 1: pcItem.pszText = L"Not defined"; break;
				case 2: pcItem.pszText = L"One-chip color area sensor"; break;
				case 3: pcItem.pszText = L"Two-chip color area sensor"; break;
				case 4: pcItem.pszText = L"Three-chip color area sensor"; break;
				case 5: pcItem.pszText = L"Color sequential area sensor"; break;
				case 7: pcItem.pszText = L"Trilinear sensor"; break;
				case 8: pcItem.pszText = L"Color sequential linear sensor"; break;
				default: pcItem.pszText = L"Unknown"; break;
				}
			break;

		default:

			swprintf_s(pcItem.pszText, 128, L"%d", pItemShort[0]);
			break;
		}
	}

//Process the property value rational
void 
CPagePropertiesListview::DoProcessPropertyValueRational(PropertyItem cProperty, LVITEM &pcItem) {

	long* pItemLong;

	pItemLong = (long*)cProperty.value;

	switch (cProperty.id) {

		case PropertyTagExifAperture:
		case PropertyTagExifFNumber:

			swprintf_s(pcItem.pszText, 128, L"F/%.1f", (float)pItemLong[0]/(float)pItemLong[1]);
			break;

		case PropertyTagExifFocalLength:

			swprintf_s(pcItem.pszText, 128, L"%.1f mm", (float)pItemLong[0]/(float)pItemLong[1]);
			break;

		case PropertyTagExifExposureTime:

			swprintf_s(pcItem.pszText, 128, L"1/%.0f sec", powf(2.0f, (float)pItemLong[0]/(float)pItemLong[1]));
			break;

		default:

			swprintf_s(pcItem.pszText, 128, L"%d/%d", pItemLong[0], pItemLong[1]);
			break;
		}
	}

//Process the property value rational
void 
CPagePropertiesListview::DoProcessPropertyValueSRational(PropertyItem cProperty, LVITEM &pcItem) {

	INT32* pItemSLong;

	pItemSLong = (INT32*)cProperty.value;

	switch (cProperty.id) {

		case PropertyTagExifShutterSpeed:

			swprintf_s(pcItem.pszText, 128, L"1/%.0f sec", powf(2.0f, (float)pItemSLong[0]/(float)pItemSLong[1]));
			break;

		default:

			swprintf_s(pcItem.pszText, 128, L"%d/%d", pItemSLong[0], pItemSLong[1]);
			break;
		}
	}
	
//This function process the label of the given propertyitem. If the label
//isn't known/supported by Copying Machine it will result in a false
bool 
CPagePropertiesListview::DoProcessPropertyLabel(PropertyItem cProperty, LVITEM &cItem) {

	bool bResult;
	int  iResource;

	bResult = true;
	iResource = -1;

	switch (cProperty.id) {

		case PropertyTagArtist: cItem.pszText =L"Artist"; break;
		case PropertyTagBitsPerSample: cItem.pszText =L"BitsPerSample"; break;
		case PropertyTagCellHeight: cItem.pszText =L"CellHeight"; break;
		case PropertyTagCellWidth: cItem.pszText =L"CellWidth"; break;
		case PropertyTagChrominanceTable: cItem.pszText =L"ChrominanceTable"; break;
		case PropertyTagColorMap: cItem.pszText =L"ColorMap"; break;
		case PropertyTagColorTransferFunction: cItem.pszText =L"ColorTransferFunction"; break;
		case PropertyTagCompression: iResource = IDS_COMPRESSION; break;
		case PropertyTagCopyright: cItem.pszText =L"Copyright"; break;
		case PropertyTagDateTime: iResource = IDS_DATEMODIFIED; break;
		case PropertyTagDocumentName: iResource = IDS_NAME; break;
		case PropertyTagDotRange: cItem.pszText =L"DotRange"; break;
		case PropertyTagEquipMake: iResource = IDS_EQUIPMAKE; break;
		case PropertyTagEquipModel: iResource = IDS_EQUIPMODEL; break;
		case PropertyTagExifAperture: iResource = IDS_APERTURE; break;
		case PropertyTagExifBrightness: iResource = IDS_BRIGHTNESS; break;
		case PropertyTagExifCfaPattern: cItem.pszText =L"ExifCfaPattern"; break;
		case PropertyTagExifColorSpace: cItem.pszText =L"ExifColorSpace"; break;
		case PropertyTagExifCompBPP: cItem.pszText =L"ExifCompBPP"; break;
		case PropertyTagExifCompConfig: cItem.pszText =L"ExifCompConfig"; break;
		case PropertyTagExifDTDigitized: iResource = IDS_DATEDIGITIZED; break;
		case PropertyTagExifDTDigSS: cItem.pszText =L"ExifDTDigSS"; break;
		case PropertyTagExifDTOrig: iResource = IDS_DATETAKEN; break;
		case PropertyTagExifDTOrigSS: cItem.pszText =L"ExifDTOrigSS"; break;
		case PropertyTagExifDTSubsec: cItem.pszText =L"ExifDTSubsec"; break;
		case PropertyTagExifExposureBias: cItem.pszText =L"ExifExposureBias"; break;
		case PropertyTagExifExposureIndex: cItem.pszText =L"ExifExposureIndex"; break;
		case PropertyTagExifExposureProg: iResource = IDS_EXPOSUREPROGRAM; break;
		case PropertyTagExifExposureTime: iResource = IDS_EXPOSURETIME; break;
		case PropertyTagExifFileSource: cItem.pszText =L"ExifFileSource"; break;
		case PropertyTagExifFlash: iResource = IDS_FLASH; break;
		case PropertyTagExifFlashEnergy: cItem.pszText =L"ExifFlashEnergy"; break;
		case PropertyTagExifFNumber: iResource = IDS_FNUMBER; break;
		case PropertyTagExifFocalLength: iResource = IDS_FOCALLENGTH; break;
		case PropertyTagExifFocalResUnit: cItem.pszText =L"ExifFocalResUnit"; break;
		case PropertyTagExifFocalXRes: cItem.pszText =L"ExifFocalXRes"; break;
		case PropertyTagExifFocalYRes: cItem.pszText =L"ExifFocalYRes"; break;
		case PropertyTagExifFPXVer: cItem.pszText =L"ExifFPXVer"; break;
		case PropertyTagExifIFD: cItem.pszText =L"ExifIFD"; break;
		case PropertyTagExifInterop: cItem.pszText =L"ExifInterop"; break;
		case PropertyTagExifISOSpeed: cItem.pszText =L"ISO"; break;
		case PropertyTagExifLightSource: iResource = IDS_LIGHTSOURCE; break;
		case PropertyTagExifMakerNote: cItem.pszText =L"ExifMakerNote"; break;
		case PropertyTagExifMaxAperture: cItem.pszText =L"ExifMaxAperture"; break;
		case PropertyTagExifMeteringMode: iResource = IDS_METERINGMODE; break;
		case PropertyTagExifOECF: cItem.pszText =L"ExifOECF"; break;
		case PropertyTagExifPixXDim: cItem.pszText =L"ExifPixXDim"; break;
		case PropertyTagExifPixYDim: cItem.pszText =L"ExifPixYDim"; break;
		case PropertyTagExifRelatedWav: cItem.pszText =L"ExifRelatedWav"; break;
		case PropertyTagExifSceneType: cItem.pszText =L"ExifSceneType"; break;
		case PropertyTagExifSensingMethod: iResource = IDS_SENSINGMETHOD; break;
		case PropertyTagExifShutterSpeed: iResource = IDS_SHUTTERSPEED; break;
		case PropertyTagExifSpatialFR: cItem.pszText =L"ExifSpatialFR"; break;
		case PropertyTagExifSpectralSense: cItem.pszText =L"ExifSpectralSense"; break;
		case PropertyTagExifSubjectDist: cItem.pszText =L"ExifSubjectDist"; break;
		case PropertyTagExifSubjectLoc: cItem.pszText =L"ExifSubjectLoc"; break;
		case PropertyTagExifUserComment: cItem.pszText =L"ExifUserComment"; break;
		case PropertyTagExifVer: cItem.pszText =L"ExifVer"; break;
		case PropertyTagExtraSamples: cItem.pszText =L"ExtraSamples"; break;
		case PropertyTagFillOrder: cItem.pszText =L"FillOrder"; break;
		case PropertyTagFrameDelay: cItem.pszText =L"FrameDelay"; break;
		case PropertyTagFreeByteCounts: cItem.pszText =L"FreeByteCounts"; break;
		case PropertyTagFreeOffset: cItem.pszText =L"FreeOffset"; break;
		case PropertyTagGamma: cItem.pszText =L"Gamma"; break;
		case PropertyTagGpsAltitude: cItem.pszText =L"GpsAltitude"; break;
		case PropertyTagGpsAltitudeRef: cItem.pszText =L"GpsAltitudeRef"; break;
		case PropertyTagGpsDestBear: cItem.pszText =L"GpsDestBear"; break;
		case PropertyTagGpsDestBearRef: cItem.pszText =L"GpsDestBearRef"; break;
		case PropertyTagGpsDestDist: cItem.pszText =L"GpsDestDist"; break;
		case PropertyTagGpsDestDistRef: cItem.pszText =L"GpsDestDistRef"; break;
		case PropertyTagGpsDestLat: cItem.pszText =L"GpsDestLat"; break;
		case PropertyTagGpsDestLatRef: cItem.pszText =L"GpsDestLatRef"; break;
		case PropertyTagGpsDestLong: cItem.pszText =L"GpsDestLong"; break;
		case PropertyTagGpsDestLongRef: cItem.pszText =L"GpsDestLongRef"; break;
		case PropertyTagGpsGpsDop: cItem.pszText =L"GpsGpsDop"; break;
		case PropertyTagGpsGpsMeasureMode: cItem.pszText =L"GpsGpsMeasureMode"; break;
		case PropertyTagGpsGpsSatellites: cItem.pszText =L"GpsGpsSatellites"; break;
		case PropertyTagGpsGpsStatus: cItem.pszText =L"GpsGpsStatus"; break;
		case PropertyTagGpsGpsTime: cItem.pszText =L"GpsGpsTime"; break;
		case PropertyTagGpsIFD: cItem.pszText =L"GpsIFD"; break;
		case PropertyTagGpsImgDir: cItem.pszText =L"GpsImgDir"; break;
		case PropertyTagGpsImgDirRef: cItem.pszText =L"GpsImgDirRef"; break;
		case PropertyTagGpsLatitude: cItem.pszText =L"GpsLatitude"; break;
		case PropertyTagGpsLatitudeRef: cItem.pszText =L"GpsLatitudeRef"; break;
		case PropertyTagGpsLongitude: cItem.pszText =L"GpsLongitude"; break;
		case PropertyTagGpsLongitudeRef: cItem.pszText =L"GpsLongitudeRef"; break;
		case PropertyTagGpsMapDatum: cItem.pszText =L"GpsMapDatum"; break;
		case PropertyTagGpsSpeed: cItem.pszText =L"GpsSpeed"; break;
		case PropertyTagGpsSpeedRef: cItem.pszText =L"GpsSpeedRef"; break;
		case PropertyTagGpsTrack: cItem.pszText =L"GpsTrack"; break;
		case PropertyTagGpsTrackRef: cItem.pszText =L"GpsTrackRef"; break;
		case PropertyTagGpsVer: cItem.pszText =L"GpsVer"; break;
		case PropertyTagGrayResponseCurve: cItem.pszText =L"GrayResponseCurve"; break;
		case PropertyTagGrayResponseUnit: cItem.pszText =L"GrayResponseUnit"; break;
		case PropertyTagGridSize: cItem.pszText =L"GridSize"; break;
		case PropertyTagHalftoneDegree: cItem.pszText =L"HalftoneDegree"; break;
		case PropertyTagHalftoneHints: cItem.pszText =L"HalftoneHints"; break;
		case PropertyTagHalftoneLPI: cItem.pszText =L"HalftoneLPI"; break;
		case PropertyTagHalftoneLPIUnit: cItem.pszText =L"HalftoneLPIUnit"; break;
		case PropertyTagHalftoneMisc: cItem.pszText =L"HalftoneMisc"; break;
		case PropertyTagHalftoneScreen: cItem.pszText =L"HalftoneScreen"; break;
		case PropertyTagHalftoneShape: cItem.pszText =L"HalftoneShape"; break;
		case PropertyTagHostComputer: cItem.pszText =L"HostComputer"; break;
		case PropertyTagICCProfile: cItem.pszText =L"ICCProfile"; break;
		case PropertyTagICCProfileDescriptor: cItem.pszText =L"ICCProfileDescriptor"; break;
		case PropertyTagImageDescription: iResource = IDS_DESCRIPTION; break;
		case PropertyTagImageHeight: iResource = IDS_HEIGHT; break;
		case PropertyTagImageTitle: iResource = IDS_TITLE; break;
		case PropertyTagImageWidth: iResource = IDS_WIDTH; break;
		case PropertyTagInkNames: cItem.pszText =L"InkNames"; break;
		case PropertyTagInkSet: cItem.pszText =L"InkSet"; break;
		case PropertyTagJPEGACTables: cItem.pszText =L"JPEGACTables"; break;
		case PropertyTagJPEGDCTables: cItem.pszText =L"JPEGDCTables"; break;
		case PropertyTagJPEGInterFormat: cItem.pszText =L"JPEGInterFormat"; break;
		case PropertyTagJPEGInterLength: cItem.pszText =L"JPEGInterLength"; break;
		case PropertyTagJPEGLosslessPredictors: cItem.pszText =L"JPEGLosslessPredictors"; break;
		case PropertyTagJPEGPointTransforms: cItem.pszText =L"JPEGPointTransforms"; break;
		case PropertyTagJPEGProc: cItem.pszText =L"JPEGProc"; break;
		case PropertyTagJPEGQTables: cItem.pszText =L"JPEGQTables"; break;
		case PropertyTagJPEGQuality: cItem.pszText =L"JPEGQuality"; break;
		case PropertyTagJPEGRestartInterval: cItem.pszText =L"JPEGRestartInterval"; break;
		case PropertyTagLoopCount: cItem.pszText =L"LoopCount"; break;
		case PropertyTagLuminanceTable: cItem.pszText =L"LuminanceTable"; break;
		case PropertyTagMaxSampleValue: cItem.pszText =L"MaxSampleValue"; break;
		case PropertyTagMinSampleValue: cItem.pszText =L"MinSampleValue"; break;
		case PropertyTagNewSubfileType: cItem.pszText =L"NewSubfileType"; break;
		case PropertyTagNumberOfInks: cItem.pszText =L"NumberOfInks"; break;
		case PropertyTagOrientation: cItem.pszText =L"Orientation"; break;
		case PropertyTagPageName: cItem.pszText =L"PageName"; break;
		case PropertyTagPageNumber: cItem.pszText =L"PageNumber"; break;
		case PropertyTagPaletteHistogram: cItem.pszText =L"PaletteHistogram"; break;
		case PropertyTagPhotometricInterp: cItem.pszText =L"PhotometricInterp"; break;
		case PropertyTagPixelPerUnitX: cItem.pszText =L"PixelPerUnitX"; break;
		case PropertyTagPixelPerUnitY: cItem.pszText =L"PixelPerUnitY"; break;
		case PropertyTagPixelUnit: cItem.pszText =L"PixelUnit"; break;
		case PropertyTagPlanarConfig: cItem.pszText =L"PlanarConfig"; break;
		case PropertyTagPredictor: cItem.pszText =L"Predictor"; break;
		case PropertyTagPrimaryChromaticities: cItem.pszText =L"PrimaryChromaticities"; break;
		case PropertyTagPrintFlags: cItem.pszText =L"PrintFlags"; break;
		case PropertyTagPrintFlagsBleedWidth: cItem.pszText =L"PrintFlagsBleedWidth"; break;
		case PropertyTagPrintFlagsBleedWidthScale: cItem.pszText =L"PrintFlagsBleedWidthScale"; break;
		case PropertyTagPrintFlagsCrop: cItem.pszText =L"PrintFlagsCrop"; break;
		case PropertyTagPrintFlagsVersion: cItem.pszText =L"PrintFlagsVersion"; break;
		case PropertyTagREFBlackWhite: cItem.pszText =L"REFBlackWhite"; break;
		case PropertyTagResolutionUnit: cItem.pszText =L"ResolutionUnit"; break;
		case PropertyTagResolutionXLengthUnit: cItem.pszText =L"ResolutionXLengthUnit"; break;
		case PropertyTagResolutionXUnit: cItem.pszText =L"ResolutionXUnit"; break;
		case PropertyTagResolutionYLengthUnit: cItem.pszText =L"ResolutionYLengthUnit"; break;
		case PropertyTagResolutionYUnit: cItem.pszText =L"ResolutionYUnit"; break;
		case PropertyTagRowsPerStrip: cItem.pszText =L"RowsPerStrip"; break;
		case PropertyTagSampleFormat: cItem.pszText =L"SampleFormat"; break;
		case PropertyTagSamplesPerPixel: cItem.pszText =L"SamplesPerPixel"; break;
		case PropertyTagSMaxSampleValue: cItem.pszText =L"SMaxSampleValue"; break;
		case PropertyTagSMinSampleValue: cItem.pszText =L"SMinSampleValue"; break;
		case PropertyTagSoftwareUsed: cItem.pszText =L"Software"; break;
		case PropertyTagSRGBRenderingIntent: cItem.pszText =L"SRGBRenderingIntent"; break;
		case PropertyTagStripBytesCount: cItem.pszText =L"StripBytesCount"; break;
		case PropertyTagStripOffsets: cItem.pszText =L"StripOffsets"; break;
		case PropertyTagSubfileType: cItem.pszText =L"SubfileType"; break;
		case PropertyTagT4Option: cItem.pszText =L"T4Option"; break;
		case PropertyTagT6Option: cItem.pszText =L"T6Option"; break;
		case PropertyTagTargetPrinter: cItem.pszText =L"TargetPrinter"; break;
		case PropertyTagThreshHolding: cItem.pszText =L"ThreshHolding"; break;
		case PropertyTagThumbnailArtist: cItem.pszText =L"ThumbnailArtist"; break;
		case PropertyTagThumbnailBitsPerSample: cItem.pszText =L"ThumbnailBitsPerSample"; break;
		case PropertyTagThumbnailColorDepth: cItem.pszText =L"ThumbnailColorDepth"; break;
		case PropertyTagThumbnailCompressedSize: cItem.pszText =L"ThumbnailCompressedSize"; break;
		case PropertyTagThumbnailCompression: cItem.pszText =L"ThumbnailCompression"; break;
		case PropertyTagThumbnailCopyRight: cItem.pszText =L"ThumbnailCopyRight"; break;
		case PropertyTagThumbnailData: cItem.pszText =L"ThumbnailData"; break;
		case PropertyTagThumbnailDateTime: cItem.pszText =L"ThumbnailDateTime"; break;
		case PropertyTagThumbnailEquipMake: cItem.pszText =L"ThumbnailEquipMake"; break;
		case PropertyTagThumbnailEquipModel: cItem.pszText =L"ThumbnailEquipModel"; break;
		case PropertyTagThumbnailFormat: cItem.pszText =L"ThumbnailFormat"; break;
		case PropertyTagThumbnailHeight: cItem.pszText =L"ThumbnailHeight"; break;
		case PropertyTagThumbnailImageDescription: cItem.pszText =L"ThumbnailImageDescription"; break;
		case PropertyTagThumbnailImageHeight: cItem.pszText =L"ThumbnailImageHeight"; break;
		case PropertyTagThumbnailImageWidth: cItem.pszText =L"ThumbnailImageWidth"; break;
		case PropertyTagThumbnailOrientation: cItem.pszText =L"ThumbnailOrientation"; break;
		case PropertyTagThumbnailPhotometricInterp: cItem.pszText =L"ThumbnailPhotometricInterp"; break;
		case PropertyTagThumbnailPlanarConfig: cItem.pszText =L"ThumbnailPlanarConfig"; break;
		case PropertyTagThumbnailPlanes: cItem.pszText =L"ThumbnailPlanes"; break;
		case PropertyTagThumbnailPrimaryChromaticities: cItem.pszText =L"ThumbnailPrimaryChromaticities"; break;
		case PropertyTagThumbnailRawBytes: cItem.pszText =L"ThumbnailRawBytes"; break;
		case PropertyTagThumbnailRefBlackWhite: cItem.pszText =L"ThumbnailRefBlackWhite"; break;
		case PropertyTagThumbnailResolutionUnit: cItem.pszText =L"ThumbnailResolutionUnit"; break;
		case PropertyTagThumbnailResolutionX: cItem.pszText =L"ThumbnailResolutionX"; break;
		case PropertyTagThumbnailResolutionY: cItem.pszText =L"ThumbnailResolutionY"; break;
		case PropertyTagThumbnailRowsPerStrip: cItem.pszText =L"ThumbnailRowsPerStrip"; break;
		case PropertyTagThumbnailSamplesPerPixel: cItem.pszText =L"ThumbnailSamplesPerPixel"; break;
		case PropertyTagThumbnailSize: cItem.pszText =L"ThumbnailSize"; break;
		case PropertyTagThumbnailSoftwareUsed: cItem.pszText =L"ThumbnailSoftwareUsed"; break;
		case PropertyTagThumbnailStripBytesCount: cItem.pszText =L"ThumbnailStripBytesCount"; break;
		case PropertyTagThumbnailStripOffsets: cItem.pszText =L"ThumbnailStripOffsets"; break;
		case PropertyTagThumbnailTransferFunction: cItem.pszText =L"ThumbnailTransferFunction"; break;
		case PropertyTagThumbnailWhitePoint: cItem.pszText =L"ThumbnailWhitePoint"; break;
		case PropertyTagThumbnailWidth: cItem.pszText =L"ThumbnailWidth"; break;
		case PropertyTagThumbnailYCbCrCoefficients: cItem.pszText =L"ThumbnailYCbCrCoefficients"; break;
		case PropertyTagThumbnailYCbCrPositioning: cItem.pszText =L"ThumbnailYCbCrPositioning"; break;
		case PropertyTagThumbnailYCbCrSubsampling: cItem.pszText =L"ThumbnailYCbCrSubsampling"; break;
		case PropertyTagTileByteCounts: cItem.pszText =L"TileByteCounts"; break;
		case PropertyTagTileLength: cItem.pszText =L"TileLength"; break;
		case PropertyTagTileOffset: cItem.pszText =L"TileOffset"; break;
		case PropertyTagTileWidth: cItem.pszText =L"TileWidth"; break;
		case PropertyTagTransferRange: cItem.pszText =L"TransferRange"; break;
		case PropertyTagWhitePoint: cItem.pszText =L"WhitePoint"; break;
		case PropertyTagXPosition: cItem.pszText =L"XPosition"; break;
		case PropertyTagXResolution: cItem.pszText =L"XResolution"; break;
		case PropertyTagYCbCrCoefficients: cItem.pszText =L"YCbCrCoefficients"; break;
		case PropertyTagYCbCrPositioning: cItem.pszText =L"YCbCrPositioning"; break;
		case PropertyTagYCbCrSubsampling: cItem.pszText =L"YCbCrSubsampling"; break;
		case PropertyTagYPosition: cItem.pszText =L"YPosition"; break;
		case PropertyTagYResolution: cItem.pszText =L"YResolution"; break;
		default:

			cItem.pszText =L"Unknown";
			bResult = false;
			break;
		}

	if (iResource!=-1) {

		LoadString(oGlobalInstances.hLanguage, iResource, cItem.pszText, MAX_PATH);
		}

	return bResult;
	}

//Save settings
void
CPagePropertiesListview::SaveSettings() {

	//Save the width of the columns
	oGlobalInstances.oRegistry->WriteInt(L"Window", L"PPProperty", GetColumnWidth(0));
	oGlobalInstances.oRegistry->WriteInt(L"Window", L"PPValue", GetColumnWidth(1));

	CWindowBase::SaveSettings();
	}

//Load settings
void
CPagePropertiesListview::LoadSettings() {

	SetColumnWidth(0, oGlobalInstances.oRegistry->ReadInt(L"Window", L"PPProperty", 80));
	SetColumnWidth(1, oGlobalInstances.oRegistry->ReadInt(L"Window", L"PPValue", 90));

	CWindowBase::LoadSettings();
	}

