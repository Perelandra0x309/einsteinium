/*einsteinium_daemon.cpp
	Functions for the main application.
*/
#include "einsteinium_daemon.h"


einsteinium_daemon::einsteinium_daemon()
	:BApplication(e_daemon_sig)
	,watchingRoster(false)
{
	//Initialize the settings file object and check to see if it instatiated correctly
	settingsFile = new EDSettingsFile();
	status_t result = settingsFile->CheckInitStatus();
	if(result!=B_OK){
		printf("Error creating Einsteinium Daemon settings file.  Cannot continue.\n");
		be_app->PostMessage(B_QUIT_REQUESTED);//Quit. Can we do anthything w/o settings?
	}
}


//Final cleanup (most done in QuitRequest())
/*einsteinium_daemon::~einsteinium_daemon()
{
}*/


//clean up, save settings
bool einsteinium_daemon::QuitRequested()
{
	//Need to stop watching the roster
	if(watchingRoster)
	{	be_roster->StopWatching(be_app_messenger); }

	delete settingsFile;

//	printf("Einsteinium daemon quitting.\n");
	return BApplication::QuitRequested();
}


//Run right after app is ready
void einsteinium_daemon::ReadyToRun()
{
	//Start watching the application roster for launches/quits/activations
	status_t result = be_roster->StartWatching(be_app_messenger,
				B_REQUEST_QUIT /*| B_REQUEST_LAUNCHED | B_REQUEST_ACTIVATED*/);
	if(result  != B_OK)
	{	//roster failed to be watched.  Should we continue or just quit???
		printf("Error initializing watching the roster.\n");
		PostMessage(B_QUIT_REQUESTED);
	}
	else//watching was sucessful
	{	watchingRoster = true;//set the flag to indicate we're watching the roster
//		printf("Einsteinium deamon running.\n");
	}
	return;
}


//Parses command line options. This runs both the first time the app runs with options,
//or anytime when the app is already running and is sent command line options
void einsteinium_daemon::ArgvReceived(int32 argc, char** argv)
{	if(argc>1)//options were given
	{	if(strcmp(argv[1],"-q")==0 || strcmp(argv[1],"--quit")==0)//option to quit
		{	PostMessage(B_QUIT_REQUESTED); }
		else//option is not recognized
		{	printf("Unknown option.\n"); }
	}
	return;
}


//Messages sent to the application
void einsteinium_daemon::MessageReceived(BMessage *msg)
{	switch(msg->what)//act according to the Message command
	{	case B_SOME_APP_QUIT: {//An application quit/crashed/was killed
			//Find the app signature
			char* sig;
			if (msg->FindString("be:signature", (const char**)&sig) != B_OK) break;
			AppRelaunchSettings *appSettings = settingsFile->FindSettingsForApp(sig);
			if(appSettings == NULL) appSettings = new AppRelaunchSettings(sig);
			int actionToTake = appSettings->relaunchAction;
		//	printf("Taking care of quit app %s with action %i...\n", sig, actionToTake);
			if(actionToTake == ACTION_IGNORE) break;
			// user wants to be prompted to restart app
			if(actionToTake == ACTION_PROMPT)
			{	BString alertS("The application '");
				alertS.Append(appSettings->name);
				alertS.Append("' has quit.  Do you wish to restart this app?");
				// Alert box to prompt for app restart.
				RememberChoiceAlert *alert = new RememberChoiceAlert("",alertS.String(), "No", "Yes");
				BMessage *runmsg = new BMessage(LAUNCH_APP);//message sent by alert
				runmsg->AddString("app_sig",sig);
				alert->Go(new BInvoker(runmsg, this));//invoke asynchronous alert
			}
			// launch app automatically
			else if(actionToTake == ACTION_AUTO)
			{	be_roster->Launch(sig); }
			break;
		}
		case LAUNCH_APP: {//message from app relaunch alert
			int32 selection;
			msg->FindInt32("which", &selection);//which button was pressed
			char *sig;
			msg->FindString("app_sig", (const char**)&sig);
			switch(selection)
			{	case 0: {//No, don't restart
					break; }
				case 1: {//Yes, restart
					be_roster->Launch(sig);//launch app by signature
					break; }
			}
			bool rememberChoice;
			status_t result = msg->FindBool(ED_ALERT_REMEMBER, &rememberChoice);
			if(result == B_OK && rememberChoice)
			{
			//	printf("ED: Found alert choice: %d\n", rememberChoice);
				settingsFile->UpdateActionForApp(sig,
						selection? ED_XMLTEXT_VALUE_AUTO : ED_XMLTEXT_VALUE_IGNORE);
			}
			break;
		}
	/*	case B_SOME_APP_LAUNCHED: {//An application launched

			break; }
		case B_SOME_APP_ACTIVATED: {//An application brought to the front??

			break; }*/
		default: BApplication::MessageReceived(msg);//The message may be for the application
	}
	return;
}
