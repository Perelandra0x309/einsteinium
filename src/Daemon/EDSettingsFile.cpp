/*EDSettingsFile.cpp

*/
#include "EDSettingsFile.h"

EDSettingsFile::EDSettingsFile()
	: BHandler("name")
	,defaultRelaunchAction(ACTION_PROMPT)
	,_initStatus(B_ERROR)
{
	// find path for settings directory
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append(e_settings_dir);
	BDirectory settingsDir;
	status_t result = settingsDir.SetTo(settingsPath.Path());
	if( result == B_ENTRY_NOT_FOUND )//if directory doesn't exist
	{	printf("Settings directory not found, creating directory\n    %s\n",
					settingsPath.Path());
		//Create directory
		result = settingsDir.CreateDirectory(settingsPath.Path(), &settingsDir);
		if(result!=B_OK)
		{	//Creating directory failed
			printf("Error creating Einsteinium settings folder.  Cannot continue.\n");
			return;//Cannot continue, exit constructor
		}
	}

	// create path for settings file
	settingsPath.Append(ed_settings_file);
	BEntry settingsEntry(settingsPath.Path());
	if(!settingsEntry.Exists())//settings file doesn't exist, create default
	{
		WriteSettingsToFile(settingsPath);
	}
	settingsEntry.GetNodeRef(&settingsNodeRef);

	//Start the Looper running
//	Run();
	watchingLooper = new BLooper("settings");
	watchingLooper->AddHandler(this);
	watchingLooper->Run();

	//watch the settings file for changes
	StartWatching();

	ReadSettingsFromFile(settingsPath);

}

EDSettingsFile::~EDSettingsFile(){
	StopWatching();
	watchingLooper->Lock();
	watchingLooper->Quit();
//	Lock();
//	Quit();
}

void EDSettingsFile::StartWatching(){
	status_t result = watch_node(&settingsNodeRef, B_WATCH_STAT, this, watchingLooper);
	watchingSettingsNode = (result==B_OK);
//	printf("Watching daemon settings node was %ssuccessful.\n", watchingSettingsNode? "": "not ");
}

void EDSettingsFile::StopWatching(){
	if(watchingSettingsNode)//settings file is being watched
	{	watch_node(&settingsNodeRef, B_STOP_WATCHING, this, watchingLooper);
//		printf("Stopped watching daemon settings node\n");
		watchingSettingsNode = false;
	}
}

status_t EDSettingsFile::ReadSettingsFromFile(BPath file)
{
	_initStatus = B_ERROR;

	//Delete existing list objects
	AppRelaunchSettings *dummyPtr;
	deleteList(settingsList, dummyPtr);
	settingsList.MakeEmpty();

	//Parse XML
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(file.Path());
	if (doc == NULL ) {
		fprintf(stderr,"Einsteinium Daemon settings not parsed successfully. \n");
		return _initStatus;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return _initStatus;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) ED_XMLTEXT_ROOT_NAME)) {
		BString errorMessage("document of the wrong type, root node != ");
		errorMessage.Append(ED_XMLTEXT_ROOT_NAME);
		fprintf(stderr, errorMessage.String());
		xmlFreeDoc(doc);
		return _initStatus;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *) ED_XMLTEXT_CHILD1_NAME)){
			//parse application settings
			parseSettings (doc, cur);
		}
		cur = cur->next;
	}
	xmlFreeDoc(doc);

	_initStatus = B_OK;
	return _initStatus;
}


void EDSettingsFile::parseSettings (xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *sigValue, *relaunchValue;
	AppRelaunchSettings *appSettings;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *) ED_XMLTEXT_CHILD2_NAME)) {
			sigValue = xmlGetProp(cur, (const xmlChar *) ED_XMLTEXT_PROPERTY_SIGNATURE);
			relaunchValue = xmlGetProp(cur, (const xmlChar *) ED_XMLTEXT_PROPERTY_RELAUNCH);
//			printf("signature: %s, relaunch: %s\n", sigValue, relaunchValue);

			// Default action for apps not specified in settings file
			if( xmlStrcmp(sigValue, (const xmlChar *) ED_XMLTEXT_VALUE_DEFAULT) == 0 )
			{
				if(!strcmp((char *)relaunchValue, ED_XMLTEXT_VALUE_AUTO))
				{	defaultRelaunchAction = ACTION_AUTO; }
				else if(!strcmp((char *)relaunchValue, ED_XMLTEXT_VALUE_PROMPT))
				{	defaultRelaunchAction = ACTION_PROMPT; }
				else if(!strcmp((char *)relaunchValue, ED_XMLTEXT_VALUE_IGNORE))
				{	defaultRelaunchAction = ACTION_IGNORE; }
				else// default to prompt
				{	defaultRelaunchAction = ACTION_PROMPT; }
			}
			else
			{
				appSettings = new AppRelaunchSettings((char *)sigValue, (char *)relaunchValue);
				settingsList.AddItem(appSettings);
			}
			xmlFree(sigValue);
			xmlFree(relaunchValue);
		}
		cur = cur->next;
	}
    return;
}


status_t EDSettingsFile::WriteSettingsToFile(BPath file)
{
	xmlDocPtr doc = NULL;       /* document pointer */
	xmlNodePtr root_node = NULL, child1node = NULL, child2node = NULL;/* node pointers */
	xmlDtdPtr dtd = NULL;       /* DTD pointer */

	/*
	* Creates a new document, a node and set it as a root node
	*/
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST ED_XMLTEXT_ROOT_NAME);
	xmlDocSetRootElement(doc, root_node);
	dtd = xmlCreateIntSubset(doc, BAD_CAST ED_XMLTEXT_ROOT_NAME, NULL, BAD_CAST "tree.dtd");

	/*
     * xmlNewProp() creates attributes, which is "attached" to an node.
     * It returns xmlAttrPtr, which isn't used here.
     */
	child1node = xmlNewChild(root_node, NULL, BAD_CAST ED_XMLTEXT_CHILD1_NAME, NULL);

	// Write default relaunch setting
	BString relaunchTextValue;
	child2node = xmlNewChild(child1node, NULL, BAD_CAST ED_XMLTEXT_CHILD2_NAME, NULL);
	xmlNewProp(child2node, BAD_CAST ED_XMLTEXT_PROPERTY_SIGNATURE,
				BAD_CAST ED_XMLTEXT_VALUE_DEFAULT );
	switch(defaultRelaunchAction)
	{
		case ACTION_AUTO: {
			relaunchTextValue.SetTo(ED_XMLTEXT_VALUE_AUTO);
			break;
		}
		case ACTION_PROMPT: {
			relaunchTextValue.SetTo(ED_XMLTEXT_VALUE_PROMPT);
			break;
		}
		case ACTION_IGNORE: {
			relaunchTextValue.SetTo(ED_XMLTEXT_VALUE_IGNORE);
			break;
		}
		default: {
			// default to prompt
			relaunchTextValue.SetTo(ED_XMLTEXT_VALUE_PROMPT);
			break;
		}
	}
	xmlNewProp(child2node, BAD_CAST ED_XMLTEXT_PROPERTY_RELAUNCH,
				BAD_CAST relaunchTextValue.String() );

	// Write each app settings
	AppRelaunchSettings *appSettings = NULL;
	int index = 0, count = settingsList.CountItems();//search each item in list
	for(index=0; index<count; index++)//search each item in list
	{	appSettings = static_cast<AppRelaunchSettings *>(settingsList.ItemAt(index));
		child2node = xmlNewChild(child1node, NULL, BAD_CAST ED_XMLTEXT_CHILD2_NAME, NULL);
		xmlNewProp(child2node, BAD_CAST ED_XMLTEXT_PROPERTY_SIGNATURE,
				BAD_CAST appSettings->appSig.String() );
		switch(appSettings->relaunchAction)
		{
			case ACTION_AUTO: {
				relaunchTextValue.SetTo(ED_XMLTEXT_VALUE_AUTO);
				break;
			}
			case ACTION_PROMPT: {
				relaunchTextValue.SetTo(ED_XMLTEXT_VALUE_PROMPT);
				break;
			}
			case ACTION_IGNORE: {
				relaunchTextValue.SetTo(ED_XMLTEXT_VALUE_IGNORE);
				break;
			}
			default: {
				relaunchTextValue.SetTo(ED_XMLTEXT_VALUE_PROMPT);
				break;
			}
		}
		xmlNewProp(child2node, BAD_CAST ED_XMLTEXT_PROPERTY_RELAUNCH,
				BAD_CAST relaunchTextValue.String() );

	}
	xmlSaveFormatFile (file.Path(), doc, 1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
}


void EDSettingsFile::SaveSettings(BList* newSettingsList, int defaultAction)
{
	//Stop watching file
	StopWatching();

	// Update the private settings list
	defaultRelaunchAction = defaultAction;
	AppRelaunchSettings *newSettings = NULL;//, *currentSettings = NULL;
	deleteList(settingsList, newSettings);
	settingsList.MakeEmpty();
	int index, count = newSettingsList->CountItems();//search each item in list
	for(index=0; index<count; index++)//search each item in list
	{	newSettings = static_cast<AppRelaunchSettings *>(newSettingsList->ItemAt(index));
		AppRelaunchSettings *clonedSettings = new AppRelaunchSettings(newSettings);
		settingsList.AddItem(clonedSettings);
	}

	// Write to file
	WriteSettingsToFile(settingsPath);

	//Start watching file
	StartWatching();
}

void EDSettingsFile::UpdateActionForApp(const char *_signature, const char *_relaunch)
{
	AppRelaunchSettings *appSettings = FindSettingsForApp(_signature);
	if(appSettings!=NULL){
		appSettings->SetRelaunchAction(_relaunch);
	}
	else {
		appSettings = new AppRelaunchSettings(_signature, _relaunch);
		settingsList.AddItem(appSettings);
	}

	//Stop watching file
	StopWatching();

	// Write to file
	WriteSettingsToFile(settingsPath);

	//Start watching file
	StartWatching();
}

AppRelaunchSettings* EDSettingsFile::FindSettingsForApp(const char *sig)
{
	AppRelaunchSettings *appSettings = NULL;
	int index = 0, count = settingsList.CountItems();//search each item in list
	for(index=0; index<count; index++)//search each item in list
	{	appSettings = static_cast<AppRelaunchSettings *>(settingsList.ItemAt(index));
		if(strcmp(sig,appSettings->appSig.String()) == 0)//Found a match for the app sig
		{
			return appSettings;
		}
	}
	return NULL;
}


void EDSettingsFile::MessageReceived(BMessage *msg)
{	switch(msg->what)
	{
		case B_NODE_MONITOR: {//a watched node_ref has changed
			int32 opcode;
			if (msg->FindInt32("opcode", &opcode) == B_OK)
			{	switch (opcode)
				{	case B_STAT_CHANGED: {//file contents modified
						node_ref nref;
						msg->FindInt32("device", &nref.device);//create node from msg
						msg->FindInt64("node", &nref.node);
						if(nref == settingsNodeRef)//settings node changed
						{	printf("Daemon settings file changed, loading new settings...\n");
							ReadSettingsFromFile(settingsPath);
						}
						break; }
				}
			}
			break;
		}
	}
}


