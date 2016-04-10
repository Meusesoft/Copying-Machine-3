// OCRLearningCenterDlg.h : header file
//

#pragma once
#include "TError.h"
#include "TDataLayer.h"
#include "TNeuralNetwork.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "shlobj.h"
#include <string>


struct cOutputResult {

    double fOutput;
	std::wstring sText;
    };

struct cShapeNetExamples {

	vector<std::wstring> sFiles;
    };

// COCRLearningCenterDlg dialog
class COCRLearningCenterDlg : public CDialog
{
// Construction
public:
	COCRLearningCenterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_OCRLEARNINGCENTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	static void DoEvents();


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:

	void OnLoad();
	void OnExit();
	bool GetFolder(CString& folderpath, 
               const wchar_t* szCaption = NULL, 
               HWND hOwner = NULL);
	void MakeExampleList(cShapeNetExamples &oBitmapFiles, std::wstring sBaseFolder);
	void LearnAndTest(bool pbLearn = false);


    GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	TDataLayer* oDataLayer;
	TNeuralNetwork* oShapeNeuralNetwork;
public:
	CListBox oNeuralNetworks;
	CEdit oNeuralNetworkName;
	CEdit oNeuralNetworkExamplesPerFolder;
	CButton oNeuralNetworkCheckSubFolders;
	afx_msg void OnNeuralNetworkSelect();
	CListCtrl oShapesListview;
	CEdit oShapeName;
	CEdit oShapeExampleFolder;
	afx_msg void OnBrowseShapeFolder();
	afx_msg void OnAddShape();
	CEdit oNeuralNodesLayer1;
	CEdit oNeuralNodesLayer2;
	CEdit oSampleEdit;
	afx_msg void OnBrowseSample();
	afx_msg void OnRunLearnShapes();
	afx_msg void OnAnalysis();
	CEdit oStatus;
	afx_msg void OnMoveSamples();
	afx_msg void OnSave();
	afx_msg void OnSelectShape(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTestSample();
	CListBox oResultListbox;
	CEdit oRepetitions;
	};
