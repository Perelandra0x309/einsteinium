/* LauncherApp.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherApp.h"

LauncherApp::LauncherApp()
	:
	BApplication(e_launcher_sig)
{

}


void
LauncherApp::ReadyToRun()
{
	// Add the shelf view
	_ShowShelfView(true);

	//Quit since the deskbar now controls the ELShelfView object
	printf("Quitting LauncherApp\n");
	be_app->PostMessage(B_QUIT_REQUESTED);
}


void
LauncherApp::_ShowShelfView(bool showShelfView)
{
	BDeskbar deskbar;
	// Don't add another ELShelfView to the Deskbar if one is already attached
	if(showShelfView && !deskbar.HasItem(EL_SHELFVIEW_NAME))
	{
		BView *shelfView = new ELShelfView();
		deskbar.AddItem(shelfView);
		delete shelfView;
	}
}
