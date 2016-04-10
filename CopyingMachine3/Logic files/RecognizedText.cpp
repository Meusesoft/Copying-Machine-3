#include "stdafx.h"
#include "CopyingMachineCore.h"

//------------------------------------------------------------------------------
//
// Class CRecognizedText

//Constructor and destructor
CRecognizedText::CRecognizedText(Rect pcBoundingBox, LANGID pcLanguage) {

	cBoundingBox = pcBoundingBox;
	cLanguage    = pcLanguage;
	}


CRecognizedText::~CRecognizedText() {

	oCharacters.clear();
	}

//This function returns the language code 
LANGID	
CRecognizedText::GetLanguage() {

	return cLanguage;
	}

//This function returns the bounding box of the recognized text
Rect
CRecognizedText::GetBoundingBox() {

	return cBoundingBox;
	}

//Get the memory size of this text block
DWORD
CRecognizedText::GetMemorySize() {

	return (DWORD)(oCharacters.size() * sizeof(sRecognizedCharacter));
	}

//Move the text block
void		
CRecognizedText::MoveBoundingBox(int piDeltaX, int piDeltaY) {

	cBoundingBox.X += piDeltaX;
	cBoundingBox.Y += piDeltaY;
	}

//Cut the bounding box. This means that the bounding box will be adjuist
//so it will exactly fit into the give rectangle. Characters outside the
//bounding box will be deleted
void
CRecognizedText::CutBoundingBox(Rect pcRectangle) {

	Rect cResultBox;
	Rect cCharacterBox;
	long lIndex;

	if (cResultBox.Intersect(cResultBox, cBoundingBox, pcRectangle)) {

		lIndex = oCharacters.size();

		while (lIndex>0) {

			lIndex--;

			cCharacterBox = oCharacters[lIndex].cBoundingBox;
			cCharacterBox.Offset(cBoundingBox.X, cBoundingBox.Y);

			if (!cResultBox.Contains(cCharacterBox)) {

				//connect the end of lines to the previous character
				if (oCharacters[lIndex].iEndOfLines>0 && lIndex>0) {

					oCharacters[lIndex-1].iEndOfLines += oCharacters[lIndex].iEndOfLines;
					}

				//remove the character
				oCharacters.erase(oCharacters.begin() + lIndex);
				}
			else {

				//reposition the character to its new position within its new bounding box
				oCharacters[lIndex].cBoundingBox.Offset(cBoundingBox.X - cResultBox.X, cBoundingBox.Y - cResultBox.Y);
				}
			}
		}

	cBoundingBox = cResultBox;
	}

//Subtract the bounding box. This means that the given rectangle will
//be subtracted from the bounding box. The characters inside the rectangle
//will be deleted
void		
CRecognizedText::SubtractBoundingBox(Rect pcRectangle) {

	Rect cResultBox;
	Rect cCharacterBox;
	long lIndex;

	if (cResultBox.Intersect(cResultBox, cBoundingBox, pcRectangle)) {

		lIndex = oCharacters.size();

		while (lIndex>0) {

			lIndex--;

			cCharacterBox = oCharacters[lIndex].cBoundingBox;
			cCharacterBox.Offset(cBoundingBox.X, cBoundingBox.Y);

			if (cResultBox.IntersectsWith(cCharacterBox)) {

				oCharacters.erase(oCharacters.begin() + lIndex);
				}
			}
		}
	}

//This function returns the number of recognized characters within this bounding box
int
CRecognizedText::GetCharacterCount() {

	return (int)oCharacters.size();
	}

//This function returns the bounding box of the requested character
Rect
CRecognizedText::GetCharacterBoundingBox(long plIndex) {

	Rect cResult;

	cResult = Rect(0, 0, 0, 0);

	if (plIndex<(long)oCharacters.size()) cResult = oCharacters[plIndex].cBoundingBox;

	return cResult;
	}

//This function returns the character code of the requested character
TCHAR		
CRecognizedText::GetCharacter(long plIndex) {

	TCHAR cResult;

	cResult = 0;

	if (plIndex<(long)oCharacters.size()) cResult = oCharacters[plIndex].cCharacter;

	return cResult;
	}

//This function returns the character code of a character at the
//requested location
sRecognizedCharacter		
CRecognizedText::GetCharacter(int piX, int piY) {

	sRecognizedCharacter cResult;

	SecureZeroMemory(&cResult, sizeof(cResult));

	long lIndex = oCharacters.size();

	while (lIndex>0) {

		lIndex--;

		if (oCharacters[lIndex].cBoundingBox.Contains(piX, piY)) {

			cResult = oCharacters[lIndex];
			lIndex=0;
			}
		}

	return cResult;
}

//This function returns the character code of a character at the
//requested location
sRecognizedCharacter		
CRecognizedText::GetCharacterStruct(long plIndex) {

	sRecognizedCharacter cResult;

	SecureZeroMemory(&cResult, sizeof(cResult));

	if (plIndex < (long)oCharacters.size()) {
	
		cResult = oCharacters[plIndex];
		}

	return cResult;
	}

//This function adds a character and its bounding box to this recognized text
void		
CRecognizedText::AddCharacter(TCHAR pcCharacter, Rect pcBoundingBox, long lNumberSpaces, long lNumberEndOfLines) {

	sRecognizedCharacter cCharacter;

	cCharacter.cCharacter	= pcCharacter;
	cCharacter.cBoundingBox = pcBoundingBox;
	cCharacter.iEndOfLines = lNumberEndOfLines;
	cCharacter.iSpaces = lNumberSpaces;

	oCharacters.push_back(cCharacter);
	}

//This function concatenates all the characters to a text (wstring)
std::wstring
CRecognizedText::GetText() {

	std::wstring psResult;
	sRecognizedCharacter cCharacter;
	long lIndex;

	lIndex = 0;

	while (lIndex<(long)oCharacters.size()) {

		for (long lSpaces=0; lSpaces<oCharacters[lIndex].iSpaces; lSpaces++) {
			
			psResult += L" ";
			}

		psResult += oCharacters[lIndex].cCharacter;

		for (long lNumberEndOfLines=0; lNumberEndOfLines<oCharacters[lIndex].iEndOfLines; lNumberEndOfLines++) {
			
			psResult += L"\r\n";
			}

		lIndex++;
		}

	return psResult;
}

//This function rotates the texts within this recognition layer
void 
CRecognizedText::DoRotate(DWORD pdAngle, RectF pcRectangleWithinRotation) {

	long lIndex;
	RectF cBoundingBoxSize;

	cBoundingBoxSize = RectF(0, 0, (int)cBoundingBox.Width, (int)cBoundingBox.Height);

	CMathVector::RotateRect(cBoundingBox, pcRectangleWithinRotation, pdAngle);

	lIndex = oCharacters.size();

	while (lIndex>0) {
	
		lIndex--;

		CMathVector::RotateRect(oCharacters[lIndex].cBoundingBox, cBoundingBoxSize, pdAngle);
		}
	}

//This function flips the texts within this recognition layer
void 
CRecognizedText::DoFlip(DWORD pdOperation, RectF pcRectangleWithinFlip) {

	long lIndex;
	RectF cBoundingBoxSize;

	cBoundingBoxSize = RectF(0, 0, (int)cBoundingBox.Width, (int)cBoundingBox.Height);

	CMathVector::MirrorRect(cBoundingBox, pcRectangleWithinFlip, pdOperation);

	lIndex = oCharacters.size();

	while (lIndex>0) {
	
		lIndex--;

		CMathVector::MirrorRect(oCharacters[lIndex].cBoundingBox, cBoundingBoxSize, pdOperation);
		}
	}


//------------------------------------------------------------------------------
//
// Class CRecognitionLayer

//Constructor and destructor
CRecognitionLayer::CRecognitionLayer() {
	
	}

CRecognitionLayer::~CRecognitionLayer() {

	//clean up
	Clear();
	}

//Get the memory size of this text block
DWORD
CRecognitionLayer::GetMemorySize() {

	DWORD dResult;
	long lIndex;

	lIndex = oTexts.size();
	dResult = 0;

	while (lIndex>0) {

		lIndex--;
		dResult += oTexts[lIndex]->GetMemorySize();
		}

	return dResult;
	}


//This function creates a copy of the recognition layer
CRecognitionLayer* 
CRecognitionLayer::Copy() {

	CRecognizedText* oTextCopy;
	CRecognitionLayer* oLayerCopy;
	sRecognizedCharacter cCharacter;

	oLayerCopy = new CRecognitionLayer();

	for (long lIndex=0; lIndex<(long)oTexts.size(); lIndex++) {

		oTextCopy = oLayerCopy->CreateRecognizedText(
			oTexts[lIndex]->GetBoundingBox(), oTexts[lIndex]->GetLanguage());
		
		for (long lTextIndex=0; lTextIndex<oTexts[lIndex]->GetCharacterCount(); lTextIndex++) {

			cCharacter = oTexts[lIndex]->GetCharacterStruct(lTextIndex);
			oTextCopy->AddCharacter(
				cCharacter.cCharacter,
				cCharacter.cBoundingBox,
				cCharacter.iSpaces,
				cCharacter.iEndOfLines);
			}
		}

	return oLayerCopy;
	}	
	
//This functions clears this layer of all recognized texts
void 
CRecognitionLayer::Clear() {

	for (long lIndex=0; lIndex<(long)oTexts.size(); lIndex++) {

		delete oTexts[lIndex];
		}
	oTexts.clear();
	}

//This function created an instance of CRecognizedText and returns the pointer to it
CRecognizedText* 
CRecognitionLayer::CreateRecognizedText(Rect pcBoundingBox, LANGID pcLanguage) {

	CRecognizedText* oResult;
	long lIndex;
	Rect cBoundingBox;
	bool bAdded;
		
	oResult = new CRecognizedText(pcBoundingBox, pcLanguage);
	lIndex = oTexts.size();
	bAdded = false;

	while (lIndex>0 && !bAdded) {

		lIndex--;
		cBoundingBox = oTexts[lIndex]->GetBoundingBox();

		if (cBoundingBox.Y < pcBoundingBox.Y) {

			oTexts.insert(oTexts.begin() + lIndex + 1, oResult);
			lIndex = 0;
			bAdded = true;
			}
		}

	if (!bAdded) {

		oTexts.insert(oTexts.begin(), oResult);
		}

	return oResult;
	}

//This function returns the number of recognized texts in this layer
int 
CRecognitionLayer::GetRecognizedTextCount() {

	return (int)oTexts.size();
	}

//This function returns the requested instance of the CRegonizedText within this layer
CRecognizedText* 
CRecognitionLayer::GetRecognizedText(long plIndex) {

	CRecognizedText* oResult;

	if (plIndex<(long)oTexts.size()) oResult = oTexts[plIndex];

	return oResult;
	}

//This function returns the instance of CRecognizedText on the given location
CRecognizedText* 
CRecognitionLayer::GetRecognizedText(int piX, int piY) {

	CRecognizedText* oResult;
	long lIndex;

	oResult = NULL;
	lIndex = oTexts.size();

	while (lIndex>0) {
	
		lIndex--;

		if (oTexts[lIndex]->GetBoundingBox().Contains(piX, piY)) {

			oResult = oTexts[lIndex];
			lIndex = 0;
			}
		}

	return oResult;
}


//This function returns the character on the requested location
sRecognizedCharacter 
CRecognitionLayer::GetCharacter(int piX, int piY) {

	CRecognizedText* oText;
	sRecognizedCharacter cResult;

	SecureZeroMemory(&cResult, sizeof(cResult));
	oText = GetRecognizedText(piX, piY);

	if (oText!=NULL) {

		cResult = oText->GetCharacter(piX-oText->GetBoundingBox().X, piY-oText->GetBoundingBox().Y);
		}

	return cResult;
}

//This function rotates the texts within this recognition layer
void 
CRecognitionLayer::DoRotate(DWORD pdAngle, RectF pcRectangleWithinRotation) {

	long lIndex;

	lIndex = oTexts.size();

	while (lIndex>0) {
	
		lIndex--;

		oTexts[lIndex]->DoRotate(pdAngle, pcRectangleWithinRotation);
		}
	}

//This function flips the texts within this recognition layer
void 
CRecognitionLayer::DoFlip(DWORD pdOperation, RectF pcRectangleWithinFlip) {

	long lIndex;

	lIndex = oTexts.size();

	while (lIndex>0) {
	
		lIndex--;

		oTexts[lIndex]->DoFlip(pdOperation, pcRectangleWithinFlip);
		}
	}

//This function processes a crop operation. It will adjust the textblock
//to the new size of the page
void 
CRecognitionLayer::DoFill(Rect pcFillRectangle) {

	long lIndex;
	CRecognizedText* oText;
	Rect cBoundingBoxTextBlock;
	Rect cFillRectangle;

	lIndex = oTexts.size();
	cFillRectangle = pcFillRectangle;

	while (lIndex>0) {

		lIndex--;

		//Get a text block and its bounding box
		oText = oTexts[lIndex];
		cBoundingBoxTextBlock = oText->GetBoundingBox();

		if (cFillRectangle.Contains(cBoundingBoxTextBlock)) {

			//this text block is totally inside the filled rectangle,
			//all the recognized characters will be lossed
			DeleteRecognizedText(oText);
			}
		else {

			if (cFillRectangle.IntersectsWith(cBoundingBoxTextBlock)) {

				//this text block is partially part of the new image
				//adjust it 
				oText->SubtractBoundingBox(cFillRectangle);
				}
			}
		}
	}

//This function processes a crop operation. It will adjust the textblock
//to the new size of the page
void 
CRecognitionLayer::DoCrop(RectF pcNewPageRectangle) {

	long lIndex;
	CRecognizedText* oText;
	Rect cBoundingBoxTextBlock;
	Rect cNewPageRectangle;

	lIndex = oTexts.size();
	cNewPageRectangle = Rect((int)pcNewPageRectangle.X, 
							 (int)pcNewPageRectangle.Y, 
							 (int)pcNewPageRectangle.Width, 
							 (int)pcNewPageRectangle.Height);

	while (lIndex>0) {

		lIndex--;

		//Get a text block and its bounding box
		oText = oTexts[lIndex];
		cBoundingBoxTextBlock = oText->GetBoundingBox();

		if (cNewPageRectangle.Contains(cBoundingBoxTextBlock)) {

			//this text block is totally inside the new page, move
			//the block to align it in the new image
			oText->MoveBoundingBox((int)(-pcNewPageRectangle.X), (int)(-pcNewPageRectangle.Y));
			}
		else {

			if (cNewPageRectangle.IntersectsWith(cBoundingBoxTextBlock)) {

				//this text block is partially part of the new image
				//adjust it 
				oText->CutBoundingBox(cNewPageRectangle);
				oText->MoveBoundingBox((int)(-pcNewPageRectangle.X), (int)(-pcNewPageRectangle.Y));
				}
			else {
				
				//remove this text block, since it is not part of the
				//new image
				DeleteRecognizedText(oText);
			}
		}
	}
}

//Copy all the text blocks to the clipboard
void 
CRecognitionLayer::DoCopyTextToClipboard(HWND phWnd) {

	std::wstring sText;
	long lIndex;

	sText = L"";

	lIndex = oTexts.size();

	while (lIndex>0) {

		lIndex--; 

		sText = oTexts[lIndex]->GetText() + L"\n\r" + sText;
		}

	CopyToClipboard(phWnd, sText);
	}

//Copy the textblock at the given location to the clipboard
void 
CRecognitionLayer::DoCopyTextBlockToClipboard(HWND phWnd, int piX, int piY) {

	std::wstring sText;
	CRecognizedText* oText;

	sText = L"";
	oText = GetRecognizedText(piX, piY);

	if (oText!=NULL) {

		sText = oText->GetText();
		}

	CopyToClipboard(phWnd, sText);
}
	
//This function deletes the Recognized Text Block at the given location
void 
CRecognitionLayer::DeleteRecognizedText(int piX, int piY) {

	CRecognizedText* oText;

	//Request the text block at the given location
	oText = GetRecognizedText(piX, piY);

	//If an instance is found, delete it
	if (oText!=NULL) {

		DeleteRecognizedText(oText);
		}
	}

//This function deletes the given recognized text block
void 
CRecognitionLayer::DeleteRecognizedText(CRecognizedText* poText) {

	long lIndex;

	lIndex = oTexts.size();

	//Loop through the instances and if the requested instance
	//is found, delete it and remove it from the vector
	while (lIndex>0) {

		lIndex--;

		if (oTexts[lIndex] == poText) {

			oTexts.erase(oTexts.begin() + lIndex);
			delete poText;
			lIndex = 0;
			}	
		}
	}

//Copy the given text to the clipboard
void 
CRecognitionLayer::CopyToClipboard(HWND phWnd, std::wstring psText) {

	long lTextLength;
	HGLOBAL hMem;
	void* pMem;

	//Determine the length of the text in the control
	lTextLength = psText.length();

	//Allocate memory and get the text from the control
	hMem = GlobalAlloc(GMEM_MOVEABLE, (lTextLength + 1) * sizeof(wchar_t));
	pMem = GlobalLock(hMem);

	wcscpy_s((wchar_t*)pMem, lTextLength+1, psText.c_str());

	GlobalUnlock(hMem);

	//Copy the text to the clipboard
	if (OpenClipboard(phWnd)) {

		SetClipboardData(CF_UNICODETEXT, hMem);

		CloseClipboard();
		}
	else {

		//Free the memory to prevent a memory leak
		GlobalFree(hMem);
		}
	}





