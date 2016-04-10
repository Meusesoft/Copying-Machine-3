//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TOcrRectangle.h"
#include "TError.h"
#include "TDataLayer.h"

//---------------------------------------------------------------------------
TDataLayer::TDataLayer(string psXMLShapeNetworksFile): TOcrThread(true /*create suspended*/) {

    oError = TError::CreateInstance();

	sXMLShapeNetworksFile = psXMLShapeNetworksFile;
    }
//---------------------------------------------------------------------------

TDataLayer::~TDataLayer() {

    Terminate();
	WaitFor();	
	
	for (long lIndex=0; lIndex<(long)oShapes.size(); lIndex++) {

        delete oShapes[lIndex].oNeuralNetwork;
        }
    oShapes.clear();

	for (long lIndex=0; lIndex<(long)oShapeNetworks.size(); lIndex++) {

        delete oShapeNetworks[lIndex].oShapeNeuralNetwork;
        }

    oShapes.clear();

	TError::DeleteInstance();
    }
//---------------------------------------------------------------------------
void __fastcall 
TDataLayer::Execute()
{
    int iReturnValue;

    iReturnValue = 0;

    try {

        //Load files
		LoadShapes(sXMLShapeNetworksFile);
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    ReturnValue = iReturnValue;
}
//---------------------------------------------------------------------------
void
__fastcall TDataLayer::LoadData() {

    cShape oCharacterVector;
    Point cPoint;

    long lId = 0;

    TiXmlElement* CharactersElement;
    TiXmlElement* CharacterElement = 0;
    TiXmlElement* ShapeElement = 0;
    TiXmlElement* PointElement = 0;

    TiXmlNode* CollectionNode;
    TiXmlNode* CharactersNode;

	std::string sCompareGrid;
    std::string sSubString;
    std::string sFeatureSpaceX;
    std::string sFeatureSpaceY;
    std::string sPixelCountX;
    std::string sPixelCountY;
	char cTempString[1000];

    char cCharBuffer[10];
    int iCharNumber;

    TiXmlDocument doc("D:\\Projects\\CopyingMachine3\\debug\\characters.xml");

    //remove current data from memory
    for (int i=0; i<(int)oShapes.size(); i++) {
        oShapes[i].oPoints.clear();
        }
    oShapes.clear();

    //laadt date
    doc.LoadFile();

    CollectionNode = doc.FirstChild("OcrCollection");

    while (CollectionNode!=NULL) {

        CharactersNode = CollectionNode->FirstChild("Characters");

        while (CharactersNode!=NULL) {

            CharactersElement = CharactersNode->ToElement();

            strcpy_s(cCharBuffer, sizeof(cCharBuffer), CharactersElement->Attribute("value"));
            iCharNumber = atoi(cCharBuffer);
            cCharBuffer[0] = (char)iCharNumber;
            cCharBuffer[1] = 0x00;

            //CharacterElement = CharactersNode->FirstChildElement("Character");
            CharacterElement = CharactersNode->FirstChildElement();

            while (CharacterElement !=NULL) {

                strcpy_s(cCharBuffer, sizeof(cCharBuffer), CharacterElement->Attribute("id"));
                oCharacterVector.lId = atoi(cCharBuffer);

                oCharacterVector.lId = lId;
                lId++;

                strcpy_s(cCharBuffer, sizeof(cCharBuffer), CharacterElement->Attribute("value"));
                iCharNumber = atoi(cCharBuffer);
                oCharacterVector.cCharacter[0] = (char)iCharNumber;
                oCharacterVector.cCharacter[1] = 0x00;

				strcpy_s(cTempString, sizeof(cTempString), CharacterElement->Attribute("sFont")); 
                oCharacterVector.sFont = cTempString;
				strcpy_s(cTempString, sizeof(cTempString), CharacterElement->Attribute("sDirectory")); 
				oCharacterVector.sDirectory = cTempString;
				strcpy_s(cTempString, sizeof(cTempString), CharacterElement->Attribute("sTitle")); 
                oCharacterVector.sTitle = oCharacterVector.cCharacter;
                oCharacterVector.sTitle += cTempString;

                ShapeElement = CharacterElement->FirstChildElement("Shape");

                if (ShapeElement!=NULL) {

                    PointElement = ShapeElement->FirstChildElement("Point");

                    while (PointElement!=NULL) {

                        cPoint.X = atoi(PointElement->Attribute("x"));
                        cPoint.Y = atoi(PointElement->Attribute("y"));

                        oCharacterVector.oPoints.push_back(cPoint);

                        PointElement = PointElement->NextSiblingElement();
                        }
                    }

                oCharacterVector.lHoles = atoi(CharacterElement->Attribute("iHoles"));
                oCharacterVector.lRatio = atoi(CharacterElement->Attribute("lCompareRatio"));
                oCharacterVector.lAveragePixelDensity = atoi(CharacterElement->Attribute("lAveragePixelDensity"));
                oCharacterVector.lDeviationPixelDensity = atoi(CharacterElement->Attribute("lDeviationPixelDensity"));
                oCharacterVector.bTopLine = (atoi(CharacterElement->Attribute("bTopLine"))==0 ? false : true);
                oCharacterVector.bMiddleLine = (atoi(CharacterElement->Attribute("bMiddleLine"))==0 ? false : true);;
                oCharacterVector.bBaseLine = (atoi(CharacterElement->Attribute("bBaseLine"))==0 ? false : true);;
                oCharacterVector.bBottomLine = (atoi(CharacterElement->Attribute("bBottomLine"))==0 ? false : true);;
                oCharacterVector.bCenterLine = (atoi(CharacterElement->Attribute("bCenterLine"))==0 ? false : true);;

                oCharacterVector.lPointMassX = atoi(CharacterElement->Attribute("lPointMassX"));
                oCharacterVector.lPointMassY = atoi(CharacterElement->Attribute("lPointMassY"));
                oCharacterVector.lDeviationPointMassX = atoi(CharacterElement->Attribute("lDeviationPointMassX"));
                oCharacterVector.lDeviationPointMassY = atoi(CharacterElement->Attribute("lDeviationPointMassY"));

                //read compare grid

                sCompareGrid = CharacterElement->Attribute("bCompareGrid");

                for (int lIndex=0; lIndex<(32*32); lIndex++) {

                    oCharacterVector.bCompareGrid[lIndex] = (BYTE)EncodeFromCharacter(sCompareGrid[lIndex+1]);
                    }

                //read feature space
                ::ZeroMemory(&oCharacterVector.lPixelCountForeX, 32 * sizeof(long));
                ::ZeroMemory(&oCharacterVector.lPixelCountForeY, 32 * sizeof(long));
                ::ZeroMemory(&oCharacterVector.lPixelCountBackX, 32 * sizeof(long));
                ::ZeroMemory(&oCharacterVector.lPixelCountBackY, 32 * sizeof(long));
                ::ZeroMemory(&oCharacterVector.lPixelFeatureBackX, 5 * sizeof(long));
                ::ZeroMemory(&oCharacterVector.lPixelFeatureBackY, 5 * sizeof(long));
                ::ZeroMemory(&oCharacterVector.lPixelFeatureForeX, 5 * sizeof(long));
                ::ZeroMemory(&oCharacterVector.lPixelFeatureForeY, 5 * sizeof(long));
                ::ZeroMemory(&oCharacterVector.lPixelDirectionX, 40 * sizeof(long));
                ::ZeroMemory(&oCharacterVector.lPixelDirectionY, 40 * sizeof(long));

                sFeatureSpaceX = CharacterElement->Attribute("lPixelFeatureForeX");
                sFeatureSpaceY = CharacterElement->Attribute("lPixelFeatureForeY");

                for (int lIndex=0; lIndex<5; lIndex++) {

                    sSubString = sFeatureSpaceX[lIndex];
                    oCharacterVector.lPixelFeatureForeX[lIndex] = sSubString[0] - 65;

                    sSubString = sFeatureSpaceY[lIndex];
                    oCharacterVector.lPixelFeatureForeY[lIndex] = sSubString[0] - 65;
                    }

                //read feature space

                sFeatureSpaceX = CharacterElement->Attribute("lPixelFeatureBackX");
                sFeatureSpaceY = CharacterElement->Attribute("lPixelFeatureBackY");

                for (int lIndex=0; lIndex<5; lIndex++) {

                    sSubString = sFeatureSpaceX[lIndex];
                    oCharacterVector.lPixelFeatureBackX[lIndex] = sSubString[0] - 65;

                    sSubString = sFeatureSpaceY[lIndex];
                    oCharacterVector.lPixelFeatureBackY[lIndex] = sSubString[0] - 65;
                    }

                //read pixel count

                sPixelCountX = CharacterElement->Attribute("lPixelCountForeX");
                sPixelCountY = CharacterElement->Attribute("lPixelCountForeY");

                if (sPixelCountX.size()>0) {

                for (int lIndex=0; lIndex<32; lIndex++) {

                    sSubString = sPixelCountX[lIndex];
                    oCharacterVector.lPixelCountForeX[lIndex] = sSubString[0] - 65;

                    sSubString = sPixelCountY[lIndex];
                    oCharacterVector.lPixelCountForeY[lIndex] = sSubString[0] - 65;
                    }
                }

                //read feature direction space

                sFeatureSpaceX = CharacterElement->Attribute("lPixelDirectionX");
                sFeatureSpaceY = CharacterElement->Attribute("lPixelDirectionY");

                if (sFeatureSpaceY.size()>0) {
                for (int lIndex=0; lIndex<20; lIndex++) {

                    sSubString = sFeatureSpaceX[lIndex];
                    oCharacterVector.lPixelDirectionX[lIndex] = sSubString[0] - 65;

                    sSubString = sFeatureSpaceY[lIndex];
                    oCharacterVector.lPixelDirectionY[lIndex] = sSubString[0] - 65;
                    }
                }

                //read pixel count
                sPixelCountX = CharacterElement->Attribute("lPixelCountBackX");
                sPixelCountY = CharacterElement->Attribute("lPixelCountBackY");

                if (sPixelCountX.size()>0) {
                for (int lIndex=0; lIndex<32; lIndex++) {

                    sSubString = sPixelCountX[lIndex];
                    oCharacterVector.lPixelCountBackX[lIndex] = sSubString[0] - 65;

                    sSubString = sPixelCountY[lIndex];
                    oCharacterVector.lPixelCountBackY[lIndex] = sSubString[0] - 65;
                    }
                }

                oCharacterVector.oNeuralNetwork = NULL;

                //Neural Network ophalen
                TiXmlNode* oNeuralNetworkNode;

                oNeuralNetworkNode = CharacterElement->FirstChild("NeuralNetwork");

                if (oNeuralNetworkNode!=NULL) {

                    oCharacterVector.oNeuralNetwork = new TNeuralNetwork();
                    oCharacterVector.oNeuralNetwork->LoadXML(oNeuralNetworkNode);
                    }


                oShapes.push_back(oCharacterVector);

                CharacterElement = CharacterElement->NextSiblingElement();
                }
            CharactersNode = CharactersNode->NextSibling("Characters");
            }
        CollectionNode = CollectionNode->NextSibling();
        }

    }

//---------------------------------------------------------------------------
char
__fastcall TDataLayer::EncodeToCharacter(long plValue) {

	std::string sEncode;
    char cReturn;

    sEncode = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.,;:";

    if (plValue<0) plValue = 0;
    if (plValue>63) plValue = 63;

    cReturn = sEncode[plValue+1];

    return cReturn;
    }
//---------------------------------------------------------------------------
long
__fastcall TDataLayer::EncodeFromCharacter(char pcCharacter) {

    //this function is extremly slow and is used a lot (1024 times per character)
    //must be optimized!!!

	std::string sEncode;
    bool bFound;
    long lReturn;

    sEncode = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.,;:";

    lReturn=0;
    bFound = false;

    do {

        if (sEncode[lReturn+1] == pcCharacter) {

            bFound = true;
            lReturn--;
            }

        lReturn++;
        } while (!bFound && lReturn!=64);

    return lReturn;
    }

//---------------------------------------------------------------------------
void
__fastcall TDataLayer::LoadShapes(string psXMLFile) {

    cShapeNet oShapeNet;

    TiXmlNode* ShapeNetsNode;
    TiXmlNode* ShapeNetNode;
    TiXmlNode* ShapeListNode;
    TiXmlNode* ShapeListEntryNode;
    TiXmlNode* ShapeNeuralNetNode;

    TiXmlElement* ShapeNetElement;
    TiXmlElement* ShapeListEntryElement;

    TiXmlAttribute* oAttribute;

    cShapeNetList oListEntry;
    long lId;

    //verwijder huidige data uit geheugen
    for (long lIndex=0; lIndex<(long)oShapeNetworks.size(); lIndex++) {

        oShapeNetworks[lIndex].oShapeList.clear();
        delete oShapeNetworks[lIndex].oShapeNeuralNetwork;
        }
    oShapeNetworks.clear();


    //load xml file
    TiXmlDocument doc(psXMLFile.c_str()); 

    doc.LoadFile();

    ShapeNetsNode = doc.FirstChild("ShapeNets");

    while (ShapeNetsNode!=NULL) {

        ShapeNetNode = ShapeNetsNode->FirstChild("ShapeNet");

        while (ShapeNetNode!=NULL) {

            ShapeNetElement = ShapeNetNode->ToElement();

            oAttribute = ShapeNetElement->FirstAttribute();

            oShapeNet.sName = "";
            oShapeNet.sNumberExamples = "30";
            oShapeNet.bSearchSubFolders = false;

            while (oAttribute!=NULL) {

                if (strcmp(oAttribute->Name(), "Name")==0) {
                    oShapeNet.sName = oAttribute->Value();
                    }
                if (strcmp(oAttribute->Name(), "lNumberExamples")==0) {
                    oShapeNet.sNumberExamples = oAttribute->Value();
                    }
                if (strcmp(oAttribute->Name(), "bSearchSubFolders")==0) {
                    oShapeNet.bSearchSubFolders = (atoi(oAttribute->Value())==1);
                    }

                oAttribute = oAttribute->Next();
                }

            lId = oShapeNetworks.size();
            oShapeNetworks.push_back(oShapeNet);

            ShapeListNode = ShapeNetNode->FirstChild("ShapeList");

            while (ShapeListNode!=NULL) {

                oShapeNet.oShapeList.clear();

                ShapeListEntryNode = ShapeListNode->FirstChild("ShapeListEntry");

                while (ShapeListEntryNode!=NULL) {

                    ShapeListEntryElement = ShapeListEntryNode->ToElement();

                    oListEntry.sShape = StringToWString(ShapeListEntryElement->Attribute("Shape"));
                    oListEntry.sSampleFolder  = StringToWString(ShapeListEntryElement->Attribute("SampleFolder"));
                    oListEntry.lShapeId  = atoi(ShapeListEntryElement->Attribute("ShapeId"));
                    //oListEntry.lShapeId = -1;
                    
                    oShapeNetworks[lId].oShapeList.push_back(oListEntry);

                    ShapeListEntryNode = ShapeListEntryNode->NextSiblingElement();
                    }

                ShapeListNode = ShapeListNode->NextSibling();
                }

            //load neural network if exists
            ShapeNeuralNetNode = ShapeNetNode->FirstChild("NeuralNetwork");

            if (ShapeNeuralNetNode!=NULL) {

                oShapeNetworks[lId].oShapeNeuralNetwork = new TNeuralNetwork();
                oShapeNetworks[lId].oShapeNeuralNetwork->LoadXML(ShapeNeuralNetNode);
                }


            ShapeNetNode = ShapeNetNode->NextSibling();
            }

        ShapeNetsNode = ShapeNetsNode->NextSibling();
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TDataLayer::SaveShapes(string psXMLFile) {

    TiXmlNode* ShapeNetsNode;
    TiXmlNode* ShapeNetNode;
    TiXmlNode* ShapeListNode;

    TiXmlElement* ShapeNetsElement;
    TiXmlElement* ShapeNetElement;
    TiXmlElement* ShapeListElement;
    TiXmlElement* ShapeListEntryElement;

    cShapeNetList oListEntry;

    TiXmlDocument doc(psXMLFile.c_str());

    ShapeNetsElement = new TiXmlElement("ShapeNets");
    ShapeNetsNode = doc.InsertEndChild(*ShapeNetsElement);

    for (long lIndex=0; lIndex<(long)oShapeNetworks.size(); lIndex++) {

        ShapeNetElement = new TiXmlElement("ShapeNet");

        ShapeNetElement->SetAttribute("Name", oShapeNetworks[lIndex].sName.c_str());
        ShapeNetElement->SetAttribute("lNumberExamples", oShapeNetworks[lIndex].sNumberExamples.c_str());
		ShapeNetElement->SetAttribute("bSearchSubFolders", oShapeNetworks[lIndex].bSearchSubFolders ? 1 : 0);

        ShapeNetNode = ShapeNetsNode->InsertEndChild(*ShapeNetElement);

        ShapeListElement = new TiXmlElement("ShapeList");

        ShapeListNode = ShapeNetNode->InsertEndChild(*ShapeListElement);


        for (long lIndex2=0; lIndex2<(long)oShapeNetworks[lIndex].oShapeList.size(); lIndex2++) {

            ShapeListEntryElement = new TiXmlElement("ShapeListEntry");

            ShapeListEntryElement->SetAttribute("Shape", WStringToString(oShapeNetworks[lIndex].oShapeList[lIndex2].sShape).c_str());
            ShapeListEntryElement->SetAttribute("SampleFolder", WStringToString(oShapeNetworks[lIndex].oShapeList[lIndex2].sSampleFolder).c_str());
            ShapeListEntryElement->SetAttribute("ShapeId", oShapeNetworks[lIndex].oShapeList[lIndex2].lShapeId);

          ShapeListNode->InsertEndChild(*ShapeListEntryElement);
            }

        //save neural net
        if (oShapeNetworks[lIndex].oShapeNeuralNetwork!=NULL) {
            oShapeNetworks[lIndex].oShapeNeuralNetwork->SaveXML(ShapeNetNode);
            }
        }
    doc.SaveFile();
    }
//---------------------------------------------------------------------------
void
__fastcall TDataLayer::RecognizeBitmapOnShapeNet(cRecognition &oRecognition) {

    cRecognitionScore oScore;
    sNeuralInput oInput;
    sNeuralOutput oOutput;


    //Extract the features from the bitmap
    ExtractFeaturesFromBitmap(oRecognition);

    //put the features through the neural network
    oInput.fInputs.clear();
    oOutput.fOutputs.clear();

    //fill the input nodes of the neural network(s)
    for (long lIndex2=0; lIndex2<5; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lPixelProjectionBackX[lIndex2]);
        }
    for (long lIndex2=0; lIndex2<5; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lPixelProjectionBackY[lIndex2]);
        }
    for (long lIndex2=0; lIndex2<5; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lPixelProjectionForeX[lIndex2]);
        }
    for (long lIndex2=0; lIndex2<5; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lPixelProjectionForeY[lIndex2]);
        }

    for (long lIndex2=0; lIndex2<20; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lStrokeDirectionX[lIndex2]);
        }
    for (long lIndex2=0; lIndex2<20; lIndex2++) {

        oInput.fInputs.push_back((float)oRecognition.lStrokeDirectionY[lIndex2]);
        }


    //do the math en compute the output
    //1. compute the font (1, 2, 3....)
    oShapeNetworks[0].oShapeNeuralNetwork->ComputeOutput(oInput, oOutput);

    long lFont;
    float fMax;

    fMax=0;
    lFont=0;

    for (long lIndex2=0; lIndex2<(long)oOutput.fOutputs.size(); lIndex2++) {

        if (fMax<oOutput.fOutputs[lIndex2]) {
            lFont = lIndex2;
            fMax = oOutput.fOutputs[lIndex2];
            }
        }

    oOutput.fOutputs.clear();

    lFont = 1;

	do {

		//compute the character based on the font
		oShapeNetworks[lFont].oShapeNeuralNetwork->ComputeOutput(oInput, oOutput);

		ProcessRecognitionScores(oRecognition, oOutput, oShapeNetworks[lFont]);

		lFont++;
		} while (lFont<(long)oShapeNetworks.size() && oRecognition.oScores[0].fScore<0.85);
	}

void 
__fastcall TDataLayer::ProcessRecognitionScores(cRecognition &oRecognition, 
												sNeuralOutput &oOutput,
												cShapeNet &oShapeNet) {

     cRecognitionScore oScore;
    bool bAdded;

	 //sort and place the results in the recognition structure
    for (long lIndex=0; lIndex<(long)oOutput.fOutputs.size(); lIndex++) {

        oScore.fScore = oOutput.fOutputs[lIndex];

		std::wstring widestring(oShapeNet.oShapeList[lIndex].sShape.begin(), oShapeNet.oShapeList[lIndex].sShape.end());
		oScore.sTitle = widestring;

		std::copy(oShapeNet.oShapeList[lIndex].sShape.begin(), oShapeNet.oShapeList[lIndex].sShape.end(), oScore.sTitle.begin());

		oScore.lId = lIndex;

        bAdded = false;

        for (long lIndex2=0; lIndex2<(long)oRecognition.oScores.size() && lIndex2<5 && !bAdded; lIndex2++) {

            if (oScore.fScore > oRecognition.oScores[lIndex2].fScore) {

                oRecognition.oScores.insert(oRecognition.oScores.begin() + lIndex2, oScore);
                bAdded = true;
                }
            }

        if (!bAdded && oRecognition.oScores.size()<5) {
            oRecognition.oScores.push_back(oScore);
            }

		while (oRecognition.oScores.size()>=5) {
            oRecognition.oScores.erase(oRecognition.oScores.end()-1);
            }
        }
}

//---------------------------------------------------------------------------
void
__fastcall TDataLayer::RecognizeBitmap(cRecognition &oRecognition) {

    ExtractFeaturesFromBitmap(oRecognition);

    ScoreExtractedFeature(2, oRecognition);
    ScoreExtractedFeature(1, oRecognition);
    ScoreExtractedFeature(3, oRecognition);

    OrderRecognitionByNeuralScore(oRecognition);
    }
//---------------------------------------------------------------------------
void
__fastcall TDataLayer::ScoreExtractedFeature(long lFeature, cRecognition &oRecognition) {

    //this function scores the given bitmap on the selected feature. If it
    //is the first scoring process on the bitmap than all existing patterns
    //are compared. If it is not the first process then the top X patterns of
    //previous processes are used to compare with.

    //assumed is that the features are already extract

    // lFeature:    1 = template matching
    //              2 = pixel projection
    //              3 = stroke direction

    long lSize;
    long lScore;
    long lInsertPosition;
    bool bFound;
    vector <cRecognitionScore> oScores;
    cRecognitionScore oScore;


    if (oRecognition.oScores.size() > 0) {

        lSize = oRecognition.oScores.size();
        oScores.clear();

        for (long lIndex=0; lIndex<lSize; lIndex++) {

            oScore = oRecognition.oScores[lIndex];

            lScore = CalculateScore(lFeature, oScore ,oRecognition);
            oScore.lCurrentScore = lScore;

            //search for position to insert score into oScores vector
            //the lower the score, the higher the score will be
            //in the order.
            lInsertPosition = oScores.size();
            bFound = false;
            for (long lIndex2=0; lIndex2<(long)oScores.size() && !bFound; lIndex2++) {

                if (oScores[lIndex2].lCurrentScore > lScore) {

                    lInsertPosition = lIndex2;
                    bFound = true;
                    }
                }
            oScores.insert(oScores.begin() + lInsertPosition, oScore);
            }
        }
    else {

        for (long lIndex=0; lIndex<(long)oShapes.size(); lIndex++) {

            ::ZeroMemory(&oScore, sizeof(oScore));

            oScore.lId = lIndex;

            lScore = CalculateScore(lFeature, oScore ,oRecognition);
            oScore.lCurrentScore = lScore;
			std::wstring widestring(oShapes[lIndex].sTitle.begin(), oShapes[lIndex].sTitle.end());
			oScore.sTitle = widestring;
			
            //search for position to insert score into oScores vector
            //the lower the score, the higher the score will be
            //in the order.
            lInsertPosition = oScores.size();
            bFound = false;
            for (long lIndex2=0; lIndex2<(long)oScores.size() && !bFound && lIndex2<25; lIndex2++) {

                if (oScores[lIndex2].lCurrentScore > lScore) {

                    lInsertPosition = lIndex2;
                    bFound = true;
                    }
                }

            //the collection is limited to 25 entries for performance
            if (lInsertPosition<25) {

                oScores.insert(oScores.begin() + lInsertPosition, oScore);
                }
            }

        lSize = oScores.size();
        if (lSize>25) {
            lSize = 25;
            }
        }


    //determine position of score in the selected feature
    oRecognition.oScores.clear();

    for (long lIndex=0; lIndex<lSize; lIndex++) {

        switch (lFeature) {

            case 1: {

                oScores[lIndex].lTemplatePosition = lIndex;
                break;
                }

            case 2: {

                oScores[lIndex].lProjectionPosition = lIndex;
                break;
                }

            case 3: {

                oScores[lIndex].lDirectionPosition = lIndex;
                break;
                }
            }

        oRecognition.oScores.push_back(oScores[lIndex]);
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TDataLayer::ExtractFeaturesFromBitmap(cRecognition &oRecognition) {

    //this function extracts features from the giving bitmap in the cRecognition
    //structure. This bitmap should already be thinned and 'stroked'.

    long lPixelCountBackX[32];
    long lPixelCountBackY[32];
    long lPixelCountForeX[32];
    long lPixelCountForeY[32];

    long lPointer;
    BYTE bPixel;
    long lAdjustedX, lAdjustedY;

    //clear the variable spaces
    ::SecureZeroMemory(lPixelCountBackX, sizeof(lPixelCountBackX));
    ::SecureZeroMemory(lPixelCountBackY, sizeof(lPixelCountBackY));
    ::SecureZeroMemory(lPixelCountForeX, sizeof(lPixelCountForeX));
    ::SecureZeroMemory(lPixelCountForeY, sizeof(lPixelCountForeY));
    ::SecureZeroMemory(oRecognition.lStrokeDirectionX, sizeof(oRecognition.lStrokeDirectionX));
    ::SecureZeroMemory(oRecognition.lStrokeDirectionY, sizeof(oRecognition.lStrokeDirectionY));

    //Start extracting the features. We count the pixel per row and column, and we
    //count the number of directions per combined column/row (6 wide).
    for (long lY=0; lY<32; lY++) {

        lPointer = 32 * lY;

        for (long lX=0; lX<32; lX++) {

            bPixel = oRecognition.cCompareBitmap[lPointer];

            if (bPixel >= 0x04) {

                //it is a black/foreground color pixel
                lPixelCountForeX[lX]++;
                lPixelCountForeY[lY]++;

                if (bPixel >= 0x0A) {

                    //the pixel has stoke direction information
                    lAdjustedX = lX;
                    lAdjustedY = lY;

                    if (lAdjustedX==0) lAdjustedX=1;
                    if (lAdjustedY==0) lAdjustedY=1;
                    if (lAdjustedX==31) lAdjustedX=30;
                    if (lAdjustedY==31) lAdjustedY=30;

                    oRecognition.lStrokeDirectionX[((lAdjustedX-1)/6)*4 + bPixel - 10]++;
                    oRecognition.lStrokeDirectionY[((lAdjustedY-1)/6)*4 + bPixel - 10]++;
                    }
                }

            if (bPixel == 0x00) {

                lPixelCountBackX[lX]++;
                lPixelCountBackY[lY]++;
                }

            lPointer++;
            }
        }

    //Do some after math on the projection feature. We combine 6 columns/rows into 1
    //by adding the seperate columns/rows so that we have 5 combined rows/columns left.
    //And we divide the the added numbers by 3 to keep the numbers a bit smaller.
    //Also we do not look at the outer pixels (x=0, x=31, y=0, y=31) to ignore
    //some irradic structures
     for (unsigned long lIndex1=0; lIndex1 < 5; lIndex1++) {

        oRecognition.lPixelProjectionForeX[lIndex1] = 0;
        oRecognition.lPixelProjectionForeY[lIndex1] = 0;

        oRecognition.lPixelProjectionBackX[lIndex1] = 0;
        oRecognition.lPixelProjectionBackY[lIndex1] = 0;

        for (unsigned long lIndex2=0; lIndex2 < 6; lIndex2++) {

            oRecognition.lPixelProjectionForeX[lIndex1] += (lPixelCountForeX[1 + lIndex1 * 6 + lIndex2]);
            oRecognition.lPixelProjectionForeY[lIndex1] += (lPixelCountForeY[1 + lIndex1 * 6 + lIndex2]);

            oRecognition.lPixelProjectionBackX[lIndex1] += (lPixelCountBackX[1 + lIndex1 * 6 + lIndex2]);
            oRecognition.lPixelProjectionBackY[lIndex1] += (lPixelCountBackY[1 + lIndex1 * 6 + lIndex2]);
            }

        oRecognition.lPixelProjectionForeX[lIndex1] = oRecognition.lPixelProjectionForeX[lIndex1] / 3;
        oRecognition.lPixelProjectionForeY[lIndex1] = oRecognition.lPixelProjectionForeY[lIndex1] / 3;

        oRecognition.lPixelProjectionBackX[lIndex1] = oRecognition.lPixelProjectionBackX[lIndex1] / 3;
        oRecognition.lPixelProjectionBackY[lIndex1] = oRecognition.lPixelProjectionBackY[lIndex1] / 3;
        }
    }
//---------------------------------------------------------------------------
long
__fastcall TDataLayer::CalculateScoreTemplate(long lPatternId, cRecognition &oRecognition) {

     //to calculate the score on the template, we match the fore/background
     //pixels of the thinned image with the pixels of the comparison (averaged)
     //grid. All differences are added into a single value (score).

    long lScore;
    long lPointer;
    long lDifference;
    long lPixelCount;
    BYTE bPixel;
    BYTE bPatternPixel;

    lScore = 0;
    lPixelCount = 0;

    for (long lY=0; lY<32; lY++) {

        lPointer = lY * 32;

        for (long lX=0; lX<32; lX++) {

            bPixel = oRecognition.cCompareBitmap[lPointer];

            if (bPixel == 0x00 || bPixel >= 0x04) {

                lPixelCount++;

                bPatternPixel = oShapes[lPatternId].bCompareGrid[lPointer];

                if (bPixel >= 0x0A) {
                    bPixel = 0x04;
                    }

                lDifference = abs((bPixel * 64) - (bPatternPixel*4));

                lScore += lDifference;
                }

            lPointer++;
            }
        }

    lScore = lScore / lPixelCount;

    return lScore;
    }
//---------------------------------------------------------------------------
long
__fastcall TDataLayer::CalculateScorePixelProjection(long lPatternId, cRecognition &oRecognition) {

     //to calculate the score on the pixel projection feature we use the
     //least square method. We combine the square difference of all the
     //4 values by adding them.

     long lScore = 0;

     for (unsigned long lIndex1=0; lIndex1 < 5; lIndex1++) {

        lScore += (long)pow((float)abs(oRecognition.lPixelProjectionForeX[lIndex1] - oShapes[lPatternId].lPixelFeatureForeX[lIndex1]), 2);
        lScore += (long)pow((float)abs(oRecognition.lPixelProjectionForeY[lIndex1] - oShapes[lPatternId].lPixelFeatureForeY[lIndex1]), 2);

        lScore += (long)pow((float)abs(oRecognition.lPixelProjectionBackX[lIndex1] - oShapes[lPatternId].lPixelFeatureBackX[lIndex1]), 2);
        lScore += (long)pow((float)abs(oRecognition.lPixelProjectionBackY[lIndex1] - oShapes[lPatternId].lPixelFeatureBackY[lIndex1]), 2);
        }

     return lScore;
    }
//---------------------------------------------------------------------------
long
__fastcall TDataLayer::CalculateScoreStrokeDirection(long lPatternId, cRecognition &oRecognition) {

     //to calculate the score on the stroke projection feature we use the
     //least square method. We combine the square difference of both values
     //by adding them.

     long lScore = 0;

     for (unsigned long lIndex1=0; lIndex1 < 20; lIndex1++) {

        lScore += (long)pow((float)abs(oRecognition.lStrokeDirectionX[lIndex1] - oShapes[lPatternId].lPixelDirectionX[lIndex1]), 2);
        lScore += (long)pow((float)abs(oRecognition.lStrokeDirectionY[lIndex1] - oShapes[lPatternId].lPixelDirectionY[lIndex1]), 2);
        }

     return lScore;
    }
//---------------------------------------------------------------------------
void
__fastcall TDataLayer::OrderRecognitionByVote(cRecognition &oRecognition) {

    bool bSwap;
    long lSize;
    long lScoreLeft;
    long lScoreRight;
    cRecognitionScore oScoreLeft;
    cRecognitionScore oScoreRight;

    lSize = oRecognition.oScores.size();

    do {

        bSwap = false;

        for (long lIndex=0; lIndex<lSize-1; lIndex++) {

            oScoreLeft = oRecognition.oScores[lIndex];
            oScoreRight= oRecognition.oScores[lIndex+1];

            lScoreLeft = oScoreLeft.lProjectionPosition + oScoreLeft.lDirectionPosition + oScoreLeft.lTemplatePosition;
            lScoreRight = oScoreRight.lProjectionPosition + oScoreRight.lDirectionPosition + oScoreRight.lTemplatePosition;

            if (lScoreLeft > lScoreRight) {

                oRecognition.oScores[lIndex] = oScoreRight;
                oRecognition.oScores[lIndex+1] = oScoreLeft;

                bSwap = true;
                }
            }
        } while (bSwap);

    }
//---------------------------------------------------------------------------
void
__fastcall TDataLayer::OrderRecognitionByNeuralScore(cRecognition &oRecognition) {

    bool bSwap;
    long lSize;
    long lId;
    TNeuralNetwork* oNeuralNetwork;
    sNeuralInput cInput;
    sNeuralOutput oOutput;
    cRecognitionScore oScoreLeft;
    cRecognitionScore oScoreRight;

    lSize = oRecognition.oScores.size();

    //first set the scores of the neural nets
    for (long lIndex=0; lIndex<lSize; lIndex++) {

        oRecognition.oScores[lIndex].fScore = 0;

        lId = oRecognition.oScores[lIndex].lId;

        if (oShapes[lId].oNeuralNetwork!=NULL) {

            cInput.fInputs.clear();

            cInput.fInputs.push_back((float)oRecognition.oScores[lIndex].lDirectionScore);
            cInput.fInputs.push_back((float)oRecognition.oScores[lIndex].lTemplateScore);
            cInput.fInputs.push_back((float)oRecognition.oScores[lIndex].lProjectionScore);
            cInput.fInputs.push_back((float)oRecognition.oScores[lIndex].lDirectionPosition);
            cInput.fInputs.push_back((float)oRecognition.oScores[lIndex].lTemplatePosition);
            cInput.fInputs.push_back((float)oRecognition.oScores[lIndex].lProjectionPosition);

            oNeuralNetwork = oShapes[lId].oNeuralNetwork;

            oNeuralNetwork->ComputeOutput(cInput, oOutput);

            oRecognition.oScores[lIndex].fScore = oOutput.fOutputs[0];
            }
        }

    //order the recognition vector by the scores of the neural networks

    do {

        bSwap = false;

        for (long lIndex=0; lIndex<lSize-1; lIndex++) {

            oScoreLeft = oRecognition.oScores[lIndex];
            oScoreRight= oRecognition.oScores[lIndex+1];

            if (oScoreLeft.fScore < oScoreRight.fScore) {

                oRecognition.oScores[lIndex] = oScoreRight;
                oRecognition.oScores[lIndex+1] = oScoreLeft;

                bSwap = true;
                }
            }
        } while (bSwap);
    }
//---------------------------------------------------------------------------
long
__fastcall TDataLayer::CalculateScore(long lFeature, cRecognitionScore &oScore, cRecognition &oRecognition) {

    long lScore;

    switch (lFeature) {

        case 1: {

            lScore = CalculateScoreTemplate(oScore.lId, oRecognition);

            oScore.lTemplateScore = lScore;
            break;
            }

        case 2: {

            lScore = CalculateScorePixelProjection(oScore.lId, oRecognition);

            oScore.lProjectionScore = lScore;
            break;
            }

        case 3: {

            lScore = CalculateScoreStrokeDirection(oScore.lId, oRecognition);

            oScore.lDirectionScore = lScore;
            break;
            }
        }

    return lScore;
    }
//---------------------------------------------------------------------------
BYTE*
__fastcall TDataLayer::CreateBitmapFromFile(std::wstring sFilename, Bitmap* poCompareBitmap,
                                            cCreateBitmapSettings &cSettings) {

   // BYTE* oCompareBitmap;
    BYTE bPoint;
    BYTE* cScanLine;
    BYTE* cCompareBitmap;
    long lDestinationPointer;
    int iCharacterY;
    int iCharacterX;
	BitmapData* oSourceBitmapData;
	Rect cSourceRect;

    cCompareBitmap = (BYTE*)malloc(32*32);

    if (sFilename.find(L"_32.bmp")==string::npos && sFilename.find(L"DefiningExamples")==string::npos) {

		oSourceBitmapData = new BitmapData();
		cSourceRect = Rect(0, 0, poCompareBitmap->GetWidth(), poCompareBitmap->GetHeight());

		//Lock the bits
		Status cStatus = poCompareBitmap->LockBits(&cSourceRect, ImageLockModeRead, poCompareBitmap->GetPixelFormat(), oSourceBitmapData);
		
		if (cStatus==Ok) {
			
			for (int y=0; y<32; y++) {

				iCharacterY = (int)((float)(poCompareBitmap->GetHeight()-2) / 32) * y;
				//cScanLine = (BYTE*)poCompareBitmap->ScanLine[iCharacterY+1];
				cScanLine = (BYTE*)((BYTE*)oSourceBitmapData->Scan0 + (oSourceBitmapData->Stride * iCharacterY));

				for (int x=0; x<32; x++) {

					iCharacterX = (int)((float)(poCompareBitmap->GetWidth()-2) / 32) * x;

					bPoint = cScanLine[iCharacterX+1];

					if (cSettings.bMakeSkeleton) {

						if (bPoint!=4 && bPoint!=0x00) {
							bPoint=2;
							}
						}
					lDestinationPointer = x + y*32;

					cCompareBitmap[lDestinationPointer] = bPoint;
					}
				}
			poCompareBitmap->UnlockBits(oSourceBitmapData);
            }

        /*if (!oGrayValues->Checked) {
            AddDistanceToCompareGrid(cCompareBitmap);
            } */
        }
    else {

		oSourceBitmapData = new BitmapData();
		cSourceRect = Rect(0, 0, poCompareBitmap->GetWidth(), poCompareBitmap->GetHeight());

		//Lock the bits
		if (poCompareBitmap->LockBits(&cSourceRect, ImageLockModeRead, poCompareBitmap->GetPixelFormat(), oSourceBitmapData)==Ok) {

			for (int y=0; y<32; y++) {

				iCharacterY = y;

				cScanLine = (BYTE*)((BYTE*)oSourceBitmapData->Scan0 + (oSourceBitmapData->Stride * iCharacterY));
				//cScanLine = (BYTE*)poCompareBitmap->ScanLine[y];

				for (int x=0; x<32; x++) {

					bPoint = cScanLine[x];

					if (cSettings.bMakeSkeleton) {

						if (bPoint!=4 && bPoint!=0x00) {
							bPoint=2;
							}
						}

					lDestinationPointer = x + y*32;

					cCompareBitmap[lDestinationPointer] = bPoint;
					}
				}
			poCompareBitmap->UnlockBits(oSourceBitmapData);
            }
        }

    if (cSettings.bMakeSkeleton) {
        //CreateSkeleton(cCompareBitmap);

        //CreateSkeleton2(cCompareBitmap);

        DoThinning(cCompareBitmap, 4, 2);
        DoThinning(cCompareBitmap, 0, 2);

        FindStrokes(cCompareBitmap, -1);
        }

    return cCompareBitmap;
    }
//---------------------------------------------------------------------------
//
//  Functions for thinning. Based on algorithm as described by Gonzales and Woods
//  in their book 'Digital Image Processing'. Source partially based on
//  implementation by the ipl98 group (http://ipl98.sourceforge.net)
//
//----------------------------------------------------------------------------
void
__fastcall TDataLayer::DoThinning(BYTE* pcCompareBitmap, int piForegroundColor, int piBackgroundColor) {

    bool bPointsRemoved;
    vector <TOcrPoint> ToBeRemovedList;
    vector <TOcrPoint> ForegroundPixelList;
    BYTE bNeighbors[3][3];
    TOcrPoint oPoint;
    bPointsRemoved=false;


    //iterate while no points are removed in the last iteration

    /*    bPointsRemoved = true;

    while (bPointsRemoved) {

        bPointsRemoved = false;

        //step 1
        for (int y=0; y<32; y++) {

            for (int x=0; x<32; x++) {

                if (pcCompareBitmap[y*32+x]== piForegroundColor) {

                oPoint.x = x;
                oPoint.y = y;

                if (DoThinningSearchNeighbors(oPoint.x, oPoint.y, pcCompareBitmap, &bNeighbors[0], piForegroundColor) &&
                    DoThinningCheckTransitions(&bNeighbors[0]) &&
                    DoThinningStep1cdTests(&bNeighbors[0])) {

                    bPointsRemoved = true;

                    pcCompareBitmap[y*32 + x] = (BYTE)piBackgroundColor;
                    }
                    }
                }
            }

        //step 2
        for (int y=0; y<32; y++) {

            for (int x=0; x<32; x++) {


                 if (pcCompareBitmap[y*32+x]== piForegroundColor) {
               oPoint.x = x;
                oPoint.y = y;

                if (DoThinningSearchNeighbors(oPoint.x, oPoint.y, pcCompareBitmap, &bNeighbors[0], piForegroundColor) &&
                    DoThinningCheckTransitions(&bNeighbors[0]) &&
                    DoThinningStep2cdTests(&bNeighbors[0])) {

                    bPointsRemoved = true;

                    pcCompareBitmap[y*32 + x] = (BYTE)piBackgroundColor;
                    }
                    }
                }
            }
        }  

           */
        //collect the foreground pixels, and do the first step in seperating
    //pixels which can be removed from other foreground pixels.
    for (int y=0; y<32; y++) {

        for (int x=0; x<32; x++) {

            oPoint.x = x;
            oPoint.y = y;

            if (pcCompareBitmap[x+y*32]==piForegroundColor) {

                if (DoThinningSearchNeighbors(x, y, pcCompareBitmap, &bNeighbors[0], piForegroundColor) &&
                    DoThinningCheckTransitions(&bNeighbors[0]) &&
                    DoThinningStep1cdTests(&bNeighbors[0])) {

                    bPointsRemoved = true;
                    ToBeRemovedList.push_back(oPoint);
                    }
                else {

                    ForegroundPixelList.push_back(oPoint);
                    }
                }
            }
        }

    //Set pixels in toberemovedlist to backgroundcolor
    for (long lIndex=0; lIndex<(long)ToBeRemovedList.size(); lIndex++) {

        pcCompareBitmap[ToBeRemovedList[lIndex].y * 32 + ToBeRemovedList[lIndex].x] = (BYTE)piBackgroundColor;
        }
    ToBeRemovedList.clear();

    if (bPointsRemoved) {

        for (long lIndex=0; lIndex<(long)ForegroundPixelList.size(); lIndex++) {

            oPoint = ForegroundPixelList[lIndex];

            if (DoThinningSearchNeighbors(oPoint.x, oPoint.y, pcCompareBitmap, &bNeighbors[0], piForegroundColor) &&
                DoThinningCheckTransitions(&bNeighbors[0]) &&
                DoThinningStep2cdTests(&bNeighbors[0])) {

                bPointsRemoved = true;

                ToBeRemovedList.push_back(oPoint);

                ForegroundPixelList.erase(ForegroundPixelList.begin() + lIndex);

                lIndex--;
                }
            }
        }

    //Set pixels in toberemovedlist to backgroundcolor
    for (long lIndex=0; lIndex<(long)ToBeRemovedList.size(); lIndex++) {

        pcCompareBitmap[ToBeRemovedList[lIndex].y * 32 + ToBeRemovedList[lIndex].x] = (BYTE)piBackgroundColor;
        }
    ToBeRemovedList.clear();


    //iterate while no points are removed in the last iteration

    while (bPointsRemoved) {

        bPointsRemoved = false;

        //step 1
        for (long lIndex=0; lIndex<(long)ForegroundPixelList.size(); lIndex++) {

            oPoint = ForegroundPixelList[lIndex];

            if (DoThinningSearchNeighbors(oPoint.x, oPoint.y, pcCompareBitmap, &bNeighbors[0], piForegroundColor) &&
                DoThinningCheckTransitions(&bNeighbors[0]) &&
                DoThinningStep1cdTests(&bNeighbors[0])) {

                bPointsRemoved = true;

                ToBeRemovedList.push_back(oPoint);

                ForegroundPixelList.erase(ForegroundPixelList.begin() + lIndex);

                lIndex--;
                }
            }

        //Set pixels in toberemovedlist to backgroundcolor
        for (long lIndex=0; lIndex<(long)ToBeRemovedList.size(); lIndex++) {

            pcCompareBitmap[ToBeRemovedList[lIndex].y * 32 + ToBeRemovedList[lIndex].x] = (BYTE)piBackgroundColor;
            }
        ToBeRemovedList.clear();


        //step 2
        for (long lIndex=0; lIndex<(long)ForegroundPixelList.size(); lIndex++) {

            oPoint = ForegroundPixelList[lIndex];

            if (DoThinningSearchNeighbors(oPoint.x, oPoint.y, pcCompareBitmap, &bNeighbors[0], piForegroundColor) &&
                DoThinningCheckTransitions(&bNeighbors[0]) &&
                DoThinningStep2cdTests(&bNeighbors[0])) {

                bPointsRemoved = true;

                ToBeRemovedList.push_back(oPoint);

                ForegroundPixelList.erase(ForegroundPixelList.begin() + lIndex);

                lIndex--;
                }
            }

        //Set pixels in toberemovedlist to backgroundcolor
        for (long lIndex=0; lIndex<(long)ToBeRemovedList.size(); lIndex++) {

            pcCompareBitmap[ToBeRemovedList[lIndex].y * 32 + ToBeRemovedList[lIndex].x] = (BYTE)piBackgroundColor;
            }
        ToBeRemovedList.clear();
        }

    ForegroundPixelList.clear();
    }

//----------------------------------------------------------------------------

bool
__fastcall TDataLayer::DoThinningStep2cdTests(BYTE pbNeighbors[][3]) {

	if ((pbNeighbors[1][0]*pbNeighbors[2][1]*pbNeighbors[0][1] == 0) &&
		(pbNeighbors[1][0]*pbNeighbors[1][2]*pbNeighbors[0][1] == 0))
		return true;
	else
		return false;
    }

//----------------------------------------------------------------------------

bool
__fastcall TDataLayer::DoThinningStep1cdTests(BYTE pbNeighbors[][3]) {

	if ((pbNeighbors[1][0]*pbNeighbors[2][1]*pbNeighbors[1][2] == 0) &&
		(pbNeighbors[2][1]*pbNeighbors[1][2]*pbNeighbors[0][1] == 0))
		return true;
	else
		return false;
    }

//----------------------------------------------------------------------------

bool
__fastcall TDataLayer::DoThinningCheckTransitions(BYTE pbNeighbors[][3]) {

	int iTransitions=0;
	if ((pbNeighbors[0][0]==1) && (pbNeighbors[1][0]==0)){ ++iTransitions;}
	if ((pbNeighbors[1][0]==1) && (pbNeighbors[2][0]==0)){ ++iTransitions;}
	if ((pbNeighbors[2][0]==1) && (pbNeighbors[2][1]==0)){ ++iTransitions;}
	if ((pbNeighbors[2][1]==1) && (pbNeighbors[2][2]==0)){ ++iTransitions;}
	if ((pbNeighbors[2][2]==1) && (pbNeighbors[1][2]==0)){ ++iTransitions;}
	if ((pbNeighbors[1][2]==1) && (pbNeighbors[0][2]==0)){ ++iTransitions;}
	if ((pbNeighbors[0][2]==1) && (pbNeighbors[0][1]==0)){ ++iTransitions;}
	if ((pbNeighbors[0][1]==1) && (pbNeighbors[0][0]==0)){ ++iTransitions;}
	if (iTransitions==1)
		return true;
	else
		return false;
    }

//----------------------------------------------------------------------------

bool
__fastcall TDataLayer::DoThinningSearchNeighbors(int x, int y, BYTE* pcCompareBitmap,
                                     BYTE pbNeighbors[][3], int piForegroundColor) {

	unsigned short ForeGroundNeighbor=0;

	if ((pbNeighbors[0][0]=(BYTE)DoThinningGetPixel(x-1, y-1, pcCompareBitmap, piForegroundColor)) == 1){ForeGroundNeighbor++;}
	if ((pbNeighbors[1][0]=(BYTE)DoThinningGetPixel(x  , y-1, pcCompareBitmap, piForegroundColor)) == 1){ForeGroundNeighbor++;}
	if ((pbNeighbors[2][0]=(BYTE)DoThinningGetPixel(x+1, y-1, pcCompareBitmap, piForegroundColor)) == 1){ForeGroundNeighbor++;}
	if ((pbNeighbors[0][1]=(BYTE)DoThinningGetPixel(x-1, y  , pcCompareBitmap, piForegroundColor)) == 1){ForeGroundNeighbor++;}
	if ((pbNeighbors[2][1]=(BYTE)DoThinningGetPixel(x+1, y  , pcCompareBitmap, piForegroundColor)) == 1){ForeGroundNeighbor++;}
	if ((pbNeighbors[0][2]=(BYTE)DoThinningGetPixel(x-1, y+1, pcCompareBitmap, piForegroundColor)) == 1){ForeGroundNeighbor++;}
	if ((pbNeighbors[1][2]=(BYTE)DoThinningGetPixel(x  , y+1, pcCompareBitmap, piForegroundColor)) == 1){ForeGroundNeighbor++;}
	if ((pbNeighbors[2][2]=(BYTE)DoThinningGetPixel(x+1, y+1, pcCompareBitmap, piForegroundColor)) == 1){ForeGroundNeighbor++;}

	if ((ForeGroundNeighbor>=2) && (ForeGroundNeighbor<=6))
		return true;
	else
		return false;
    }

//----------------------------------------------------------------------------

int
__fastcall TDataLayer::DoThinningGetPixel(int x, int y, BYTE* pcCompareBitmap, int piForegroundColor) {

    int iReturnValue;

    iReturnValue = 0; //by default return the pixel as background

    if (x>=0 && x<32 && y>=0 && y<32) {

        if (pcCompareBitmap[y*32 + x] == (BYTE)piForegroundColor) {

            iReturnValue = 1; //this pixel has the foreground color
            }
        }

    return iReturnValue;
    }

//---------------------------------------------------------------------------
//
//  Functions for thinning/skeletonization. Based on  article 'Improved
//  low complexity fully parallel thinning algorithm' by Thierry M. Bernard
//  and Antoine Manzanera
//
//----------------------------------------------------------------------------

struct cSkeletonPixelDefinition {

    int dx;
    int dy;
    bool bForeground;
    };

void
__fastcall TDataLayer::CreateSkeleton(BYTE* pcCompareBitmap) {

    long lDestinationPointer;
    int  iForegroundColor;
    bool bChanges;
    bool bPixelRemoved;
    bool bAlpha1, bAlpha2, bBeta;
    long lIterations;


    for (int iColors=0; iColors<2; iColors++) {

        //doorloop het geheel 2*, 1x voor wit als
        //voorgrondkleur en 1x voor zwart als
        //voorgrondkleur
        iForegroundColor = 4-iColors*4;

        bChanges = true;
        lIterations = 0;

        while (bChanges && lIterations<32) {

            lIterations++;

            //loop while there aren't any changes made
            //in the last iteration

            bChanges = false;

            for (int y=0; y<32; y++) {

                lDestinationPointer = y*32;

                for (int x=0; x<32; x++) {

                    bPixelRemoved = false;

                    for (int iRotation=0; iRotation<4 && !bPixelRemoved; iRotation++) {

                        //rotate the MB2 patters 90 degrees to test them in the
                        //4 possible rotations.
                        bBeta = CreateSkeletonTestBeta(pcCompareBitmap, x, y, iForegroundColor, iRotation);

                        if (!bBeta) {

                            bAlpha1 = CreateSkeletonTestAlpha1(pcCompareBitmap, x, y, iForegroundColor, iRotation);

                            bAlpha2 = CreateSkeletonTestAlpha2(pcCompareBitmap, x, y, iForegroundColor, iRotation);

                            if (bAlpha1 || bAlpha2) {


                                bPixelRemoved = true;
                                bChanges = true;

                                pcCompareBitmap[y*32 + x] = 0x01;

                                }
                            }
                        }

                    lDestinationPointer++;
                    }
                }
            }
        }
    }
//---------------------------------------------------------------------------
bool
 __fastcall TDataLayer::CreateSkeletonTestAlpha1(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation) {

    long lSize;
    bool bTestValue;
    int  iX;
    int  iY;

    cSkeletonPixelDefinition cAlpha1[4][5]={{{-1, 0, false}, {1, -1, true}, {1, 0, true}, {2, 0, true}, {1, 1, true}},
                                        {{0, -1, false}, {-1, 1, true}, {0, 1, true}, {1, 1, true}, {0, 2, true}},
                                        {{1, 0, false}, {-1, -1, true}, {-1, 0, true}, {-2, 0, true}, {-1, 1, true}},
                                        {{0, 1, false}, {-1, -1, true}, {0, -1, true}, {1, -1, true}, {0, -2, true}}};

    bTestValue = true;
    lSize = 0;

    do {

        iX = x + cAlpha1[iRotation][lSize].dx;
        iY = y + cAlpha1[iRotation][lSize].dy;

        bTestValue = (cAlpha1[iRotation][lSize].bForeground == CreateSkeletonIsForeground(pcCompareBitmap, iX, iY, iColor));

        lSize++;

        } while (bTestValue && lSize<5);

    return bTestValue;
    }
//---------------------------------------------------------------------------
bool
__fastcall TDataLayer::CreateSkeletonTestAlpha2(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation) {

    cSkeletonPixelDefinition cAlpha2[4][8]={{{-1, -1, false}, {-1, 0, false}, {0, -1, false}, {1, 0, true}, {0, 1, true}, {1, 1, true}, {2, 1, true}, {1, 2, true}},
                                        {{-1, 0, false}, {-1, 1, false}, {0, 1, false}, {0, -1, true}, {1, 0, true}, {1, -1, true}, {1, -2, true}, {2, -1, true}},
                                        {{1, 0, false}, {1, 1, false}, {0, 1, false}, {-1, 0, true}, {-2, 1, true}, {-1, 1, true}, {0, 1, true}, {-1, -2, true}},
                                        {{1, 0, false}, {1, -1, false}, {0, -1, false}, {-1, 0, true}, {-2, 1, true}, {-1, 1, true}, {0, 1, true}, {-1, 2, true}}};

    long lSize;
    bool bTestValue;
    int  iX;
    int  iY;

    bTestValue = true;
    lSize = 0;

    do {

        iX = x + cAlpha2[iRotation][lSize].dx;
        iY = y + cAlpha2[iRotation][lSize].dy;

        bTestValue = (cAlpha2[iRotation][lSize].bForeground == CreateSkeletonIsForeground(pcCompareBitmap, iX, iY, iColor));

        lSize++;

        } while (bTestValue && lSize<8);

    return bTestValue;
    }
//---------------------------------------------------------------------------
bool
__fastcall TDataLayer::CreateSkeletonTestBeta(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation) {

    cSkeletonPixelDefinition cBeta[4][3]={  {{1, 0, false}, {0, 1, false}, {1, 1, true}},
                                        {{0, 1, false}, {1, 1, true}, {1, 0, false}},
                                        {{-1, 0, false}, {-1, -1, true}, {0, -1, false}},
                                        {{0, -1, false}, {-1, 1, true}, {-1, 0, false}}};

    long lSize;
    bool bTestValue;
    int  iX;
    int  iY;

    bTestValue = true;
    lSize = 0;

    do {

        iX = x + cBeta[iRotation][lSize].dx;
        iY = y + cBeta[iRotation][lSize].dy;

        bTestValue = (cBeta[iRotation][lSize].bForeground == CreateSkeletonIsForeground(pcCompareBitmap, iX, iY, iColor));

        lSize++;

        } while (bTestValue && lSize<3);

    return bTestValue;
    }
//---------------------------------------------------------------------------

bool
__fastcall TDataLayer::CreateSkeletonIsForeground(BYTE* pcCompareBitmap, int x, int y, int iColor) {

    bool bReturnValue;

    bReturnValue = false;

    if (x>=0 && x<32 && y>=0 && y<32) {

        bReturnValue = (pcCompareBitmap[y*32 + x] == iColor);
        }

    return bReturnValue;
    }
//---------------------------------------------------------------------------
//
//  Functions for thinning/skeletonization. Based on algorithm described in
//  website http://www.cee.hw.ac.uk/hipr/html/thin.html
//
//----------------------------------------------------------------------------


void
__fastcall TDataLayer::CreateSkeleton2(BYTE* pcCompareBitmap) {

    bool bPointsRemoved;
    vector <TOcrPoint> ToBeRemovedList;
    vector <TOcrPoint> ForegroundPixelList;
    BYTE bNeighbors[3][3];
    TOcrPoint oPoint;
    bPointsRemoved=false;
    int  iForegroundColor;
    bool bChanges;
    bool bPixelRemoved;
    bool bAlpha1, bAlpha2;

    //collect the foreground pixels, and do the first step in seperating
    //pixels which can be removed from other foreground pixels.
    for (int iColors=0; iColors<2; iColors++) {

        //doorloop het geheel 2*, 1x voor wit als
        //voorgrondkleur en 1x voor zwart als
        //voorgrondkleur
        iForegroundColor = iColors*4;

    bPointsRemoved = true;

    while (bPointsRemoved) {

        bPointsRemoved = false;

    for (int y=0; y<32; y++) {

        for (int x=0; x<32; x++) {

            oPoint.x = x;
            oPoint.y = y;

            if (pcCompareBitmap[y*32 + x]==iForegroundColor) {

                DoThinningSearchNeighbors(x, y, pcCompareBitmap, &bNeighbors[0], iForegroundColor);

                bPixelRemoved = false;

                for (int iRotation=0; iRotation<4 && !bPixelRemoved; iRotation++) {

                    bAlpha1 = CreateSkeleton2TestAlpha1(&bNeighbors[0], 1, 1, iForegroundColor, iRotation);

                    bAlpha2 = CreateSkeleton2TestAlpha2(&bNeighbors[0], 1, 1, iForegroundColor, iRotation);

                    if (bAlpha1 || bAlpha2) {

                        bPixelRemoved = true;
                        bPointsRemoved = true;
                        bChanges = true;
                        }
                    }

                if (bPixelRemoved) {
                    pcCompareBitmap[y*32 + x] = 0x02;
                    ToBeRemovedList.push_back(oPoint);
                    }
                else {
                    ForegroundPixelList.push_back(oPoint);
                    }
                }
            }
        }

    //Set pixels in toberemovedlist to backgroundcolor
    for (long lIndex=0; lIndex<(long)ToBeRemovedList.size(); lIndex++) {

       // pcCompareBitmap[ToBeRemovedList[lIndex].y * 32 + ToBeRemovedList[lIndex].x] = (BYTE)2;
        }
    ToBeRemovedList.clear();
    }


    ForegroundPixelList.clear();
    /*
    //iterate while no points are removed in the last iteration

    while (bPointsRemoved) {

        bPointsRemoved = false;

        //step 1
        for (long lIndex=0; lIndex<ForegroundPixelList.size(); lIndex++) {

            oPoint = ForegroundPixelList[lIndex];

            DoThinningSearchNeighbors(oPoint.x, oPoint.y, pcCompareBitmap, &bNeighbors[0], iForegroundColor);

            bPixelRemoved = false;

            bAlpha1 = CreateSkeleton2TestAlpha1(&bNeighbors[0], 1, 1, iForegroundColor, iRotation);

            bAlpha2 = CreateSkeleton2TestAlpha2(&bNeighbors[0], 1, 1, iForegroundColor, iRotation);

            if (bAlpha1 || bAlpha2) {

                  bPixelRemoved = true;
                  bPointsRemoved = true;
                  }

            if (bPixelRemoved) {
                ToBeRemovedList.push_back(oPoint);
                ForegroundPixelList.erase(ForegroundPixelList.begin() + lIndex);
                lIndex--;
                }
            }

        //Set pixels in toberemovedlist to backgroundcolor
        for (long lIndex=0; lIndex<ToBeRemovedList.size(); lIndex++) {

            pcCompareBitmap[ToBeRemovedList[lIndex].y * 32 + ToBeRemovedList[lIndex].x] = (BYTE)2;
            }
        ToBeRemovedList.clear();
        }
    ForegroundPixelList.clear();
    }

    long lDestinationPointer;
    int  iForegroundColor;
    bool bChanges;
    bool bPixelRemoved;
    bool bAlpha1, bAlpha2, bBeta;
    long lIterations;


    for (int iColors=0; iColors<2; iColors++) {

        //doorloop het geheel 2*, 1x voor wit als
        //voorgrondkleur en 1x voor zwart als
        //voorgrondkleur
        iForegroundColor = 4-iColors*4;

        bChanges = true;
        lIterations = 0;

        while (bChanges && lIterations<32) {

            lIterations++;

            //loop while there aren't any changes made
            //in the last iteration

            bChanges = false;

            for (int y=0; y<32; y++) {

                lDestinationPointer = y*32;

                for (int x=0; x<32; x++) {

                    bPixelRemoved = false;

                    for (int iRotation=0; iRotation<4 && !bPixelRemoved; iRotation++) {

                        bAlpha1 = CreateSkeleton2TestAlpha1(pcCompareBitmap, x, y, iForegroundColor, iRotation);

                        bAlpha2 = CreateSkeleton2TestAlpha2(pcCompareBitmap, x, y, iForegroundColor, iRotation);

                        if (bAlpha1 || bAlpha2) {


                            bPixelRemoved = true;
                            bChanges = true;

                            pcCompareBitmap[y*32 + x] = 0x01;

                            }
                        }

                    lDestinationPointer++;
                    }
                }
            }
        } */
    }
    }
//---------------------------------------------------------------------------
bool
__fastcall TDataLayer::CreateSkeleton2TestAlpha1(BYTE pbNeighbors[][3], int x, int y, int iColor, int piRotation) {
// __fastcall TDataLayer::CreateSkeleton2TestAlpha1(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation) {

    long lSize;
    bool bTestValue;
    int  iX;
    int  iY;

    cSkeletonPixelDefinition cAlpha1[4][6]={{{-1, -1, false}, {0, -1, false}, {1, -1, false}, {-1, 1, true}, {0, 1, true}, {1, 1, true}},
                                        {{-1, -1, false}, {-1, 0, false}, {-1, 1, false}, {1, -1, true}, {1, 0, true}, {1, 1, true}},
                                        {{-1, -1, true}, {0, -1, true}, {1, -1, true}, {-1, 1, false}, {0, 1, false}, {1, 1, false}},
                                        {{-1, -1, true}, {-1, 0, true}, {-1, 1, true}, {1, -1, false}, {1, 0, false}, {1, 1, false}}};

    bTestValue = false;

    for (int iRotation=0; iRotation<4 && !bTestValue; iRotation++) {

        bTestValue = true;
        lSize = 0;

        do {

            iX = x + cAlpha1[iRotation][lSize].dx;
            iY = y + cAlpha1[iRotation][lSize].dy;

            bTestValue = (cAlpha1[iRotation][lSize].bForeground == CreateSkeleton2IsForeground(&pbNeighbors[0], iX, iY, iColor));

            lSize++;

            } while (bTestValue && lSize<6);
        }

    return bTestValue;
    }
//---------------------------------------------------------------------------
bool
//__fastcall TDataLayer::CreateSkeleton2TestAlpha2(BYTE* pcCompareBitmap, int x, int y, int iColor, int iRotation) {
__fastcall TDataLayer::CreateSkeleton2TestAlpha2(BYTE pbNeighbors[][3], int x, int y, int iColor, int piRotation) {

    cSkeletonPixelDefinition cAlpha2[4][5]={{{0, 1, false}, {1, -1, false}, {1, 0, false}, {-1, 0, true}, {0, 1, true}},
                                        {{-1, -1, false}, {-1, 0, false}, {0, -1, false}, {1, 0, true}, {0, 1, true}},
                                        {{0, -1, true}, {-1, 0, true}, {1, 0, false}, {1, 1, false}, {0, 1, false}},
                                        {{0, -1, true}, {1, 0, true}, {0, 1, false}, {-1, 1, false}, {-1, 0, false}}};

    long lSize;
    bool bTestValue;
    int  iX;
    int  iY;


    bTestValue = false;

    for (int iRotation=0; iRotation<4 && !bTestValue; iRotation++) {

        bTestValue = true;
        lSize = 0;

        do {

            iX = x + cAlpha2[iRotation][lSize].dx;
            iY = y + cAlpha2[iRotation][lSize].dy;

            bTestValue = (cAlpha2[iRotation][lSize].bForeground == CreateSkeleton2IsForeground(&pbNeighbors[0], iX, iY, iColor));

            lSize++;

            } while (bTestValue && lSize<5);
        }


    return bTestValue;
    }
//---------------------------------------------------------------------------

bool
__fastcall TDataLayer::CreateSkeleton2IsForeground(BYTE pbNeighbors[][3], int x, int y, int iColor) {

    bool bReturnValue;

    bReturnValue = false;

    if (x>=0 && x<32 && y>=0 && y<32) {

        //bReturnValue = (pcCompareBitmap[y*32 + x] == iColor);
        bReturnValue = (pbNeighbors[x][y] == 0);

        }

    return bReturnValue;
    }
//---------------------------------------------------------------------------
void
__fastcall TDataLayer::FindStrokes(BYTE* cBitmap, long lCharacter) {

	//Search for points of thinned image starting from the lower left corner

	long lPointer;

	for (long lDistance=0; lDistance<32; lDistance++) {

		for (long lX=0; lX<lDistance; lX++) {

			lPointer = lX + 32 * (31 - lDistance);

			if (cBitmap[lPointer]==0x04) {

				InvestigateStroke(lX, 31 - lDistance, cBitmap, lCharacter);

				}
			}

		for (long lY=31-lDistance; lY<32 ; lY++) {


			lPointer = lDistance + 32 * lY;

			if (cBitmap[lPointer]==0x04) {

				InvestigateStroke(lDistance, lY, cBitmap, lCharacter);

				}
			}
		}
	}

//---------------------------------------------------------------------------
void
__fastcall TDataLayer::InvestigateStroke(long lX, long lY, BYTE* cBitmap, long lCharacter) {

	long lFirstDirection;
	vector <cDirection> oStroke;

	lFirstDirection = 0;

	lFirstDirection = FollowStroke(cBitmap, oStroke, lX, lY, lFirstDirection);

	if (lFirstDirection!=-1) {

		lFirstDirection = (4 + lFirstDirection) & 7; 

		FollowStroke(cBitmap, oStroke, lX, lY, lFirstDirection);
		}


	//Optimize strokes, remove some 'noisy' pictures
	OptimizeStrokes(oStroke);

	//place direction marks into bitmap
	for (long lIndex=0; lIndex<(long)oStroke.size(); lIndex++) {

    		cBitmap[oStroke[lIndex].lBitmapPointer] = (BYTE)10 + (oStroke[lIndex].lDirection & 3);
        	}

	//SplitAndSaveStrokes(oStroke, lCharacter);
	}

//---------------------------------------------------------------------------
long
__fastcall TDataLayer::FollowStroke(BYTE* cBitmap, vector<cDirection> &oStroke, long lX, long lY, long lDirection) {


	long lTestDirection;
	long lCounter;
	long lPointer;
	bool bContinueFollowStroke;
	long lDeltaX[8] = {0, 1, 1, 1, 0, -1, -1, -1};
	long lDeltaY[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
	long lFirstDirection;
	cDirection oDirection;

	bContinueFollowStroke = true;
	lFirstDirection = -1;

	do {

		//find the next point on the stroke

		lPointer = lX + 32*lY;
		bContinueFollowStroke = false;
		lTestDirection = lDirection;
		lCounter = 0;

		do {

			lTestDirection = (lDirection + lCounter) & 7;

			bContinueFollowStroke = GetPixelBitmap(cBitmap, lX+lDeltaX[lTestDirection], lY+lDeltaY[lTestDirection])==0x04;

			if (!bContinueFollowStroke) {

				lTestDirection = (lDirection - lCounter) & 7;

				bContinueFollowStroke = GetPixelBitmap(cBitmap, lX+lDeltaX[lTestDirection], lY+lDeltaY[lTestDirection])==0x04;
				}

			if (bContinueFollowStroke) {

		                cBitmap[lPointer]=0x05;

				if (lFirstDirection == -1) {

					lFirstDirection = lTestDirection;
					}				

				lDirection = lTestDirection;
				lX += lDeltaX[lTestDirection];
				lY += lDeltaY[lTestDirection];

		                oDirection.lBitmapPointer = lPointer;
		                oDirection.lDirection = lDirection;

				oStroke.push_back(oDirection);
				}

			lCounter++;

			} while (!bContinueFollowStroke && lCounter<4);


		} while (bContinueFollowStroke);

	return lFirstDirection;
	}
//---------------------------------------------------------------------------
void
__fastcall TDataLayer::OptimizeStrokes(vector<cDirection> oStroke) {

	//this function smoothes the strokes somewhat, it helps the identification
	//of strokes when small irregularities are removed.

	long lSize;
	long lComplementTest;

	lSize = oStroke.size();

	for (long lIndex=0; lIndex<lSize; lIndex++) {


		//pattern 1: two pixels are in the middle of two the same directions, and these two pixels are both
		//           eachoters complement (ie, direction nw+ne, or nw+sw)

		if (lIndex>0 && lIndex<lSize-2) {

			if (oStroke[lIndex-1].lDirection == oStroke[lIndex+2].lDirection) {

				lComplementTest = (oStroke[lIndex].lDirection+oStroke[lIndex+1].lDirection) & 7;

				if (lComplementTest == 4 || lComplementTest==0) {

					oStroke[lIndex].lDirection = oStroke[lIndex-1].lDirection;
					oStroke[lIndex+1].lDirection = oStroke[lIndex-1].lDirection;
					}
				}
			}

		//pattern 2: two pixels are in the middle of two the same directions, and these two pixels are both
		//           eachoters complement (ie, direction nw+ne, or nw+sw)

		if (lIndex>0 && lIndex<lSize-3) {

			if (oStroke[lIndex-1].lDirection == oStroke[lIndex+3].lDirection) {

				lComplementTest = (oStroke[lIndex].lDirection+oStroke[lIndex+2].lDirection) & 7;

				if (lComplementTest == 4 || lComplementTest==0) {

					oStroke[lIndex].lDirection = oStroke[lIndex-1].lDirection;
					oStroke[lIndex+2].lDirection = oStroke[lIndex-1].lDirection;
					}
				}
			}

		}
	}

//---------------------------------------------------------------------------

void
__fastcall TDataLayer::SplitAndSaveStrokes(vector<long> oStroke, long lCharacter) {

	vector<float> oSlidingAverageUp;
	vector<float> oSlidingAverageDown;
	long lSize;
	float lAverageUp, lAverageDown;

	lSize = oStroke.size();

	//we only look at strokes which are at least 4 pixels in length.

	//problem: irregular results when going from direction 0 to 7 and visa versa.
	if (lSize>4) {

		//fill the sliding average vectors
		for (long lIndex=0; lIndex<lSize-4; lIndex++) {

			lAverageUp = lAverageDown = 0;

			for (long lIndex2=0; lIndex2<4; lIndex2++) {

				lAverageUp+=oStroke[lIndex + lIndex2];
				lAverageDown+=oStroke[lSize -lIndex - lIndex2 - 1];

				}

			oSlidingAverageUp.push_back(lAverageUp / 4);
			oSlidingAverageDown.push_back(lAverageDown / 4);
			}
		}

	//compare the two sliding average sequences, the parts that are similar point
	//to a stroke.
	lSize = oSlidingAverageUp.size();
	}

//---------------------------------------------------------------------------

BYTE
__fastcall TDataLayer::GetPixelBitmap(BYTE* cBitmap, long lX, long lY) {


	//get a pixel from the bitmap if it is an existing pixel (X in range 0..31
	//and Y in range 0..31). If it is nog an existing pixel then return 0

	BYTE bReturn;

	bReturn = 0;

	if (lX>=0 && lX<32 && lY>=0 && lY<32) {


		bReturn = cBitmap[lX + lY * 32];
		}

	return bReturn;
	}
//---------------------------------------------------------------------------

	std::wstring TDataLayer::StringToWString(const std::string& s)
	{
	std::wstring temp(s.length(),L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp; 
	}

	std::string TDataLayer::WStringToString(const std::wstring& s)
	{
	std::string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp; 
	}




