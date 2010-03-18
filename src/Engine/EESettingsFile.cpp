/*EESettingsFile.cpp

*/
#include "EESettingsFile.h"


EESettingsFile::EESettingsFile()
	: BHandler("name")
	,_status(B_ERROR)
	,launch_scale(DEFAULTVALUE_LAUNCHES)
	,first_scale(DEFAULTVALUE_LAUNCHES)
	,last_scale(DEFAULTVALUE_LAUNCHES)
	,interval_scale(DEFAULTVALUE_LAUNCHES)
	,runtime_scale(DEFAULTVALUE_LAUNCHES)
	,inclusionDefault(EE_XMLTEXT_VALUE_PROMPT)
	,showDeskbarMenu(false)
	,deskbarMenuCount(0)
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
	settingsPath.Append(ee_settings_file);
	BEntry settingsEntry(settingsPath.Path());
	if(!settingsEntry.Exists())//settings file doesn't exist, create default
	{
		//settings file doesn't exist, create default
		WriteSettingsToFile();
	}
	else
	{
		ReadSettingsFromFile(settingsPath);
	}
	settingsEntry.GetNodeRef(&settingsNodeRef);

	//Start the Looper running
	watchingLooper = new BLooper("settings");
	watchingLooper->AddHandler(this);
	watchingLooper->Run();

	//watch the settings file for changes
	StartWatching();

	_status = B_OK;
}


EESettingsFile::~EESettingsFile()
{
	StopWatching();
	watchingLooper->Lock();
	watchingLooper->Quit();
}

void EESettingsFile::StartWatching(){
	status_t error = watch_node(&settingsNodeRef, B_WATCH_STAT, this, watchingLooper);
	watchingSettingsNode = (error==B_OK);
	printf("Watching engine settings node was %ssuccessful.\n", watchingSettingsNode? "": "not ");
	if(!watchingSettingsNode) printf("Error=%i (B_BAD_VALUE=%i, B_NO_MEMORY=%i, B_ERROR=%i)\n",
			error, B_BAD_VALUE, B_NO_MEMORY, B_ERROR);
}

void EESettingsFile::StopWatching(){
	if(watchingSettingsNode)//settings file is being watched
	{	watch_node(&settingsNodeRef, B_STOP_WATCHING, this, watchingLooper);
		printf("Stopped watching engine settings node\n");
		watchingSettingsNode = false;
	}
}


void EESettingsFile::ReadSettingsFromFile(BPath file)
{
	//Parse XML
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
			inclusionDefault.SetTo((char *)value);
		}
		// scale settings
		if (!xmlStrcmp(cur->name, (const xmlChar *) EE_XMLTEXT_CHILD_NAME_RANK))
		{
			launch_scale = xmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_LAUNCHES);
			first_scale = xmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_FIRSTLAUNCH);
			last_scale = xmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_LASTLAUNCH);
			interval_scale = xmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_INTERVAL);
			runtime_scale = xmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_RUNTIME);
		}
		// deskbar settings
		if (!xmlStrcmp(cur->name, (const xmlChar *) EE_XMLTEXT_CHILD_NAME_DESKBAR))
		{
			xmlChar *value = xmlGetProp(cur, (const xmlChar *) EE_XMLTEXT_PROPERTY_SHOW);
			showDeskbarMenu = strcmp("true", (char *)value)==0;
			deskbarMenuCount = xmlGetIntProp(cur, EE_XMLTEXT_PROPERTY_COUNT);
		}

		cur = cur->next;
	}

}


int EESettingsFile::xmlGetIntProp(xmlNodePtr cur, char *name){
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


status_t EESettingsFile::WriteSettingsToFile()
{
	char launchChar[32], firstChar[32], lastChar[32], intervalChar[32], runtimeChar[32];
	sprintf(launchChar, "%ld", launch_scale);
	sprintf(firstChar, "%ld", first_scale);
	sprintf(lastChar, "%ld", last_scale);
	sprintf(intervalChar, "%ld", interval_scale);
	sprintf(runtimeChar, "%ld", runtime_scale);
	BString showDeskbar;
	if(showDeskbarMenu) showDeskbar.SetTo("true");
	else showDeskbar.SetTo("false");
	char countChar[32];
	sprintf(countChar, "%i", deskbarMenuCount);

	BString xml_text("<?xml version=\"1.0\"?>\n");
	xml_text.Append("<").Append(EE_XMLTEXT_ROOT_NAME).Append(">\n");
	// write default link inclusion value
	xml_text.Append("	<").Append(EE_XMLTEXT_CHILD_NAME_INCLUSION)
			.Append(" ").Append(EE_XMLTEXT_PROPERTY_INCLUSION_DEFAULT)
			.Append("=\"").Append(inclusionDefault).Append("\"/>\n");

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


	//Stop watching file
	StopWatching();

	BFile eds_file;
	status_t result = eds_file.SetTo(settingsPath.Path(),
				B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if(result != B_OK)
	{
		StartWatching();
		return B_ERROR;
	}
	eds_file.Write(xml_text.String(), xml_text.Length());
	eds_file.Unset();

	//Start watching file
	StartWatching();

	return B_OK;
}


void EESettingsFile::MessageReceived(BMessage *msg)
{	switch(msg->what)//act according to the Message command
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
						{	printf("Engine settings file changed, loading new settings...\n");
							ReadSettingsFromFile(settingsPath);
						}
						break; }
				}
			}
			break;
		}
	}
}


status_t EESettingsFile::CheckStatus(){
	return _status;
}

int* EESettingsFile::GetScales(){
	scales[LAUNCH_INDEX]=launch_scale;
	scales[FIRST_INDEX]=first_scale;
	scales[LAST_INDEX]=last_scale;
	scales[INTERVAL_INDEX]=interval_scale;
	scales[RUNTIME_INDEX]=runtime_scale;

	return scales;
}

void EESettingsFile::SaveScales(int *scales)
{
	launch_scale=scales[LAUNCH_INDEX];
	first_scale=scales[FIRST_INDEX];
	last_scale=scales[LAST_INDEX];
	interval_scale=scales[INTERVAL_INDEX];
	runtime_scale=scales[RUNTIME_INDEX];
	WriteSettingsToFile();
}

const char* EESettingsFile::GetLinkInclusionDefaultValue()
{
	return inclusionDefault.String();
}

void EESettingsFile::SaveLinkInclusionDefaultValue(const char *value)
{
	// Only save setting when one of these three values
	if( strcmp(value,EE_XMLTEXT_VALUE_INCLUDE)==0
		|| strcmp(value,EE_XMLTEXT_VALUE_PROMPT)==0
		|| strcmp(value,EE_XMLTEXT_VALUE_EXCLUDE)==0 )
	{
		inclusionDefault.SetTo(value);
		WriteSettingsToFile();
	}
}

void EESettingsFile::GetDeskbarSettings(bool &show, int &count)
{
	show = showDeskbarMenu;
	count = deskbarMenuCount;
}

void EESettingsFile::SaveDeskbarSettings(bool show, int count)
{
	showDeskbarMenu = show;
	deskbarMenuCount = count;
	WriteSettingsToFile();
}
