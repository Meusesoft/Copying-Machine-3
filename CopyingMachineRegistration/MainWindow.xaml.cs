using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Runtime.InteropServices;
using System.IO;
using System.Net.Mail;

namespace CopyingMachineRegistration
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        [DllImport(@"rsadll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int GenerateKey(char* KeyFile, byte[] input, int inputLen);

        static unsafe int Key(String KeyFile, byte[] src, int srcIndex)
        {
            char* strKeyFile = (char*)(void*)Marshal.StringToHGlobalAnsi(KeyFile);

            return GenerateKey(strKeyFile, src, srcIndex);
        }

        private void btnGenerate_Click(object sender, RoutedEventArgs e)
        {
            GenerateKeyAndSend(txtName.Text, txtOrganisation.Text, txtEmailaddress.Text, chkSendMail.IsChecked == true);
        }

        private void GenerateKeyAndSend(String Name, String Organisation, String EmailAddress, bool SendMail) 
        {
            //Opbouwen compleet bericht
            String  MessageToEncrypt ="<xml><applicatie name=\"Copying Machine\" version=\"2.00\"/>";
                MessageToEncrypt+="<user name=\"";
                MessageToEncrypt += Name;
                MessageToEncrypt+="\" organisation=\"";
                MessageToEncrypt += Organisation;
                MessageToEncrypt+="\" id=\"";
                MessageToEncrypt += System.DateTime.Now.ToShortDateString() + System.DateTime.Now.ToShortTimeString();
                MessageToEncrypt+="\" emailaddress=\"";
                MessageToEncrypt += EmailAddress;
                MessageToEncrypt+="\"/></xml>";

            System.Text.ASCIIEncoding  encoding=new System.Text.ASCIIEncoding();
            Byte[] bytes = encoding.GetBytes(MessageToEncrypt);
            byte[] dataToEncrypt = new byte[1 + bytes.Length];

            //Add the checksum to the xml
            int iChecksum = 0;
            for (int i=0; i<MessageToEncrypt.Length; i++) {
                iChecksum = iChecksum + bytes[i];
                dataToEncrypt[i+1] = bytes[i];
                }

            iChecksum = (iChecksum & 0x07);
            iChecksum = (iChecksum << 5 ) & 0xFF;
            iChecksum = iChecksum | 0x20;

            dataToEncrypt[0] = Convert.ToByte(iChecksum);

            //Construct the name of the keyfile to be generated
            String KeyFile;

            KeyFile="Keys\\copymach_";
            KeyFile+=Name.ToLower();
            KeyFile+=".cmk";
            KeyFile = KeyFile.Replace(" ", "");

            if (!Directory.Exists("Keys"))
            {
                Directory.CreateDirectory("Keys");
            }

            //Create the key
            Key(KeyFile, dataToEncrypt, dataToEncrypt.Length);

            //Send e-mail
            if (SendMail) MailKey(KeyFile, EmailAddress);
        }

        private void btnClear_Click(object sender, RoutedEventArgs e)
        {
            txtName.Text = "";
            txtOrganisation.Text = "";
            txtEmailaddress.Text = "";
        }

        private void MailKey(String KeyFile, String Recipient)
        {

            MailMessage eMail = new MailMessage(); ;

            eMail.From = new MailAddress("rainier@meusesoft.com", "Rainier Maas");
            eMail.To.Add(new MailAddress(Recipient));
            eMail.Bcc.Add(new MailAddress("rainier@meusesoft.com", "Rainier Maas"));
            eMail.Subject = "Registration key Copying Machine";

            String sEmailBody = "Hello,\nthank you for purchasing Copying Machine. Attached to this e-mail is a file called '" + System.IO.Path.GetFileName(KeyFile) + "'. This file contains your personal registration key. To use this key place it on your harddrive in for example the folder'my documents'. After you have saved the file, you can open Copying Machine, wait for the reminder screen to disappear and go to the 'Help' menu and open the 'About' item. A dialog will appear containing information about the version of Copying Machine and the registration. Press here 'Place key'. Another dialog will appear, press here 'Load...'. Browse to the location where you have saved the registration file, open this file and after that Copying Machine will be registered. ";
            sEmailBody += "\nIf you delete the registration key on accident, now or in the future. Don't hesitate to contact me. I will send you a copy of it.";
            sEmailBody += "\nOnce again, thank you for purchasing Copying Machine and I hope you will enjoy it!";
            sEmailBody += "\nRegards,";
            sEmailBody += "\nRainier Maas";

            sEmailBody.Replace("%s", System.IO.Path.GetFileName(KeyFile));

            eMail.Body = sEmailBody;


            eMail.Attachments.Add(new Attachment(KeyFile));

            //Send the e-mail
            SmtpClient SMTP = new SmtpClient("smtp.ziggo.nl");
            SMTP.Send(eMail);
        }
 
    private void btnImport_Click(object sender, RoutedEventArgs e)
        {
        // loop through e-mails
            string[] Files;
        
            Files = System.IO.Directory.GetFiles("import");

            foreach (string Filename in Files) {

                ProcessEmail(Filename);
            }
        }

    private void ProcessEmail(string eMail) 
        {

        
        
        //move e-mail to processed folder
        System.IO.File.Move(eMail, "import\\processed");
       }
    }
}
