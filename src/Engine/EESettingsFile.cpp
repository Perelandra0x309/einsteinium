/* EESettingsFile.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "EESettingsFile.h"


EESettingsFile::EESettingsFile()
	:
	BHandler("name"),
	fStatus(B_ERROR),
	fLaunchScale(DEFAULTVALUE_LAUNCHES),
	fFirstScale(DEFAULTVALUE_LAUNCHES),
	fLastScale(DEFAULTVALUE_LAUNCHES),
	fIntervalScale(DEFAULTVALUE_LAUNCHES),
	fRuntimeScale(DEFAULTVALUE_LAUNCHES),
	fInclusionDefault(EE_XMLTEXT_VALUE_PROMPT),
	fShowDeskbarMenu(false),
	fDeskbarMenuCount(0)
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
			printf("Error creating Einsteinium settings folder.  Cannot continue.\n");
			return;
				//Cannot continue, exit constructor
		}
	}

	// create path for settings file
	fSettingsPath.Append(ee_settings_file);
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


EESettingsFile::~EESettingsFile()
{
	_StopWatching();
	fWatchingLooper->Lock();
	fWatchingLooper->Quit();
}


void
EESettingsFile::_StartWatching(){
	status_t error = watch_node(&fSettingsNodeRef, B_WATCH_STAT, this, fWatchingLooper);
	fWatchingSettingsNode = (error==B_OK);
	printf("Watching engine settings node was %ssuccessful.\n", fWatchingSettingsNode? "": "not ");
	if(!fWatchingSettingsNode)
		printf("Error=%i (B_BAD_VALUE=%i, B_NO_MEMORY=%i, B_ERROR=%i)\n",
			error, B_BAD_VALUE, B_NO_MEMORY, B_ERROR);
}


void
EESettingsFile::_StopWatching(){
	if(fWatchingSettingsNode)
	{	watch_node(&fSettingsNodeRef, B_STOP_WATCHING, this, fWatchingLooper);
		printf("Stopped watching engine settings node\n");
		fWatchingSettingsNode = false;
	}
}


void
EESettingsFile::_ReadSettingsFromFile(BPath file)
{
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

	if (xmlStrcmp(cur->name, (const xmlChar *) EE_XMLTEXT_ROOT_NAME)) {
		BString errorMessage("document of the wrong type, root node != ");
		errorMessage.Append(EE_XMLTEXT_ROOT_NAME);
		fprintf(stderr, errorMessage.String());
		xmlFreeDoc(doc);
		return;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		// default list inclusion setting
		if (!xmlStrcmp(cur->name, (const xmlChar *) EE_XMLTEXT_CHILD_NAME_INCLUSION))
		{
			xmlChar *value = xmlGetProp(cur,
							(const xmlChar *) EE_XMLTEXT_PROPERTY_INCLUSION_DEFAULT);
			fInclusionDefault.SetTo((char *)value);
		}
		// scale settings
		if (!xmlStrcmp(cur->name, (const xmlChar *) EE_XMLTEXT_CHILD_NAME_RANK))
		{
			fLaunchScale = _XmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_LAUNCHES);
			fFirstScale = _XmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_FIRSTLAUNCH);
			fLastScale = _XmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_LASTLAUNCH);
			fIntervalScale = _XmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_INTERVAL);
			fRuntimeScale = _XmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_RUNTIME);
		}
		// deskbar settings
		if (!xmlStrcmp(cur->name, (const xmlChar *) EE_XMLTEXT_CHILD_NAME_DESKBAR))
		{
			xmlChar *value = xmlGetProp(cur, (const xmlChar *) EE_XMLTEXT_PROPERTY_SHOW);
			fShowDeskbarMenu = strcmp("true", (char *)value)==0;
			fDeskbarMenuCount = _XmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_COUNT);
		}

		cur = cur->next;
	}

}


int
EESettingsFile::_XmlGetIntProp(xmlNodePtr cur, char *name)
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
EESettingsFile::_WriteSettingsToFile()
{
	char launchChar[32], firstChar[32], lastChar[32], intervalChar[32], runtimeChar[32];
	sprintf(launchChar, "%ld", fLaunchScale);
	sprintf(firstChar, "%ld", fFirstScale);
	sprintf(lastChar, "%ld", fLastScale);
	sprintf(intervalChar, "%ld", fIntervalScale);
	sprintf(runtimeChar, "%ld", fRuntimeScale);
	BString showDeskbar;
	if(fShowDeskbarMenu)
		showDeskbar.SetTo("true");
	else
		showDeskbar.SetTo("false");
	char countChar[32];
	sprintf(countChar, "%i", fDeskbarMenuCount);

	BString xml_text("<?xml version=\"1.0\"?>\n");
	xml_text.Append("<").Append(EE_XMLTEXT_ROOT_NAME).Append(">\n");
	// write default link inclusion value
	xml_text.Append("	<").Append(EE_XMLTEXT_CHILD_NAME_INCLUSION)
			.Append(" ").Append(EE_XMLTEXT_PROPERTY_INCLUSION_DEFAULT)
			.Append("=\"").Append(fInclusionDefault).Append("\"/>\n");

	// write scale values
	xml_text.Append("	<").Append(EE_XMLTEXT_CHILD_NAME_RANK)
			.Append(" ").Append(EE_XMLTEXT_PROPERTY_LAUNCHES)
				.Append("=\"").Append(launchChar).Append("\"")
			.Append(" ").Append(EE_XMLTEXT_PROPERTY_FIRSTLAUNCH)
				.Append("=\"").Append(firstChar).Append("\"")
			.Append(" ").Append(EE_XMLTEXT_PROPERTY_LASTLAUNCH)
				.Append("=\"").Append(lastChar).Append("\"")
			.Append(" ").Append(EE_XMLTEXT_PROPERTY_INTERVAL)
				.Append("=\"").Append(intervalChar).Append("\"")
			.Append(" ").Append(EE_XMLTEXT_PROPERTY_RUNTIME)
				.Append("=\"").Append(runtimeChar).Append("\"")
			.Append("/>\n");

	// write deskbar settings
	xml_text.Append("	<").Append(EE_XMLTEXT_CHILD_NAME_DESKBAR)
			.Append(" ").Append(EE_XMLTEXT_PROPERTY_SHOW)
				.Append("=\"").Append(showDeskbar.String()).Append("\"")
			.Append(" ").Append(EE_XMLTEXT_PROPERTY_COUNT)
				.Append("=\"").Append(countChar).Append("\"")
			.Append("/>\n");

	xml_text.Append("</").Append(EE_XMLTEXT_ROOT_NAME).Append(">\n");

	_StopWatching();

	BFile eds_file;
	status_t result = eds_file.SetTo(fSettingsPath.Path(),
				B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if(result != B_OK)
	{
		_StartWatching();
		return B_ERROR;
	}
	eds_file.Write(xml_text.String(), xml_text.Length());
	eds_file.Unset();

	_StartWatching();
	return B_OK;
}


void
EESettingsFile::MessageReceived(BMessage *msg)
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
						}
						break; }
				}
			}
			break;
		}
	}
}


status_t
EESettingsFile::CheckStatus()
{
	return fStatus;
}


int*
EESettingsFile::GetScales()
{
	scales[LAUNCH_INDEX]=fLaunchScale;
	scales[FIRST_INDEX]=fFirstScale;
	scales[LAST_INDEX]=fLastScale;
	scales[INTERVAL_INDEX]=fIntervalScale;
	scales[RUNTIME_INDEX]=fRuntimeScale;
	return scales;
}


void
EESettingsFile::SaveScales(int *scales)
{
	fLaunchScale=scales[LAUNCH_INDEX];
	fFirstScale=scales[FIRST_INDEX];
	fLastScale=scales[LAST_INDEX];
	fIntervalScale=scales[INTERVAL_INDEX];
	fRuntimeScale=scales[RUNTIME_INDEX];
	_WriteSettingsToFile();
}


const char*
EESettingsFile::GetLinkInclusionDefaultValue()
{
	return fInclusionDefault.String();
}


void
EESettingsFile::SaveLinkInclusionDefaultValue(const char *value)
{
	// Only save setting when one of these three values
	if( strcmp(value,EE_XMLTEXT_VALUE_INCLUDE)==0
		|| strcmp(value,EE_XMLTEXT_VALUE_PROMPT)==0
		|| strcmp(value,EE_XMLTEXT_VALUE_EXCLUDE)==0 )
	{
		fInclusionDefault.SetTo(value);
		_WriteSettingsToFile();
	}
}


void
EESettingsFile::GetDeskbarSettings(bool &show, int &count)
{
	show = fShowDeskbarMenu;
	count = fDeskbarMenuCount;
}


void
EESettingsFile::SaveDeskbarSettings(bool show, int count)
{
	fShowDeskbarMenu = show;
	fDeskbarMenuCount = count;
	_WriteSettingsToFile();
}
