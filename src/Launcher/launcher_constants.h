/* launcher_constants.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EL_CONSTANTS_H
#define EL_CONSTANTS_H

//Einsteinium_daemon application signature
#define e_launcher_sig "application/x-vnd.Einsteinium_Launcher"
//Main directory for all Einsteinium settings

//Settings file name

// Other text
#define EL_SHELFVIEW_NAME "el_shelfview"
#define EL_TOOLTIP_TEXT "Einsteinium  \nRanked\nApplications"

//Enumerations for messages
enum launcher_messages
{	EL_SHELFVIEW_OPENPREFS='Esop',
	EL_SHELFVIEW_OPEN,
	EL_SHELFVIEW_MENU_QUIT
};

#endif
