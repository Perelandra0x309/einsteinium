/* launcher_constants.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EL_CONSTANTS_H
#define EL_CONSTANTS_H

//Einsteinium_daemon application signature
#define e_launcher_sig "application/x-vnd.Einsteinium_Launcher"
//Main directory for all Einsteinium settings
#define e_settings_dir "Einsteinium"
//Settings file name
#define el_settings_file "launcher_settings"

//XML Text
#define EL_XMLTEXT_ROOT_NAME "einsteinium_launcher"
#define EL_XMLTEXT_CHILD_NAME_INCLUSION "list_inclusion"
#define EL_XMLTEXT_PROPERTY_INCLUSION_DEFAULT "default"
#define EL_XMLTEXT_VALUE_INCLUDE "include"
#define EL_XMLTEXT_VALUE_PROMPT "prompt"
#define EL_XMLTEXT_VALUE_EXCLUDE "exclude"
#define EL_XMLTEXT_CHILD_NAME_RANK "rank_scales"
#define EL_XMLTEXT_PROPERTY_LAUNCHES "launches"
#define EL_XMLTEXT_PROPERTY_FIRSTLAUNCH "first_launch"
#define EL_XMLTEXT_PROPERTY_LASTLAUNCH "last_launch"
#define EL_XMLTEXT_PROPERTY_INTERVAL "interval"
#define EL_XMLTEXT_PROPERTY_RUNTIME "run_time"
#define EL_XMLTEXT_CHILD_NAME_DESKBAR "deskbar"
#define EL_XMLTEXT_PROPERTY_SHOW "show"
#define EL_XMLTEXT_PROPERTY_COUNT "count"
#define EL_XMLTEXT_CHILD_NAME_EXCLUSIONS "app_exclusions"
#define EL_XMLTEXT_CHILD_NAME_APP "app"
#define EL_XMLTEXT_PROPERTY_SIGNATURE "sig"
#define EL_XMLTEXT_PROPERTY_NAME "name"

// Other text
#define EL_SHELFVIEW_NAME "el_shelfview"
#define EL_TOOLTIP_TEXT "Einsteinium  \nLauncher"
#define EL_EXCLUDE_SIGNATURE E_SUBSCRIPTION_EXCLUSIONS
#define EL_EXCLUDE_NAME "name"

//Defaults
#define DEFAULT_VALUE_LAUNCH_SCALE 4
#define DEFAULT_VALUE_FIRST_SCALE 0
#define DEFAULT_VALUE_LAST_SCALE 0
#define DEFAULT_VALUE_INTERVAL_SCALE 0
#define DEFAULT_VALUE_RUNTIME_SCALE 0

//Indexes
enum scales_index
{	LAUNCH_INDEX=0,
	FIRST_INDEX,
	LAST_INDEX,
	INTERVAL_INDEX,
	RUNTIME_INDEX
};

//Enumerations for messages
enum launcher_messages
{	EL_SHELFVIEW_OPENPREFS='Esop',
	EL_SHELFVIEW_LAUNCH_REF,
	EL_SHELFVIEW_MENU_QUIT,
	EL_MESSAGE_WHAT_EXCLUDED_APPS,
	EL_SETTINGS_FILE_CHANGED_EXTERNALLY,
	EL_START_ENGINE_ALERT,
	EL_CHECK_ENGINE_STATUS,
	EL_GOTO_LAUNCHER_SETTINGS,
	EL_START_ENGINE
};

#endif
