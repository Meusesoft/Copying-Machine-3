#pragma once

struct sVector {
	float fX;
	float fY;
	};

struct sMathRect {
	
	float left;
	float right;
	float top;
	float bottom;
	};

class CMathVector {

public:

	CMathVector();
	~CMathVector(void);

	static void RotateVector(float pdAngle, float &fX, float &fY);
	static void MirrorVector(DWORD pdOperation, float &fX, float &fY);
	
	static void RotateRect(Rect& pcRect,  RectF pcRotateWithinRect, DWORD pdAngle);
	static void RotateRect(RectF& pcRect, RectF pcRotateWithinRect, DWORD pdAngle);
	static void RotateRect(sMathRect& pcRect, RectF pcRotateWithinRect, DWORD pdAngle);
	static void MirrorRect(Rect& pcRect,  RectF pcMirrorWithinRect, DWORD pdFlip); 
	static void MirrorRect(RectF& pcRect, RectF pcMirrorWithinRect, DWORD pdFlip); 
	static void MirrorRect(sMathRect& pcRect,  RectF pcMirrorWithinRect, DWORD pdFlip); 
	
private:

	static void MultiplyMatrixVector(float fMatrix[2][2], sVector& pcVector);

	static void ConvertRectToMathRect(Rect pcRect, sMathRect &pcMathRect);
	static void ConvertRectFToMathRect(RectF pcRect, sMathRect &pcMathRect);
	static void ConvertMathRectToRect(sMathRect pcMathRect, Rect &pcRect);
	static void ConvertMathRectToRectF(sMathRect pcMathRect, RectF &pcRect);
	static void NormalizeMathRect(sMathRect &pcMathRect);
};