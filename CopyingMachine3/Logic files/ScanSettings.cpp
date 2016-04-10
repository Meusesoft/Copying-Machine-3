#include "StdAfx.h"
#include "CopyingMachineCore.h"

CScanSettings::CScanSettings(sGlobalInstances pInstances) :
	CScriptObject(L"ScanSettings")
{
	//Set helper classes
	oTrace = pInstances.oTrace;
	oRegistry = pInstances.oRegistry;
	oNotifications = (CCoreNotifications*)pInstances.oNotifications;
	oCore = pInstances.oCopyingMachineCore;

	cAcquireType = eAcquireNewDocument;

	oScanDevice = NULL;
	bSendNotifications = true;

	//Registrations for script object
	RegisterAttribute(ScrContrast);
	RegisterAttribute(ScrBrightness);
	RegisterAttribute(ScrResolution);
	RegisterAttribute(ScrColor);
	RegisterAttribute(ScrInterface);
}

CScanSettings::~CScanSettings(void)
{
}

//Load and save the settings from the registry
void CScanSettings::Load() {

	oTrace->StartTrace(__WFUNCTION__);

	bShowInterface = (oRegistry->ReadInt(L"Scan", L"bShowInterface", 0) == 1);
	bAlwaysShowInterface = (oRegistry->ReadInt(L"Scan", L"bAlwaysShowInterface", 0) == 1);
	bShowErrorMessage = (oRegistry->ReadInt(L"Scan", L"bShowErrorMessage", 1) == 1);
	bAskForNextPage = (oRegistry->ReadInt(L"Scan", L"bAskNextPage", 1) == 1);
	bUseFeeder = (oRegistry->ReadInt(L"Scan", L"bUseFeeder", 1) == 1);
	bInvertColor = (oRegistry->ReadInt(L"Scan", L"bInvertColor", 0) == 1);
	bAssumeDPI = (oRegistry->ReadInt(L"Scan", L"bAssumeDPI", 1) == 1);
	iAssumedDPI = oRegistry->ReadInt(L"Scan", L"bAssumeDPI", 200);
	bBrightnessNegotiate = (oRegistry->ReadInt(L"Scan", L"bBrightnessNegotiate", 1) == 1);
	bBrightnessUse = (oRegistry->ReadInt(L"Scan", L"bBrightnessUse", 1) == 1);
	bContrastNegotiate = (oRegistry->ReadInt(L"Scan", L"bContrastNegotiate", 1) == 1);
	bContrastUse = (oRegistry->ReadInt(L"Scan", L"bContrastUse", 1) == 1);
	bPageNegotiate = (oRegistry->ReadInt(L"Scan", L"bPageNegotiate", 1) == 1);
	bPageUse = (oRegistry->ReadInt(L"Scan", L"bPageUse", 1) == 1);
	bOrientationNegotiate = (oRegistry->ReadInt(L"Scan", L"bOrientationNegotiate", 1) == 1);
	bOrientationUse = (oRegistry->ReadInt(L"Scan", L"bOrientationUse", 1) == 1);
	bColorDepthUse = (oRegistry->ReadInt(L"Scan", L"bColorDepthUse", 1) == 1);
	bColorDepthNegotiate = (oRegistry->ReadInt(L"Scan", L"bColorDepthNegotiate", 1) == 1);
	bResolutionNegotiate = (oRegistry->ReadInt(L"Scan", L"bResolutionNegotiate", 1) == 1);
	bResolutionUse = (oRegistry->ReadInt(L"Scan", L"bResolutionUse", 1) == 1);
	iBrightnessValue = oRegistry->ReadInt(L"Scan", L"iBrightnessValue", 0);
	iContrastValue = oRegistry->ReadInt(L"Scan", L"iContrastValue", 0);
	iPage = oRegistry->ReadInt(L"Scan", L"iPage", 0);
	iOrientation = oRegistry->ReadInt(L"Scan", L"iOrientation", 0);
	iColorDepth = oRegistry->ReadInt(L"Scan", L"iColorDepth", 0);
	iResolution = oRegistry->ReadInt(L"Scan", L"iResolution", 200);

	//(Try to) get the current scanning device
	CCopyingMachineCore* oCmCore;
	std::wstring sDevice;
	std::wstring sDeviceId;
	eScanInterface cInterface;

	oRegistry->ReadString(L"Scan", L"cScanningDevice", L"-", sDevice);
	oRegistry->ReadString(L"Scan", L"cScanningDeviceID", L"-", sDeviceId);
	cInterface = (eScanInterface)oRegistry->ReadInt(L"Scan", L"cScanningDeviceInterface", eTWAIN);

	oCmCore = (CCopyingMachineCore*)oCore;

	oScanDevice = oCmCore->oScanCore->GetDevice(cInterface, sDevice, sDeviceId);

	oTrace->EndTrace(__WFUNCTION__);
}

void CScanSettings::Save() {

	oTrace->StartTrace(__WFUNCTION__);

	oRegistry->WriteInt(L"Scan", L"bShowInterface", bShowInterface ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bAlwaysShowInterface", bAlwaysShowInterface ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bShowErrorMessage", bShowErrorMessage ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bBrightnessNegotiate", bBrightnessNegotiate ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bBrightnessUse", bBrightnessUse ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"iBrightnessValue", iBrightnessValue);
	oRegistry->WriteInt(L"Scan", L"bContrastNegotiate", bContrastNegotiate ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bContrastUse", bContrastUse ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"iContrastValue", iContrastValue);
	oRegistry->WriteInt(L"Scan", L"bPageNegotiate", bPageNegotiate ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bPageUse", bPageUse ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"iPage", iPage);
	oRegistry->WriteInt(L"Scan", L"bOrientationNegotiate", bOrientationNegotiate ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bOrientationUse", bOrientationUse ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"iOrientation", iOrientation);
	oRegistry->WriteInt(L"Scan", L"bColorDepthNegotiate", bColorDepthNegotiate ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bColorDepthUse", bColorDepthUse ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"iColorDepth", iColorDepth);
	oRegistry->WriteInt(L"Scan", L"bResolutionNegotiate", bResolutionNegotiate ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bResolutionUse", bResolutionUse ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"iResolution", iResolution);
	oRegistry->WriteString(L"Scan", L"cScanningDevice", oScanDevice->cScanningDevice);
	oRegistry->WriteString(L"Scan", L"cScanningDeviceID", oScanDevice->cScanningDeviceID);
	oRegistry->WriteInt(L"Scan", L"cScanningDeviceInterface", oScanDevice->cInterface);
	oRegistry->WriteInt(L"Scan", L"bAskNextPage", bAskForNextPage ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bUseFeeder", bUseFeeder ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bInvertColor", bInvertColor ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"bAssumeDPI", bAssumeDPI ? 1 : 0);
	oRegistry->WriteInt(L"Scan", L"iAssumedDPI", iAssumedDPI);

	oTrace->EndTrace(__WFUNCTION__);
}

//Make a copy of the settings from another instance
void CScanSettings::Copy(CScanSettings* poSettings) {

	oTrace->StartTrace(__WFUNCTION__);

	bShowInterface = poSettings->GetBool(eShowInterface);
	bAlwaysShowInterface = poSettings->GetBool(eAlwaysShowInterface);
	bShowErrorMessage = poSettings->GetBool(eShowErrorMessage);
	bAskForNextPage = poSettings->GetBool(eAskScanNextPage);
	bUseFeeder = poSettings->GetBool(eUseFeeder);
	bInvertColor = poSettings->GetBool(eInvertBlackWhite);
	bAssumeDPI = poSettings->GetBool(eAssumeDPI);
	iAssumedDPI = poSettings->GetInt(eAssumeDPI);
	bBrightnessNegotiate = poSettings->GetBool(eBrightnessNegotiate);
	bBrightnessUse = poSettings->GetBool(eBrightnessUse);
	bContrastNegotiate = poSettings->GetBool(eContrastNegotiate);
	bContrastUse = poSettings->GetBool(eContrastUse);
	bPageNegotiate = poSettings->GetBool(ePageNegotiate);
	bPageUse = poSettings->GetBool(ePageUse);
	bOrientationNegotiate = poSettings->GetBool(eOrientationNegotiate);
	bOrientationUse = poSettings->GetBool(eOrientationUse);
	bColorDepthUse = poSettings->GetBool(eColorDepthUse);
	bColorDepthNegotiate = poSettings->GetBool(eColorDepthNegotiate);
	bResolutionNegotiate = poSettings->GetBool(eResolutionNegotiate);
	bResolutionUse = poSettings->GetBool(eResolutionUse);
	iBrightnessValue = poSettings->GetInt(eBrightnessValue);
	iContrastValue = poSettings->GetInt(eContrastValue);
	iPage = poSettings->GetInt(ePageValue);
	iOrientation = poSettings->GetInt(eOrientationValue);
	iColorDepth = poSettings->GetInt(eColorDepthValue);
	iResolution = poSettings->GetInt(eResolutionValue);

	cAcquireType = poSettings->cAcquireType;
	iCollection = poSettings->iCollection;
	sFolder = poSettings->sFolder;

	//(Try to) get the current scanning device
	oScanDevice = poSettings->GetScanner();

	oTrace->EndTrace(__WFUNCTION__);
}

//the following functions are the implementations of the get/set methode
//which communicate with the actual variables
void 
CScanSettings::SetInt(eScanSetting pSetting, unsigned int piValue) {

	oTrace->StartTrace(__WFUNCTION__, eAll);
	
	if (GetInt(pSetting)!=piValue) {

		switch (pSetting) {

			case eBrightnessValue:	iBrightnessValue = piValue; break;
			case eContrastValue:	iContrastValue = piValue; break;
			case ePageValue:		iPage = piValue; break;
			case eOrientationValue:	iOrientation = piValue; break;
			case eColorDepthValue:	iColorDepth = piValue; break;
			case eResolutionValue:	iResolution = piValue; break;
			case eAssumeDPI:		iAssumedDPI = piValue; break;
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
CScanSettings::GetInt(eScanSetting pSetting) {

	unsigned int iReturn;
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	switch (pSetting) {

		case eBrightnessValue:	iReturn = iBrightnessValue; break;
		case eContrastValue:	iReturn = iContrastValue; break; 
		case ePageValue:		iReturn = iPage; break;
		case eOrientationValue:	iReturn = iOrientation; break;
		case eColorDepthValue:	iReturn = iColorDepth; break;
		case eResolutionValue:	iReturn = iResolution; break;
		case eAssumeDPI:		iReturn = iAssumedDPI; break;
		default:
			oTrace->Add(L"Error, unknown setting requested", eAll);
			::MessageBox(NULL, L"Error, unknown settings requested", L"Copying Machine", MB_ICONERROR | MB_OK);
			break;
		}

	oTrace->EndTrace(__WFUNCTION__, (int)iReturn, eAll);

	return iReturn;
	}

void 
CScanSettings::SetBool(eScanSetting pSetting, bool pbValue, bool pbNotify) {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	if (GetBool(pSetting)!=pbValue) {

		switch (pSetting) {

			case eBrightnessNegotiate:	bBrightnessNegotiate = pbValue; break;	
			case eBrightnessUse:		bBrightnessUse = pbValue; break;
			case eContrastNegotiate:	bContrastNegotiate = pbValue; break;
			case eContrastUse:			bContrastUse = pbValue; break;
			case ePageNegotiate:		bPageNegotiate = pbValue; break;
			case ePageUse:				bPageUse = pbValue; break;
			case eOrientationNegotiate:	bOrientationNegotiate = pbValue; break;
			case eOrientationUse:		bOrientationUse = pbValue; break;
			case eColorDepthNegotiate:	bColorDepthNegotiate = pbValue; break;
			case eColorDepthUse:		bColorDepthUse = pbValue; break;
			case eColorInvert:			bInvertColor = pbValue; break;
			case eResolutionNegotiate:	bResolutionNegotiate = pbValue; break;
			case eResolutionUse:		bResolutionUse = pbValue; break;
			case eAssumeDPI:			bAssumeDPI = pbValue; break;
			case eShowErrorMessage:		bShowErrorMessage = pbValue; break;
			case eAlwaysShowInterface:	bAlwaysShowInterface = pbValue; break;
			case eAskScanNextPage:		bAskForNextPage = pbValue; break;
			case eUseFeeder:			bUseFeeder = pbValue; break;
			case eInvertBlackWhite:		bInvertColor = pbValue; break;
			case eShowInterface:		bShowInterface = pbValue; break;
			default:
				oTrace->Add(L"Error, unknown setting requested", eAll);
				::MessageBox(NULL, L"Error, unknown settings requested", L"Copying Machine", MB_ICONERROR | MB_OK);
				break;
			}

		if (pbNotify) DoChangedSetting();
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

bool 
CScanSettings::GetBool(eScanSetting pSetting) {

	bool bReturn;

	bReturn = false;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	switch (pSetting) {

		case eBrightnessNegotiate:	bReturn = bBrightnessNegotiate; break;
		case eBrightnessUse:		bReturn = bBrightnessUse; break;
		case eContrastNegotiate:	bReturn = bContrastNegotiate; break;
		case eContrastUse:			bReturn = bContrastUse; break;
		case ePageNegotiate:		bReturn = bPageNegotiate; break;
		case ePageUse:				bReturn = bPageUse; break;
		case eOrientationNegotiate:	bReturn = bOrientationNegotiate; break;
		case eOrientationUse:		bReturn = bOrientationUse; break;
		case eColorDepthNegotiate:	bReturn = bColorDepthNegotiate; break;
		case eColorDepthUse:		bReturn = bColorDepthUse; break;
		case eColorInvert:			bReturn = bInvertColor; break;
		case eResolutionNegotiate:	bReturn = bResolutionNegotiate; break;
		case eResolutionUse:		bReturn = bResolutionUse; break;
		case eAssumeDPI:			bReturn = bAssumeDPI; break;
		case eShowErrorMessage:		bReturn = bShowErrorMessage; break;
		case eAlwaysShowInterface:	bReturn = bAlwaysShowInterface; break;
		case eAskScanNextPage:		bReturn = bAskForNextPage; break;
		case eUseFeeder:			bReturn = bUseFeeder; break;
		case eInvertBlackWhite:		bReturn = bInvertColor; break;
		case eShowInterface:		bReturn = bShowInterface; break;

		default:
			oTrace->Add(L"Error, unknown setting requested", eAll);
			::MessageBox(NULL, L"Error, unknown settings requested", L"Copying Machine", MB_ICONERROR | MB_OK);
			break;
		}

	oTrace->EndTrace(__WFUNCTION__, bReturn, eAll);

	return bReturn;
	}

//This function gets the value from the request objectname
sScriptExpression 
CScanSettings::Get(std::wstring psObjectname) {

	sScriptExpression oResult;

	oResult = CScriptObject::Get(psObjectname);

	if (oResult.cValueType == eScriptValueAny &&
		QueryType(psObjectname) == eScriptObjectAttribute) { 

		if (psObjectname == ScrContrast) {

			oResult.iResult = GetInt(eContrastValue);
			oResult.cValueType = eScriptValueInteger;
			}

		if (psObjectname == ScrBrightness) {

			oResult.iResult = GetInt(eBrightnessValue);
			oResult.cValueType = eScriptValueInteger;
			}

		if (psObjectname == ScrResolution) {

			oResult.iResult = GetInt(eResolutionValue);
			oResult.cValueType = eScriptValueInteger;
			}

		if (psObjectname == ScrInterface) {

			oResult.bResult = GetBool(eShowInterface);
			oResult.cValueType = eScriptValueBoolean;
			}

		if (psObjectname == ScrColor) {

			oResult.iResult = GetInt(eColorDepthValue);
			oResult.cValueType = eScriptValueInteger;
			}
		}

	return oResult;
	}

//This function sets the value of the given attribute
bool 
CScanSettings::Set(std::wstring psObjectname, sScriptExpression psNewValue) {

	bool bResult;

	if (psObjectname == ScrBrightness) {

		if (psNewValue.isNumber()) {
			SetInt(eContrastValue, (int)psNewValue.GetNumber());
			bResult = true;
			}
		}

	if (psObjectname == ScrBrightness) {

		if (psNewValue.isNumber()) {
			SetInt(eBrightnessValue, (int)psNewValue.GetNumber());
			bResult = true;
			}
		}

	if (psObjectname == ScrResolution) {

		if (psNewValue.isNumber()) {
			SetInt(eResolutionValue, (int)psNewValue.GetNumber());
			bResult = true;
			}
		}

	if (psObjectname == ScrColor) {

		if (psNewValue.isInteger()) {
			SetInt(eColorDepthValue, (int)psNewValue.iResult);
			bResult = true;
			}
		}

	if (psObjectname == ScrInterface) {

		if (psNewValue.isBoolean()) {
			SetBool(eShowInterface, (bool)psNewValue.bResult);
			bResult = true;
			}
		}

	return bResult;
	}


void 
CScanSettings::DoChangedSetting() {

	if (bSendNotifications) oNotifications->CommunicateEvent(eNotificationScanSettingsChanged, NULL);
	}

//This function sets the default scanner and sends a notification
//if it has changed.
bool 
CScanSettings::SetScanner(CScanDevice* poScanDevice) {

	oTrace->StartTrace(__WFUNCTION__);

	if (oScanDevice==NULL) {

		oScanDevice = poScanDevice;
		if (bSendNotifications) oNotifications->CommunicateEvent(eNotificationScanDeviceChanged, NULL);
		}

	if (poScanDevice->cScanningDeviceID!=oScanDevice->cScanningDeviceID) {

		oScanDevice = poScanDevice;

		//cScanDevice.cScanningDevice = pcScanDevice.cScanningDevice;
		//cScanDevice.cScanningDeviceID = pcScanDevice.cScanningDeviceID;
		//cScanDevice.cInterface = pcScanDevice.cInterface;

		if (bSendNotifications) oNotifications->CommunicateEvent(eNotificationScanDeviceChanged, NULL);
		}

	oTrace->EndTrace(__WFUNCTION__);

	return true;
}

//This function gets the default scanner
CScanDevice* 
CScanSettings::GetScanner() {

	return oScanDevice;
	}
