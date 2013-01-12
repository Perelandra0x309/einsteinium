/* LauncherApp.cpp
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherApp.h"

LauncherApp::LauncherApp()
	:
	BApplication(e_launcher_sig),
	EngineSubscriber(),
	fSettingsFile(NULL),
	fQuitRequested(false)
{
	// Get saved settings
	BPath settingsPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append(e_settings_dir);
	settingsPath.Append(el_settings_file);

	AppSettings settings;
	ScaleSettings scaleSettings;
	BMessage appExclusions;
	BRect mainWindowRect;
	status_t frameResult = B_ERROR;
	fSettingsFile = new LauncherSettingsFile(this);
	status_t result = fSettingsFile->CheckStatus();
	if(result==B_OK)
	{
		// Layout settings
		settings.appCount = fSettingsFile->GetDeskbarCount();
		settings.minIconSize = fSettingsFile->GetMinAppIconSize();
		settings.maxIconSize = fSettingsFile->GetMaxAppIconSize();
		settings.docIconSize = fSettingsFile->GetDocIconSize();
		settings.recentDocCount = fSettingsFile->GetDocCount();
		settings.recentFolderCount = fSettingsFile->GetFolderCount();
		settings.recentQueryCount = fSettingsFile->GetQueryCount();
		settings.fontSize = fSettingsFile->GetFontSize();
		settings.windowLook = fSettingsFile->GetWindowLook();
		mainWindowRect = fSettingsFile->GetWindowFrame();
		if(mainWindowRect.IsValid() && mainWindowRect.Width()!=0 && mainWindowRect.Height()!=0)
			frameResult=B_OK;
		else
			mainWindowRect.Set(0,0,200,400);

		// App rank scales settings
		fSettingsFile->GetScales(&scaleSettings);

		// App exclusion settings
		appExclusions = fSettingsFile->GetExclusionsList();
	}

	// Create Windows
	fSettings = new SettingsWindow(&settings, &scaleSettings, &appExclusions);
	settings = fSettings->GetAppSettings();
	fWindow = new MainWindow(mainWindowRect, settings);
	// Set feel outside of constructor- setting a feel of B_MODAL_ALL_WINDOW_FEEL
	// inside constructor causes CTRL-Q and CTRL-W to not work.
//	fWindow->SetFeel(B_MODAL_ALL_WINDOW_FEEL);
	if(frameResult != B_OK)
		fWindow->CenterOnScreen();
	fWindow->Show();
}


bool
LauncherApp::QuitRequested()
{
	// Unsubscribe from the Einsteinium Engine
	_UnsubscribeFromEngine();

//	_SaveSettingsToFile();
	delete fSettingsFile;
	fSettingsFile = NULL;

//	_ShowShelfView(false);



	return true;//BApplication::QuitRequested();
}

void
LauncherApp::ReadyToRun()
{
	// Add the shelf view
//	_ShowShelfView(!fQuitRequested);

	//Quit since the deskbar now controls the ELShelfView object
//	be_app->PostMessage(B_QUIT_REQUESTED);

	//Initialize the settings file object and check to see if it instatiated correctly
/*	fSettingsFile = new LauncherSettingsFile(this);
	status_t result = fSettingsFile->CheckStatus();
	if(result!=B_OK){
		printf("Error creating Einsteinium Launcher settings file.  Cannot continue.\n");
//		_Quit();
			// TODO Can we do anything w/o settings?
		return;
	}*/

	_Subscribe();
//	be_app->PostMessage(EL_UPDATE_RECENT_LISTS);
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
LauncherApp::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
/*		case LINES_OPTION_CHANGED: {
			AppSettings settings = fSettings->GetSettings();
			bool twoLines = fSettings->GetDrawTwoLines();
			msg->AddBool(NAME_LINES_OPTION, twoLines);
			fWindow->SettingsChanged(settings, msg);
			break;
		}*/
		case EL_APP_COUNT_OPTION_CHANGED:
		{
			AppSettings settings = fSettings->GetAppSettings();
			fWindow->SettingsChanged(msg->what, settings);
			_SaveSettingsToFile(msg->what, settings);
			_Subscribe();
			break;
		}
		case EL_APP_ICON_OPTION_CHANGED:
		case EL_DOC_ICON_OPTION_CHANGED:
		case EL_DOC_COUNT_OPTION_CHANGED:
		case EL_FOLDER_COUNT_OPTION_CHANGED:
		case EL_QUERY_COUNT_OPTION_CHANGED:
		case EL_FONT_OPTION_CHANGED: {
			AppSettings settings = fSettings->GetAppSettings();
			fWindow->SettingsChanged(msg->what, settings);
			_SaveSettingsToFile(msg->what, settings);
			break;
		}
		case EL_LOOK_OPTION_CHANGED: {
		//	window_look look = fSettings->GetWindowLook();
			AppSettings settings = fSettings->GetAppSettings();
			fWindow->Lock();
			fWindow->SetLook(settings.windowLook);
			fWindow->Unlock();
			_SaveSettingsToFile(EL_LOOK_OPTION_CHANGED, settings);
			break;
		}
	/*	case FLOAT_OPTION_CHANGED: {
			window_feel feel = fSettings->GetFloat();
			fWindow->Lock();
			fWindow->SetFeel(feel);
			fWindow->Unlock();
			_SaveSettingsToFile(FLOAT_OPTION_CHANGED);
			break;
		}*/
		case EL_SAVE_RANKING:
		{
			// Save to file
			ScaleSettings newSettings = fSettings->GetScaleSettings();
			fSettingsFile->SaveScales(newSettings);
			// Resubscribe with new settings
			_Subscribe();
			break;
		}
		case EL_EXCLUSIONS_CHANGED:
		{
			// Save to file
			BMessage newExclusionsList = fSettings->GetAppExclusions();
			fSettingsFile->SaveExclusionsList(newExclusionsList);
			// Resubscribe with new settings
			_Subscribe();
			break;
		}
		case EL_SHOW_SETTINGS:
		{
			if(fSettings->IsHidden())
				fSettings->Show(fWindow->Frame());
			fSettings->Activate();
			break;
		}
		case EL_SHOW_WINDOW:
		{
			fWindow->SelectDefaultTab();
			if(fWindow->IsHidden())
			{
				fWindow->Show();
				fWindow->UpdateIfNeeded();
			}
			else
				be_roster->ActivateApp(Team());

			be_app->PostMessage(EL_UPDATE_RECENT_LISTS);
			break;
		}
		case EL_WINDOW_MOVED:
		{
			fSettingsFile->SaveWindowFrame(fWindow->Frame());
			break;
		}
		case EL_HIDE_APP:
		{
			if(!fSettings->IsHidden())
				fSettings->Hide();
			fWindow->Hide();
			break;
		}
		case EL_UPDATE_RECENT_LISTS:
		case EL_UPDATE_RECENT_LISTS_FORCE:
		case EL_UPDATE_INFOVIEW:
		{
			fWindow->PostMessage(msg);
			break;
		}
		// Settings file was updated by an external application
		case EL_SETTINGS_FILE_CHANGED_EXTERNALLY:
		{
			// TODO update settings window

			_Subscribe();
			fWindow->PostMessage(EL_UPDATE_RECENT_LISTS_FORCE);
			break;
		}
		case EL_ADD_APP_EXCLUSION:
		{
			// Add app to exclusion list and resubscribe
			fSettingsFile->AddToExclusionsList(msg);
			BMessage exclusionsList = fSettingsFile->GetExclusionsList();
			fSettings->SetAppExclusions(&exclusionsList);
			_Subscribe();
			break;
		}
		default:
			BApplication::MessageReceived(msg);
			break;
	}
}

/*
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
}*/


void
LauncherApp::_SaveSettingsToFile(uint32 what, AppSettings settings)
{
	switch(what)
	{
		case EL_APP_COUNT_OPTION_CHANGED:
		{
			fSettingsFile->SaveDeskbarCount(settings.appCount);
			break;
		}
		case EL_APP_ICON_OPTION_CHANGED:
		{
			fSettingsFile->SaveAppIconSize(settings.minIconSize, settings.maxIconSize);
			break;
		}
		case EL_DOC_ICON_OPTION_CHANGED:
		{
			fSettingsFile->SaveDocIconSize(settings.docIconSize);
			break;
		}
		case EL_DOC_COUNT_OPTION_CHANGED:
		{
			fSettingsFile->SaveDocCount(settings.recentDocCount);
			break;
		}
		case EL_FOLDER_COUNT_OPTION_CHANGED:
		{
			fSettingsFile->SaveFolderCount(settings.recentFolderCount);
			break;
		}
		case EL_QUERY_COUNT_OPTION_CHANGED:
		{
			fSettingsFile->SaveQueryCount(settings.recentQueryCount);
			break;
		}
		case EL_FONT_OPTION_CHANGED:
		{
			fSettingsFile->SaveFontSize(settings.fontSize);
			break;
		}
		case EL_LOOK_OPTION_CHANGED:
		{
			fSettingsFile->SaveWindowLook(settings.windowLook);
			break;
		}
	/*	case FLOAT_OPTION_CHANGED:
		{

			break;
		}*/
	}
}


void
LauncherApp::_Subscribe()
{
	// Get values from the settings file
	// TODO change to get from settings window
	int itemCount = fSettingsFile->GetDeskbarCount();
	const int *scales = fSettingsFile->GetScales();
	BMessage exclusionsList = fSettingsFile->GetExclusionsList();

	// Subscribe to the Einsteinium Engine.  If already subscribed, this will update
	// the values used for subscription messages
	_ResetSubscriberValues();
	_SetCount(itemCount);
	_SetTotalLaunchesScale(scales[LAUNCH_INDEX]);
	_SetFirstLaunchScale(scales[FIRST_INDEX]);
	_SetLastLaunchScale(scales[LAST_INDEX]);
	_SetLastIntervalScale(scales[INTERVAL_INDEX]);
	_SetTotalRuntimeScale(scales[RUNTIME_INDEX]);
	_SetExcludeList(&exclusionsList);
	_SubscribeToEngine();
}


void
LauncherApp::_SubscribeFailed()
{
	// Show error message
	BAlert *subscriptioneAlert = new BAlert("Subscription",
			"The Einsteinium Launcher sent an invalid subscription to the Engine.  Please check your Launcher settings.",
			"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
	// Set the feel to FLOATING so that the alert does not block use of the Deskbar while showing
	subscriptioneAlert->SetFeel(B_FLOATING_ALL_WINDOW_FEEL);
	subscriptioneAlert->Go(NULL);
}


void
LauncherApp::_SubscribeConfirmed()
{
	// Don't need to do anything- just wait for an updated subscription message
//	printf("Subscription confirmed.\n");

}


void
LauncherApp::_UpdateReceived(BMessage *message)
{
	// Rebuild the application selection view with the list received from the Engine
//	printf("Received update from Engine.\n");
	fWindow->BuildAppsListView(message);
}
