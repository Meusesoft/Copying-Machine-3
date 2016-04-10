#pragma once
#include "DocumentCollections.h"

class CDlgPreferencesAutoSave :
	public CDialogTemplate
{
public:
	CDlgPreferencesAutoSave(HINSTANCE hInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances);
	~CDlgPreferencesAutoSave(void);

	virtual void OnInitDialog();
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(int iCommand, WPARAM wParam, LPARAM lParam); 
	virtual	void OnCloseOk();
	virtual void OnApply();

	virtual void Localize(HINSTANCE phLanguage=NULL);


private:

	void DoValidateControls();
	void DoFillControls();
	void DoFillDocumentTypeForm(int piItem);
	void DoFillFileTypeCombo();
	void DoFillColorDepth();
	void DoFillResolution();
	void DoSelectFileType(int pcFileType);
	void DoSelectColorDepth(int piColorDepth);
	void DoSelectResolution(int piResolution);
	void DoSelectComboboxItem(int piComboId, int piItem);

	void DoBrowseForDefaultFolder();
	void DoProcessChanges();
	void DoSynchronizeComboBox(int piSelectedItem=1);

	void DoShowTagMenu();
	void DoAddTag(std::wstring psTag);
	
	void OnChangeDocumentType();
	void OnDeleteDocumentCollection();
	void OnKillFocusEditName();

	bool bInitialised;
	bool bSuspendUpdate;
	CDocumentCollections* oDocumentCollections;
	sEditControlValidation* oEditControlValidation;
};
