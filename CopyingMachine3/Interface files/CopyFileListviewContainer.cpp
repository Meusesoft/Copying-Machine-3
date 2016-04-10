#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "CopyFileListviewContainer.h"


CCopyFileListviewContainer::CCopyFileListviewContainer(sGlobalInstances poGlobalInstances,
													   int piTitle,
													   std::wstring sWindowName) :
	   CWindowContainer(poGlobalInstances, piTitle, sWindowName) {
	
	oFileListview = NULL;

//	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	}

CCopyFileListviewContainer::~CCopyFileListviewContainer(void) {

	CCopyingMachineCore* oCore;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	oCore->oNotifications->UnsubscribeAll(hWnd);

	RemoveChildWindow(oFileListview);
	delete oFileListview;

	Destroy();
	}

//Create the copy toolbar
HWND 
CCopyFileListviewContainer::Create(HWND phParent, HINSTANCE phInstance, int piButtons) {
	
	sWindowContainerButton cWCButton;
	
	oTrace->StartTrace(__WFUNCTION__);

	CWindowContainer::Create(phParent, phInstance, piButtons);

	//Create and add the filelistview
	oFileListview = new CCopyFileListview(oGlobalInstances, L"WPages");
	oFileListview->Create(hWnd, oGlobalInstances.hInstance, 0);
	AddChildWindow(oFileListview);

	SetContainedWindow(oFileListview->hWnd);

	//add the folderup button
	sWindowContainerButton cButton;

	cButton.hCommandWindow = hWnd;
	cButton.iCommand       = IDC_FOLDERUP;
	cButton.iTooltip	   = IDS_FOLDERUP;
	cButton.sImage		   = L"PNG_TOOLFOLDERGO";
	cButton.bImageHot	   = false;
	cButton.bVisible	   = false;

	AddButton(cButton);

	Close();

	//Subscribe to notification
	CCopyingMachineCore* oCore;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	oCore->oNotifications->Subscribe(hWnd, eNotificationDocumentCollectionFolderSelect);

	oTrace->EndTrace(__WFUNCTION__, (bool)(hWnd!=NULL));

	return hWnd;
	}

//This function changes the items in this container to the current language
void
CCopyFileListviewContainer::Localize(HINSTANCE phLanguage) {

	CWindowContainer::Localize(phLanguage);

	if (oFileListview) oFileListview->Localize(phLanguage);
	}

//This function processes all the messages, it returns true if it processed a message
bool 
CCopyFileListviewContainer::WndProc(HWND phWnd, UINT message, 
						   WPARAM wParam, LPARAM lParam, 
						   int &piReturn) {

	bool bReturn;

	bReturn = false;

	switch (message) {

		case WM_CORENOTIFICATION:

			if (phWnd == hWnd) {

				bReturn = OnEventCoreNotification();
				}
			bReturn=false;
			break;
		}

	if (!bReturn) bReturn = CWindowContainer::WndProc(phWnd, message, wParam, lParam, piReturn);

	return bReturn;
	}

//This function processes the WM_COMMAND message
bool 
CCopyFileListviewContainer::OnCommand(int piCommand) {

	bool bResult;

	bResult = true;

	switch (piCommand) {

		case IDC_REFRESHFILELIST:
			oFileListview->Refresh();
			break;

		case IDC_FOLDERUP:
			oFileListview->DoFolderBack();
			break;

		case IDC_EXPLORE:
			ShellExecute(NULL, L"explore", oFileListview->GetFolder().c_str(), NULL, NULL, SW_SHOWNORMAL);
			break;

		case IDC_EXPLOREFOLDER:
			ShellExecute(NULL, L"explore", oFileListview->GetSelectedFolder().c_str(), NULL, NULL, SW_SHOWNORMAL);
			break;

		case IDC_OPENFILEFROMCOLLECTION:
			
			int iItem;

			iItem = oFileListview->GetSelectedItem();
			oFileListview->DoOpenFile(iItem);
			break;

		default:
			bResult = false;
			break;
		}

	if (!bResult) bResult = CWindowContainer::OnCommand(piCommand);

	return bResult;
	}

bool 
CCopyFileListviewContainer::OnEventCoreNotification() {

	bool bReturn;
	sCoreNotification cNotification;
	CCopyingMachineCore* oCore;
	sWindowContainerButton cButton;

	bReturn = false;
	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;

	while (oCore->oNotifications->GetNotification(this->hWnd, cNotification)) {

		switch (cNotification.eNotification) {

			case eNotificationDocumentCollectionFolderSelect:

				//Test if there is folderhistory, and if so show the icon
				GetButton(IDC_FOLDERUP, cButton);

				cButton.bVisible = oFileListview->FolderHistory();
				cButton.bImageHot = oFileListview->FolderHistory();

				UpdateButton(IDC_FOLDERUP, cButton);

				//Update the title
				SetTitle(oFileListview->GetFolder());

				InvalidateRect(hWnd, NULL, false);
				break;
			}
		}

	return bReturn;
	}



