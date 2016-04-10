#include "StdAfx.h"
#include "ScanWiaDataCallback.h"
#include "CopyingMachineCore.h"

CScanWiaDataCallback::CScanWiaDataCallback(void)
{
    m_cRef              = 0;    
    m_BytesTransfered   = 0;
    m_lPageCount = 0;
    m_bCanceled         = FALSE;
    m_bBitmapCreated    = FALSE;

    hProgressBar = NULL;
	hImage = NULL;
}

CScanWiaDataCallback::~CScanWiaDataCallback(void)
{

}

HRESULT
_stdcall CScanWiaDataCallback::QueryInterface(const IID& iid, void** ppv) {

    *ppv = NULL;
    if (iid == IID_IUnknown || iid == IID_IWiaDataCallback)
        *ppv = (IWiaDataCallback*) this;
    else
        return E_NOINTERFACE;
    AddRef();
    return S_OK;
    }

//-----------------------------------------------------------------------------

ULONG
_stdcall CScanWiaDataCallback::AddRef() {

    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
    }

//-----------------------------------------------------------------------------

ULONG
_stdcall CScanWiaDataCallback::Release() {

    ULONG ulRefCount = m_cRef - 1;
    if (InterlockedDecrement((long*) &m_cRef) == 0) {

        delete this;
        return 0;
        }

    return ulRefCount;
    }

//-----------------------------------------------------------------------------

HRESULT
_stdcall CScanWiaDataCallback::BandedDataCallback(LONG  lMessage, LONG  lStatus, LONG  lPercentComplete, LONG  lOffset, LONG  lLength, LONG  lReserved, LONG  lResLength, BYTE* pbBuffer) {

    CScanWia* oParent;
	oParent = (CScanWia*)oScanWia;

	if (oScanWia!=NULL) {

		m_bCanceled = oParent->GetEndThread();
		}

    if (m_bCanceled) {

        if (hImage!=NULL) GlobalFree(hImage);
        hImage = NULL;
        return S_FALSE;
        }

    switch (lMessage) {

        case IT_MSG_DATA_HEADER: {

            //Receiving data header of image

            PWIA_DATA_CALLBACK_HEADER pHeader =
                 (PWIA_DATA_CALLBACK_HEADER)pbBuffer;

            hImage = GlobalAlloc(GMEM_MOVEABLE, pHeader->lBufferSize + sizeof(BITMAPINFOHEADER) + 256 * 4);
            m_BufferLength = pHeader->lBufferSize + sizeof(BITMAPINFOHEADER) + 256 * 4;
            m_BytesTransfered = 0;
            m_cFormat = pHeader->guidFormatID;

            break;
            }

        case IT_MSG_DATA: {

            //Receiving content data of image

            if (hImage != NULL) {

                lProgress = lPercentComplete;
                if (hProgressBar!=NULL) {
                    SendMessage(hProgressBar, PBM_SETPOS, lProgress, 0); 
                    }

                m_pBuffer = (PBYTE)GlobalLock(hImage);
                memcpy(m_pBuffer + lOffset, pbBuffer, lLength);
                m_BytesTransfered += lLength;
                GlobalUnlock(hImage);
                }
            break;
            }


        case IT_MSG_STATUS: {

            //Receiving status messages

            switch (lStatus) {

                case IT_STATUS_TRANSFER_FROM_DEVICE: {
                    break;
                    }

                case IT_STATUS_PROCESSING_DATA: {

                    lProgress = lPercentComplete;
					if (hProgressBar!=NULL) {
						SendMessage(hProgressBar, PBM_SETPOS, lProgress, 0); 
						}
                    break;
                    }

                case IT_STATUS_TRANSFER_TO_CLIENT: {

                    lProgress = lPercentComplete;
					if (hProgressBar!=NULL) {
						SendMessage(hProgressBar, PBM_SETPOS, lProgress, 0); 
						}
                    break;
                    }
                }
            break;
            }
        };

    return S_OK;
    }

//-----------------------------------------------------------------------------

