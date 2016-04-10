#include "StdAfx.h"
#include "BitmapPCX.h"

CBitmapPCX::CBitmapPCX(void)
{
}

CBitmapPCX::~CBitmapPCX(void)
{
}



/*	PCX_GETC

	Read the next "token" in the picture file. If the high order
	two bits are not 11, then it's a single byte. If they are,
	it's a repeat count, and the next byte is the value to repeat.
*/

int 
CBitmapPCX::pcx_getc(WORD *c,WORD *n,FILE *fp, WORD maxn)
{
	static int csave=-1, nsave=-1;
	WORD i;

	if ( !fp )
	   return nsave = csave = -1;

	if ( nsave == -1 )
	{  *n = 1;

	   if ( (i=(WORD)getc(fp)) == EOF )
	      return false;

	   if ( (i & 0xc0) == 0xc0 )
	   {  *n = (WORD) (i & 0x3f);
	      if ( (i=getc(fp)) == EOF )
	         return false;
	   };

	   *c = i;
	} else
	{  *n = nsave;
	   *c = csave;
	   nsave = csave = -1;
	};

	if ( *n > maxn )
	{  nsave = *n - maxn;
	   csave = *c;
	   *n = maxn;
	};
	return true;
}


/*	PCX_PUTC

	Write a "token" to the picture file. If the repeat count is
	not 1, then write the repeat count | 0xc0 as the first byte,
	then the character as the next. If the character has its two
	high order bits set, then first write out a repeat count of
	1, then the character.

	pcx_xputc() counts the number of times that it receives a
	character, then passes that information on to pcx_putc().
*/

int 
CBitmapPCX::pcx_xputc(int c, FILE *fp)
{
	static int csave = -1, n = -1;

	if ( c == -1 )
	{  if ( csave != -1 )
	      if (!pcx_putc( csave, n, fp ) )
	         return false;
	   csave = n = -1;
	   return true;
	};

	if ( c == csave )
	{  n++;
	   return true;
	};

	if ( csave != -1 )
	{  if ( !pcx_putc(csave, n,  fp ) )
	      return false;
	   csave = n = -1;
	};

	csave = c;
	n = 1;
	return true;
	}

int 
CBitmapPCX::pcx_putc(int c,int n, FILE *fp )
{
	if ( (n > 1) || ((c & 0xc0) == 0xc0 ))
	{  while ( n > 0x3f )
	      if (!pcx_putc( c, 0x3f, fp ) )
	         return false;
	      else n -= 0x3f;

	   if ( !n )
	      return true;

      char d= 0xc0 | n;
	   if ( fwrite(&d, 1, 1, fp ) != 1  )
	      return false;
	}

   char d=(char)c;
	if ( fwrite(&d, 1, 1, fp ) != 1 )
	   return false;

	return true;
	}

bool 
CBitmapPCX::GetPCXPalette(std::wstring psFilename, char* pColor)
{
	PCXHDR ph;
	FILE * fp;

	if ((_wfopen_s(&fp, psFilename.c_str(), L"rb"))!=0) {

		return false;
		}

	fseek( fp, 0l, 0 );
	if (fread( &ph, 1, sizeof(ph), fp ) != sizeof(ph) )
		{
		fclose (fp);
		return false; // header is niet geheel gelezen
		}

   if (ph.bpp!=1)
   	{
		fclose (fp);
      return false; //PCX is not supported
      }

	//Read the pallete
	TRIPLET kleur;
	for (char i = 0; i < 2; i++)
		{
		kleur = ph.triple[i];
		*pColor=(char)(kleur.red);
		pColor++;
		*pColor=(char)(kleur.green);
		pColor++;
		*pColor=(char)(kleur.blue);
		pColor++;
		*pColor=(char)0;
		pColor++;
		}

	fclose (fp);
	return true;
	}


bool 
CBitmapPCX::GetPCXFileDimension(std::wstring psFilename, WORD* width, WORD* height,
		WORD *resolution)
{
	PCXHDR ph;
	FILE * fp;

	if ((_wfopen_s(&fp, psFilename.c_str(), L"rb"))!=0) {

		return false;
		}

	fseek( fp, 0l, 0 );
	if (fread( &ph, 1, sizeof(ph), fp ) != sizeof(ph) )
      {
      fclose (fp);
	   return false; // header is niet geheel gelezen
      }
	*height = ph.y2 - ph.y1+1;
   *width = ph.x2 - ph.x1+1;
	*resolution = ph.hres;

   if (ph.bpp!=1)
   	{
	fclose (fp);
      return false; //unsupported PCX type, it is not 1bpp
      }

   fclose (fp);
   return true;
}
	
eActionResult 
CBitmapPCX::GetLastStatus() {

	return eStatus;
	}

//This function loads the PCX file
Bitmap*
CBitmapPCX::LoadPCX(std::wstring psFilename, HGLOBAL& hMemoryBitmap) {

	int i, j, c, n, row, nrows, m_width;
	unsigned short bytes;
	PCXHDR ph;
	FILE * fp;
	char* dataBuf;
	char* p;
	bool bContinue;
    WORD height, width, resolution;

	eStatus = eOk;

	if (!GetPCXFileDimension(psFilename,  &width, &height, &resolution)) {

		eStatus = eFileResultInvalidParameter;
		return false;
		}
	
	if ((resolution<1) || (resolution>4800)) {

		//resulution valt buiten scope

		}

	BITMAPINFOHEADER bmiHeader;
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = width;
	bmiHeader.biHeight = height;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 1;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = 0;
	bmiHeader.biXPelsPerMeter = (LONG)((resolution*100)/2.54); //pixels per meter
	bmiHeader.biYPelsPerMeter = (LONG)((resolution*100)/2.54);
	bmiHeader.biClrUsed = 2;
	bmiHeader.biClrImportant = 0;
	bmiHeader.biSizeImage =
				(int)((bmiHeader.biWidth*bmiHeader.biBitCount+31)&(~31))/8 * bmiHeader.biHeight;

	DWORD size = sizeof(BITMAPINFOHEADER) + int(bmiHeader.biClrUsed * sizeof(RGBQUAD))+ bmiHeader.biSizeImage;

	HGLOBAL DibHandle1 = ::GlobalAlloc(GMEM_MOVEABLE, size);
                                                                    
	LPBITMAPINFO DibHandle;
	DibHandle = (LPBITMAPINFO)::GlobalLock(DibHandle1);
	DibHandle->bmiHeader = bmiHeader;

	if (!GetPCXPalette(psFilename,  (char*)DibHandle->bmiColors)) {

		//Error reading pallette, guess one and pray it is correct...
		char* pColor;
		pColor=(char*)DibHandle->bmiColors;
		for (char i = 0; i < 2; i++)
			{
			*pColor=(char)(i*255);
			pColor++;
			*pColor=(char)(i*255);
			pColor++;
			*pColor=(char)(i*255);
			pColor++;
			*pColor=(char)0;
			pColor++;
			}
		}

	dataBuf = (char*)DibHandle+sizeof(BITMAPINFOHEADER) + int(bmiHeader.biClrUsed * sizeof(RGBQUAD));

	//try to open file
	if ((_wfopen_s(&fp, psFilename.c_str(), L"rb"))!=0) {

		//terminate
		eStatus = eFileResultNotFound;
		GlobalUnlock(DibHandle1);
		GlobalFree(DibHandle1);
		return NULL;
		}

	//check if it is a valid pcx file
	fseek( fp, 0l, 0 );
	if (fread( &ph, 1, sizeof(ph), fp ) != sizeof(ph) )
		{
		fclose (fp);

		//terminate
		GlobalUnlock(DibHandle1);
		GlobalFree(DibHandle1);
		eStatus = eFileResultNotSupported;
		return NULL;
		}

	//check if it is a pcx supported by copying machine (bpp!=1)
	pcx_getc( (WORD *)0, (WORD *)0, (FILE *)0, 0 );

	bytes = ph.bpl;
	nrows = ph.y2 - ph.y1 +1;

	if (ph.bpp!=1)
		{
		fclose (fp);

		//terminate
		GlobalUnlock(DibHandle1);
		GlobalFree(DibHandle1);
		eStatus = eFileResultInvalidParameter;
		return 0;
		}

	//start reading
	m_width = (int)(((ph.x2 - ph.x1 +1)*1+31)&(~31))/8;
	row=nrows-1;

	bContinue = true;

	while (row>=0 && bContinue)
		{
		p = dataBuf + row*m_width;
		for (n=0; n<m_width; n++)
			{
			*(p+n)=(char)0;
			}

		for ( n=i=0; i<bytes; i += n )
			{
			if (!pcx_getc( (unsigned short *)&c, (unsigned short *)&n, (FILE *)fp, bytes - i ) )
				{
				//terminate
				eStatus = eFileResultInvalidParameter;
				bContinue = false;
				}

			if (bContinue) for ( j=0; j < n; j++ ) *p++ = (char)c;
			}
		row--;
		}

	//done: close the file
	fclose(fp);

	if (eStatus!=eOk) {
		
		GlobalUnlock(DibHandle1);
		GlobalFree(DibHandle1);
		return NULL;
		}

	GlobalUnlock(DibHandle1);
	
	LPBITMAPINFO pBitmapInfo;
	void* pBitmapData;
	
	pBitmapInfo = (LPBITMAPINFO)GlobalLock(DibHandle1);
	pBitmapData = ((BYTE*)pBitmapInfo->bmiColors + pBitmapInfo->bmiHeader.biClrUsed*sizeof(RGBQUAD));

	Bitmap* oBitmap= new Bitmap(pBitmapInfo, pBitmapData);
	oBitmap->SetResolution(ph.hres, ph.vres);

	GlobalUnlock(DibHandle1);

	return oBitmap;
}

bool
CBitmapPCX::SavePCX(std::wstring psFilename, Bitmap* poBitmap) {

		int i, row, nrows, m_width;
		unsigned short bytes;
		PCXHDR ph;
		FILE * fp;
		UINT8* dataBuf;
		UINT8* p;

	//open the new pcx file for writing
	if (_wfopen_s(&fp, psFilename.c_str(), L"wb")!=0) {

		//terminate
		eStatus = eFileResultCouldNotCreate;
		return false;
		}

	fseek( fp, 0l, 0 );

   //Initialise PCX header
	ph.maker=(char)10;
	ph.version=(char)3;
	ph.code=(char)1;
	ph.bpp=(char)1;
	ph.x1=(WORD)0;
	ph.x2=(WORD)poBitmap->GetWidth()-1;
	ph.y1=(WORD)0;
	ph.y2=(WORD)poBitmap->GetHeight()-1;
	ph.hres=(WORD)poBitmap->GetHorizontalResolution();
	ph.vres=(WORD)poBitmap->GetVerticalResolution();

	UINT size = poBitmap->GetPaletteSize();
	ColorPalette* palette = (ColorPalette*)malloc(size);
	poBitmap->GetPalette(palette, size);

	if (palette->Count!=2) {

		TRIPLET kleur;
		kleur.red=0;
		kleur.green=0;
		kleur.blue=0;
		ph.triple[0]=kleur;

		kleur.red=255;
		kleur.green=255;
		kleur.blue=255;
		ph.triple[1]=kleur;
		}
	else
		{
		TRIPLET kleur;
		for (int i=0; i<2; i++)
			{
			kleur.red=(unsigned char)(palette->Entries[i] >> RED_SHIFT) & 0xFF;
			kleur.green=(unsigned char)(palette->Entries[i] >> GREEN_SHIFT) & 0xFF;
			kleur.blue=(unsigned char)(palette->Entries[i] >>BLUE_SHIFT) & 0xFF;
			ph.triple[i]=kleur;
			}
		}

	free(palette);

	ph.nplanes=(char)1;
	ph.bpl=(WORD)(((poBitmap->GetWidth())+7))/8;
	ph.vmode=(char)1;

	bytes = ph.bpl;
	nrows = poBitmap->GetHeight();
	m_width = (int)(((poBitmap->GetWidth())*1+31)&(~31))/8;

	//Write PCX Header
	if ( fwrite(&ph, 1, sizeof(ph), fp ) != sizeof(ph) )
		{
		fclose(fp);
		eStatus = eFileResultUnknownError;
		return false;
		}

	row=nrows-1;

	Rect rect(0, 0, poBitmap->GetWidth(), poBitmap->GetHeight());
	BitmapData* bitmapData = new BitmapData;
	poBitmap->LockBits(&rect, ImageLockModeRead, PixelFormat1bppIndexed, bitmapData);

	dataBuf = (UINT8*)bitmapData->Scan0;
	
	while (row>=0)
		{
		pcx_xputc(-1,fp);
		p = dataBuf + bitmapData->Stride * (nrows - 1 - row);
		for (i=0; i<bytes; i++)
			{
			if (!pcx_xputc( *p++, fp ) )
				{
				fclose(fp);
				poBitmap->UnlockBits(bitmapData);
				delete bitmapData;
				//terminate
				eStatus = eFileResultUnknownError;
				return false;
				}
			}
		row--;
		}

	poBitmap->UnlockBits(bitmapData);
	delete bitmapData;

	//close file
	pcx_xputc( -1, fp );
	fclose(fp);

	return true;
}