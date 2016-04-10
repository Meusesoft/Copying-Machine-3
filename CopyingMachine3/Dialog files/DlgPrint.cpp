#include "StdAfx.h"
#include "DlgPrint.h"
#include "CopyingMachineCore.h"

CDlgPrint::CDlgPrint(void)
{
}

CDlgPrint::~CDlgPrint(void)
{
}

bool 
CDlgPrint::ExecutePrintDialog(sGlobalInstances pcInstances, HWND hParent, void* poPrintSettings) {

	bool bResult;
	HRESULT hResult;
	HGLOBAL hDevNames;
	CPrintSettings* oPrintSettings;
	CRegistry*		oRegistry;

	oRegistry = pcInstances.oRegistry;

	bResult = false;
	oPrintSettings = (CPrintSettings*)poPrintSettings;

	LPPRINTDLGEX	 pPDX		 = NULL;
	LPPRINTPAGERANGE pPageRanges = NULL;
	LPDEVNAMES		 pDevNames	 = NULL;

	// Allocate the PRINTDLGEX structure.
	pPDX = (LPPRINTDLGEX)GlobalAlloc(GPTR, sizeof(PRINTDLGEX));

	// Allocate an array of PRINTPAGERANGE structures.
	pPageRanges = (LPPRINTPAGERANGE) GlobalAlloc(GPTR, 
					   10 * sizeof(PRINTPAGERANGE));

	if (pPDX && pPageRanges) {

		//Initialize the DEVNAMES structure
		long lSize = sizeof(DEVNAMES) + (MAX_PATH * 4 * sizeof(wchar_t)); //reserve a huge space, to avoid errors
		hDevNames = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, lSize*4);
		
		if (hDevNames) {
			pDevNames = (LPDEVNAMES)GlobalLock(hDevNames);
			pDevNames->wDeviceOffset = sizeof(DEVNAMES);
			
			wcscpy_s((wchar_t*)pDevNames+8, lSize-10, oPrintSettings->oPrintData.sDevice.c_str());
			
			GlobalUnlock(hDevNames);
			}



		//Initialize the PRINTDLGEX structure.
		pPDX->lStructSize = sizeof(PRINTDLGEX);
		pPDX->hwndOwner = hParent;
		pPDX->hDevMode = NULL;
		pPDX->hDevNames = hDevNames;
		pPDX->hDC = NULL;
		pPDX->Flags = PD_RETURNDC | PD_NOSELECTION | PD_NOCURRENTPAGE;
		//The collate setting is the only setting read directly from registry. It isn't a
		//setting from the print settings of Copying Machine but we do want to maintain the
		//last value
		if (oRegistry->ReadInt(L"Print", L"Collate", 1)==1) pPDX->Flags |= PD_COLLATE;
		pPDX->Flags2 = 0;
		pPDX->ExclusionFlags = 0;
		pPDX->nPageRanges = 0;
		pPDX->nMaxPageRanges = 10;
		pPDX->lpPageRanges = pPageRanges;
		pPDX->nMinPage = 1;
		pPDX->nMaxPage = (DWORD)oPrintSettings->oPrintData.lPage.size();
		pPDX->nCopies = oPrintSettings->oPrintData.iCopies;
		pPDX->hInstance = 0;
		pPDX->lpPrintTemplateName = NULL;
		pPDX->lpCallback = NULL;
		pPDX->nPropertyPages = 0;
		pPDX->lphPropertyPages = NULL;
		pPDX->nStartPage = START_PAGE_GENERAL;
		pPDX->dwResultAction = 0;

		//  Invoke the Print property sheet.

		hResult = PrintDlgEx(pPDX);

		if ( (hResult == S_OK) &&
				   pPDX->dwResultAction == PD_RESULT_PRINT) {

			// User clicked the Print button, so
			// use the DC and other information returned in the 
			// PRINTDLGEX structure to print the document

			LPDEVMODE pDevMode;
			pDevMode = (LPDEVMODE)GlobalLock(pPDX->hDevMode);

			oPrintSettings->oPrintData.hDC = pPDX->hDC;
			oPrintSettings->oPrintData.iCopies = max(pPDX->nCopies, (DWORD)pDevMode->dmCopies);
			oPrintSettings->oPrintData.bCollate = ((pPDX->Flags & PD_COLLATE) != 0);
			oRegistry->WriteInt(L"Print", L"Collate", oPrintSettings->oPrintData.bCollate ? 1 : 0);
			
			//Fill the device name
			oPrintSettings->oPrintData.sDevice = pDevMode->dmDeviceName;

			GlobalUnlock(pPDX->hDevMode);

			//Fill the page ranges
			if (pPDX->nPageRanges>0) oPrintSettings->oPrintData.lPage.clear();
			DWORD dPage;
			for (long lIndex=0; lIndex<(long)pPDX->nPageRanges; lIndex++) {

				//make sure from is smaller than to
				if (pPageRanges[lIndex].nFromPage > pPageRanges[lIndex].nToPage) {
					dPage = pPageRanges[lIndex].nToPage;
					pPageRanges[lIndex].nToPage = pPageRanges[lIndex].nFromPage;
					pPageRanges[lIndex].nFromPage = dPage;
					}
				
				//fill the lpage vector with the given ranges
				for (long lPageIndex=(long)pPageRanges[lIndex].nFromPage; lPageIndex<=(long)pPageRanges[lIndex].nToPage; lPageIndex++) {
					oPrintSettings->oPrintData.lPage.push_back(lPageIndex);
					}
				}

			bResult = true;
			}

		//Free memory
		lSize = (long)GlobalSize(pPDX->hDevNames);

		if (pPDX->hDevMode	!= NULL) GlobalFree(pPDX->hDevMode); 
		if (pPDX->hDevNames != NULL) GlobalFree(pPDX->hDevNames); 

		lSize++;
		}

	//Free memory allocated by me
	GlobalFree(pPDX);
	GlobalFree(pPageRanges);

	return bResult;
	}

UINT
CALLBACK CDlgPrint::HookProc(HWND hDlg, UINT msg, WPARAM, LPARAM param2) {

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
