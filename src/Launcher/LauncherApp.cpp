/* LauncherApp.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherApp.h"


/*bool IsEngineRunning(){
	return ((LauncherApp *)be_app)->GetIsEngineRunning();
};*/

AppSettings* GetAppSettings()
{
	return ((LauncherApp *)be_app)->GetAppSettings();
};

LauncherApp::LauncherApp()
	:
	BApplication(e_launcher_sig),
	EngineSubscriber(),
	fSettingsFile(NULL),
	fQuitRequested(false)
{
	fAppSettings.subscribedToEngine=false;
	// Get saved settings
	BPath settingsPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append(e_settings_dir);
	settingsPath.Append(el_settings_file);

	ScaleSettings scaleSettings;
	BMessage appExclusions;
	BRect mainWindowRect;
	status_t frameResult = B_ERROR;
	fSettingsFile = new LauncherSettingsFile(this);
	status_t result = fSettingsFile->CheckStatus();
	if(result==B_OK)
	{
		// Layout settings
		fAppSettings.appCount = fSettingsFile->GetDeskbarCount();
		fAppSettings.minIconSize = fSettingsFile->GetMinAppIconSize();
		fAppSettings.maxIconSize = fSettingsFile->GetMaxAppIconSize();
		fAppSettings.docIconSize = fSettingsFile->GetDocIconSize();
		fAppSettings.recentDocCount = fSettingsFile->GetDocCount();
		fAppSettings.recentFolderCount = fSettingsFile->GetFolderCount();
		fAppSettings.recentQueryCount = fSettingsFile->GetQueryCount();
		fAppSettings.fontSize = fSettingsFile->GetFontSize();
		fAppSettings.windowLook = fSettingsFile->GetWindowLook();
		mainWindowRect = fSettingsFile->GetWindowFrame();
	//	if(mainWindowRect.IsValid() && mainWindowRect.Width()!=0 && mainWindowRect.Height()!=0)
			frameResult=B_OK;
	//	else
	//		mainWindowRect.Set(0,0,200,400);

		// App rank scales settings
		fSettingsFile->GetScales(&scaleSettings);

		// App exclusion settings
		appExclusions = fSettingsFile->GetExclusionsList();
		_CreateExclusionsSignatureList(&appExclusions);

		//Deskbar menu
		fAppSettings.showDeskbarMenu = fSettingsFile->GetShowDeskbarMenu();
	}

	// Create Windows
	fSettingsWindow = new SettingsWindow(&fAppSettings, &scaleSettings, &appExclusions);
	fWindow = new MainWindow(mainWindowRect, fAppSettings.windowLook);
	// Set feel outside of constructor- setting a feel of B_MODAL_ALL_WINDOW_FEEL
	// inside constructor causes CTRL-Q and CTRL-W to not work.
//	fWindow->SetFeel(B_FLOATING_ALL_WINDOW_FEEL);
	if(frameResult != B_OK)
		fWindow->CenterOnScreen();
}


bool
LauncherApp::QuitRequested()
{
	fSettingsFile->SaveWindowFrame(fWindow->Frame());
	// Unsubscribe from the Einsteinium Engine
	_UnsubscribeFromEngine();
	delete fSettingsFile;
	fSettingsFile = NULL;

	return true;
}

void
LauncherApp::ReadyToRun()
{
	//Were we sent a quit arguement?
	if(fQuitRequested)
	{
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}

	if(_IsEngineRunning())
		_Subscribe();
	else
	{
		// Force an update to the Apps list
		fWindow->SelectDefaultTab();

/*		BNotification notify(B_INFORMATION_NOTIFICATION);
		notify.SetGroup("Einsteinium Launcher");
//		notify.SetTitle("Einsteinium Engine not running");
		notify.SetContent("The Apps list will be populated from Haiku's recent applications list.  Please start the Engine if you wish to use the Engine's app statitistics.");
		notify.Send(10000000);*/
		BString alertS("The Einsteinium Engine is not running.  Without the Engine the Launcher's Apps list will be populated from Haiku's recent applications list.  Do you wish to start the Engine now?");
		BAlert *engineAlert = new BAlert("StartEngine",alertS.String(), "No",
					"Yes", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		engineAlert->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
		BMessage *runMessage = new BMessage(EL_START_ENGINE_ALERT);
		engineAlert->Go(new BInvoker(runMessage, this));

	}
	fWindow->Show();

	// Create shelf view
	_ShowShelfView(fAppSettings.showDeskbarMenu);
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
		{	fQuitRequested=true;
		//	be_app->PostMessage(B_QUIT_REQUESTED);
		}
		//only create the deskbar shelfview according to settings, then quit
		else if(strcmp(argv[1],"--prepdeskbar")==0)
		{
			_ShowShelfView(fAppSettings.showDeskbarMenu);
			fQuitRequested=true;
		}
		//option is not recognized
		else
		{	printf("Unknown option.\n"); }
	}
}


/*void
LauncherApp::AppActivated(bool active)
{
	if(active)
	{
		be_app->PostMessage(EL_SHOW_WINDOW);
	}
}*/


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
			fSettingsWindow->PopulateAppSettings(&fAppSettings);
			fWindow->SettingsChanged(msg->what);
			_SaveSettingsToFile(msg->what);
			_Subscribe();
			break;
		}
		case EL_APP_ICON_OPTION_CHANGED:
		case EL_DOC_ICON_OPTION_CHANGED:
		case EL_DOC_COUNT_OPTION_CHANGED:
		case EL_FOLDER_COUNT_OPTION_CHANGED:
		case EL_QUERY_COUNT_OPTION_CHANGED:
		case EL_FONT_OPTION_CHANGED: {
			fSettingsWindow->PopulateAppSettings(&fAppSettings);
			fWindow->SettingsChanged(msg->what);
			_SaveSettingsToFile(msg->what);
			break;
		}
		case EL_LOOK_OPTION_CHANGED: {
			fSettingsWindow->PopulateAppSettings(&fAppSettings);
			fWindow->Lock();
			fWindow->SetLook(fAppSettings.windowLook);
			fWindow->Unlock();
			_SaveSettingsToFile(EL_LOOK_OPTION_CHANGED);
			break;
		}
		case EL_DESKBAR_OPTION_CHANGED: {
			fSettingsWindow->PopulateAppSettings(&fAppSettings);
			_SaveSettingsToFile(msg->what);
			_ShowShelfView(fAppSettings.showDeskbarMenu);
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
			ScaleSettings newSettings = fSettingsWindow->GetScaleSettings();
			fSettingsFile->SaveScales(newSettings);
			// Resubscribe with new settings
			_Subscribe();
			break;
		}
		case EL_EXCLUSIONS_CHANGED:
		{
			// Save to file
			BMessage newExclusionsList = fSettingsWindow->GetAppExclusions();
			fSettingsFile->SaveExclusionsList(newExclusionsList);
			// Update the apps list from recent items
			_CreateExclusionsSignatureList(&newExclusionsList);
			fWindow->PostMessage(EL_EXCLUSIONS_CHANGED);
			// Resubscribe with new settings
			_Subscribe();
			break;
		}
		case EL_SHOW_SETTINGS:
		{
			if(fSettingsWindow->IsHidden())
				fSettingsWindow->Show(fWindow->Frame());
			fSettingsWindow->Activate();
			break;
		}
		case EL_SHOW_WINDOW:
		{
			//fWindow->SelectDefaultTab();
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
/*		case EL_WINDOW_MOVED:
		{
			fSettingsFile->SaveWindowFrame(fWindow->Frame());
			break;
		}*/
		case EL_HIDE_APP:
		{
			if(!fSettingsWindow->IsHidden())
				fSettingsWindow->Hide();
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
			fSettingsWindow->SetAppExclusions(&exclusionsList);
			_CreateExclusionsSignatureList(&exclusionsList);
			fWindow->PostMessage(EL_EXCLUSIONS_CHANGED);
			_Subscribe();
			break;
		}
		case EL_START_ENGINE_ALERT:
		{
			int32 selection;
			msg->FindInt32("which", &selection);
			// If the Yes button was selected, launch the Engine
			if(selection){
				status_t result = _LaunchEngine();
				if(result==B_OK)
					_Subscribe();
			}
			break;
		}
		default:
			BApplication::MessageReceived(msg);
			break;
	}
}


void
LauncherApp::_CreateExclusionsSignatureList(BMessage *exclusions)
{
	fAppSettings.exclusionsSignatureList.MakeEmpty();
	type_code typeFound;
	int32 signatureCount = 0;
	status_t result = exclusions->GetInfo(EL_EXCLUDE_SIGNATURE, &typeFound, &signatureCount);
	BString sig;
	for(int i=0; i<signatureCount; i++)
	{
		sig.SetTo("");
		exclusions->FindString(EL_EXCLUDE_SIGNATURE, i, &sig);
		if(sig.Length()>0)
			fAppSettings.exclusionsSignatureList.Add(sig);
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


void
LauncherApp::_SaveSettingsToFile(uint32 what)
{
	switch(what)
	{
		case EL_APP_COUNT_OPTION_CHANGED:
		{
			fSettingsFile->SaveDeskbarCount(fAppSettings.appCount);
			break;
		}
		case EL_APP_ICON_OPTION_CHANGED:
		{
			fSettingsFile->SaveAppIconSize(fAppSettings.minIconSize, fAppSettings.maxIconSize);
			break;
		}
		case EL_DOC_ICON_OPTION_CHANGED:
		{
			fSettingsFile->SaveDocIconSize(fAppSettings.docIconSize);
			break;
		}
		case EL_DOC_COUNT_OPTION_CHANGED:
		{
			fSettingsFile->SaveDocCount(fAppSettings.recentDocCount);
			break;
		}
		case EL_FOLDER_COUNT_OPTION_CHANGED:
		{
			fSettingsFile->SaveFolderCount(fAppSettings.recentFolderCount);
			break;
		}
		case EL_QUERY_COUNT_OPTION_CHANGED:
		{
			fSettingsFile->SaveQueryCount(fAppSettings.recentQueryCount);
			break;
		}
		case EL_FONT_OPTION_CHANGED:
		{
			fSettingsFile->SaveFontSize(fAppSettings.fontSize);
			break;
		}
		case EL_LOOK_OPTION_CHANGED:
		{
			fSettingsFile->SaveWindowLook(fAppSettings.windowLook);
			break;
		}
		case EL_DESKBAR_OPTION_CHANGED:
		{
			fSettingsFile->SaveShowDeskbarMenu(fAppSettings.showDeskbarMenu);
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
	// No point in trying to subscribe if Engine is not running
	if(!_IsEngineRunning())
	{
		fAppSettings.subscribedToEngine=false;
		return;
	}

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
	fAppSettings.subscribedToEngine=false;
	// Show error message
	BAlert *subscriptioneAlert = new BAlert("Subscription",
			"The Einsteinium Launcher sent an invalid subscription to the Engine.  Please check your Launcher settings.",
			"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
	// Set the feel to FLOATING so that the alert does not block use of the Deskbar while showing
	subscriptioneAlert->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
	subscriptioneAlert->Go(NULL);
}


void
LauncherApp::_SubscribeConfirmed()
{
	fAppSettings.subscribedToEngine=true;
//	printf("Subscription confirmed.\n");

}


void
LauncherApp::_UpdateReceived(BMessage *message)
{
	// Rebuild the application selection view with the list received from the Engine
//	printf("Received update from Engine.\n");
	fWindow->BuildAppsListView(message);
}
