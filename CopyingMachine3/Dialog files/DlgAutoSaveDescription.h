#pragma once

class CDlgAutoSaveDescription :
	public CDialogTemplate
{
public:
	CDlgAutoSaveDescription(HINSTANCE hInstance, HWND phParent, sGlobalInstances poGlobalInstances);
	~CDlgAutoSaveDescription(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void Localize();

	void SetDocumentCollection(int piDocumentType);
	void SetDocumentId(int piDocumentId);
	std::wstring GetDescription();

private:

	void DoShowControls();

	void OnChangeDescription();


	CDocumentCollections* oDocumentCollections;
	sEditControlValidation* oEditControlValidation;

	int iDocumentType;
	int iDocumentId;
	std::wstring sDescription;
};
