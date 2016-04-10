#include "StdAfx.h"
#include "DlgOpenSave.h"

CDlgOpenSave::CDlgOpenSave(void)
{
}

CDlgOpenSave::~CDlgOpenSave(void)
{
}

bool 
CDlgOpenSave::ExecuteOpenDialog(HMODULE phLanguage, HWND hParent, wchar_t* pcFileFilters, 
								int &piDefaultFilter, wchar_t* pcFilename) {

    OPENFILENAME cOpenFileName;
    bool bReturnValue;
    long lSizeOfOpenFileName;
	wchar_t cTitle[128];

    //eerste versie van windows bepalen, sizeof OPENFILENAME structure hangt af
    //van de windowsversie
    OSVERSIONINFO lpVersionInformation;

    lpVersionInformation.dwOSVersionInfoSize = sizeof(lpVersionInformation);

    lSizeOfOpenFileName = OPENFILENAME_SIZE_VERSION_400;

    if (GetVersionEx(&lpVersionInformation)!=0) {

        if (lpVersionInformation.dwMajorVersion>=5) {

            lSizeOfOpenFileName = sizeof(OPENFILENAME);
            }
        }

    //nu de OPENFILENAME gaan vullen met info
    ZeroMemory(&cOpenFileName, lSizeOfOpenFileName);

    LoadStringW(phLanguage, IDS_FILEDLGOPENDOCUMENT, cTitle, 128);
	cOpenFileName.lpstrTitle = cTitle;
	cOpenFileName.lStructSize = lSizeOfOpenFileName;//sizeof(cOpenFileName);
    cOpenFileName.hwndOwner = hParent;
    cOpenFileName.lpstrFilter = pcFileFilters;
    cOpenFileName.nFilterIndex = piDefaultFilter;
    cOpenFileName.lpstrFile = pcFilename;
    cOpenFileName.Flags = OFN_FILEMUSTEXIST | OFN_ENABLESIZING | OFN_ENABLEHOOK | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    cOpenFileName.nMaxFile = MAX_PATH;
    cOpenFileName.lpfnHook = LPOFNHOOKPROC(HookProc);

    bReturnValue = (GetOpenFileName(&cOpenFileName)==TRUE);

    if (bReturnValue) {

        piDefaultFilter = cOpenFileName.nFilterIndex;
        }

    return bReturnValue;
	}

bool 
CDlgOpenSave::ExecuteSaveDialog(HMODULE phLanguage, HWND hParent, wchar_t* pcFileFilters, 
								int &piDefaultFilter, wchar_t* pcFilename) {

    OPENFILENAME cOpenFileName;
    bool bReturnValue;
    long lSizeOfOpenFileName;
	wchar_t cTitle[128];

    //eerste versie van windows bepalen, sizeof OPENFILENAME structure hangt af
    //van de windowsversie
    OSVERSIONINFO lpVersionInformation;

    lpVersionInformation.dwOSVersionInfoSize = sizeof(lpVersionInformation);

    lSizeOfOpenFileName = OPENFILENAME_SIZE_VERSION_400;

    if (GetVersionEx(&lpVersionInformation)!=0) {

        if (lpVersionInformation.dwMajorVersion>=5) {

            lSizeOfOpenFileName = sizeof(OPENFILENAME);
            }
        }

    //nu de OPENFILENAME gaan vullen met info
    ZeroMemory(&cOpenFileName, lSizeOfOpenFileName);

    LoadStringW(phLanguage, IDS_FILEDLGSAVEDOCUMENT, cTitle, 128);
	cOpenFileName.lpstrTitle = cTitle;
	cOpenFileName.lStructSize = lSizeOfOpenFileName;//sizeof(cOpenFileName);
    cOpenFileName.hwndOwner = hParent;
    cOpenFileName.lpstrFilter = pcFileFilters;
    cOpenFileName.nFilterIndex = piDefaultFilter;
    cOpenFileName.lpstrFile = pcFilename;
    cOpenFileName.Flags = OFN_FILEMUSTEXIST | OFN_ENABLESIZING | OFN_ENABLEHOOK | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    cOpenFileName.nMaxFile = MAX_PATH;
    cOpenFileName.lpfnHook = LPOFNHOOKPROC(HookProc);

    bReturnValue = (GetSaveFileName(&cOpenFileName)==TRUE);

    if (bReturnValue) {

        piDefaultFilter = cOpenFileName.nFilterIndex;
        }

    return bReturnValue;
	}

UINT
CALLBACK CDlgOpenSave::HookProc(HWND hDlg, UINT msg, WPARAM, LPARAM param2) {

    HWND hParent, hParent2;
    RECT tempRect, dlgRect;
    NMHDR* lpnmhdr;
    POINT newOrigin;
    UINT iResult;

    wchar_t* cFilename;
    wchar_t* cFileFilters;
    wchar_t cExtension[30];
    long lExtensionSize;
	std::wstring sFilter;
    long lFilterIndex;
    long lExtensionPos;
	long lFilterPointer;

	iResult = 0;

    if(msg == WM_NOTIFY) {

        lpnmhdr = (LPNMHDR)param2;

        OFNOTIFY * pon = (OFNOTIFY *)param2;
        OPENFILENAME *pofn = pon->lpOFN;

        switch (lpnmhdr->code) {

            case CDN_INITDONE: {

                //center the dialog in the screen
				hParent = GetParent(hDlg);
                GetWindowRect(hParent, &dlgRect);

                hParent2= GetParent(hParent);
                GetWindowRect(hParent2, &tempRect);

                newOrigin.x = tempRect.left + ((tempRect.right - tempRect.left) - (dlgRect.right - dlgRect.left))/ 2;
                newOrigin.y = tempRect.top + ((tempRect.bottom - tempRect.top) - (dlgRect.bottom - dlgRect.top))/ 2;

                MoveWindow(hParent, newOrigin.x, newOrigin.y, (dlgRect.right - dlgRect.left),
                                        (dlgRect.bottom - dlgRect.top),true);

                iResult = 1;
                break;
                }
			
			case CDN_TYPECHANGE: {

              cFilename = new wchar_t[MAX_PATH];

              lExtensionSize = 0;

              if (cFilename!=NULL) {

                  if (CommDlg_OpenSave_GetSpec(GetParent(hDlg), (WPARAM)cFilename, MAX_PATH)>0) {

                      //we got a filename with maybe an extension...
                      //now lets get the filter extension

                      lFilterIndex = pofn->nFilterIndex;
                      lFilterIndex--;

                      cFileFilters = (wchar_t*)pofn->lpstrFilter;
					  lFilterPointer = 0;

                      while (lFilterIndex>0) {

                          //enumerate through the filters to find the starting position
                          //of the filter we are looking for.
                          while (cFileFilters[lFilterPointer]!=0x00) {
                              lFilterPointer++;
                              }
                          lFilterPointer++;
                          while (cFileFilters[lFilterPointer]!=0x00) {
                              lFilterPointer++;
                              }
                          lFilterPointer++;

                          lFilterIndex--;
                          }

                      //move pass the file filter's description
                      while (cFileFilters[lFilterPointer]!=0x00) {
                          lFilterPointer++;
                          }
                      lFilterPointer++;

                      if (cFileFilters[lFilterPointer]!=0x00) {

                          //move pass the '*.'
                          lFilterPointer++;
                          lFilterPointer++;

                          //copy the first extension into cExtension;

                          lExtensionSize = 0;

                          while (cFileFilters[lFilterPointer]!=0x00 && cFileFilters[lFilterPointer]!=0x3B) {   //0x59 = ;

                              cExtension[lExtensionSize] = cFileFilters[lFilterPointer];

                              lFilterPointer++;
                              lExtensionSize++;
                              }

                          cExtension[lExtensionSize] = (wchar_t)0;

                          if (wcscmp(cExtension, L"*")==0) {

                              cExtension[0]=0x00;
                              lExtensionSize=0;
                              }
                          }

                      //we got the filter extension available in cExtension;
                      //replace current extension of file with the one in
                      //cExtension;

                      if (lExtensionSize>0) {

                          lExtensionPos = (long)wcslen(cFilename) - 1;

                          if (lExtensionPos>0) {

                              while ((lExtensionPos>0) && (cFilename[lExtensionPos]!=0x2E)) {  //0x2E = .
                                  lExtensionPos--;
                                  }

                              if (lExtensionPos>0) {

                                  lExtensionPos++;
                                  cFilename[lExtensionPos]=0x00;
                                  }
                              else {
                                //no . found in the filename, add it now
                                wcscat_s(cFilename, MAX_PATH, L".");
                                }

                                wcscat_s(cFilename, MAX_PATH, cExtension);

                              //change the filename in the dialog
                              SendMessage(GetParent(hDlg), CDM_SETCONTROLTEXT, edt1, LPARAM(cFilename));
                              }
                          }
                      }

                  delete[] cFilename;
                  }

                iResult = 1;
                break;
                }
            }
        }

    return iResult;
}
