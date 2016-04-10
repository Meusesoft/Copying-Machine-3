//---------------------------------------------------------------------------

#ifndef TOcrFindEdgeH
#define TOcrFindEdgeH
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TOcrFindEdge : public TOcrThread
{            
    public:
        TOcrFindEdge(HGLOBAL phSourceHandle, HGLOBAL phDestinationHandle, int piTreshold, bool CreateSuspended);

		virtual void __fastcall Execute();

	private:
        HGLOBAL hSourceHandle;
        HGLOBAL hDestinationHandle;
        int iTreshold;
        TError* oError;

        //bool __fastcall CheckPixel(BYTE* lpPixel, int x, int y, int piBitsPerPixel, long piRowLength, int piTreshold, BYTE* lpPallette);
        //bool __fastcall ComparePixels(BYTE* lpPixel, int x, int y, int piBitsPerPixel, long piRowLength, int piAngle, int piTreshold, BYTE* lpPallette);
};
//---------------------------------------------------------------------------
#endif
