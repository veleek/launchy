#pragma once

#include <vector>
#include <map>
#include <boost/regex.hpp>
#include "FileRecord.h"
#include "Options.h"

using namespace std;

typedef uint PluginID;


/*
	Plugins Can:
	. Add items to the main Launchy list which can optionally point to their plugin when executed or selected
	. Register ownership of searches that match regular expressions
	. Store data in the launchy db
	. Store configuration options
*/

/*
	The plugin API: Functions that the plugins can call

//	Launchy_SetTimer // So that plugins can have timed events
	Launchy_GetIcon // Get the icon for a file
	Launchy_GetSpecialPath // Path to special folder
	Launchy_Launch // Use Launchy to launch a file
	Launchy_Search(txt, List) // Tell Launchy to use its standard search algorithm on the list
*/



struct SearchResult {
	TCHAR* DisplayString; // Must be set
	TCHAR* FullPath; // Can be NULL
	TCHAR* Location; // Can be NULL
	HICON DisplayIcon; // Make sure you 0 this on creation!!
};

struct IndexItem {
	char* DisplayString;
	char* FullPath; // Can be NULL
};



typedef vector<SearchResult> SearchResults;
typedef vector<IndexItem> IndexItems;

typedef TCHAR* (* PLUGINGETREGEXS) (int*);
typedef IndexItems (* PLUGINGETINDEXITEMS) (void);
typedef SearchResult* (* PLUGINUPDATESEARCH) (	int NumStrings, const TCHAR* Strings, const TCHAR* FinalString, int* NumResults);
typedef SearchResult* (* PLUGINFILEOPTIONS) (const TCHAR* FullPath, int NumStrings, const TCHAR* Strings,  const TCHAR* FinalString, int* NumResults );
typedef void (* PLUGINDOACTION) (	int NumStrings, const TCHAR* Strings, const TCHAR* FinalString, const TCHAR* FullPath);
typedef void (* PLUGINADDINDEXITEMS) (IndexItems);
typedef  SearchResult* (* PLUGINGETIDENTIFIERS) (int*);
typedef void (* PLUGINFREERESULTS) ( SearchResult*, int);
typedef void (* PLUGINFREESTRINGS) ( TCHAR* );
typedef TCHAR* (* PLUGINGETSEPARATOR) (void);
typedef TCHAR* (* PLUGINGETNAME) (void);
typedef TCHAR* (* PLUGINGETDESCRIPTION) (void);
typedef void (* PLUGINCALLOPTIONSDLG) (HWND);
typedef void (* PLUGINCLOSE) (void);
typedef void (* PLUGININITIALIZE) (void);
typedef void (* PLUGINGETSTORAGE) (int* NumItems, TCHAR** ItemNames, TCHAR** ItemValues);
typedef void (* PLUGINSETSTORAGE) (int NumItems, TCHAR* ItemNames, TCHAR* ItemValues);
typedef bool (* PLUGINHASOPTIONSDLG) (void);


struct PluginFunctions {
	PLUGINGETREGEXS PluginGetRegexs;
	PLUGINGETINDEXITEMS PluginGetIndexItems;
	PLUGINUPDATESEARCH PluginUpdateSearch;
	PLUGINFILEOPTIONS PluginFileOptions;
	PLUGINDOACTION PluginDoAction;
	PLUGINGETIDENTIFIERS PluginGetIdentifiers;
	PLUGINGETSTORAGE PluginGetStorage;
	PLUGINSETSTORAGE PluginSetStorage;
	PLUGINFREERESULTS PluginFreeResults;
	PLUGINFREESTRINGS PluginFreeStrings;
	PLUGINGETSEPARATOR PluginGetSeparator;
	PLUGINGETNAME PluginGetName;
	PLUGINGETDESCRIPTION PluginGetDescription;
	PLUGINCALLOPTIONSDLG PluginCallOptionsDlg;
	PLUGININITIALIZE PluginInitialize;
	PLUGINCLOSE PluginClose;
	PLUGINHASOPTIONSDLG PluginHasOptionsDlg;
};

struct DLLProperties {
	CString name;
	CString filename;
	CString description;
	bool loaded;
	bool hasOptionsDlg;
};



class Plugin
{
	struct DLLInstance {
		HINSTANCE handle;
		vector<boost::wregex> regexs;
		CString name;
		unsigned long nametag;
		CString filename;
		CString description;
	};

private:
	vector<PluginFunctions> pfuncs;
	vector<DLLInstance> loadedPlugins;
	void LoadRegExs();
public:
	
	vector<DLLProperties> allPlugins;
	Plugin(void);
	~Plugin(void);

	void LoadDlls(bool FirstLoad = true);
	vector<FileRecordPtr> Plugin::GetIdentifiers(Options*);
	void Launch(int PluginID, TCHAR* FullPath);
	int IsSearchOwned(CString searchTxt);
	std::shared_ptr<vector<FileRecordPtr> > GetSearchOptions(int owner);
	CString Plugin::GetSeparator(int PluginID);
	unsigned long Plugin::GetPluginNameTag(int id);
	HICON Plugin::GetIcon(int id);
	void CallOptionsDlg(const DLLProperties &, HWND);
	void Plugin::GetStorage(int id, Options*);
	void SendStorage(CString PluginName, PLUGINSETSTORAGE);
	/*

	LoadDlls(void);
	UnloadDlls(void);

	// Initialization Functions
	PluginID PluginOwnsSearch(CString txt); // Defaults to 0 (for launchy)
	IndexItems PluginGetIndexItems();
	
	// Runtime Functions
	SearchResults PluginDoSearch(PluginID, CString txt);
	void PluginDoAction(PluginID, uint searchID);

	// Indexing Functions
	IndexItems
	// Shutdown Functions
	*/
};


