#pragma once

class CRegistration {

	public:

		CRegistration(sGlobalInstances pcGlobalInstances);
		~CRegistration();

		bool CheckRegistration();
		void Reminder(HWND phWnd);

		bool PlaceKeyText(std::wstring psKeyText);
		bool PlaceKeyFile(std::wstring psFilename);

		std::wstring sName;
		std::wstring sOrganisation;
		bool bRegistered;
		
		std::wstring sHash;

	private:

		char* GetRegistrationXML();
		void  GetHash();

		int ConvertHexDoubleDigit(char* cInput);
		int ConvertHexDigit(char cInput);

		sGlobalInstances oGlobalInstances;
		CRegistry*		 oRegistry;
		CTracer*		 oTrace;

	};