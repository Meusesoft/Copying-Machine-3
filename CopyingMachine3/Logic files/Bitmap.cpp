#include "stdafx.h"
#include "bitmap.h"


//CBitmap::CBitmap(HGLOBAL phBitmap) {
//
//	LPBITMAPINFO pBitmapInfo;
//	void* pBitmapData;
//	
//	pBitmapInfo = (LPBITMAPINFO)GlobalLock(phBitmap);
//	pBitmapData = ((BYTE*)pBitmapInfo->bmiColors + pBitmapInfo->bmiHeader.biClrUsed*sizeof(RGBQUAD));
//
//	oBitmap= new Bitmap(pBitmapInfo, pBitmapData);
//
//	GlobalUnlock(phBitmap);
//
//	hMemoryBitmap = phBitmap;
//	oThumbnail = NULL;
//	}
	
CBitmap::CBitmap(Bitmap* poBitmap, HGLOBAL phBitmap) {

	oBitmap = poBitmap;

	oThumbnail = NULL;
	hMemoryBitmap = phBitmap;
	}

CBitmap::~CBitmap(void) {

	if (oThumbnail!=NULL) delete oThumbnail;
	if (hMemoryBitmap!=NULL) GlobalFree(hMemoryBitmap);

	delete oBitmap;
	}

Bitmap*
CBitmap::GetBitmap() {
 
	return oBitmap;
	}

Bitmap*
CBitmap::GetThumbnail() {

	if (oThumbnail==NULL) {
	
		CreateThumbnail();
		}

	return oThumbnail;
	}

Bitmap* 
CBitmap::UpdateThumbnail() {

	CreateThumbnail();

	return oThumbnail;
	}

//Delete the current thumbnail
void	
CBitmap::ClearThumbnail() {

	delete oThumbnail;
	oThumbnail = NULL;
	}

CBitmap* 
CBitmap::GetFromHandle(HGLOBAL phBitmap) {

	LPBITMAPINFO pBitmapInfo;
	void* pBitmapData;
	Bitmap* oBitmap;
	CBitmap* oResult;

	pBitmapInfo = (LPBITMAPINFO)GlobalLock(phBitmap);
	pBitmapData = ((BYTE*)pBitmapInfo->bmiColors + pBitmapInfo->bmiHeader.biClrUsed*sizeof(RGBQUAD));

	oBitmap= new Bitmap(pBitmapInfo, pBitmapData);

	GlobalUnlock(phBitmap);

	oResult = new CBitmap(oBitmap, phBitmap);

	return oResult;
	}

//This function creates a global handle to a bitmapinfo (DIB) image.
HGLOBAL		   
CBitmap::ConvertToHandle() {

	return ConvertToHandle(oBitmap);
}

HGLOBAL 
CBitmap::ConvertToHandle(Bitmap* poBitmap) {

	BITMAPINFOHEADER	cBitmapInfoHeader;
	BYTE*		lpDib;
	HGLOBAL		hResult;
	Rect		cImageRect;
	
	hResult = NULL;

	//Do a lock bits on the bitmap to gain direct access to the bits of the image
	BitmapData* bitmapDataSource = new BitmapData;
	cImageRect = Rect(0, 0, poBitmap->GetWidth(), poBitmap->GetHeight());
	poBitmap->LockBits(&cImageRect, ImageLockModeRead, poBitmap->GetPixelFormat(), bitmapDataSource); 

	//Fill the bitmapinfo
	long lSizeImage;
	long lSizeRow;
	long lPaletteSize;
	Status cStatus;

	SecureZeroMemory(&cBitmapInfoHeader, sizeof(cBitmapInfoHeader));
	cBitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	cBitmapInfoHeader.biWidth = poBitmap->GetWidth();
	cBitmapInfoHeader.biHeight = poBitmap->GetHeight();
	cBitmapInfoHeader.biPlanes = 1;
	cBitmapInfoHeader.biBitCount = GetPixelFormatSize(poBitmap->GetPixelFormat());
	cBitmapInfoHeader.biCompression = BI_RGB;
	cBitmapInfoHeader.biXPelsPerMeter = (long)((poBitmap->GetHorizontalResolution() * 100) / 2.54) ;
	cBitmapInfoHeader.biYPelsPerMeter = (long)((poBitmap->GetVerticalResolution() * 100) / 2.54) ;

	lSizeRow = (long)((((poBitmap->GetWidth() * cBitmapInfoHeader.biBitCount) + 31) & (~31)) / 8);
	lSizeImage =  lSizeRow * poBitmap->GetHeight(); 

	

	//Get the palette of the image
	cBitmapInfoHeader.biSizeImage = lSizeImage;

	lPaletteSize = poBitmap->GetPaletteSize();
    ColorPalette* oPalette = (ColorPalette*)malloc(lPaletteSize);
	
	cStatus = poBitmap->GetPalette(oPalette, lPaletteSize); 
	if (cStatus==Ok) {

		cBitmapInfoHeader.biClrUsed = oPalette->Count;
		}
	lPaletteSize = cBitmapInfoHeader.biClrUsed * sizeof(RGBQUAD);

	//Create a memory object
	hResult = GlobalAlloc(GMEM_MOVEABLE, lSizeImage + cBitmapInfoHeader.biSize + lPaletteSize);
	lpDib = (BYTE*)GlobalLock(hResult);

	//add the bitmapinfoheader
	memcpy_s(lpDib, cBitmapInfoHeader.biSize, &cBitmapInfoHeader, cBitmapInfoHeader.biSize);
	
	//add the palette
	long lIndex = oPalette->Count;
	long lCounter;
	RGBQUAD cRGBColor;

	BYTE* lpPalette;

	lpPalette = lpDib + sizeof(cBitmapInfoHeader);
	lCounter = 0;

	while (lIndex>0) {

		lIndex--;

		cRGBColor.rgbReserved	= 0;
		cRGBColor.rgbBlue		= (oPalette->Entries[lCounter] >> BLUE_SHIFT) & 255;
		cRGBColor.rgbGreen		= (oPalette->Entries[lCounter] >> GREEN_SHIFT) & 255;
		cRGBColor.rgbRed		= (oPalette->Entries[lCounter] >> RED_SHIFT) & 255;
	
		memcpy_s(lpPalette, sizeof(RGBQUAD), &cRGBColor, sizeof(RGBQUAD));
		
		lpPalette += sizeof(RGBQUAD);
		lCounter++;
		}
	
	//add the bits
	BYTE* lpBitsDestination;
	BYTE* lpBitsSource;

	lIndex = poBitmap->GetHeight();

	lpBitsDestination = lpPalette + (lIndex * lSizeRow);
	lpBitsSource = (BYTE*)bitmapDataSource->Scan0;

	while (lIndex>0) {

		lIndex--;
		lpBitsDestination -= lSizeRow;

		memcpy_s(lpBitsDestination, abs(bitmapDataSource->Stride), 
					lpBitsSource, abs(bitmapDataSource->Stride));

		lpBitsSource += bitmapDataSource->Stride;
		}

	//unlock and free memory handles
	GlobalUnlock(hResult);				
	free(oPalette);
	poBitmap->UnlockBits(bitmapDataSource);
	delete bitmapDataSource;

	//return the result
	return hResult;
	}

int 
CBitmap::PixelFormatNumberBits(PixelFormat pcFormat) {

	int iResult;

	iResult = (pcFormat >> 8) & 255;  

	return iResult;
}


//HGLOBAL 
//CBitmap::ConvertToHandle(Bitmap* poBitmap) {
//
//	HBITMAP hBitmap;
//	DIBSECTION cDibSection;
//	BYTE*      lpDib;
//	HGLOBAL    hResult;
//
//	hResult = NULL;
//
//	if (poBitmap->GetHBITMAP(Color(255,0,0,0), &hBitmap) == Ok) {
//
//		//Convert the HGDIOBJECT to a DIB.
//		GetObject(hBitmap, sizeof(DIBSECTION), &cDibSection);
//
//		cDibSection.dsBmih.biXPelsPerMeter = (long)(poBitmap->GetHorizontalResolution() * 100) / 2.54 ;
//		cDibSection.dsBmih.biYPelsPerMeter = (long)(poBitmap->GetVerticalResolution() * 100) / 2.54;
//
//		hResult = GlobalAlloc(GMEM_MOVEABLE, cDibSection.dsBmih.biSize + cDibSection.dsBmih.biSizeImage);
//		lpDib = (BYTE*)GlobalLock(hResult);
//
//		memcpy_s(lpDib, cDibSection.dsBmih.biSize, &cDibSection.dsBmih, cDibSection.dsBmih.biSize);
//		memcpy_s(lpDib + cDibSection.dsBmih.biSize, cDibSection.dsBmih.biSizeImage, cDibSection.dsBm.bmBits, cDibSection.dsBmih.biSizeImage);
//
//		GlobalUnlock(hResult);				
//		}
//
//	return hResult;
//	}


//This function creates a (new) thumbnail image.
bool
CBitmap::CreateThumbnail() {

	bool bResult;
	SolidBrush* oWhiteBrush;
	Color* oColor;
	Pen* oPen;
	Rect cThumbRect;
	Rect cScaledRect;
	double dScaleFactor;
	long lDeltaX;
	long lDeltaY;

	//delete the previous thumbnail image if present
	if (oThumbnail!=NULL) delete oThumbnail;	

	//initialise some variables
	bResult = true;	
	cThumbRect = Rect(0, 0, ThumbnailSizeX, ThumbnailSizeY);
	oThumbnail = new Bitmap(ThumbnailSizeX, ThumbnailSizeY, PixelFormat32bppARGB);

	// Create a brush and a pen
	if (bUseCustomControlView) {
		oColor = new Color(0xFF, defBackgroundR, defBackgroundG, defBackgroundB);
		}
	else {
		oColor = new Color(0xFF, 255, 255, 255);
		}
	oWhiteBrush = new SolidBrush(*oColor);
	oPen = new Pen(oColor->LightGray);

	// Calculate the scaling rectangle.
	dScaleFactor = (double)oBitmap->GetWidth() / (double)oBitmap->GetHeight();

	lDeltaX = lDeltaY = 0; 

	if (dScaleFactor>1) {

		lDeltaY = (long)(ThumbnailSizeY - (ThumbnailSizeY / dScaleFactor)) / 2;
		}
	else {

		lDeltaX = (long)(ThumbnailSizeX - (ThumbnailSizeX * dScaleFactor)) / 2;
		}

	if (oBitmap->GetWidth() < ThumbnailSizeX &&
		oBitmap->GetHeight() < ThumbnailSizeY) {
		
		//the image is smaller than the thumbnail its size. Just center it.

		lDeltaX = (ThumbnailSizeX - oBitmap->GetWidth()) / 2;
		lDeltaY = (ThumbnailSizeY - oBitmap->GetHeight()) / 2;
		}

	cScaledRect = Rect(lDeltaX, lDeltaY, ThumbnailSizeX - lDeltaX * 2, ThumbnailSizeY - lDeltaY * 2);


	// Do the scaling
	Graphics* oCanvas = Graphics::FromImage(oThumbnail);

	//Draw the background
	double dTan = ((double)cThumbRect.Height / (double)cThumbRect.Width);
	double dTanResult = atan((double)dTan);
	double dDegree = dTanResult * 270 / 3.14f;
	LinearGradientBrush* oBackgroundBrush = new LinearGradientBrush( 
							cThumbRect, 
							Color(255, defBackgroundR, defBackgroundG, defBackgroundB), 
							Color(255, (BYTE)(defBackgroundR * 0.6f), (BYTE)(defBackgroundG * 0.6f), (BYTE)(defBackgroundB * 0.6f)), 
							(REAL)dDegree, 
							false); 

	oCanvas->FillRectangle(oBackgroundBrush, cThumbRect);


//	oCanvas->FillRectangle(oWhiteBrush, cThumbRect);
	oCanvas->SetInterpolationMode(InterpolationModeBicubic);
	oCanvas->DrawImage(oBitmap, cScaledRect);

	// Add a border to the bitmap
	oCanvas->DrawRectangle(oPen, Rect(0, 0, ThumbnailSizeX-1, ThumbnailSizeY-1));

	//clean up
	delete oColor;
	delete oWhiteBrush;
	delete oPen;
	delete oCanvas;

	return bResult;
}

//This function returns the frame count of the bitmap
int		
CBitmap::GetFrameCount() {

	GUID* oDimensions;
	int iDimensionCount;
	int iResult;

	iDimensionCount = oBitmap->GetFrameDimensionsCount();
	
	oDimensions = new GUID[iDimensionCount];
	oBitmap->GetFrameDimensionsList(oDimensions, iDimensionCount);

	iResult = oBitmap->GetFrameCount(oDimensions);

	delete[] oDimensions;

	return iResult;
	}

//This function return a bitmap which contains the content of the given frame
Bitmap*	
CBitmap::GetFrame(int piFrame) {

	Bitmap* oFrame;
	GUID* oDimensions;
	Rect  cImageRect;
	int iDimensionCount;

	oFrame = NULL;

	iDimensionCount = oBitmap->GetFrameDimensionsCount();
	oDimensions = new GUID[iDimensionCount];

	oBitmap->GetFrameDimensionsList(oDimensions, iDimensionCount);
	oBitmap->SelectActiveFrame(oDimensions, piFrame);
	
	cImageRect = Rect(0, 0, oBitmap->GetWidth(), oBitmap->GetHeight());
	BitmapData* bitmapDataSource = new BitmapData;
	BitmapData* bitmapDataDestination = new BitmapData;

	oBitmap->LockBits(&cImageRect, ImageLockModeRead, oBitmap->GetPixelFormat(), bitmapDataSource); 

	oFrame = new Bitmap(oBitmap->GetWidth(), oBitmap->GetHeight(), oBitmap->GetPixelFormat());
	oFrame->LockBits(&cImageRect, ImageLockModeWrite, oBitmap->GetPixelFormat(), bitmapDataDestination); 

	BYTE *Source;
	BYTE *Destination;

	Source = (BYTE*)bitmapDataSource->Scan0;
	Destination = (BYTE*)bitmapDataDestination->Scan0;

	for (long lIndex=0; lIndex<cImageRect.Height; lIndex++) {

		memcpy(Destination, Source, abs(bitmapDataSource->Stride));

		Source += bitmapDataSource->Stride;
		Destination += bitmapDataDestination->Stride;
		}

	oBitmap->UnlockBits(bitmapDataSource);
	oFrame->UnlockBits(bitmapDataDestination);

	delete []oDimensions;

	//copy the dpi's
	oFrame->SetResolution(oBitmap->GetHorizontalResolution(), oBitmap->GetVerticalResolution());
	
	return oFrame;
	}

//This function calculates the size of a row in a windows bitmap given the
//image its width and the bits per pixel
long 
CBitmap::CalculateRowsize(long plImageWidth, long plBitsPerPixel) {

	return ((plImageWidth*plBitsPerPixel+31)&(~31))/8;
	}

//This function translates the GDI+ result to a BitmapFileResult
eActionResult 
CBitmap::ConvertGDIPlusToFileResult(int piResult) {

	eActionResult eResult;

	switch (piResult) {

		case Ok: eResult = eOk; break;
		case FileNotFound: eResult = eFileResultNotFound; break;
		case UnknownImageFormat: eResult = eFileResultNotSupported; break;
		case AccessDenied: eResult = eFileResultAccessDenied; break;

		case Win32Error: 

 //   TCHAR szBuf[80]; 
 //   LPVOID lpMsgBuf;
 //   DWORD dw;
	//
	//dw = GetLastError(); 

 //   FormatMessage(
 //       FORMAT_MESSAGE_ALLOCATE_BUFFER | 
 //       FORMAT_MESSAGE_FROM_SYSTEM,
 //       NULL,
 //       dw,
 //       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
 //       (LPTSTR) &lpMsgBuf,
 //       0, NULL );

 //   wsprintf(szBuf, 
 //       L"failed with error %d: %s", 
 //       dw, lpMsgBuf); 
 //
 //   MessageBox(NULL, szBuf, L"Error", MB_OK); 

 //   LocalFree(lpMsgBuf);

	switch (GetLastError()) {

				case ERROR_ACCESS_DENIED: eResult = eFileResultAccessDenied; break;
				case ERROR_PATH_NOT_FOUND:
				case ERROR_FILE_NOT_FOUND: eResult = eFileResultNotFound; break;
				case ERROR_NOT_ENOUGH_MEMORY: eResult = eGeneralOutOfMemory; break;
				case ERROR_WRITE_FAULT: eResult = eFileResultAccessDenied; break;

				default: 
					eResult = eFileResultUnknownError;
					break;
				}

			break;

		default:
			eResult = eFileResultUnknownError;
			break;
		}

	return eResult;
}

//This function multiplies two matrices and stores the result in a result colormatrix.
//poResult can be the same as poLeft and/or poRight.
bool
CBitmap::MultiplyColorMatrix(ColorMatrix* poLeft, ColorMatrix* poRight, ColorMatrix* poResult) {

	bool bResult;
	float fTemp;
	float fResultMatrix[5][5];

	bResult = false;

	if (poLeft!=NULL && poRight!=NULL && poResult!=NULL) {

		
		//Multiply
		for (int iX=0; iX<5; iX++) {

			for (int iY=0; iY<5; iY++) {

				fTemp = 0;

				for (int i=0; i<5; i++) {

					fTemp += poLeft->m[i][iY] * poRight->m[iX][i];
					}

				fResultMatrix[iX][iY] = fTemp;
				}
			}
	
		//Copy the result
		memcpy_s(poResult->m, 25 * sizeof(REAL), fResultMatrix, 25 * sizeof(REAL));


		bResult = true;
		}

	return bResult;
}

//This function creates a color matrix which is set to perform a contrast action. The contast value needs
//to be within the -100 to +100 range.
ColorMatrix*
CBitmap::CreateContrastColorMatrix(int piContrast) {

	float fValue;
	ColorMatrix* poResult;
	
	poResult = new ColorMatrix();
	ZeroMemory(poResult->m, 25 * sizeof(REAL));

	if (piContrast>=-100 && piContrast<=100) {

		fValue = ((float)piContrast + 100.0f) / 100.0f; //add here the contrast function

		for (int i=0; i<5; i++) {

			poResult->m[i][i] = 1.0f;
			}
		
		for (int i=0; i<3; i++) {

			poResult->m[i][i] = fValue;
			poResult->m[i][4] = 0.0001f; //a workaround to prevend overflow
			}
		}

	return poResult;
	}

//This function creates a color matrix which is set to perform a contrast action. The contast value needs
//to be within the -100 to +100 range.
ColorMatrix*
CBitmap::CreateBrightnessColorMatrix(int piBrightness) {

	ColorMatrix* poResult;
	float fValue;

	poResult = new ColorMatrix();
	ZeroMemory(poResult->m, 25 * sizeof(REAL));

	if (piBrightness>=-100 && piBrightness<=100) {

		fValue = (float)piBrightness / 100.0f; //add here the contrast function

		for (int i=0; i<5; i++) {

			poResult->m[i][i] = 1.0f;
			}
		
		for (int i=0; i<3; i++) {

			poResult->m[4][i] = fValue;
			}
		}

	return poResult;
	}


//This function creates the standard color matrix
ColorMatrix* 
CBitmap::CreateStandardColorMatrix() {

	ColorMatrix* oResult;

	oResult = new ColorMatrix();
	ZeroMemory(oResult->m, 25 * sizeof(REAL));

	for (int i=0; i<5; i++) {

		oResult->m[i][i] = 1.0f;
		}

	return oResult;
}

//This function gets the bitmap/DIB from the clipboard and creates
//a GDI++ Bitmap instance for it.
Bitmap* 
CBitmap::GetFromClipboard(HWND phWnd) {

	Bitmap* oResult;
	Bitmap* oClipboardBitmap;
	HBITMAP hBitmap;
	HPALETTE hPalette;
	BYTE*  pBitmapInfoHeaderPointer;
	LPBITMAPINFOHEADER  pBitmapInfoHeader;
	HGLOBAL hDib;
	HGLOBAL hMemoryBlock;
	long	lBitmapInfoSize;
	SIZE_T	lMemoryBlockSize;
	BYTE*   pMemoryBlock;

	bool ConvertTo24bpp = false;

	oResult = NULL;
	oClipboardBitmap = NULL;
	
	//Determine if the clipboard contains a supported format
	if (IsClipboardFormatAvailable(CF_BITMAP) || 
		IsClipboardFormatAvailable(CF_DIB)) {

		if (OpenClipboard(phWnd)) {
				
			//Paste a DIB
			if (IsClipboardFormatAvailable(CF_DIB) && oClipboardBitmap==NULL) {
			
				hDib = GetClipboardData(CF_DIB);
				
				if (hDib!=NULL) {
					pBitmapInfoHeaderPointer = (BYTE*)GlobalLock(hDib);
					pBitmapInfoHeader = (LPBITMAPINFOHEADER)pBitmapInfoHeaderPointer;

					if (pBitmapInfoHeader->biBitCount > 24) ConvertTo24bpp = true;
					
					//Make a copy of the thing in memory
					lMemoryBlockSize = GlobalSize(hDib);
					hMemoryBlock =     GlobalAlloc(GMEM_MOVEABLE, lMemoryBlockSize);
					pMemoryBlock =     (BYTE*)GlobalLock(hMemoryBlock);
					lBitmapInfoSize =  lMemoryBlockSize - pBitmapInfoHeader->biSizeImage;
					memcpy_s(pMemoryBlock, lMemoryBlockSize, pBitmapInfoHeader, lMemoryBlockSize);
				
					//Create a GDI+ bitmap based on the bitmapinfo and bitmap data
					oClipboardBitmap = new Bitmap((BITMAPINFO *)pMemoryBlock, pMemoryBlock+lBitmapInfoSize);

					//If the operation results in a 32 bpp bitmap convert it to 24bpp. This is the
					//format Copying Machine handles.
					if (ConvertTo24bpp) {
						
						oResult = CBitmap::ConvertBitmap(oClipboardBitmap, PixelFormat24bppRGB);
						delete oClipboardBitmap;
						oClipboardBitmap = oResult;
					}
					
					GlobalUnlock(hMemoryBlock);
					}

				GlobalUnlock(hDib);
				}
			
			//Paste a Bitmap
			if (IsClipboardFormatAvailable(CF_BITMAP) && oClipboardBitmap==NULL) {
			
				hBitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
				hPalette = (HPALETTE)GetClipboardData(CF_PALETTE);

				if (hBitmap!=NULL) {
					
					oClipboardBitmap = new Bitmap((HBITMAP)hBitmap, (HPALETTE)hPalette);
					}

				if (oClipboardBitmap!=NULL) {

					oResult = oClipboardBitmap->Clone((REAL)0, (REAL)0,
													  (REAL)oClipboardBitmap->GetWidth(),
													  (REAL)oClipboardBitmap->GetHeight(),
													  oClipboardBitmap->GetPixelFormat());

					delete oClipboardBitmap;
					}
				}

			CloseClipboard();
			}
		}	
				
	if (oClipboardBitmap!=NULL) {

		oResult = oClipboardBitmap->Clone((REAL)0, (REAL)0,
										  (REAL)oClipboardBitmap->GetWidth(),
										  (REAL)oClipboardBitmap->GetHeight(),
										  oClipboardBitmap->GetPixelFormat());

		delete oClipboardBitmap;
		}

	return oResult;
	}

//Copy the complage image to the clipboard
eActionResult  
CBitmap::CopyToClipboard(HWND phWnd) {

	eActionResult cResult;
	RectF		  cRectangle;

	//Get the dimension of the bitmap
	cRectangle.X = 0;
	cRectangle.Y = 0;
	cRectangle.Width = (REAL)GetBitmap()->GetWidth();
	cRectangle.Height = (REAL)GetBitmap()->GetHeight();

	cResult = CopyToClipboard(phWnd, cRectangle);

	return cResult;
	}

//This function creates a partial copy of the current image.
Bitmap*	
CBitmap::CreatePartialBitmap(RectF pcRectangle) {

	Bitmap*	oPartialBitmap;

	//Create a new image which contains the content of the selection rectangle
	oPartialBitmap = new Bitmap((INT)(pcRectangle.Width),
							 (INT)(pcRectangle.Height),
							 GetBitmap()->GetPixelFormat());
	oPartialBitmap->SetResolution(GetBitmap()->GetHorizontalResolution(),
								GetBitmap()->GetVerticalResolution());

	switch (oPartialBitmap->GetPixelFormat()) {
	
		case PixelFormat1bppIndexed:
		case PixelFormat8bppIndexed:

			//Copy the palette
			int iPaletteSize;
			ColorPalette* oPalette;
			
			iPaletteSize = oBitmap->GetPaletteSize();			
			oPalette = (ColorPalette*)malloc(iPaletteSize);

			oBitmap->GetPalette(oPalette, iPaletteSize);

			oPartialBitmap->SetPalette(oPalette);

			free(oPalette);
			
			//Transfer bits and bytes to the partial bitmap through
			//the lockbits method. GDI+ cannot draw images onto
			//images with an indexed pixel format.
			if (!CreatePartialBitmapLockBitsMethod(oPartialBitmap, pcRectangle)) {
				
				delete oPartialBitmap;
				oPartialBitmap = NULL;
				};
			break;

		default:

			//Draw the image to a copy of the image
			Graphics* oCanvas;
			
			oCanvas = new Graphics(oPartialBitmap);
			if (oCanvas->DrawImage(GetBitmap(),
							   (INT)0, (INT)0,
							   (INT)pcRectangle.X,
							   (INT)pcRectangle.Y,
							   (INT)pcRectangle.Width,
							   (INT)pcRectangle.Height,
							   (Gdiplus::Unit)UnitPixel)!=Ok) {
				//DrawImage not succesfull, clean up the bitmap
				delete oPartialBitmap;
				oPartialBitmap = NULL;
				}

			delete oCanvas;
			break;
		}

	return oPartialBitmap;
	}

//This function fills the partial bitmap with the content of
//this bitmap
bool 
CBitmap::CreatePartialBitmapLockBitsMethod(Bitmap* poPartialBitmap, RectF pcRectangle) {

	bool bResult;
	BitmapData* oDestinationBitmapData;
	BitmapData* oSourceBitmapData;
	BYTE* pDestinationPointer;
	BYTE* pSourcePointer;
	Rect  cDestinationRect;
	Rect  cSourceRect;

	bResult = false;

	//Test if the pixelformat is the same, if not we cannot do the
	//transfer
	if (oBitmap->GetPixelFormat()==poPartialBitmap->GetPixelFormat()) {

		//Initialise variables for the lockbits
		oDestinationBitmapData = new BitmapData();
		oSourceBitmapData = new BitmapData();

		cSourceRect = Rect((INT)pcRectangle.X, (INT)pcRectangle.Y, (INT)pcRectangle.Width, (INT)pcRectangle.Height);
		cDestinationRect = Rect(0, 0, cSourceRect.Width, cSourceRect.Height);

		//Lock the bits
		if (oBitmap->LockBits(&cSourceRect, ImageLockModeRead, oBitmap->GetPixelFormat(), oSourceBitmapData)==Ok) {
			if (poPartialBitmap->LockBits(&cDestinationRect, ImageLockModeWrite, poPartialBitmap->GetPixelFormat(), oDestinationBitmapData)==Ok) {

				//Do the transfer of bits and bytes
				switch (oDestinationBitmapData->PixelFormat) {
		
					case PixelFormat1bppIndexed:
					case PixelFormat8bppIndexed:

						//Transfer for grayscale and blackwhite images
						for (long lY = 0; lY<(long)oDestinationBitmapData->Height; lY++) {

							pDestinationPointer = (BYTE*)oDestinationBitmapData->Scan0 + (oDestinationBitmapData->Stride * lY);
							pSourcePointer = (BYTE*)oSourceBitmapData->Scan0 + (oSourceBitmapData->Stride * lY);

							memcpy_s(pDestinationPointer, abs(oDestinationBitmapData->Stride), 
									 pSourcePointer, abs(oDestinationBitmapData->Stride));
							}

						bResult = true;
						break;
					}

				//Unlock
				poPartialBitmap->UnlockBits(oSourceBitmapData);
				}
			
			oBitmap->UnlockBits(oDestinationBitmapData);
			}

		//Clean up
		delete oDestinationBitmapData;
		delete oSourceBitmapData;
		}

	return bResult;
}


//This function draws the partial bitmap over the current bitmap
bool	
CBitmap::DrawPartialBitmap(Bitmap* poPartialBitmap, PointF pcPoint) {

	bool bResult;
	Status cStatus;

	bResult = false;

	switch (oBitmap->GetPixelFormat()) {

		case PixelFormat1bppIndexed:
		case PixelFormat8bppIndexed:

			bResult = DrawPartialBitmapLockBitsMethod(poPartialBitmap, pcPoint);
			break;

		default:

			//Try to draw the partial bitmap over the bitmap
			Graphics* oCanvas;
			
			oCanvas = new Graphics(oBitmap);
			
			cStatus = oCanvas->DrawImage(poPartialBitmap,
							   (INT)pcPoint.X, 
							   (INT)pcPoint.Y,
							   (INT)0,
							   (INT)0,
							   (INT)poPartialBitmap->GetWidth(),
							   (INT)poPartialBitmap->GetHeight(),
							   (Gdiplus::Unit)UnitPixel);
			delete oCanvas;

			bResult = (cStatus == Ok);
			break;
		}

	return bResult;
}

//This function draws the partial bitmap over the current bitmap
bool	
CBitmap::DrawPartialBitmapLockBitsMethod(Bitmap* poPartialBitmap, PointF pcPoint) {

	bool bResult;
	BitmapData* oDestinationBitmapData;
	BitmapData* oSourceBitmapData;
	BYTE* pDestinationPointer;
	BYTE* pSourcePointer;
	Rect  cDestinationRect;
	Rect  cSourceRect;

	bResult = false;

	//Test if the pixelformat is the same, if not we cannot do the
	//drawing
	if (oBitmap->GetPixelFormat()==poPartialBitmap->GetPixelFormat()) {

		//Initialise variables for the lockbits
		oDestinationBitmapData = new BitmapData();
		oSourceBitmapData = new BitmapData();

		cSourceRect = Rect(0, 0, poPartialBitmap->GetWidth(), poPartialBitmap->GetHeight());
		cDestinationRect = Rect((INT)pcPoint.X, (INT)pcPoint.Y, cSourceRect.Width, cSourceRect.Height);

		//Lock the bits
		if (oBitmap->LockBits(&cDestinationRect, ImageLockModeWrite, oBitmap->GetPixelFormat(), oDestinationBitmapData)==Ok) {
			if (poPartialBitmap->LockBits(&cSourceRect, ImageLockModeRead, poPartialBitmap->GetPixelFormat(), oSourceBitmapData)==Ok) {

				//Do the transfer of bits and bytes
				switch (oDestinationBitmapData->PixelFormat) {
		
					case PixelFormat1bppIndexed:
					case PixelFormat8bppIndexed:

						//Transfer for grayscale and blackwhite images
						for (long lY = 0; lY<(long)oDestinationBitmapData->Height; lY++) {

							pDestinationPointer = (BYTE*)oDestinationBitmapData->Scan0 + (oDestinationBitmapData->Stride * lY);
							pSourcePointer = (BYTE*)oSourceBitmapData->Scan0 + (oSourceBitmapData->Stride * lY);

							memcpy_s(pDestinationPointer, abs(oDestinationBitmapData->Stride), 
									 pSourcePointer, abs(oSourceBitmapData->Stride));
							}

						bResult = true;
						break;
					}

				//Unlock
				poPartialBitmap->UnlockBits(oSourceBitmapData);
				}
			
			oBitmap->UnlockBits(oDestinationBitmapData);
			}

		//Clean up
		delete oDestinationBitmapData;
		delete oSourceBitmapData;
		}

	return bResult;
}
	
//Copy the selection or the complage image to the clipboard
eActionResult  
CBitmap::CopyToClipboard(HWND phWnd, RectF pcRectangle) {

	Bitmap*	oCopyBitmap;
	eActionResult cResult;
	HBITMAP hBitmap;
	HGLOBAL hDIB;

	cResult = eImageActionError;

	//Create a new image which contains the content of the selection rectangle
	oCopyBitmap = CreatePartialBitmap(pcRectangle);

	//Copy the bitmap to the clipboard
	if (oCopyBitmap!=NULL) {
	
		if (OpenClipboard(phWnd)) {

			EmptyClipboard();

			if (oCopyBitmap->GetHBITMAP(Color(255,0,0,0), &hBitmap) == Ok) {

				//Convert the HGDIOBJECT to a DIB.
				DIBSECTION cDibSection;
				BYTE*      lpDib;

				GetObject(hBitmap, sizeof(DIBSECTION), &cDibSection);

				cDibSection.dsBmih.biXPelsPerMeter = (long)((oBitmap->GetHorizontalResolution() * 100) / 2.54);
				cDibSection.dsBmih.biYPelsPerMeter = (long)((oBitmap->GetVerticalResolution() * 100) / 2.54);

				hDIB = GlobalAlloc(GMEM_MOVEABLE, cDibSection.dsBmih.biSize + cDibSection.dsBmih.biSizeImage);
				lpDib = (BYTE*)GlobalLock(hDIB);

				memcpy_s(lpDib, cDibSection.dsBmih.biSize, &cDibSection.dsBmih, cDibSection.dsBmih.biSize);
				memcpy_s(lpDib + cDibSection.dsBmih.biSize, cDibSection.dsBmih.biSizeImage, cDibSection.dsBm.bmBits, cDibSection.dsBmih.biSizeImage);

				GlobalUnlock(hDIB);				
		
				//Place the dib on the clipboard
				if (SetClipboardData(CF_DIB, hDIB)!=NULL) cResult = eOk;
				}

			CloseClipboard();
			}

		//clean up
		delete oCopyBitmap;
		}

	return cResult;
	}


//This function returns the size of the bitmap/image in physical units (cm or inch)
SizeF	
CBitmap::GetBitmapPhysicalSize(eSizeMeasure pcMeasure) {

	SizeF cResult;

	cResult.Width = 0.0f;
	cResult.Height = 0.0f;

	if (oBitmap!=NULL) {

		oBitmap->GetPhysicalDimension(&cResult);

		cResult.Width  /= oBitmap->GetHorizontalResolution();
		cResult.Height /= oBitmap->GetVerticalResolution();

		if (pcMeasure==eCm) {

			cResult.Width  *= (REAL)2.54;
			cResult.Height *= (REAL)2.54;
			}
		}

	return cResult;
	}

//This function returns the size of the bitmap/image in bytes.
DWORD	
CBitmap::GetBitmapMemorySize() {

	return GetBitmapMemorySize(oBitmap);
	}

DWORD 
CBitmap::GetBitmapMemorySize(Bitmap* poBitmap) {

	DWORD dResult;

	dResult = 0;

	if (poBitmap!=NULL) {

		dResult = (DWORD)poBitmap->GetHeight();
		dResult = dResult * (DWORD)CBitmap::CalculateRowsize(poBitmap->GetHeight(), GetPixelFormatSize(poBitmap->GetPixelFormat()));
		}

	return dResult;
	}


//This function returns a string with all the flags of the bitmap
std::wstring 
CBitmap::GetFlags() {

	std::wstring sFlags;
	UINT iFlags;

	sFlags = L"";
	iFlags = oBitmap->GetFlags();

	if (iFlags & ImageFlagsScalable) sFlags += L"ImageFlagsScalable ";
	if (iFlags & ImageFlagsHasAlpha) sFlags += L"ImageFlagsHasAlpha ";
	if (iFlags & ImageFlagsHasTranslucent) sFlags += L"ImageFlagsHasTranslucent ";
	if (iFlags & ImageFlagsPartiallyScalable) sFlags += L"ImageFlagsPartiallyScalable ";
	if (iFlags & ImageFlagsColorSpaceRGB) sFlags += L"ImageFlagsColorSpaceRGB ";
	if (iFlags & ImageFlagsColorSpaceCMYK) sFlags += L"ImageFlagsColorSpaceCMYK ";
	if (iFlags & ImageFlagsColorSpaceGRAY) sFlags += L"ImageFlagsColorSpaceGRAY ";
	if (iFlags & ImageFlagsColorSpaceYCBCR) sFlags += L"ImageFlagsColorSpaceYCBCR ";
	if (iFlags & ImageFlagsColorSpaceYCCK) sFlags += L"ImageFlagsColorSpaceYCCK ";
	if (iFlags & ImageFlagsHasRealDPI) sFlags += L"ImageFlagsHasRealDPI ";
	if (iFlags & ImageFlagsHasRealPixelSize) sFlags += L"ImageFlagsHasRealPixelSize ";
	if (iFlags & ImageFlagsReadOnly) sFlags += L"ImageFlagsReadOnly ";
	if (iFlags & ImageFlagsCaching) sFlags += L"ImageFlagsCaching ";

	MessageBox(NULL, sFlags.c_str(), L"Flags", MB_OK);

	return sFlags;
	}

//This function convert the given bitmap to the given pixelformat
Bitmap* 
CBitmap::ConvertBitmap(Bitmap* oSource, PixelFormat pcPixelFormat) {

	Bitmap* oBitmap;

	oBitmap = new Bitmap(oSource->GetWidth(), oSource->GetHeight(),
						 pcPixelFormat);
	oBitmap->SetResolution((REAL)oSource->GetHorizontalResolution(), 
						   (REAL)oSource->GetVerticalResolution());

	//Convert the contents of the source to the new bitmap
	if (ConvertContentsBitmap(oSource, oBitmap)!=eOk) {

		delete oBitmap;
		oBitmap = NULL;
		}

	return oBitmap;
	}

//This function copies the contents of the given bitmap to the
//destination bitmap which has a different pixelformat
eActionResult 
CBitmap::ConvertContentsBitmap(Bitmap* oSource, Bitmap* oDestination) {

	eActionResult cResult;
	Rect cRectangle;
	BitmapData* oSourceBitmapData;
	BitmapData* oDestinationBitmapData;
	int iSize;
	BYTE* pDestinationPointer;
	BYTE* pSourcePointer;
	DWORD   dwSizeColorPalette; 
	ColorPalette *cPalette;

	//Initialize variables
	cResult = eImageActionError;

	cRectangle.Width = oSource->GetWidth();
	cRectangle.Height = oSource->GetHeight();

	//Set the palette
	switch (oDestination->GetPixelFormat()) {

		case PixelFormat8bppIndexed:

			// allocate the new color table 
			dwSizeColorPalette = sizeof(ColorPalette);      // size of core struct 
			dwSizeColorPalette += sizeof(ARGB)*(255);   // the other entries 


			// Allocate some raw space to back our ColorPalette structure pointer 
			cPalette = (ColorPalette *)new BYTE[dwSizeColorPalette]; 
			ZeroMemory(cPalette, dwSizeColorPalette); 

			cPalette->Flags = PaletteFlagsGrayScale;
			cPalette->Count = 256;

			for (long lIndex=0; lIndex<256; lIndex++) {

				cPalette->Entries[lIndex] = Color::MakeARGB(255, (BYTE)lIndex, (BYTE)lIndex, (BYTE)lIndex);
				}

			oDestination->SetPalette(cPalette);

			delete [] (LPBYTE)cPalette;

			break;

		case PixelFormat1bppIndexed:

			// allocate the new color table 
			dwSizeColorPalette = sizeof(ColorPalette);      // size of core struct 
			dwSizeColorPalette += sizeof(ARGB)*(255);   // the other entries 

			// Allocate some raw space to back our ColorPalette structure pointer 
			cPalette = (ColorPalette *)new BYTE[dwSizeColorPalette]; 
			ZeroMemory(cPalette, dwSizeColorPalette); 

			cPalette->Flags = PaletteFlagsGrayScale;
			cPalette->Count = 2;

			for (long lIndex=0; lIndex<2; lIndex++) {

				cPalette->Entries[lIndex] = Color::MakeARGB(255, (BYTE)lIndex*255, (BYTE)lIndex*255, (BYTE)lIndex*255);
				}

			oDestination->SetPalette(cPalette);

			delete [] (LPBYTE)cPalette;

			break;
		}

	//Open buffers to manipulate the images
	oSourceBitmapData = new BitmapData();
	oDestinationBitmapData = new BitmapData();

	if (oSource->LockBits(&cRectangle, ImageLockModeRead, PixelFormat24bppRGB, oSourceBitmapData)==Ok) {

		if (oDestination->LockBits(&cRectangle, ImageLockModeWrite, oDestination->GetPixelFormat(), oDestinationBitmapData)==Ok) {

			switch (oDestinationBitmapData->PixelFormat) {

				case PixelFormat24bppRGB: //Mostly used for upscaling an image to 24 bpp format

					//Do a quick copy of the data
					iSize = oSourceBitmapData->Width * oSourceBitmapData->Height * 3;

					memcpy_s(oDestinationBitmapData->Scan0, iSize, oSourceBitmapData->Scan0, iSize);
					break;

				case PixelFormat8bppIndexed:

					//Downscale to grayscale

					for (long lY = 0; lY<(long)oDestinationBitmapData->Height; lY++) {

						pSourcePointer		= (BYTE*)oSourceBitmapData->Scan0 + (oSourceBitmapData->Stride * lY);
						pDestinationPointer = (BYTE*)oDestinationBitmapData->Scan0 + (oDestinationBitmapData->Stride * lY);
						
						for (long lX = 0; lX<(long)oDestinationBitmapData->Width; lX++) {

							pDestinationPointer[0] = (BYTE)(pSourcePointer[2] * 0.3 + pSourcePointer[1] * 0.59 + pSourcePointer[0] * 0.11);
							pSourcePointer+=3;
							pDestinationPointer++;
							}
						}

					break;

				case PixelFormat1bppIndexed:

					//Downscale to Black and White
					int iCounter;
					BYTE bByte;

					for (long lY = 0; lY<(long)oDestinationBitmapData->Height; lY++) {

						pSourcePointer		= (BYTE*)oSourceBitmapData->Scan0 + (oSourceBitmapData->Stride * lY);
						pDestinationPointer = (BYTE*)oDestinationBitmapData->Scan0 + (oDestinationBitmapData->Stride * lY);
						
						iCounter = 0;
						bByte = 0;

						for (long lX = 0; lX<(long)oDestinationBitmapData->Width; lX++) {

							bByte <<= 1;							
							if ((pSourcePointer[0] * 0.3 + pSourcePointer[0] * 0.59 + pSourcePointer[0] * 0.11)>127) bByte |= 1;
							
							pSourcePointer+=3;
											
							//Write the byte
							iCounter++;
							if (iCounter==8) {
								
								iCounter=0;
								pDestinationPointer[0] = bByte;
								pDestinationPointer++;
								}
							}

						//Write the byte
						if (iCounter>0) {

							bByte <<= (8 - iCounter);
							pDestinationPointer[0] = bByte;
							}
						}

					break;
				}
	
			oSource->UnlockBits(oSourceBitmapData);

			//Assume everything worked
			cResult = eOk;
			}
		
		oDestination->UnlockBits(oDestinationBitmapData);
		}

	return cResult;
}

//This function determines the smallest selection which contains all the
//content of the bitmap.
RectF	
CBitmap::DetermineContentRectangleBitmap() {

	RectF cResult;
	Rect cWorkRectangle;
	float fDivide;
	Bitmap* oWorkBitmap;
	int* iVarianceX;
	int* iVarianceY;
	vector<int> iLocalMinimaX;
	vector<int> iLocalMinimaY;

	//initialize
	cResult.X = 0;
	cResult.Y = 0;
	cResult.Width = (REAL)oBitmap->GetWidth();
	cResult.Height = (REAL)oBitmap->GetHeight();

	//Process the bitmap to a black/white image with a 
	oWorkBitmap = DetermineContentRectangleBitmapProcessBitmap(oBitmap, fDivide);

	//determine the variance per row and per column 
	cWorkRectangle.X = 0;
	cWorkRectangle.Y = 0;
	cWorkRectangle.Width = oWorkBitmap->GetWidth();
	cWorkRectangle.Height = oWorkBitmap->GetHeight();

	iVarianceX = new int[oWorkBitmap->GetWidth()];
	iVarianceY = new int[oWorkBitmap->GetHeight()];

	DetermineContentRectangleBitmapCalculateVariation(oWorkBitmap, iVarianceX, iVarianceY, cWorkRectangle);

	//process the data
	DetermineContentRectangleBitmapProcessData(iVarianceX, oWorkBitmap->GetWidth(),  cWorkRectangle.X, cWorkRectangle.Width,  iLocalMinimaX, L"c:\\TOCRImages\\VarianceX.txt");
	DetermineContentRectangleBitmapProcessData(iVarianceY, oWorkBitmap->GetHeight(), cWorkRectangle.Y, cWorkRectangle.Height, iLocalMinimaY, L"c:\\TOCRImages\\VarianceY.txt");

	//analyze the image parts
	DetermineContentRectangleBitmapAnalyzeParts(oWorkBitmap, cWorkRectangle, iLocalMinimaX, iLocalMinimaY);


	//compute the result
	cResult.X		= cWorkRectangle.X * fDivide;
	cResult.Y		= cWorkRectangle.Y * fDivide;
	cResult.Width	= cWorkRectangle.Width * fDivide;
	cResult.Height	= cWorkRectangle.Height * fDivide;

	//clean up
	delete[] iVarianceX;
	delete[] iVarianceY;
	delete oWorkBitmap;

	//return the result
	return cResult;
	}

Bitmap* 
CBitmap::DetermineContentRectangleBitmapProcessBitmap(Bitmap* poBitmap, float& pfDivide) {

	Bitmap* oTempBitmap;
	ColorMatrix* oColorMatrix;
	int	 iBrightness[256];

	BitmapData* oSourceBitmapData;
	BYTE* pPointer;
	Status cGDIStatus;

	//ImageAttributes instance
	oColorMatrix = CBitmap::CreateStandardColorMatrix();
	ImageAttributes* oImageAttributes;
	oImageAttributes = new ImageAttributes();
	oImageAttributes->SetColorMatrix(oColorMatrix);

	//Resize the image to a workable size (performance wise).
	Rect cImageRect = Rect(0, 0, poBitmap->GetWidth(), poBitmap->GetHeight());
	pfDivide = sqrt((cImageRect.Width * cImageRect.Height) / 150000.0f);
	//fDivide = 4;
	Rect cDestinationRect = Rect(0, 0, (INT)(poBitmap->GetWidth() / pfDivide), (INT)(poBitmap->GetHeight() / pfDivide));
	oTempBitmap = new Bitmap(cDestinationRect.Width, cDestinationRect.Height, PixelFormat24bppRGB);
	Graphics* oGraphics = new Graphics(oTempBitmap);
	oGraphics->DrawImage(poBitmap, cDestinationRect, 0, 0, cImageRect.Width, cImageRect.Height, UnitPixel, oImageAttributes, NULL, NULL);
	delete oGraphics;
	Bitmap* oTemp2Bitmap = ConvertBitmap(oTempBitmap, PixelFormat8bppIndexed);
	delete oTempBitmap;
	oTempBitmap = oTemp2Bitmap;
	delete oColorMatrix;
	delete oImageAttributes;
	
	#ifdef DEBUG
	CBitmap::SaveBMP(L"C:\\TOcrImages\\DetectContent.bmp", oTempBitmap);
	#endif

	for (long lIndex=0; lIndex<256; lIndex++) {
		iBrightness[lIndex]=0;
		}

	oSourceBitmapData = new BitmapData();

	Rect cRectangle;

	cRectangle.X = 0;
	cRectangle.Y = 0;
	cRectangle.Width = oTempBitmap->GetWidth();
	cRectangle.Height = oTempBitmap->GetHeight();

	cGDIStatus = oTempBitmap->LockBits(&cRectangle, ImageLockModeWrite, oTempBitmap->GetPixelFormat(), oSourceBitmapData);
			
	int iTreshold;

	if (cGDIStatus==Ok) {


		for (long lY = 0; lY<(long)oSourceBitmapData->Height; lY++) {

			pPointer = (BYTE*)oSourceBitmapData->Scan0 + (oSourceBitmapData->Stride * lY);
							
			for (long lX = 0; lX<(long)oSourceBitmapData->Width; lX++) {

				iBrightness[pPointer[lX]]++;
				}
			}
		//CalculateCurveFitting(iBrightness);
		//int iTresholdMethod1 = DetermineConstrastTreshold(iBrightness, 128, 0) * 0.9f;
		//int iTresholdMethod2 = DetermineConstrastTresholdFitNormalDistribution(iBrightness);
		//int iTresholdMethod3 = DetermineConstrastTresholdTriangleMethod(iBrightness);
		//int iTresholdMethod4 = DetermineConstrastThresholdOtsuMethod(iBrightness);

		//iTreshold = min(min(min(iTresholdMethod1, iTresholdMethod2), iTresholdMethod3), iTresholdMethod4);
		iTreshold = DetermineConstrastThresholdOtsuMethod(iBrightness);;

		for (long lY = 0; lY<(long)oSourceBitmapData->Height; lY++) {

			pPointer = (BYTE*)oSourceBitmapData->Scan0 + (oSourceBitmapData->Stride * lY);
							
			for (long lX = 0; lX<(long)oSourceBitmapData->Width; lX++) {

				if (pPointer[lX]>=iTreshold) pPointer[lX]=255;
				if (pPointer[lX]<iTreshold) pPointer[lX]=0;
				}
			}

		oTempBitmap->UnlockBits(oSourceBitmapData);

		delete oSourceBitmapData;
		}

	#ifdef DEBUG
	CBitmap::SaveBMP(L"C:\\TOcrImages\\DetectContentTreshold.bmp", oTempBitmap);
	#endif

	//in debug mode write the variances to a text file
	#ifdef DEBUG

		std::ofstream* out;
		char cBuffer[100];

		out = new ofstream(L"c:\\TOCRImages\\Brigthness.txt", ios_base::out);

		if (out!=NULL) {

			for (unsigned int i=0; i<256; i++) {

				sprintf_s(cBuffer, sizeof(cBuffer), "%d\r\n", iBrightness[i]);
				
				out->write(cBuffer, strlen(cBuffer));
			}

			out->close();
			}
	#endif

	return oTempBitmap;
	}

//This function calculates the variation per row and column of the given
//bitmap (8bpp and black/white)
void 
CBitmap::DetermineContentRectangleBitmapCalculateVariation(Bitmap* poBitmap, 
														   int* piVarianceX, 
														   int* piVarianceY,
														   Rect pcWorkRectangle) {

	Rect cRectangle;
	BitmapData* oSourceBitmapData;
	BYTE* pPointer;
	BYTE* pPointerPreviousRow;
	Status cGDIStatus;

	//Initialize the arrays
	for (long lIndex=0; lIndex<(long)(pcWorkRectangle.Width); lIndex++) {
		piVarianceX[lIndex]=0;
		}
	for (long lIndex=0; lIndex<(long)(pcWorkRectangle.Height); lIndex++) {
		piVarianceY[lIndex]=0;
		}

	//Lock the source bitmap and analyze it contents.
	cRectangle.X = pcWorkRectangle.X;
	cRectangle.Y = pcWorkRectangle.Y;
	cRectangle.Width = pcWorkRectangle.Width;
	cRectangle.Height = pcWorkRectangle.Height;

	oSourceBitmapData = new BitmapData();

	cGDIStatus = poBitmap->LockBits(&cRectangle, ImageLockModeWrite, poBitmap->GetPixelFormat(), oSourceBitmapData);
			
	if (cGDIStatus==Ok) {

		for (long lY = 0; lY<(long)pcWorkRectangle.Height; lY++) {

			pPointer = (BYTE*)oSourceBitmapData->Scan0 + (oSourceBitmapData->Stride * lY);
							
			for (long lX = 0; lX<(long)pcWorkRectangle.Width; lX++) {

				if (lX>0) {
					
					if (abs(pPointer[lX] - pPointer[lX-1])>32) piVarianceY[lY]++;
					}
				if (lY>0) {
					pPointerPreviousRow = pPointer - oSourceBitmapData->Stride;
					if (abs(pPointer[lX] - pPointerPreviousRow[lX])>32) piVarianceX[lX]++;
					}
				}
			}

		poBitmap->UnlockBits(oSourceBitmapData);

		delete oSourceBitmapData;
		}
	}

//This function analyzes the image parts to determine which one contain content and so what
//the largest rectangle is that contains content
void 
CBitmap::DetermineContentRectangleBitmapAnalyzeParts(Bitmap* poWorkBitmap, 
													 Rect& pcWorkRectangle, 
													 vector<int> &piLocalMinimaX, 
													 vector<int> &piLocalMinimaY) {

	bool *bPartHasContent;
	int	*iVarianceX;
	int *iVarianceY;
	int iPartWidth, iPartHeight;
	int iTotalX, iTotalY;
	int  iNumberParts;
	int  iIndex;
	Rect cWorkRectangle;

	//allocate and initalize
	iNumberParts = (piLocalMinimaX.size()-1) * (piLocalMinimaY.size()-1);

	bPartHasContent = new bool[iNumberParts];

	for (iIndex=0; iIndex<iNumberParts; iIndex++) {

		bPartHasContent[iIndex] = false;
		}

	//analyze the individual parts
	for (int iPartX = 0; iPartX < (int)piLocalMinimaX.size()-1; iPartX++) {

		for (int iPartY = 0; iPartY < (int)piLocalMinimaY.size()-1; iPartY++) {

			iPartWidth  = piLocalMinimaX[iPartX+1] - piLocalMinimaX[iPartX]; 
			iPartHeight = piLocalMinimaY[iPartY+1] - piLocalMinimaY[iPartY]; 

			iVarianceX = new int[iPartWidth];
			iVarianceY = new int[iPartHeight];

			cWorkRectangle.X		= piLocalMinimaX[iPartX];
			cWorkRectangle.Y		= piLocalMinimaY[iPartY];
			cWorkRectangle.Width	= iPartWidth;
			cWorkRectangle.Height	= iPartHeight;

			DetermineContentRectangleBitmapCalculateVariation(poWorkBitmap, iVarianceX, iVarianceY, cWorkRectangle);

			//determine if there is a lot of variation present in this part
			iTotalX = 0;
			iTotalY = 0;

			iIndex = iPartWidth;
			while (iIndex>0) {

				iIndex--;
				iTotalX += iVarianceX[iIndex];
				}

			iIndex = iPartHeight;
			while (iIndex>0) {

				iIndex--;
				iTotalY += iVarianceY[iIndex];
				}

			if ((iTotalY > iPartHeight / 5) && (iTotalX > iPartWidth / 5)) {

				bPartHasContent[iPartX + iPartY * piLocalMinimaX.size()] = true;
				}

			delete[] iVarianceX;
			delete[] iVarianceY;
			}
		}

	//determine the dimension of the workrectangle based on the
	//analyzed parts
	pcWorkRectangle.X		= piLocalMinimaX[piLocalMinimaX.size()-1];
	pcWorkRectangle.Y		= piLocalMinimaY[piLocalMinimaY.size()-1];
	pcWorkRectangle.Width	= piLocalMinimaX[0];
	pcWorkRectangle.Height	= piLocalMinimaY[0];

	for (int iPartX = 0; iPartX < (int)piLocalMinimaX.size()-1; iPartX++) {

		for (int iPartY = 0; iPartY < (int)piLocalMinimaY.size()-1; iPartY++) {

			if (bPartHasContent[iPartX + iPartY * piLocalMinimaX.size()]) {

				if (pcWorkRectangle.X		> piLocalMinimaX[iPartX])		pcWorkRectangle.X = piLocalMinimaX[iPartX];
				if (pcWorkRectangle.Y		> piLocalMinimaY[iPartY])		pcWorkRectangle.Y = piLocalMinimaY[iPartY];
				if (pcWorkRectangle.Width	< piLocalMinimaX[iPartX+1])		pcWorkRectangle.Width = piLocalMinimaX[iPartX+1];
				if (pcWorkRectangle.Height	< piLocalMinimaY[iPartY+1])		pcWorkRectangle.Height = piLocalMinimaY[iPartY+1];
				}
			}
		}

	pcWorkRectangle.Width	-= pcWorkRectangle.X;
	pcWorkRectangle.Height	-= pcWorkRectangle.Y;
	}



//This function determines the treshold for the front and back
int  
CBitmap::DetermineConstrastTreshold(int* piBrightness, int piTreshold, int piDepth) {

	int iMeanLeft	= 0;
	int iMeanRight	= 0;
	int iMean		= 0;
	int iTreshold	= 0;

	if (piDepth>20 || piTreshold==0) return piTreshold;

	//calculate two sample means
	for (long lIndex=0; lIndex<piTreshold; lIndex++) {

		iMeanLeft += piBrightness[lIndex];
		}
	iMeanLeft = iMeanLeft / piTreshold;

	for (long lIndex=piTreshold; lIndex<256; lIndex++) {

		iMeanRight += piBrightness[lIndex];
		}
	iMeanRight = iMeanRight / (256 - piTreshold);

	//calculate the mean of the two samples
	iMean = (iMeanRight + iMeanLeft) / 2;

	//find the new treshold
	iTreshold = piTreshold;

	if (iMean > iMeanLeft) {
		while (iTreshold<256 && piBrightness[iTreshold]<iMean) {
			iTreshold++;
			}
		}
	if (iMean < iMeanLeft) {
		while (iTreshold<256 && piBrightness[iTreshold]<iMean) {
			iTreshold--;
			}
		}

	//If the new treshold isn't the same do a new iteration
	if (iTreshold != piTreshold) iTreshold = DetermineConstrastTreshold(piBrightness, iTreshold, piDepth+1);

	return iTreshold;	
	}

//This function determines the treshold for the front and back. It uses a method which tries
//to fit a normal distribution function over the global maximum peak. 
int  
CBitmap::DetermineConstrastTresholdFitNormalDistribution(int* piBrightness) {

	int iResult;

	//determine the global maximum in the brightness array. The assumption is that this
	//is the peak which relates to the background colors.
	int iMaximum;
	int iMaximumIndex;

	iMaximum = 0;
	iMaximumIndex = 0;

	for (int iIndex=0; iIndex<256; iIndex++) {
		
		if (iMaximum < piBrightness[iIndex]) {
			iMaximum = piBrightness[iIndex];
			iMaximumIndex = iIndex;
			}
		}

	//continuously change the deviation and find the best fit.
	float fDeviation;
	float fPreviousFit;
	float fNewFit;

	fDeviation = 0.2f;
	fPreviousFit = DetermineConstrastTresholdFitNormalDistributionFit(piBrightness, iMaximumIndex, fDeviation);
	fNewFit = fPreviousFit;

	while (fPreviousFit >= fNewFit && fDeviation<32.0f) {
		
		fDeviation += 1.0f;
		fPreviousFit = fNewFit;

		fNewFit = DetermineConstrastTresholdFitNormalDistributionFit(piBrightness, iMaximumIndex, fDeviation);
		}

	//determine the treshold (= mean -/+ 4*deviation).
	if (iMaximumIndex < 128) {
		
		iResult = (int)min(254, iMaximumIndex + 4 * fDeviation);
		}
	else {

		iResult = (int)max(1, iMaximumIndex - 4 * fDeviation);
		}

	//in debug mode write the variances to a text file
	#ifdef DEBUG

		std::ofstream* out;
		char cBuffer[100];

		out = new ofstream(L"c:\\TOCRImages\\BrigthnessAndNormalDistribution.txt", ios_base::out);

		if (out!=NULL) {

			float fFactor;

			fFactor = CalculateNormalDistribution(iMaximumIndex, iMaximumIndex, fDeviation);
			fFactor = piBrightness[iMaximumIndex] / fFactor;

			for (unsigned int i=0; i<256; i++) {

				sprintf_s(cBuffer, sizeof(cBuffer), "%d;%d\r\n", piBrightness[i], (int)abs(fFactor * CalculateNormalDistribution(i, iMaximumIndex, fDeviation)));
				
				out->write(cBuffer, strlen(cBuffer));
			}

			out->close();
			}

	#endif


	return iResult;	
	}

//this function calculates the fit of the normal distribution function over the brightness
//histogram. The input is the mean and deviation for the normal function.
float  
CBitmap::DetermineConstrastTresholdFitNormalDistributionFit(int* piBrightness, int piMean, float pfDeviation) {

	float fFit;
	float fFactor;

	fFit = 0;
	fFactor = CalculateNormalDistribution(piMean, piMean, pfDeviation);
	fFactor = piBrightness[piMean] / fFactor;

	for (int iIndex=0; iIndex<256; iIndex++) {

		fFit += abs(fFactor * CalculateNormalDistribution(iIndex, piMean, pfDeviation) - piBrightness[iIndex]);
		}

	return fFit;
}


//this function determines the treshold based on the method of maximizing the distance
//between the histogram values and the line between the maximum peak and the lowest value.
int  
CBitmap::DetermineConstrastTresholdTriangleMethod(int* piBrightness) {

	//determine the global maximum in the brightness array. The assumption is that this
	//is the peak which relates to the background colors.
	int iMaximum;
	int iMaximumIndex;

	iMaximum = 0;
	iMaximumIndex = 0;

	for (int iIndex=0; iIndex<256; iIndex++) {
		
		if (iMaximum < piBrightness[iIndex]) {
			iMaximum = piBrightness[iIndex];
			iMaximumIndex = iIndex;
			}
		}

	//determine the first position in the histogram which is larger than 0. This is our 
	//starting point
	int iMinimumIndex;
	int iMinimum;

	if (iMaximumIndex > 128) {
	
		iMinimum = 0;
		iMinimumIndex = 0;

		while (iMinimum==0 && iMinimumIndex<256) {

			iMinimum = piBrightness[iMinimumIndex];
			iMinimumIndex++;
			}
		iMinimumIndex--;
		}
	else {

		iMinimum = 0;
		iMinimumIndex = 255;

		while (iMinimum==0 && iMinimumIndex>=0) {

			iMinimum = piBrightness[iMinimumIndex];
			iMinimumIndex--;
			}
		iMinimumIndex++;
		}
	
	//calculate the vector between the minimum and the maximum.
	float fVector;

	fVector = (float)(iMaximum - iMinimum) / (float)(iMaximumIndex - iMinimumIndex);

	float fDistance;
	float fMaximumDistance;
	int   iMaximumDistanceIndex;
	float fLambda;

	//http://nl.wikipedia.org/wiki/Afstand#Afstand_tussen_een_punt_en_een_lijn

	iMaximumDistanceIndex = iMinimumIndex;
	fMaximumDistance = 0;

	for (int iIndex = iMinimumIndex; iIndex < iMaximumIndex; iIndex++) {

		//calculate the distance
		fLambda = (float)(iMaximumIndex - iMinimumIndex)*(iIndex - iMinimumIndex) ;
		fLambda += (iMaximum - iMinimum)*(piBrightness[iIndex]-iMinimum);
		fLambda /=  (pow((float)(iMaximumIndex - iMinimumIndex), 2) + pow((float)(iMaximum - iMinimum), 2));

		fDistance  = pow((float)(iIndex - iMinimumIndex - fLambda*(iMaximumIndex - iMinimumIndex)), 2);
		fDistance += pow((float)(piBrightness[iIndex] - iMinimum - fLambda*(iMaximum - iMinimum)), 2);
		fDistance = sqrt(fDistance);

		if (fDistance > fMaximumDistance) {

			fMaximumDistance = fDistance;
			iMaximumDistanceIndex = iIndex;
			}
		}

	return iMaximumDistanceIndex;
}

//This function calculates a treshold based on the Otsu method
int  
CBitmap::DetermineConstrastThresholdOtsuMethod(int* piBrightness) {


	double pi[256];
	double N;
	long lIndex;
	double w1, w2, BetweenClassVariance, u1, u2, uTotal;
	double MaxVariance;
	int	   iBestThreshold;
	int		i;

	lIndex = 256;

	//Determine the number of pixels
	N = 0;
	while (lIndex>0) {

		lIndex--;

		N += (double)piBrightness[lIndex];
		}

	//Calculate probability of gray level i in the image
	lIndex = 256;

	while (lIndex>0) {

		lIndex--;

		pi[lIndex] = (double)piBrightness[lIndex] / N;
		}

	//Loop through all the possible thresholds to see which one gives the
	//best threshold according to the otsu algorithm
	lIndex = 255;
	iBestThreshold = 0;
	MaxVariance = 0;

	while (lIndex>1) {

		lIndex--;

		//calculate w1
		w1 = 0;
		for (i=1; i<=lIndex;i++) w1 += pi[i]; 

		//calculate w2
		w2 = 0;
		for (i=lIndex+1; i<=255;i++) w2 += pi[i]; 

		//calculate u1
		u1 = 0;
		for (i=1; i<=lIndex;i++) u1 += ((i * pi[i])/w1); 

		//calculate u2
		u2 = 0;
		for (i=lIndex+1; i<=255;i++) u2 += ((i * pi[i])/w2); 

		//calculate uTotal
		uTotal = w1*u1 + w2*u2;

		//calculate the between variance
		BetweenClassVariance = w1*pow((u1-uTotal),2) + w2*pow((u2-uTotal),2);

		//check if this new variance is larger than a previously calculated variance
		if (BetweenClassVariance > MaxVariance) {

			MaxVariance = BetweenClassVariance;
			iBestThreshold = lIndex;
			}
		}
	
	return iBestThreshold;
	}




//this function calculates the fit of the normal distribution function over the brightness
//histogram. The input is the mean and deviation for the normal function.
float  
CBitmap::CalculateNormalDistribution(int piPosition, int piMean, float pfDeviation) {

	float fResult;

	//=1/($H$2*WORTEL(2*3,14))*EXP(-0,5*((A2-$H$3)/$H$2)^2)

	fResult = 1/(pfDeviation * sqrt(2 * 3.14159265358979323846f)) * exp(-0.5f * pow(((piPosition - piMean)/pfDeviation),2));

	return fResult;
}

//This function calculate the coefficients of an polynomial function. That functions
//describes the histogram. Polynomial Function: f(x) = a0x^0 + a1x^1 + a2x^2 + a3x^3.
//For solving the Gaussian Elimination algorithm is used.
void 
CBitmap::CalculateCurveFitting(int* piBrightness) {

	int iPolynomialDegrees = 5;

	int DataPts = 256;

	double MatrixA[40][40];
	double MatrixB[40][40];
	double SumX[80];
	double SumYX[40];
	double Power[80];
	double Unknown[40];
	double C[40];
	int row;
	int col;
	int i, k, j;

	//Calculate the maximal value
	double fMaximumInput;

	fMaximumInput = 0;

	for (row=0; row<256; row++) {
		
		if (piBrightness[row]>fMaximumInput) fMaximumInput = piBrightness[row];
		}

	//Totalize the histogram
	double* dValue;

	dValue = new double[256];
	double dTotal=0;

	for (row=0; row<256; row++) {

		dTotal += piBrightness[row];
		dValue[row] = dTotal;
		}

	for (row=0; row<256; row++) {

		dValue[row] = dValue[row] / dTotal;
		dValue[row] = piBrightness[row] / fMaximumInput;
		}	

	//Init the matrices
	for(row=0; row<40; row++){

		for(col=0; col<40; col++){

			MatrixA[row][col]=0;
			MatrixB[row][col]=0;
			SumYX[col] = 0;
			C[col] = 0;
			SumX[col+row]=0;
			}
		}

	//Fill the matrices

	//Powers
	for ( i=1; i<=iPolynomialDegrees*2; i++) {

		Power[i] = 0;

		for ( j=0; j<=255; j++) {

			Power[i] = Power[i] + pow(((double)j)/64.0f,(int)i);
			}
		}

	//Sums
	int Temp;
	for ( i=1; i<=iPolynomialDegrees+1; i++) {

		for ( j=1; j<=iPolynomialDegrees+1; j++) {

			Temp = i + j - 2;
			if (Temp>0) {

				MatrixA[i][j] = Power[Temp];
				}
			else 
				{
				MatrixA[1][1] = 256;
				}
			}
		}

	//Constants
	for ( i=1; i<=iPolynomialDegrees+1; i++) {
		
		C[i] = 0;

		for ( j=0; j<256; j++) {

			C[i] = C[i] + dValue[j] * pow(((double)j)/64.0f,(int)(i-1));
			}
		}


	//Solve
	int O1 = iPolynomialDegrees+1;
	int l;
	double T;

	for ( k=1; k<=iPolynomialDegrees; k++) {

		l = k;

		for ( i=k+1; i<=iPolynomialDegrees+1; i++) {

			if ((abs(MatrixA[i][k]) - abs(MatrixA[l][k]))>0) {
				l = i;
				}
			}

		if (l!=k) {

			for ( j=k; j<=iPolynomialDegrees+1; j++) {

				T = MatrixA[l][j];
				MatrixA[l][j] = MatrixA[k][j];
				MatrixA[k][j] = T;
				}
			T = C[k];
			C[k] = C[l];
			C[l] = T;
			}


		for ( i=k+1; i<=iPolynomialDegrees+1; i++) {

			T = MatrixA[i][k] / MatrixA[k][k];
			MatrixA[i][k] = 0;
			for (j=k+1; j<=iPolynomialDegrees+1; j++) {

				MatrixA[i][j] = MatrixA[i][j] - T * MatrixA[k][j];
				}
			C[i] = C[i] - T * C[k];
			}
		}

	//Back solution
//	int i;
	Unknown[iPolynomialDegrees+1] = C[iPolynomialDegrees+1] / MatrixA[iPolynomialDegrees+1][iPolynomialDegrees+1];
	i = iPolynomialDegrees;
	double SumVar;

	while (i>0) {
		
		SumVar = 0;
		for ( j=i+1; j<=iPolynomialDegrees+1; j++) {

			SumVar = SumVar + MatrixA[i][j] * Unknown[j];
			}
		Unknown[i] = (C[i] - SumVar) / MatrixA[i][i];
		i--;
		}

	//in debug mode write the variances to a text file
	#ifdef DEBUG

		std::ofstream* out;
		char cBuffer[100];
		wchar_t cFilename[100];
		double dResult;

		swprintf_s(cFilename, sizeof(cFilename), L"c:\\TOCRImages\\BrigthnessPolynomial_%d.txt",iPolynomialDegrees);

		out = new ofstream(cFilename, ios_base::out);

		if (out!=NULL) {

			for(row=1; row<=iPolynomialDegrees+1; row++){

				sprintf_s(cBuffer, sizeof(cBuffer), "%.6f\r\n",Unknown[row]);
				out->write(cBuffer, strlen(cBuffer));
				}

			for(row=0; row<256; row++){

				dResult = 0;

				for (i=0; i<=iPolynomialDegrees; i++) {

					dResult += Unknown[i+1] * pow(((double)row)/64.0f, i);
					}

				sprintf_s(cBuffer, sizeof(cBuffer), "%.3f;%.3f;%.3f\r\n",((double)row)/256.0f, dValue[row], dResult);
				out->write(cBuffer, strlen(cBuffer));
				}

			out->close();
			}

	#endif

	delete [] dValue;

	}


//This function processes the variance data and returns the interval which probably
//contains the content of the image
void 
CBitmap::DetermineContentRectangleBitmapProcessData(int* piVariance, 
													int piVarianceLength, 
													int& piStart, 
													int& piEnd,
													vector<int> &piMinima, 
													std::wstring psFile) {



	int* iVarianceNoNoise;
	int* iVarianceFlat;
	int* iVarianceSmooth;
	int* iVarianceSquare;

	iVarianceFlat = new int[piVarianceLength];
	iVarianceNoNoise = new int[piVarianceLength];
	iVarianceSmooth = new int[piVarianceLength];
	iVarianceSquare = new int[piVarianceLength];

	//fill our array and remove noise
	for (long lIndex=0; lIndex<piVarianceLength; lIndex++) {
		
		iVarianceNoNoise[lIndex] = piVariance[lIndex];
		iVarianceSmooth[lIndex] = 0;
		iVarianceSquare[lIndex] = 0;
		//if (iVarianceNoNoise[lIndex]<3) iVarianceNoNoise[lIndex]=0;
		}

	//Remove noise from borders
	long lPointer=0;
	while (lPointer<piVarianceLength-1 && iVarianceNoNoise[lPointer]>iVarianceNoNoise[lPointer+1]) {

		iVarianceNoNoise[lPointer] = iVarianceNoNoise[lPointer+1];
		for (long lIndex=lPointer; lIndex>=0; lIndex--) {
			iVarianceNoNoise[lIndex] = iVarianceNoNoise[lPointer+1];
			}

		lPointer++;
		}
	lPointer=piVarianceLength;
	while (lPointer>0 && iVarianceNoNoise[lPointer]>iVarianceNoNoise[lPointer-1]) {

		iVarianceNoNoise[lPointer] = iVarianceNoNoise[lPointer-1];
		for (long lIndex=lPointer; lIndex<piVarianceLength; lIndex++) {
			iVarianceNoNoise[lIndex] = iVarianceNoNoise[lPointer-1];
			}

		lPointer--;
		}

	//Create a smooth graph and a square graph
	for (long lIndex=0; lIndex<piVarianceLength; lIndex++) {

		iVarianceSmooth[lIndex] = (int)(0.2261*iVarianceNoNoise[max(0, lIndex-1)] + 0.5478*iVarianceNoNoise[lIndex] + 0.2261*iVarianceNoNoise[min(piVarianceLength-1, lIndex+1)]);
		iVarianceSquare[lIndex] = (int)(pow((double)iVarianceSmooth[lIndex], 2));
		//dSin = sinf((M_PI * ((double)lIndex) / (double)(piVarianceLength)));
		//dValue = (double)(dSin * (double)iVarianceSmooth[lIndex]);
		//iVarianceSmooth[lIndex] = dValue;
		}


	//Smooth out the variance data.
	int iCurrentLevel;
	iCurrentLevel = 0;
	for (long lIndex=0; lIndex<piVarianceLength; lIndex++) {
		
		if (iVarianceNoNoise[lIndex]>iCurrentLevel) iCurrentLevel = iVarianceNoNoise[lIndex];
		iVarianceFlat[lIndex] = iCurrentLevel;		
		}
	iCurrentLevel = 0;
	for (long lIndex=piVarianceLength-1; lIndex>=0; lIndex--) {
		
		if (iVarianceNoNoise[lIndex]>iCurrentLevel) iCurrentLevel = iVarianceNoNoise[lIndex];
		if (iVarianceFlat[lIndex]>iCurrentLevel) iVarianceFlat[lIndex] = iCurrentLevel;		
		}

	delete[] iVarianceNoNoise;


	//in debug mode write the variances to a text file
	#ifdef DEBUG

		std::ofstream* out;
		char cBuffer[100];

		out = new ofstream(psFile.c_str(), ios_base::out);

		if (out!=NULL) {

			for (unsigned int i=0; i<(unsigned int)piVarianceLength; i++) {

				sprintf_s(cBuffer, sizeof(cBuffer), "%d;%d\r\n", piVariance[i], iVarianceSquare[i]);
				
				out->write(cBuffer, strlen(cBuffer));
			}

			out->close();
			}

	#endif

	//Determine the global maximum
	int		iVarianceTotal		= 0;
	int     iVarianceMax		= 0;
	int     iVarianceMin		= 0;
	int     iVarianceMaxIndex	= 0;
	int		iVarianceMinIndex	= 0;
	long	lIndex = piVarianceLength;
	vector <int> iMinimaRightOfGlobalMax;
	vector <int> iMinimaLeftOfGlobalMax;
	int* iVariance;
	int* iVarianceSum;

	iVarianceSum = new int[piVarianceLength];

	iVariance = iVarianceSquare;
	lIndex = piVarianceLength;

	for (lIndex=0; lIndex<piVarianceLength; lIndex++) {

		iVarianceTotal		+= iVariance[lIndex];

		iVarianceSum[lIndex] = iVarianceTotal;

		if (iVariance[lIndex] > iVarianceMax) {
			iVarianceMax = iVariance[lIndex];
			iVarianceMaxIndex = lIndex;
			}
		}

	//Find the most outer local minima... these are the edges of the content
	lIndex = iVarianceMaxIndex;
	iVarianceMin = iVariance[lIndex];
	bool bToMinimum = true;
	iVarianceMinIndex = iVarianceMaxIndex;

	if (lIndex==0) lIndex++;

	while (lIndex < piVarianceLength) {

		if (iVariance[lIndex] < iVarianceMin) {
			
			iMinimaRightOfGlobalMax.clear();
			iVarianceMin = iVariance[lIndex];
			/*if (lVarianceX[lIndex]<iVarianceXAverage)*/ iVarianceMinIndex = lIndex;
			iMinimaRightOfGlobalMax.push_back(lIndex);
			bToMinimum = false;
			}
		//if (lVarianceX[lIndex]>iVarianceXAverage) iVarianceXMin = iVarianceXMax; //search for a new local minimum

		//found another local minimum with the same value as before
		//can be an candidate
		if (iVariance[lIndex] == iVarianceMin && bToMinimum) {

			bToMinimum = false;
			iMinimaRightOfGlobalMax.push_back(lIndex);
			}

		//we moved away from out local minimum. Look out for another
		if (iVariance[lIndex] > iVarianceMin) {

			bToMinimum = true;
			}

		lIndex++;
		}

	piEnd = iVarianceMinIndex;
	if (iMinimaRightOfGlobalMax.size()>0) piEnd = iMinimaRightOfGlobalMax[iMinimaRightOfGlobalMax.size()-1];
		
	iVarianceMin = iVarianceMax;
	iVarianceMinIndex = iVarianceMaxIndex;
	lIndex = iVarianceMaxIndex;
	
	while (lIndex > 0) {


		//if (lDeltaVarianceX[lIndex]<=0 && lDeltaVarianceX[lIndex-1]>=0) {

		//	iMinimaLeftOfGlobalMax.push_back(lIndex);
		//	}

		lIndex--;

		//if (lVarianceX[lIndex] < iVarianceXMin) {
		//	
		//	iVarianceXMin = lVarianceX[lIndex];
		//	if (lVarianceX[lIndex]<iVarianceXAverage) iVarianceXMinIndex = lIndex;
		//	}
		//if (lVarianceX[lIndex]>iVarianceXAverage) iVarianceXMin = iVarianceXMax; //search for a new local minimum
		if (iVariance[lIndex] < iVarianceMin) {
			
			iMinimaLeftOfGlobalMax.clear();
			iVarianceMin = iVariance[lIndex];
			/*if (lVarianceX[lIndex]<iVarianceXAverage)*/ iVarianceMinIndex = lIndex;
			iMinimaLeftOfGlobalMax.push_back(lIndex);
			bToMinimum = false;
			}
		//if (lVarianceX[lIndex]>iVarianceXAverage) iVarianceXMin = iVarianceXMax; //search for a new local minimum

		//found another local minimum with the same value as before
		//can be an candidate
		if (iVariance[lIndex] == iVarianceMin && bToMinimum) {

			bToMinimum = false;
			iMinimaLeftOfGlobalMax.push_back(lIndex);
			}

		//we moved away from out local minimum. Look out for another
		if (iVariance[lIndex] > iVarianceMin) {

			bToMinimum = true;
			}
		}

	//Make sure we have minima
	if (iMinimaLeftOfGlobalMax.size()==0) iMinimaLeftOfGlobalMax.push_back(0);
	if (iMinimaRightOfGlobalMax.size()==0) iMinimaRightOfGlobalMax.push_back(piVarianceLength-1);

	//Copy the local minima
	for (int iIndex=0; iIndex<(int)iMinimaLeftOfGlobalMax.size(); iIndex++) {

		piMinima.insert(piMinima.begin(), iMinimaLeftOfGlobalMax[iIndex]);
		}
	for (int iIndex=0; iIndex<(int)iMinimaRightOfGlobalMax.size(); iIndex++) {

		piMinima.push_back(iMinimaRightOfGlobalMax[iIndex]);
		}

	//Enlarge the interval so that it contains 98% of all the squared variance
	piStart = iVarianceMaxIndex;
	piEnd = iVarianceMaxIndex;
	double dPercentage;

	dPercentage = ((((double)100 * ((double)iVarianceSum[piEnd] - (double)iVarianceSum[piStart]))/(double)iVarianceTotal));

	while (dPercentage<99 && (iMinimaRightOfGlobalMax.size()>0 || iMinimaLeftOfGlobalMax.size()>0)) {

		if ((abs(iVarianceSum[piEnd]-iVarianceTotal)>iVarianceSum[piStart] && iMinimaRightOfGlobalMax.size()>0) || (iMinimaLeftOfGlobalMax.size()==0)) {

			//search for the next minimum to the right
			piEnd = iMinimaRightOfGlobalMax[0];
			iMinimaRightOfGlobalMax.erase(iMinimaRightOfGlobalMax.begin());
			}
		else {

			//search for the next minimum to the left
			piStart = iMinimaLeftOfGlobalMax[0];
			iMinimaLeftOfGlobalMax.erase(iMinimaLeftOfGlobalMax.begin());
			}
	dPercentage = ((((double)100 * ((double)iVarianceSum[piEnd] - (double)iVarianceSum[piStart]))/(double)iVarianceTotal));
		}

	delete[] iVarianceSum;
	//piStart = iVarianceMinIndex;
	//if (iMinimaLeftOfGlobalMax.size()>0) piStart = iMinimaLeftOfGlobalMax[iMinimaLeftOfGlobalMax.size()-1];

	piEnd -= piStart;

	//clear up
	delete[] iVarianceFlat;
	delete[] iVarianceSmooth;
	delete[] iVarianceSquare;
	}



int 
CBitmap::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

void
CBitmap::SaveBMP(std::wstring psFilename, Bitmap* poBitmap) {

	CLSID pngClsid;
	GetEncoderClsid(L"image/bmp", &pngClsid);
	Status iResult;

	EncoderParameters* pEncoderParameters = (EncoderParameters*)
	malloc(sizeof(EncoderParameters) + 2 * sizeof(EncoderParameter));

	// An EncoderParameters object has an array of
	// EncoderParameter objects. In this case, there is only
	// one EncoderParameter object in the array.
	pEncoderParameters->Count = 0;

	iResult = poBitmap->Save(psFilename.c_str(), &pngClsid, pEncoderParameters);

	free(pEncoderParameters);

}



//This function draws a rectangle on the image with the given color
eActionResult 
CBitmap::FillRectangle(DWORD pdColor, Rect &pcRectangle) {

	Graphics* oCanvas;
	SolidBrush* oBrush;
	eActionResult cResult;
	BitmapData* oDestinationBitmapData;
	BYTE* pDestinationPointer;
	BYTE bColor;
	Status cGDIStatus;

	//Initialize variables
	cResult = eImageActionError;

	//Clear the thumbnail because we will be change the image
	ClearThumbnail();

	bColor = (BYTE)(GetRValue(pdColor) * 0.3 + GetGValue(pdColor) * 0.59 + GetBValue(pdColor) * 0.11);
	
	//Open buffers to manipulate the images
	switch (oBitmap->GetPixelFormat()) {

		//Fill the rectangle for indexed bitmaps
		case PixelFormat8bppIndexed:
		case PixelFormat1bppIndexed:

			oDestinationBitmapData = new BitmapData();

			cGDIStatus = oBitmap->LockBits(&pcRectangle, ImageLockModeWrite, oBitmap->GetPixelFormat(), oDestinationBitmapData);
			
			if (cGDIStatus==Ok) {

				switch (oDestinationBitmapData->PixelFormat) {


					case PixelFormat8bppIndexed:

						//fill in grayscale
						for (long lY = 0; lY<(long)oDestinationBitmapData->Height; lY++) {

							pDestinationPointer = (BYTE*)oDestinationBitmapData->Scan0 + (oDestinationBitmapData->Stride * lY);
							
							for (long lX = 0; lX<(long)oDestinationBitmapData->Width; lX++) {

								pDestinationPointer[0] = bColor;
								pDestinationPointer++;
								}
							}

						break;

					case PixelFormat1bppIndexed:

						//Fill in Black and White
						int iCounter;
						BYTE bByte;
						BYTE bMask;
						bColor = (bColor>127) ? 1 : 0;

						for (long lY = 0; lY<(long)oDestinationBitmapData->Height; lY++) {

							pDestinationPointer = (BYTE*)oDestinationBitmapData->Scan0 + (oDestinationBitmapData->Stride * lY);
							
							iCounter = 0;
							bByte = 0;

							for (long lX = 0; lX<(long)oDestinationBitmapData->Width; lX++) {

								bByte <<= 1;							
								bByte |= bColor;
								
								//Write the byte
								iCounter++;
								if (iCounter==8) {
									
									iCounter=0;
									pDestinationPointer[0] = bByte;
									pDestinationPointer++;
									}
								}

							//Write the byte
							if (iCounter>0) {

								bMask = 0xff;
								bMask <<= (8 - iCounter);
								bMask ^= 0xff;
								bByte <<= (8 - iCounter);
								pDestinationPointer[0] &= bMask;
								pDestinationPointer[0] |= bByte;
								}
							}

						break;
						}

					//Assume everything worked
					cResult = eOk;

					oBitmap->UnlockBits(oDestinationBitmapData);
					}

				break;

			//Fill a rectangle in a non indexed bitmap. We can use
			//the standard GDI+ api here
			default:

				oCanvas = new Graphics(oBitmap);
				oBrush  = new SolidBrush(Color(GetRValue(pdColor), GetGValue(pdColor), GetBValue(pdColor)));
				
				if (oCanvas->GetLastStatus() == Ok) {

					if (oCanvas->FillRectangle(oBrush, pcRectangle) == Ok) {
						
						cResult = eOk;
						} 
					}

				delete oBrush;
				delete oCanvas;

				break;
		}

	return cResult;
}

//This function loads an image from the resource

Bitmap* 
CBitmap::LoadFromResource(LPCTSTR pName, LPCTSTR pType, HMODULE hInst)
{
    Bitmap* oBitmap;

	oBitmap = NULL;

	HRSRC hResource = ::FindResource(hInst, pName, pType);
    if (!hResource) return NULL;
    
    DWORD imageSize = ::SizeofResource(hInst, hResource);
    if (!imageSize) return NULL;

    const void* pResourceData = ::LockResource(::LoadResource(hInst, 
                                              hResource));
    if (!pResourceData) return NULL;

	HGLOBAL m_hBuffer  = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (m_hBuffer) {

        void* pBuffer = ::GlobalLock(m_hBuffer);
        
		if (pBuffer) {

            CopyMemory(pBuffer, pResourceData, imageSize);

            IStream* pStream = NULL;
            if (::CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) == S_OK) {

                oBitmap = Gdiplus::Bitmap::FromStream(pStream);
                pStream->Release();

				if (oBitmap) { 

					if (oBitmap->GetLastStatus() != Gdiplus::Ok) {
						
						delete oBitmap;
						oBitmap = NULL;
						}
					}
	            }
			}
        ::GlobalUnlock(m_hBuffer);
        }
    ::GlobalFree(m_hBuffer);

    return oBitmap;
	}

//This function adds this bitmap to an imagelist
int 
CBitmap::AddToImagelist(HIMAGELIST phImageList) {

	int iResult;

	iResult = -1;

	if (oBitmap!=NULL) iResult = AddToImagelist(oBitmap, phImageList);

	return iResult;
	}

//This function adds a bitmap to an imagelist while preserving transparancy
int 
CBitmap::AddToImagelist(Bitmap* poBitmap, HIMAGELIST phImageList, 
						bool pbDisabled, bool pbGrayed, int piImagelistReplaceIndex)
{
    HBITMAP hBitmap;
	void* ppvBits;
    BITMAPINFO* bmi = new BITMAPINFO();
	long lX, lY;
	long lValue;
	long lAlpha;
	Color oColor;
	Bitmap* oDestinationBitmap;

	if (!poBitmap) return -1;

	lX = poBitmap->GetWidth();
	lY = poBitmap->GetHeight();
	oDestinationBitmap = new Bitmap(lX, lY, PixelFormat32bppARGB);		

	while (lX>0) {

		lX--;
		lY = poBitmap->GetHeight();

		while (lY>0) {

			lY--;

			poBitmap->GetPixel(lX, lY, &oColor);

			if (oColor.GetValue() != Color::MakeARGB(255,255,0,255)) {

				if (pbGrayed || pbDisabled) {

					lValue = (oColor.GetR() + oColor.GetB() + oColor.GetG())/3;
					lAlpha = oColor.GetA();
					
					oColor.SetValue(Color::MakeARGB((BYTE)lAlpha,(BYTE)lValue,(BYTE)lValue,(BYTE)lValue));
					}

				if (pbDisabled) {

					oColor.SetValue(Color::MakeARGB((BYTE)(lAlpha/2),(BYTE)lValue,(BYTE)lValue,(BYTE)lValue));
					}
				}
			else {

				oColor = Color::MakeARGB(0,0,0,0);
				}
					
			oDestinationBitmap->SetPixel(lX, lY, oColor);
			}
		}

	//add the image to the imagelist
	bmi->bmiHeader.biSize = 40;            // Needed for RtlMoveMemory()
    bmi->bmiHeader.biBitCount = 32;        // Number of bits
    bmi->bmiHeader.biPlanes = 1;           // Number of planes
    bmi->bmiHeader.biWidth = oDestinationBitmap->GetWidth();     // Width of our new bitmap
    bmi->bmiHeader.biHeight = oDestinationBitmap->GetHeight();   // Height of our new bitmap
    
	oDestinationBitmap->RotateFlip( RotateNoneFlipY );
    // Required due to the way bitmap is copied and read

    hBitmap = CreateDIBSection(NULL, bmi, 0, &ppvBits, NULL, 0 );

    //create our new bitmap
    BitmapData bitmapData;

	oDestinationBitmap->LockBits( new Rect( 0, 0, 
									oDestinationBitmap->GetWidth(), 
									oDestinationBitmap->GetHeight() ), ImageLockModeRead, 
									PixelFormat32bppARGB, &bitmapData);
    RtlMoveMemory( ppvBits, bitmapData.Scan0, 
                   oDestinationBitmap->GetHeight() * bitmapData.Stride );
                   // copies the bitmap
    oDestinationBitmap->UnlockBits( &bitmapData );

	delete oDestinationBitmap;

	//Process the image
	int iResult;

	if (piImagelistReplaceIndex!=-1) {

		iResult = ImageList_Replace(phImageList, piImagelistReplaceIndex, hBitmap, NULL);
		}
	else {
		iResult = ImageList_Add( phImageList, hBitmap, NULL);
		}

	return iResult;
}



