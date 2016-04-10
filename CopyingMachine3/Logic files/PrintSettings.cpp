#include "StdAfx.h"
#include "CopyingMachineCore.h"

CPrintSettings::CPrintSettings(sGlobalInstances pInstances)
{
	//Set helper classes
	oTrace = pInstances.oTrace;
	oRegistry = pInstances.oRegistry;
	oNotifications = (CCoreNotifications*)pInstances.oNotifications;

	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pInstances, sizeof(sGlobalInstances));

	//Load the current settings
	Load();

	hStatusWindow		= NULL;
	hProgressBar		= NULL;
	hWindow				= NULL;
	bDeleteAfterPrint	= false;
	bNotifyChanges		= true;

	oPrintData.bInitializePageRange = true;
}

CPrintSettings::~CPrintSettings(void)
{
}

//This function creates a copy of the current printsettings instance
CPrintSettings* 
CPrintSettings::Copy() {

	CPrintSettings* oCopy;
	sPrintSettingsData oCopyData;
	sOutputDevice oOutputDevice;

	oTrace->StartTrace(__WFUNCTION__);

	oCopy = new CPrintSettings(oGlobalInstances);

	memcpy_s(&oCopyData, sizeof(sPrintSettingsData), 
			 &oData, sizeof(sPrintSettingsData));

	oCopy->CopyData(oCopyData);	

	GetDevice(oOutputDevice);
	oCopy->bNotifyChanges = false;
	oCopy->SetDevice(oOutputDevice);
	oCopy->bNotifyChanges = true;

	oTrace->EndTrace(__WFUNCTION__);

	return oCopy;
	}

void 
CPrintSettings::CopyData(sPrintSettingsData& poData) {
	
	oTrace->StartTrace(__WFUNCTION__);

	memcpy_s(&oData, sizeof(sPrintSettingsData), 
			 &poData, sizeof(sPrintSettingsData));

	hProgressBar = NULL;
	hWindow      = NULL;

	oTrace->EndTrace(__WFUNCTION__);
}


//Load and save the settings from the registry
void 
CPrintSettings::Load() {

	oTrace->StartTrace(__WFUNCTION__);

	cOutputType = (eOutputType)oRegistry->ReadInt(L"Print", L"iOutputDeviceType", (int)eOutputFile);
	oData.bAlwaysShowInterface = (oRegistry->ReadInt(L"Print", L"bAlwaysShowInterface", 0) == 1);
	oData.bShowInterface = (oRegistry->ReadInt(L"Print", L"bShowInterface", 1) == 1);
	oData.bMultiPagesOnPage = (oRegistry->ReadInt(L"Print", L"bMultiPagesOnPage", 0) == 1);
	oData.bMultipleCopiesOnPage = (oRegistry->ReadInt(L"Print", L"bMultipleCopiesOnPage", 0) == 1);
	oData.bShrinkToFit = (oRegistry->ReadInt(L"Print", L"bShrinkToFit", 0) == 1);
	oData.bStretchToPage = (oRegistry->ReadInt(L"Print", L"bStretchToPage", 0) == 1);
	oData.bPortrait = (oRegistry->ReadInt(L"Print", L"bPortrait", 1) == 1);
	oData.iCopies = oRegistry->ReadInt(L"Print", L"iCopies", 1);	
	oData.iAlignment = oRegistry->ReadInt(L"Print", L"iAlignment", (int)eAlignNorthWest);	
	oData.iMagnification = oRegistry->ReadInt(L"Print", L"iMagnification", 100);	
	oData.iMarginLeft = oRegistry->ReadInt(L"Print", L"iMarginLeft", 0);	
	oData.iMarginRight = oRegistry->ReadInt(L"Print", L"iMarginRight", 0);	
	oData.iMarginTop = oRegistry->ReadInt(L"Print", L"iMarginTop", 0);	
	oData.iMarginBottom = oRegistry->ReadInt(L"Print", L"iMarginBottom", 0);	
	oData.iPageType = oRegistry->ReadInt(L"Print", L"iPageType", 0);	
	
	oRegistry->ReadString(L"Print", L"sOutputDeviceName", L"", sOutputDeviceName);

	oTrace->EndTrace(__WFUNCTION__);
	}

void 
CPrintSettings::Save() {

	oTrace->StartTrace(__WFUNCTION__);

	oRegistry->WriteInt(L"Print", L"iOutputDeviceType",		(int)cOutputType);
	oRegistry->WriteInt(L"Print", L"bShowInterface",		(int)oData.bShowInterface ? 1 : 0);
	oRegistry->WriteInt(L"Print", L"bAlwaysShowInterface",	(int)oData.bAlwaysShowInterface ? 1 : 0);
	oRegistry->WriteInt(L"Print", L"bMultiPagesOnPage",		(int)oData.bMultiPagesOnPage ? 1 : 0);
	oRegistry->WriteInt(L"Print", L"bMultipleCopiesOnPage", (int)oData.bMultipleCopiesOnPage ? 1 : 0);
	oRegistry->WriteInt(L"Print", L"bShrinkToFit",			(int)oData.bShrinkToFit ? 1 : 0);
	oRegistry->WriteInt(L"Print", L"bStretchToPage",		(int)oData.bStretchToPage ? 1 : 0);
	oRegistry->WriteInt(L"Print", L"bPortrait",				(int)oData.bPortrait ? 1 : 0);
	oRegistry->WriteInt(L"Print", L"iCopies",				(int)oData.iCopies);
	oRegistry->WriteInt(L"Print", L"iAlignment",			(int)oData.iAlignment);
	oRegistry->WriteInt(L"Print", L"iMagnification",		(int)oData.iMagnification);
	oRegistry->WriteInt(L"Print", L"iMarginLeft",			(int)oData.iMarginLeft);
	oRegistry->WriteInt(L"Print", L"iMarginRight",			(int)oData.iMarginRight);
	oRegistry->WriteInt(L"Print", L"iMarginTop",			(int)oData.iMarginTop);
	oRegistry->WriteInt(L"Print", L"iMarginBottom",			(int)oData.iMarginBottom);
	oRegistry->WriteInt(L"Print", L"iPageType",				(int)oData.iPageType);

	oRegistry->WriteString(L"Print", L"sOutputDeviceName", sOutputDeviceName.c_str());

	oTrace->EndTrace(__WFUNCTION__);
	}

//This function outputs the print settings to the trace file
void 
CPrintSettings::Trace() {

	oTrace->StartTrace(__WFUNCTION__);

	oTrace->Add(L"iOutputDeviceType",		(int)cOutputType);
	oTrace->Add(L"sOutputDeviceName",		sOutputDeviceName);
	oTrace->Add(L"bShowInterface",			oData.bShowInterface);
	oTrace->Add(L"bAlwaysShowInterface",	oData.bAlwaysShowInterface);
	oTrace->Add(L"bMultiPagesOnPage",		oData.bMultiPagesOnPage);
	oTrace->Add(L"bMultipleCopiesOnPage",	oData.bMultipleCopiesOnPage);
	oTrace->Add(L"bShrinkToFit",			oData.bShrinkToFit);
	oTrace->Add(L"bStretchToPage",			oData.bStretchToPage);
	oTrace->Add(L"bPortrait",				oData.bPortrait);
	oTrace->Add(L"iCopies",					oData.iCopies);
	oTrace->Add(L"iAlignment",				oData.iAlignment);
	oTrace->Add(L"iMagnification",			oData.iMagnification);
	oTrace->Add(L"iMarginLeft",				oData.iMarginLeft);
	oTrace->Add(L"iMarginRight",			oData.iMarginRight);
	oTrace->Add(L"iMarginTop",				oData.iMarginTop);
	oTrace->Add(L"iMarginBottom",			oData.iMarginBottom);
	oTrace->Add(L"iPageType",				oData.iPageType);

	oTrace->EndTrace(__WFUNCTION__);
	}	


//Get and set the currently selected outputdevice
bool 
CPrintSettings::SetDevice(sOutputDevice &pcOutputDevice) {

	oTrace->StartTrace(__WFUNCTION__);

	if (pcOutputDevice.cType != cOutputType ||
		pcOutputDevice.oPrinterDevice.sName != sOutputDeviceName) {
	
		cOutputType			= pcOutputDevice.cType;
		oOutputBitmap		= pcOutputDevice.oBitmap;
		sOutputDeviceName	= L"";

		if (cOutputType==eOutputPrinter) sOutputDeviceName = pcOutputDevice.oPrinterDevice.sName;

		if (bNotifyChanges) oNotifications->CommunicateEvent(eNotificationPrintDeviceChanged, NULL);
		}
		
	oTrace->EndTrace(__WFUNCTION__);
	
	return true;
	}

bool 
CPrintSettings::GetDevice(sOutputDevice &pcOutputDevice) {

	bool bResult;
	CCopyingMachineCore* oCore;
	
	oTrace->StartTrace(__WFUNCTION__);

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	if (cOutputType!=eOutputBitmap) {

		bResult = oCore->oPrintCore->FindOutputDevice(cOutputType, sOutputDeviceName, pcOutputDevice);
		}
	else {
		pcOutputDevice.cType = eOutputBitmap;
		pcOutputDevice.oBitmap = oOutputBitmap;
		bResult = true;
		}

	oTrace->EndTrace(__WFUNCTION__,bResult);
	
	return bResult;
	}

//Get and set boolean settings
bool 
CPrintSettings::GetBool(ePrintSetting pSetting) {

	bool bReturn;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	switch (pSetting) {

		case ePrintShowInterface:		bReturn = oData.bShowInterface;			break;
		case ePrintAlwaysShowInterface:	bReturn = oData.bAlwaysShowInterface;			break;
		case ePrintShrink:				bReturn = oData.bShrinkToFit;			break;
		case ePrintStretch:				bReturn = oData.bStretchToPage;			break;
		case ePrintPageOrientation:		bReturn = oData.bPortrait;				break;
		case ePrintMultiPagesOnPage:	bReturn = oData.bMultiPagesOnPage;		break;
		case ePrintMultipleCopiesOnPage:bReturn = oData.bMultipleCopiesOnPage;	break;
		
		default:
			oTrace->Add(L"Error, unknown setting requested", eAll);
			::MessageBox(NULL, L"Error, unknown settings requested", L"Copying Machine", MB_ICONERROR | MB_OK);
			break;
		}

	oTrace->EndTrace(__WFUNCTION__, bReturn, eAll);

	return bReturn;
	}

void 
CPrintSettings::SetBool(ePrintSetting pSetting, bool pbValue, bool pbNotify) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	if (GetBool(pSetting)!=pbValue) {

		switch (pSetting) {

			case ePrintShowInterface:		oData.bShowInterface = pbValue;			break;
			case ePrintAlwaysShowInterface:	oData.bAlwaysShowInterface = pbValue;			break;
			case ePrintShrink:				oData.bShrinkToFit = pbValue;			break;
			case ePrintStretch:				oData.bStretchToPage = pbValue;			break;
			case ePrintPageOrientation:		oData.bPortrait = pbValue;				break;
			case ePrintMultiPagesOnPage:	oData.bMultiPagesOnPage = pbValue;		break;
			case ePrintMultipleCopiesOnPage:oData.bMultipleCopiesOnPage = pbValue;	break;

			default:
				oTrace->Add(L"Error, unknown setting requested", eAll);
				::MessageBox(NULL, L"Error, unknown settings requested", L"Copying Machine", MB_ICONERROR | MB_OK);
				break;
			}

		if (pbNotify) DoChangedSetting();
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//the following functions are the implementations of the get/set methode
//which communicate with the actual variables
void 
CPrintSettings::SetInt(ePrintSetting pSetting, unsigned int piValue) {

	oTrace->StartTrace(__WFUNCTION__, eAll);
	
	if (GetInt(pSetting)!=piValue) {

		switch (pSetting) {

			case ePrintMagnification:	oData.iMagnification = piValue; break;
			case ePrintCopies:			
				oData.iCopies = piValue;		
				break;
			case ePrintAlignment:		oData.iAlignment = piValue;		break;
			case ePrintMarginLeft:		oData.iMarginLeft = piValue;	break;
			case ePrintMarginRight:		oData.iMarginRight = piValue;	break;
			case ePrintMarginTop:		oData.iMarginTop = piValue;		break;
			case ePrintMarginBottom:	oData.iMarginBottom = piValue;	break;
			case ePrintPageType:		oData.iPageType = piValue;		break;
			
			default:
				oTrace->Add(L"Error, unknown setting requested", eAll);
				::MessageBox(NULL, L"Error, unknown settings requested", L"Copying Machine", MB_ICONERROR | MB_OK);
				break;
			}

		DoChangedSetting();
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

unsigned int 
CPrintSettings::GetInt(ePrintSetting pSetting) {

	unsigned int iReturn;
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	switch (pSetting) {

		case ePrintMagnification:	iReturn = oData.iMagnification; break;
		case ePrintCopies:			
			iReturn = oData.iCopies;		
			break;
		case ePrintAlignment:		iReturn = oData.iAlignment;		break;
		case ePrintMarginLeft:		iReturn = oData.iMarginLeft;	break;
		case ePrintMarginRight:		iReturn = oData.iMarginRight;	break;
		case ePrintMarginTop:		iReturn = oData.iMarginTop;		break;
		case ePrintMarginBottom:	iReturn = oData.iMarginBottom;	break;
		case ePrintPageType:		iReturn = oData.iPageType;		break;

		default:
			oTrace->Add(L"Error, unknown setting requested", eAll);
			::MessageBox(NULL, L"Error, unknown settings requested", L"Copying Machine", MB_ICONERROR | MB_OK);
			break;
		}

	oTrace->EndTrace(__WFUNCTION__, (int)iReturn, eAll);

	return iReturn;
	}

//Notify changes to the settings
void 
CPrintSettings::DoChangedSetting() {

	if (bNotifyChanges) oNotifications->CommunicateEvent(eNotificationPrintSettingsChanged, NULL);
	}
