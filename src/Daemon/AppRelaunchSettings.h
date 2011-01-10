/* AppRelaunchSettings.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef ED_APPRELAUNCHSETTINGS_H
#define ED_APPRELAUNCHSETTINGS_H

#include <StorageKit.h>
#include <String.h>
#include <Roster.h>
#include "daemon_constants.h"


class AppRelaunchSettings {
public:
					AppRelaunchSettings();
					AppRelaunchSettings(AppRelaunchSettings*);
					AppRelaunchSettings(const char *_signature, int _relaunch = ACTION_DEFAULT);
					AppRelaunchSettings(const char *_signature, BPath _path,
							int _relaunch = ACTION_DEFAULT);
	int				relaunchAction;
	BString			appPath, appSig, name;
	bool			Equals(AppRelaunchSettings*);
private:
	BEntry			_GetEntryFromSig(const char* sig);
	// TODO add timestamp that records the last time settings were updated?
};

#endif
