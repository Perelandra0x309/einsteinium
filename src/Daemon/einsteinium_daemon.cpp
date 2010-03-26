/* einsteinium_daemon.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "einsteinium_daemon.h"


einsteinium_daemon::einsteinium_daemon()
	:
	BApplication(e_daemon_sig),
	fWatchingRoster(false)
{
	//Initialize the settings file object and check to see if it instatiated correctly
	fSettingsFile = new EDSettingsFile();
	status_t result = fSettingsFile->CheckInitStatus();
	if(result!=B_OK){
		printf("Error creating Einsteinium Daemon settings file.  Cannot continue.\n");
		be_app->PostMessage(B_QUIT_REQUESTED);//Quit. Can we do anthything w/o settings?
	}
}


//clean up, save settings
bool
einsteinium_daemon::QuitRequested()
{
	//Need to stop watching the roster
	if(fWatchingRoster)
	{	be_roster->StopWatching(be_app_messenger); }

	delete fSettingsFile;

//	printf("Einsteinium daemon quitting.\n");
	return BApplication::QuitRequested();
}


void
einsteinium_daemon::ReadyToRun()
{
	//Start watching the application roster for launches/quits/activations
	status_t result = be_roster->StartWatching(be_app_messenger,
				B_REQUEST_QUIT /*| B_REQUEST_LAUNCHED | B_REQUEST_ACTIVATED*/);
	if(result  != B_OK)
	{	//roster failed to be watched.  Can't do anything, so must quit.
		// TODO display Alert?
		printf("Error initializing watching the roster.\n");
		PostMessage(B_QUIT_REQUESTED);
	}
	else
	{	fWatchingRoster = true;
			//set the flag to indicate we're watching the roster
//		printf("Einsteinium deamon running.\n");
	}
}


//Parses command line options. This runs both the first time the app runs with options,
//or anytime when the app is already running and is sent command line options
void
einsteinium_daemon::ArgvReceived(int32 argc, char** argv)
{
	if(argc>1)
	{
		//option to quit
		if(strcmp(argv[1],"-q")==0 || strcmp(argv[1],"--quit")==0)
		{	PostMessage(B_QUIT_REQUESTED); }
		//option is not recognized
		else
		{	printf("Unknown option.\n"); }
	}
}


//Messages sent to the application
void
einsteinium_daemon::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		//An application quit/crashed/was killed
		case B_SOME_APP_QUIT: {
			//Find the app signature
			char* sig;
			status_t result = msg->FindString("be:signature", (const char**)&sig);
			if (result != B_OK)
				break;
			AppRelaunchSettings *appSettings = fSettingsFile->FindSettingsForApp(sig);
			if(appSettings == NULL)
				appSettings = new AppRelaunchSettings(sig);
			int actionToTake = appSettings->relaunchAction;
		//	printf("Taking care of quit app %s with action %i...\n", sig, actionToTake);
			if(actionToTake == ACTION_IGNORE)
				break;
			// user wants to be prompted to restart app
			if(actionToTake == ACTION_PROMPT)
			{	BString alertS("The application '");
				alertS.Append(appSettings->name);
				alertS.Append("' has quit.  Do you wish to restart this app?");
				RememberChoiceAlert *alert = new RememberChoiceAlert("",alertS.String(), "No", "Yes");
					// Custom Alert window to prompt for app restart.
				BMessage *runMessage = new BMessage(LAUNCH_APP);
				runMessage->AddString("app_sig",sig);
				alert->Go(new BInvoker(runMessage, this));
			}
			// launch app automatically
			else if(actionToTake == ACTION_AUTO)
			{	be_roster->Launch(sig); }
			break;
		}
		//message from RememberChoiceAlert window
		case LAUNCH_APP: {
			int32 selection;
			msg->FindInt32("which", &selection);
				//find which button was pressed
			char *sig;
			msg->FindString("app_sig", (const char**)&sig);
			switch(selection)
			{	//No, don't restart
				case 0:
					break;
				//Yes, restart
				case 1:
					be_roster->Launch(sig);
					break;
			}
			bool rememberChoice;
			status_t result = msg->FindBool(ED_ALERT_REMEMBER, &rememberChoice);
			if(result == B_OK && rememberChoice)
			{
			//	printf("ED: Found alert choice: %d\n", rememberChoice);
				fSettingsFile->UpdateActionForApp(sig,
						selection? ED_XMLTEXT_VALUE_AUTO : ED_XMLTEXT_VALUE_IGNORE);
			}
			break;
		}
	/*	case B_SOME_APP_LAUNCHED: {//An application launched

			break; }
		case B_SOME_APP_ACTIVATED: {//An application brought to the front??

			break; }*/
		default:
			BApplication::MessageReceived(msg);//The message may be for the application
	}
}
