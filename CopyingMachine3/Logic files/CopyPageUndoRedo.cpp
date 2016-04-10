#include "StdAfx.h"
#include "CopyingMachineCore.h"

CCopyPageUndoRedo::CCopyPageUndoRedo(sGlobalInstances pInstances)
{
	//Set helper classes
	oTrace = pInstances.oTrace;	

	//copy the global instances
	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances),
				&pInstances, sizeof(sGlobalInstances));

	//Init variables
	bUndoEnabled = true;
}

CCopyPageUndoRedo::~CCopyPageUndoRedo(void)
{
	oTrace->StartTrace(__WFUNCTION__);

	ClearUndoRedo();

	oTrace->EndTrace(__WFUNCTION__);
	}

//Get the number of action in the undo vector
int
CCopyPageUndoRedo::GetUndoCount() {

	return (int)oUndo.size();
	}

//Get the number of action in the redo vector
int
CCopyPageUndoRedo::GetRedoCount() {

	return (int)oRedo.size();
	}

//This function enables or disables the undo function
void 
CCopyPageUndoRedo::EnableUndo(bool pbEnable) {

	bUndoEnabled = pbEnable;
}

//Clear the complete queue
void 
CCopyPageUndoRedo::ClearUndoRedo() {

	//Clean up vectors
	while (oUndo.size()>0) {

		DeleteUndoPoint();
		}
	while (oRedo.size()>0) {

		DeleteRedoPoint();
		}
	}

//Clear all undo points for the given document
void 
CCopyPageUndoRedo::ClearUndoRedo(int piDocumentId) {

	long lIndex;

	lIndex = oUndo.size();

	while (lIndex>0) {

		lIndex--;

		if (oUndo[lIndex]->iDocumentId == piDocumentId) {
			DeleteUndoRedoPoint(true, lIndex);
			}
		}

	lIndex = oRedo.size();

	while (lIndex>0) {

		lIndex--;

		if (oRedo[lIndex]->iDocumentId == piDocumentId) {
			DeleteUndoRedoPoint(false, lIndex);
			}
		}
	}

//Clear all undo points for the given page
void 
CCopyPageUndoRedo::ClearUndoRedo(DWORD pdPageId) {

	long lIndex;

	lIndex = oUndo.size();

	while (lIndex>0) {

		lIndex--;

		if (oUndo[lIndex]->dPageId == pdPageId) {
			DeleteUndoRedoPoint(true, lIndex);
			}
		}

	lIndex = oRedo.size();

	while (lIndex>0) {

		lIndex--;

		if (oRedo[lIndex]->dPageId == pdPageId) {
			DeleteUndoRedoPoint(false, lIndex);
			}
		}
	}

//Create a undo point for the given page and action
bool 
CCopyPageUndoRedo::MakeUndoPoint(CCopyDocumentPage* poPage, ePageImageAction pcImageAction, 
								DWORD pdActionData) {

	sCopyDocumentPageUndoRedo* sUndoPoint;
	bool bResult;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	if (bUndoEnabled) {

		//Analyse memory usage by the Undo and the Redo vectors. Clean up if possible.
		AnalyseMemoryUsage();

		sUndoPoint = new sCopyDocumentPageUndoRedo();

		//Initialise the undo action
		sUndoPoint->iDocumentId			= poPage->GetDocumentID();
		sUndoPoint->dPageId				= poPage->GetPageID();
		sUndoPoint->sDescription		= L"Undo";
		sUndoPoint->cUndoRedoAction		= pcImageAction;
		sUndoPoint->dUndoRedoActionData = pdActionData;
		sUndoPoint->oPageImage			= NULL;
		sUndoPoint->oRecognitionLayer	= poPage->GetRecognitionLayer()->Copy();
		poPage->GetSelection(sUndoPoint->cSelection);

		switch (pcImageAction) {

			case eImageFillRectangle:

				sUndoPoint->cUndoRedoType = eUndoRedoBitmapPart;
				bResult = DoMakeUndoPointFillRectangle(poPage, sUndoPoint);
				break;

			case eImageBlackWhite:
			case eImageGrayscale:
			case eImageCrop:

				sUndoPoint->cUndoRedoType = eUndoRedoImage;
				sUndoPoint->oPageImage = poPage->GetImage();

				if (sUndoPoint->oPageImage!=NULL) bResult = true;
				break;
		
			case eImageFlipHorizontal:
			case eImageFlipVertical:
			case eImageRotate:
				
				sUndoPoint->cUndoRedoType = eUndoRedoAction;

				//All necessary information already available, just store the undopoint in the vector
				bResult = true;
				break;

			case eImageChangeDPIX:

				//Get the current resolution and store the undopoint in the vector
				sUndoPoint->cUndoRedoType = eUndoRedoProperty;
				sUndoPoint->iValue = poPage->GetResolutionX();
				bResult = true;
				break;

			case eImageChangeDPIY:

				//Get the current resolution and store the undopoint in the vector
				sUndoPoint->cUndoRedoType = eUndoRedoProperty;
				sUndoPoint->iValue = poPage->GetResolutionY();
				bResult = true;
				break;

			case eImageChangeSelection:

				//Get the current selection and store it in the undopoint
				sUndoPoint->cUndoRedoType = eUndoRedoProperty;
				poPage->GetSelection(sUndoPoint->cSelection);
				bResult = true;
				break;
				}
		

		//if succesfull than save the undo point
		if (bResult) {
			oUndo.push_back(sUndoPoint);
			}
		else {
			delete sUndoPoint;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

//This function will create an undo point for the fillrectangle action
bool 
CCopyPageUndoRedo::DoMakeUndoPointFillRectangle(CCopyDocumentPage* poPage, 
												sCopyDocumentPageUndoRedo* poUndoPoint) {

	bool bResult;
	RectF cFillRectangle;
	sCopyDocumentPageProperties oProperties;
	sCopyDocumentPageUndoRedoImage oPartialImage;

	bResult = true;

	//Get all the properties of the page
	oProperties = poPage->GetAllProperties();
	
	if (oProperties.cSelection.bOuter) {

		//Top
		if (oProperties.cSelection.fTop>0) {

			cFillRectangle = RectF((REAL)0,
								   (REAL)0,
								   (REAL)(oProperties.oImage->GetBitmap()->GetWidth()),
								   (REAL)(oProperties.cSelection.fTop));

			oPartialImage.cBitmapRect.X = cFillRectangle.X;
			oPartialImage.cBitmapRect.Y = cFillRectangle.Y;
			oPartialImage.oBitmap = oProperties.oImage->CreatePartialBitmap(cFillRectangle);
			if (oPartialImage.oBitmap!=NULL) {
				poUndoPoint->oBitmapParts.push_back(oPartialImage);
				}
			else {
				bResult = false;
				}	
			}
		//Bottom
		if (oProperties.cSelection.fBottom<oProperties.oImage->GetBitmap()->GetHeight()) {
			
			cFillRectangle = RectF( (REAL)0,
								   (REAL)(oProperties.cSelection.fBottom),
								   (REAL)(oProperties.oImage->GetBitmap()->GetWidth()),
								   (REAL)(oProperties.oImage->GetBitmap()->GetHeight() - oProperties.cSelection.fBottom));

			oPartialImage.cBitmapRect.X = cFillRectangle.X;
			oPartialImage.cBitmapRect.Y = cFillRectangle.Y;
			oPartialImage.oBitmap = oProperties.oImage->CreatePartialBitmap(cFillRectangle);
			if (oPartialImage.oBitmap!=NULL) {
				poUndoPoint->oBitmapParts.push_back(oPartialImage);
				}
			else {
				bResult = false;
				}	
			}

		//Left
		if (oProperties.cSelection.fLeft>0) {
			
			cFillRectangle = RectF(  (REAL)0,
									(REAL)oProperties.cSelection.fTop,
									(REAL)oProperties.cSelection.fLeft,
									(REAL)(oProperties.cSelection.fBottom - oProperties.cSelection.fTop + 1));
			
			oPartialImage.cBitmapRect.X = cFillRectangle.X;
			oPartialImage.cBitmapRect.Y = cFillRectangle.Y;
			oPartialImage.oBitmap = oProperties.oImage->CreatePartialBitmap(cFillRectangle);
			if (oPartialImage.oBitmap!=NULL) {
				poUndoPoint->oBitmapParts.push_back(oPartialImage);
				}
			else {
				bResult = false;
				}	
			}

		//Right
		if (oProperties.cSelection.fRight<oProperties.oImage->GetBitmap()->GetWidth()) {
			
			cFillRectangle = RectF(  (REAL)oProperties.cSelection.fRight,
									(REAL)oProperties.cSelection.fTop,
									(REAL)(oProperties.oImage->GetBitmap()->GetWidth()-oProperties.cSelection.fRight),
									(REAL)(oProperties.cSelection.fBottom - oProperties.cSelection.fTop + 1));
			
			oPartialImage.cBitmapRect.X = cFillRectangle.X;
			oPartialImage.cBitmapRect.Y = cFillRectangle.Y;
			oPartialImage.oBitmap = oProperties.oImage->CreatePartialBitmap(cFillRectangle);
			if (oPartialImage.oBitmap!=NULL) {
				poUndoPoint->oBitmapParts.push_back(oPartialImage);
				}
			else {
				bResult = false;
				}	
			}
		}
	else {
				
		cFillRectangle = RectF(  (REAL)oProperties.cSelection.fLeft,
								(REAL)oProperties.cSelection.fTop,
								(REAL)(oProperties.cSelection.fRight - oProperties.cSelection.fLeft),
								(REAL)(oProperties.cSelection.fBottom - oProperties.cSelection.fTop));
			
		oPartialImage.cBitmapRect.X = cFillRectangle.X;
		oPartialImage.cBitmapRect.Y = cFillRectangle.Y;
		oPartialImage.oBitmap = oProperties.oImage->CreatePartialBitmap(cFillRectangle);
		
		if (oPartialImage.oBitmap!=NULL) {
			poUndoPoint->oBitmapParts.push_back(oPartialImage);
			}
		else {
			bResult = false;
			}	
		}

	return bResult;
	}



//This function will undo the last action on the give page
bool 
CCopyPageUndoRedo::Undo() {

	bool bResult;
	sCopyDocumentPageUndoRedo* cUndoPoint;
	CCopyDocument* oDocument;
	CCopyDocumentPage* oPage;
	CCopyingMachineCore* oCore;
	
	oTrace->StartTrace(__WFUNCTION__);

	//Disable the undo function, to be able to undo actions without creating new
	//UndoPoints
	bUndoEnabled = false;

	bResult = false;

	if (oUndo.size()>0) {

		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		
		//Get the latest undo point
		cUndoPoint = oUndo[oUndo.size()-1];

		oDocument = oCore->oDocuments->GetDocumentByID(cUndoPoint->iDocumentId);

		if (oDocument!=NULL) {

			oPage = oDocument->GetPageByID(cUndoPoint->dPageId); 
				
			if (oPage!=NULL) {

				//Perform the undo action on the page
				bResult = DoUndo(oPage, cUndoPoint);
				}
			}

		//Remove the undo point and make it a redo point (if succesful)
		if (bResult) {
			
			//DeleteUndoRedoPoint(true, oUndo.size()-1);
			oUndo.erase(oUndo.begin() + oUndo.size()-1);
			oRedo.push_back(cUndoPoint);
			}
		}

	//Enable the undo function again
	bUndoEnabled = true;

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

bool 
CCopyPageUndoRedo::DoUndo(CCopyDocumentPage* poPage, sCopyDocumentPageUndoRedo* cUndoPoint) {

	bool bResult;
	CCopyingMachineCore* oCore;
	CBitmap* oPageImage;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	switch (cUndoPoint->cUndoRedoAction) {

		case eImageChangeSelection:

			//Undo by setting the selection to the selection in the UndoPoint
			poPage->SetSelection(cUndoPoint->cSelection);
			bResult = true;
			break;

		case eImageChangeDPIX:

			//Undo by setting the resolution by the value from the UndoPoint
			poPage->SetResolutionX(cUndoPoint->iValue);
			bResult = true;
			break;

		case eImageChangeDPIY:

			//Undo by setting the resolution by the value from the UndoPoint
			poPage->SetResolutionY(cUndoPoint->iValue);
			bResult = true;
			break;

		case eImageRotate:

			//Undo the rotation by rotating the image counterwise
			poPage->DoRotateImage(360 - cUndoPoint->dUndoRedoActionData);
			bResult = true;
			break;

		case eImageFlipVertical:
		case eImageFlipHorizontal:

			//Undo the flipping operation by flipping the image again
			poPage->DoFlipImage(cUndoPoint->dUndoRedoActionData);
			bResult = true;
			break;

		case eImageCrop:
		case eImageBlackWhite:
		case eImageGrayscale:

			//Replace the current image with the image from the UndoPoint.
			cUndoPoint->oPageImage->ClearThumbnail();
			oPageImage = poPage->SwapImage(cUndoPoint->oPageImage);
			if (oPageImage!=NULL) {
				delete oPageImage; //for now we delete it, but in the future it can be stored in the redo point
				}
			cUndoPoint->oPageImage = NULL; 
			
			//Send notification of changes
			oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
			oCore->oNotifications->CommunicateEvent(eNotificationPageSettingUpdate, (void*)poPage->GetPageID());
			oCore->oNotifications->CommunicateEvent(eNotificationPageUpdate, (void*)poPage->GetPageID());

			//Set dirty flag
			poPage->SetDirty(true);

			bResult = true;
			break;

		case eImageFillRectangle:

			//Replace the parts on the image with the parts from the
			//UndoPoint.
			oPageImage = poPage->GetImage();

			if (oPageImage!=NULL) {

				bResult = true;

				//Draw all the partial bitmaps from the UndoPoint
				for (long lIndex=0; lIndex<(long)cUndoPoint->oBitmapParts.size() && bResult; lIndex++) {

					bResult = oPageImage->DrawPartialBitmap(cUndoPoint->oBitmapParts[lIndex].oBitmap,
															cUndoPoint->oBitmapParts[lIndex].cBitmapRect);					
					}

				if (bResult) {
					
					//Send notification of changes
					oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
					oCore->oNotifications->CommunicateEvent(eNotificationPageUpdate, (void*)poPage->GetPageID());
					
					//Set dirty flag
					poPage->SetDirty(true);
					}
				}
			break;
		}

	poPage->SetRecognitionLayer(cUndoPoint->oRecognitionLayer);
	cUndoPoint->oRecognitionLayer = NULL; //Prevent it from being deleted

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return bResult;
	}


//This function performs the last action again
bool 
CCopyPageUndoRedo::Redo() {

	bool bResult;
	sCopyDocumentPageUndoRedo* cRedoPoint;
	CCopyDocument* oDocument;
	CCopyDocumentPage* oPage;
	CCopyingMachineCore* oCore;
	
	oTrace->StartTrace(__WFUNCTION__);

	//Disable the undo function, to be able to undo actions without creating new
	//UndoPoints
	//bUndoEnabled = false;

	bResult = false;

	if (oRedo.size()>0) {

		oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
		
		//Get the latest undo point
		cRedoPoint = oRedo[oRedo.size()-1];

		oDocument = oCore->oDocuments->GetDocumentByID(cRedoPoint->iDocumentId);

		if (oDocument!=NULL) {

			oPage = oDocument->GetPageByID(cRedoPoint->dPageId); 
				
			if (oPage!=NULL) {

				//Perform the undo action on the page
				bResult = DoRedo(oPage, cRedoPoint);
				}
			}

		//Remove the undo point and make it a redo point (if succesful)
		if (bResult) {
			
			DeleteUndoRedoPoint(false, oRedo.size()-1);
			}
		}

	//Enable the undo function again
	//bUndoEnabled = true;

	oTrace->EndTrace(__WFUNCTION__, bResult);

	return bResult;
	}

bool 
CCopyPageUndoRedo::DoRedo(CCopyDocumentPage* poPage, sCopyDocumentPageUndoRedo* cRedoPoint) {

	bool bResult;
	sCopyDocumentSelection cSelection;

	oTrace->StartTrace(__WFUNCTION__);

	bResult = false;

	switch (cRedoPoint->cUndoRedoAction) {

		case eImageChangeDPIX:

			//Redo by setting the resolution by the value from the UndoPoint
			poPage->SetResolutionX(cRedoPoint->iValue);
			bResult = true;
			break;

		case eImageChangeDPIY:

			//Redo by setting the resolution by the value from the UndoPoint
			poPage->SetResolutionY(cRedoPoint->iValue);
			bResult = true;
			break;

		case eImageChangeSelection:

			//Redo the change of the selection
			poPage->SetSelection(cRedoPoint->cSelection);
			break;

		case eImageRotate:

			//Redo the rotation by rotating the image counterwise
			poPage->DoRotateImage(cRedoPoint->dUndoRedoActionData);
			bResult = true;
			break;

		case eImageFlipVertical:
		case eImageFlipHorizontal:

			//Redo the flipping operation by flipping the image again
			poPage->DoFlipImage(cRedoPoint->dUndoRedoActionData);
			bResult = true;
			break;

		case eImageCrop:

			//Redo the cropping the image
			poPage->GetSelection(cSelection);
			poPage->SetSelection(cRedoPoint->cSelection);
			poPage->DoCropImage();
			poPage->SetSelection(cSelection);
			bResult = true;
			break;

		case eImageGrayscale:

			//Redo converting the image to black and white
			poPage->DoConvertGrayscale(0);
			bResult = true;
			break;

		case eImageBlackWhite:

			//Redo the conversion of the image to black and white
			poPage->DoConvertBlackWhite(0);
			bResult = true;
			break;

		case eImageFillRectangle:

			//Redo the filling of the rectangle
			poPage->GetSelection(cSelection);
			poPage->SetSelection(cRedoPoint->cSelection);
			poPage->DoFillRectangle(cRedoPoint->dUndoRedoActionData);
			poPage->SetSelection(cSelection);
			bResult = true;
			break;
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);

	return bResult;
	}

//This function deletes the oldes UndoPoint;
void 
CCopyPageUndoRedo::DeleteUndoPoint() {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	if (oUndo.size()>0) {

		DeleteUndoRedoPoint(true, 0);
		}
	
	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function deletes the oldes RedoPoint;
void 
CCopyPageUndoRedo::DeleteRedoPoint() {

	oTrace->StartTrace(__WFUNCTION__, eAll);

	if (oRedo.size()>0) {
		
		DeleteUndoRedoPoint(false, 0);
		}
	
	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function deletes the give UndoRedoPoint;
void 
CCopyPageUndoRedo::DeleteUndoRedoPoint(bool bUndo, long plIndex) {

	sCopyDocumentPageUndoRedo* oUndoRedoPoint;
	
	oTrace->StartTrace(__WFUNCTION__, eAll);

	//Retrieve the undoredopoint
	if (bUndo) {

		oUndoRedoPoint = oUndo[plIndex];
		oUndo.erase(oUndo.begin() + plIndex);
		}
	else {

		oUndoRedoPoint = oRedo[plIndex];
		oRedo.erase(oRedo.begin() + plIndex);
		}

	//Delete the image 
	if (oUndoRedoPoint->cUndoRedoType == eUndoRedoImage && 
		oUndoRedoPoint->oPageImage !=NULL) {

		delete oUndoRedoPoint->oPageImage;
		}

	//Delete the partial bitmaps
	while (oUndoRedoPoint->oBitmapParts.size()>0) {

		if (oUndoRedoPoint->oBitmapParts[0].oBitmap!=NULL) {
			
			delete oUndoRedoPoint->oBitmapParts[0].oBitmap;
			}	

		oUndoRedoPoint->oBitmapParts.erase(oUndoRedoPoint->oBitmapParts.begin());
		}

	//Delete the recognition layer
	if (oUndoRedoPoint->oRecognitionLayer!=NULL) {

		delete oUndoRedoPoint->oRecognitionLayer;
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

//This function analyses the memory usage by the undo and the redo action. If it exceeds
//a certain limit (ie > 8% of total physical system memory) it will start cleaning up undo and redo actions
void 
CCopyPageUndoRedo::AnalyseMemoryUsage() {

	MEMORYSTATUSEX cMemoryStatus;
	DWORDLONG      dUndoRedoMemoryStatus;

	oTrace->StartTrace(__WFUNCTION__, eAll);

	//Get the status of the memory
	 cMemoryStatus.dwLength = sizeof(cMemoryStatus);
	GlobalMemoryStatusEx(&cMemoryStatus);

	//Get the size of the undo and redo vectors my adding all
	//the image sizes together (rest is not significant);
	dUndoRedoMemoryStatus = 0;
	for (long lIndex=0; lIndex<(long)oUndo.size(); lIndex++) {

		dUndoRedoMemoryStatus += GetMemorySizeUndoRedoPoint(oUndo[lIndex]);
		}
	for (long lIndex=0; lIndex<(long)oRedo.size(); lIndex++) {

		dUndoRedoMemoryStatus += GetMemorySizeUndoRedoPoint(oRedo[lIndex]);
		}

	//Clean up until UndoRedo status smaller than 8% of physical memory
	while (dUndoRedoMemoryStatus > cMemoryStatus.ullTotalPhys * 0.8f &&
			(oUndo.size()+oRedo.size())>0) {

		if (oUndo.size()>0) {
			
			dUndoRedoMemoryStatus -= GetMemorySizeUndoRedoPoint(oUndo[0]);
			DeleteUndoPoint();
			}

		if (oRedo.size()>0) {
			
			dUndoRedoMemoryStatus -= GetMemorySizeUndoRedoPoint(oRedo[0]);
			DeleteRedoPoint();
			}
		}

	oTrace->EndTrace(__WFUNCTION__, eAll);
	}

DWORD 
CCopyPageUndoRedo::GetMemorySizeUndoRedoPoint(sCopyDocumentPageUndoRedo *oUndoRedoPoint) {

	DWORD dResult;

	dResult = 0;

	if (oUndoRedoPoint->oPageImage!=NULL) {

		dResult += oUndoRedoPoint->oPageImage->GetBitmapMemorySize();
		}

	for (long lIndex=0; lIndex<(long)oUndoRedoPoint->oBitmapParts.size(); lIndex++) {
		
		if (oUndoRedoPoint->oBitmapParts[lIndex].oBitmap!=NULL) {

			dResult += CBitmap::GetBitmapMemorySize(oUndoRedoPoint->oBitmapParts[lIndex].oBitmap);
			}
		} 

	if (oUndoRedoPoint->oRecognitionLayer!=NULL) {

		dResult += oUndoRedoPoint->oRecognitionLayer->GetMemorySize();
		}

	return dResult;
	}

