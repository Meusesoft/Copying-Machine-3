//---------------------------------------------------------------------------

#ifndef TOcrCollectionH
#define TOcrCollectionH

//#include <vector.h>
//---------------------------------------------------------------------------
struct cLongGroup {

    long lStartLong;
    long lEndLong;
    long lSize;
    };

class TOcrCollection {

    public:
        TOcrCollection();
        ~TOcrCollection();

        long Average();
        long Median();
        long lMeanDeviation(long lValue);

        void AddItem(long plItem);
        long lNumberItems();


        cLongGroup GetGroup(long plItem);
        long NumberGroups();

        void SortGroupsBySize();
        void SortGroupsByStart();

    private:
        bool bAverage;
        long lAverage;

        bool bMedian;
        long lMedian;

        vector <long> oItems;
        vector <cLongGroup> oGroups;

       // TError* oError;
    };
#endif
 