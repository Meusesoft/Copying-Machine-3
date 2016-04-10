#include "StdAfx.h"
#include "CopyingMachineCore.h"
#define _USE_MATH_DEFINES
#include <cmath>


CCopyDocumentPage::CCopyDocumentPage(sGlobalInstances pInstances, int piDocumentID, int piPageID) :
	CScriptObject(L"CopyDocumentPage")
{
	//Set helper classes
	oTrace = pInstances.oTrace;
	
	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pInstances, sizeof(sGlobalInstances));


	//Init properties
	ZeroMemory(&oProperties, sizeof(oProperties));

	oProperties.bDirty				= false;
	oProperties.cSelection.bEnabled = false;
	oProperties.cSelection.bOuter   = true;
	oProperties.oRecognitionLayer   = new CRecognitionLayer();

	/*std::wstring wsText = L"AëC";
	CRecognizedText* oText = oProperties.oRecognitionLayer->CreateRecognizedText(Rect(10,10,100,100), LANG_ENGLISH);
	oText->AddCharacter(wsText[0], Rect(30,10,10,80), 0, 0);
	oText->AddCharacter(wsText[1], Rect(30,10,10,80), 0, 0);
	oText->AddCharacter(wsText[2], Rect(30,10,10,80), 1, 1);*/


	SetPageID(piDocumentID, piPageID);

	//Script
	RegisterAttribute(L"id", true);
}

CCopyDocumentPage::~CCopyDocumentPage(void)
{
	CCopyingMachineCore* oCore;
	
	oTrace->StartTrace(__WFUNCTION__);

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	oCore->oNotifications->CommunicateEvent(eNotificationPageDeleted, (void*)oProperties.dPageID);

	//free the memory and delete instances
	delete oProperties.oImage;
	delete oProperties.oRecognitionLayer;
	if (oProperties.pMetadata!=NULL) free(oProperties.pMetadata);

	//Clear the undo/redo queue
	oCore->oUndoRedo->ClearUndoRedo(oProperties.dPageID);

	oTrace->EndTrace(__WFUNCTION__);
}

//This function sets the image its memory handle
void
CCopyDocumentPage::SetImage(HGLOBAL phImage, bool pbUpdateMetadata) {

	bool bGetMetadata;

	bGetMetadata = true;

	//delete the current image if it exists
	if (oProperties.oImage!=NULL) {
		delete oProperties.oImage;
		bGetMetadata = false;
		}

	//Set the new image
	oProperties.oImage = CBitmap::GetFromHandle(phImage);

	//If it was a new image then update the metadata. If not then
	//get the metadata
	if (bGetMetadata && pbUpdateMetadata) {

		GetMetadata();
		}
	else {
		
		SetMetadata();
		}
	}

//This function sets the image its memory handle
void
CCopyDocumentPage::SetImage(Bitmap* poBitmap, bool pbUpdateMetadata) {

	bool bGetMetadata;

	bGetMetadata = true;

	//delete the current image if it exists
	if (oProperties.oImage!=NULL) {
		delete oProperties.oImage;
		bGetMetadata = false;
		}

	//Set the new image
	oProperties.oImage = new CBitmap(poBitmap, NULL);

	//If it was a new image then update the metadata. If not then
	//get the metadata
	if (bGetMetadata && pbUpdateMetadata) {

		GetMetadata();
		}
	else {
		
		SetMetadata();
		}
	}

//This function sets the image its memory handle
void
CCopyDocumentPage::SetImage(CBitmap* poImage, bool pbUpdateMetadata) {

	bool bGetMetadata;

	bGetMetadata = true;

	//delete the current image if it exists
	if (oProperties.oImage!=NULL) {
		delete oProperties.oImage;
		bGetMetadata = false;
		}

	//Set the new image
	oProperties.oImage = poImage;	

	//If it was a new image then update the metadata. If not then
	//get the metadata
	if (bGetMetadata && pbUpdateMetadata) {

		GetMetadata();
		}
	else {
		
		SetMetadata();
		}
	}

//This function switches the image of the page (it is primarily used in undo/redo)
CBitmap*
CCopyDocumentPage::SwapImage(CBitmap* poImage) {
	
	CBitmap* oResultImage;

	oResultImage = oProperties.oImage;
	oProperties.oImage = poImage;	
	
	return oResultImage;
	}

//This function sets the image its memory handle
CBitmap*
CCopyDocumentPage::GetImage() {

	return oProperties.oImage;
	}

//This function returns the GDI+ Bitmap, if it exists
Bitmap* 
CCopyDocumentPage::GetBitmap() {

	Bitmap* oResult;

	oResult = NULL;

	if (GetImage()!=NULL) {

		oResult = GetImage()->GetBitmap();
		}

	return oResult;
}


//This function retrieves the resolution of the page
int
CCopyDocumentPage::GetResolutionX() {

	int iResult;

	iResult = 0;

	if (oProperties.oImage!=NULL) {

		iResult = (int)oProperties.oImage->GetBitmap()->GetHorizontalResolution();
		}

	return iResult;
	}

//This function set the resolution property of the page
void 
CCopyDocumentPage::SetResolutionX(int piResolutionX) {

	CCopyingMachineCore* oCore;
	Bitmap* oBitmap;
	

	if (piResolutionX != GetResolutionX()) {

		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		oBitmap = GetBitmap();
		
		//Make an undo point before making changes
		oCore->oUndoRedo->MakeUndoPoint(this, eImageChangeDPIX, 0);

		if (piResolutionX<1 || piResolutionX>9999) {
		
			//Try the resolution inside the image
			if (oBitmap!=NULL) piResolutionX = (int)oBitmap->GetHorizontalResolution();
			}

		if (piResolutionX<1 || piResolutionX>9999) {

			//Use the assumed DPI
			piResolutionX = oCore->oScanSettings->GetInt(eAssumeDPI);
			}


		if (oBitmap!=NULL) {

			oBitmap->SetResolution((REAL)piResolutionX, (REAL)oBitmap->GetVerticalResolution());

			SetDirty(true);
			
			//Send notification of changes
			oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
			oCore->oNotifications->CommunicateEvent(eNotificationPageSettingUpdate, (void*)oProperties.dPageID);
			}
		}
	}

//This function retrieves the resolution of the page
int
CCopyDocumentPage::GetResolutionY() {

	int iResult;

	iResult = 0;

	if (oProperties.oImage!=NULL) {

		iResult = (int)oProperties.oImage->GetBitmap()->GetVerticalResolution();
		}

	return iResult;
	}

//This function set the resolution property of the page
void 
CCopyDocumentPage::SetResolutionY(int piResolutionY) {

	CCopyingMachineCore* oCore;
	Bitmap* oBitmap;

	if (piResolutionY != GetResolutionY()) {
		
		oBitmap = GetBitmap();
		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

		//Make an undo point before making changes
		oCore->oUndoRedo->MakeUndoPoint(this, eImageChangeDPIY, 0);

		if (piResolutionY<1 || piResolutionY>9999) {
		
			//Try the resolution inside the image
			if (oBitmap!=NULL) piResolutionY = (int)oBitmap->GetVerticalResolution();
			}

		if (piResolutionY<1 || piResolutionY>9999) {
		
			//Use the assumed DPI
			piResolutionY = oCore->oScanSettings->GetInt(eAssumeDPI);
			}

		if (oBitmap!=NULL) {

			oBitmap->SetResolution((REAL)oBitmap->GetHorizontalResolution(), (REAL)piResolutionY);

			SetDirty(true);
			
			//Send notification of changes
			oCore->oNotifications->CommunicateEvent(eNotificationPageSettingUpdate, (void*)oProperties.dPageID);
			}
		}
	}

void
CCopyDocumentPage::SetPageID(int piDocumentID, int piPageID) {

	DWORD dPageID;

	dPageID = MAKELONG(piPageID, piDocumentID);

	oProperties.dPageID = dPageID;
	oProperties.iDocumentID = piDocumentID;
	}

int 
CCopyDocumentPage::GetDocumentID() {

	return oProperties.iDocumentID;
	}

DWORD
CCopyDocumentPage::GetPageID() {

	return oProperties.dPageID;
	}

//These two functions get and set the Dirty flag.
bool
CCopyDocumentPage::GetDirty() {

	return oProperties.bDirty;
	}

void
CCopyDocumentPage::SetDirty(bool pbDirty) {

	oProperties.bDirty = pbDirty;
	}

//This function returns all the properties of this page
sCopyDocumentPageProperties 
CCopyDocumentPage::GetAllProperties() {

	return oProperties;
	}

//This function returns the current selection on the image
void 
CCopyDocumentPage::GetSelection(sCopyDocumentSelection &pcSelection) {

	pcSelection.bEnabled = oProperties.cSelection.bEnabled;
	pcSelection.bOuter   = oProperties.cSelection.bOuter;
	pcSelection.fBottom  = oProperties.cSelection.fBottom;
	pcSelection.fLeft	 = oProperties.cSelection.fLeft;
	pcSelection.fRight   = oProperties.cSelection.fRight;
	pcSelection.fTop	 = oProperties.cSelection.fTop;
	}

//This function returns the current selection on the image	
void 
CCopyDocumentPage::SetSelection(sCopyDocumentSelection pcSelection) {

	CCopyingMachineCore* oCore;

	//Send notification of changes
	if ((oProperties.cSelection.bEnabled != pcSelection.bEnabled) ||
		(oProperties.cSelection.bOuter	 != pcSelection.bOuter)) {
		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		oCore->oNotifications->CommunicateEvent(eNotificationPageSettingUpdate, (void*)oProperties.dPageID);
		}

	//Update the selection
	//The min/max function are used to normalize the selection. This means
	//that TopLeft is really TopLeft and BottomRight is BottomRight.
	oProperties.cSelection.bEnabled	= pcSelection.bEnabled;
	oProperties.cSelection.bOuter	= pcSelection.bOuter;
	oProperties.cSelection.fBottom	= max(pcSelection.fBottom, pcSelection.fTop);
	oProperties.cSelection.fLeft	= min(pcSelection.fLeft,   pcSelection.fRight);
	oProperties.cSelection.fRight   = max(pcSelection.fLeft,   pcSelection.fRight);
	oProperties.cSelection.fTop		= min(pcSelection.fBottom, pcSelection.fTop);
	}



//This function rotates the current selection. The selection is rotated
//around the center of the image. This operation is performed before the
//page itself is rotated.
void 
CCopyDocumentPage::DoRotateSelection(DWORD pdRotate) {

	sCopyDocumentSelection cSelection;
	RectF	  cPageRectangle;
	sMathRect sSelection;

	//Get the current selection
	GetSelection(cSelection);

	//Convert selection to a sMathRect
	sSelection.left		= cSelection.fLeft;
	sSelection.right	= cSelection.fRight;
	sSelection.top		= cSelection.fTop;
	sSelection.bottom	= cSelection.fBottom;

	//Do the rotation
	cPageRectangle = RectF(0, 0, (float)(oProperties.oImage->GetBitmap()->GetWidth()), (float)(oProperties.oImage->GetBitmap()->GetHeight()));
	CMathVector::RotateRect(sSelection, cPageRectangle, pdRotate);

	//Convert result back to selection
	cSelection.fLeft	= sSelection.left;
	cSelection.fRight	= sSelection.right;
	cSelection.fTop		= sSelection.top;
	cSelection.fBottom	= sSelection.bottom;

	//Update the selection
	SetSelection(cSelection);
	}

//This function rotates the current selection. The selection is rotated
//around the center of the image. This operation is performed before the
//page itself is rotated.
void 
CCopyDocumentPage::DoFlipSelection(DWORD pdFlip) {

	sCopyDocumentSelection cSelection;
	RectF	  cPageRectangle;
	sMathRect sSelection;

	//Get the current selection
	GetSelection(cSelection);

	//Convert selection to a sMathRect
	sSelection.left		= cSelection.fLeft;
	sSelection.right	= cSelection.fRight;
	sSelection.top		= cSelection.fTop;
	sSelection.bottom	= cSelection.fBottom;

	//Do the rotation
	cPageRectangle = RectF(0, 0, (float)(oProperties.oImage->GetBitmap()->GetWidth()), (float)(oProperties.oImage->GetBitmap()->GetHeight()));
	CMathVector::MirrorRect(sSelection, cPageRectangle, pdFlip);

	//Convert result back to selection
	cSelection.fLeft	= sSelection.left;
	cSelection.fRight	= sSelection.right;
	cSelection.fTop		= sSelection.top;
	cSelection.fBottom	= sSelection.bottom;

	//Update the selection
	SetSelection(cSelection);
	}

//This function returns the instance of the recognition layer of this page
CRecognitionLayer* 
CCopyDocumentPage::GetRecognitionLayer() {

	return oProperties.oRecognitionLayer;
	}

//This function sets a new recognition layer
void
CCopyDocumentPage::SetRecognitionLayer(CRecognitionLayer *poLayer) {

	delete oProperties.oRecognitionLayer;
	oProperties.oRecognitionLayer = poLayer;
}


//This function checks if the given action can be performed on thie
//image of this page.
bool 
CCopyDocumentPage::CheckPageImageAction(ePageImageAction pcAction, DWORD pdData) {

	bool bResult;

	bResult = false;

	switch (pcAction) {

		case eImageCut:
			bResult = (oProperties.cSelection.bEnabled && 
						!oProperties.cSelection.bOuter &&
						oProperties.oImage!=NULL);
			break;

		case eImageCrop:
		case eImageFillRectangle:
		case eImageCopy:

			bResult = (oProperties.cSelection.bEnabled && oProperties.oImage!=NULL);
			break;

		case eImageRotate:
		case eImageFlipHorizontal:
		case eImageFlipVertical:

			bResult = (oProperties.oImage!=NULL);
			break;

		case eImageGrayscale:

			if (oProperties.oImage!=NULL) {

				if (oProperties.oImage->GetBitmap()->GetPixelFormat() != PixelFormat1bppIndexed &&
					oProperties.oImage->GetBitmap()->GetPixelFormat() != PixelFormat8bppIndexed) {
				
					bResult = true;
					}	
				}
			break;

		case eImageBlackWhite:

			if (oProperties.oImage!=NULL) {

				if (oProperties.oImage->GetBitmap()->GetPixelFormat() != PixelFormat1bppIndexed) {
				
					bResult = true;
					}	
				}
			break;
		}

	return bResult;
}

//This function copies the selection
eActionResult
CCopyDocumentPage::DoCopyImage(HWND phWnd) {

	eActionResult cResult;
	RectF		  cRectangle;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;

	if (CheckPageImageAction(eImageCopy, 0)) {

		cRectangle.X =		oProperties.cSelection.fLeft;
		cRectangle.Y =		oProperties.cSelection.fTop;
		cRectangle.Width =	oProperties.cSelection.fRight - oProperties.cSelection.fLeft;
		cRectangle.Height = oProperties.cSelection.fBottom - oProperties.cSelection.fTop;

		cResult = oProperties.oImage->CopyToClipboard(phWnd, cRectangle);
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
}

//This function cuts the selection
eActionResult
CCopyDocumentPage::DoCutImage(DWORD dColor, HWND phWnd) {

	eActionResult cResult;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;

	if (CheckPageImageAction(eImageCut, dColor)) {
		cResult = DoCopyImage(phWnd);
		if (cResult != eImageActionError) {
			cResult = DoFillRectangle(dColor);
			}
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
}

//This funciton crops the image based on the current selection
eActionResult 
CCopyDocumentPage::DoCropImage() {

	CCopyingMachineCore* oCore;
	Bitmap*				 oCropBitmap;
	eActionResult		 cResult;
	Status				 cGDIStatus;
	RectF				 cCropRectangle;

	oTrace->StartTrace(__WFUNCTION__);
	
	cResult = eImageActionError;

	try {
	
		if (CheckPageImageAction(eImageCrop, 0)) {

			//Create a new image with the different dimension, but the same resolution
			cCropRectangle.X = oProperties.cSelection.fLeft;
			cCropRectangle.Y = oProperties.cSelection.fTop;
			cCropRectangle.Width = oProperties.cSelection.fRight - oProperties.cSelection.fLeft;
			cCropRectangle.Height = oProperties.cSelection.fBottom - oProperties.cSelection.fTop;
				
			oCropBitmap = new Bitmap((INT)(cCropRectangle.Width),
									 (INT)(cCropRectangle.Height),
									 PixelFormat24bppRGB);
			oCropBitmap->SetResolution(oProperties.oImage->GetBitmap()->GetHorizontalResolution(),
									   oProperties.oImage->GetBitmap()->GetVerticalResolution());

			//Draw the image to the cropped image
			Graphics* oCanvas;
			
			oCanvas = new Graphics(oCropBitmap);
			cGDIStatus = oCanvas->DrawImage(oProperties.oImage->GetBitmap(),
							   (INT)0, (INT)0,
							   (INT)cCropRectangle.X,
							   (INT)cCropRectangle.Y,
							   (INT)cCropRectangle.Width,
							   (INT)cCropRectangle.Height,
							   (Gdiplus::Unit)UnitPixel);
			delete oCanvas;

			if (cGDIStatus==Ok) {
			
				//If we had to upgrade the image to 24bpp, convert it back to its
				//previous pixelformat
				if (oProperties.oImage->GetBitmap()->GetPixelFormat()!=
					oCropBitmap->GetPixelFormat()) {
					
					Bitmap* oNewBitmap;

					oNewBitmap = CBitmap::ConvertBitmap(oCropBitmap, oProperties.oImage->GetBitmap()->GetPixelFormat());
					delete oCropBitmap;

					oCropBitmap = oNewBitmap;				
					}

				oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
				
				//Make an undo point before making changes
				if (oCore->oUndoRedo->MakeUndoPoint(this, eImageCrop, 0)) {

					//The undo point was succesful, set the oImage to NULL to prevent it from
					//being deleted by SetImage
					oProperties.oImage = NULL;
					}

				SetImage(oCropBitmap, false);

				//Disable the selection, it has become invalid after the give
				//changes
				oProperties.cSelection.bEnabled = false;

				//Adjust the text blocks in the recognition layer
				oProperties.oRecognitionLayer->DoCrop(cCropRectangle);

				//Send notification of changes
				oCore->oNotifications->CommunicateEvent(eNotificationPageUpdate, (void*)oProperties.dPageID);
				oCore->oNotifications->CommunicateEvent(eNotificationPageSettingUpdate, (void*)oProperties.dPageID);
				
				//Make the page dirty
				SetDirty(true);

				cResult = eOk;
				}
			else {

				delete oCropBitmap;
				}
			}
		}
	catch (...) {

		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}

//This function rotates the image by the dRotation number of degrees
eActionResult 
CCopyDocumentPage::DoRotateImage(DWORD dRotation) {

	CCopyingMachineCore* oCore;
	Status cGDIStatus;
	eActionResult cResult;	
	RectF cPageRectangle;

	oTrace->StartTrace(__WFUNCTION__);

	
	cResult = eImageActionError;
	cGDIStatus = Aborted;

	if (CheckPageImageAction(eImageRotate, dRotation)) {

		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

		//Make undo point
		oCore->oUndoRedo->MakeUndoPoint(this, eImageRotate, dRotation);

		//Do the rotation
		cPageRectangle = RectF(0, 0, (REAL)oProperties.oImage->GetBitmap()->GetWidth(), (REAL)oProperties.oImage->GetBitmap()->GetHeight()); 
		DoRotateSelection(dRotation);

		switch (dRotation) {

			case 90:

				cGDIStatus = oProperties.oImage->GetBitmap()->RotateFlip(Rotate90FlipNone);
				break;

			case 180:

				cGDIStatus = oProperties.oImage->GetBitmap()->RotateFlip(Rotate180FlipNone);
				break;
		
			case 270:

				cGDIStatus = oProperties.oImage->GetBitmap()->RotateFlip(Rotate270FlipNone);
				break;
			}

		if (cGDIStatus==Ok) {

			//remove current thumbnail
			oProperties.oImage->ClearThumbnail();

			//Send notification of changes
			oCore->oNotifications->CommunicateEvent(eNotificationPageUpdate, (void*)oProperties.dPageID);
			oCore->oNotifications->CommunicateEvent(eNotificationPageSettingUpdate, (void*)oProperties.dPageID);
			
			//Clear the recognition layer
			oProperties.oRecognitionLayer->DoRotate(dRotation, cPageRectangle);

			//Set the dirty flag
			SetDirty(true);

			cResult = eOk;
			}
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}

//This function flips the image vertically or horizontally
eActionResult 
CCopyDocumentPage::DoFlipImage(DWORD dFlip) {

	CCopyingMachineCore* oCore;
	Status cGDIStatus;
	eActionResult cResult;	
	RectF cPageRectangle;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;
	cGDIStatus = Aborted;

	if (CheckPageImageAction(dFlip==0 ? eImageFlipHorizontal : eImageFlipVertical, dFlip)) {

		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		
		//Make undo point
		oCore->oUndoRedo->MakeUndoPoint(this, dFlip==0 ? eImageFlipHorizontal : eImageFlipVertical, dFlip);

		//Do the flip
		cPageRectangle = RectF(0, 0, (REAL)oProperties.oImage->GetBitmap()->GetWidth(), (REAL)oProperties.oImage->GetBitmap()->GetHeight()); 
		DoFlipSelection(dFlip);
		
		if (dFlip==0) {

			cGDIStatus = oProperties.oImage->GetBitmap()->RotateFlip(RotateNoneFlipY);
			}
		else {

			cGDIStatus = oProperties.oImage->GetBitmap()->RotateFlip(RotateNoneFlipX);
			}
		
		if (cGDIStatus==Ok) {

			//remove current thumbnail
			oProperties.oImage->ClearThumbnail();

			//Clear the recognition layer
			oProperties.oRecognitionLayer->DoFlip(dFlip, cPageRectangle);

			//Send notification of changes
			oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
			oCore->oNotifications->CommunicateEvent(eNotificationPageUpdate, (void*)oProperties.dPageID);
		
			//Set the dirty flag
			SetDirty(true);

			cResult = eOk;
			}	
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}



//This function fills the current selection with the given color, the color is a colorref
eActionResult 
CCopyDocumentPage::DoFillRectangle(DWORD dColor) {

	CCopyingMachineCore* oCore;
	eActionResult cResult;
	Rect cFillRectangle;
	
	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;

	if (CheckPageImageAction(eImageFillRectangle, dColor)) {

		//Make an undo point
		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		oCore->oUndoRedo->MakeUndoPoint(this, eImageFillRectangle, dColor);

		cResult = eOk;

		if (oProperties.cSelection.bOuter) {

			//Top
			if (cResult == eOk && oProperties.cSelection.fTop>0) {

				cFillRectangle = Rect( (INT)0,
									   (INT)0,
									   (INT)(oProperties.oImage->GetBitmap()->GetWidth()),
									   (INT)(oProperties.cSelection.fTop));

				cResult = oProperties.oImage->FillRectangle(dColor, cFillRectangle);

				//Adjust the text blocks in the recognition layer
				if (cResult == eOk) oProperties.oRecognitionLayer->DoFill(cFillRectangle);
				}

			//Bottom
			if (cResult == eOk && oProperties.cSelection.fBottom<oProperties.oImage->GetBitmap()->GetHeight()) {
				
				cFillRectangle = Rect( (INT)0,
									   (INT)(oProperties.cSelection.fBottom),
									   (INT)(oProperties.oImage->GetBitmap()->GetWidth()),
									   (INT)(oProperties.oImage->GetBitmap()->GetHeight() - oProperties.cSelection.fBottom));

				cResult = oProperties.oImage->FillRectangle(dColor, cFillRectangle);

				//Adjust the text blocks in the recognition layer
				if (cResult == eOk) oProperties.oRecognitionLayer->DoFill(cFillRectangle);
				}

			//Left
			if (cResult == eOk && oProperties.cSelection.fLeft>0) {
				
				cFillRectangle = Rect(  (INT)0,
										(INT)oProperties.cSelection.fTop,
										(INT)oProperties.cSelection.fLeft,
										(INT)(oProperties.cSelection.fBottom - oProperties.cSelection.fTop));
				
				cResult = oProperties.oImage->FillRectangle(dColor, cFillRectangle);

				//Adjust the text blocks in the recognition layer
				if (cResult == eOk) oProperties.oRecognitionLayer->DoFill(cFillRectangle);
				}

			//Right
			if (cResult == eOk && oProperties.cSelection.fRight<oProperties.oImage->GetBitmap()->GetWidth()) {
				
				cFillRectangle = Rect(  (INT)oProperties.cSelection.fRight,
										(INT)oProperties.cSelection.fTop,
										(INT)(oProperties.oImage->GetBitmap()->GetWidth()-oProperties.cSelection.fRight),
										(INT)(oProperties.cSelection.fBottom - oProperties.cSelection.fTop));
				
				cResult = oProperties.oImage->FillRectangle(dColor, cFillRectangle);

				//Adjust the text blocks in the recognition layer
				if (cResult == eOk) oProperties.oRecognitionLayer->DoFill(cFillRectangle);
				}
			}
		else {
					
			cFillRectangle = Rect(  (INT)oProperties.cSelection.fLeft,
									(INT)oProperties.cSelection.fTop,
									(INT)(oProperties.cSelection.fRight - oProperties.cSelection.fLeft),
									(INT)(oProperties.cSelection.fBottom - oProperties.cSelection.fTop));
				
			cResult = oProperties.oImage->FillRectangle(dColor, cFillRectangle);

			//Adjust the text blocks in the recognition layer
			if (cResult == eOk) oProperties.oRecognitionLayer->DoFill(cFillRectangle);
			}
		
		if (cResult == eOk) {

			//Send notification of changes
			oCore->oNotifications->CommunicateEvent(eNotificationPageUpdate, (void*)oProperties.dPageID);

			//Set dirty flag
			SetDirty(true);
			}
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}

//This function convert the page to black/white
eActionResult 
CCopyDocumentPage::DoConvertBlackWhite(DWORD pdData) {

	CCopyingMachineCore* oCore;
	Status cGDIStatus;
	eActionResult cResult;	
	Bitmap* oNewBitmap;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;
	cGDIStatus = Aborted;

	if (CheckPageImageAction(eImageBlackWhite, pdData)) {

		oNewBitmap = CBitmap::ConvertBitmap(oProperties.oImage->GetBitmap(), PixelFormat1bppIndexed);

		if (oNewBitmap!=NULL) {

			oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
			
			//Make an undo point before making changes
			if (oCore->oUndoRedo->MakeUndoPoint(this, eImageBlackWhite, 0)) {

				//The undo point was succesful, set the oImage to NULL to prevent it from
				//being deleted by SetImage
				oProperties.oImage = NULL;
				}

			//update the image
			SetImage(oNewBitmap, false);
			
			//Set dirty flag
			SetDirty(true);

			cResult = eOk;

			//Send notification of changes
			oCore->oNotifications->CommunicateEvent(eNotificationPageUpdate, (void*)oProperties.dPageID);
			}
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}

//This function converts the page to grayscale
eActionResult 
CCopyDocumentPage::DoConvertGrayscale(DWORD pdData) {

	CCopyingMachineCore* oCore;
	Status cGDIStatus;
	eActionResult cResult;	
	Bitmap* oNewBitmap;

	oTrace->StartTrace(__WFUNCTION__);

	cResult = eImageActionError;
	cGDIStatus = Aborted;

	if (CheckPageImageAction(eImageGrayscale, pdData)) {

		oNewBitmap = CBitmap::ConvertBitmap(oProperties.oImage->GetBitmap(), PixelFormat8bppIndexed);

		if (oNewBitmap!=NULL) {

			oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
			
			//Make an undo point before making changes
			if (oCore->oUndoRedo->MakeUndoPoint(this, eImageGrayscale, 0)) {

				//The undo point was succesful, set the oImage to NULL to prevent it from
				//being deleted by SetImage
				oProperties.oImage = NULL;
				}

			//update the image
			SetImage(oNewBitmap, false);

			//Set dirty flag
			SetDirty(true);

			cResult = eOk;

			//Send notification of changes
			oCore->oNotifications->CommunicateEvent(eNotificationPageUpdate, (void*)oProperties.dPageID);

			}
		}

	oTrace->EndTrace(__WFUNCTION__);

	return cResult;
	}


//This function retrieves the memory size of the current page
DWORD 
CCopyDocumentPage::GetMemorySize() {

	DWORD dResult;
	
	dResult = 0;

	if (oProperties.oImage!=NULL) {

		dResult = oProperties.oImage->GetBitmapMemorySize();
		}

	return dResult;
}

SizeF 
CCopyDocumentPage::GetPhysicalSize(eSizeMeasure pcMeasure) {

	eSizeMeasure cMeasure;
	SizeF	     cResult;

	cResult.Width = 0;
	cResult.Height = 0;
	cMeasure = pcMeasure;

	if (oProperties.oImage!=NULL) {

		if (cMeasure==eSizeMeasureUnknown) {

			//No measure given, use the registry to get the preferred measure
			cMeasure = (eSizeMeasure)oGlobalInstances.oRegistry->ReadInt(L"General", L"Measurement", (int)eInch);
			}

		cResult = oProperties.oImage->GetBitmapPhysicalSize(cMeasure);
		}

	return cResult;
	}

//This function tries to get the properties from the image
void 
CCopyDocumentPage::GetMetadata() {

	UINT size = 0;
	UINT count = 0;
	Bitmap* oBitmap;

	oBitmap = oProperties.oImage->GetBitmap();

	//Get the property size from the image
	oBitmap->GetPropertySize(&size, &count);

	// GetAllPropertyItems returns an array of PropertyItem objects.
	// Allocate a buffer large enough to receive that array.
	PropertyItem* pPropBuffer =(PropertyItem*)malloc(size);

	// Get the array of PropertyItem objects.
	oProperties.iMetadataCount = count;

	oBitmap->GetAllPropertyItems(size, count, pPropBuffer);

	oProperties.pMetadata = pPropBuffer;
	}

//This function tries to set the properties of the image
void 
CCopyDocumentPage::SetMetadata() {

	Bitmap* oBitmap;
	int iIndex;

	oBitmap = oProperties.oImage->GetBitmap();
	iIndex = oProperties.iMetadataCount;

	while (iIndex > 0) {

		iIndex--;

		oBitmap->SetPropertyItem(&oProperties.pMetadata[iIndex]);
		}
	}

//Script functions
bool 
CCopyDocumentPage::Set(std::wstring psObjectname, sScriptExpression psNewValue) {

	bool bResult;

	bResult = CScriptObject::Set(psObjectname, psNewValue);

	if (!bResult && QueryType(psObjectname) == eScriptObjectAttribute) { 

		//process the document attributes
		}

	return bResult;
	}


sScriptExpression 
CCopyDocumentPage::Get(std::wstring psObjectname) {

	sScriptExpression oResult;

	oResult = CScriptObject::Get(psObjectname);

	if (oResult.cValueType == eScriptValueAny &&
		QueryType(psObjectname) == eScriptObjectAttribute) { 

		//process the document attributes
		if (psObjectname == L"id") {

			oResult.iResult = (int)GetPageID();
			oResult.cValueType = eScriptValueInteger;
			}
		}

	return oResult;
	}

bool 
CCopyDocumentPage::Execute(std::wstring psObjectname, 
							 sScriptExpression &psValue) {

	bool bResult;
	CCopyingMachineCore* oCore;

	bResult = false;

	bResult = CScriptObject::Execute(psObjectname, psValue);
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	if (!bResult) {
		
		if (psObjectname == L"save") {

			}

		}

	return bResult;
	}




