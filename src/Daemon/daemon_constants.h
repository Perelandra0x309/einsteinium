/* daemon_constants.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef ED_CONSTANTS_H
#define ED_CONSTANTS_H

//Main directory for all Einsteinium settings
#define e_settings_dir "Einsteinium"
//Settings file name
#define ed_settings_file "daemon_settings"

//Enumerations for messages
enum {
	ED_LAUNCH_APP,
	ED_SETTINGS_FILE_CHANGED_EXTERNALLY
};

enum {
//	ACTION_NONE = 0,
	ACTION_AUTO,
	ACTION_PROMPT,
	ACTION_IGNORE,
	ACTION_DEFAULT = ACTION_PROMPT
};

//Text
#define ED_XMLTEXT_ROOT_NAME "einsteinium_daemon"
#define ED_XMLTEXT_CHILD1_NAME "relaunch"
#define ED_XMLTEXT_CHILD2_NAME "app"
#define ED_XMLTEXT_PROPERTY_SIGNATURE "sig"
#define ED_XMLTEXT_VALUE_DEFAULT "default"
#define ED_XMLTEXT_PROPERTY_RELAUNCH "relaunch"
#define ED_XMLTEXT_VALUE_IGNORE "ignore"
#define ED_XMLTEXT_VALUE_PROMPT "prompt"
#define ED_XMLTEXT_VALUE_AUTO "auto"
#define ED_ALERT_REMEMBER "remember"

//Paths of executables to ignore by default
#define PATH_SYSTEM_BIN "/boot/system/bin"
#define PATH_SYSTEM_SERVERS "/boot/system/servers"
#define PATH_SYSTEM_PREFERENCES "/boot/system/preferences"
#define PATH_SYSTEM_TRACKER_ADDONS "/boot/system/add-ons/Tracker"
#define PATH_COMMON_BIN "/boot/common/bin"
#define PATH_COMMON_TRACKER_ADDONS "/boot/common/add-ons/Tracker"
#define PATH_HOME_BIN "/boot/home/config/bin"
#define PATH_HOME_TRACKER_ADDONS "/boot/home/config/add-ons/Tracker"

#endif
