// rsadll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "rsadll.h"
#include "rsa.h"
#include "stdio.h"

extern int GenerateKey(char* KeyFile, char* KeyInput, unsigned int KeyInputLen) 
{


	R_RSA_PRIVATE_KEY privateKey;
	
    ZeroMemory(&privateKey, sizeof(privateKey));

    FILE* fileHandle = fopen("D:\\Projects\\CopyingMachine3\\debug\\copyingmachineprivate.key", "rb");

    if (fileHandle!=NULL) {
       
		int bytesread;

       bytesread = fread(&privateKey, 1, sizeof(privateKey), fileHandle);

       if (bytesread<sizeof(privateKey)) {
            
			MessageBox(NULL, L"No private key", L"Error", MB_OK);
		   return -1;
          };
       fclose(fileHandle);
       }
    else {
			MessageBox(NULL, L"No private key", L"Error", MB_OK);
            return -1;
         }

		//MessageBox(NULL, L"Key read", L"Status", MB_OK);

		char hexval[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

		unsigned char output[MAX_RSA_MODULUS_LEN + 1]; /* output block */
		unsigned int outputLen; /* length of output block */
		unsigned char input[MAX_RSA_MODULUS_LEN + 1]; /* input block */
		unsigned int inputLen; /* length of input block */
		int maxMessageLength = 21;
		int strPointer = 0;
		char KeyOutput[8196];
		char HexValue[3];

		ZeroMemory(KeyOutput, sizeof(KeyOutput));

		fileHandle = fopen(KeyFile, "w");

		//MessageBox(NULL, L"Start create key", L"Status", MB_OK);

		if (fileHandle==NULL) {

			MessageBox(NULL, L"Key file not created", L"Error", MB_OK);
			return -1;
		}

		do {

			ZeroMemory(input, MAX_RSA_MODULUS_LEN+1);
			if (strlen(KeyInput + strPointer) >= maxMessageLength) {

				//MessageBox(NULL, L"Input >= Length", L"Status", MB_OK);

				strncpy((char*)&input[0], KeyInput+strPointer, maxMessageLength);
				strPointer += maxMessageLength;
			}
			else {

				//MessageBox(NULL, L"Input < Length", L"Status", MB_OK);

				strncpy((char*)&input[0], KeyInput+strPointer, strlen(KeyInput + strPointer));
				strPointer += strlen(KeyInput + strPointer);
			}

			ZeroMemory(output, MAX_RSA_MODULUS_LEN+1);
			inputLen = strlen((char*)input);

			//MessageBox(NULL, L"Start encryption cycle", L"Status", MB_OK);

			if (RSAPrivateEncrypt(output, &outputLen, input, inputLen, &privateKey)==RE_LEN) {
				
				MessageBox(NULL, L"Error encryption cycle", L"Error", MB_OK);
                return -1;
                }
            else {
				
                for (int i=0; i<outputLen; i++) {

					HexValue[2] = 0x00;
					HexValue[0] = hexval[(output[i] >> 4) & 0xF];
					HexValue[1] = hexval[output[i] & 0x0F];
					strcat(KeyOutput, HexValue);
                    }
				};

		//MessageBox(NULL, L"End encryption cycle", L"Status", MB_OK);


        } while (strlen(KeyInput + strPointer)>0);

		fwrite(KeyOutput, 1, strlen(KeyOutput), fileHandle);

		fclose(fileHandle);

		//MessageBox(NULL, L"Done", L"Done", MB_OK);

		return 0; 
}

// This is the constructor of a class that has been exported.
// see rsadll.h for the class definition
Crsadll::Crsadll()
{
	return;
}
