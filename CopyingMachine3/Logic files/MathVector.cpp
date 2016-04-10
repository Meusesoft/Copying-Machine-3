#include "stdafx.h"
#include "mathvector.h"

#define _USE_MATH_DEFINES
#include <cmath>

CMathVector::CMathVector() {

	}
	
CMathVector::~CMathVector() {

	}

//Rotate the given vector in angle (radial) degrees
void 
CMathVector::RotateVector(float pfAngle, float &fX, float &fY) {

	sVector cVector;
	float   fMatrix[2][2];

	cVector.fX = fX;
	cVector.fY = fY;

	//Rotate the vector. We use the matrix multiplication
	//[ cos a, - sin a ] [ fVectorX ] 
	//[ sin a,   cos a ] [ fVectorY ]
	fMatrix[0][0] = cos(pfAngle);
	fMatrix[0][1] = sin(pfAngle);
	fMatrix[1][0] = -sin(pfAngle);
	fMatrix[1][1] = cos(pfAngle);

	//Perform the matrix multplication
	MultiplyMatrixVector(fMatrix, cVector);

	fX = cVector.fX;
	fY = cVector.fY;
	}

//Mirror the given vection in the X-axis (pdOperation==0) or the Y-axis (pdOperation==1)
void 
CMathVector::MirrorVector(DWORD pdOperation, float &fX, float &fY) {

	sVector cVector;
	float   fMatrix[2][2];

	cVector.fX = fX;
	cVector.fY = fY;

	//Mirror the vector. We use matrix multiplication.
	//[-1 0] or [1  0]
	//[0  1]    [0 -1]

	fMatrix[0][0] = (pdOperation==1 ? -1.0f : 1.0f);
	fMatrix[0][1] = 0;
	fMatrix[1][0] = 0;
	fMatrix[1][1] = (pdOperation==0 ? -1.0f : 1.0f);

	//Perform the matrix multplication
	MultiplyMatrixVector(fMatrix, cVector);

	fX = cVector.fX;
	fY = cVector.fY;
	}

//This function performs the multiplication of the matrix and the vector(2D)
void
CMathVector::MultiplyMatrixVector(float fMatrix[2][2], sVector& pcVector) {

	sVector cResult;

	cResult.fX = fMatrix[0][0] * pcVector.fX + fMatrix[1][0] * pcVector.fY;
	cResult.fY = fMatrix[0][1] * pcVector.fX + fMatrix[1][1] * pcVector.fY;

	pcVector.fX = cResult.fX;
	pcVector.fY = cResult.fY;
	}

//This function rotates the given rectangle with the rotated rectangle
void 
CMathVector::RotateRect(RectF& pcRect, RectF pcRotateWithinRect, DWORD pdAngle) {

	sMathRect cRectangle;

	//Convert the rectangle to a sMathRect
	ConvertRectFToMathRect(pcRect, cRectangle);

	//Rotate the selection
	RotateRect(cRectangle, pcRotateWithinRect, pdAngle);

	//Convert the result back to a RectF
	ConvertMathRectToRectF(cRectangle, pcRect);
	}

//This function rotates the given rectangle with the rotated rectangle
void 
CMathVector::RotateRect(Rect& pcRect, RectF pcRotateWithinRect, DWORD pdAngle) {

	sMathRect cRectangle;

	//Convert the rectangle to a sMathRect
	ConvertRectToMathRect(pcRect, cRectangle);

	//Rotate the selection
	RotateRect(cRectangle, pcRotateWithinRect, pdAngle);

	//Convert the result back to a Rect
	ConvertMathRectToRect(cRectangle, pcRect);
	}

//This function rotates the given rectangle with the rotated rectangle
void 
CMathVector::RotateRect(sMathRect& pcRect, RectF pcRotateWithinRect, DWORD pdAngle) {

	float fAngle;
	float fHeightWithinRectangle;
	float fWidthWithinRectangle;

	//Rotate the selection
	fWidthWithinRectangle  = (float)(pcRotateWithinRect.Width);
	fHeightWithinRectangle = (float)(pcRotateWithinRect.Height);
	fAngle = ((float)pdAngle * (float)M_PI) / 180.0f;

	//We are going to do the rotation around [0,0] instead of the center of the image
	//Adjust the selection to this change
	pcRect.left		-= fWidthWithinRectangle / 2;
	pcRect.right	-= fWidthWithinRectangle / 2;
	pcRect.top		-= fHeightWithinRectangle / 2;
	pcRect.bottom	-= fHeightWithinRectangle / 2;

	//Determine the vector from the center of the plane to the upper-left corner
	//of the rectangle
	CMathVector::RotateVector(fAngle, pcRect.left,  pcRect.top);
	CMathVector::RotateVector(fAngle, pcRect.right, pcRect.bottom);
	CMathVector::RotateVector(fAngle, fWidthWithinRectangle, fHeightWithinRectangle);

	//Adjust the rectangle back to the new dimensions
	pcRect.left		+= abs(fWidthWithinRectangle  / 2);
	pcRect.right	+= abs(fWidthWithinRectangle  / 2);
	pcRect.top		+= abs(fHeightWithinRectangle  / 2);
	pcRect.bottom	+= abs(fHeightWithinRectangle  / 2);

	NormalizeMathRect(pcRect);
	}

//This function mirrors the given rectangle with the rotated rectangle
void 
CMathVector::MirrorRect(RectF &pcRect, RectF pcMirrorWithinRect, DWORD pdFlip) {

	sMathRect cRectangle;

	//Convert the rectangle to a sMathRect
	ConvertRectFToMathRect(pcRect, cRectangle);

	//Rotate the selection
	MirrorRect(cRectangle, pcMirrorWithinRect, pdFlip);

	//Convert the result back to a RectF
	ConvertMathRectToRectF(cRectangle, pcRect);
	}

//This function mirrors the given rectangle with the rotated rectangle
void 
CMathVector::MirrorRect(Rect &pcRect, RectF pcMirrorWithinRect, DWORD pdFlip) {

	sMathRect cRectangle;

	//Convert the rectangle to a sMathRect
	ConvertRectToMathRect(pcRect, cRectangle);

	//Rotate the selection
	MirrorRect(cRectangle, pcMirrorWithinRect, pdFlip);

	//Convert the result back to a RectF
	ConvertMathRectToRect(cRectangle, pcRect);
	}


//This function mirrors the given rectangle with the rotated rectangle
void 
CMathVector::MirrorRect(sMathRect &pcRect, RectF pcMirrorWithinRect, DWORD pdFlip) {

	float fHeightWithinRectangle;
	float fWidthWithinRectangle;

	//Flip the rectangle
	fWidthWithinRectangle  = (float)(pcMirrorWithinRect.Width);
	fHeightWithinRectangle = (float)(pcMirrorWithinRect.Height);

	//We are going to do the mirroring around [0,0] instead of the center of the image
	//Adjust the selection to this change
	pcRect.left		-= (LONG)fWidthWithinRectangle / 2;
	pcRect.right	-= (LONG)fWidthWithinRectangle / 2;
	pcRect.top		-= (LONG)fHeightWithinRectangle / 2;
	pcRect.bottom	-= (LONG)fHeightWithinRectangle / 2;

	//Determine the vector from the center of the plane to the upper-left corner
	//of the selection
	CMathVector::MirrorVector(pdFlip, (float)pcRect.left,  (float)pcRect.top);
	CMathVector::MirrorVector(pdFlip, (float)pcRect.right, (float)pcRect.bottom);

	//Adjust the selection back to the new dimensions
	pcRect.left		+= abs(fWidthWithinRectangle  / 2);
	pcRect.right	+= abs(fWidthWithinRectangle  / 2);
	pcRect.top		+= abs(fHeightWithinRectangle  / 2);
	pcRect.bottom	+= abs(fHeightWithinRectangle  / 2);

	NormalizeMathRect(pcRect);
	}

//Convert a RectF to a sMathRect rectangle
void	
CMathVector::ConvertRectFToMathRect(RectF pcRect, sMathRect &pcMathRect) {

	pcMathRect.left		= (float)pcRect.X;
	pcMathRect.top		= (float)pcRect.Y;
	pcMathRect.right	= (float)(pcRect.X + pcRect.Width);
	pcMathRect.bottom	= (float)(pcRect.Y + pcRect.Height);
	}

//Convert a sMathRect to a RectF rectangle
void	
CMathVector::ConvertMathRectToRectF(sMathRect pcMathRect, RectF &pcRect) {

	pcRect.X		= (REAL)pcMathRect.left;
	pcRect.Y		= (REAL)pcMathRect.top;
	pcRect.Width	= (REAL)(pcMathRect.right - pcMathRect.left);
	pcRect.Height	= (REAL)(pcMathRect.bottom - pcMathRect.top);
	}

//Convert a RectF to a sMathRect rectangle
void	
CMathVector::ConvertRectToMathRect(Rect pcRect, sMathRect &pcMathRect) {

	pcMathRect.left		= (float)pcRect.X;
	pcMathRect.top		= (float)pcRect.Y;
	pcMathRect.right	= (float)(pcRect.X + pcRect.Width);
	pcMathRect.bottom	= (float)(pcRect.Y + pcRect.Height);
	}

//Convert a sMathRect to a RectF rectangle
void	
CMathVector::ConvertMathRectToRect(sMathRect pcMathRect, Rect &pcRect) {

	pcRect.X		= (INT)pcMathRect.left;
	pcRect.Y		= (INT)pcMathRect.top;
	pcRect.Width	= (INT)(pcMathRect.right - pcMathRect.left);
	pcRect.Height	= (INT)(pcMathRect.bottom - pcMathRect.top);
	}

//This function normalizes the rectangle (no negative width / height)
void 
CMathVector::NormalizeMathRect(sMathRect &pcMathRect) {
	
	if (pcMathRect.left > pcMathRect.right) {

		swap(pcMathRect.left, pcMathRect.right);
		}

	if (pcMathRect.top > pcMathRect.bottom) {

		swap(pcMathRect.top, pcMathRect.bottom);
		}

	}



