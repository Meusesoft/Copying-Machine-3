//---------------------------------------------------------------------------

#ifndef TDataLayerH
#define TDataLayerH

//---------------------------------------------------------------------------
#include "TOcrThread.h"
#include "TNeuralNetwork.h"
//---------------------------------------------------------------------------
struct cShape {

    long lId;
    string sTitle;
    string sFont;
    string sDirectory;
    char cCharacter[2];
    vector <Point> oPoints;

    long lMinX, lMinY, lMaxX, lMaxY;

    long lHoles;
    bool bTopLine;
    bool bBottomLine;
    bool bBaseLine;
    bool bMiddleLine;
    bool bCenterLine;

    BYTE bCompareGrid[32*32];

    long lPixelCountBackX[32];
    long lPixelCountBackY[32];

    long lPixelCountForeX[32];
    long lPixelCountForeY[32];

    long lPixelFeatureBackX[5];
    long lPixelFeatureBackY[5];

    long lPixelFeatureForeX[5];
    long lPixelFeatureForeY[5];

    long lPixelDirectionX[40];
    long lPixelDirectionY[40];

    long lRatio;

    long lAveragePixelDensity;
    long lDeviationPixelDensity;

    long lPointMassX;
    long lPointMassY;
    long lDeviationPointMassX;
    long lDeviationPointMassY;

    TNeuralNetwork* oNeuralNetwork;
    TNeuralNetwork* oNeuralNetworkResult;
    };
//---------------------------------------------------------------------------
struct cRecognitionScore {

    long    lId;
    long    lCurrentScore;

    long    lProjectionScore;
    long    lProjectionPosition;

    long    lDirectionScore;
    long    lDirectionPosition;

    long    lTemplateScore;
    long    lTemplatePosition;

    wstring sTitle;
    float   fScore;
    };
//---------------------------------------------------------------------------
struct cRecognition {

    //The projection feature
    long lPixelProjectionBackX[5];
    long lPixelProjectionBackY[5];

    long lPixelProjectionForeX[5];
    long lPixelProjectionForeY[5];

    //The direction feature
    long lStrokeDirectionX[20];
    long lStrokeDirectionY[20];

    //The 32x32 comparison bitmap
    BYTE* cCompareBitmap;

    //indication of font
    long lFont; 

    //the scores
    vector <cRecognitionScore> oScores;
    };
//---------------------------------------------------------------------------
struct cCreateBitmapSettings {

    bool bGrayValues;
    bool bMakeSkeleton;
    };
//---------------------------------------------------------------------------
struct cDirection {

    long lBitmapPointer;
    long lDirection;
    };
//---------------------------------------------------------------------------
struct cShapeNetList {

	std::wstring sShape;
    std::wstring sSampleFolder;
    long       lShapeId;
    };
//---------------------------------------------------------------------------
struct cShapeNet {

    string sName;
    string sNumberExamples;
    bool       bSearchSubFolders;

    TNeuralNetwork* oShapeNeuralNetwork;
    vector<cShapeNetList> oShapeList;
    };
//---------------------------------------------------------------------------

struct cShapeExample {

    long lId;
	std::wstring sFile;
    };
//---------------------------------------------------------------------------

class TDataLayer : public TOcrThread {

    public:
		TDataLayer(std::string psXMLShapeNetworksFile);
        ~TDataLayer();

		virtual void __fastcall Execute();


		void __fastcall LoadData();
		void __fastcall LoadShapes(string psXMLFile);
        void __fastcall SaveShapes(string psXMLFile);

        vector<cShapeNet> oShapeNetworks;
        vector<cShape> oShapes;

        void __fastcall RecognizeBitmap(cRecognition &oRecognition);
        void __fastcall RecognizeBitmapOnShapeNet(cRecognition &oRecognition);
		void __fastcall ProcessRecognitionScores(cRecognition &oRecognition, sNeuralOutput &oOutput, cShapeNet &oShapeNet);
		BYTE* __fastcall CreateBitmapFromFile(std::wstring sFilename, Bitmap* poCompareBitmap, cCreateBitmapSettings &cSettings);

        //functions for calculating recognition score
        void __fastcall ExtractFeaturesFromBitmap(cRecognition &oRecognition);
        void __fastcall ScoreExtractedFeature(long lFeature, cRecognition &oRecognition);
        void __fastcall OrderRecognitionByVote(cRecognition &oRecognition);
        void __fastcall OrderRecognitionByNeuralScore(cRecognition &oRecognition);
        long __fastcall CalculateScore(long lFeature, cRecognitionScore &oScore, cRecognition &oRecognition);
        long __fastcall CalculateScoreTemplate(long lPatternId, cRecognition &oRecognition);
        long __fastcall CalculateScorePixelProjection(long lPatternId, cRecognition &oRecognition);
        long __fastcall CalculateScoreStrokeDirection(long lPatternId, cRecognition &oRecognition);


        //Thinning algorithm 1
        void  __fastcall CreateSkeleton(BYTE* pcCompareBitmap);
        bool __fastcall CreateSkeletonIsForeground(BYTE* pcCompareBitmap, int x, int y, int iColor);
        bool __fastcall CreateSkeletonTestAlpha1(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation);
        bool __fastcall CreateSkeletonTestAlpha2(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation);
        bool __fastcall CreateSkeletonTestBeta(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation);

        //Thinning algorithm 2
        void  __fastcall CreateSkeleton2(BYTE* pcCompareBitmap);
    /*  bool __fastcall CreateSkeleton2IsForeground(BYTE* pcCompareBitmap, int x, int y, int iColor);
        bool __fastcall CreateSkeleton2TestAlpha1(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation);
        bool __fastcall CreateSkeleton2TestAlpha2(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation);   */
        bool __fastcall CreateSkeleton2IsForeground(BYTE pbNeighbors[][3], int x, int y, int iColor);
        bool __fastcall CreateSkeleton2TestAlpha1(BYTE pbNeighbors[][3], int x, int y, int iColor, int iRotation);
        bool __fastcall CreateSkeleton2TestAlpha2(BYTE pbNeighbors[][3], int x, int y, int iColor, int iRotation);

        //Thinning algorithm 3
        void __fastcall DoThinning(BYTE* pcCompareBitmap, int piForegroundColor, int piBackgroundColor);
        bool __fastcall DoThinningStep2cdTests(BYTE pbNeighbors[][3]);
        bool __fastcall DoThinningStep1cdTests(BYTE pbNeighbors[][3]);
        bool __fastcall DoThinningCheckTransitions(BYTE pbNeighbors[][3]);
        bool __fastcall DoThinningSearchNeighbors(int x, int y, BYTE* pcCompareBitmap, BYTE pbNeighbors[][3], int piForegroundColor);
        int  __fastcall DoThinningGetPixel(int x, int y, BYTE* pcCompareBitmap, int piForegroundColor);

        //Find Stroke algorithm
        void __fastcall FindStrokes(BYTE* cBitmap, long lCharacter);
        void __fastcall InvestigateStroke(long lX, long lY, BYTE* cBitmap, long lCharacter);
        long __fastcall FollowStroke(BYTE* cBitmap, vector<cDirection> &oStroke, long lX, long lY, long lDirection);
        void __fastcall OptimizeStrokes(vector<cDirection> oStroke);
        void __fastcall SplitAndSaveStrokes(vector<long> oStroke, long lCharacter);
        BYTE __fastcall GetPixelBitmap(BYTE* cBitmap, long lX, long lY);

    private:

		char __fastcall EncodeToCharacter(long plValue);
		long __fastcall EncodeFromCharacter(char pcCharacter);


		std::wstring StringToWString(const std::string& s);
		std::string WStringToString(const std::wstring& s);

		std::string sXMLShapeNetworksFile;
        TError* oError;
    };



#endif
