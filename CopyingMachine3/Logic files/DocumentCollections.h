#pragma once

LRESULT CALLBACK EditValidateFilename (HWND hwndd, UINT message, WPARAM wParam, LPARAM lParam);

struct sEditControlValidation {

	sGlobalInstances* oGlobalInstances;
	long lOldProc;
	};

struct sDocumentCollection {

	std::wstring sName;
	std::wstring sFilenameTemplate;
	std::wstring sLocation;
	int cFileType;
	unsigned int iColorDepth;		//gekozen kleur: -1=default; 0=BW; 1=Grijs; 2=Kleur
	unsigned int iResolution;  		//gekozen resolutie in dpi : -1=default
	};

class CDocumentCollections {

public:
	CDocumentCollections(sGlobalInstances pInstances);
	~CDocumentCollections();

	int GetDocumentCollectionCount();
	bool GetDocumentCollection(long plIndex, sDocumentCollection &cType);
	bool GetDocumentCollectionIndex(std::wstring psSearch, long& piIndex);
	bool SetDocumentCollection(long plIndex, sDocumentCollection pcType);
	int AddDocumentCollection(sDocumentCollection pcType);
	bool DeleteDocumentCollection(long plIndex);
	
	bool CheckNameEditable(long plIndex);
	bool CheckDocumentTypeDeletable(long plIndex);

	void Load();
	void Save();
	void EnableNotifications(bool pbEnable);

	bool GetDescriptionFromUser(HWND phWnd, long plDocumentType, long plDocumentID);
	void SetDescription(std::wstring psDescription);
	std::wstring CreateFilename(long plDocumentType, long plDocumentId, std::wstring psNewLocation = L"");
	static bool ValidateFilename(std::wstring psFilename);
	static std::wstring RemoveReservedCharactersFromFilename(std::wstring psFilename);

private:

	bool bNotifications; //false if notifications are disabled

	typedef unsigned char base;
	std::wstring itos(int i, base b=10);
	std::wstring DoReplace(std::wstring psSource, std::wstring psFind, std::wstring psReplace,int iMinimumLength=0);

	CTracer*	oTrace;
	CRegistry*	oRegistry;
	sGlobalInstances oGlobalInstances;

	std::wstring sDescription;
	vector <sDocumentCollection> oDocumentCollections;
	};

