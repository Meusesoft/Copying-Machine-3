object Form1: TForm1
  Left = 215
  Top = 178
  Width = 696
  Height = 480
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label4: TLabel
    Left = 224
    Top = 248
    Width = 36
    Height = 13
    Caption = 'Crypted'
  end
  object Label5: TLabel
    Left = 456
    Top = 248
    Width = 49
    Height = 13
    Caption = 'Decrypted'
  end
  object Label3: TLabel
    Left = 8
    Top = 232
    Width = 28
    Height = 13
    Caption = 'Naam'
  end
  object Label6: TLabel
    Left = 8
    Top = 288
    Width = 53
    Height = 13
    Caption = 'Organisatie'
  end
  object Label7: TLabel
    Left = 8
    Top = 344
    Width = 54
    Height = 13
    Caption = 'E-mailadres'
  end
  object Label2: TLabel
    Left = 24
    Top = 120
    Width = 50
    Height = 13
    Caption = 'Key name:'
  end
  object Label9: TLabel
    Left = 8
    Top = 392
    Width = 9
    Height = 13
    Caption = 'Id'
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 673
    Height = 89
    Caption = 'Key'
    TabOrder = 11
    object Label8: TLabel
      Left = 208
      Top = 24
      Width = 54
      Height = 13
      Caption = 'Private Key'
    end
    object Label1: TLabel
      Left = 208
      Top = 56
      Width = 50
      Height = 13
      Caption = 'Public Key'
    end
  end
  object Button1: TButton
    Left = 24
    Top = 32
    Width = 97
    Height = 25
    Caption = 'Generate Key'
    TabOrder = 0
    OnClick = Button1Click
  end
  object txtOrgMessage: TMemo
    Left = 472
    Top = 104
    Width = 201
    Height = 49
    TabOrder = 1
    Visible = False
  end
  object txtCryptedMessage: TMemo
    Left = 224
    Top = 264
    Width = 217
    Height = 169
    TabOrder = 2
  end
  object txtDecryptedMessage: TMemo
    Left = 456
    Top = 264
    Width = 217
    Height = 169
    TabOrder = 3
  end
  object Button2: TButton
    Left = 224
    Top = 216
    Width = 75
    Height = 25
    Caption = 'Encrypt'
    Enabled = False
    TabOrder = 4
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 456
    Top = 216
    Width = 75
    Height = 25
    Caption = 'Decrypt'
    Enabled = False
    TabOrder = 5
    OnClick = Button3Click
  end
  object txtVoornaam: TEdit
    Left = 8
    Top = 248
    Width = 209
    Height = 21
    TabOrder = 6
    Text = 'Rainier Maas'
  end
  object txtAchternaam: TEdit
    Left = 8
    Top = 304
    Width = 209
    Height = 21
    TabOrder = 7
    Text = '-'
  end
  object txtEmailadres: TEdit
    Left = 8
    Top = 360
    Width = 209
    Height = 21
    TabOrder = 8
    Text = 'rainier@meusesoft.com'
  end
  object txtNewPrivateKeyFilename: TEdit
    Left = 280
    Top = 24
    Width = 385
    Height = 21
    TabOrder = 9
    Text = 'CopyingMachinePrivate.key'
  end
  object txtNewPublicKeyFilename: TEdit
    Left = 280
    Top = 56
    Width = 385
    Height = 21
    TabOrder = 10
    Text = 'CopyingMachinePublic.key'
  end
  object txtKeyName: TEdit
    Left = 80
    Top = 120
    Width = 281
    Height = 21
    TabOrder = 12
    Text = 'CopyingMachine'
  end
  object Button4: TButton
    Left = 224
    Top = 184
    Width = 89
    Height = 25
    Caption = 'Export Public Key'
    TabOrder = 13
    OnClick = Button4Click
  end
  object txtID: TEdit
    Left = 8
    Top = 408
    Width = 209
    Height = 21
    TabOrder = 14
    Text = '1'
  end
end
