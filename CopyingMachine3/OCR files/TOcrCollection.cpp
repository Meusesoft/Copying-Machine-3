//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "TOcrCollection.h"
//#include "TError.h"

//-----------------------------------------------------------------------------

TOcrCollection::TOcrCollection() {

    //oError = TError::GetInstance();

    bMedian = false;
    bAverage = false;
    }

//-----------------------------------------------------------------------------

TOcrCollection::~TOcrCollection() {

    oItems.clear();
    }

//-----------------------------------------------------------------------------

long
TOcrCollection::Average() {

    long lSum;

    if (!bAverage) {
        lAverage = 0;

        if (oItems.size() > 0) {

            for (long lPointer=0; lPointer<(long)oItems.size(); lPointer++) {

                lSum += oItems[lPointer];

                }

            lAverage = lSum / oItems.size();

            }

        bAverage = true;
        }

    return lAverage;
    }

//-----------------------------------------------------------------------------

long
TOcrCollection::Median() {

    long lPointer;

    if (!bMedian) {

        lMedian = -1;

        if (oItems.size()>0) {
            lPointer = oItems.size()/2;
            lMedian = oItems[lPointer];
            }

        bMedian = true;
        }

    return lMedian;

    }

//-----------------------------------------------------------------------------
long
TOcrCollection::lMeanDeviation(long lValue) {

    long lTotalSum;
    long lReturnValue;

    lReturnValue = -1;

    if (oItems.size()>0) {

        for (unsigned long lIndex = 0; lIndex<oItems.size(); lIndex++) {

            lTotalSum += abs(lValue - oItems[lIndex]);
            }

        lReturnValue = lTotalSum / oItems.size();
        }

    return lReturnValue;
    }
//-----------------------------------------------------------------------------
long
TOcrCollection::lNumberItems() {

    return oItems.size();

    }

//-----------------------------------------------------------------------------

void
TOcrCollection::AddItem(long plItem) {

    bool bFound;
    long lCounter;

    bFound = false;
    lCounter = 0;

    //find a place for the item in the vector oItems, this vector
    //is sorted.
    while (!bFound && lCounter<(long)oItems.size()) {

        if (oItems[lCounter] > plItem) {
            bFound = true;

            oItems.insert(oItems.begin() + lCounter, plItem);
            }

        lCounter++;
        }

    if (!bFound) {

       // try {
       //     MakeSure(oItems.size() < oItems.max_size(), ERR_OUTOFMEMORY, LEVEL_CRITICAL, "");

            oItems.push_back(plItem);
       /*     }
        catch (const TExceptionContainer& T) {

            ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
            }
        catch (...) {

            ReportError(LEVEL_WARNING, ERR_UNKNOWN, "");
            }   */
        }

    //find a group to which this long belongs to.
    bFound = false;
    lCounter = 0;
    cLongGroup oGroup;
    long lNextCounter;
    bool bFoundNext;

    while (!bFound && lCounter<(long)oGroups.size()) {

        //the item fits perfectly into one of the groups
        if (plItem >= oGroups[lCounter].lStartLong && plItem <= oGroups[lCounter].lEndLong) {

            oGroups[lCounter].lSize++;
            bFound = true;
            }

        //the items doesn't fit into one of the groups, but if we enlarge the group it will
        //fit into it.
        if (plItem == oGroups[lCounter].lStartLong-1 || plItem == oGroups[lCounter].lEndLong+1) {

            oGroups[lCounter].lSize++;
            //enlarge the group, the start of it or the end of it.
            if (plItem == oGroups[lCounter].lStartLong-1) {
                oGroups[lCounter].lStartLong--;
                }
            else {
                oGroups[lCounter].lEndLong++;
                }

            lNextCounter = lCounter+1;
            bFoundNext = false;

            //see if we need to merge to groups into one.
            while (!bFoundNext && lNextCounter<(long)oGroups.size()) {

                if (oGroups[lNextCounter].lStartLong == oGroups[lCounter].lEndLong) {
                    oGroups[lCounter].lEndLong = oGroups[lNextCounter].lEndLong;
                    oGroups[lCounter].lSize += oGroups[lNextCounter].lSize;
                    }

                if (oGroups[lNextCounter].lEndLong == oGroups[lCounter].lStartLong) {
                    oGroups[lCounter].lStartLong = oGroups[lNextCounter].lStartLong;
                    oGroups[lCounter].lSize += oGroups[lNextCounter].lSize;
                    }

                oGroups.erase(oGroups.begin() + lNextCounter);

                lNextCounter++;
                }

            bFound = true;
            }


        lCounter++;
        }


    if (!bFound) {
        //add a new group to the vector oGroups

        oGroup.lSize = 1;
        oGroup.lStartLong = plItem;
        oGroup.lEndLong = plItem;

        oGroups.push_back(oGroup);

        }

    }

//-----------------------------------------------------------------------------

cLongGroup
TOcrCollection::GetGroup(long plItem) {

    return oGroups[plItem];
    }

//-----------------------------------------------------------------------------

long
TOcrCollection::NumberGroups() {

    return oGroups.size();
    }

//-----------------------------------------------------------------------------

void
TOcrCollection::SortGroupsBySize() {

    bool bSwap;
    long lMax;
    cLongGroup oGroup;

    lMax = oGroups.size()-1;
    bSwap = true;

    while (bSwap) {

        bSwap = false;

        for (long lIndex = 0; lIndex<lMax; lIndex++) {

            if (oGroups[lIndex].lSize > oGroups[lIndex+1].lSize) {

                oGroup.lStartLong = oGroups[lIndex].lStartLong;
                oGroup.lEndLong = oGroups[lIndex].lEndLong;
                oGroup.lSize = oGroups[lIndex].lSize;

                oGroups[lIndex].lStartLong = oGroups[lIndex+1].lStartLong;
                oGroups[lIndex].lEndLong = oGroups[lIndex+1].lEndLong;
                oGroups[lIndex].lSize = oGroups[lIndex+1].lSize;

                oGroups[lIndex+1].lStartLong = oGroup.lStartLong;
                oGroups[lIndex+1].lEndLong = oGroup.lEndLong;
                oGroups[lIndex+1].lSize = oGroup.lSize;

                bSwap = true;
                }
            }

        lMax--;
        }
    }

//-----------------------------------------------------------------------------

void
TOcrCollection::SortGroupsByStart() {

    bool bSwap;
    long lMax;
    cLongGroup oGroup;

    lMax = oGroups.size()-1;
    bSwap = true;

    while (bSwap) {

        bSwap = false;

        for (long lIndex = 0; lIndex<lMax; lIndex++) {

            if (oGroups[lIndex].lStartLong > oGroups[lIndex+1].lStartLong) {

                oGroup.lStartLong = oGroups[lIndex].lStartLong;
                oGroup.lEndLong = oGroups[lIndex].lEndLong;
                oGroup.lSize = oGroups[lIndex].lSize;

                oGroups[lIndex].lStartLong = oGroups[lIndex+1].lStartLong;
                oGroups[lIndex].lEndLong = oGroups[lIndex+1].lEndLong;
                oGroups[lIndex].lSize = oGroups[lIndex+1].lSize;

                oGroups[lIndex+1].lStartLong = oGroup.lStartLong;
                oGroups[lIndex+1].lEndLong = oGroup.lEndLong;
                oGroups[lIndex+1].lSize = oGroup.lSize;

                bSwap = true;
                }
            }

        lMax--;
        }
    }
