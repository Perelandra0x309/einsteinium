/* EDSettingsFile.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "EDSettingsFile.h"


EDSettingsFile::EDSettingsFile(BHandler *messageHandler=NULL)
	:
	BHandler("name"),
	fExternalMessageHandler(messageHandler),
	fDefaultRelaunchAction(ACTION_PROMPT),
	fInitStatus(B_ERROR)
{
	// find path for settings directory
	find_directory(B_USER_SETTINGS_DIRECTORY, &fSettingsPath);
	fSettingsPath.Append(e_settings_dir);
	BDirectory settingsDir;
	status_t result = settingsDir.SetTo(fSettingsPath.Path());
	if( result == B_ENTRY_NOT_FOUND )
	{	printf("Settings directory not found, creating directory\n    %s\n",
					fSettingsPath.Path());
		result = settingsDir.CreateDirectory(fSettingsPath.Path(), &settingsDir);
		if(result!=B_OK)
		{
			printf("Error creating Einsteinium settings folder.  Cannot continue.\n");
			return;
				//Cannot continue, exit constructor
		}
	}

	// create path for settings file
	fSettingsPath.Append(ed_settings_file);
	BEntry settingsEntry(fSettingsPath.Path());
	// Settings file doesn't exist, create default settings file
	if(!settingsEntry.Exists())
	{
		fSettingsList.AddItem(new AppRelaunchSettings("application/x-vnd.Be-input_server",
					ACTION_AUTO));
		fSettingsList.AddItem(new AppRelaunchSettings("application/x-vnd.Einsteinium_Launcher",
					ACTION_IGNORE));
		_WriteSettingsToFile(fSettingsPath);
	}
	settingsEntry.GetNodeRef(&fSettingsNodeRef);

	//Start the Looper running
	fWatchingLooper = new BLooper("settings");
	fWatchingLooper->AddHandler(this);
	fWatchingLooper->Run();

	//watch the settings file for changes
	_StartWatching();

	_ReadSettingsFromFile(fSettingsPath);

}


EDSettingsFile::~EDSettingsFile()
{
	_StopWatching();
	fWatchingLooper->Lock();
	fWatchingLooper->Quit();
}


void
EDSettingsFile::_StartWatching()
{
	status_t result = watch_node(&fSettingsNodeRef, B_WATCH_STAT, this, fWatchingLooper);
	fWatchingSettingsNode = (result==B_OK);
//	printf("Watching daemon settings node was %ssuccessful.\n", fWatchingSettingsNode? "": "not ");
}


void
EDSettingsFile::_StopWatching()
{
	if(fWatchingSettingsNode)//settings file is being watched
	{	watch_node(&fSettingsNodeRef, B_STOP_WATCHING, this, fWatchingLooper);
//		printf("Stopped watching daemon settings node\n");
		fWatchingSettingsNode = false;
	}
}


status_t
EDSettingsFile::_ReadSettingsFromFile(BPath file)
{
	fInitStatus = B_ERROR;

	//Delete existing list objects
	AppRelaunchSettings *dummyPtr;
	DeleteList(fSettingsList, dummyPtr);
	fSettingsList.MakeEmpty();

	//Parse XML
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(file.Path());
	if (doc == NULL ) {
		fprintf(stderr,"Einsteinium Daemon settings not parsed successfully. \n");
		return fInitStatus;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return fInitStatus;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) ED_XMLTEXT_ROOT_NAME)) {
		BString errorMessage("document of the wrong type, root node != ");
		errorMessage.Append(ED_XMLTEXT_ROOT_NAME);
		fprintf(stderr, errorMessage.String());
		xmlFreeDoc(doc);
		return fInitStatus;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *) ED_XMLTEXT_CHILD1_NAME)){
			_ParseSettings (doc, cur);
		}
		cur = cur->next;
	}
	xmlFreeDoc(doc);

	fInitStatus = B_OK;
	return fInitStatus;
}


void
EDSettingsFile::_ParseSettings (xmlDocPtr doc, xmlNodePtr cur)
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
				fDefaultRelaunchAction = _TranslateRelaunchXML(relaunchValue);
			}
			else
			{
				appSettings = new AppRelaunchSettings((char *)sigValue,
					_TranslateRelaunchXML(relaunchValue));
				fSettingsList.AddItem(appSettings);
			}
			xmlFree(sigValue);
			xmlFree(relaunchValue);
		}
		cur = cur->next;
	}
    return;
}


int
EDSettingsFile::_TranslateRelaunchXML(xmlChar *value)
{
	if(!strcmp((char *)value, ED_XMLTEXT_VALUE_AUTO))
		return ACTION_AUTO;
	else if(!strcmp((char *)value, ED_XMLTEXT_VALUE_PROMPT))
		return ACTION_PROMPT;
	else if(!strcmp((char *)value, ED_XMLTEXT_VALUE_IGNORE))
		return ACTION_IGNORE;
	else
		return ACTION_DEFAULT;
}


status_t
EDSettingsFile::_WriteSettingsToFile(BPath file)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL, child1node = NULL, child2node = NULL;
	xmlDtdPtr dtd = NULL;

	//Creates a new document, a node and set it as a root node
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST ED_XMLTEXT_ROOT_NAME);
	xmlDocSetRootElement(doc, root_node);
	dtd = xmlCreateIntSubset(doc, BAD_CAST ED_XMLTEXT_ROOT_NAME, NULL, BAD_CAST "tree.dtd");

	child1node = xmlNewChild(root_node, NULL, BAD_CAST ED_XMLTEXT_CHILD1_NAME, NULL);

	// Write default relaunch setting
	BString relaunchTextValue;
	child2node = xmlNewChild(child1node, NULL, BAD_CAST ED_XMLTEXT_CHILD2_NAME, NULL);
	xmlNewProp(child2node, BAD_CAST ED_XMLTEXT_PROPERTY_SIGNATURE,
				BAD_CAST ED_XMLTEXT_VALUE_DEFAULT );
	switch(fDefaultRelaunchAction)
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
			switch(ACTION_DEFAULT)
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
					relaunchTextValue.SetTo("error");
				}
			}
		}
	}
	xmlNewProp(child2node, BAD_CAST ED_XMLTEXT_PROPERTY_RELAUNCH,
				BAD_CAST relaunchTextValue.String() );

	// Write each app settings
	AppRelaunchSettings *appSettings = NULL;
	int index = 0, count = fSettingsList.CountItems();
	for(index=0; index<count; index++)
	{	appSettings = static_cast<AppRelaunchSettings *>(fSettingsList.ItemAt(index));
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
				switch(ACTION_DEFAULT)
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
						relaunchTextValue.SetTo("error");
					}
				}
			}
		}
		xmlNewProp(child2node, BAD_CAST ED_XMLTEXT_PROPERTY_RELAUNCH,
				BAD_CAST relaunchTextValue.String() );

	}
	xmlSaveFormatFile (file.Path(), doc, 1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
}


void
EDSettingsFile::UpdateDefaultAction(int _relaunch)
{
	fDefaultRelaunchAction = _relaunch;

	_StopWatching();
	_WriteSettingsToFile(fSettingsPath);
	_StartWatching();
}


void
EDSettingsFile::UpdateActionForApp(const char *_signature, int _relaunch)
{
	AppRelaunchSettings *appSettings = FindSettingsForApp(_signature);
	if(appSettings!=NULL){
		appSettings->relaunchAction = _relaunch;
	}
	else {
		appSettings = new AppRelaunchSettings(_signature, _relaunch);
		fSettingsList.AddItem(appSettings);
	}

	_StopWatching();
	_WriteSettingsToFile(fSettingsPath);
	_StartWatching();
}


void
EDSettingsFile::RemoveApp(const char *_signature)
{
	AppRelaunchSettings *appSettings = FindSettingsForApp(_signature);
	if(appSettings==NULL) return;
	fSettingsList.RemoveItem(appSettings);
	delete appSettings;

	_StopWatching();
	_WriteSettingsToFile(fSettingsPath);
	_StartWatching();
}


AppRelaunchSettings*
EDSettingsFile::FindSettingsForApp(const char *sig)
{
	AppRelaunchSettings *appSettings = NULL;
	int index = 0, count = fSettingsList.CountItems();
	for(index=0; index<count; index++)
	{	appSettings = static_cast<AppRelaunchSettings *>(fSettingsList.ItemAt(index));
		if(strcmp(sig,appSettings->appSig.String()) == 0)
			return appSettings;
	}
	return NULL;
}


void
EDSettingsFile::MessageReceived(BMessage *msg)
{	switch(msg->what)
	{
		//the settings file node_ref has changed
		case B_NODE_MONITOR: {
			int32 opcode;
			if (msg->FindInt32("opcode", &opcode) == B_OK)
			{	switch (opcode)
				{	case B_STAT_CHANGED: {
						node_ref nref;
						msg->FindInt32("device", &nref.device);
						msg->FindInt64("node", &nref.node);
						if(nref == fSettingsNodeRef)
						{	printf("Daemon settings file changed, loading new settings...\n");
							_ReadSettingsFromFile(fSettingsPath);
							// Notify the external message handler if available
							if(fExternalMessageHandler!=NULL)
							{
								status_t mErr;
								BMessenger messenger(fExternalMessageHandler, NULL, &mErr);
								if(messenger.IsValid())
									messenger.SendMessage(ED_SETTINGS_FILE_CHANGED_EXTERNALLY);
							}
						}
						break;
					}
				}
			}
			break;
		}
	}
}


