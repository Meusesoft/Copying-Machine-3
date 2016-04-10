//---------------------------------------------------------------------------

#ifndef TOcrDistancePointsH
#define TOcrDistancePointsH
//---------------------------------------------------------------------------
struct cDistancePoint {

    //long lIndex;
    int  iX;
    int  iY;

    bool bShapePoint;
    int iDistance;
    };

class TOcrDistancePoints {

    public:
        TOcrDistancePoints();
        ~TOcrDistancePoints();

        int GetNextMatchPoint(int iIndex);
        int GetPreviousMatchPoint(int iIndex);

        int GetNextShapePoint(int iIndex);
        int GetPreviousShapePoint(int iIndex);

        void AddPoint(cDistancePoint& poDistancePoint);

        vector <cDistancePoint> oDistancePoints;

    };
#endif
 