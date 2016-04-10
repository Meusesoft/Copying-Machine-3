#include "StdAfx.h"
#include "Dialogs.h"
#include "CopyingMachineCore.h"
#include "shobjidl.h"

#define BCM_SETSHIELD            (BCM_FIRST + 0x000C)
#define Button_SetElevationRequiredState(hwnd, fRequired) \
    (LRESULT)SNDMSG((hwnd), BCM_SETSHIELD, 0, (LPARAM)fRequired)

CDlgPreferencesGeneral::CDlgPreferencesGeneral(HINSTANCE phInstance, HWND phParent, int piResource, sGlobalInstances poGlobalInstances) :
	CDialogTemplate(phInstance, phParent, piResource, poGlobalInstances) {

	bInitialised = false;
	}

CDlgPreferencesGeneral::~CDlgPreferencesGeneral(void)
{
	if (hDlg!=NULL) DestroyWindow(hDlg);
}

void 
CDlgPreferencesGeneral::OnInitDialog() {

	DoFillControls();
	DoValidateControls();

	bInitialised = true;
	}
	
//this function fills/initialises the controls of the dialog
void 
CDlgPreferencesGeneral::DoFillControls() {

	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;
	
	//Set the associate stuff
	SendDlgItemMessage(hDlg, IDC_FILEPCX, BM_SETCHECK, DoTestAssociation(L".pcx") ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_FILEJPG, BM_SETCHECK, DoTestAssociation(L".jpg") ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_FILETIFF, BM_SETCHECK, DoTestAssociation(L".tif") ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_FILEBMP, BM_SETCHECK, DoTestAssociation(L".bmp") ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_FILEPNG, BM_SETCHECK, DoTestAssociation(L".png") ? BST_CHECKED : BST_UNCHECKED, 0);

	//Set the JPEG and TIFF compression data
	long lJPEGCompression = oGlobalInstances.oRegistry->ReadInt(L"General", L"JPEGcompression", 75);
	wchar_t cJPEGCompression[10];

	swprintf_s(cJPEGCompression, 10, L"%d", lJPEGCompression);
	SendDlgItemMessage(hDlg, IDC_JPEGCOMPRESSION, WM_SETTEXT, 0, (LPARAM)cJPEGCompression);
	SendDlgItemMessage(hDlg, IDC_JPEGCOMPRESSION, EM_SETLIMITTEXT, (WPARAM)3, 0);
	SendDlgItemMessage(hDlg, IDC_JPEGSPIN, UDM_SETRANGE, (WPARAM)0, (LPARAM)100);

	SendDlgItemMessage(hDlg, IDC_TIFFCOMPRESSION, BM_SETCHECK, oGlobalInstances.oRegistry->ReadInt(L"General", L"TIFFcompression", 0) == 0 ? BST_UNCHECKED : BST_CHECKED, 0);
	
	//Set the Scanner Events checkbox
	//SendDlgItemMessage(hDlg, IDC_CHKSCANNEREVENTS, BM_SETCHECK, oGlobalInstances.oRegistry->ReadInt(L"General", L"StillImage", 0) == 0 ? BST_UNCHECKED : BST_CHECKED, 0);

	//Enable association controls
	//Get the version of windows. Association only in WindowsXP
	OSVERSIONINFO cVersionInformation;
	eWindowsVersion cWindowsVersion;

	cWindowsVersion = eWindowsUnknown;
	cVersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	if (GetVersionEx(&cVersionInformation)) {

		cWindowsVersion = eWindowsOlderThanXP;
		if (cVersionInformation.dwMajorVersion==5 && cVersionInformation.dwMinorVersion>0) 
			cWindowsVersion = eWindowsXP;
		if (cVersionInformation.dwMajorVersion>=6) cWindowsVersion = eWindowsNewerThanVista;
		if (cVersionInformation.dwMajorVersion==6 && cVersionInformation.dwMinorVersion==0) cWindowsVersion = eWindowsVista;
		}

	EnableControl(IDC_FILEPCX, cWindowsVersion == eWindowsXP);
	EnableControl(IDC_FILEJPG, cWindowsVersion == eWindowsXP);
	EnableControl(IDC_FILETIFF, cWindowsVersion == eWindowsXP);
	EnableControl(IDC_FILEBMP, cWindowsVersion == eWindowsXP);
	EnableControl(IDC_FILEPNG, cWindowsVersion == eWindowsXP);

	EnableControl(IDC_STILLIMAGEUNREGISTER, oGlobalInstances.oRegistry->ReadIntLocalMachine(L"General", L"StillImage", 0) != 0);
	EnableControl(IDC_STILLIMAGEREGISTER,   oGlobalInstances.oRegistry->ReadIntLocalMachine(L"General", L"StillImage", 0) != 1);

	if (cWindowsVersion == eWindowsVista || cWindowsVersion == eWindowsNewerThanVista) {

		HWND hButtonWnd;

		hButtonWnd = GetDlgItem(hDlg, IDC_STILLIMAGEUNREGISTER);
		Button_SetElevationRequiredState(hButtonWnd, true);
		hButtonWnd = GetDlgItem(hDlg, IDC_STILLIMAGEREGISTER);
		Button_SetElevationRequiredState(hButtonWnd, true);
		}

	//Fill the language box
	DoFillLanguageControl();
	}
	
//This function fills the language combobox
void 
CDlgPreferencesGeneral::DoFillLanguageControl() {

	CCopyingMachineCore* oCmCore;
	wchar_t		cText[100];
	int			iItemIndex;
	LANGID		lLanguageId;
	int			iInsertIndex;
	int			iSelectedItem;
	int			iCurrentSetting;
	vector <std::wstring> sLanguages;

	oCmCore = (CCopyingMachineCore*)oCore;

	//Clear the combo box
	SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_RESETCONTENT, 0, 0);

	iCurrentSetting = oCmCore->GetPreferredLanguage();
	iSelectedItem = 0;

	//Fill the combobox with the initial choice, local regional setting
	LoadString(oGlobalInstances.hLanguage, IDS_LOCALLANGUAGE, cText, 100);
	iItemIndex=(int)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_ADDSTRING, 0, (LPARAM)cText);
	if (iItemIndex!=CB_ERR) {
		SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)0);
		}

	//Fill the combobox, loop through the available languages
	for (long lIndex=0; lIndex<oCmCore->GetLanguageCount(); lIndex++) {

		lLanguageId = oCmCore->GetLanguage(lIndex);

		VerLanguageName(lLanguageId, cText, 100);
		
		iInsertIndex = -1;

		for (long lIndex=0; lIndex<(long)sLanguages.size() && iInsertIndex==-1; lIndex++) {
			
			if (wcscmp(cText, sLanguages[lIndex].c_str())<0) iInsertIndex=lIndex+1;
			}

		iItemIndex=(int)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_INSERTSTRING, (WPARAM)iInsertIndex, (LPARAM)cText);
		sLanguages.insert(sLanguages.begin(), cText);
		if (iItemIndex!=CB_ERR) {
			SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)lLanguageId);
			}
		}

	//determine which item to select
	long lIndex = (long)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_GETCOUNT, 0, 0);

	if (lIndex!=CB_ERR) {

		while (lIndex>0) {

			lIndex--;

			if (SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_GETITEMDATA, lIndex, 0)==iCurrentSetting) {

				iSelectedItem = lIndex;
				}			
			}
		}

	//Select an entry
	SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_SETCURSEL, (WPARAM)iSelectedItem, 0);
	}


BOOL 
CDlgPreferencesGeneral::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {

	BOOL bReturn;

	bReturn = false;
	
	switch (message) {
	
		case WM_CTLCOLORSTATIC:

             if ((HWND) lParam == GetDlgItem(hDlg, IDC_CAPTION)) { 
                 bReturn=((LONG) GetStockObject(WHITE_BRUSH)); 
				}

			break;
		};	


	return bReturn;
	}

BOOL 
CDlgPreferencesGeneral::OnCommand(int iCommand, WPARAM wParam, LPARAM lParam) {


	if (!bInitialised) return FALSE;

	switch (iCommand) {

		case IDC_TIFFCOMPRESSION: 
		case IDC_FILEBMP:
		case IDC_FILEJPG:
		case IDC_FILEPCX:
		case IDC_FILETIFF:
		case IDC_FILEPNG:
		case IDC_LANGUAGE:
			SendMessage(hParent, WM_ENABLEAPPLY, 0, 0); 
			break;
		
		case IDC_JPEGCOMPRESSION:

			if (oGlobalInstances.oRegistry->ReadInt(L"General", L"JPEGcompression", 75)!=
				GetIntegerFromEditControl(IDC_JPEGCOMPRESSION)) {

				DoValidateControls();
				SendMessage(hParent, WM_ENABLEAPPLY, 0, 0);
				}

			break;

		//case IDC_CHKSCANNEREVENTS:

			//DoStillImage();
			//break;

		case IDC_STILLIMAGEREGISTER:

			DoRegisterStillImage(true);
			break;

		case IDC_STILLIMAGEUNREGISTER:

			DoRegisterStillImage(false);
			break;
		}

	return FALSE;
	}

//This function handles the OK event
void
CDlgPreferencesGeneral::OnCloseOk() {


	}

//This function handles the localize action
void 
CDlgPreferencesGeneral::Localize(HINSTANCE phLanguage) {

	CDialogTemplate::Localize(phLanguage);

	LocalizeControl(IDC_TOPIC1, IDS_LANGUAGE);
	LocalizeControl(IDC_TOPIC2, IDS_ASSOCIATE);
	LocalizeControl(IDC_TOPIC3, IDS_JPEGCOMPRESSION);
	LocalizeControl(IDC_TOPIC4, IDS_TIFFCOMPRESSION);
	LocalizeControl(IDC_TOPIC5, IDS_AFTERACQUISITION);

	LocalizeControl(IDC_RADIOASSUME, IDS_ASSUME);
	LocalizeControl(IDC_RADIOWARNING, IDS_GIVEWARNING);
	LocalizeControl(IDC_FILETYPES, IDS_FILETYPES);
	LocalizeControl(IDC_TXTEVENTS, IDS_SCANNEREVENTS);
	LocalizeControl(IDC_STILLIMAGEREGISTER, IDS_REGISTER);
	LocalizeControl(IDC_STILLIMAGEUNREGISTER, IDS_UNREGISTER);
	LocalizeControl(IDC_TIFFCOMPRESSION, IDS_TIFFCOMPRESSIONDEFAULT);
	LocalizeControl(IDC_SCANNEXTPAGE, IDS_ASKTOSCANNEXTPAGE);
	LocalizeControl(IDC_INVERTIMAGE, IDS_INVERTBWIMAGE);

	DoFillLanguageControl();
	}

//This function handles the Apply event
void 
CDlgPreferencesGeneral::OnApply() {

	CCopyingMachineCore* oCmCore;

	oCmCore = (CCopyingMachineCore*)oCore;

	//Set the compression type of TIFF and the compression level of JPEG
	oGlobalInstances.oRegistry->WriteInt(L"General", L"TIFFcompression", SendDlgItemMessage(hDlg, IDC_TIFFCOMPRESSION, BM_GETCHECK, 0, 0)==BST_CHECKED ? 1 : 0);
	oGlobalInstances.oRegistry->WriteInt(L"General", L"JPEGcompression", GetIntegerFromEditControl(IDC_JPEGCOMPRESSION));

	//Do file associations
	if (SendDlgItemMessage(hDlg, IDC_FILEPCX, BM_GETCHECK, 0, 0)==BST_CHECKED) DoAssociate(L".pcx");
	if (SendDlgItemMessage(hDlg, IDC_FILEPCX, BM_GETCHECK, 0, 0)!=BST_CHECKED) DoRemoveAssociation(L".pcx");
	if (SendDlgItemMessage(hDlg, IDC_FILEJPG, BM_GETCHECK, 0, 0)==BST_CHECKED) DoAssociate(L".jpg");
	if (SendDlgItemMessage(hDlg, IDC_FILEJPG, BM_GETCHECK, 0, 0)!=BST_CHECKED) DoRemoveAssociation(L".jpeg");
	if (SendDlgItemMessage(hDlg, IDC_FILEJPG, BM_GETCHECK, 0, 0)==BST_CHECKED) DoAssociate(L".jpeg");
	if (SendDlgItemMessage(hDlg, IDC_FILEJPG, BM_GETCHECK, 0, 0)!=BST_CHECKED) DoRemoveAssociation(L".jpg");
	if (SendDlgItemMessage(hDlg, IDC_FILEPNG, BM_GETCHECK, 0, 0)==BST_CHECKED) DoAssociate(L".png");
	if (SendDlgItemMessage(hDlg, IDC_FILEPNG, BM_GETCHECK, 0, 0)!=BST_CHECKED) DoRemoveAssociation(L".png");
	if (SendDlgItemMessage(hDlg, IDC_FILEBMP, BM_GETCHECK, 0, 0)==BST_CHECKED) DoAssociate(L".bmp");
	if (SendDlgItemMessage(hDlg, IDC_FILEBMP, BM_GETCHECK, 0, 0)!=BST_CHECKED) DoRemoveAssociation(L".bmp");
	if (SendDlgItemMessage(hDlg, IDC_FILETIFF, BM_GETCHECK, 0, 0)==BST_CHECKED) DoAssociate(L".tif");
	if (SendDlgItemMessage(hDlg, IDC_FILETIFF, BM_GETCHECK, 0, 0)!=BST_CHECKED) DoRemoveAssociation(L".tif");
	if (SendDlgItemMessage(hDlg, IDC_FILETIFF, BM_GETCHECK, 0, 0)==BST_CHECKED) DoAssociate(L".tiff");
	if (SendDlgItemMessage(hDlg, IDC_FILETIFF, BM_GETCHECK, 0, 0)!=BST_CHECKED) DoRemoveAssociation(L".tiff");

	//Set the scanner events check
	//oGlobalInstances.oRegistry->WriteInt(L"General", L"StillImage", SendDlgItemMessage(hDlg, IDC_CHKSCANNEREVENTS, BM_GETCHECK, 0, 0)==BST_CHECKED ? 1 : 0);


	//Set the language settings
	int iLanguage;
	int iItem;
	iItem = (int)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_GETCURSEL, 0, 0);
	if (iItem!=CB_ERR) {

		iLanguage = (int)SendDlgItemMessage(hDlg, IDC_LANGUAGE, CB_GETITEMDATA, iItem, 0);
		
		if (iLanguage!=CB_ERR) {
			
			oCmCore->SetPreferredLanguage(iLanguage);
			}
		}
	}

void
CDlgPreferencesGeneral::DoValidateControls() {

	wchar_t cJPEGCompression[10];
	int		iJPEGCompression;

	//Make sure the JPEG compression edit control is between 0 and 100
	iJPEGCompression = GetIntegerFromEditControl(IDC_JPEGCOMPRESSION);

	if (iJPEGCompression<0 || iJPEGCompression>100) {
		if (iJPEGCompression<0) iJPEGCompression = 0;
		if (iJPEGCompression>100) iJPEGCompression = 100;

		swprintf_s(cJPEGCompression, 10, L"%d", iJPEGCompression);

		SendDlgItemMessage(hDlg, IDC_JPEGCOMPRESSION, WM_SETTEXT, 0, (LPARAM)cJPEGCompression);
		}
	}


//This function tests if the given extension is associated with Copying Machine
bool
CDlgPreferencesGeneral::DoTestAssociation(wstring psExtension) {

	wstring psName;

	CRegistry::ReadClass(psExtension, L"", L"nothing", psName);

	return (psName==L"CopyingMachineImage");
	}

//This function associates the give extension with this installation of Copying Machine
void 
CDlgPreferencesGeneral::DoAssociate(wstring psExtension)
{	
	wchar_t* cApplicationFilename;
	wstring sCommand;
	wstring sIcon;
	DWORD nSize;

	if (!DoTestAssociation(psExtension)) {

		//Get the applications full path and filename
		nSize = MAX_PATH;
		cApplicationFilename = (wchar_t*)malloc(sizeof(wchar_t) * nSize);

		if (GetModuleFileName(NULL, cApplicationFilename, nSize)!=0) {

			//Create the class entry in the registry
			sCommand = cApplicationFilename;
			sCommand += L" \"%1\"";

			CRegistry::WriteClass(L"CopyingMachineImage", L"Copying Machine Image");
			CRegistry::WriteClass(L"CopyingMachineImage\\shell\\open\\command", sCommand);

			sIcon = cApplicationFilename;
			sIcon += L",1";
			CRegistry::WriteClass(L"CopyingMachineImage\\DefaultIcon", sIcon);

			//Associate the file extension
			CRegistry::WriteClass(psExtension, L"CopyingMachineImage");

			//Notify the OS of the change;
			SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
			}

		free(cApplicationFilename);
		}
	}

//This function removes the association between the given extension and Copying Machine
void
CDlgPreferencesGeneral::DoRemoveAssociation(wstring psExtension) {

	if (DoTestAssociation(psExtension)) {

		CRegistry::WriteClass(psExtension, L"");
		}
	}

//This function (un)register Copying Machine with StillImage
void 
CDlgPreferencesGeneral::DoRegisterStillImage(bool pbRegister) {

	std::wstring sCommand;
	bool bRegister;
	wchar_t wAppPath[MAX_PATH];

	//Register with Still Image
	GetModuleFileName(NULL, wAppPath, MAX_PATH);

	//bRegister = (SendDlgItemMessage(hDlg, IDC_CHKSCANNEREVENTS, BM_GETCHECK, 0, 0)==BST_CHECKED);
	bRegister = pbRegister;

	if (bRegister) {

		sCommand = L" /register \"" ;
		sCommand += wAppPath;
		sCommand += L"\"";
		}
	else {

		sCommand = L"/unregister" ;
		}

	//Execute the command
	//CreateProcess(NULL, (LPWSTR)sCommand.c_str(), NULL, NULL, false, NULL, NULL, NULL, NULL, NULL);
	SHELLEXECUTEINFOW cExecInfo;

	SecureZeroMemory(&cExecInfo, sizeof(cExecInfo));
	
	cExecInfo.cbSize = sizeof(cExecInfo);
	cExecInfo.hwnd = hDlg;
	cExecInfo.lpVerb = L"runas";
	cExecInfo.lpFile = wAppPath;
	cExecInfo.lpParameters = sCommand.c_str();
	cExecInfo.nShow = SW_SHOWNORMAL;
	
	if (ShellExecuteEx(&cExecInfo)) {


		}

	//Give the external process some time to finish
	Sleep(1500);

	//Update the dialog
	DoFillControls();
	}
