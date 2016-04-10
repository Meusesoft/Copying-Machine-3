#pragma once

struct sRecognizedCharacter {

	Rect cBoundingBox;		//Bounding box of the recognized character
	TCHAR cCharacter;		//The character in Unicode format
	int   iSpaces;			//Number of spaces following this character
	int   iEndOfLines;		//Number of end-of-lines following this character
	};

class CRecognizedText {

	public:
		CRecognizedText(Rect pcBoundingBox, LANGID pcLanguage);
		~CRecognizedText();

		LANGID		GetLanguage();
		Rect		GetBoundingBox();
		void		CutBoundingBox(Rect pcRectangle);
		void		MoveBoundingBox(int piDeltaX, int piDeltaY);
		void		SubtractBoundingBox(Rect pcRectangle);
		DWORD		GetMemorySize();


		int			GetCharacterCount();
		Rect		GetCharacterBoundingBox(long plIndex);
		TCHAR		GetCharacter(long plIndex);
		sRecognizedCharacter		GetCharacter(int piX, int piY);
		sRecognizedCharacter		GetCharacterStruct(long plIndex);
		std::wstring GetText();

		void AddCharacter(TCHAR pcCharacter, Rect pcBoundingBox, long plNumberSpaces, long plNumberEndOfLines);
		
		void DoRotate(DWORD pdAngle, RectF cRectangleWithinRotation);
		void DoFlip(DWORD pdOperation, RectF cRectangleWithinFlip);

	private:

		LANGID	cLanguage;
		Rect	cBoundingBox;

		vector <sRecognizedCharacter> oCharacters;	//The characters inside this bounding box;

	};

class CRecognitionLayer {

	public:
		CRecognitionLayer();
		~CRecognitionLayer();

		CRecognizedText* CreateRecognizedText(Rect pcBoundingBox, LANGID pcLanguage);
		int GetRecognizedTextCount();
		CRecognizedText* GetRecognizedText(long plIndex);
		CRecognizedText* GetRecognizedText(int piX, int piY);
		sRecognizedCharacter GetCharacter(int piX, int piY);
		CRecognitionLayer* Copy();
		DWORD		GetMemorySize();

		void Clear();
		void DeleteRecognizedText(int piX, int piY);
		void DeleteRecognizedText(CRecognizedText* poText);

		void DoRotate(DWORD pdAngle, RectF cRectangleWithinRotation);
		void DoFlip(DWORD pdOperation, RectF cRectangleWithinFlip);
		void DoCrop(RectF pcNewPageRectangle);
		void DoFill(Rect pcFillRectangle);
		void DoCopyTextToClipboard(HWND phWnd);
		void DoCopyTextBlockToClipboard(HWND phWnd, int piX, int piY);

	private:

		void CopyToClipboard(HWND ohWnd, std::wstring psText);

		vector <CRecognizedText*> oTexts;	//The recognized texts in this layer;
	};


   