/*AppRelaunchSettings.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "AppRelaunchSettings.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AppRelaunchSettings.cpp"

AppRelaunchSettings::AppRelaunchSettings()
	:
	appPath(""),
	name(B_TRANSLATE_COMMENT("<Name not found>", "Keep first and last < >")),
	relaunchAction(ACTION_PROMPT)
{	}


AppRelaunchSettings::AppRelaunchSettings(AppRelaunchSettings* sourceSettings)
{
	relaunchAction = sourceSettings->relaunchAction;
	appPath.SetTo(sourceSettings->appPath);
	appSig.SetTo(sourceSettings->appSig);
	name.SetTo(sourceSettings->name);
}


AppRelaunchSettings::AppRelaunchSettings(const char *_signature, int _relaunch = ACTION_DEFAULT)
	:
	appPath(""),
	name(B_TRANSLATE_COMMENT("<Name not found>", "Keep first and last < >"))
{
	appSig.SetTo(_signature);
	relaunchAction = _relaunch;

	//Get the path and file name based on application signature
	BEntry appEntry = _GetEntryFromSig(appSig);
	if(appEntry.Exists()){
		BPath path;
		appEntry.GetPath(&path);
		appPath.SetTo(path.Path());
		name.SetTo(path.Leaf());
	}
}


AppRelaunchSettings::AppRelaunchSettings(const char *_signature, BPath _path,
	int _relaunch = ACTION_DEFAULT)
	:
	appPath(""),
	name(B_TRANSLATE_COMMENT("<Name not found>", "Keep first and last < >"))
{
	appSig.SetTo(_signature);
	appPath.SetTo(_path.Path());
	name.SetTo(_path.Leaf());
	relaunchAction = _relaunch;
}


bool
AppRelaunchSettings::Equals(AppRelaunchSettings* compare)
{
	bool equal = (appPath == compare->appPath
				&& appSig == compare->appSig
				&& name == compare->name
				&& relaunchAction == compare->relaunchAction);
	return equal;
}


BEntry
AppRelaunchSettings::_GetEntryFromSig(const char* sig)
{
	entry_ref ref;
	be_roster->FindApp(sig, &ref);
	BEntry entry(&ref);
	return entry;
}
