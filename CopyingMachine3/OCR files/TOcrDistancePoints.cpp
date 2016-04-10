//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TOcrDistancePoints.h"

//---------------------------------------------------------------------------


TOcrDistancePoints::TOcrDistancePoints() {

    }

//-----------------------------------------------------------------------------

TOcrDistancePoints::~TOcrDistancePoints() {

    oDistancePoints.clear();
    }

//-----------------------------------------------------------------------------

int
TOcrDistancePoints::GetNextMatchPoint(int iIndex) {

    int iRetValue = -1;
    int iCounter;

    iCounter = iIndex+1;

    while (iCounter < (long)oDistancePoints.size() && iRetValue==-1) {

        if (!oDistancePoints[iCounter].bShapePoint) {

            iRetValue = iCounter;

            }

        iCounter++;
        }


    return iRetValue;
    }

//-----------------------------------------------------------------------------

int
TOcrDistancePoints::GetPreviousMatchPoint(int iIndex) {

    int iRetValue = -1;
    int iCounter;

    iCounter = iIndex-1;

    while (iCounter > 0 && iRetValue==-1) {

        if (!oDistancePoints[iCounter].bShapePoint) {

            iRetValue = iCounter;

            }

        iCounter--;
        }


    return iRetValue;
    }


//-----------------------------------------------------------------------------

int
TOcrDistancePoints::GetNextShapePoint(int iIndex) {

    int iRetValue = -1;
    int iCounter;

    iCounter = iIndex+1;

    while (iCounter < (long)oDistancePoints.size() && iRetValue==-1) {

        if (oDistancePoints[iCounter].bShapePoint) {

            iRetValue = iCounter;

            }

        iCounter++;
        }


    return iRetValue;
    }


//-----------------------------------------------------------------------------

int
TOcrDistancePoints::GetPreviousShapePoint(int iIndex) {

    int iRetValue = -1;
    int iCounter;

    iCounter = iIndex-1;

    while (iCounter > 0 && iRetValue==-1) {

        if (oDistancePoints[iCounter].bShapePoint) {

            iRetValue = iCounter;

            }

        iCounter--;
        }


    return iRetValue;
    }

//-----------------------------------------------------------------------------

void
TOcrDistancePoints::AddPoint(cDistancePoint& poDistancePoint) {

    oDistancePoints.push_back(poDistancePoint);

    }
 