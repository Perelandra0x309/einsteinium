/* daemon_constants.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef ED_CONSTANTS_H
#define ED_CONSTANTS_H

//Einsteinium_daemon application signature
#define e_daemon_sig "application/x-vnd.Einsteinium_Daemon"
//Main directory for all Einsteinium settings
#define e_settings_dir "Einsteinium"
//Settings file name
#define ed_settings_file "daemon_settings"

//Enumerations for messages
enum {
	LAUNCH_APP,
//	LAUNCH_APP_NEW
};

enum {
//	ACTION_NONE = 0,
	ACTION_AUTO,
	ACTION_PROMPT,
	ACTION_IGNORE
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

#endif
