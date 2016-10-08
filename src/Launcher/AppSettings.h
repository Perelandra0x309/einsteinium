/* AppSettings.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EL_APPSETTINGS_H
#define EL_APPSETTINGS_H

#include <StringList.h>

struct AppSettings {
//	bool drawTwoLines;
	int minIconSize, maxIconSize, docIconSize;
	uint appCount, recentDocCount, recentFolderCount, recentQueryCount;
	float fontSize;
	window_look windowLook;
	BStringList exclusionsSignatureList;
	bool subscribedToEngine, showDeskbarMenu;
};

struct ScaleSettings
{	int		launches_scale, first_launch_scale, last_launch_scale,
			interval_scale, total_run_time_scale;

};

AppSettings* GetAppSettings();

#endif
