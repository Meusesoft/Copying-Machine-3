// OCRLearningCenterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OCRLearningCenter.h"
#include "OCRLearningCenterDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COCRLearningCenterDlg dialog

COCRLearningCenterDlg::COCRLearningCenterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COCRLearningCenterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	oDataLayer = NULL;
	oShapeNeuralNetwork = NULL;
}

void COCRLearningCenterDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_LISTNEURALNETWORKS, oNeuralNetworks);
DDX_Control(pDX, IDC_NEURALNETWORKNAME, oNeuralNetworkName);
DDX_Control(pDX, IDC_NUMBEREXAMPLESPERFOLDER, oNeuralNetworkExamplesPerFolder);
DDX_Control(pDX, IDC_CHECKSUBFOLDERS, oNeuralNetworkCheckSubFolders);
DDX_Control(pDX, IDC_LISTVIEWSHAPES, oShapesListview);
DDX_Control(pDX, IDC_SHAPE, oShapeName);
DDX_Control(pDX, IDC_SHAPEEXAMPLES, oShapeExampleFolder);
DDX_Control(pDX, IDC_NODESLAYER1, oNeuralNodesLayer1);
DDX_Control(pDX, IDC_NODESLAYER2, oNeuralNodesLayer2);
DDX_Control(pDX, IDC_EDIT7, oSampleEdit);
DDX_Control(pDX, IDC_STATUS, oStatus);
DDX_Control(pDX, IDC_RESULTLISTBOX, oResultListbox);
DDX_Control(pDX, IDC_REPETITIONS, oRepetitions);
	}

BEGIN_MESSAGE_MAP(COCRLearningCenterDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(IDC_LOAD, OnLoad)
	ON_COMMAND(IDC_EXIT, OnExit)
	//}}AFX_MSG_MAP
	ON_LBN_SELCHANGE(IDC_LISTNEURALNETWORKS, &COCRLearningCenterDlg::OnNeuralNetworkSelect)
	ON_BN_CLICKED(IDC_BROWSEEXAMPLE, &COCRLearningCenterDlg::OnBrowseShapeFolder)
	ON_BN_CLICKED(IDC_ADDSHAPE, &COCRLearningCenterDlg::OnAddShape)
	ON_BN_CLICKED(IDC_BROWSESAMPLE, &COCRLearningCenterDlg::OnBrowseSample)
	ON_BN_CLICKED(IDC_RUNIT, &COCRLearningCenterDlg::OnRunLearnShapes)
	ON_BN_CLICKED(IDC_ANALYSIS, &COCRLearningCenterDlg::OnAnalysis)
	ON_BN_CLICKED(IDC_MOVESAMPLES, &COCRLearningCenterDlg::OnMoveSamples)
	ON_BN_CLICKED(IDC_SAVE, &COCRLearningCenterDlg::OnSave)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTVIEWSHAPES, &COCRLearningCenterDlg::OnSelectShape)
	ON_BN_CLICKED(IDC_TEST, &COCRLearningCenterDlg::OnTestSample)
END_MESSAGE_MAP()


// COCRLearningCenterDlg message handlers

BOOL COCRLearningCenterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// TODO: Add extra initialization here
	LVCOLUMN cColumn;

	cColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	cColumn.cx = 100;
	cColumn.pszText = L"Shape";
	oShapesListview.InsertColumn(0, &cColumn);

	cColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	cColumn.cx = 500;
	cColumn.pszText = L"Examples";
	oShapesListview.InsertColumn(1, &cColumn);

	//Set the number of nodes in the hidden layers
	oNeuralNodesLayer1.SetWindowText(L"10");
	oNeuralNodesLayer2.SetWindowText(L"5");
	oRepetitions.SetWindowText(L"10");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COCRLearningCenterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COCRLearningCenterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void 
COCRLearningCenterDlg::OnLoad() {

	oDataLayer = new TDataLayer("D:\\Projects\\CopyingMachine3\\debug\\shapenet.xml");
	//oDataLayer->LoadData();
	oDataLayer->LoadShapes("D:\\Projects\\CopyingMachine3\\debug\\shapenet.xml");
	//Fill the dialog its ShapeNetworks combobox
	oNeuralNetworks.ResetContent();
	CString sName;
	int iItem;

	for (int iIndex=0; iIndex<(int)oDataLayer->oShapeNetworks.size(); iIndex++) {

		sName = oDataLayer->oShapeNetworks[iIndex].sName.c_str();
		iItem = oNeuralNetworks.AddString(sName);
		oNeuralNetworks.SetItemData(iItem, iIndex);
		}
	}

void 
COCRLearningCenterDlg::OnExit() {

	if (oDataLayer) delete oDataLayer;
	if (oShapeNeuralNetwork) delete oShapeNeuralNetwork;

	DestroyWindow();
	GdiplusShutdown(gdiplusToken);
	}

void COCRLearningCenterDlg::OnNeuralNetworkSelect()
	{
	int iIndex;

		iIndex = oNeuralNetworks.GetCurSel();

		if (iIndex>=0 && iIndex<(int)oDataLayer->oShapeNetworks.size()) {

			//set the name
			CString sName;
			sName = oDataLayer->oShapeNetworks[iIndex].sName.c_str();
			oNeuralNetworkName.SetWindowText(sName);

			//set the number of examples
			CString sExamples;
			sExamples = oDataLayer->oShapeNetworks[iIndex].sNumberExamples.c_str();
			oNeuralNetworkExamplesPerFolder.SetWindowText(sExamples);

			//set the checkbox for search in subfolders
			oNeuralNetworkCheckSubFolders.SetCheck(oDataLayer->oShapeNetworks[iIndex].bSearchSubFolders ? BST_CHECKED : BST_UNCHECKED);

			
			//fill the listview with shapes
			oShapesListview.DeleteAllItems();

			for (int iItem=0; iItem<(int)oDataLayer->oShapeNetworks[iIndex].oShapeList.size(); iItem++) {

				sName = oDataLayer->oShapeNetworks[iIndex].oShapeList[iItem].sShape.c_str();
				oShapesListview.InsertItem(LVIF_TEXT, iItem, sName, 0, 0, 0, iItem);

				// Initialize the text of the subitems.
				sExamples = oDataLayer->oShapeNetworks[iIndex].oShapeList[iItem].sSampleFolder.c_str();
				oShapesListview.SetItemText(iItem, 1, sExamples);
				}

			//fill the number of nodes in layer one
			int iLayer1, iLayer2;
			wchar_t cText[20];

			iLayer1 = 0;
			iLayer2 = 0;
			TNeuralNetwork* oNetwork;

			oNetwork = oDataLayer->oShapeNetworks[iIndex].oShapeNeuralNetwork;

			for (int iItem=0; iItem<(int)oNetwork->oNodes.size(); iItem++) {

				if (oNetwork->oNodes[iItem]->lLayer==1) iLayer1++;
				if (oNetwork->oNodes[iItem]->lLayer==2) iLayer2++;
				}
			_itow_s(iLayer1, cText, 20, 10);
			oNeuralNodesLayer1.SetWindowText(cText);
			_itow_s(iLayer2, cText, 20, 10);
			oNeuralNodesLayer2.SetWindowText(cText);
		}
	}

void COCRLearningCenterDlg::OnBrowseShapeFolder()
	{
	CString sSelectedFolder;

	if (GetFolder(sSelectedFolder, L"Browse Sample", this->m_hWnd)) {

		oShapeExampleFolder.SetWindowText(sSelectedFolder);
		}
	}

void COCRLearningCenterDlg::OnAddShape()
	{
	int iIndex;
	CString sName;
	CString sExamples;
	int iItem;
	cShapeNetList oShapeNetList;

	iIndex = oNeuralNetworks.GetCurSel();

	//add shape to data layer
	iItem = oDataLayer->oShapeNetworks[iIndex].oShapeList.size();

	oShapeName.GetWindowText(sName);
	oShapeExampleFolder.GetWindowText(sExamples);

	oShapeNetList.lShapeId = iItem;
	oShapeNetList.sShape = CStringW(sName);
	oShapeNetList.sSampleFolder = CStringW(sExamples); 

	//add shape to listview
	oShapesListview.InsertItem(LVIF_TEXT, iItem, sName, 0, 0, 0, iItem);
	oShapesListview.SetItemText(iItem, 1, sExamples);
	}

void COCRLearningCenterDlg::OnBrowseSample()
	{
	CString sSelectedFile;

	CFileDialog* oFileOpenDialog;

	oFileOpenDialog = new CFileDialog(true);
	oFileOpenDialog->GetOFN().lpstrInitialDir = L"D:\\Projects\\TOcr\\CreateCompareDatabase\\DefiningExamples";

	if (oFileOpenDialog->DoModal()==IDOK) {

		sSelectedFile = oFileOpenDialog->GetPathName();
		oSampleEdit.SetWindowText(sSelectedFile);
		}

	delete oFileOpenDialog;
	}

void COCRLearningCenterDlg::OnRunLearnShapes()
	{
	// TODO: Add your control notification handler code here
	LearnAndTest(true);	
	}

void COCRLearningCenterDlg::LearnAndTest(bool pbLearn) {

    Bitmap* oBitmap;
    BYTE* bComparisonBitmap;
	wchar_t sMessage[128];
    cRecognition oRecognition;
    TLearningData* oLearningData;
    vector<cShapeExample> oExamples;
	std::string sBitmapFile;
    cShapeNetExamples oBitmapFiles;
    cShapeExample oExample;
    cCreateBitmapSettings cSettings;
    long lGood;
    long lIndexMax;
    double dMax;
    sNeuralInput oInput;
    sNeuralOutput oOutput;

    cSettings.bGrayValues = true;
    cSettings.bMakeSkeleton = true;
	int iShapeNetworkIndex = oNeuralNetworks.GetCurSel();

	if (pbLearn) {

		if (oShapeNeuralNetwork!=NULL) {

			delete oShapeNeuralNetwork;
			}

		oShapeNeuralNetwork = new TNeuralNetwork();
		}
	else {

		oShapeNeuralNetwork = oDataLayer->oShapeNetworks[iShapeNetworkIndex].oShapeNeuralNetwork;
		}

    //init learning data
    //collect all available examples
	//int iShapeIndex = oShapesListview.GetSelectionMark();
	int iNumberExamples;
	int iRandomExample;
	CString sText;

	oNeuralNetworkExamplesPerFolder.GetWindowText(sText);
	iNumberExamples = _wtoi(sText.GetBuffer(0));

	 oStatus.SetWindowText(L"Search for examples");
	 DoEvents();

	 for (long lIndex=0; lIndex<(int)oDataLayer->oShapeNetworks[iShapeNetworkIndex].oShapeList.size(); lIndex++) {

        oBitmapFiles.sFiles.clear();

 		wsprintf(sMessage, L"Search for examples: %d / %d", lIndex+1, (int)oDataLayer->oShapeNetworks[iShapeNetworkIndex].oShapeList.size());
		oStatus.SetWindowTextW(sMessage);
		DoEvents();

		MakeExampleList(oBitmapFiles, oDataLayer->oShapeNetworks[iShapeNetworkIndex].oShapeList[lIndex].sSampleFolder);

        if (oBitmapFiles.sFiles.size()>0) {

            srand((unsigned)time(NULL));

            for (long lIndex2=0; lIndex2<iNumberExamples; lIndex2++) {

				iRandomExample = (int)((double)rand() / (RAND_MAX + 1) * (oBitmapFiles.sFiles.size()));

                oExample.sFile = oBitmapFiles.sFiles[iRandomExample];
                oExample.lId = lIndex;

                oExamples.push_back(oExample);
                }
             }

		}

	 if (pbLearn) {
	 //fill the network with nodes
		int iNumberNodes1, iNumberNodes2, iRepetitions;

		oNeuralNodesLayer1.GetWindowText(sText);
		iNumberNodes1 = _wtoi(sText.GetBuffer(0));
		oNeuralNodesLayer2.GetWindowText(sText);
		iNumberNodes2 = _wtoi(sText.GetBuffer(0));
		oRepetitions.GetWindowText(sText);
		iRepetitions = _wtoi(sText.GetBuffer(0));

		for (long lIndex=0; lIndex<60; lIndex++) {

			oShapeNeuralNetwork->AddNode(0, eInput);
			}

		for (long lIndex=0; lIndex<(long)oDataLayer->oShapeNetworks[iShapeNetworkIndex].oShapeList.size(); lIndex++) {

			oShapeNeuralNetwork->AddNode(3, eOutput);
			}

		for (long lIndex=0; lIndex<iNumberNodes1; lIndex++) {

			oShapeNeuralNetwork->AddNode(1, eHidden);
			}
		for (long lIndex=0; lIndex<iNumberNodes2; lIndex++) {

			oShapeNeuralNetwork->AddNode(2, eHidden);
			}

		 oStatus.SetWindowText(L"Filling learning data");
		 DoEvents();

		//fill the learning data
		for (long lIndex=0; lIndex<(long)oExamples.size(); lIndex++) {

 			wsprintf(sMessage, L"Fill learning data: %d / %d", lIndex+1, (long)oExamples.size());
			oStatus.SetWindowTextW(sMessage);
			DoEvents();

			oBitmap = new Bitmap(oExamples[lIndex].sFile.c_str());       

			bComparisonBitmap = oDataLayer->CreateBitmapFromFile(oExamples[lIndex].sFile, oBitmap, cSettings);

			oRecognition.cCompareBitmap = bComparisonBitmap;

			oDataLayer->ExtractFeaturesFromBitmap(oRecognition);

			oLearningData = new TLearningData();

			oLearningData->oInput.fInputs.clear();
			oLearningData->oOutput.fOutputs.clear();

			for (long lIndex2=0; lIndex2<5; lIndex2++) {

				oLearningData->oInput.fInputs.push_back((float)oRecognition.lPixelProjectionBackX[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<5; lIndex2++) {

				oLearningData->oInput.fInputs.push_back((float)oRecognition.lPixelProjectionBackY[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<5; lIndex2++) {

				oLearningData->oInput.fInputs.push_back((float)oRecognition.lPixelProjectionForeX[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<5; lIndex2++) {

				oLearningData->oInput.fInputs.push_back((float)oRecognition.lPixelProjectionForeY[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<20; lIndex2++) {

				oLearningData->oInput.fInputs.push_back((float)oRecognition.lStrokeDirectionX[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<20; lIndex2++) {

				oLearningData->oInput.fInputs.push_back((float)oRecognition.lStrokeDirectionY[lIndex2]);
				}

			for (long lIndex2=0; lIndex2<(long)oDataLayer->oShapeNetworks[iShapeNetworkIndex].oShapeList.size(); lIndex2++) {

				if (oExamples[lIndex].lId==lIndex2) {
					oLearningData->oOutput.fOutputs.push_back((float)1);
					}
				else {
					oLearningData->oOutput.fOutputs.push_back((float)0);
					}
				}

			oShapeNeuralNetwork->AddSituation(oLearningData);

			free(bComparisonBitmap);
		
			delete oBitmap;
			}


		oShapeNeuralNetwork->ComputeInputRatios();


		//let the network learn!
		oShapeNeuralNetwork->hStatusLabel = oStatus.GetSafeHwnd();
		oShapeNeuralNetwork->Learn(iRepetitions);

		oStatus.SetWindowText(L"Learning done");
		DoEvents();
		}

	if (!pbLearn) {
		//compute success percentage of learned examples
		lGood = 0;

		for (long lIndex=0; lIndex<(long)oExamples.size(); lIndex++) {

 			wsprintf(sMessage, L"Compute succes percentage: %d / %d examples", lIndex+1, (long)oExamples.size()); 
			oStatus.SetWindowText(sMessage);
			DoEvents();

			oBitmap = new Bitmap(oExamples[lIndex].sFile.c_str());

			bComparisonBitmap = oDataLayer->CreateBitmapFromFile(oExamples[lIndex].sFile, oBitmap, cSettings);

			oRecognition.cCompareBitmap = bComparisonBitmap;

			oDataLayer->ExtractFeaturesFromBitmap(oRecognition);

			oInput.fInputs.clear();
			oOutput.fOutputs.clear();

			for (long lIndex2=0; lIndex2<5; lIndex2++) {

				oInput.fInputs.push_back((float)oRecognition.lPixelProjectionBackX[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<5; lIndex2++) {

				oInput.fInputs.push_back((float)oRecognition.lPixelProjectionBackY[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<5; lIndex2++) {

				oInput.fInputs.push_back((float)oRecognition.lPixelProjectionForeX[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<5; lIndex2++) {

				oInput.fInputs.push_back((float)oRecognition.lPixelProjectionForeY[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<20; lIndex2++) {

				oInput.fInputs.push_back((float)oRecognition.lStrokeDirectionX[lIndex2]);
				}
			for (long lIndex2=0; lIndex2<20; lIndex2++) {

				oInput.fInputs.push_back((float)oRecognition.lStrokeDirectionY[lIndex2]);
				}


			oShapeNeuralNetwork->ComputeOutput(oInput, oOutput);

			dMax=0;
			lIndexMax=0;
			for (long lIndex2=0; lIndex2<(long)oOutput.fOutputs.size(); lIndex2++) {

				if (oOutput.fOutputs[lIndex2]>dMax) {
					dMax = oOutput.fOutputs[lIndex2];
					lIndexMax = lIndex2;
					}
				}

			if (oDataLayer->oShapeNetworks[iShapeNetworkIndex].oShapeList[lIndexMax].sShape == oDataLayer->oShapeNetworks[iShapeNetworkIndex].oShapeList[oExamples[lIndex].lId].sShape) {

				lGood++;
				}

			free(bComparisonBitmap);

			delete oBitmap;
			}

		wsprintf(sMessage, L"Result: %d percent", (lGood * 100 / oExamples.size())); 
		oStatus.SetWindowText(sMessage);
		}

    //update the pointer in the list of shape trees
	if (pbLearn) {
		oDataLayer->oShapeNetworks[iShapeNetworkIndex].oShapeNeuralNetwork = oShapeNeuralNetwork;

		//save the network to be safe
		//OnSave();
		oStatus.SetWindowText(L"Learning completed");
		}
	}

void 
COCRLearningCenterDlg::MakeExampleList(cShapeNetExamples &oBitmapFiles, std::wstring sBaseFolder) {

	_wfinddata_t cFindData;

	intptr_t hFile;
	std::wstring sSearch;
	std::wstring sBitmapFile;

    sSearch = sBaseFolder;
    sSearch += L"\\*.bmp";

	if ((hFile = _wfindfirst(sSearch.c_str(), &cFindData)) != -1) {

       do {

            sBitmapFile = sBaseFolder;
            sBitmapFile += L"\\";
            sBitmapFile += cFindData.name;

            oBitmapFiles.sFiles.push_back(sBitmapFile);

            } while( _wfindnext( hFile, &cFindData ) == 0 );

        _findclose(hFile);
        }

    //loop through sub folders
    sSearch = sBaseFolder;
    sSearch += L"\\*.*";

	if ((hFile = _wfindfirst(sSearch.c_str(), &cFindData)) != -1) {

        do {

			if (cFindData.attrib & _A_SUBDIR) {

				sBitmapFile = cFindData.name;

				if (sBitmapFile!=L"." && sBitmapFile!=L"..") {

					sBitmapFile = sBaseFolder;
					sBitmapFile += L"\\";
					sBitmapFile += cFindData.name;

					MakeExampleList(oBitmapFiles, sBitmapFile);
					}
				}
			} while( _wfindnext( hFile, &cFindData ) == 0 );

         _findclose(hFile);
        }
    }

void COCRLearningCenterDlg::OnAnalysis()
	{
	// TODO: Add your control notification handler code here
	LearnAndTest(false);	
	}

void COCRLearningCenterDlg::OnMoveSamples()
	{
	// TODO: Add your control notification handler code here
	}

void COCRLearningCenterDlg::OnSave()
	{
	// TODO: Add your control notification handler code here
	oDataLayer->SaveShapes("D:\\Projects\\CopyingMachine3\\debug\\shapenet.xml");
	}

void COCRLearningCenterDlg::OnSelectShape(NMHDR *pNMHDR, LRESULT *pResult)
	{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	int iItem;
	int iNetwork;
	CString sName;
	CString sFolder;

	iItem = pNMLV->iItem;
	iNetwork = oNeuralNetworks.GetCurSel();

	sName = oDataLayer->oShapeNetworks[iNetwork].oShapeList[iItem].sShape.c_str();
	oShapeName.SetWindowText(sName);

	sFolder = oDataLayer->oShapeNetworks[iNetwork].oShapeList[iItem].sSampleFolder.c_str();
	oShapeExampleFolder.SetWindowText(sFolder);

	*pResult = 0;
	}

bool COCRLearningCenterDlg::GetFolder(CString& folderpath, 
               const wchar_t* szCaption, 
               HWND hOwner)
{
   bool retVal = false;

   // The BROWSEINFO struct tells the shell 
   // how it should display the dialog.
   BROWSEINFO bi;
   memset(&bi, 0, sizeof(bi));

   bi.ulFlags   = BIF_USENEWUI;
   bi.hwndOwner = hOwner;
   bi.lpszTitle = szCaption;

   // must call this if using BIF_USENEWUI
   ::OleInitialize(NULL);

   // Show the dialog and get the itemIDList for the 
   // selected folder.
   LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

   if(pIDL != NULL)
   {
      // Create a buffer to store the path, then 
      // get the path.
      wchar_t buffer[_MAX_PATH] = {'\0'};
      if(::SHGetPathFromIDList(pIDL, buffer) != 0)
      {
         // Set the string value.
         folderpath = buffer;
         retVal = true;
      }

      // free the item id list
      CoTaskMemFree(pIDL);
   }

   ::OleUninitialize();

   return retVal;
}

void COCRLearningCenterDlg::OnTestSample()
	{
	// TODO: Add your control notification handler code here
    Bitmap* oBitmap;
    BYTE* bComparisonBitmap;
    cRecognition oRecognition;
    cCreateBitmapSettings cSettings;
    sNeuralInput oInput;
    sNeuralOutput oOutput;
	std::string sFilename;
	std::wstring sFilenameW;
	wchar_t sMessage[800];

 	int iNetworkIndex = oNeuralNetworks.GetCurSel();
   cSettings.bGrayValues = true;
    cSettings.bMakeSkeleton = true;

	CString sName;
	oSampleEdit.GetWindowText(sName);
	sFilenameW = CStringW(sName);

	oBitmap = new Bitmap(sFilenameW.c_str());
	//oBitmap = new Bitmap(10,10,PixelFormat24bppRGB);
	sFilename = CStringA(sName);
	bComparisonBitmap = oDataLayer->CreateBitmapFromFile(sFilenameW, oBitmap, cSettings);
    oRecognition.cCompareBitmap = bComparisonBitmap;
	oRecognition.lFont = iNetworkIndex;

    oDataLayer->ExtractFeaturesFromBitmap(oRecognition);

    //put the features through the neural network
    oInput.fInputs.clear();
    oOutput.fOutputs.clear();

    for (long lIndex2=0; lIndex2<5; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lPixelProjectionBackX[lIndex2]);
        }
    for (long lIndex2=0; lIndex2<5; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lPixelProjectionBackY[lIndex2]);
        }
    for (long lIndex2=0; lIndex2<5; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lPixelProjectionForeX[lIndex2]);
        }
    for (long lIndex2=0; lIndex2<5; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lPixelProjectionForeY[lIndex2]);
        }

    for (long lIndex2=0; lIndex2<20; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lStrokeDirectionX[lIndex2]);
        }
    for (long lIndex2=0; lIndex2<20; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lStrokeDirectionY[lIndex2]);
        }


	//add shape to data layer
	oDataLayer->oShapeNetworks[iNetworkIndex].oShapeNeuralNetwork->ComputeOutput(oInput, oOutput);

    //place the results in the listbox
    oResultListbox.ResetContent();
	
    vector<cOutputResult> oOutputSorted;
    cOutputResult oOutputResult;
    bool bAdded;
	std::wstring sCompareResult;

    oOutputSorted.clear();

    for (long lIndex=0; lIndex<(long)oOutput.fOutputs.size(); lIndex++) {

		_swprintf(sMessage, L"%s: %f", oDataLayer->oShapeNetworks[iNetworkIndex].oShapeList[lIndex].sShape.c_str(), oOutput.fOutputs[lIndex]);

        sCompareResult = sMessage;

        bAdded = false;

        oOutputResult.fOutput = oOutput.fOutputs[lIndex];
        oOutputResult.sText = sCompareResult;

        for (long lIndex2=0; lIndex2<(long)oOutputSorted.size() && lIndex2<5 && !bAdded; lIndex2++) {

            if (oOutputResult.fOutput > oOutputSorted[lIndex2].fOutput) {

                oOutputSorted.insert(oOutputSorted.begin() + lIndex2, oOutputResult);
                bAdded = true;
                }
            }

        if (!bAdded && oOutputSorted.size()<5) {
            oOutputSorted.push_back(oOutputResult);
            }
        }

    for (long lIndex=0; lIndex<(long)oOutputSorted.size(); lIndex++) {

		oResultListbox.AddString((LPCTSTR)oOutputSorted[lIndex].sText.c_str());

       // lstShapeNetResults->Items->Add(oOutputSorted[lIndex].sText);
        }
    free (bComparisonBitmap);
    delete oBitmap;
	}

void 
COCRLearningCenterDlg::DoEvents() {

	MSG msg;

	while ((PeekMessage( &msg, NULL, 0, 0 , PM_REMOVE)) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
	}
