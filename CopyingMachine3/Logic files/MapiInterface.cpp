#include "stdafx.h"
#include "MapiInterface.h"

//----------------------------------------------------------------------------------------------

CMapiInterface::CMapiInterface() {

  	// Initialize MAPI
	hMAPI = LoadLibrary(L"MAPI32.DLL");
  	if (hMAPI == NULL) {
   		MAPIFailure(L"Unable to find MAPI32 Library");
		}
   else {

      (FARPROC&)m_lpfnMAPISendMail = GetProcAddress(hMAPI, "MAPISendMail");
      if (m_lpfnMAPISendMail == NULL) {
            MAPIFailure(L"Unable to find MAPISendMail");
         }

      (FARPROC&)m_lpfnMAPILogon = GetProcAddress(hMAPI, "MAPILogon");
      if (m_lpfnMAPILogon == NULL) {
            MAPIFailure(L"Unable to find MAPILogon");
         }

      (FARPROC&)m_lpfnMAPILogoff = GetProcAddress(hMAPI, "MAPILogoff");
      if (m_lpfnMAPILogoff == NULL) {
            MAPIFailure(L"Unable to find MAPILogoff");
         }
		}
	}
//----------------------------------------------------------------------------------------------
CMapiInterface::~CMapiInterface() {


	if (hMAPI!=NULL) {

		//release the MAPI library
		FreeLibrary(hMAPI);
		}
	}
//----------------------------------------------------------------------------------------------
bool
CMapiInterface::bMapiLoaded() {

   return hMAPI!=NULL;
	}
//----------------------------------------------------------------------------------------------
ULONG
CMapiInterface::SendFile(std::wstring psFilename, std::wstring psFilePath) {

	ULONG lReturnValue;
	MapiMessage msg;
	MapiFileDesc atch;

	char sPath[MAX_PATH];
	char sFilename[MAX_PATH];

   if (bMapiLoaded()) {

      // Build Attachment
      if (psFilename.length()>0) {
			
			CW2A sConversionPath( psFilePath.c_str() );
			CW2A sConversionFilename( psFilename.c_str() );

			strcpy_s(sPath, MAX_PATH, sConversionPath);
			strcpy_s(sFilename, MAX_PATH, sConversionFilename);

            atch.ulReserved = 0;
            atch.flFlags = 0;
            atch.nPosition = (ULONG)-1;
            atch.lpszPathName = sPath;
            atch.lpszFileName = sFilename;
            atch.lpFileType = NULL;

            // Attach it to the message
            msg.nFileCount = 1;
            msg.lpFiles = &atch;
         }
      else {
            msg.nFileCount = 0;
            msg.lpFiles = NULL;
         }

      // Message Structure
      msg.ulReserved = 0;
      msg.lpszSubject = NULL;
      msg.lpszNoteText = NULL;
      msg.lpszMessageType = NULL;
      msg.lpszDateReceived = NULL;
      msg.lpszConversationID = NULL;
      msg.flFlags = 0;
      msg.lpOriginator = NULL;
      msg.nRecipCount = 0;
      msg.lpRecips = NULL;

      lReturnValue = (m_lpfnMAPISendMail(0, 0, &msg, MAPI_DIALOG, 0));

		switch (lReturnValue) {

		case SUCCESS_SUCCESS:
			break;

		case MAPI_E_TOO_MANY_RECIPIENTS:
		case MAPI_E_UNKNOWN_RECIPIENT:
		case MAPI_E_AMBIGUOUS_RECIPIENT:
			MAPIFailure(L"Unable to resolve recipient");
			break;

         case MAPI_E_ATTACHMENT_NOT_FOUND:
         case MAPI_E_ATTACHMENT_OPEN_FAILURE:
               MAPIFailure(L"Unable to attach file");
               break;

         case MAPI_E_BAD_RECIPTYPE:
            MAPIFailure(L"Unknown recipient type");
            break;

         case MAPI_E_INSUFFICIENT_MEMORY:
            MAPIFailure(L"Not enough memory to send a message");
            break;

         case MAPI_E_LOGIN_FAILURE:
            MAPIFailure(L"Not Logged In");
            break;

         case MAPI_E_TEXT_TOO_LARGE:
            MAPIFailure(L"Too much Message Text");
            break;

         case MAPI_E_TOO_MANY_FILES:
            MAPIFailure(L"Unable to attach file");
            break;

         case MAPI_E_USER_ABORT:
            //MAPIFailure("User Requested Mail Cancel");
            break;

         case MAPI_E_FAILURE:
         default:
            MAPIFailure(L"Undefined Failure");
         }
      }

	return lReturnValue;
	}

//----------------------------------------------------------------------------------------------
void
CMapiInterface::MAPIFailure(wchar_t *msg) {

	wprintf(L"\t%s\n", msg);
	MessageBox(NULL, msg, L"Error", MB_ICONERROR|MB_OK);
	}
//----------------------------------------------------------------------------------------------


/*




	ULONG MAPIResult;
	char msg[1024];
	char TempFile[MAX_PATH];

  	if ((MAPIResult = m_lpfnMAPILogon(0, NULL, NULL, 0, 0, &MAPISession)) != SUCCESS_SUCCESS) {
   		
		MAPIResult = m_lpfnMAPILogon(0, NULL, NULL, MAPI_LOGON_UI, 0, &MAPISession);
   
		switch (MAPIResult) {
   			case SUCCESS_SUCCESS:
    				break;

   			case MAPI_E_INSUFFICIENT_MEMORY:
  				MAPIFailure("Insufficient memory available.  Close some applications before attempting to transmit again.", MAPILOGONTITLE);
    				break;

   			case MAPI_E_LOGIN_FAILURE:
    				MAPIFailure("Invalid Mail Logon", MAPILOGONTITLE);
    				break;

   			case MAPI_E_TOO_MANY_SESSIONS:
    				MAPIFailure("Mail system busy.  Try to transmit later", MAPILOGONTITLE);
    				break;

   			case MAPI_E_USER_ABORT:
    				MAPIFailure("User requested to cancel mail", MAPILOGONTITLE);
    				break;

   			case MAPI_E_FAILURE:
   			default:
    				MAPIFailure("Unknown Mail System Failure", MAPILOGONTITLE);
   			}
  		}


	MAPIResult = SendFile("waremote@wakeassoc.com", "SendMail Test", "Message Body only.", NULL);
  
	switch (MAPIResult) {
  		case SUCCESS_SUCCESS:
   		break;

  		case MAPI_E_TOO_MANY_RECIPIENTS:
  		case MAPI_E_UNKNOWN_RECIPIENT:
  		case MAPI_E_AMBIGUOUS_RECIPIENT:
   			MAPIFailure("Unable to resolve 'waremote@wakeassoc.com'", MAPISENDTITLE);
   			break;

  		case MAPI_E_ATTACHMENT_NOT_FOUND:
  		case MAPI_E_ATTACHMENT_OPEN_FAILURE:
   			MAPIFailure("Unable to attach file", MAPILOGONTITLE);
   			break;

  		case MAPI_E_BAD_RECIPTYPE:
   			MAPIFailure("Unknown recipient type 'mailto:waremote@wakeassoc.com'",MAPISENDTITLE);
   			break;

  		case MAPI_E_INSUFFICIENT_MEMORY:
   			MAPIFailure("Not enough memory to send a message", MAPISENDTITLE);
   			break;

  		case MAPI_E_LOGIN_FAILURE:
   			MAPIFailure("Not Logged In", MAPISENDTITLE);
   			break;

  		case MAPI_E_TEXT_TOO_LARGE:
   			MAPIFailure("Too much Message Text", MAPISENDTITLE);
   			break;

  		case MAPI_E_TOO_MANY_FILES:
   			MAPIFailure("Unable to attach file", MAPISENDTITLE);
   			break;

  		case MAPI_E_USER_ABORT:
   			MAPIFailure("User Requested Mail Cancel", MAPISENDTITLE);
   			break;

  		case MAPI_E_FAILURE:
  		default:
   			MAPIFailure("Undefined Failure", MAPISENDTITLE);
  		}
		
  	if (CreateTempFile(TempFile)) {
   		sprintf(msg, "Temporary File '%s' attached\nContents are file name.", TempFile);
   		printf("Sending Message with file attachment\n");
   		MAPIResult = SendFile("waremote@wakeassoc.com", "SendMail Test", msg, TempFile);
   		DeleteFile(TempFile);

   		switch (MAPIResult) {
   
			case SUCCESS_SUCCESS:
    				break;

   			case MAPI_E_TOO_MANY_RECIPIENTS:
   			case MAPI_E_UNKNOWN_RECIPIENT:
   			case MAPI_E_AMBIGUOUS_RECIPIENT:
   				MAPIFailure("Unable to resolve 'waremote@wakeassoc.com'", MAPISENDTITLE);
    				break;

   			case MAPI_E_ATTACHMENT_NOT_FOUND:
   			case MAPI_E_ATTACHMENT_OPEN_FAILURE:
   	 			MAPIFailure("Unable to attach file", MAPILOGONTITLE);
    				break;

   			case MAPI_E_BAD_RECIPTYPE:
    				MAPIFailure("Unknown recipient type 'mailto:waremote@wakeassoc.com'", MAPISENDTITLE);
    				break;

   			case MAPI_E_INSUFFICIENT_MEMORY:
    				MAPIFailure("Not enough memory to send a message", MAPISENDTITLE);
    				break;

   			case MAPI_E_LOGIN_FAILURE:
    				MAPIFailure("Not Logged In", MAPISENDTITLE);
    				break;

   			case MAPI_E_TEXT_TOO_LARGE:
    				MAPIFailure("Too much Message Text", MAPISENDTITLE);
    				break;

   			case MAPI_E_TOO_MANY_FILES:
    				MAPIFailure("Unable to attach file", MAPISENDTITLE);
    				break;

   			case MAPI_E_USER_ABORT:
    				MAPIFailure("User Requested Mail Cancel", MAPISENDTITLE);
    				break;

   			case MAPI_E_FAILURE:
   			default:
    				MAPIFailure("Undefined Failure", MAPISENDTITLE);
   			}
  		}

	m_lpfnMAPILogoff(MAPISession, 0, 0, 0);

  	MessageBox(NULL, "Your system can mail files in background.", "TestMail Success", MB_ICONINFORMATION|MB_OK);
  	exit(0);
	}
ULONG
SendFile(char *mailto, char *subject, char *note, char *file) {

	MapiMessage msg;
	MapiRecipDesc recip;
	MapiFileDesc atch;
	char *p;
	char internet[1000] = "";
	char Path[MAX_PATH];
	char *f;

	// Assemble full MAPI address
	p = mailto;
	while (*p) {
	if (*p++ == '@') {
		strcpy(internet, "SMTP:");
		break;
		}
	}
  	
	strcat(internet, mailto);

	// Remove trailing punctuation

  	// Build Recipient
  	recip.ulReserved = 0;
  	recip.ulRecipClass = MAPI_TO;
  	recip.lpszName = internet;
  	recip.lpszAddress = internet;
  	recip.ulEIDSize = 0;
  	recip.lpEntryID = NULL;

  	// Build Attachment
  	if (file) {
   		GetFullPathName(file, MAX_PATH, Path, &f);

		atch.ulReserved = 0;
		atch.flFlags = 0;
   		atch.nPosition = (ULONG)-1;
   		atch.lpszPathName = file;
   		atch.lpszFileName = f;
   		atch.lpFileType = NULL;

   		// Attach it to the message
   		msg.nFileCount = 1;
   		msg.lpFiles = &atch;
  		} 
	else {
   		msg.nFileCount = 0;
   		msg.lpFiles = NULL;
  		}

  	// Message Structure
  	msg.ulReserved = 0;
  	msg.lpszSubject = subject;
  	msg.lpszNoteText = note;
  	msg.lpszMessageType = NULL;
  	msg.lpszDateReceived = NULL;
  	msg.lpszConversationID = NULL;
  	msg.flFlags = 0;
  	msg.lpOriginator = NULL;
  	msg.nRecipCount = 1;
  	msg.lpRecips = &recip;

  	return(m_lpfnMAPISendMail(MAPISession, 0, &msg, 0, 0));
 	}
*/

