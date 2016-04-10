#pragma once

enum eUndoRedoType {

	eUndoRedoAction,		//the UndoRedo describes the action performed on the page
	eUndoRedoImage,			//the UndoRedo contains the image to restore/redo the last action
	eUndoRedoBitmapPart,	//the UndoRedo contains the bitmap to restore/redo the last action
	eUndoRedoProperty		//the UndoRedo contains the last state of the property (ie DPI)
	};

struct sCopyDocumentPageUndoRedoImage {

	PointF			 cBitmapRect;	  //the position of the bitmap;
	Bitmap*			 oBitmap;		  //the contents of the partial bitmap
	};

struct sCopyDocumentPageUndoRedo {

	int				iDocumentId;			//the document to which it is related
	DWORD			dPageId;				//the page to which it is related
	std::wstring	sDescription;			//a description of the undo action
	eUndoRedoType	cUndoRedoType;			//the type of undoredo action

	ePageImageAction cUndoRedoAction;		//the action performed on the page
	DWORD			 dUndoRedoActionData;
	CBitmap*		 oPageImage;			//the bitmap containing the last version of the page (or a part of it)
	int				 iValue;				//the last value of the changed property (ie DPI)
	vector			 <sCopyDocumentPageUndoRedoImage> oBitmapParts; //the parts of the bitmap which can be restored
	sCopyDocumentSelection cSelection;		//the selection of the page
	CRecognitionLayer* oRecognitionLayer;	//a recognition layer
	};

class CCopyPageUndoRedo
{
public:
	CCopyPageUndoRedo(sGlobalInstances pInstances);
	~CCopyPageUndoRedo(void);

	int GetUndoCount();
	int GetRedoCount();

	void EnableUndo(bool pbEnable);

	bool MakeUndoPoint(CCopyDocumentPage* poPage, ePageImageAction pcImageAction, DWORD pdActionData);
	bool Undo();
	bool Redo();

	void ClearUndoRedo();					//Clear the complete queue
	void ClearUndoRedo(int piDocumentId);	//Clear all undo points for the given document
	void ClearUndoRedo(DWORD pdPageId);		//Clear all undo points for the given page

private:

	void DeleteUndoPoint();
	void DeleteRedoPoint();
	void DeleteUndoRedoPoint(bool bUndo, long plIndex);
	void AnalyseMemoryUsage();
	DWORD GetMemorySizeUndoRedoPoint(sCopyDocumentPageUndoRedo *oUndoRedoPoint);
	bool DoUndo(CCopyDocumentPage* poPage, sCopyDocumentPageUndoRedo* cUndoPoint);
	bool DoRedo(CCopyDocumentPage* poPage, sCopyDocumentPageUndoRedo* cRedoPoint);
	bool DoMakeUndoPointFillRectangle(CCopyDocumentPage* poPage, sCopyDocumentPageUndoRedo* poUndoPoint);

	CTracer* oTrace;
	sGlobalInstances oGlobalInstances;

	vector<sCopyDocumentPageUndoRedo*> oUndo;
	vector<sCopyDocumentPageUndoRedo*> oRedo;

	bool bUndoEnabled;
};
