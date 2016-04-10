#pragma once

enum eCopyDocumentFiletype {eUnknown, ePDF, ePCX, eJPG, eBMP, ePNG,
							eTIFFCompressed, eTIFFUncompressed, eAny};


struct sCopyDocumentValidFileTypes {

	vector <eCopyDocumentFiletype> eValidFiletypes;
	};

class CCopyDocument	:
	public CScriptObject
{
public:
	CCopyDocument(sGlobalInstances pInstances);
	~CCopyDocument(void);

	int GetDocumentID();

	CCopyDocumentPage* InsertNewPage(long plIndex, HGLOBAL phImage);
	CCopyDocumentPage* InsertNewPage(long plIndex, Bitmap* poImage);
	CCopyDocumentPage* InsertNewPage(long plIndex, CBitmap* poImage);
	CCopyDocumentPage* InsertNewPage(long plIndex, CCopyDocumentPage* poPage);

	CCopyDocumentPage* GetPage(long plIndex);
	CCopyDocumentPage* GetPageByID(DWORD dPageId);

	long GetPageIndex(CCopyDocumentPage* poPage);
	long GetPageIndex(DWORD dPageId);

	int GetPageCount();
	bool GetDirty();
	void SetDirty();
	DWORD GetMemorySize();

	void SetFilename(std::wstring psFilename);
	std::wstring GetFilename();
	void SetTitle(std::wstring psTitle);
	std::wstring GetTitle();

	eCopyDocumentFiletype GetFiletype();
	bool SetFiletype(eCopyDocumentFiletype pcFiletype);
	bool ForceFiletype(eCopyDocumentFiletype pcFiletype);

	bool GetValidFiletypes(sCopyDocumentValidFileTypes &pcValidFiletypes);
	eActionResult SaveAs(HWND phWnd);
	eActionResult Save(HWND phWnd);
	eActionResult AutoSaveSetDocumentName(HWND phWnd);
	eActionResult AutoSave(HWND phWnd);

	static eCopyDocumentFiletype DetermineFiletype(std::wstring psFilename);
	static std::wstring GetFiletypeExtension(eCopyDocumentFiletype pcFiletype);

	eActionResult MailTo();
	eActionResult Print(CPrintSettings* poPrintSettings=NULL);
	int PrintThread(CPrintSettings* poPrintSettings=NULL);

	eActionResult DoDeletePage(DWORD pdData, HWND phWnd);	//Delete the current page
	eActionResult DoCopyPage(DWORD pdData, HWND phWnd);	//Copy the current page
	eActionResult DoCutPage(DWORD pdData, HWND phWnd);	//Cut the current page
	int DoMovePage(DWORD pdData, int iNewPosition); //Move the given page to a new position, it returns the new position of the previous page on that position
	int DoMovePage(DWORD pdData, CCopyDocument* poDocument); //Move the given page to a another document

	bool CheckPageImageAction(ePageImageAction pcAction, DWORD pdData);
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	int	 iNewDocumentID;

	//Script function
	virtual bool Execute(std::wstring psObjectname, 
						 sScriptExpression &psValue);
	virtual bool Set(std::wstring psObjectname, sScriptExpression psNewValue);
	virtual sScriptExpression Get(std::wstring psObjectname);


private:

	sGlobalInstances oGlobalInstances;
	CTracer* oTrace;
	
	//Document properties
	std::wstring sFilename;
	std::wstring sTitle;
	eCopyDocumentFiletype cFiletype;
	eCopyDocumentFiletype cForceFileType; // force the filetype
	int			 iDocumentID;
	int			 iPageIDCounter;
	bool		 bDirty;


	//Seperate pages of the document
	vector<CCopyDocumentPage*> oPages;

	//private functions
	std::wstring ConvertFilenameToTitle();
	bool AddExtension();

	void SendNotification(eCoreNotification peNotification = eNotificationDocumentUpdate);

	eActionResult SaveTiff();
	eActionResult SaveCompressedTiff();
	eActionResult SaveJPG();
	eActionResult SaveBMP();
	eActionResult SavePCX();
	eActionResult SavePDF();
	eActionResult SavePDF(std::wstring psFilename);
};
