/*AppRelaunchSettings.cpp
	Settings read from file
*/
#include "AppRelaunchSettings.h"

AppRelaunchSettings::AppRelaunchSettings()
	:appPath("")
	,name("<App not found>")
	,relaunchAction(ACTION_PROMPT)
{	}

AppRelaunchSettings::AppRelaunchSettings(AppRelaunchSettings* sourceSettings)
{
	relaunchAction = sourceSettings->relaunchAction;
	appPath.SetTo(sourceSettings->appPath);
	appSig.SetTo(sourceSettings->appSig);
	name.SetTo(sourceSettings->name);
}


AppRelaunchSettings::AppRelaunchSettings(const char *_signature, const char *_relaunch = NULL)
	:appPath("")
	,name("<App not found>")
	,relaunchAction(ACTION_PROMPT)
{
	appSig.SetTo(_signature);
	SetRelaunchAction(_relaunch);

	//Get the path and file name based on application signature
	BEntry appEntry = getEntryFromSig(appSig);
	if(appEntry.Exists()){
		BPath path;
		appEntry.GetPath(&path);
		appPath.SetTo(path.Path());
		name.SetTo(path.Leaf());
	}
}


AppRelaunchSettings::AppRelaunchSettings(const char *_signature, BPath _path, char *action = NULL)
	:relaunchAction(ACTION_PROMPT)
{
	appSig.SetTo(_signature);
	appPath.SetTo(_path.Path());
	name.SetTo(_path.Leaf());
	SetRelaunchAction(action);
}

status_t AppRelaunchSettings::SetRelaunchAction(const char *action)
{
	if (action == NULL) { return B_ERROR; }

	if(!strcmp(action, ED_XMLTEXT_VALUE_AUTO))
	{	relaunchAction = ACTION_AUTO; }
	else if(!strcmp(action, ED_XMLTEXT_VALUE_PROMPT))
	{	relaunchAction = ACTION_PROMPT; }
	else if(!strcmp(action, ED_XMLTEXT_VALUE_IGNORE))
	{	relaunchAction = ACTION_IGNORE; }
	else
	{	return B_ERROR; }
	return B_OK;
}

bool AppRelaunchSettings::Equals(AppRelaunchSettings* compare)
{
	bool equal = (appPath == compare->appPath
				&& appSig == compare->appSig
				&& name == compare->name
				&& relaunchAction == compare->relaunchAction);
	return equal;
}


BEntry AppRelaunchSettings::getEntryFromSig(const char* sig)
{
	entry_ref ref;
	be_roster->FindApp(sig, &ref);
	BEntry entry(&ref);
	return entry;
/*BVolumeRoster v_roster;
	BVolume vol;
	BQuery query;
	BEntry entry;
	BString pred("BEOS:APP_SIG==");
	pred.Append(sig);
	v_roster.Rewind();
	while(v_roster.GetNextVolume(&vol)==B_NO_ERROR)
	{	if(vol.KnowsAttr() && vol.KnowsMime() && vol.KnowsQuery())
		{	query.Clear();
			query.SetPredicate(pred.String());
			query.SetVolume(&vol);
			if(query.Fetch()!=B_OK) continue;
			if(query.GetNextEntry(&entry)==B_OK)//entry found
			{	break; }
		}
	}
	return entry;*/
}
