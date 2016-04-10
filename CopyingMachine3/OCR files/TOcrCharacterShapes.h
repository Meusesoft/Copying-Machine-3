//---------------------------------------------------------------------------

#ifndef TOcrCharacterShapesH
#define TOcrCharacterShapesH

#include "tinyxml\tinyxml.h"
#include "TOcrRectangle.h"

//---------------------------------------------------------------------------
struct cOcrShapesGroup {

    vector <long> oIndexes;
    };
//---------------------------------------------------------------------------
struct cOcrCharacterShape {

    char cCharacter[2];
    vector<TOcrPoint> oShapePoints;

    long iMinX, iMinY, iMaxX, iMaxY;

    long lHoles;
    bool bTopLine;
    bool bBottomLine;
    bool bBaseLine;
    bool bMiddleLine;
    bool bCenterLine;

    BYTE bCompareGrid[1024];
    long lRatio;
    };
//---------------------------------------------------------------------------
class TShapeTreeItem {

    public:

        TShapeTreeItem();
        ~TShapeTreeItem();

        long   lX;
        long   lY;
        long   lMaxTreeScore;

        vector <long> lShapeId;

        TShapeTreeItem* oShapeBlack;
        TShapeTreeItem* oShapeWhite;
        TShapeTreeItem* oShapeUndetermined;
    };

//---------------------------------------------------------------------------
class TOcrCharacterShapes {

    public:

        TOcrCharacterShapes();
        ~TOcrCharacterShapes();

        std::vector<cOcrCharacterShape> oShapes;
        std::vector<cOcrShapesGroup> oShapeGroups;

        bool LoadData(string cFileName);
        long GetGroupGapSize();

        cOcrCharacterShape oCharacterShape;
        TShapeTreeItem* oShapeTreeItems;

    private:

        void __fastcall LoadShapeTreeItems(TShapeTreeItem* poShapeTreeItem, TiXmlNode* poParentNode);

        long lGroupGapSize;
        TError* oError;
    };


#endif
