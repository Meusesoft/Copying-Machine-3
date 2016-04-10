//---------------------------------------------------------------------------
#include "stdafx.h"

#include "TNeuralNetwork.h"

//---------------------------------------------------------------------------
TNeuralNetwork::TNeuralNetwork() {

    bDebug = true; //when true some text file will be created for debugging
    bTrace = true; //when true tracing messages will be created.

    hStatusLabel = NULL;
    }
//---------------------------------------------------------------------------
TNeuralNetwork::~TNeuralNetwork() {

    //deleta all nodes en connections
    ClearNetwork();

    //delete all nodes from the connections and clear the
    //vector
    ClearSituations();
    }
//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::ClearNetwork() {

    //clear the vector of pointers to the input nodes
    oInputNodes.clear();
    oOutputNodes.clear();

    //delete all nodes from the network and clear the
    //vector
    for (long lIndex=0; lIndex<(long)oNodes.size(); lIndex++) {

        delete oNodes[lIndex];
        }
    oNodes.clear();


    //delete all nodes from the connections and clear the
    //vector
    for (long lIndex=0; lIndex<(long)oConnections.size(); lIndex++) {

        delete oConnections[lIndex];
        }
    oConnections.clear();
    }
//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::CopyNetwork(TNeuralNetwork* poSourceNetwork) {

    TiXmlNode* oXMLNode;
    TiXmlElement* oXMLElement;
    TiXmlDocument * oXMLDocument;

    //clear the current network
    ClearNetwork();

    //copy the network by creating XML of the source
    //network and use this to recreate it for us.
    oXMLDocument = new TiXmlDocument("c:\\test.xml");

    oXMLElement = new TiXmlElement("NeuralNetwork");
    oXMLNode = oXMLDocument->InsertEndChild(*oXMLElement);

    poSourceNetwork->SaveXML(oXMLNode);

//    oXMLDocument->SaveFile("c:\\test.xml");

    LoadXML(oXMLNode->FirstChild());

    delete oXMLDocument;
    }


//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::LoadXML(TiXmlNode* oParentNode) {

    TiXmlElement* oXMLNeuralElement;
    TiXmlNode* oXMLNodesNode;
    TiXmlElement* oXMLNodeElement;
    TiXmlNode* oXMLConnectionsNode;
    TiXmlElement* oXMLConnectionElement;

    TNeuralNode* oNeuralNode;
    TNeuralConnection* oNeuralConnection;

    long lLayer;
    float fTreshold;
    eNeuralNodeType eType;
    float fWeight;
    long lStartNode;
    long lEndNode;

    string sValue;

    //reconstruct the neural network which is described
    //in the xml. We use TinyXML for reading out the
    //nodes and elements.
    //No checks are made whether the network is valid (all
    //nodes connected)

    //First we make sure this neural network is empty by
    //deleting all nodes en connections, and we read out
    //the maximum input values
    ClearNetwork();

    oXMLNeuralElement = oParentNode->ToElement();


    //Secondly we add all the nodes described in the
    //<nodes> node
    oXMLNodesNode = oParentNode->FirstChild("Nodes");

    if (oXMLNodesNode!=NULL) {

        oXMLNodeElement = oXMLNodesNode->FirstChildElement("Node");

        while (oXMLNodeElement!=NULL) {

            lLayer = atoi(oXMLNodeElement->Attribute("lLayer"));
            eType = (eNeuralNodeType)atoi(oXMLNodeElement->Attribute("lType"));

            sValue = oXMLNodeElement->Attribute("fTreshold");
			fTreshold = StringToFloat(sValue);

            oNeuralNode = new TNeuralNode(eType, lLayer);
            oNeuralNode->fTreshold = fTreshold;
            oNeuralNode->lNodeId = oNodes.size();

            oNodes.push_back(oNeuralNode);

            if (oNeuralNode->eType == eInput) {

                //if this is an input node, then also read the input max value and
                //place the node in the vector of input nodes
                oNeuralNode->fInputMax = StringToFloat(oXMLNodeElement->Attribute("fInputMax"));
                oInputNodes.push_back(oNeuralNode);
                }

            if (oNeuralNode->eType == eOutput) {

                oOutputNodes.push_back(oNeuralNode);
                }

            oXMLNodeElement = oXMLNodeElement->NextSiblingElement();
            }
        }

    //Finally we add all the connections described in the
    //<connections> node
    oXMLConnectionsNode = oParentNode->FirstChild("Connections");

    if (oXMLConnectionsNode!=NULL) {

        oXMLConnectionElement = oXMLConnectionsNode->FirstChildElement("Connection");

        while (oXMLConnectionElement!=NULL) {

            lStartNode = atoi(oXMLConnectionElement->Attribute("lStart"));
            lEndNode = atoi(oXMLConnectionElement->Attribute("lEnd"));

            sValue = oXMLConnectionElement->Attribute("fWeight");
            fWeight = StringToFloat(sValue);

            oNeuralConnection = new TNeuralConnection(lStartNode, lEndNode);
            oNeuralConnection->fWeight = fWeight;

            oConnections.push_back(oNeuralConnection);

			oNodes[lEndNode]->oConnectionsTo.push_back(oNeuralConnection);

            oXMLConnectionElement = oXMLConnectionElement->NextSiblingElement();
            }
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::SaveXML(TiXmlNode* oParentNode) {

    //save the neural network to XML. We use the TinyXML
    //for creating and filling nodes.

    TiXmlElement* oXMLNeuralElement;
    TiXmlNode* oXMLNeuralNode;
    TiXmlElement* oXMLNodesElement;
    TiXmlNode* oXMLNodesNode;
    TiXmlElement* oXMLNodeElement;
    TiXmlElement* oXMLConnectionsElement;
    TiXmlNode* oXMLConnectionsNode;
    TiXmlElement* oXMLConnectionElement;
	char sNumber[20];

    oXMLNeuralElement = new TiXmlElement("NeuralNetwork");

    oXMLNeuralNode = oParentNode->InsertEndChild(*oXMLNeuralElement);

    //Save nodes as XML
    oXMLNodesElement = new TiXmlElement("Nodes");
    oXMLNodesNode = oXMLNeuralNode->InsertEndChild(*oXMLNodesElement);

    for (long lIndex=0; lIndex<(long)oNodes.size(); lIndex++) {

        oXMLNodeElement = new TiXmlElement("Node");

        oXMLNodeElement->SetAttribute("lLayer", oNodes[lIndex]->lLayer);
        oXMLNodeElement->SetAttribute("lType", oNodes[lIndex]->eType);
		sprintf_s(sNumber, 20, "%f", oNodes[lIndex]->fTreshold); 
        oXMLNodeElement->SetAttribute("fTreshold", sNumber);

        if (oNodes[lIndex]->eType == eInput) {

 			sprintf_s(sNumber, 20, "%f", oNodes[lIndex]->fInputMax); 
			oXMLNodeElement->SetAttribute("fInputMax", sNumber);
            }

        oXMLNodesNode->InsertEndChild(*oXMLNodeElement);
        }

    //Save nodes as XML
    oXMLConnectionsElement = new TiXmlElement("Connections");
    oXMLConnectionsNode = oXMLNeuralNode->InsertEndChild(*oXMLConnectionsElement);

    for (long lIndex=0; lIndex<(long)oConnections.size(); lIndex++) {

        oXMLConnectionElement = new TiXmlElement("Connection");

        oXMLConnectionElement->SetAttribute("lStart", oConnections[lIndex]->lStartNode);
        oXMLConnectionElement->SetAttribute("lEnd", oConnections[lIndex]->lEndNode);
  		sprintf_s(sNumber, 20, "%f", oConnections[lIndex]->fWeight); 
		oXMLConnectionElement->SetAttribute("fWeight", sNumber);

        oXMLConnectionsNode->InsertEndChild(*oXMLConnectionElement);
        }
    }
//---------------------------------------------------------------------------
float 
TNeuralNetwork::StringToFloat(std::string psInput) {

	float fResult;
	std::string sValue;
	int iIndex;

	sValue = psInput;

	//replace a comma by a point
	iIndex = sValue.find ( "," , 0 );
	if (iIndex!= string::npos ) {

		sValue.replace(iIndex, 1, ".");
		}

	//convert to string
	fResult = (float)atof(sValue.c_str());

	return fResult;
}
//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::ClearSituations() {

    for (long lIndex=0; lIndex<(long)oLearningData.size(); lIndex++) {

        delete oLearningData[lIndex];
        }
    oLearningData.clear();
    }
//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::AddSituation(TLearningData* poLearningData) {

    oLearningData.push_back(poLearningData);
    }
//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::ComputeInputRatios() {

    vector<float>fMax;

    //init the values of the vector, set them all to 1.
    for (long lIndex=0; lIndex<(long)oInputNodes.size(); lIndex++) {

        fMax.push_back(1);
        }

    //determine the maximum values in the learning data per input node
    //to do: we assume the oInputs vector in the learning data is as
    //large as the number of input nodes.
    for (long lIndex=0; lIndex<(long)oLearningData.size(); lIndex++) {

        for (long lIndex2=0; lIndex2<(long)oInputNodes.size(); lIndex2++) {

            if (oLearningData[lIndex]->oInput.fInputs[lIndex2] > fMax[lIndex2]) {

                fMax[lIndex2] = oLearningData[lIndex]->oInput.fInputs[lIndex2];
                }
            }
        }

    //set the maximum allowed value per input node
    for (long lIndex=0; lIndex<(long)oInputNodes.size(); lIndex++) {

        oInputNodes[lIndex]->fInputMax = fMax[lIndex];
        }
    }

//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::AddNode(long plLayer, eNeuralNodeType peType) {

    TNeuralNode* oNode;
    TNeuralConnection* oConnection;
    long lNodeIndex;

    //create node
    oNode = new TNeuralNode(peType, plLayer);
    oNode->fTreshold = 0;
    oNode->lNodeId = oNodes.size();
    oNodes.push_back(oNode);

    switch (peType) {

        case eInput: {

            oInputNodes.push_back(oNode);
            break;
            }

        case eOutput: {

            oOutputNodes.push_back(oNode);
            break;
            }
        }


    lNodeIndex = oNodes.size()-1;

    //create connections to all nodes in previous layer
    for (long lIndex=0; lIndex<(long)oNodes.size(); lIndex++) {

        if (oNodes[lIndex]->lLayer == plLayer-1) {

            oConnection = new TNeuralConnection(lIndex, lNodeIndex);

            oConnection->fWeight = 0;

            oConnections.push_back(oConnection);

			oNode->oConnectionsTo.push_back(oConnection);
            }
        }

    //create connections to all nodes in next layer
    for (long lIndex=0; lIndex<(long)oNodes.size(); lIndex++) {

        if (oNodes[lIndex]->lLayer == plLayer+1) {

            oConnection = new TNeuralConnection(lNodeIndex, lIndex);

            oConnection->fWeight = 0;

            oConnections.push_back(oConnection);

			oNodes[lIndex]->oConnectionsTo.push_back(oConnection);
            }
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::InitNetworkForLearning() {

    //fill the network with random values as a starting
    //point for 'learning'

    //randomize();
	srand( (unsigned)time( NULL ) );

    for (long lIndex=0; lIndex<(long)oNodes.size(); lIndex++) {

        oNodes[lIndex]->fTreshold = 0.5 - (((float)random(1000)) / 1000);
        }

    for (long lIndex=0; lIndex<(long)oConnections.size(); lIndex++) {

        oConnections[lIndex]->fWeight = 0.5 - (((float)random(1000)) / 1000);
        }
    }

int 
__fastcall TNeuralNetwork::random(int piMax) {

	return (int)(((double) rand() / (double) RAND_MAX) * piMax);
}

//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::Learn(int piRepetitions) {


    float fError;
    float fTotalError;
    long lRepetition;
    long lLayer;
    long lCounter;
    vector <float> oTotalErrors;
    sNeuralOutput oOutput;
    string sLine;
    long lExample;
	wchar_t sMessage[128];

    //int iFileHandle;
    //int iFileHandle2;

    InitNetworkForLearning();
    lCounter = 0;

    //if (bDebug) {

        //in debug mode the start situation of the neural network
        //will be recorded into a text file

        //iFileHandle2 = FileCreate("c:\\epochs.txt");

        //iFileHandle = FileCreate("c:\\neuralnetwork.txt");

        //if (iFileHandle != -1) {

        //    for (long lIndex=0; lIndex<(long)oNodes.size(); lIndex++) {

        //        sLine = "Node ";
        //        sLine += lIndex;
        //        sLine += " Treshold: ";
        //        sLine += oNodes[lIndex]->fTreshold;
        //        sLine += "\r\n";

        //        FileWrite(iFileHandle, sLine.c_str(), sLine.Length());
        //        }
        //    for (long lIndex=0; lIndex<(long)oConnections.size(); lIndex++) {

        //        sLine = "Connection ";
        //        sLine += lIndex;
        //        sLine += " from ";
        //        sLine += oConnections[lIndex]->lStartNode;
        //        sLine += " to ";
        //        sLine += oConnections[lIndex]->lEndNode;
        //        sLine += " Weight: ";
        //        sLine += oConnections[lIndex]->fWeight;
        //        sLine += "\r\n";

        //        FileWrite(iFileHandle, sLine.c_str(), sLine.length());
        //        }
        //    }
        //}

    //start learning by back-propagation
    fError = 0;
    fTotalError = 0;
    lRepetition = 0;

    float fGoodError = 0;
    long  lGood = 0;
    float fFaultError = 0;
    long  lFault = 0;

    if (oLearningData.size()>0) {

        do {

            fGoodError = 0;
            lGood = 0;
            fFaultError = 0;
            lFault = 0;

            fTotalError = 0;

            //Randomize();
			srand( (unsigned)time( NULL ) );

            for (long lIndex=0; lIndex<(long)oLearningData.size(); lIndex++) {

                //get an example from the learning vector
                lExample = random(oLearningData.size()-1);

                //compute the value suggested by the network
                ComputeOutput(oLearningData[lExample]->oInput, oOutput);

                //make sure the outputs (desired and computed) are the same size
                if (oOutput.fOutputs.size() != oLearningData[lExample]->oOutput.fOutputs.size()) {
                    ::MessageBox(NULL, L"Desired and computed output differ in size. Cannot compare", L"Error in learning.", MB_OK);
                    return;
                    }

                //compute error
                for (long lIndex2=0; lIndex2<(long)oOutput.fOutputs.size(); lIndex2++) {

                    if (oLearningData[lExample]->oOutput.fOutputs[lIndex2] > 0.5) {

                        fGoodError += fabs(oOutput.fOutputs[lIndex2] - 1);
                        lGood++;
                        }
                    else {

                        fFaultError += fabs(oOutput.fOutputs[lIndex2]);
                        lFault++;
                        }

                    //fError += fabs(oOutput.fOutputs[lIndex2] - oLearningData[lIndex]->oOutput.fOutputs[lIndex2]);
                    }
                
                //fTotalError += fError;

                //determine the number of the output layer in the network
                lLayer =  oOutputNodes[0]->lLayer;

                //set cumulative error to 0 in all nodes
                for (long lIndex2=0; lIndex2<(long)oNodes.size(); lIndex2++) {

                    oNodes[lIndex2]->fCumulativeErrorDelta = 0;
                    }

                //do the back propagation
                //start with output layer first
                for (long lIndex2=0; lIndex2<(long)oOutputNodes.size(); lIndex2++) {

                    LearnUpdateWeights(oOutputNodes[lIndex2]->lNodeId, oLearningData[lExample]->oOutput.fOutputs[lIndex2], oOutput.fOutputs[lIndex2], 0.50);
                    }

                //followed by the other nodes
                lLayer--;

                do {

                    for (long lIndex2=0; lIndex2<(long)oNodes.size(); lIndex2++) {

                        if (oNodes[lIndex2]->lLayer==lLayer && oOutputNodes[0]->lLayer) {

                            LearnUpdateWeights(lIndex2, 0, 0, 0.50); //the outputs don't matter for the hidden layers, therefor they are set to 0
                            }
                        }

                    lLayer--;

                    } while (lLayer>0);
                }

            /*oTotalErrors.push_back(fTotalError);

            fAverage = 0.6;

            if (oTotalErrors.size() > 11) {

                //compute average difference of last 10 repetitions
                fAverage = 0;
                for (long lIndex = oTotalErrors.size()-11; lIndex < oTotalErrors.size()-1; lIndex++) {

                    fAverage += fabs(oTotalErrors[lIndex] - oTotalErrors[lIndex+1]);
                    }

                fAverage = fAverage / 10;
                }  */

             lRepetition++;

             fFaultError = fFaultError / (float)lFault;
             fGoodError = fGoodError / (float)lGood;

             //if (bDebug) {

             //    sLine = fFaultError;
             //    sLine += ";";
             //    sLine += fGoodError;
             //    sLine += "\r\n";

             //    FileWrite(iFileHandle2, sLine.c_str(), sLine.Length());
             //    }
                  
             lCounter++;

		     if (hStatusLabel!=NULL) {
				 wsprintf(sMessage, L"Repetition %d / %d: good %d, fault %d", lRepetition, piRepetitions, (int)(fGoodError*100), (int)(fFaultError*100));				
		        SetWindowText(hStatusLabel, sMessage);
				MSG msg;

				while ((PeekMessage( &msg, NULL, 0, 0 , PM_REMOVE)) != 0) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					}
				}

             //if (oStatusLabel!=NULL && lCounter>=2) {
             //   oStatusLabel->Caption = lRepetition;
             //   Application->ProcessMessages();
             //   lCounter = 0;
             //   }
                
            //} while (/*fAverage > 0.005 && */lRepetition<450);
            } while (lRepetition<piRepetitions); // || (fabs(fFaultError-fGoodError)>0.3 && lRepetition<2000));
        }

    if (hStatusLabel!=NULL) {

        SetWindowText(hStatusLabel, L"");
        }

    //if (bDebug) {

    //    //in debug mode the end situation of the neural network
    //    //will be recorded into a text file
    //    if (iFileHandle != -1) {

    //        for (long lIndex=0; lIndex<(long)oNodes.size(); lIndex++) {

    //            sLine = "Node ";
    //            sLine += lIndex;
    //            sLine += " Treshold: ";
    //            sLine += oNodes[lIndex]->fTreshold;
    //            sLine += "\r\n";

    //            FileWrite(iFileHandle, sLine.c_str(), sLine.Length());
    //            }
    //        for (long lIndex=0; lIndex<oConnections.size(); lIndex++) {

    //            sLine = "Connection ";
    //            sLine += lIndex;
    //            sLine += " from ";
    //            sLine += oConnections[lIndex]->lStartNode;
    //            sLine += " to ";
    //            sLine += oConnections[lIndex]->lEndNode;
    //            sLine += " Weight: ";
    //            sLine += oConnections[lIndex]->fWeight;
    //            sLine += "\r\n";

    //            FileWrite(iFileHandle, sLine.c_str(), sLine.Length());
    //            }
    //        }
    //    FileClose(iFileHandle);

    //    FileClose(iFileHandle2);
    //    }
    }
//---------------------------------------------------------------------------
float
__fastcall TNeuralNetwork::ComputeSuccessPercentage(float pfTreshold) {

//    float fOutput;
    float fReturn;
	bool bGood;
    long lGood;
    long lGoodExamples;
    sNeuralOutput oOutput;

    lGood = lGoodExamples = 0;
    fReturn = 0;

    //loop through the examples
    for (long lIndex=0; lIndex<(long)oLearningData.size(); lIndex++) {

        ComputeOutput(oLearningData[lIndex]->oInput, oOutput);

		if (oOutput.fOutputs.size() == oLearningData[lIndex]->oOutput.fOutputs.size()) {

            lGoodExamples++;
			bGood = true; 

			for (long lOutputIndex=0; bGood && lOutputIndex<(long)oLearningData[lIndex]->oOutput.fOutputs.size(); lOutputIndex++) {
			
				if (fabs(oLearningData[lIndex]->oOutput.fOutputs[lOutputIndex] - oOutput.fOutputs[lOutputIndex])>pfTreshold) {
					bGood = false;
					}
                }

			if (bGood) lGood++;
            }
        }

    if (lGoodExamples>0) {

        fReturn = (float)((lGood * 100) / lGoodExamples);
        }

    return fReturn;
    }
//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::ComputeOutput(sNeuralInput oInput, sNeuralOutput &oOutput) {

    //clear the output structure
    oOutput.fOutputs.clear();

    //init network
    for (long lIndex=0; lIndex<(long)oNodes.size(); lIndex++) {

        oNodes[lIndex]->bCalculated = false;
        }

    //fill input nodes
    for (long lIndex=0; lIndex<(long)oInput.fInputs.size() && lIndex<(long)oInputNodes.size(); lIndex++) {

        oInputNodes[lIndex]->SetInputValue(oInput.fInputs[lIndex]);
        }

    //compute output nodes
    for (long lIndex=0; lIndex<(long)oOutputNodes.size(); lIndex++) {

        oOutput.fOutputs.push_back(ComputeNode(oOutputNodes[lIndex]->lNodeId));
        }
    }
//---------------------------------------------------------------------------
float
__fastcall TNeuralNetwork::ComputeNode(long plNode) {

    float fValue;
	long lIndex;
	TNeuralNode* oNode;

    if (!oNodes[plNode]->bCalculated) {

		oNode = oNodes[plNode];

        fValue = (float)(oNode->fTreshold * -1);

        //loop through the connection list and get the values of all nodes before
        //the current one.
		lIndex = oNode->oConnectionsTo.size();

		while (lIndex>0) {

			lIndex--;

            //Calculate the input value of this node by summing all output values of previous
            //nodes multiplied by the weight of the connection.
            fValue = fValue + oNode->oConnectionsTo[lIndex]->fWeight * ComputeNode(oNode->oConnectionsTo[lIndex]->lStartNode);
        }

        oNode->bCalculated = true;
        oNode->fValue = fValue;

        //On other nodes then the input and output nodes apply an
        //activation function. In this case it is a step function, meaning
        //that if a certain treshold is crossed it will output 1, else
        //it will output zero.
        if (oNode->eType == eHidden) {
            oNode->fValue = Sigmoid(0, fValue);
            }

        if (oNode->eType == eOutput) {
            oNode->fValue = Sigmoid(0, fValue);
            }
        }

    //return the output of the node
    return (float)oNodes[plNode]->fValue;
    }
//---------------------------------------------------------------------------

float
__fastcall TNeuralNetwork::Sigmoid(float pfTreshold, float pfInput) {

    float fValue;

    fValue = 1 / (1 + exp(-(pfInput - pfTreshold)));

    return fValue;
    }
//---------------------------------------------------------------------------
void
__fastcall TNeuralNetwork::LearnUpdateWeights(long plNode, float pfDesired, float pfOutput, float pfLearnRate) {

    float fWeightDelta;
    float fErrorDelta;
	TNeuralNode* oNode;
	long lIndex;

    fWeightDelta = 0;
	oNode = oNodes[plNode];
	lIndex = oNode->oConnectionsTo.size();

	while (lIndex>0) {

		lIndex--;

        if (oNode->eType == eHidden) {
            fErrorDelta = oNode->fValue * (1- oNode->fValue) * oNode->fCumulativeErrorDelta;
            oNodes[oNode->oConnectionsTo[lIndex]->lStartNode]->fCumulativeErrorDelta += fErrorDelta * oNode->oConnectionsTo[lIndex]->fWeight;
            }

        if (oNode->eType == eOutput) {
            fErrorDelta = pfOutput*(1-pfOutput)*(pfDesired-pfOutput);
            oNodes[oNode->oConnectionsTo[lIndex]->lStartNode]->fCumulativeErrorDelta += fErrorDelta * oNode->oConnectionsTo[lIndex]->fWeight;
            }

        fWeightDelta = pfLearnRate * oNodes[oNode->oConnectionsTo[lIndex]->lStartNode]->fValue * fErrorDelta;

        oNode->oConnectionsTo[lIndex]->fWeight += fWeightDelta;
        }

    //adjust treshold, the treshold is seen as another connection with a constant input of -1. The weight
    //of the connection will be the treshold value.

    if (oNode->eType == eHidden) {
        fErrorDelta = oNode->fValue * (1- oNode->fValue) * oNode->fCumulativeErrorDelta;
        }

    if (oNode->eType == eOutput) {
        fErrorDelta = pfOutput*(1-pfOutput)*(pfDesired-pfOutput);
        }

    fWeightDelta = pfLearnRate * -1 * fErrorDelta;
    oNode->fTreshold += fWeightDelta;
    }
//---------------------------------------------------------------------------
TNeuralNode::TNeuralNode(eNeuralNodeType peType, long plLayer) {

    eType = peType;
    lLayer = plLayer;

    fTreshold = 0;
    fValue = 0;
    fDerivative = 0;
    fErrorDelta = 0;
    fCumulativeErrorDelta = 0;
    fInputMax = 0;
    bCalculated = false;
    }
//---------------------------------------------------------------------------
TNeuralNode::~TNeuralNode() {
    }
//---------------------------------------------------------------------------
void
__fastcall TNeuralNode::SetInputValue(float pfInput) {

    bCalculated = true;

    if (fInputMax==0) {

        fValue = pfInput;
        }
    else {

        fValue = pfInput / fInputMax;
        }
    }
//---------------------------------------------------------------------------
TNeuralConnection::TNeuralConnection(long plStartNode, long plEndNode) {

    lStartNode = plStartNode;
    lEndNode = plEndNode;

    fWeight = 0;
    fLastError = 0;
    }
//---------------------------------------------------------------------------
TNeuralConnection::~TNeuralConnection() {
    }
//---------------------------------------------------------------------------
TLearningData::TLearningData() {

    oInput.fInputs.clear();
    oOutput.fOutputs.clear();
    }
//---------------------------------------------------------------------------
TLearningData::~TLearningData() {
    }
//---------------------------------------------------------------------------



