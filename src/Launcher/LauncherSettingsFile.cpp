/* LauncherSettingsFile.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherSettingsFile.h"


LauncherSettingsFile::LauncherSettingsFile(BHandler *messageHandler=NULL)
	:
	BHandler("name"),
	fStatus(B_ERROR),
	fExternalMessageHandler(messageHandler),
	fLaunchScale(DEFAULTVALUE_LAUNCHES),
	fFirstScale(DEFAULTVALUE_LAUNCHES),
	fLastScale(DEFAULTVALUE_LAUNCHES),
	fIntervalScale(DEFAULTVALUE_LAUNCHES),
	fRuntimeScale(DEFAULTVALUE_LAUNCHES),
	fInclusionDefault(EL_XMLTEXT_VALUE_PROMPT),
//	fShowDeskbarMenu(true),
	fDeskbarMenuCount(20),
	fExclusionsList(EL_MESSAGE_WHAT_EXCLUDED_APPS)
{
	// find path for settings directory
	find_directory(B_USER_SETTINGS_DIRECTORY, &fSettingsPath);
	fSettingsPath.Append(e_settings_dir);
	BDirectory settingsDir;
	status_t result = settingsDir.SetTo(fSettingsPath.Path());
	if( result == B_ENTRY_NOT_FOUND )
	{	printf("Settings directory not found, creating directory\n    %s\n",
					fSettingsPath.Path());
		//Create directory
		result = settingsDir.CreateDirectory(fSettingsPath.Path(), &settingsDir);
		if(result!=B_OK)
		{
			printf("Error creating Launcher settings folder.  Cannot continue.\n");
			return;
				//Cannot continue, exit constructor
		}
	}

	// create path for settings file
	fSettingsPath.Append(el_settings_file);
	BEntry settingsEntry(fSettingsPath.Path());
	if(!settingsEntry.Exists())
	{
		//settings file doesn't exist, create default file
		_WriteSettingsToFile();
	}
	else
	{
		_ReadSettingsFromFile(fSettingsPath);
	}
	settingsEntry.GetNodeRef(&fSettingsNodeRef);

	//Start the Looper running
	fWatchingLooper = new BLooper("settings");
	fWatchingLooper->AddHandler(this);
	fWatchingLooper->Run();

	_StartWatching();
	fStatus = B_OK;
}


LauncherSettingsFile::~LauncherSettingsFile()
{
	_StopWatching();
	fWatchingLooper->Lock();
	fWatchingLooper->Quit();
}


void
LauncherSettingsFile::_StartWatching(){
	status_t error = watch_node(&fSettingsNodeRef, B_WATCH_STAT, this, fWatchingLooper);
	fWatchingSettingsNode = (error==B_OK);
	printf("Watching engine settings node was %ssuccessful.\n", fWatchingSettingsNode? "": "not ");
	if(!fWatchingSettingsNode)
		printf("Error=%i (B_BAD_VALUE=%i, B_NO_MEMORY=%i, B_ERROR=%i)\n",
			error, B_BAD_VALUE, B_NO_MEMORY, B_ERROR);
}


void
LauncherSettingsFile::_StopWatching(){
	if(fWatchingSettingsNode)
	{	watch_node(&fSettingsNodeRef, B_STOP_WATCHING, this, fWatchingLooper);
		printf("Stopped watching engine settings node\n");
		fWatchingSettingsNode = false;
	}
}


void
LauncherSettingsFile::_ReadSettingsFromFile(BPath file)
{
	// Parse XML
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(file.Path());
	if (doc == NULL ) {
		fprintf(stderr,"Einsteinium Engine settings not parsed successfully. \n");
		return;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_ROOT_NAME)) {
		BString errorMessage("document of the wrong type, root node != ");
		errorMessage.Append(EL_XMLTEXT_ROOT_NAME);
		fprintf(stderr, errorMessage.String());
		xmlFreeDoc(doc);
		return;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		// default list inclusion setting
		if (!xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_CHILD_NAME_INCLUSION))
		{
			xmlChar *value = xmlGetProp(cur,
							(const xmlChar *) EL_XMLTEXT_PROPERTY_INCLUSION_DEFAULT);
			// Only save setting when one of these three values
			if( strcmp((char *)value,EL_XMLTEXT_VALUE_INCLUDE)==0
				|| strcmp((char *)value,EL_XMLTEXT_VALUE_PROMPT)==0
				|| strcmp((char *)value,EL_XMLTEXT_VALUE_EXCLUDE)==0 )
			{
				fInclusionDefault.SetTo((char *)value);
			}
		}
		// scale settings
		if (!xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_CHILD_NAME_RANK))
		{
			fLaunchScale = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_LAUNCHES);
			fFirstScale = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_FIRSTLAUNCH);
			fLastScale = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_LASTLAUNCH);
			fIntervalScale = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_INTERVAL);
			fRuntimeScale = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_RUNTIME);
		}
		// deskbar settings
		if (!xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_CHILD_NAME_DESKBAR))
		{
//			xmlChar *value = xmlGetProp(cur, (const xmlChar *) EL_XMLTEXT_PROPERTY_SHOW);
//			fShowDeskbarMenu = strcmp("true", (char *)value)==0;
			fDeskbarMenuCount = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_COUNT);
		}
		// exclusion settings
		if (!xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_CHILD_NAME_EXCLUSIONS))
		{
			//Delete existing list objects
			fExclusionsList.MakeEmpty();
			_ParseExclusionSettings(doc, cur);
		}

		cur = cur->next;
	}

}


void
LauncherSettingsFile::_ParseExclusionSettings(xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *sigValue, *nameValue;
	BString stringValue;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_CHILD_NAME_APP)) {
			sigValue = xmlGetProp(cur, (const xmlChar *) EL_XMLTEXT_PROPERTY_SIGNATURE);
			stringValue.SetTo((char *)sigValue);
			fExclusionsList.AddString(EL_EXCLUDE_SIGNATURE, stringValue.String());
			nameValue = xmlGetProp(cur, (const xmlChar *) EL_XMLTEXT_PROPERTY_NAME);
			stringValue.SetTo((char *)nameValue);
			fExclusionsList.AddString(EL_EXCLUDE_NAME, stringValue.String());
			xmlFree(sigValue);
		}
		cur = cur->next;
	}
}


int
LauncherSettingsFile::_XmlGetIntProp(xmlNodePtr cur, char *name)
{
	int intValue;
	xmlChar *xmlValue = xmlGetProp(cur, (const xmlChar *) name);
	if(xmlValue==NULL){
		intValue = 0;
//		printf("%s property value: NULL, integer: %i\n", name, intValue);
	}
	else{
		intValue = atoi((char *)xmlValue);
//		printf("%s property value: %s, integer: %i\n", name, xmlValue, intValue);
	}
	return intValue;
}


status_t
LauncherSettingsFile::_WriteSettingsToFile()
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL, child1node = NULL, child2node = NULL;
	xmlDtdPtr dtd = NULL;

	BString launchChar, firstChar, lastChar, intervalChar, runtimeChar;
	launchChar << fLaunchScale;
	firstChar << fFirstScale;
	lastChar << fLastScale;
	intervalChar << fIntervalScale;
	runtimeChar << fRuntimeScale;

/*	BString showDeskbar;
	if(fShowDeskbarMenu)
		showDeskbar.SetTo("true");
	else
		showDeskbar.SetTo("false");*/
	BString countChar;
	countChar << fDeskbarMenuCount;

	//Creates a new document, a node and set it as a root node
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST EL_XMLTEXT_ROOT_NAME);
	xmlDocSetRootElement(doc, root_node);
	dtd = xmlCreateIntSubset(doc, BAD_CAST EL_XMLTEXT_ROOT_NAME, NULL, BAD_CAST "tree.dtd");

	// write default link inclusion value
	child1node = xmlNewChild(root_node, NULL, BAD_CAST EL_XMLTEXT_CHILD_NAME_INCLUSION, NULL);
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_INCLUSION_DEFAULT,
				BAD_CAST fInclusionDefault.String() );

	// write scale values
	child1node = xmlNewChild(root_node, NULL, BAD_CAST EL_XMLTEXT_CHILD_NAME_RANK, NULL);
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_LAUNCHES,
				BAD_CAST launchChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_FIRSTLAUNCH,
				BAD_CAST firstChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_LASTLAUNCH,
				BAD_CAST lastChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_INTERVAL,
				BAD_CAST intervalChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_RUNTIME,
				BAD_CAST runtimeChar.String() );

	// write deskbar settings
	child1node = xmlNewChild(root_node, NULL, BAD_CAST EL_XMLTEXT_CHILD_NAME_DESKBAR, NULL);
//	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_SHOW,
//				BAD_CAST showDeskbar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_COUNT,
				BAD_CAST countChar.String() );

	// Write excluded applications
	child1node = xmlNewChild(root_node, NULL, BAD_CAST EL_XMLTEXT_CHILD_NAME_EXCLUSIONS, NULL);
	type_code typeFound;
	int32 signatureCount = 0;
	status_t result1 = fExclusionsList.GetInfo(EL_EXCLUDE_SIGNATURE, &typeFound, &signatureCount);
	BString sig, name;
	for(int i=0; i<signatureCount; i++)
	{
		sig.SetTo("");
		name.SetTo("");
		fExclusionsList.FindString(EL_EXCLUDE_SIGNATURE, i, &sig);
		fExclusionsList.FindString(EL_EXCLUDE_NAME, i, &name);
		child2node = xmlNewChild(child1node, NULL, BAD_CAST EL_XMLTEXT_CHILD_NAME_APP, NULL);
		xmlNewProp(child2node, BAD_CAST EL_XMLTEXT_PROPERTY_SIGNATURE,
				BAD_CAST sig.String() );
		xmlNewProp(child2node, BAD_CAST EL_XMLTEXT_PROPERTY_NAME,
				BAD_CAST name.String() );
	}

	// Save file
	_StopWatching();
	xmlSaveFormatFile(fSettingsPath.Path(), doc, 1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	_StartWatching();

	return B_OK;
}

/*
void
LauncherSettingsFile::_DeleteExclusionsList()
{
	BString *item;
	do
	{	item = static_cast<BString *>(fExclusionsList.RemoveItem(int32(0)));
		delete item;
	}while(item);
	fExclusionsList.MakeEmpty();
}*/


void
LauncherSettingsFile::MessageReceived(BMessage *msg)
{	switch(msg->what)
	{
		case B_NODE_MONITOR: {//a watched node_ref has changed
			int32 opcode;
			if (msg->FindInt32("opcode", &opcode) == B_OK)
			{	switch (opcode)
				{	case B_STAT_CHANGED: {//file contents modified
						node_ref nref;
						msg->FindInt32("device", &nref.device);
						msg->FindInt64("node", &nref.node);
						if(nref == fSettingsNodeRef)
						{	printf("Engine settings file changed, loading new settings...\n");
							_ReadSettingsFromFile(fSettingsPath);
							// Notify the external message handler if available
							if(fExternalMessageHandler!=NULL)
							{
								status_t mErr;
								BMessenger messenger(fExternalMessageHandler, NULL, &mErr);
								if(messenger.IsValid())
									messenger.SendMessage(EL_SETTINGS_FILE_CHANGED_EXTERNALLY);
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

/*
status_t
LauncherSettingsFile::CheckStatus()
{
	return fStatus;
}*/


int*
LauncherSettingsFile::GetScales()
{
	scales[LAUNCH_INDEX]=fLaunchScale;
	scales[FIRST_INDEX]=fFirstScale;
	scales[LAST_INDEX]=fLastScale;
	scales[INTERVAL_INDEX]=fIntervalScale;
	scales[RUNTIME_INDEX]=fRuntimeScale;
	return scales;
}


void
LauncherSettingsFile::SaveScales(int *scales)
{
	fLaunchScale=scales[LAUNCH_INDEX];
	fFirstScale=scales[FIRST_INDEX];
	fLastScale=scales[LAST_INDEX];
	fIntervalScale=scales[INTERVAL_INDEX];
	fRuntimeScale=scales[RUNTIME_INDEX];
	_WriteSettingsToFile();
}


const char*
LauncherSettingsFile::GetLinkInclusionDefaultValue()
{
	return fInclusionDefault.String();
}


void
LauncherSettingsFile::SaveLinkInclusionDefaultValue(const char *value)
{
	// Only save setting when one of these three values
	if( strcmp(value,EL_XMLTEXT_VALUE_INCLUDE)==0
		|| strcmp(value,EL_XMLTEXT_VALUE_PROMPT)==0
		|| strcmp(value,EL_XMLTEXT_VALUE_EXCLUDE)==0 )
	{
		fInclusionDefault.SetTo(value);
		_WriteSettingsToFile();
	}
}


/*
int
LauncherSettingsFile::GetDeskbarSettings()
{
//	show = fShowDeskbarMenu;
	count = fDeskbarMenuCount;
}*/

/*
void
LauncherSettingsFile::SaveDeskbarSettings(bool show, int count)
{
	fShowDeskbarMenu = show;
	fDeskbarMenuCount = count;
	_WriteSettingsToFile();
}*/


void
LauncherSettingsFile::SaveDeskbarCount(int count)
{
	fDeskbarMenuCount = count;
	_WriteSettingsToFile();
}


void
LauncherSettingsFile::SaveExclusionsList(BMessage &exclusionsList)
{
	fExclusionsList.MakeEmpty();
	fExclusionsList = exclusionsList;
	_WriteSettingsToFile();
}

