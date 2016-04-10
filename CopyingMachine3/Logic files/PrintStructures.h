#pragma once

enum eOutputType {eOutputFile, eOutputMail, eOutputPDF, eOutputPrinter, eOutputBitmap};

struct sPrinterDevice {

	std::wstring sName;
	};

struct sOutputDevice {
	
	eOutputType		cType;			//Type of the device
	std::wstring	sDescription;	//A short description of the device
	CBitmap*		oBitmap;		//The result bitmap of the 'printed' document

	sPrinterDevice	oPrinterDevice;	//The properties of the printer, only valid if type is ePrinter
	};

enum ePrintAlignmentType {	eAlignNorthWest, eAlignNorthCenter, eAlignNorthEast,
							eAlignCenterWest, eAlignCenterCenter, eAlignCenterEast,
							eAlignSouthWest, eAlignSouthCenter, eAlignSouthEast};

enum ePrintSetting {ePrintShowInterface, ePrintMultiPage, ePrintShrink, ePrintStretch,
					ePrintMagnification, ePrintAlignment, ePrintPageOrientation,
					ePrintMarginLeft, ePrintMarginRight, ePrintMarginTop, ePrintMarginBottom,
					ePrintPageType, ePrintMultiPagesOnPage, ePrintMultipleCopiesOnPage,
					ePrintCopies, ePrintAlwaysShowInterface};



