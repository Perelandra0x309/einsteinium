/* prefs_constants.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_CONSTANTS_H
#define EP_CONSTANTS_H

/*	constant definitions and message commands */

#define BORDER_SIZE 5
static const char* app_sig = "application/x-vnd.Einsteinium_Preferences";
static const rgb_color bg_color = {216,216,216,0};//grey background color

//settings
struct scale_settings
{	int		launches_scale, first_launch_scale, last_launch_scale,
			interval_scale, total_run_time_scale;

};

//Messages
enum prefs_messages
{	//Daemon settings messages
	ED_RELAPP_SELECTION_CHANGED=1,
	ED_ADD_APPITEM,
	ED_ADD_APPITEM_REF,
	ED_REMOVE_APPITEM,
	ED_AUTO_RELAUNCH_CHANGED,
//	ED_PROMPT_RELAUNCH_CHANGED,
	//Engine settings messages

	//Launcher settings messages
	EL_LAUNCHES_SL_CHANGED,
	EL_FIRST_SL_CHANGED,
	EL_LAST_SL_CHANGED,
	EL_INTERVAL_SL_CHANGED,
	EL_RUNTIME_SL_CHANGED,
//	EE_IGNORE_ATTR_CHANGED,
	EL_EXCLUSION_SELECTION_CHANGED,
	EL_LIST_INCLUSION_CHANGED,
	EL_ADD_EXCLUSION,
	EL_ADD_EXCLUSION_REF,
	EL_REMOVE_EXCLUSION,
	EL_DESKBAR_CHANGED,
	//app messages
	PREFS_ITEM_CHANGED,
	EL_SAVE_RANKING,
	E_RECALC_SCORES,
	E_RECALC_QUARTS,
	E_RESCAN_DATA,
	//services
	RESTART_SERVICE,
	START_SERVICE,
	STOP_SERVICE

};

//Colors
const rgb_color enabled_color = {0,0,0,0};
const rgb_color disabled_color = {100,100,100,0};
const rgb_color auto_relaunch_color = {0, 100, 0, 0};
const rgb_color prompt_relaunch_color = {0, 0, 150, 0};
const rgb_color dontrelaunch_color = {150,0,0,0};
const rgb_color selected_color = {178,178,178,255};

#endif
