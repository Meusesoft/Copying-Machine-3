#include <mapi.h>

class CMapiInterface {

	public:
		CMapiInterface();
		~CMapiInterface();

		bool bMapiLoaded();

		ULONG SendFile(std::wstring psFilename, std::wstring psFilePath);

	private:

		HINSTANCE hMAPI;
		HANDLE MAPISession;
		ULONG (PASCAL *m_lpfnMAPISendMail)(LHANDLE, ULONG, lpMapiMessage, FLAGS, ULONG);
		ULONG (PASCAL *m_lpfnMAPILogon)(ULONG, LPTSTR, LPTSTR, FLAGS, ULONG, LPLHANDLE);
		ULONG (PASCAL *m_lpfnMAPILogoff)(LHANDLE, ULONG, FLAGS, ULONG);

		void MAPIFailure(wchar_t *);
	};

   