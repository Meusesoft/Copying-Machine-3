//---------------------------------------------------------------------------

#ifndef TestformH
#define TestformH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
    TButton *Button1;
    TMemo *txtOrgMessage;
    TMemo *txtCryptedMessage;
    TMemo *txtDecryptedMessage;
    TLabel *Label4;
    TLabel *Label5;
    TButton *Button2;
    TButton *Button3;
    TEdit *txtVoornaam;
    TEdit *txtAchternaam;
    TEdit *txtEmailadres;
    TLabel *Label3;
    TLabel *Label6;
    TLabel *Label7;
    TEdit *txtNewPrivateKeyFilename;
    TEdit *txtNewPublicKeyFilename;
    TGroupBox *GroupBox1;
    TLabel *Label8;
    TLabel *Label1;
    TEdit *txtKeyName;
    TLabel *Label2;
    TButton *Button4;
    TLabel *Label9;
    TEdit *txtID;
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall Button2Click(TObject *Sender);
    void __fastcall Button3Click(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall Button4Click(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
    R_RSA_PUBLIC_KEY publicKey;
    R_RSA_PRIVATE_KEY privateKey;

    unsigned char cryptedstring[MAX_RSA_MODULUS_LEN + 1]; /* input block */
    unsigned int cryptedstringLen; /* length of input block */


public:		// User declarations
    __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
