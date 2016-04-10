//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop


#include "stdio.h"
#include "sti.h"
#include "rsa.h"
#include "Testform.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
  R_RANDOM_STRUCT randomStruct;
  R_RSA_PROTO_KEY protoKey;
  int status;

  /* Initialise random structure ready for keygen */
  R_RandomCreate(&randomStruct);

  /* Initialise prototype key structure */
  protoKey.bits=512;
  protoKey.useFermat4 = 1;

  /* Generate keys */
  status = R_GeneratePEMKeys(&publicKey, &privateKey, &protoKey, &randomStruct);
  if (status)
    {
    MessageBox(NULL, "Error", "Error", MB_OK);
    return;
    }

  FILE* fileHandle;

  //save private key in binary format

    fileHandle = fopen(txtNewPrivateKeyFilename->Text.c_str(), "w+b");

  if (fileHandle!=NULL) {

     int byteswritten;
     AnsiString test;

     byteswritten = fwrite(&privateKey, 1, sizeof(privateKey), fileHandle);

     if (byteswritten<sizeof(privateKey)) {
        test = byteswritten;
        MessageBox(NULL, test.c_str(), "Error saving Private key", MB_OK);
        return;
        };
     fclose(fileHandle);
     }
  else {
    MessageBox(NULL, "Error", "Error saving Private key", MB_OK);
    return;
    }

  //save public key in binary format

  fileHandle = fopen(txtNewPublicKeyFilename->Text.c_str(), "w+b");

  if (fileHandle!=NULL) {

     fwrite(&publicKey, sizeof(publicKey), 1, fileHandle);
     fclose(fileHandle);
     }
  else {
    MessageBox(NULL, "Error", "Error saving Public key", MB_OK);
    return;
    }


 FormShow(Sender);




}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{
    unsigned char output[MAX_RSA_MODULUS_LEN + 1]; /* output block */
    unsigned int outputLen; /* length of output block */
    unsigned char input[MAX_RSA_MODULUS_LEN + 1]; /* input block */
    unsigned int inputLen; /* length of input block */

    txtCryptedMessage->Lines->Clear();

    AnsiString MessageToEncrypt;
    AnsiString MessageFragment;

    int maxMessageLength = 21;

    //Opbouwen compleet bericht
    MessageToEncrypt ="<xml><applicatie name=\"Copying Machine\" version=\"2.00\"/>";
    MessageToEncrypt+="<user name=\"";
    MessageToEncrypt+= txtVoornaam->Text;
    MessageToEncrypt+="\" organisation=\"";
    MessageToEncrypt+= txtAchternaam->Text;
    MessageToEncrypt+="\" id=\"";
    MessageToEncrypt+= txtID->Text;
    MessageToEncrypt+="\" emailaddress=\"";
    MessageToEncrypt+= txtEmailadres->Text;
    MessageToEncrypt+="\"/></xml>";

    char cForChecksum[2048];

    strcpy(cForChecksum, MessageToEncrypt.c_str());

    int iChecksum;
    for (int i=0; i<strlen(cForChecksum); i++) {
        iChecksum = iChecksum + cForChecksum[i];
        }

    iChecksum = iChecksum & 0xFF;
    iChecksum = (iChecksum << 5 ) & 0xFF;

    MessageFragment = (char)iChecksum;
    MessageFragment += MessageToEncrypt;
    MessageToEncrypt = MessageFragment;

    txtOrgMessage->Lines->Add(MessageToEncrypt);
  //  MessageToEncrypt = "abc";

    FILE* hFileHandle;

    hFileHandle = fopen("registration.cmk", "w+t");

    do {
        if (MessageToEncrypt.Length()>maxMessageLength) {
            MessageFragment = MessageToEncrypt.SubString(0, maxMessageLength);
            }
        else {
            MessageFragment = MessageToEncrypt.SubString(0, MessageToEncrypt.Length());
            }

       strcpy(input, MessageFragment.c_str());
       ZeroMemory(&output, MAX_RSA_MODULUS_LEN+1);
       inputLen = MessageFragment.Length();
       
        if (RSAPrivateEncrypt(output, &outputLen, input, inputLen, &privateKey)==RE_LEN) {
            MessageBox(NULL, "Error", "Error", MB_OK);
            return;
            }
        else {
            AnsiString print;
            print = "";

            for (int i=0; i<outputLen; i++) {
                print += IntToHex(output[i], 2);
                }

        if (hFileHandle!=NULL) {
            fwrite(print.c_str(), 1, print.Length(), hFileHandle);
            }

        txtCryptedMessage->Lines->Add(print);

        if (MessageToEncrypt.Length()>maxMessageLength) {
            MessageToEncrypt = MessageToEncrypt.SubString(maxMessageLength+1, MessageToEncrypt.Length()-maxMessageLength);
            }
        else {
            MessageToEncrypt = "";
            }

        };
    } while (MessageToEncrypt.Length()>0);

    if (hFileHandle!=NULL) {
        fclose(hFileHandle);
        }


}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button3Click(TObject *Sender)
{
    char output[MAX_RSA_MODULUS_LEN + 1]; /* output block */
    unsigned int outputLen; /* length of output block */
    char input[MAX_RSA_MODULUS_LEN + 1]; /* input block */
    unsigned int inputLen; /* length of input block */

    AnsiString OutputMessage;
    AnsiString temp;
    temp="";
    OutputMessage="";

    int startrow=0;

   txtDecryptedMessage->Lines->Clear();

    do {

    temp="";
    for (int i=0; i<4; i++) {
        temp+=txtCryptedMessage->Lines->Strings[i+startrow];
       }


    inputLen = temp.Length() / 2;

    HexToBin(temp.c_str(), input, inputLen);

    if (RSAPublicDecrypt(output, &outputLen, input, inputLen, &publicKey)!=RE_LEN) {


        temp = output;
        OutputMessage += temp;
      ZeroMemory(&output, MAX_RSA_MODULUS_LEN+1);

        }

        startrow += 4;

    } while (startrow<=(txtCryptedMessage->Lines->Count-4));

    txtDecryptedMessage->Lines->Add(OutputMessage);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::FormShow(TObject *Sender)
{

    FILE* fileHandle;
    AnsiString filename;

    ZeroMemory(&privateKey, sizeof(privateKey));
    ZeroMemory(&publicKey, sizeof(publicKey));

    filename = txtKeyName->Text + "private.key";

    fileHandle = fopen(filename.c_str(), "rb");

    if (fileHandle!=NULL) {
       AnsiString test;
       int bytesread;

       bytesread = fread(&privateKey, 1, sizeof(privateKey), fileHandle);

       if (bytesread<sizeof(privateKey)) {
          test = bytesread;
          MessageBox(NULL, test.c_str(), "Error loading Private key", MB_OK);
         return;
          };
       fclose(fileHandle);
       }
    else {
         MessageBox(NULL, "Error", "Error loading Private key", MB_OK);
         return;
         }

    filename = txtKeyName->Text + "public.key";

    fileHandle = fopen(filename.c_str(), "rb");

    if (fileHandle!=NULL) {
       fread(&publicKey, sizeof(publicKey), 1, fileHandle);
       fclose(fileHandle);
       }
    else {
         MessageBox(NULL, "Error", "Error loading Public key", MB_OK);
         return;
         }


     Button2->Enabled=true;
     Button3->Enabled=true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
  //save public key in text format
  unsigned char *cByte;
  int iByte;
  FILE* fileHandle;

  AnsiString Byte;
  char buffer[3];

  AnsiString filename2 = txtNewPublicKeyFilename->Text + "2";
  fileHandle = fopen(filename2.c_str(), "w+");

 if (fileHandle!=NULL) {

     cByte = (char *) malloc(sizeof(publicKey)+1);
     memcpy(cByte, &publicKey, sizeof(publicKey));

 		R_DIGEST_CTX md5ctxt;
		unsigned char digestOut[MAX_DIGEST_LEN];
		unsigned int stringlength, i, digestLen;
      char buffer[MAX_DIGEST_LEN*2+1];
      char digestformat[MAX_DIGEST_LEN*2+1];
      char Demostring[100];

      //strcpy(Demostring, "Hallo");
      /* Initialise MD5 context */
		if (R_DigestInit(&md5ctxt, DA_MD5) != ID_OK) {
			::MessageBox(NULL, "Error: Invalid digest type passed to R_DigestInit!\n", "Error", MB_OK);
			return; }

		/* Update the digest context a byte at a time */
		stringlength = sizeof(publicKey);
		for (i=0; i<stringlength; i++) {
			R_DigestUpdate(&md5ctxt, &cByte[i], 1); }

		/* Finalise digest context and print final value */
		R_DigestFinal(&md5ctxt, digestOut, &digestLen);
		for( i=0; i<digestLen; i++) {
			sprintf(buffer, "%02x:", digestOut[i]);
			}

			::MessageBox(NULL, digestformat, digestformat, MB_OK);



    char ByteBuffer[3];
    ByteBuffer[2] = (char)0;

     for (int i=0; i<sizeof(publicKey); i++) {
         switch (cByte[i]/16) {
            case 0: {ByteBuffer[0]=48; break;}
            case 1: {ByteBuffer[0]=49; break;}
            case 2: {ByteBuffer[0]=50; break;}
            case 3: {ByteBuffer[0]=51; break;}
            case 4: {ByteBuffer[0]=52; break;}
            case 5: {ByteBuffer[0]=53; break;}
            case 6: {ByteBuffer[0]=54; break;}
            case 7: {ByteBuffer[0]=55; break;}
            case 8: {ByteBuffer[0]=56; break;}
            case 9: {ByteBuffer[0]=57; break;}
            case 10: {ByteBuffer[0]=65; break;}
            case 11: {ByteBuffer[0]=66; break;}
            case 12: {ByteBuffer[0]=67; break;}
            case 13: {ByteBuffer[0]=68; break;}
            case 14: {ByteBuffer[0]=69; break;}
            case 15: {ByteBuffer[0]=70; break;}
            }

         switch (cByte[i]%16) {
            case 0: {ByteBuffer[1]=48; break;}
            case 1: {ByteBuffer[1]=49; break;}
            case 2: {ByteBuffer[1]=50; break;}
            case 3: {ByteBuffer[1]=51; break;}
            case 4: {ByteBuffer[1]=52; break;}
            case 5: {ByteBuffer[1]=53; break;}
            case 6: {ByteBuffer[1]=54; break;}
            case 7: {ByteBuffer[1]=55; break;}
            case 8: {ByteBuffer[1]=56; break;}
            case 9: {ByteBuffer[1]=57; break;}
            case 10: {ByteBuffer[1]=65; break;}
            case 11: {ByteBuffer[1]=66; break;}
            case 12: {ByteBuffer[1]=67; break;}
            case 13: {ByteBuffer[1]=68; break;}
            case 14: {ByteBuffer[1]=69; break;}
            case 15: {ByteBuffer[1]=70; break;}
            }

         fwrite(&ByteBuffer, 2, 1, fileHandle);
         }

     free(cByte);
     fclose(fileHandle);
     }
  else {
    MessageBox(NULL, "Error", "Error saving Public key", MB_OK);
    return;
    }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
	//Register with Still Image
	wchar_t wAppPath[MAX_PATH];
	wchar_t wAppName[MAX_PATH];

	char cAppPath[MAX_PATH];
	HRESULT hRes;
	IStillImage * g_StillImage;

	strcpy(cAppPath, "c:\\");
	strcat(cAppPath, "copying.exe");

hRes = StiCreateInstance(GetModuleHandle(NULL), STI_VERSION,
                         &g_StillImage,NULL);

//	hRes = StiCreateInstance(GetModuleHandle(NULL), STI_VERSION, &g_StillImage,NULL);

	if (SUCCEEDED(hRes)) {
 
		MultiByteToWideChar(CP_ACP, NULL, "Copying Machine", -1, wAppName, sizeof(wAppName));
		MultiByteToWideChar(CP_ACP, NULL, cAppPath, -1, wAppPath, sizeof(wAppPath));


		if (g_StillImage->RegisterLaunchApplication(wAppName, wAppPath)!=S_OK) {
			::MessageBox(NULL, "Error registering launch Application", "Error", MB_OK);
			}

        g_StillImage->UnregisterLaunchApplication(wAppName);
        g_StillImage->Release();
		}

}
//---------------------------------------------------------------------------



