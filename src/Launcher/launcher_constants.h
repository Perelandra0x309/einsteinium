/* launcher_constants.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EL_CONSTANTS_H
#define EL_CONSTANTS_H

#include <Window.h>

//bool IsEngineRunning();

//Main directory for all Einsteinium settings
#define e_settings_dir "Einsteinium"
//Settings file name
#define el_settings_file "launcher"
#define el_legacy_settings_file "launcher_settings"

//static const char* settings_filename = "Launcher_settings";
const int32 kIconDefaultSize = 32;
const int kIconMargin = 2;
const int kTextMargin = 2;
const uint32 kPreventHideModifier = B_SHIFT_KEY | B_OPTION_KEY | B_COMMAND_KEY;
const uint32 kExcludeAppModifier = B_CONTROL_KEY;
static const char* kQueryType = "application/x-vnd.Be-query";
static const char* kQueryTemplateType = "application/x-vnd.Be-queryTemplate";
static const char* kApplicationType = "application/x-vnd.Be-elfexecutable";
static const char* kDirectoryType = "application/x-vnd.Be-directory";

//Settings flat BMessage text
#define EL_HMFL_PROPERTY_AUTOLAUNCH "auto_launch"
#define EL_HMFL_PROPERTY_LAUNCHES "launches"
#define EL_HMFL_PROPERTY_FIRSTLAUNCH "first_launch"
#define EL_HMFL_PROPERTY_LASTLAUNCH "last_launch"
#define EL_HMFL_PROPERTY_INTERVAL "interval"
#define EL_HMFL_PROPERTY_RUNTIME "run_time"
#define EL_HMFL_PROPERTY_SHOWDESKBAR "show_deskbar"
#define EL_HMFL_PROPERTY_APPCOUNT "app_count"
#define EL_HMFL_PROPERTY_MAXAPP "max_app_icon"
#define EL_HMFL_PROPERTY_MINAPP "min_app_icon"
#define EL_HMFL_PROPERTY_DOCICON	"doc_icon"
#define EL_HMFL_PROPERTY_DOCCOUNT "doc_count"
#define EL_HMFL_PROPERTY_FOLDERCOUNT "folder_count"
#define EL_HMFL_PROPERTY_QUERYCOUNT "query_count"
#define EL_HMFL_PROPERTY_FONTSIZE "font_size"
#define EL_HMFL_PROPERTY_WINDOWLOOK "window_look"
#define EL_HMFL_PROPERTY_WINDOWFRAMELEFT "frame_left"
#define EL_HMFL_PROPERTY_WINDOWFRAMETOP "frame_top"
#define EL_HMFL_PROPERTY_WINDOWFRAMERIGHT "frame_right"
#define EL_HMFL_PROPERTY_WINDOWFRAMEBOTTOM "frame_bottom"
#define EL_HMFL_PROPERTY_APP_EXCLUSIONS "app_exclusions"

//XML Text (legacy settings file)
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
#define EL_XMLTEXT_CHILD_NAME_LAYOUT "layout"
#define EL_XMLTEXT_PROPERTY_APPCOUNT "app_count"
#define EL_XMLTEXT_PROPERTY_MAXAPP "max_app_icon"
#define EL_XMLTEXT_PROPERTY_MINAPP "min_app_icon"
#define EL_XMLTEXT_PROPERTY_DOCICON	"doc_icon"
#define EL_XMLTEXT_PROPERTY_DOCCOUNT "doc_count"
#define EL_XMLTEXT_PROPERTY_FOLDERCOUNT "folder_count"
#define EL_XMLTEXT_PROPERTY_QUERYCOUNT "query_count"
#define EL_XMLTEXT_PROPERTY_FONTSIZE "font_size"
#define EL_XMLTEXT_CHILD_NAME_WINDOW "window"
#define EL_XMLTEXT_PROPERTY_WINDOWLOOK "look"
#define EL_XMLTEXT_PROPERTY_WINDOWFRAMELEFT "left"
#define EL_XMLTEXT_PROPERTY_WINDOWFRAMETOP "top"
#define EL_XMLTEXT_PROPERTY_WINDOWFRAMERIGHT "right"
#define EL_XMLTEXT_PROPERTY_WINDOWFRAMEBOTTOM "bottom"
#define EL_XMLTEXT_CHILD_NAME_EXCLUSIONS "app_exclusions"
#define EL_XMLTEXT_CHILD_NAME_APP "app"
#define EL_XMLTEXT_PROPERTY_SIGNATURE "sig"
#define EL_XMLTEXT_PROPERTY_NAME "name"
#define EL_XMLTEXT_CHILD_NAME_ENGINE "engine"
#define EL_XMLTEXT_PROPERTY_AUTOLAUNCH "auto_launch"


// Other text
#define EL_SHELFVIEW_NAME "el_shelfview"
#define EL_TOOLTIP_TEXT "Einsteinium  \nLauncher"
#define EL_EXCLUDE_SIGNATURE E_SUBSCRIPTION_EXCLUSIONS
#define EL_EXCLUDE_NAME "name"
#define EL_REMOVE_APPLICATION "RemoveApp"
#define EL_UNKNOWN_SUPERTYPE "Unknown"
#define EL_SUPERTYPE_NAMES "names"
#define EL_INFO_STRING "info_text"

//Defaults
#define DEFAULT_VALUE_LAUNCH_SCALE 4
#define DEFAULT_VALUE_FIRST_SCALE 0
#define DEFAULT_VALUE_LAST_SCALE 0
#define DEFAULT_VALUE_INTERVAL_SCALE 0
#define DEFAULT_VALUE_RUNTIME_SCALE 0
#define DEFAULT_LAUNCH_ENGINE_ON_START true
#define DEFAULT_VALUE_RECENT_COUNT 25
#define DEFAULT_VALUE_SHOW_DESKBAR_MENU true

// Preferences
/*#define NAME_LINES_OPTION "linesOption"
#define NAME_MAX_ICON_OPTION "maxIconSize"
#define NAME_MIN_ICON_OPTION "minIconSize"
#define NAME_DOC_ICON_OPTION "docIconSize"
#define NAME_DOC_COUNT_OPTION "docCount"
#define NAME_FONT_OPTION "fontSize"
#define NAME_WINDOW_FRAME "windowFrame"
#define NAME_LOOK_OPTION "windowLook"
#define NAME_FEEL_OPTION "windowFeel"*/

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
	EL_SHELFVIEW_MENUITEM_INVOKED,
	EL_SHELFVIEW_MENU_QUIT,
	EL_MESSAGE_WHAT_EXCLUDED_APPS,
	EL_SETTINGS_FILE_CHANGED_EXTERNALLY,
	EL_START_ENGINE_ALERT,
	EL_CHECK_ENGINE_STATUS,
	EL_GOTO_LAUNCHER_SETTINGS,
	EL_START_ENGINE,
//	EL_RESTART_SERVICE,
	EL_START_SERVICE,
	EL_STOP_SERVICE,
	EL_SHOW_SETTINGS,
//	EL_LINES_OPTION_CHANGED,
	EL_APP_ICON_OPTION_CHANGED,
	EL_DOC_ICON_OPTION_CHANGED,
	EL_FONT_OPTION_CHANGED,
	EL_LOOK_OPTION_CHANGED,
	EL_FLOAT_OPTION_CHANGED,
	EL_HIDE_APP,
	EL_UPDATE_RECENT_LISTS,
	EL_SHOW_IN_TRACKER,
	EL_APP_ICON_OPTION_DRAG,
	EL_DOC_ICON_OPTION_DRAG,
	EL_DOC_COUNT_OPTION_CHANGED,
	EL_ADD_APP_EXCLUSION,
	EL_REDIRECTED_MOUSE_WHEEL_CHANGED,
	EL_UPDATE_RECENT_LISTS_FORCE,
	EL_WINDOW_MOVED,
	EL_APP_COUNT_OPTION_CHANGED,
	EL_DESKBAR_OPTION_CHANGED,
	//Launcher settings messages
	EL_LAUNCHES_SL_CHANGED,
	EL_FIRST_SL_CHANGED,
	EL_LAST_SL_CHANGED,
	EL_INTERVAL_SL_CHANGED,
	EL_RUNTIME_SL_CHANGED,
	EL_SAVE_RANKING,
	EL_EXCLUSION_SELECTION_CHANGED,
	EL_LIST_INCLUSION_CHANGED,
	EL_ADD_EXCLUSION,
	EL_ADD_EXCLUSION_REF,
	EL_REMOVE_EXCLUSION,
	EL_EXCLUSIONS_CHANGED,
	EL_DESKBAR_CHANGED,
	EL_ENGINELAUNCH_CHANGED,
	EL_FOLDER_COUNT_OPTION_CHANGED,
	EL_QUERY_COUNT_OPTION_CHANGED,
	EL_UPDATE_INFOVIEW,
	EL_QUIT_FROM_MENUBAR,
	EL_SHOW_SETTINGS_LAYOUT,
	EL_SHOW_SETTINGS_RANKINGS,
	EL_SHOW_SETTINGS_EXCLUSIONS,
	EL_INITIATE_INFO_VIEW_UPDATE
};

#endif
