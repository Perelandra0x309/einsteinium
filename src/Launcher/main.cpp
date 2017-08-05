/* main.cpp
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherApp.h"

int main()
{
	// Is the Launcher already running?
/*	bool multipleLaunch = false;
	BList teamsList;
	be_roster->GetAppList(e_launcher_sig, &teamsList);
	int32 count = teamsList.CountItems();
	if(count)
	{
		// Show the running Launcher
		printf("Found multiple Launchers running.\n");
//		team_id team = (team_id)(teamsList.ItemAt(0));
		BMessenger messenger(e_launcher_sig);
		if(messenger.IsValid())
		{
			messenger.SendMessage(EL_SHOW_WINDOW);
			multipleLaunch = true;
		}
	}
*/
//	if(!multipleLaunch)
//	{
		LauncherApp myApp;
		myApp.Run();
//	}
	return 0;
}
