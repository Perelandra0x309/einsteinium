/* LauncherSettingsFile.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherSettingsFile.h"


LauncherSettingsFile::LauncherSettingsFile(BHandler *messageHandler)
	:
	BHandler("name"),
	fStatus(B_ERROR),
	fExternalMessageHandler(messageHandler),
	fLaunchScale(DEFAULT_VALUE_LAUNCH_SCALE),
	fFirstScale(DEFAULT_VALUE_FIRST_SCALE),
	fLastScale(DEFAULT_VALUE_LAST_SCALE),
	fIntervalScale(DEFAULT_VALUE_INTERVAL_SCALE),
	fRuntimeScale(DEFAULT_VALUE_RUNTIME_SCALE),
	fShowDeskbarMenu(DEFAULT_VALUE_SHOW_DESKBAR_MENU),
	fAppCount(DEFAULT_VALUE_RECENT_COUNT),
	fExclusionsList(EL_MESSAGE_WHAT_EXCLUDED_APPS),
	fLaunchEngineOnStart(DEFAULT_LAUNCH_ENGINE_ON_START),
	fMinAppIconSize(16),
	fMaxAppIconSize(kIconDefaultSize),
	fDocIconSize(kIconDefaultSize),
	fRecentDocCount(DEFAULT_VALUE_RECENT_COUNT),
	fRecentFolderCount(DEFAULT_VALUE_RECENT_COUNT),
	fRecentQueryCount(DEFAULT_VALUE_RECENT_COUNT),
	fFontSize(0),
	fWindowLook(B_TITLED_WINDOW_LOOK),
	fWindowFrame(0,0,0,0)
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
	if(settingsEntry.Exists())
	{
		_ReadSettingsFromFile(fSettingsPath);
	}
	else
	{
		// look for legacy xml formatted settings file and convert if found
		BPath legacySettingsPath;
		fSettingsPath.GetParent(&legacySettingsPath);
		legacySettingsPath.Append(el_legacy_settings_file);
		BEntry legacySettingsEntry(legacySettingsPath.Path());
		if(legacySettingsEntry.Exists())
		{
			_ReadLegacyXMLSettingsFromFile(legacySettingsPath);
			_WriteSettingsToFile();
			if(settingsEntry.Exists())
				legacySettingsEntry.Remove();
		}
		//settings files don't exist, create default file
		else
		{
			fExclusionsList.AddString(EL_EXCLUDE_SIGNATURE, e_launcher_sig);
			fExclusionsList.AddString(EL_EXCLUDE_NAME, "Einsteinium_Launcher");
			fExclusionsList.AddString(EL_EXCLUDE_SIGNATURE, e_daemon_sig);
			fExclusionsList.AddString(EL_EXCLUDE_NAME, "einsteinium_daemon");
			fExclusionsList.AddString(EL_EXCLUDE_SIGNATURE, e_engine_sig);
			fExclusionsList.AddString(EL_EXCLUDE_NAME, "einsteinium_engine");
			fExclusionsList.AddString(EL_EXCLUDE_SIGNATURE, e_preferences_sig);
			fExclusionsList.AddString(EL_EXCLUDE_NAME, "Einsteinium_Preferences");
			fExclusionsList.AddString(EL_EXCLUDE_SIGNATURE, "application/x-vnd.Be-TSKB");
			fExclusionsList.AddString(EL_EXCLUDE_NAME, "Deskbar");
			fExclusionsList.AddString(EL_EXCLUDE_SIGNATURE, "application/x-vnd.Be-TRAK");
			fExclusionsList.AddString(EL_EXCLUDE_NAME, "Tracker");

			_WriteSettingsToFile();
		}
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
/*	printf("Watching Launcher settings node was %ssuccessful.\n", fWatchingSettingsNode? "": "not ");
	if(!fWatchingSettingsNode)
		printf("Error=%i (B_BAD_VALUE=%i, B_NO_MEMORY=%i, B_ERROR=%i)\n",
			error, B_BAD_VALUE, B_NO_MEMORY, B_ERROR);*/
}


void
LauncherSettingsFile::_StopWatching(){
	if(fWatchingSettingsNode)
	{	watch_node(&fSettingsNodeRef, B_STOP_WATCHING, this, fWatchingLooper);
//		printf("Stopped watching engine settings node\n");
		fWatchingSettingsNode = false;
	}
}


void
LauncherSettingsFile::_ReadSettingsFromFile(BPath settingsPath)
{
	BFile settingsFile;
	BMessage settings;
	status_t result = settingsFile.SetTo(settingsPath.Path(), B_READ_ONLY);
	if(result == B_OK)
	{
		settings.Unflatten(&settingsFile);
	}
	else
		return;

	// engine auto launch
	fLaunchEngineOnStart = settings.GetBool(EL_HMFL_PROPERTY_AUTOLAUNCH, DEFAULT_LAUNCH_ENGINE_ON_START);

	// scale settings
	fLaunchScale = settings.GetInt8(EL_HMFL_PROPERTY_LAUNCHES, DEFAULT_VALUE_LAUNCH_SCALE);
	fFirstScale = settings.GetInt8(EL_HMFL_PROPERTY_FIRSTLAUNCH, DEFAULT_VALUE_FIRST_SCALE);
	fLastScale = settings.GetInt8(EL_HMFL_PROPERTY_LASTLAUNCH, DEFAULT_VALUE_LAST_SCALE);
	fIntervalScale = settings.GetInt8(EL_HMFL_PROPERTY_INTERVAL, DEFAULT_VALUE_INTERVAL_SCALE);
	fRuntimeScale = settings.GetInt8(EL_HMFL_PROPERTY_RUNTIME, DEFAULT_VALUE_RUNTIME_SCALE);

	// deskbar settings
	fShowDeskbarMenu = settings.GetBool(EL_HMFL_PROPERTY_SHOWDESKBAR, DEFAULT_VALUE_SHOW_DESKBAR_MENU);

	// layout settings
	fAppCount = settings.GetInt8(EL_HMFL_PROPERTY_APPCOUNT, DEFAULT_VALUE_RECENT_COUNT);
	fMinAppIconSize = settings.GetInt8(EL_HMFL_PROPERTY_MINAPP, 16);
	fMaxAppIconSize = settings.GetInt8(EL_HMFL_PROPERTY_MAXAPP, kIconDefaultSize);
	fDocIconSize = settings.GetInt8(EL_HMFL_PROPERTY_DOCICON, kIconDefaultSize);
	fRecentDocCount = settings.GetInt8(EL_HMFL_PROPERTY_DOCCOUNT, DEFAULT_VALUE_RECENT_COUNT);
	fRecentFolderCount = settings.GetInt8(EL_HMFL_PROPERTY_FOLDERCOUNT, DEFAULT_VALUE_RECENT_COUNT);
	fRecentQueryCount = settings.GetInt8(EL_HMFL_PROPERTY_QUERYCOUNT, DEFAULT_VALUE_RECENT_COUNT);
	fFontSize = settings.GetInt8(EL_HMFL_PROPERTY_FONTSIZE, 0);

	// window settings
	int8 intValue;
	result = settings.FindInt8(EL_HMFL_PROPERTY_WINDOWLOOK, &intValue);
	if(result == B_OK)
	{
		switch(intValue)
		{
			case 0:
			{
				fWindowLook = B_TITLED_WINDOW_LOOK;
				break;
			}
			case 1:
			{
				fWindowLook = B_FLOATING_WINDOW_LOOK;
				break;
			}
			case 2:
			{
				fWindowLook = B_MODAL_WINDOW_LOOK;
				break;
			}
			case 3:
			{
				fWindowLook = B_BORDERED_WINDOW_LOOK;
				break;
			}
		}
	}
	int16 windowLeft, windowTop, windowRight, windowBottom;
	status_t result1 = settings.FindInt16(EL_HMFL_PROPERTY_WINDOWFRAMELEFT, &windowLeft);
	status_t result2 = settings.FindInt16(EL_HMFL_PROPERTY_WINDOWFRAMETOP, &windowTop);
	status_t result3 = settings.FindInt16(EL_HMFL_PROPERTY_WINDOWFRAMERIGHT, &windowRight);
	status_t result4 = settings.FindInt16(EL_HMFL_PROPERTY_WINDOWFRAMEBOTTOM, &windowBottom);
	if( (result1==B_OK) && (result2==B_OK) && (result3==B_OK) && (result4==B_OK) )
		fWindowFrame.Set(windowLeft, windowTop, windowRight, windowBottom);

	// exclusion settings
	fExclusionsList.MakeEmpty();
	BMessage exclusionsMessage;
	result = settings.FindMessage(EL_HMFL_PROPERTY_APP_EXCLUSIONS, &exclusionsMessage);
	if(result == B_OK)
		fExclusionsList = exclusionsMessage;
}


void
LauncherSettingsFile::_ReadLegacyXMLSettingsFromFile(BPath settingsPath)
{
	// Parse XML
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(settingsPath.Path());
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
/*		if (!xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_CHILD_NAME_INCLUSION))
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
		}*/
		// engine auto launch
		if (!xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_CHILD_NAME_ENGINE))
		{
			xmlChar *value = xmlGetProp(cur, (const xmlChar *) EL_XMLTEXT_PROPERTY_AUTOLAUNCH);
			fLaunchEngineOnStart = strcmp("true", (char *)value)==0;
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
			xmlChar *value = xmlGetProp(cur, (const xmlChar *) EL_XMLTEXT_PROPERTY_SHOW);
			if(value){
				fShowDeskbarMenu = strcmp("true", (char *)value)==0;
			}
			fAppCount = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_COUNT);
		}
		// layout settings
		if (!xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_CHILD_NAME_LAYOUT))
		{
			fAppCount = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_APPCOUNT, DEFAULT_VALUE_RECENT_COUNT);
			fMinAppIconSize = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_MINAPP, kIconDefaultSize);
			fMaxAppIconSize = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_MAXAPP, kIconDefaultSize);
			fDocIconSize = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_DOCICON, kIconDefaultSize);
			fRecentDocCount = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_DOCCOUNT, DEFAULT_VALUE_RECENT_COUNT);
			fRecentFolderCount = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_FOLDERCOUNT, DEFAULT_VALUE_RECENT_COUNT);
			fRecentQueryCount = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_QUERYCOUNT, DEFAULT_VALUE_RECENT_COUNT);
			fFontSize = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_FONTSIZE);
		}
		// window settings
		if (!xmlStrcmp(cur->name, (const xmlChar *) EL_XMLTEXT_CHILD_NAME_WINDOW))
		{
			int look = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_WINDOWLOOK);
			switch(look)
			{
				case 0:
				{
					fWindowLook = B_TITLED_WINDOW_LOOK;
					break;
				}
				case 1:
				{
					fWindowLook = B_FLOATING_WINDOW_LOOK;
					break;
				}
				case 2:
				{
					fWindowLook = B_MODAL_WINDOW_LOOK;
					break;
				}
				case 3:
				{
					fWindowLook = B_BORDERED_WINDOW_LOOK;
					break;
				}
			}
			int left = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_WINDOWFRAMELEFT);
			int top = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_WINDOWFRAMETOP);
			int right = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_WINDOWFRAMERIGHT);
			int bottom = _XmlGetIntProp(cur, EL_XMLTEXT_PROPERTY_WINDOWFRAMEBOTTOM);
			fWindowFrame.Set(left, top, right, bottom);
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
LauncherSettingsFile::_XmlGetIntProp(xmlNodePtr cur, const char *name, int defaultValue)
{
	int intValue;
	xmlChar *xmlValue = xmlGetProp(cur, (const xmlChar *) name);
	if(xmlValue==NULL){
		intValue = defaultValue;
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
	BMessage settings;

	// engine auto launch
	settings.AddBool(EL_HMFL_PROPERTY_AUTOLAUNCH, fLaunchEngineOnStart);

	// scale settings
	settings.AddInt8(EL_HMFL_PROPERTY_LAUNCHES, fLaunchScale);
	settings.AddInt8(EL_HMFL_PROPERTY_FIRSTLAUNCH, fFirstScale);
	settings.AddInt8(EL_HMFL_PROPERTY_LASTLAUNCH, fLastScale);
	settings.AddInt8(EL_HMFL_PROPERTY_INTERVAL, fIntervalScale);
	settings.AddInt8(EL_HMFL_PROPERTY_RUNTIME, fRuntimeScale);

	// deskbar settings
	settings.AddBool(EL_HMFL_PROPERTY_SHOWDESKBAR, fShowDeskbarMenu);

	// layout settings
	settings.AddInt8(EL_HMFL_PROPERTY_APPCOUNT, fAppCount);
	settings.AddInt8(EL_HMFL_PROPERTY_MINAPP, fMinAppIconSize);
	settings.AddInt8(EL_HMFL_PROPERTY_MAXAPP, fMaxAppIconSize);
	settings.AddInt8(EL_HMFL_PROPERTY_DOCICON, fDocIconSize);
	settings.AddInt8(EL_HMFL_PROPERTY_DOCCOUNT, fRecentDocCount);
	settings.AddInt8(EL_HMFL_PROPERTY_FOLDERCOUNT, fRecentFolderCount);
	settings.AddInt8(EL_HMFL_PROPERTY_QUERYCOUNT, fRecentQueryCount);
	settings.AddInt8(EL_HMFL_PROPERTY_FONTSIZE, fFontSize);

	// window settings
	switch(fWindowLook)
	{
		case B_TITLED_WINDOW_LOOK:
		{
			settings.AddInt8(EL_HMFL_PROPERTY_WINDOWLOOK, 0);
			break;
		}
		case B_FLOATING_WINDOW_LOOK:
		{
			settings.AddInt8(EL_HMFL_PROPERTY_WINDOWLOOK, 1);
			break;
		}
		case B_MODAL_WINDOW_LOOK:
		{
			settings.AddInt8(EL_HMFL_PROPERTY_WINDOWLOOK, 2);
			break;
		}
		case B_BORDERED_WINDOW_LOOK:
		{
			settings.AddInt8(EL_HMFL_PROPERTY_WINDOWLOOK, 3);
			break;
		}
	}
	settings.AddInt16(EL_HMFL_PROPERTY_WINDOWFRAMELEFT, fWindowFrame.left);
	settings.AddInt16(EL_HMFL_PROPERTY_WINDOWFRAMETOP, fWindowFrame.top);
	settings.AddInt16(EL_HMFL_PROPERTY_WINDOWFRAMERIGHT, fWindowFrame.right);
	settings.AddInt16(EL_HMFL_PROPERTY_WINDOWFRAMEBOTTOM, fWindowFrame.bottom);

	// exclusion settings
	settings.AddMessage(EL_HMFL_PROPERTY_APP_EXCLUSIONS, &fExclusionsList);

	// Save file
	_StopWatching();
	BFile settingsFile(fSettingsPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	settings.Flatten(&settingsFile);
	_StartWatching();

	return B_OK;
}


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
						{	printf("Launcher settings file changed, loading new settings...\n");
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
LauncherSettingsFile::GetScales(ScaleSettings* settings)
{
	settings->launches_scale=fLaunchScale;
	settings->first_launch_scale=fFirstScale;
	settings->last_launch_scale=fLastScale;
	settings->interval_scale=fIntervalScale;
	settings->total_run_time_scale=fRuntimeScale;
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


void
LauncherSettingsFile::SaveScales(ScaleSettings scales)
{
	fLaunchScale=scales.launches_scale;
	fFirstScale=scales.first_launch_scale;
	fLastScale=scales.last_launch_scale;
	fIntervalScale=scales.interval_scale;
	fRuntimeScale=scales.total_run_time_scale;
	_WriteSettingsToFile();
}

/*
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
}*/


void
LauncherSettingsFile::SaveEngineAutoLaunch(bool autoLaunch)
{
	fLaunchEngineOnStart = autoLaunch;
	_WriteSettingsToFile();
}


void
LauncherSettingsFile::SaveDeskbarCount(int count)
{
	fAppCount = count;
	_WriteSettingsToFile();
}


void
LauncherSettingsFile::SaveShowDeskbarMenu(bool show)
{
	fShowDeskbarMenu = show;
	_WriteSettingsToFile();
}


void
LauncherSettingsFile::SaveExclusionsList(BMessage &exclusionsList)
{
	fExclusionsList.MakeEmpty();
	fExclusionsList = exclusionsList;
	_WriteSettingsToFile();
}


void
LauncherSettingsFile::AddToExclusionsList(const char *signature, const char *name)
{
	fExclusionsList.AddString(EL_EXCLUDE_SIGNATURE, signature);
	fExclusionsList.AddString(EL_EXCLUDE_NAME, name);
	_WriteSettingsToFile();
}


void
LauncherSettingsFile::AddToExclusionsList(BMessage *msg)
{
	type_code typeFound;
	int32 signatureCount = 0;
	status_t result1 = msg->GetInfo(EL_EXCLUDE_SIGNATURE, &typeFound, &signatureCount);
	BString sig, name;
	for(int i=0; i<signatureCount; i++)
	{
		sig.SetTo("");
		name.SetTo("");
		msg->FindString(EL_EXCLUDE_SIGNATURE, i, &sig);
		msg->FindString(EL_EXCLUDE_NAME, i, &name);
		fExclusionsList.AddString(EL_EXCLUDE_SIGNATURE, sig);
		fExclusionsList.AddString(EL_EXCLUDE_NAME, name);
	}
	_WriteSettingsToFile();
}


void
LauncherSettingsFile::SaveAppIconSize(int minSize, int maxSize)
{
	fMinAppIconSize = minSize;
	fMaxAppIconSize = maxSize;
	_WriteSettingsToFile();
}


void
LauncherSettingsFile::SaveDocIconSize(int size)
{
	fDocIconSize = size;
	_WriteSettingsToFile();
}

void
LauncherSettingsFile::SaveDocCount(uint count)
{
	fRecentDocCount = count;
	_WriteSettingsToFile();
}

void
LauncherSettingsFile::SaveFolderCount(uint count)
{
	fRecentFolderCount = count;
	_WriteSettingsToFile();
}

void
LauncherSettingsFile::SaveQueryCount(uint count)
{
	fRecentQueryCount = count;
	_WriteSettingsToFile();
}

void
LauncherSettingsFile::SaveFontSize(float fontSize)
{
	fFontSize = fontSize;
	_WriteSettingsToFile();
}

void
LauncherSettingsFile::SaveWindowLook(window_look look)
{
	fWindowLook = look;
	_WriteSettingsToFile();
}


void
LauncherSettingsFile::SaveWindowFrame(BRect frame)
{
	fWindowFrame = frame;
	_WriteSettingsToFile();
}
