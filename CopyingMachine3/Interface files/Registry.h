// Registry.h: class for saving/loading settings to and from the registry
//
#pragma once

#include "XMLDocument.h"

enum eRegistryType {eWindowsRegistry, eXMLDocument};

#define SettingsFile "copyingmachine_settings.xml"
#define RegistryBase "software\\Meusesoft\\Copying Machine 3\\"
#define TopNodeName "Settings"
#define QueryTopNode "//Settings"

class CRegistry {

public:

	CRegistry();
	~CRegistry();

	//methods for reading/writing from/to the registry (or another way of storing preferences
	//and settings)
	int ReadInt(std::wstring pKey, std::wstring pEntry, int piDefaultValue);
	void WriteInt(std::wstring pKey, std::wstring pEntry, int piValue);
	
	void WriteString(std::wstring pKey, std::wstring pEntry, std::wstring psValue);
	void ReadString(std::wstring pKey, std::wstring pEntry, std::wstring psDefaultValue, std::wstring &psValue);

	static void ReadClass(std::wstring pKey, std::wstring pEntry, std::wstring psDefaultValue, std::wstring &psValue);
	static void WriteClass(std::wstring pKey, std::wstring psValue);
	static void NukeClass(std::wstring pKey);

	int ReadIntLocalMachine(std::wstring pKey, std::wstring pEntry, int piDefaultValue);
	void WriteIntLocalMachine(std::wstring pKey, std::wstring pEntry, int piValue);

private:

	int ReadIntWindows(HKEY phKey, std::wstring pKey, std::wstring pEntry, int piDefaultValue);
	void WriteIntWindows(HKEY phKey, std::wstring pKey, std::wstring pEntry, int piValue);
	
	void WriteStringWindows(std::wstring pKey, std::wstring pEntry, std::wstring psValue);
	void ReadStringWindows(std::wstring pKey, std::wstring pEntry, std::wstring psDefaultValue, std::wstring &psValue);

	int ReadIntXML(std::wstring pKey, std::wstring pEntry, int piDefaultValue);
	void WriteIntXML(std::wstring pKey, std::wstring pEntry, int piValue);
	
	void WriteStringXML(std::wstring pKey, std::wstring pEntry, std::wstring psValue);
	void ReadStringXML(std::wstring pKey, std::wstring pEntry, std::wstring psDefaultValue, std::wstring &psValue);


	CXMLDocument* oXMLDocument;
	eRegistryType eType;
};