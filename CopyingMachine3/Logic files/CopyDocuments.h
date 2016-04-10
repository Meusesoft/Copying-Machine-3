#pragma once

struct sLoadFilePageResult {

	Bitmap* oBitmap;
	HGLOBAL hMemoryBitmap;
	};

struct sLoadFileResult {

	eActionResult eResult;
	eCopyDocumentFiletype eFiletype;
	std::vector<sLoadFilePageResult> oPages;
	};

class CCopyDocuments
{
public:
	CCopyDocuments(sGlobalInstances pInstances);
	~CCopyDocuments(void);

	CCopyDocument* CreateNewDocument();

	int	GetDocumentCount();
	CCopyDocument* GetDocument(long plIndex);
	CCopyDocument* GetDocumentByID(int piDocumentID);
	CCopyDocumentPage* GetPageByID(DWORD pdPageID);

	eActionResult LoadDocument(std::wstring sFilename, CCopyDocument* poResultDocument);

	bool CloseDocument(CCopyDocument* poDocument);

	bool SaveChangesToDirtyDocuments(HWND phWnd);
	int SaveChangesToDirtyDocument(HWND phWnd, CCopyDocument* poDocument, bool bShowSaveChangesDialog);

	eActionResult DoSplitDocument(CCopyDocument* poDocument, DWORD pdPageId, HWND phWnd);

private:

	CTracer* oTrace;
	CCoreNotifications *oNotification;
	sGlobalInstances oGlobalInstances;

	vector<CCopyDocument*> oDocuments;
	int iNewDocumentCounter;

	sLoadFileResult  LoadCompressedTiff(std::wstring psFilename);
};
