/*AppRelaunchSettings.h
	Settings read from file
*/
#ifndef ED_APPRELAUNCHSETTINGS_H
#define ED_APPRELAUNCHSETTINGS_H

#include <StorageKit.h>
#include <String.h>
#include <Roster.h>
#include "daemon_constants.h"


class AppRelaunchSettings
{public:
					AppRelaunchSettings();
					AppRelaunchSettings(AppRelaunchSettings*);
					AppRelaunchSettings(const char *_signature, const char *_relaunch = NULL);
					AppRelaunchSettings(const char *_signature, BPath _path, char *action = NULL);
	int				relaunchAction;
	BString			appPath, appSig, name;
	status_t		SetRelaunchAction(const char *action);
	BString			GetRelaunchActionString();
	bool			Equals(AppRelaunchSettings*);
private:
	BEntry			getEntryFromSig(const char* sig);
	// TODO add timestamp that records the last time settings were updated?
};

#endif
