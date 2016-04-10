//---------------------------------------------------------------------------

#ifndef TOcrSmoothEdgeH
#define TOcrSmoothEdgeH
//---------------------------------------------------------------------------
class TOcrSmoothEdges : public TOcrThread
{            
private:
    HGLOBAL hImageHandle;

    long iRowLength;
    long iImageHeight;
    long iImageWidth;
    BYTE* lpBits;

public:
    TOcrSmoothEdges(HGLOBAL phIntermediateHandle, bool CreateSuspended);

	void __fastcall Execute();
};
//---------------------------------------------------------------------------
#endif
