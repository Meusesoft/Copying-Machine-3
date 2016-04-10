#include "stdafx.h"
#include "PDFObject.h"
#include "zlib.h"
//---------------------------------------------------------------------------

CPDFObject::CPDFObject(long plObjectId, long plParentId, ePDFObjectType peType) {

    lObjectId = plObjectId;
    lParentId = plParentId;

    lResourceId = -1;
    lContentId = -1;
	lFontId = -1;
    lJPEGCompression = 25;
	bTextObjectOpen = false;

    oMemoryObject = NULL;
	lMemoryObjectSize = 0;

    eType = peType;

	//Initialise page width and height
	lWidth = 0;
	lHeight = 0;

	if (peType == ePage) {

        lWidth = 612;
        lHeight = 792;
		}

	//Initialise page content box
	cContentBox = Rect(0, 0, lWidth, lHeight);
    }
//---------------------------------------------------------------------------
CPDFObject::~CPDFObject() {

    //we don't need to delete all objects in the vector, just clearing
    //the vector is enough. Deleting all objects is done in the CPDF
    //instance

    oChildObjects.clear();

    //free memory of stream objects
    if (oMemoryObject!=NULL) {

        free(oMemoryObject);
        }

    }
//---------------------------------------------------------------------------
long 
CPDFObject::GetWidth() {

	return lWidth;
}
//---------------------------------------------------------------------------
long 
CPDFObject::GetHeight() {

	return lHeight;
}
//---------------------------------------------------------------------------
Rect 
CPDFObject::GetContentBox() {

	return cContentBox;
	}

//---------------------------------------------------------------------------
float 
CPDFObject::GetContentScale() {

	return fContentScale;
	}
//---------------------------------------------------------------------------
bool
CPDFObject::GetTextObjectOpen() {

	return bTextObjectOpen;
	}
//---------------------------------------------------------------------------
void 
CPDFObject::SetContentScale(float pfContentScale) {

	fContentScale = pfContentScale;
	}
//---------------------------------------------------------------------------
void 
CPDFObject::SetContentBox(Rect pcContentBox) {

	cContentBox = pcContentBox;
	}
//---------------------------------------------------------------------------
void
CPDFObject::AddChild(CPDFObject* poChild) {

    oChildObjects.push_back(poChild);
    }

//---------------------------------------------------------------------------
CPDFObject* 
CPDFObject::GetChildObject(ePDFObjectType peType) {

	CPDFObject* oResult;
	unsigned long lIndex;

	lIndex = oChildObjects.size();
	oResult = NULL;

	while (lIndex>0) {

		lIndex--;

		if (oChildObjects[lIndex]->eType == peType) {

			oResult = oChildObjects[lIndex];
			lIndex=0;
			}
		}
	
	return oResult;
	}
//---------------------------------------------------------------------------
void
CPDFObject::WriteToFile(ofstream* poFile) {

    char cNumber[20];
    CPDFObject* oChild;
    //TDateTime oCurrentDateTime;
    TIME_ZONE_INFORMATION cTimeZoneInformation;
    long lBiasMinutes;
    long lBiasHours;
	std::string sBias;
	std::string sData;
	long lIndex;

    try {

        //save the current position in the file
        lFilePosition = (long)poFile->tellp();

        //write the contents of this object to file

        //write start of object
        sprintf_s(cNumber, sizeof(cNumber), "%d", lObjectId);
		poFile->write(cNumber, (streamsize)strlen(cNumber));
        sData = " 0 obj\r<< \r";
		poFile->write(sData.c_str(), sData.length());

        switch (eType) {

            //the catalog is the root of a documents tree hierarchy
            case eCatalog: {

                sData = "/MarkInfo<</Marked true>>\r";
				poFile->write(sData.c_str(), sData.length());
                sData = "/Type /Catalog\r";
				poFile->write(sData.c_str(), sData.length());
				

                for (unsigned long lIndex=0; lIndex < oChildObjects.size(); lIndex++) {

                    oChild = (CPDFObject*)oChildObjects[lIndex];

                    //child is a pages object
                    if (oChild->eType == ePages) {

                        sData = "/Pages ";
						poFile->write(sData.c_str(), sData.length());

						sprintf_s(cNumber, sizeof(cNumber), "%d", oChild->lObjectId);
                        poFile->write(cNumber, (streamsize)strlen(cNumber));

                        sData = " 0 R\r";
        				poFile->write(sData.c_str(), sData.length());
		                }

                    //child is a metadata object
                    if (oChild->eType == eMetadata) {

						sData = "/Metadata ";
        				poFile->write(sData.c_str(), sData.length());

						sprintf_s(cNumber, sizeof(cNumber), "%d", oChild->lObjectId);
                        poFile->write(cNumber, (streamsize)strlen(cNumber));

                        sData = " 0 R\r";
        				poFile->write(sData.c_str(), sData.length());
                        }

					//child is an outline objects
                    if (oChild->eType == eOutline) {

                        sData = "/Outline ";
        				poFile->write(sData.c_str(), sData.length());

						sprintf_s(cNumber, sizeof(cNumber), "%d", oChild->lObjectId);
                        poFile->write(cNumber, (streamsize)strlen(cNumber));

                        sData = " 0 R\r";
        				poFile->write(sData.c_str(), sData.length());
                        }

					//child is an output intent
                    if (oChild->eType == eOutputIntent) {

						std::string OutputIntents = "/OutputIntents[<</Type/OutputIntent/S/GTS_PDFA1/OutputConditionIdentifier(sRGB IEC61966-2.1) /RegistryName(http://www.color.org) /Info(sRGB IEC61966-2.1)/DestOutputProfile ";
						poFile->write(OutputIntents.c_str(), OutputIntents.length());

						sprintf_s(cNumber, sizeof(cNumber), "%d", oChild->lObjectId);

                        poFile->write(cNumber, (streamsize)strlen(cNumber));

                        sData = " 0 R>>]\r";
        				poFile->write(sData.c_str(), sData.length());
                        }                    
				}

                break;
                }

            case eOutline: {

                sData = "/Type /Outlines\r";
				poFile->write(sData.c_str(), sData.length());
                sData = "/Count ";
				poFile->write(sData.c_str(), sData.length());

				sprintf_s(cNumber, sizeof(cNumber), "%d", oChildObjects.size());
                poFile->write(cNumber, (streamsize)strlen(cNumber));

                sData = "\r";
				poFile->write(sData.c_str(), sData.length());

                break;
                }

            //the pages are a page tree node, and contains a some pages
            case ePages: {

                sData = "/Type /Pages\r";
				poFile->write(sData.c_str(), sData.length());

                if (oChildObjects.size() > 0) {

                    sData = "/Kids [ ";
					poFile->write(sData.c_str(), sData.length());

                    for (unsigned long lIndex=0; lIndex < oChildObjects.size(); lIndex++) {

                        oChild = (CPDFObject*)oChildObjects[lIndex];

						sprintf_s(cNumber, sizeof(cNumber), "%d", oChild->lObjectId);
                        poFile->write(cNumber, (streamsize)strlen(cNumber));

                        sData = " 0 R\r";
						poFile->write(sData.c_str(), sData.length());
                        }
                    
					sData = "]\r";
					poFile->write(sData.c_str(), sData.length());
                    }


                sData = "/Count ";
				poFile->write(sData.c_str(), sData.length());

				sprintf_s(cNumber, sizeof(cNumber), "%d", oChildObjects.size());
                poFile->write(cNumber, (streamsize)strlen(cNumber));

				sData = "\r";
				poFile->write(sData.c_str(), sData.length());

                break;
                }

            //the page is a leaf of the page tree, and contains info on the
            //page and it contents
            case ePage: {

                Write(poFile, "/Type /Page\r\n");

                //the media box, 
                Write(poFile, "/MediaBox [0 0 ");
				sprintf_s(cNumber, sizeof(cNumber), "%d %d", lWidth, lHeight);
                poFile->write(cNumber, (streamsize)strlen(cNumber));
                Write(poFile, "]\r\n");

				//get the parent
                if (lParentId!=-1) {

                    Write(poFile, "/Parent ");

					sprintf_s(cNumber, sizeof(cNumber), "%d", lParentId);
                    poFile->write(cNumber, (streamsize)strlen(cNumber));
                    Write(poFile, " 0 R\r\n");
                    }


                if (lContentId!=-1) {
                    //contents of the page
					sprintf_s(cNumber, sizeof(cNumber), "%d", lContentId);
                    Write(poFile, "/Contents ");
                    poFile->write(cNumber, (streamsize)strlen(cNumber));
                    Write(poFile, " 0 R\r\n");
                    }

				//Write the resource-tag
                Write(poFile, "/Resources << /ProcSet [/PDF /ImageB /ImageC /Text]\r\n");
				
				lIndex = oChildObjects.size();

				//Loop through the child object
				while (lIndex>0) {

					lIndex--;
				
					oChild = oChildObjects[lIndex];

					switch (oChild->eType) {
	
						case eText:

							Write(poFile, "/ProcSet ");
							sprintf_s(cNumber, sizeof(cNumber), "%d", oChild->lObjectId);
							poFile->write(cNumber, (streamsize)strlen(cNumber));
							Write(poFile, " 0 R\r\n");
							break;
			
						case eFont:

							Write(poFile, "/Font << /F1 ");
							sprintf_s(cNumber, sizeof(cNumber), "%d", oChild->lObjectId);
							poFile->write(cNumber, (streamsize)strlen(cNumber));
							Write(poFile, " 0 R >>\r\n");
							break;

						case eImage:

							Write(poFile, "/XObject << /Image1 ");
							sprintf_s(cNumber, sizeof(cNumber), "%d", oChild->lObjectId);
							poFile->write(cNumber, (streamsize)strlen(cNumber));
							Write(poFile, " 0 R\r\n");
							Write(poFile, ">>\r\n");
							break;
						}
					}
				
				Write(poFile, ">>\r\n");

                break;
                }

			//this object is content and describes the graphical
			//operations on the website
			case eContents: {

				if (bTextObjectOpen) EndText();

				Write(poFile, "/Length ");
				sprintf_s(cNumber, sizeof(cNumber), "%d", lMemoryObjectSize);
                poFile->write(cNumber, (streamsize)strlen(cNumber));
                Write(poFile, "\r");
				break;
				}


			case eText: {

				Write(poFile, "[/PDF /Text]");
				break;
				}

			case eFont: {

				sData = "/Type /Font ";
				poFile->write(sData.c_str(), sData.length());
				sData = "/Subtype /Type1 ";
				poFile->write(sData.c_str(), sData.length());
				sData = "/Name /F2 ";
				poFile->write(sData.c_str(), sData.length());
				sData = "/BaseFont /Courier ";
				poFile->write(sData.c_str(), sData.length());				
				sData = "/Encoding /MacRomanEncoding";
				poFile->write(sData.c_str(), sData.length());
				//poFile->write(">>", 2);

				break;
				}

            //the object is an ICC profile
            case eOutputIntent: {

                Write(poFile, "/N 3\r");
                //poFile->write("/Filter[/FlateDecode]\r", 21);
               
				//std::string FilterHexDecode = "/Filter/ASCIIHexDecode";
				std::string FilterHexDecode = "/Filter/FlateDecode\r";
                poFile->write(FilterHexDecode.c_str(), FilterHexDecode.length());

                if (oMemoryObject!=NULL) {

                    Write(poFile, "/Length ");
					sprintf_s(cNumber, sizeof(cNumber), "%d", lMemoryObjectSize + 1);
                    poFile->write(cNumber, (streamsize)strlen(cNumber));
                    Write(poFile, "\r");
                    }

                break;
                }

            //the Image is a stream containg the info of a bitmap
            case eImage: {

                Write(poFile, "/Type /XObject\r");
                Write(poFile, "/Subtype /Image\r");
                Write(poFile, "/Name /Image1\r");

                Write(poFile, "/Width ");
				sprintf_s(cNumber, sizeof(cNumber), "%d", lWidth);
                poFile->write(cNumber, (streamsize)strlen(cNumber));
                Write(poFile, "\r");

                Write(poFile, "/Height ");
				sprintf_s(cNumber, sizeof(cNumber), "%d", lHeight);
                poFile->write(cNumber, (streamsize)strlen(cNumber));
                Write(poFile, "\r");


                switch (lColorSpace) {

                    case 1: //Black/white
                        {
                        Write(poFile, "/Filter /FlateDecode\r");
                        Write(poFile, "/ColorSpace /DeviceGray\r");
                        Write(poFile, "/BitsPerComponent ");
						sprintf_s(cNumber, sizeof(cNumber), "%d", lColorSpace);
                        poFile->write(cNumber, (streamsize)strlen(cNumber));
                        Write(poFile, "\r");
                        break;
                        }

                    case 8: //Greyscale
                    case 24: //Color
                        {
                        Write(poFile, "/Filter [ /DCTDecode ]\r");
//						std::string ColorSpace = "/ColorSpace /DeviceRGB\r";
						sprintf_s(cNumber, sizeof(cNumber), "%d", lICCResourceId);
						std::string ICCResourceId = cNumber;
						std::string ColorSpace = "/ColorSpace [/ICCBased "+ICCResourceId+" 0 R]\r";
						// ColorSpace = "/ColorSpace /DeviceRGB\r";
                        Write(poFile, ColorSpace);
                        Write(poFile, "/BitsPerComponent 8\r");
                        break;
                        }
                    }

                if (oMemoryObject!=NULL) {

                    Write(poFile, "/Length ");
					sprintf_s(cNumber, sizeof(cNumber), "%d", lMemoryObjectSize);
                    poFile->write(cNumber, (streamsize)strlen(cNumber));
                    Write(poFile, "\r");
                    }

                break;
                }

			//the Image is a stream containg the info of a bitmap
           /* case eContents: {

                if (oMemoryObject!=NULL) {

                    poFile->write("/Filter /FlateDecode\r", 21);
                    poFile->write("/Length ", 8);
					sprintf_s(cNumber, sizeof(cNumber), "%d", lMemoryObjectSize);
                    poFile->write(cNumber, (streamsize)strlen(cNumber));
                    poFile->write("\r", 1);
                    }

                break;
                }*/

			case eMetadata: {

				std::string sMetaData;

				char cBeginCode[5];
				cBeginCode[0] = 0xEF;
				cBeginCode[1] = 0xBB;
				cBeginCode[2] = 0xBF;
				cBeginCode[3] = 0x00;
				std::string sBeginCode = cBeginCode;


				sMetaData = "<?xpacket begin=\""+sBeginCode+"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>";
				sMetaData += "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 5.2-c001 63.139439, 2010/09/27-13:37:26        \">";
				sMetaData += "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">";
				sMetaData += "<rdf:Description rdf:about=\"\" xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\">";
				sMetaData += "<xmp:ModifyDate>"+sCreationDate+"</xmp:ModifyDate>";
				sMetaData += "<xmp:CreateDate>"+sCreationDate+"</xmp:CreateDate>";
				sMetaData += "<xmp:MetadataDate>"+sCreationDate+"</xmp:MetadataDate>";
				sMetaData += "<xmp:CreatorTool>"+sCreator+"</xmp:CreatorTool>";
				sMetaData += "</rdf:Description>";
				sMetaData += "<rdf:Description rdf:about=\"\" xmlns:xmpMM=\"http://ns.adobe.com/xap/1.0/mm/\">";
				sMetaData += "<xmpMM:DocumentID>uuid:ff695e49-867a-4f41-b803-e46b087dfae3</xmpMM:DocumentID>";
				sMetaData += "<xmpMM:InstanceID>uuid:27458b5c-0eb6-4b3a-937f-55ca0f77a8fd</xmpMM:InstanceID>";
				sMetaData += "</rdf:Description>";
				sMetaData += "<rdf:Description rdf:about=\"\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\">";
				sMetaData += "<dc:format>application/pdf</dc:format>";
				/*sMetaData += "<dc:creator>";
				sMetaData += "<rdf:Seq>";
				sMetaData += "<rdf:li>maasrj</rdf:li>";
				sMetaData += "</rdf:Seq>";
				sMetaData += "</dc:creator>";*/
				sMetaData += "</rdf:Description>";
				sMetaData += "<rdf:Description rdf:about=\"\" xmlns:pdf=\"http://ns.adobe.com/pdf/1.3/\">";
				sMetaData += "<pdf:Producer>"+sProducer+"</pdf:Producer>";
				sMetaData += "</rdf:Description>";
				/*sMetaData += "<rdf:Description rdf:about=\"\" xmlns:pdfx=\"http://ns.adobe.com/pdfx/1.3/\">";
				sMetaData += "<pdfx:SourceModified>D:20141230141758</pdfx:SourceModified>";
				sMetaData += "<pdfx:Company>Provincie Zuid-Holland</pdfx:Company>";
				sMetaData += "<pdfx:Copyright>Copyright 2007-2008 PDF/A Competence Center</pdfx:Copyright>";
				sMetaData += "<pdfx:WebStatement>www.pdfa.org</pdfx:WebStatement>";
				sMetaData += "</rdf:Description>";*/


				sMetaData += "<rdf:Description rdf:about=\"\" xmlns:pdfaid=\"http://www.aiim.org/pdfa/ns/id/\">";
				sMetaData += "<pdfaid:part>1</pdfaid:part>";
				sMetaData += "<pdfaid:conformance>A</pdfaid:conformance>";
				sMetaData += "</rdf:Description>";


				sMetaData += "</rdf:RDF>";
				sMetaData += "</x:xmpmeta>";
				sMetaData += "<?xpacket end=\"w\"?>";

				AddContent(sMetaData);

				int iStreamLength = sMetaData.length();				
				sprintf_s(cNumber, sizeof(cNumber), "%d", iStreamLength);

				Write(poFile, "/Length ");
				poFile->write(cNumber, (streamsize)strlen(cNumber));
				Write(poFile, "/Subtype/XML/Type/Metadata");

				break;
				}	



            //the info is a set of data about the PDF creation
            case eInfo: {

				std::string sDate;

                Write(poFile, "/Type /Info\r");

                Write(poFile, "/Creator (");
                Write(poFile, sCreator);
                Write(poFile, ")\r");

                Write(poFile, "/Producer (");
                Write(poFile, sProducer);
                Write(poFile, ")\r");

                Write(poFile, "/CreationDate (");
                Write(poFile, sCreationDate);
                Write(poFile, ")\r");

                Write(poFile, "/ModDate (");
                Write(poFile, sCreationDate);
                Write(poFile, ")\r");

                break;
                }
            }

        //write end of object
        Write(poFile, " >> \r");

        if (oMemoryObject!=NULL) {

            //write the stream
            if (oMemoryObject!=NULL) {

                Write(poFile, "stream\r\n");

                poFile->write((char*)oMemoryObject, lMemoryObjectSize);

				//if (eType==eOutputIntent) poFile->write(">", 1);

                Write(poFile, "\r\nendstream\r");
                }
            }

        Write(poFile, "endobj\r");
        }
    catch (...) {

        }
    }
//---------------------------------------------------------------------------
bool
CPDFObject::AddBitmapStream(Bitmap* poBitmap) {

	bool bResult;

	bResult = true;

    //remove old stream if present
    if (oMemoryObject!=NULL) {
      
		free(oMemoryObject);
        }

    //init variables containing info about characteristics of bitmap
    lWidth = poBitmap->GetWidth();
    lHeight = poBitmap->GetHeight();

	//compress the bitmap. Type of compression depends on pixelformat
	switch (poBitmap->GetPixelFormat()) {

        case PixelFormat1bppIndexed: 

			lColorSpace = 1;
			bResult = MakeZLibImage(poBitmap);
			break;

        case PixelFormat8bppIndexed: 

			lColorSpace = 8;

        case PixelFormat24bppRGB: 

			lColorSpace = 24;
			bResult = MakeJPEGImage(poBitmap);
			break;

        default: 

			lColorSpace = 0;
			bResult = false;	 
			break;
        }

	return bResult;
    }

//Add allocated memory block to the PDFobject
bool 
CPDFObject::AddMemoryBlock(void* poBlock, long plSize) {

	    //remove old stream if present

		if (oMemoryObject!=NULL) free(oMemoryObject);
		oMemoryObject = malloc(plSize);
		bool bResult = (oMemoryObject!=NULL);
		if (bResult) {
			lMemoryObjectSize = plSize;
			memcpy_s(oMemoryObject, plSize, poBlock, plSize);
			}

		//oMemoryObject = oMemoryObject;
	//lMemoryObjectSize = plSize;

	return true;
}

//Add allocated memory block to the PDFobject and compressed it
bool 
CPDFObject::AddCompressedMemoryBlock(void* poBlock, long plSize) {

	void* oInMemoryObject;
	void* oOutMemoryObject;
    long lScanLineSize;
	long lInMemoryObjectSize;
    unsigned long lCompressedSize;
    int iResult;
	bool bResult;

	oOutMemoryObject = malloc((long)(plSize * 1.1 + 20));

	//compress the data
	lCompressedSize =  plSize;
    iResult = compress((unsigned char*)oOutMemoryObject, &lCompressedSize, (unsigned char*)poBlock, plSize);

    switch (iResult) {

        case Z_MEM_ERROR: {

            //MessageBox(NULL, L"Not enough memory", L"Error", MB_OK);
			bResult = false;
            break;
            }

        case Z_BUF_ERROR: {

			realloc(oOutMemoryObject, (plSize * 2 + 5000));
            lCompressedSize =  plSize * 2 + 5000;

			iResult = compress((unsigned char*)oOutMemoryObject, &lCompressedSize, (unsigned char*)poBlock, plSize);

            if (iResult == Z_BUF_ERROR) {
                //MessageBox(NULL, L"Not enough room in the output buffer", L"Error", MB_OK);
				bResult = false;
				}

            break;
            }

        case Z_STREAM_ERROR: {

            //MessageBox(NULL, L"Level parameter is invalid", L"Error", MB_OK);
			bResult = false;
            break;
            }
        }

	//Copy the data to the memory object
    if (iResult == Z_OK) {

		if (oMemoryObject!=NULL) free(oMemoryObject);
		oMemoryObject = malloc(lCompressedSize);
		bResult = (oMemoryObject!=NULL);
		if (bResult) {
			lMemoryObjectSize = lCompressedSize;
			memcpy_s(oMemoryObject, lCompressedSize, oOutMemoryObject, lCompressedSize);
			}
		}

	free(oOutMemoryObject);

	return bResult;
}


//---------------------------------------------------------------------------
void
CPDFObject::StartText() {

	std::string sTextObject;

	if (!bTextObjectOpen) {
	
		sTextObject = "q\r\n";//BT\r\n";

		bTextObjectOpen = true;

		AddContent(sTextObject);
	}
}	
//---------------------------------------------------------------------------
void
CPDFObject::EndText() {

	std::string sTextObject;

	if (bTextObjectOpen) {
		
		bTextObjectOpen = false;

		sTextObject = "Q";//ET\r\n";

		AddContent(sTextObject);
		}
}	
//---------------------------------------------------------------------------
bool
CPDFObject::AddText(std::string psText, RectF pcTextBox) {

	std::string sTextObject;
	char cNumber[30];
	float fHorizontalScaling;
	float fTextBoxLengthStandardSpacing;

	if (!bTextObjectOpen) StartText();

	fTextBoxLengthStandardSpacing = psText.length() * ((pcTextBox.Height / 0.9f) * 0.5f);
	fHorizontalScaling = 100 * (pcTextBox.Width / fTextBoxLengthStandardSpacing);

	sTextObject = "BT\r\n/F1 ";
	sprintf_s(cNumber, sizeof(cNumber), "%0.1f", pcTextBox.Height);
	sTextObject += cNumber;
	sTextObject += " Tf\r\n"; //Font type and size
	sprintf_s(cNumber, sizeof(cNumber), "%0.2f", pcTextBox.X);
	sTextObject += cNumber;
	sTextObject += " ";
	sprintf_s(cNumber, sizeof(cNumber), "%0.2f", pcTextBox.Y);
	sTextObject += cNumber;
	sTextObject += " Td\r\n";	//Positioning of the text
	sprintf_s(cNumber, sizeof(cNumber), "%0.1f", fHorizontalScaling);
	sTextObject += cNumber;
	sTextObject += " Tz\r\n";	//Horizontal scaling
	sTextObject += " 3 Tr\r\n[("; //Do not render the characters (3)
	sTextObject += psText;
	sTextObject += ")] TJ\r\nET\r\n"; //The text array
	//sTextObject += " 10 10 m\r\n(abc) Tj\r\n";

	return AddContent(sTextObject);
	}
//---------------------------------------------------------------------------
bool
CPDFObject::AddContent(std::string psOperation) {

	std::string sOperation;

	sOperation = "";

	if (oMemoryObject!=NULL) {

		sOperation = (char*)oMemoryObject;
        free(oMemoryObject);
		}

	sOperation += psOperation;
	
    //create a new (empty) memory stream
	lMemoryObjectSize = sOperation.length() + 1;
	oMemoryObject = malloc(lMemoryObjectSize);

    memcpy_s(oMemoryObject, lMemoryObjectSize, sOperation.c_str(), lMemoryObjectSize);
	lMemoryObjectSize--;

	return true;
	}

//---------------------------------------------------------------------------
void
 CPDFObject::AddTextStream(std::string psText) {

    void* oTempObject;
    unsigned long lCompressedSize;

    //remove old memory stream
    if (oMemoryObject!=NULL) {
        free(oMemoryObject);
        }

    //create a new (empty) memory stream

    //create temporary stream in which compressed data will be written
    oTempObject = malloc((int)(psText.length() * 1.1 + 20));

    //compress the data
    compress((unsigned char*)oTempObject, &lCompressedSize, (unsigned char*)psText.c_str(), (long)psText.length());
	
ZEXTERN int ZEXPORT uncompress OF((Bytef *dest,   uLongf *destLen,
                                   const Bytef *source, uLong sourceLen));


    oMemoryObject = malloc(lCompressedSize);
	lMemoryObjectSize = lCompressedSize;
    memcpy_s(oMemoryObject, lMemoryObjectSize, oTempObject, lCompressedSize);

    free (oTempObject);
    }
//---------------------------------------------------------------------------
void
 CPDFObject::AddCreator(std::string psCreator) {

    sCreator = psCreator;
    }
//---------------------------------------------------------------------------
void
 CPDFObject::AddProducer(std::string psProducer) {

    sProducer = psProducer;
    }
//---------------------------------------------------------------------------
void
 CPDFObject::AddCreationDate(std::string psDate) {

    sCreationDate = psDate;
    }
//---------------------------------------------------------------------------
bool
CPDFObject::MakeZLibImage(Bitmap* poBitmap) {

	void* oInMemoryObject;
	void* oOutMemoryObject;
    long lScanLineSize;
	long lInMemoryObjectSize;
    unsigned long lCompressedSize;
    int iResult;
	bool bResult;

	bResult = true;

	//determine the size of the temporary memory objects
    lScanLineSize = ((((poBitmap->GetWidth() * lColorSpace) + 7) & ~7) / 8);
	//lScanLineSize = 108;
	lInMemoryObjectSize = lScanLineSize * poBitmap->GetHeight();

	oInMemoryObject = malloc(lInMemoryObjectSize);
	oOutMemoryObject = malloc((long)(lInMemoryObjectSize * 1.1 + 20));

	//copy the image to the memory object
	Rect rect(0, 0, poBitmap->GetWidth(), poBitmap->GetHeight());
	BitmapData* bitmapData = new BitmapData;
	poBitmap->LockBits(&rect, ImageLockModeRead, PixelFormat1bppIndexed, bitmapData);

	UINT8* oInPointer;
	UINT8* oSourcePointer;
	
	for (long lIndex = 0; lIndex<(long)poBitmap->GetHeight(); lIndex++) {

		oInPointer = (UINT8*)oInMemoryObject + min(abs(bitmapData->Stride), lScanLineSize) * lIndex;
		oSourcePointer = (UINT8*)bitmapData->Scan0 + bitmapData->Stride * lIndex;

		memcpy_s(oInPointer, lScanLineSize, oSourcePointer, lScanLineSize);
		}
				
	delete bitmapData;

	//compress the data
	lCompressedSize =  lInMemoryObjectSize;
    iResult = compress((unsigned char*)oOutMemoryObject, &lCompressedSize, (unsigned char*)oInMemoryObject, lInMemoryObjectSize);

    switch (iResult) {

        case Z_MEM_ERROR: {

            MessageBox(NULL, L"Not enough memory", L"Error", MB_OK);
			bResult = false;
            break;
            }

        case Z_BUF_ERROR: {

			realloc(oOutMemoryObject, (lInMemoryObjectSize * 2 + 5000));
            lCompressedSize =  lInMemoryObjectSize * 2 + 5000;

			iResult = compress((unsigned char*)oOutMemoryObject, &lCompressedSize, (unsigned char*)oInMemoryObject, lInMemoryObjectSize);

            if (iResult == Z_BUF_ERROR) {
                MessageBox(NULL, L"Not enough room in the output buffer", L"Error", MB_OK);
				bResult = false;
				}

            break;
            }

        case Z_STREAM_ERROR: {

            MessageBox(NULL, L"Level parameter is invalid", L"Error", MB_OK);
			bResult = false;
            break;
            }
        }

	//Copy the data to the memory object
    if (iResult == Z_OK) {

		if (oMemoryObject!=NULL) free(oMemoryObject);
		oMemoryObject = malloc(lCompressedSize);
		bResult = (oMemoryObject!=NULL);
		if (bResult) {
			lMemoryObjectSize = lCompressedSize;
			memcpy_s(oMemoryObject, lCompressedSize, oOutMemoryObject, lCompressedSize);
			}
		}

	free(oInMemoryObject);
	free(oOutMemoryObject);
        
	return bResult;
}

//This function creates a JPEG in a temporary file and reads this
//temporary file into a memory block
bool 
CPDFObject::MakeJPEGImage(Bitmap* poBitmap) {

	bool bResult;
    wchar_t szTempName[MAX_PATH];
    wchar_t lpPathBuffer[MAX_PATH];

	bResult = true;

	// Get the temp path

    if (GetTempPath(MAX_PATH,					// length of the buffer
         lpPathBuffer)==0) return false;		// buffer for path 

    // Create a temporary file. 
    
    if (GetTempFileName(lpPathBuffer,			// directory for temp files 
        L"copymach_",							// temp file name prefix 
        0,										// create unique name 
        szTempName)==0) return false;           // buffer for name 

	// Create a temporary jpeg file
	CLSID TiffClsid;

	ULONG	compressionValue;

	EncoderParameters* pEncoderParameters = (EncoderParameters*)
		malloc(sizeof(EncoderParameters) + 2 * sizeof(EncoderParameter));

	// An EncoderParameters object has an array of
	// EncoderParameter objects. In this case, there is only
	// one EncoderParameter object in the array.
	pEncoderParameters->Count = 1;

	// Initialize the one EncoderParameter object.
	pEncoderParameters->Parameter[0].Guid = EncoderQuality;
	pEncoderParameters->Parameter[0].Type = EncoderParameterValueTypeLong;
	pEncoderParameters->Parameter[0].NumberOfValues = 1;
	pEncoderParameters->Parameter[0].Value = &compressionValue;

	GetEncoderClsid(L"image/jpeg", &TiffClsid);

	compressionValue = lJPEGCompression;

	bResult = (poBitmap->Save(szTempName, &TiffClsid, pEncoderParameters)==Ok);

	free(pEncoderParameters);

	//copy the contents of the temp file to the memoryblock
	FILE* FileHandle;
	long lFileSize;

	_wfopen_s(&FileHandle, szTempName, L"rb");

	if (FileHandle!=NULL) {

		//determine the size
		fseek(FileHandle, 0, SEEK_END);
		lFileSize = ftell(FileHandle);
		fseek(FileHandle, 0, SEEK_SET);

		bResult = (lFileSize>0);

		if (bResult) {
			//adapt the memoryobject
			if (oMemoryObject!=NULL) {
				free(oMemoryObject);
				}
			
			oMemoryObject = malloc(lFileSize);
			lMemoryObjectSize = lFileSize;

			//read the data and put it in memory
			bResult = (fread_s(oMemoryObject, lMemoryObjectSize, 1, lFileSize, FileHandle)==lFileSize);
			}

		fclose (FileHandle);
		}
	else {

		bResult = false;
		}

	//delete the tempfile
	DeleteFile(szTempName);

	//return the result
	return bResult;
	}

int 
CPDFObject::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
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

//This function write the given line to the file
void 
CPDFObject::Write(ofstream* poFile, std::string psLine) {

	poFile->write(psLine.c_str(), psLine.length());
}

