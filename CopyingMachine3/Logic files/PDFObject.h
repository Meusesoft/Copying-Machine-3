#pragma once

enum ePDFObjectType {eCatalog, ePages, ePage, eOutline, eResources, eContents, eImage, eInfo, eText, eTextContents, eFont, eMetadata, eOutputIntent};

class CPDFObject {

    public:

        CPDFObject(long plObjectId, long plParentId, ePDFObjectType peType);
        ~CPDFObject();

        void WriteToFile(ofstream* poFile);
        void AddChild(CPDFObject* poChild);
		CPDFObject* GetChildObject(ePDFObjectType peType);

		void StartText();
		bool AddText(std::string psText, RectF pcTextBox);
		void EndText();
		bool GetTextObjectOpen();

		bool AddContent(std::string psOperation);
		bool AddMemoryBlock(void* poBlock, long plSize);
		bool AddCompressedMemoryBlock(void* poBlock, long plSize);
        bool AddBitmapStream(Bitmap* poBitmap);
		void AddTextStream(std::string psText);

        void AddCreator(std::string psCreator);
        void AddProducer(std::string psProducer);
		void AddCreationDate(std::string psDate);

        long lFilePosition;
        long lObjectId;

        long lContentId;
        long lResourceId;
		long lFontId;
        long lJPEGCompression;
		long lICCResourceId;

		long GetWidth();
		long GetHeight();
		Rect GetContentBox();
		void SetContentBox(Rect pcContentBox);
		float GetContentScale();
		void SetContentScale(float pfScale);

        ePDFObjectType eType;

    private:

		bool MakeZLibImage(Bitmap* poBitmap);
		bool MakeJPEGImage(Bitmap* poBitmap);
		int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
		void Write(ofstream* poFile, std::string psLine);

		vector<CPDFObject*> oChildObjects;
        long lParentId;
		Rect cContentBox;

		void* oMemoryObject;
		long  lMemoryObjectSize;

        long lWidth;
        long lHeight;
        long lColorSpace;
        long lFilter;
		float fContentScale;

		std::string sCreator;
        std::string sProducer;
		std::string sCreationDate;
		bool bTextObjectOpen;

    };
 