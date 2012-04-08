/* prefs_app.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "prefs_app.h"

prefs_app::prefs_app()
	:
	BApplication(e_preferences_sig)
{
	fMainWindow = new prefsWindow(BRect(200,50,800,500));
}


prefs_app::~prefs_app()
{	}


/*
void
prefs_app::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{	case EL_GOTO_LAUNCHER_SETTINGS: {
			fMainWindow->MessageReceived(msg);
			// Bring the window to the front
			app_info appInfo;
			GetAppInfo(&appInfo);
			be_roster->ActivateApp(appInfo.team);
			break;
		}
	}
	BApplication::MessageReceived(msg);
}*/
