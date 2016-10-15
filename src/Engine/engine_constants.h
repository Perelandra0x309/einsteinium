/* engine_constants.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EE_CONSTANTS_H
#define EE_CONSTANTS_H

/*	Any global constants should be declared here */

#define e_app_attr_filetype "document/x-E-app-attr"
//Main directory for all Einsteinium settings
#define e_settings_dir "Einsteinium"
#define e_settings_app_dir "Applications"

//Attribute names
#define ATTR_SESSION_NAME "EIN:SESSION"
#define ATTR_APP_PATH_NAME "EIN:APP_PATH"
#define ATTR_APP_SIG_NAME "EIN:APP_SIG"
#define ATTR_APP_FILENAME_NAME "EIN:APP_FILENAME"
#define ATTR_LAUNCHES_NAME "EIN:LAUNCHES"
//#define ATTR_SCORE_NAME "EIN:SCORE"
//#define ATTR_IGNORE_NAME "EIN:IGNORE"
#define ATTR_LAST_LAUNCH_NAME "EIN:LAST_LAUNCH"
#define ATTR_FIRST_LAUNCH_NAME "EIN:FIRST_LAUNCH"
#define ATTR_LAST_INTERVAL_NAME "EIN:LAST_INTERVAL"
#define ATTR_TOTAL_RUNTIME_NAME "EIN:TOTAL_RUN_TIME"

//Quartile array indexes
enum quartile_indexes
{	Q_FIRST_LAUNCH_INDEX=0,
	Q_LAST_LAUNCH_INDEX=5,
	Q_LAST_INTERVAL_INDEX=10,
	Q_LAUNCHES_INDEX=15,
	Q_TOTAL_RUN_TIME_INDEX=20,
	Q_ARRAY_SIZE=25
};

//Messages
enum engine_messages
{	E_PRINT_RANKING_APPS='Epra',
	E_PRINT_RECENT_APPS='Eprc',
	E_REPLY_RANKING_APPS='Erra',
	E_RANKING_APPS_REPLY='Erar',
	E_UPDATE_QUARTILES='Eupq',
	E_RESCAN_DATA_FILES,
//	E_UPDATE_SCORES,
	E_TEST_MODE_INIT
};

//Actions
enum sort_actions
{
	SORT_BY_NONE=1000,
	SORT_BY_SCORE,
	SORT_BY_LAST_LAUNCHTIME,
	SORT_BY_FIRST_LAUNCHTIME,
	SORT_BY_LAUNCH_COUNT,
	SORT_BY_LAST_INTERVAL,
	SORT_BY_TOTAL_RUNNING_TIME
};
enum attr_file_actions
{	UPDATE_ATTR_SCORE=2000,
	RESCAN_ATTR_DATA,
	CREATE_APP_LIST
};

#endif
