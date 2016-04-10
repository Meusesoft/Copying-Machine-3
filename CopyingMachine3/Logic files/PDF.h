#pragma once

class CPDF {

    public:

        CPDF();
        ~CPDF();

		eActionResult WriteToFile(std::wstring psFilename);

        long  AddPage();
        bool  AddBitmapToPage(long plPageId, Bitmap* poBitmap);
		bool  AddTextToPage(long plPageId, std::string psText, RectF pcTextBox);
        bool  AddHBitmapToPage(long plPageId, HANDLE phBitmap);

        long lJPEGCompression;

    private:

        void WriteCrossReferenceToFile();
        Bitmap* ConvertDIBtoBitmap(HANDLE phDIB);
		std::wstring CPDF::CreateID(std::wstring psFilename);

		std::ofstream* oFile;

        long lNextObjectNumber;
		long lICCResourceId;
        vector<void*> oPDFObjects;
        void* oPagesObject;
        void* oRootObject;
        void* oInfoObject;

    };
//---------------------------------------------------------------------------
 