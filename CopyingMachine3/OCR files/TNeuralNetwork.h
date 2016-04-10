//---------------------------------------------------------------------------
#ifndef TNeuralNetworkH
#define TNeuralNetworkH

#include "tinyxml/tinyxml.h"

//---------------------------------------------------------------------------
enum eNeuralNodeType {eInput, eOutput, eHidden};

//---------------------------------------------------------------------------
struct sNeuralInput {

    vector<float> fInputs;
    };
//---------------------------------------------------------------------------
struct sNeuralOutput {

    vector<float> fOutputs;
    };
//---------------------------------------------------------------------------

class TNeuralConnection {

    public:

        TNeuralConnection(long plStartNode, long plEndNode);
        ~TNeuralConnection();

        double fWeight;
        double fLastError;

        long  lStartNode;
        long  lEndNode;
    };

//---------------------------------------------------------------------------

class TNeuralNode {

    public:
        TNeuralNode(eNeuralNodeType peType, long plLayer);
        ~TNeuralNode();

        void __fastcall SetInputValue(float pfInput);

        double fTreshold;
        eNeuralNodeType eType;
        long lLayer;
        long lNodeId;

        double fValue;
        double fDerivative;
        double fErrorDelta;
        double fCumulativeErrorDelta;
        double fInputMax;
        bool bCalculated;

		vector<TNeuralConnection*> oConnectionsTo; //reference to connections
    };

//---------------------------------------------------------------------------

class TLearningData {

    public:
        TLearningData();
        ~TLearningData();

        sNeuralInput oInput;

        sNeuralOutput oOutput;
    };

//---------------------------------------------------------------------------

class TNeuralNetwork {

    public:
        TNeuralNetwork();
        ~TNeuralNetwork();

        void __fastcall LoadXML(TiXmlNode* oParentNode);
        void __fastcall SaveXML(TiXmlNode* oParentNode);

        void __fastcall AddNode(long plLayer, eNeuralNodeType peType);

        void __fastcall AddSituation(TLearningData* oLearningData);
        void __fastcall ClearSituations();

        void __fastcall ComputeInputRatios();

        void __fastcall Learn(int piRepetitions = 100);

        void __fastcall ComputeOutput(sNeuralInput oInput, sNeuralOutput &oOutput);
        float __fastcall ComputeNode(long plNode);
        float __fastcall ComputeSuccessPercentage(float pfTreshold);

        void __fastcall ClearNetwork();
        void __fastcall CopyNetwork(TNeuralNetwork* poSourceNetwork);

        vector<TLearningData*> oLearningData;
        vector<TNeuralNode*> oNodes;

        HWND hStatusLabel;

    protected:
        float __fastcall Sigmoid(float pfTreshold, float pfInput);
        void __fastcall LearnUpdateWeights(long plNode, float pfDesired, float pfOutput, float pfLearnRate);
        void __fastcall InitNetworkForLearning();

    private:

		float StringToFloat(std::string psInput);

        vector<TNeuralConnection*> oConnections;
        vector<TNeuralNode*> oInputNodes;
        vector<TNeuralNode*> oOutputNodes;

		int __fastcall random(int piMax);

        bool bDebug;
        bool bTrace;
    };

//---------------------------------------------------------------------------
#endif

