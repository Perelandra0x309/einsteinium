/* LauncherSettingsFile.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherSettingsFile.h"


LauncherSettingsFile::LauncherSettingsFile(BHandler *messageHandler=NULL)
	:
	BHandler("name"),
	fStatus(B_ERROR),
	fExternalMessageHandler(messageHandler),
	fLaunchScale(DEFAULT_VALUE_LAUNCH_SCALE),
	fFirstScale(DEFAULT_VALUE_FIRST_SCALE),
	fLastScale(DEFAULT_VALUE_LAST_SCALE),
	fIntervalScale(DEFAULT_VALUE_INTERVAL_SCALE),
	fRuntimeScale(DEFAULT_VALUE_RUNTIME_SCALE),
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
	if(!settingsEntry.Exists())
	{
		//settings file doesn't exist, create default file
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
/*	printf("Watching engine settings node was %ssuccessful.\n", fWatchingSettingsNode? "": "not ");
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
//			xmlChar *value = xmlGetProp(cur, (const xmlChar *) EL_XMLTEXT_PROPERTY_SHOW);
//			fShowDeskbarMenu = strcmp("true", (char *)value)==0;
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
LauncherSettingsFile::_XmlGetIntProp(xmlNodePtr cur, char *name, int defaultValue=0)
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
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL, child1node = NULL, child2node = NULL;
	xmlDtdPtr dtd = NULL;

	BString launchChar, firstChar, lastChar, intervalChar, runtimeChar;
	launchChar << fLaunchScale;
	firstChar << fFirstScale;
	lastChar << fLastScale;
	intervalChar << fIntervalScale;
	runtimeChar << fRuntimeScale;
	BString minAppIconChar, maxAppIconChar, docIconChar, docCountChar, folderCountChar, queryCountChar, fontSizeChar;
	minAppIconChar << fMinAppIconSize;
	maxAppIconChar << fMaxAppIconSize;
	docIconChar << fDocIconSize;
	docCountChar << fRecentDocCount;
	folderCountChar << fRecentFolderCount;
	queryCountChar << fRecentQueryCount;
	fontSizeChar << int(fFontSize);
	BString windowLookChar;
	switch(fWindowLook){
		case B_TITLED_WINDOW_LOOK:
		{
			windowLookChar << 0;
			break;
		}
		case B_FLOATING_WINDOW_LOOK:
		{
			windowLookChar << 1;
			break;
		}
		case B_MODAL_WINDOW_LOOK:
		{
			windowLookChar << 2;
			break;
		}
		case B_BORDERED_WINDOW_LOOK:
		{
			windowLookChar << 3;
			break;
		}
	}
	BString frameLeftChar, frameTopChar, frameRightChar, frameBottomChar;
	frameLeftChar << fWindowFrame.left;
	frameTopChar << fWindowFrame.top;
	frameRightChar << fWindowFrame.right;
	frameBottomChar << fWindowFrame.bottom;


	BString autoLaunchValue;
	if(fLaunchEngineOnStart)
		autoLaunchValue.SetTo("true");
	else
		autoLaunchValue.SetTo("false");

	BString countChar;
	countChar << fAppCount;

	//Creates a new document, a node and set it as a root node
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST EL_XMLTEXT_ROOT_NAME);
	xmlDocSetRootElement(doc, root_node);
	dtd = xmlCreateIntSubset(doc, BAD_CAST EL_XMLTEXT_ROOT_NAME, NULL, BAD_CAST "tree.dtd");

	// write default link inclusion value
/*	child1node = xmlNewChild(root_node, NULL, BAD_CAST EL_XMLTEXT_CHILD_NAME_INCLUSION, NULL);
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_INCLUSION_DEFAULT,
				BAD_CAST fInclusionDefault.String() );
	*/
	// Automatically launch the Engine when Launcher starts
	child1node = xmlNewChild(root_node, NULL, BAD_CAST EL_XMLTEXT_CHILD_NAME_ENGINE, NULL);
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_AUTOLAUNCH,
				BAD_CAST autoLaunchValue.String() );

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

	// layout settings
	child1node = xmlNewChild(root_node, NULL, BAD_CAST EL_XMLTEXT_CHILD_NAME_LAYOUT, NULL);
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_APPCOUNT,
				BAD_CAST countChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_MINAPP,
				BAD_CAST minAppIconChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_MAXAPP,
				BAD_CAST maxAppIconChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_DOCICON,
				BAD_CAST docIconChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_DOCCOUNT,
				BAD_CAST docCountChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_FOLDERCOUNT,
				BAD_CAST folderCountChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_QUERYCOUNT,
				BAD_CAST queryCountChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_FONTSIZE,
				BAD_CAST fontSizeChar.String() );

	// window settings
	child1node = xmlNewChild(root_node, NULL, BAD_CAST EL_XMLTEXT_CHILD_NAME_WINDOW, NULL);
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_WINDOWLOOK,
				BAD_CAST windowLookChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_WINDOWFRAMELEFT,
				BAD_CAST frameLeftChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_WINDOWFRAMETOP,
				BAD_CAST frameTopChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_WINDOWFRAMERIGHT,
				BAD_CAST frameRightChar.String() );
	xmlNewProp(child1node, BAD_CAST EL_XMLTEXT_PROPERTY_WINDOWFRAMEBOTTOM,
				BAD_CAST frameBottomChar.String() );

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
