#pragma once
#include "Bitmap.h"

/*	PCX   Header for PCX file gen. routines

	Based on ZSOFT technical reference manual 
	from Shannon at ZSOFT, who asked to see a copy 
	of this when it's done.
	
	ZSOFT corp.
	1950 Spectrum Circle
	Suite A-495
	Marietta, GA 30067
	(404) 428-0008
*/

#define WIDTHcharS(bits)    (((bits) + 31) / 32 * 4)

typedef struct {
   unsigned char red, green, blue;
} TRIPLET;

typedef struct {
   char maker,
 	version,
	code,
	bpp;
   WORD	x1, y1, x2, y2,
   	hres, vres;
   TRIPLET triple[16]; /* palette */
   char vmode, 
   	nplanes;
   WORD	bpl;
   char __unused[128-68];
} PCXHDR;

typedef struct {
   PCXHDR hdr;
   UCHAR **rows[4];
} PCXPIC;

//-----------------------------------------------------------------------

class CBitmapPCX
{
public:
	CBitmapPCX(void);
	~CBitmapPCX(void);

	Bitmap* LoadPCX(std::wstring psFilename, HGLOBAL& hMemoryBitmap);
	bool SavePCX(std::wstring psFilename, Bitmap* poBitmap);

	static bool GetPCXFileDimension(std::wstring psFilename, WORD* width, WORD* height, WORD *resolution);
	static bool GetPCXPalette(std::wstring psFilename, char* pColor);

	eActionResult GetLastStatus();


private:

	eActionResult eStatus;

	int pcx_getc(WORD *c,WORD *n,FILE *fp, WORD maxn);
	int pcx_xputc(int c, FILE *fp);
	int pcx_putc(int c, int n, FILE *fp );
};
