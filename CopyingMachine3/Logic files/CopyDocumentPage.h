#pragma once

struct sCopyDocumentSelection {

	float fLeft;		//coordinates of rectangle.
	float fRight;
	float fTop;
	float fBottom;

	bool bOuter;		//true: outer part of rectangle is selected. false: inner part.
	bool bEnabled;		//true: selection is enabled
	};

struct sCopyDocumentPageProperties {
	
	CBitmap*	oImage;
	DWORD		dPageID;
	int			iDocumentID;
	bool		bDirty;
	PixelFormat cPixelFormat;
	PropertyItem*	pMetadata;
	int				iMetadataCount;

	sCopyDocumentSelection cSelection;
	CRecognitionLayer* oRecognitionLayer;
	};

enum ePageImageAction {eImageCrop, eImageFillRectangle, eImageRotate, 
					   eImageFlipHorizontal, eImageFlipVertical,
					   eImageGrayscale, eImageBlackWhite,
					   eImageCopy, eImageCut, ePageDelete, ePageCut, ePageCopy,
					   ePageSplit, eImageChangeDPIX, eImageChangeDPIY,
					   eImageChangeSelection};


class CCopyDocumentPage : 
	public CScriptObject
{
public:
	CCopyDocumentPage(sGlobalInstances pInstances, int piDocumentId, int piPageID);
	~CCopyDocumentPage(void);

	CBitmap* GetImage();
	void SetImage(CBitmap* poBitmap, bool pbUpdateMetadata=true);
	void SetImage(Bitmap* poBitmap, bool pbUpdateMetadata=true);
	void SetImage(HGLOBAL phImage, bool pbUpdateMetadata=true);
	CBitmap* SwapImage(CBitmap* poImage);

	int GetResolutionX();
	void SetResolutionX(int piResolutionX);
	int GetResolutionY();
	void SetResolutionY(int piResolutionY);
	DWORD GetPageID();
	int	  GetDocumentID();
	void SetPageID(int piDocumentId, int piPageID);
	bool GetDirty();
	void SetDirty(bool pbDirty);
	void GetSelection(sCopyDocumentSelection &pcSelection);
	void SetSelection(sCopyDocumentSelection pcSelection);
	CRecognitionLayer* GetRecognitionLayer();
	void SetRecognitionLayer(CRecognitionLayer* poLayer);
	
	DWORD GetMemorySize();
	SizeF GetPhysicalSize(eSizeMeasure pcMeasure=eSizeMeasureUnknown);

	void GetMetadata();
	void SetMetadata();

	sCopyDocumentPageProperties GetAllProperties();

	//Action on the page
	eActionResult DoCropImage();						//Resize the image to the size of the selection
	eActionResult DoFlipImage(DWORD dFlip = 0);			//0=Horizontal or else Vertical
	eActionResult DoRotateImage(DWORD dRotation);		//Rotate the page
	eActionResult DoFillRectangle(DWORD dColor);		//Fill the selection with the give color 
	eActionResult DoCopyImage(HWND phWnd);				//Copy the selection to the clipboard
	eActionResult DoCutImage(DWORD dColor, HWND phWnd);	//Cut the selection and put it on the clipboard
	eActionResult DoConvertGrayscale(DWORD pdData);		//Convert the image to grayscale
	eActionResult DoConvertBlackWhite(DWORD pdData);	//Convert the image to black/white
	bool CheckPageImageAction(ePageImageAction pcAction, DWORD pdData);

	//Script function
	virtual bool Execute(std::wstring psObjectname, 
						 sScriptExpression &psValue);
	virtual bool Set(std::wstring psObjectname, sScriptExpression psNewValue);
	virtual sScriptExpression Get(std::wstring psObjectname);

private:

	sGlobalInstances oGlobalInstances;
	sCopyDocumentPageProperties oProperties;

	void DoRotateSelection(DWORD pdRotation);
	void DoFlipSelection(DWORD pdFlip);
	Bitmap* GetBitmap();

	CTracer* oTrace;
};
