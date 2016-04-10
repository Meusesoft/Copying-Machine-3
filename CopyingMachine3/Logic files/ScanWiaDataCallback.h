#pragma once
#include "wia.h"

class CScanWiaDataCallback :
	public IWiaDataCallback
{
public:
	CScanWiaDataCallback(void);
	~CScanWiaDataCallback(void);

	HRESULT _stdcall QueryInterface(const IID&,void**);
	ULONG   _stdcall AddRef();
	ULONG   _stdcall Release();
	HRESULT _stdcall BandedDataCallback(LONG  lMessage, LONG  lStatus, LONG  lPercentComplete, LONG  lOffset, LONG  lLength, LONG  lReserved, LONG  lResLength, BYTE* pbBuffer);

	HGLOBAL hImage;					//Handle of image buffer

	HWND hProgressDlg;				//Progress dialog
	HWND hProgressBar;				//Progress bar on progress form
	void* oScanWia;

private:

	PBYTE m_pBuffer;                // Data buffer
	LONG  m_BufferLength;           // Length of buffer
	ULONG m_cRef;                   // Object reference count.
	LONG  m_MemBlockSize;
	LONG  m_BytesTransfered;
	GUID  m_cFormat;
	LONG  m_lPageCount;
	BOOL  m_bCanceled;
	BOOL  m_bBitmapCreated;

	long lProgress;
};
