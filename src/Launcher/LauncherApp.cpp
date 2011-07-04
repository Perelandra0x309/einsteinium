/* LauncherApp.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherApp.h"

LauncherApp::LauncherApp()
	:
	BApplication(e_launcher_sig),
	fQuitRequested(false)
{

}


void
LauncherApp::ReadyToRun()
{
	// Add the shelf view
	_ShowShelfView(!fQuitRequested);

	//Quit since the deskbar now controls the ELShelfView object
	be_app->PostMessage(B_QUIT_REQUESTED);
}


//Parses command line options. This runs both the first time the app runs with options,
//or anytime when the app is already running and is sent command line options
void
LauncherApp::ArgvReceived(int32 argc, char** argv)
{
	if(argc>1)
	{
		//option to quit
		if(strcmp(argv[1],"-q")==0 || strcmp(argv[1],"--quit")==0)
		{	fQuitRequested=true; }
		//option is not recognized
		else
		{	printf("Unknown option.\n"); }
	}
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
	// Remove ELShelfView if there is one in the deskbar
	else if(!showShelfView && deskbar.HasItem(EL_SHELFVIEW_NAME))
	{
		deskbar.RemoveItem(EL_SHELFVIEW_NAME);
	}
}
