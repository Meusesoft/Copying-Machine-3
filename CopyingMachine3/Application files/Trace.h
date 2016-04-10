#pragma once

#include <vector>

//Trace definitions
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)
//#define __WLINE__ WIDEN(__LINE__)



enum eTraceLevel {eMinimum=0, eNormal=1, eAll=2, eExtreme=3};
enum eTraceNodeType {eTraceFunction, eTraceVariable};

class CTraceNode {

public:
	CTraceNode(CTraceNode* poParent);
	~CTraceNode(void);

	void Write(FILE* poFile, bool pbEndNode = false);
	CTraceNode* AddNode(std::wstring psFunction);
	void SetFunction(std::wstring psFunction);
	void SetResult(std::wstring psResult);

	eTraceNodeType eType;
	CTraceNode* oParent;

private:

	std::wstring sClass;
	std::wstring sFunction;
	std::wstring sResult;

	std::vector<CTraceNode*> oChilds;
	};

class CTracer
{
public:
	CTracer(CRegistry* poRegistry);
	~CTracer(void);

	bool bTrace;

	void	InitTracing(HINSTANCE phInstance);

	void	StartTrace(const wchar_t* cFunction, eTraceLevel eLevel= eTraceLevel::eNormal);

	void	Add(const wchar_t* cFunction, eTraceLevel eLevel= eTraceLevel::eNormal);
	void	Add(const wchar_t* cFunction, int iReturn, eTraceLevel eLevel= eTraceLevel::eNormal);
	void	Add(const wchar_t* cFunction, const wchar_t* psReturn, eTraceLevel eLevel= eTraceLevel::eNormal);
	void	Add(const wchar_t* cFunction, std::wstring psReturn, eTraceLevel eLevel= eTraceLevel::eNormal);
	void	Add(const wchar_t* cFunction, bool bReturn, eTraceLevel eLevel= eTraceLevel::eNormal);

	void	EndTrace(const wchar_t* cFunction, eTraceLevel eLevel= eTraceLevel::eNormal);
	void	EndTrace(const wchar_t* cFunction, int iReturn, eTraceLevel eLevel= eTraceLevel::eNormal);
	void	EndTrace(const wchar_t* cFunction, const wchar_t* psReturn, eTraceLevel eLevel= eTraceLevel::eNormal);
	void	EndTrace(const wchar_t* cFunction, std::wstring psReturn, eTraceLevel eLevel= eTraceLevel::eNormal);
	void	EndTrace(const wchar_t* cFunction, bool bReturn, eTraceLevel eLevel= eTraceLevel::eNormal);

private:

	static INT_PTR CALLBACK TraceDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static CRegistry* oDialogRegistry;

	void VersionInfo(HINSTANCE phInstance);
	void AddProductVersion(HINSTANCE phInstance);
	void AddWindowsInfo();
	void MakeTraceFileName();

	std::wstring cTraceFileName;
	CTraceNode* oNode;
	CRegistry* oRegistry;

	bool bAlwaysTrace;
	eTraceLevel cTraceLevel;
	FILE* File;
};
