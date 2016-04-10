// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the RSADLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// RSADLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#define RSADLL_API __declspec(dllexport)

// This class is exported from the rsadll.dll
class RSADLL_API Crsadll {
public:
	Crsadll(void);
	// TODO: add your methods here.
};

extern "C" {
	__declspec(dllexport) int __cdecl GenerateKey(char* keyfile, char* input, unsigned int inputLen); 

}
