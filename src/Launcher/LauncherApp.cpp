/* LauncherApp.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherApp.h"

LauncherApp::LauncherApp()
	:
	BApplication(e_launcher_sig),
	fShelfViewId(0)
{

}


void
LauncherApp::ReadyToRun()
{
	// TODO check if there is anlready an instance in the deskbar
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
	if(fShelfViewId)// shelf view is currently showing
	{
		deskbar.RemoveItem(fShelfViewId);
		fShelfViewId = 0;
	}
	if(showShelfView)
	{
		// TODO detect if there is already an instance running in the deskbar
		BView *shelfView = new ELShelfView();
		deskbar.AddItem(shelfView, &fShelfViewId);
		delete shelfView;
	}
}
