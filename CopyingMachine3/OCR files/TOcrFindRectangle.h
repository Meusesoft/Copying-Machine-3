//---------------------------------------------------------------------------

#ifndef TOcrFindRectangleH
#define TOcrFindRectangleH
#define cColorReplacementStep2  0x01
#define cColorReplacementStep1  0x02
//---------------------------------------------------------------------------
class TOcrFindRectangle : public TOcrThread
{            
public:
    TOcrFindRectangle(HGLOBAL, TOcrRectangle*, bool CreateSuspended);

    void __fastcall Execute();

	long lTimeInGrowRectangle;
    long lTimeInCheckRectangle;
    long lNumberInGrowRectangle;
    long lNumberInCheckRectangle;

private:
    HGLOBAL hImageHandle;
    TOcrRectangle* oRectangles;
    TError* oError;

    long iRowLength;
    long iImageHeight;
    long iImageWidth;
    BYTE* lpBits;

    vector<int> oRectangleCache;
    long lNextRectangle;

    void __fastcall AnalyseRectangle(TOcrRectangle* poRectangle, int iRecursionDepth);
    void __fastcall ConnectedComponentGeneration(TOcrRectangle* poRectangle);
    bool __fastcall GenerateConnectedComponent(TOcrRectangle* poRectangle, TOcrRectangle* oCurrentRectangle);
    bool __fastcall GCC_CheckNeighbour(TOcrRectangle* poRectangle, long x, int dx, long y, int dy, vector<long>&, vector<long>&);

    bool __fastcall GrowRectangle(TOcrRectangle* poParent, TOcrRectangle* poChild);
    bool __fastcall CheckInRectangle(TOcrRectangle* poRectangle, long& x, long y);
    void __fastcall CleanupCacheForNextRow(TOcrRectangle* poRectangle, long y);
    long __fastcall FindNextSpace(TOcrRectangle* poRectangle, long x, long y);
    void __fastcall MergeRectangle(TOcrRectangle* poRectangle);

    void __fastcall GenerateBitmap(TOcrRectangle* poRectangle);
};
//---------------------------------------------------------------------------
#endif
