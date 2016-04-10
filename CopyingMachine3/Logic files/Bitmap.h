#pragma once

enum eSizeMeasure{eCm, eInch, eSizeMeasureUnknown};

class CBitmap {

public:

	CBitmap(Bitmap* poBitmap, HGLOBAL phBitmap);
	//CBitmap(HGLOBAL phBitmap);
	~CBitmap(void);

	static CBitmap* GetFromHandle(HGLOBAL phBitmap);
	static HGLOBAL ConvertToHandle(Bitmap* poBitmap);
	static Bitmap* GetFromClipboard(HWND phWnd);
	static Bitmap* LoadFromResource(LPCTSTR pName, LPCTSTR pType, HMODULE hInst);
	static void	SaveBMP(std::wstring psFilename, Bitmap* poBitmap);
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	eActionResult  CopyToClipboard(HWND phWnd, RectF pcRectangle);
	eActionResult  CopyToClipboard(HWND phWnd);
	Bitmap* GetBitmap();
	HGLOBAL		   ConvertToHandle();

	Bitmap* UpdateThumbnail();
	Bitmap* GetThumbnail();
	void	ClearThumbnail();

	int		GetFrameCount();
	Bitmap*	GetFrame(int piIndex);

	SizeF	GetBitmapPhysicalSize(eSizeMeasure pcMeasure=eSizeMeasureUnknown);
	DWORD	GetBitmapMemorySize();
	static DWORD GetBitmapMemorySize(Bitmap* poBitmap);

	Bitmap*	CreatePartialBitmap(RectF pcRectangle);
	bool	DrawPartialBitmap(Bitmap* poBitmap, PointF pcPoint);
	RectF	DetermineContentRectangleBitmap();

	static bool LoadPCX(std::wstring psFilename, Bitmap* poBitmap);
	static long CalculateRowsize(long plImageWidth, long plBitsPerPixel);
	static eActionResult ConvertGDIPlusToFileResult(int piResult);
	static Bitmap* ConvertBitmap(Bitmap* oSource, PixelFormat pcPixelFormat);
	static eActionResult ConvertContentsBitmap(Bitmap* oSource, Bitmap* oDestination);
	eActionResult FillRectangle(DWORD pdColor, Rect &pcRectangle);

	static bool MultiplyColorMatrix(ColorMatrix* poLeft, ColorMatrix* poRight, ColorMatrix* poResult);
	static ColorMatrix* CreateContrastColorMatrix(int piContrast);
	static ColorMatrix* CreateBrightnessColorMatrix(int piBrightness);
	static ColorMatrix* CreateStandardColorMatrix();

	static int AddToImagelist(Bitmap* bm, HIMAGELIST phImageList, bool pbDisabled=false, bool pbGrayed=false, int piImagelistReplaceIndex=-1);
	int AddToImagelist(HIMAGELIST phImageList );
	static int PixelFormatNumberBits(PixelFormat pcFormat);

	std::wstring GetFlags();

private:

	bool CreateThumbnail();
	bool DrawPartialBitmapLockBitsMethod(Bitmap* poPartialBitmap, PointF pcPoint);
	bool CreatePartialBitmapLockBitsMethod(Bitmap* poPartialBitmap, RectF pcRectangle);

	Bitmap* DetermineContentRectangleBitmapProcessBitmap(Bitmap* poBitmap, float& pfDivide);
	void DetermineContentRectangleBitmapCalculateVariation(Bitmap* poBitmap, int* piVarianceX, int* piVarianceY, Rect pcWorkRectangle);
	void DetermineContentRectangleBitmapProcessData(int* piVariance, int piVarianceLength, int& piStart, int& piEnd, vector<int> &piMinima, std::wstring psFile);
	void DetermineContentRectangleBitmapAnalyzeParts(Bitmap* poWorkBitmap, Rect& pcWorkRectangle, vector<int> &piLocalMinimaX, vector<int> &piLocalMinimaY);

	int  DetermineConstrastTreshold(int* piBrightness, int piTreshold, int piDepth);
	int  DetermineConstrastTresholdFitNormalDistribution(int* piBrightness);
	float DetermineConstrastTresholdFitNormalDistributionFit(int* piBrightness, int piMean, float pfDeviation);
	float CalculateNormalDistribution(int piPosition, int piMean, float pfDeviation);
	int  DetermineConstrastTresholdTriangleMethod(int* piBrightness);
	void CalculateCurveFitting(int* piBrightness);
	int  DetermineConstrastThresholdOtsuMethod(int* piBrightness);

	Bitmap* oBitmap;
	Bitmap* oThumbnail;

	HGLOBAL hMemoryBitmap;
};