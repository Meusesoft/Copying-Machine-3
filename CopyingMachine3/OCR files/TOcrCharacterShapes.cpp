//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TOcrCharacterShapes.h"
#include "TError.h"

//---------------------------------------------------------------------------
TShapeTreeItem::TShapeTreeItem() {


    lX = -1;
    lY = -1;
    lMaxTreeScore = 0;

    oShapeBlack = NULL;
    oShapeWhite = NULL;
    oShapeUndetermined = NULL;
    }
//---------------------------------------------------------------------------
TShapeTreeItem::~TShapeTreeItem() {

    if (oShapeBlack!=NULL) {
        delete oShapeBlack;
        }
    if (oShapeWhite!=NULL) {
        delete oShapeWhite;
        }
    if (oShapeUndetermined!=NULL) {
        delete oShapeUndetermined;
        }
    }
//---------------------------------------------------------------------------
TOcrCharacterShapes::TOcrCharacterShapes() {

    oError = TError::GetInstance();

    try {
        oShapeTreeItems = new TShapeTreeItem();

        MakeSure(oShapeTreeItems!=NULL, ERR_CREATEOBJECT, LEVEL_WARNING, L"oShapeTreeItems");
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    }
//---------------------------------------------------------------------------
TOcrCharacterShapes::~TOcrCharacterShapes() {

    //verwijder huidige data uit geheugen
    for (unsigned int i=0; i<oShapes.size(); i++) {
        oShapes[i].oShapePoints.clear();
        }
    oShapes.clear();

    try {
        MakeSure(oShapeTreeItems!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"oShapeTreeItems==NULL");

        delete oShapeTreeItems;
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    }
//---------------------------------------------------------------------------
long
TOcrCharacterShapes::GetGroupGapSize() {

    return lGroupGapSize;
    }
//---------------------------------------------------------------------------
bool
TOcrCharacterShapes::LoadData(string cFileName) {

    TOcrPoint cPoint;
    cOcrShapesGroup oShapeGroup;

    int iCharNumber;
    char cCharBuffer[10];

    TiXmlElement* GroupsElement = 0;
    TiXmlElement* GroupItemElement = 0;
    TiXmlElement* CharactersElement = 0;
    TiXmlElement* CharacterElement = 0;
    TiXmlElement* ShapeElement = 0;
    TiXmlElement* PointElement = 0;

    TiXmlNode* DocumentNode;
    TiXmlNode* GroupsNode;
    TiXmlNode* GroupNode;
//    TiXmlNode* CharacterNode;
    TiXmlNode* CharactersNode;
//    TiXmlNode* VectorNode;
    TiXmlNode* oShapeTreeItemNode;

    TiXmlDocument* doc;

    string sCompareGrid;

    //verwijder huidige data uit geheugen
    for (unsigned int i=0; i<oShapes.size(); i++) {
        oShapes[i].oShapePoints.clear();
        }
    oShapes.clear();

    //laadt data
    try {

        doc = new TiXmlDocument(cFileName.c_str());
        MakeSure(doc!=NULL, ERR_CREATEOBJECT, LEVEL_WARNING, L"doc==NULL");

        doc->LoadFile();

        DocumentNode = doc->FirstChild("OcrCollection");
        MakeSure(DocumentNode!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"DocumentNode==NULL");

        CharactersNode = DocumentNode->FirstChild("Characters");

        while (CharactersNode!=NULL) {

            CharacterElement = CharactersNode->FirstChildElement("Character");

            while (CharacterElement !=NULL) {

                oCharacterShape.oShapePoints.clear();
                strcpy_s(cCharBuffer, 10, CharacterElement->Attribute("value"));

                iCharNumber = atoi(cCharBuffer);
                oCharacterShape.cCharacter[0] = (char)iCharNumber;
                oCharacterShape.cCharacter[1] = 0x00;

                oCharacterShape.iMinX = atoi(CharacterElement->Attribute("iMinX"));
                oCharacterShape.iMinY = atoi(CharacterElement->Attribute("iMinY"));
                oCharacterShape.iMaxX = atoi(CharacterElement->Attribute("iMaxX"));
                oCharacterShape.iMaxY = atoi(CharacterElement->Attribute("iMaxY"));

                ShapeElement = CharacterElement->FirstChildElement("Shape");

                if (ShapeElement!=NULL) {

                    PointElement = ShapeElement->FirstChildElement("Point");

                    while (PointElement!=NULL) {

                        cPoint.x = atoi(PointElement->Attribute("x"));
                        cPoint.y = atoi(PointElement->Attribute("y"));

                        oCharacterShape.oShapePoints.push_back(cPoint);

                        PointElement = PointElement->NextSiblingElement();

                        }
                    }

                oCharacterShape.lHoles = atoi(CharacterElement->Attribute("iHoles"));
                oCharacterShape.lRatio = atoi(CharacterElement->Attribute("lCompareRatio"));
                oCharacterShape.bTopLine = (atoi(CharacterElement->Attribute("bTopLine"))==0 ? false : true);
                oCharacterShape.bMiddleLine = (atoi(CharacterElement->Attribute("bMiddleLine"))==0 ? false : true);;
                oCharacterShape.bBaseLine = (atoi(CharacterElement->Attribute("bBaseLine"))==0 ? false : true);;
                oCharacterShape.bBottomLine = (atoi(CharacterElement->Attribute("bBottomLine"))==0 ? false : true);;
                oCharacterShape.bCenterLine = (atoi(CharacterElement->Attribute("bCenterLine"))==0 ? false : true);;

                sCompareGrid = CharacterElement->Attribute("bCompareGrid");

                for (int lIndex=0; lIndex<1024; lIndex++) {

                    oCharacterShape.bCompareGrid[lIndex] = (BYTE)(atoi(&sCompareGrid[lIndex+1]));
                    }

                oShapes.push_back(oCharacterShape);

                CharacterElement = CharacterElement->NextSiblingElement();
                }

            CharactersNode = CharactersNode->NextSiblingElement("Characters");
            }

        //Get the groups containing indexes of similar charactershapes. These
        //groups are used to speed up the character recognition process.
        GroupsNode = DocumentNode->FirstChild("Groups");
        GroupsElement = GroupsNode->ToElement();

        lGroupGapSize = atoi(GroupsElement->Attribute("GapSize"));

        oShapeGroups.clear();

        while (GroupsNode!=NULL) {

            GroupNode = GroupsNode->FirstChild("Group");

            while (GroupNode!=NULL) {

                oShapeGroup.oIndexes.clear();

                GroupItemElement = GroupNode->FirstChildElement("GroupItem");

                while (GroupItemElement!=NULL) {

                    oShapeGroup.oIndexes.push_back(atoi(GroupItemElement->Attribute("Id")));

                    GroupItemElement = GroupItemElement->NextSiblingElement();
                    }

                oShapeGroups.push_back(oShapeGroup);

                GroupNode = GroupNode->NextSibling("Group");
                }

            GroupsNode = GroupsNode->NextSibling("Groups");
            }


        //Load the index tree of shapes
        //First remove the possible old tree and create a new empty instance
        delete oShapeTreeItems;
        oShapeTreeItems = new TShapeTreeItem();

        oShapeTreeItemNode = DocumentNode->FirstChild("oShapeTree");

        if (oShapeTreeItemNode!=NULL) {
            LoadShapeTreeItems(oShapeTreeItems, oShapeTreeItemNode);
            }

        delete doc;

        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(ERR_UNKNOWN, LEVEL_WARNING, L"");
        }

    return true;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrCharacterShapes::LoadShapeTreeItems(TShapeTreeItem* poShapeTreeItem, TiXmlNode* poParentNode) {

    TiXmlElement* oThisItem;
    TiXmlNode* oBlackNode;
    TiXmlNode* oWhiteNode;
    TiXmlNode* oUndeterminedNode;
    TiXmlNode* oShapeIds;
    TiXmlNode* oShapeId;
    TiXmlElement* oShapeIdElement;
    TShapeTreeItem* oShapeBlack, *oShapeWhite, *oShapeUndetermined;

    try {

        oThisItem = poParentNode->ToElement();
        MakeSure(oThisItem!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"oThisItem==NULL");

        poShapeTreeItem->lX = atoi(oThisItem->Attribute("lX"));
        poShapeTreeItem->lY = atoi(oThisItem->Attribute("lY"));
        poShapeTreeItem->lMaxTreeScore = atoi(oThisItem->Attribute("lMaxTreeScore"));

        oBlackNode = poParentNode->FirstChild("Black");

        if (oBlackNode!=NULL) {

            oShapeBlack = new TShapeTreeItem();
            MakeSure(oShapeBlack!=NULL, ERR_CREATEOBJECT, LEVEL_WARNING, L"oShapeBlack==NULL");

            poShapeTreeItem->oShapeBlack = oShapeBlack;

            LoadShapeTreeItems(oShapeBlack, oBlackNode);
            }

        oWhiteNode = poParentNode->FirstChild("White");

        if (oWhiteNode!=NULL) {

            oShapeWhite = new TShapeTreeItem();
            MakeSure(oShapeWhite!=NULL, ERR_CREATEOBJECT, LEVEL_WARNING, L"oShapeWhite==NULL");

            poShapeTreeItem->oShapeWhite = oShapeWhite;

            LoadShapeTreeItems(oShapeWhite, oWhiteNode);
            }

        oUndeterminedNode = poParentNode->FirstChild("Undetermined");

        if (oUndeterminedNode!=NULL) {

            oShapeUndetermined = new TShapeTreeItem();
            MakeSure(oShapeUndetermined!=NULL, ERR_CREATEOBJECT, LEVEL_WARNING, L"oShapeUndetermined==NULL");

            poShapeTreeItem->oShapeUndetermined = oShapeUndetermined;

            LoadShapeTreeItems(oShapeUndetermined, oUndeterminedNode);
            }

        oShapeIds = poParentNode->FirstChild("ShapeIds");

        if (oShapeIds!=NULL) {

            oShapeId = oShapeIds->FirstChild("ShapeId");

            while (oShapeId!=NULL) {

                oShapeIdElement = oShapeId->ToElement();

                poShapeTreeItem->lShapeId.push_back(atoi(oShapeIdElement->Attribute("Id")));

                oShapeId = oShapeId->NextSibling("ShapeId");
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(ERR_UNKNOWN, LEVEL_WARNING, L"");
        }
    }

