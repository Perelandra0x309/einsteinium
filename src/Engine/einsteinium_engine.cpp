/*einsteinium_engine.cpp
	Functions for the main application.
*/
#include "einsteinium_engine.h"


einsteinium_engine::einsteinium_engine()//Einsteinium Engine Constructor
	:BApplication(e_engine_sig)//Initialize application
	,watchingRoster(false)//initially roster is not being watched
	,quartileRunner(NULL)
	,ee_session(time(NULL))
	,shelfViewId(0)
{
//	printf("ee_session=%i\n", ee_session);
	//Create settings directories if they are not found
	BDirectory settingsDir;
	//find ~/config/settings/Einsteinium directory
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsDirPath);
	settingsDirPath.Append(e_settings_dir);
	//create path for main directory
	if( settingsDir.SetTo(settingsDirPath.Path()) == B_ENTRY_NOT_FOUND )//if it doesn't exist
	{	printf("Settings directory not found, creating directory\n    %s\n",
					settingsDirPath.Path());
		//Create directory
		if(settingsDir.CreateDirectory(settingsDirPath.Path(), &settingsDir)!=B_OK)
		{	//Creating directory failed
			printf("Error creating Einsteinium settings folder.  Cannot continue.\n");
			be_app->PostMessage(B_QUIT_REQUESTED);//Quit. Can we do anthything w/o settings?
			return;//Cannot continue, exit constructor
		}
	}
	//find ~/config/settings/Einsteinium/Applications directory
	BPath appSettingsPath(settingsDirPath);
	appSettingsPath.Append(e_settings_app_dir);
	if( settingsDir.SetTo(appSettingsPath.Path()) == B_ENTRY_NOT_FOUND )//if it doesn't exist
	{	printf("Settings directory not found, creating directory\n    %s\n",
					appSettingsPath.Path());
		//Create subdirectory
		if(settingsDir.CreateDirectory(appSettingsPath.Path(), &settingsDir)!=B_OK)
		{	//Creating directory failed
			printf("Error creating %s settings folder.  "
					"Cannot continue.\n", appSettingsPath.Path());
			be_app->PostMessage(B_QUIT_REQUESTED);//Quit
			return;
		}
	}

	//Initialize the settings file object and check to see if it instatiated correctly
	settingsFile = new EESettingsFile();
	status_t result = settingsFile->CheckStatus();
	if(result!=B_OK){
		printf("Error creating Einsteinium Daemon settings file.  Cannot continue.\n");
		be_app->PostMessage(B_QUIT_REQUESTED);//Quit. Can we do anthything w/o settings?
	}
}

/*
einsteinium_engine::~einsteinium_engine()//Final cleanup (most stuff done in QuitRequested())
{	}*/


bool einsteinium_engine::QuitRequested()//clean up
{
	//Need to stop watching the roster
	if(watchingRoster)
	{	be_roster->StopWatching(be_app_messenger); }

	//disable timer
	if(quartileRunner != NULL) { delete quartileRunner; }

	// TODO wait for all messages to clear the message queue

	// Delete subscribers list items
	Subscriber *sub;
	deleteList(subscribersList, sub);
	subscribersList.MakeEmpty();

	// Remove shelf view
	if(shelfViewId)
	{
		BDeskbar deskbar;
		deskbar.RemoveItem(shelfViewId);
	}

	printf("Einsteinium engine quitting.\n");
	return BApplication::QuitRequested();
}


void einsteinium_engine::ReadyToRun()//Run right after app is ready
{	//Start watching the application roster for launches/quits/activations
	if (be_roster->StartWatching(be_app_messenger,
				B_REQUEST_QUIT | B_REQUEST_LAUNCHED/* | B_REQUEST_ACTIVATED*/) != B_OK)
	{	//roster failed to be watched.  Should we continue or just quit???
		printf("Error initializing watching the roster.\n");
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
	else//watching was sucessful
	{	watchingRoster = true;//set the flag to indicate we're watching the roster
		printf("Einsteinium engine running.\n");
	}
	//update quartiles, set up timer to update quartiles regularly
	updateQuartiles();
	quartileRunner = new BMessageRunner(this, new BMessage(E_UPDATE_QUARTILES), 3600*10e6, -1);
	if(quartileRunner->InitCheck()!=B_OK)
	{	delete quartileRunner;
		quartileRunner = NULL;
	}

	// Add the shelf view
	BDeskbar deskbar;
	shelfView = new EEShelfView(BRect(0, 0, 15, 15));
	deskbar.AddItem(shelfView, &shelfViewId);
//	delete shelfView;
}


//Parses command line options. This runs both the first time the app runs with options,
//or anytime when the app is already running and is sent command line options
void einsteinium_engine::ArgvReceived(int32 argc, char** argv)
{	if(argc>1)//options were given
	{	if(strcmp(argv[1],"-q")==0 || strcmp(argv[1],"--quit")==0)//option to quit
		{	PostMessage(B_QUIT_REQUESTED); }
		else if(strcmp(argv[1], "--rank")==0)//request list of apps printed to terminal
		{	PostMessage(E_PRINT_RANKING_APPS); }
		else if(strcmp(argv[1], "--recent")==0)//request list of apps printed to terminal
		{	PostMessage(E_PRINT_RECENT_APPS); }
		else if(strcmp(argv[1], "--updateQuart")==0)//
		{	PostMessage(E_UPDATE_QUARTILES); }
		else if(strcmp(argv[1], "--rescanData")==0)//
		{	PostMessage(E_RESCAN_DATA_FILES); }
		else if(strcmp(argv[1], "--updateScores")==0)//
		{	PostMessage(E_UPDATE_SCORES); }
		else//option is not recognized
		{	printf("Usage: einsteinium_engine [option]\n"
					"Options:/n"
					"-q or --quit		Quit/n"
					"--rank				List apps by overall rank/n"
					"--recent			List most recent apps/n"
					"--updateScores		Recalculate app scores/n"
					"--updateQuart		Recalculate quartiles, update scores/n"
					"--rescanData		Rescan database files, update quartiles & scores/n"); }
	}
}

// TODO monitor for SYSTEM_SHUTDOWN
//Messages sent to the application
void einsteinium_engine::MessageReceived(BMessage *msg)
{	switch(msg->what)//act according to the Message command
	{	case B_SOME_APP_QUIT://An application quit
		case B_SOME_APP_LAUNCHED: {//An application launched
			char* sig;//Find signature
			if (msg->FindString("be:signature", (const char**)&sig) == B_OK)//sig found
			{	//get entry_ref for app from Message
				entry_ref appRef;
				BEntry appEntry;
				if (msg->FindRef("be:ref", &appRef) != B_OK)//entry_ref not found in Message
				{	printf("App entry not found in message.  Attempting to query app.\n");
					//Query app by signature
					appEntry = getEntryFromSig(sig);
				}
				else { appEntry.SetTo(&appRef); }//entry_ref found in Message
				if(!appEntry.Exists())//Entry doesn't exist.  Shouldn't happen, but just in case.
				{	printf("Entry for app %s could not be found.\n", sig);
					break;//Can't do anything, so exit switch
				}
				AppAttrFile appFile(sig, &appEntry);//Open attribute file
				switch(msg->what)
				{	case B_SOME_APP_LAUNCHED: {
						appFile.UpdateAppLaunched();//Update object's data (times, score)
						break;
					}
					case B_SOME_APP_QUIT: {
						appFile.UpdateAppQuit();
						break;
					}
				}
				appFile.Close();//Write attributes to file

			}
			else//no signature found
			{	printf("Application has no signature.\n"); }
			break;
		}
	/*	case B_SOME_APP_ACTIVATED: {//An application brought to the front??

			break; }*/
		case E_SUBSCRIBE_RANKED_APPS: {
			printf("Received subscription message\n");
			Subscriber* newSubscriber = new Subscriber();
			status_t result = msg->FindMessenger("messenger", &(newSubscriber->messenger));
			if(result != B_OK)
			{
				printf("Einsteinium Engine received invalid subscriber messenger.\n");
				delete newSubscriber;
				break;
			}
			int32 count;
			result = msg->FindInt32("count", &(newSubscriber->count));
			if(result != B_OK)
				count = 10;  //set a default value
			subscribersList.AddItem(newSubscriber);

			// Test message
			BMessage test(E_SUBSCRIBER_UPDATE_RANKED_APPS);
			entry_ref ref;
			be_roster->FindApp("application/x-vnd.Einsteinium_Preferences", &ref);
			test.AddRef("refs", &ref);
			be_roster->FindApp(e_engine_sig, &ref);
			test.AddRef("refs", &ref);
			newSubscriber->messenger.SendMessage(&test);

			break;
		}
		case E_PRINT_RECENT_APPS:
		case E_PRINT_RANKING_APPS: {//print the recent list to the terminal
		//	printf("Constructing Recent Apps List:\n");
			//create list of eligible apps
			createAppList();
			switch(msg->what){
				case E_PRINT_RECENT_APPS: {//sort apps by most recent
					printf("Sorting list by most recent...\n");
					appsList.SortItems(AppStatsSortLastLaunch);
					break; }
				case E_PRINT_RANKING_APPS: {//sort apps by score
					printf("Sorting list by score...\n");
					appsList.SortItems(AppStatsSortScore);
					break; }
			}
			//print results
			printf("Ordered list of apps:\n");
			AppStats *stats;
			int32 count = appsList.CountItems();
			for(int i=0; i<count; i++)//print info for each AppAttrFile in the list
			{	stats = (AppStats*)(appsList.ItemAt(i));
				printf("%i: %s (%s)\n", i+1, stats->getSig(), stats->getFilename() );
			}
			deleteList(appsList, stats);
			break; }
		case E_REPLY_RECENT_APPS:
		case E_REPLY_RANKING_APPS: {//Send a reply back to another app with data from the recent apps list
			int recent_count = msg->FindInt8("count");//number of apps requested to return
			//create list of eligible apps
			createAppList();//created list
			BMessage replymsg;//Create reply BMessage
			switch(msg->what)
			{	case E_REPLY_RECENT_APPS: {//sort apps by recent
					printf("Sorting list by most recent...\n");
					appsList.SortItems(AppStatsSortLastLaunch);
					replymsg.what = E_RECENT_APPS_REPLY;
					break; }
				case E_REPLY_RANKING_APPS: {//sort apps by score
					printf("Sorting list by score...\n");
					appsList.SortItems(AppStatsSortScore);
					replymsg.what = E_RANKING_APPS_REPLY;
					break; }
			}
			AppStats *stats;
			if(recent_count == 0) { recent_count = appsList.CountItems(); }//add all apps to message
			else { recent_count = min_c(recent_count, appsList.CountItems()); }//add only requested count or number of apps in list, whichever is least
			for(int i=0; i<recent_count; i++)//for each AppAttrFile object starting at index 0
			{	stats = (AppStats*)appsList.ItemAt(i);
				replymsg.AddString("app_signature", stats->getSig());//add app signature to message
				replymsg.AddString("app_path", stats->getPath());//add app path to message
				replymsg.AddInt32("score", stats->getScore());//add app score to message
				replymsg.AddInt32("rank", i+1);//add app rank
				replymsg.AddInt32("last_launch", stats->getLastLaunch());//add app last_date to message
			}
			//Send message
			msg->SendReply(&replymsg);//reply is sent to the app which sent msg
			deleteList(appsList, stats);
			break; }
		case E_RESCAN_DATA_FILES:
			rescanAllAttrFiles();
		case E_UPDATE_QUARTILES:
			updateQuartiles();
		case E_UPDATE_SCORES:
			updateAllAttrScores();
			break;
		case E_SET_IGNORE_ATTR: {
			char* sig;//Find signature
			BEntry appEntry;
			int32 which;
			bool ignore;
			int i=0;
			while(msg->FindString("app_signature", i++, (const char**)&sig) == B_OK)//sig found
			{	//get entry_ref for app from Message
				appEntry = getEntryFromSig(sig);
				if(!appEntry.Exists())//Entry doesn't exist.
				{	printf("Entry for app %s could not be found.\n", sig);
					continue;//Can't do anything, so go to next sig
				}
				if(msg->FindInt32("which", &which) == B_OK)
				{	ignore = (which == 0); }
				else continue;
				// write ignore attribute
				AppAttrFile attrFile(sig, &appEntry);
				attrFile.setIgnore(ignore);
				attrFile.Close();
			}
			break; }
		default: BApplication::MessageReceived(msg);//The message may be for the application
	}
}
void einsteinium_engine::forEachAttrFile(int action)
{
	//create path for the application attribute files directory
	BPath appAttrDirPath(settingsDirPath);
	appAttrDirPath.Append(e_settings_app_dir);
	BDirectory appAttrDir(appAttrDirPath.Path());//create a directory object from the path
	BEntry attrEntry;//a BEntry to get the entry for each file in the directory
	BNode attrNode;
	BNodeInfo attrNodeInfo;
	char nodeType[B_MIME_TYPE_LENGTH];
	appAttrDir.Rewind();//start at beginning of entry list
	while(appAttrDir.GetNextEntry(&attrEntry) == B_OK)//get each entry in the directory
	{	if( (attrNode.SetTo(&attrEntry)) != B_OK) { continue; }
		if( (attrNodeInfo.SetTo(&attrNode)) != B_OK) { continue; }
		if( (attrNodeInfo.GetType(nodeType)) != B_OK ||
			strcmp(nodeType, e_app_attr_filetype) != 0) { continue; }
		attrNode.Unset();
		switch(action)
		{	case UPDATE_ATTR_SCORE: {
				updateAttrScore(&attrEntry);
				break; }
			case RESCAN_ATTR_DATA: {
				rescanAttrFile(&attrEntry);
				break; }
			case CREATE_APP_LIST: {
				AppAttrFile attrFile(&attrEntry);
				// check ignore value- skip if ignore flag is true
				if(attrFile.getIgnore() == false)
				{
					AppStats *appStatsData = new AppStats();
					attrFile.CopyAppStatsInto(appStatsData);
					appsList.AddItem(appStatsData);//add object to the list
//					attrEntry.GetPath(&appAttrDirPath);//create path from entry
//					printf("Adding app %s to list\n", appAttrDirPath.Path());//debug info
				}
				break;
			}
		}
	}
}
void einsteinium_engine::rescanAllAttrFiles()
{	forEachAttrFile(RESCAN_ATTR_DATA); }

void einsteinium_engine::rescanAttrFile(BEntry* entry)
{	AppAttrFile attrFile(entry);
	attrFile.rescanData();
	attrFile.Close();
}


void einsteinium_engine::updateAllAttrScores()
{	forEachAttrFile(UPDATE_ATTR_SCORE); }

void einsteinium_engine::updateAttrScore(BEntry *entry)
{
//	BPath path(entry);
//	printf("Updating attribute %s\n", path.Path());
	AppAttrFile attrFile(entry);
	attrFile.calculateScore();
	attrFile.Close();
}


void einsteinium_engine::createAppList()//Created list of apps
{	AppStats* t;
	deleteList(appsList, t);
	forEachAttrFile(CREATE_APP_LIST);
}


void einsteinium_engine::updateQuartiles()
{	createAppList();
	//Update Overall Score
	printf("Getting quartiles for score\n");
	appsList.SortItems(AppStatsSortScore);
	getQuartiles(getStatsScore, appsList, quartiles+Q_SCORE_INDEX);
	//Update last launch time
	printf("Getting quartiles for last launch time\n");
	appsList.SortItems(AppStatsSortLastLaunch);
	getQuartiles(getStatsLastLaunch, appsList, quartiles+Q_LAST_LAUNCH_INDEX);
	//Update first launch time
	printf("Getting quartiles for first launch time\n");
	appsList.SortItems(AppStatsSortFirstLaunch);
	getQuartiles(getStatsFirstLaunch, appsList, quartiles+Q_FIRST_LAUNCH_INDEX);
	//Update last interval
	printf("Getting quartiles for last interval\n");
	// remove apps from the list that have a last interval of 0 so they don't
	// skew the quartiles with lots of 0's
	BList intervalList;
	for(int i=0; i<appsList.CountItems(); i++)
	{
		AppStats *stats = (AppStats*)appsList.ItemAt(i);
		if(stats->last_interval != 0) intervalList.AddItem(stats);
	}
	intervalList.SortItems(AppStatsSortLastInterval);
	getQuartiles(getStatsLastInterval, intervalList, quartiles+Q_LAST_INTERVAL_INDEX);
	intervalList.MakeEmpty();
	//Update Total Run Time
	printf("Getting quartiles for total running time\n");
	appsList.SortItems(AppStatsSortRunningTime);
	getQuartiles(getStatsRunningTime, appsList, quartiles+Q_TOTAL_RUN_TIME_INDEX);
	//Update Launch Count
	printf("Getting quartiles for launch count\n");
	appsList.SortItems(AppStatsSortLaunchCount);
	getQuartiles(getStatsLaunchCount, appsList, quartiles+Q_LAUNCHES_INDEX);

	// Write to file
	BPath EstatsPath(settingsDirPath);
	EstatsPath.Append("engine_quartiles");
	BFile statsFile(EstatsPath.Path(), B_READ_WRITE | B_CREATE_FILE);
	if(statsFile.InitCheck() != B_OK) return;
	writeQuartiles(&statsFile, quartiles);
	statsFile.Unset();
	AppStats *stats;
	deleteList(appsList, stats);
}

template < class itemType >
void einsteinium_engine::getQuartiles(itemType (*getFunc)(AppStats*), BList &workingList,
						double *Q)
{	int count = workingList.CountItems();
	if(count<5) return;
	double index;
	itemType ql, qu;
	for(int i=0; i<5; i++)
	{
		index = ((4.0-i)/4.0)*(count-1);
		// index is the location in workingList where the quartile Q0, Q1, Q2, Q3 or Q4
		// falls.  index can be a non integer, in which case the quartile value actually
		// falls in between the values of two items in workingList.
		qu = getFunc( (AppStats*)workingList.ItemAt(int(index)) );
		// qu is the upper level of the quartile value
		if(i==0 || i==4)
		// For Q0 and Q4 the quartile value is exactly the value of the first and last
		// item respectively
		{	Q[i] = qu; }
		else
		// Q1, Q2 and Q3 values usually fall in between item values
		{	ql = getFunc( (AppStats*)workingList.ItemAt(int(index)+1) );
			// ql is the value lower than the actual quartile value (remember index 0
			// is the highest value, then the values decrease as the index increases)
			Q[i] = ql + double(qu - ql)*(index-int(index));
			// double(qu - ql) is the difference between the two values.
			// (index-int(index)) is the fractional place between the the items where the quartile falls
			// Multiply the two and get the fraction of the difference to add to the lower
			// item value to get the final quartile value.
		}
		printf("Q%i (index value %f): %f\n", i, index, Q[i]);
	}
}


void einsteinium_engine::writeQuartiles(BFile* file, double *Q)
{	writeQuartilesNamed(file, Q+Q_SCORE_INDEX, "EIN:SCORE");
	writeQuartilesNamed(file, Q+Q_FIRST_LAUNCH_INDEX, "EIN:FIRST_LAUNCH");
	writeQuartilesNamed(file, Q+Q_LAST_LAUNCH_INDEX, "EIN:LAST_LAUNCH");
	writeQuartilesNamed(file, Q+Q_LAST_INTERVAL_INDEX, "EIN:LAST_INTERVAL");
	writeQuartilesNamed(file, Q+Q_LAUNCHES_INDEX, "EIN:LAUNCHES");
	writeQuartilesNamed(file, Q+Q_TOTAL_RUN_TIME_INDEX, "EIN:TOTAL_RUN_TIME");
}

void einsteinium_engine::writeQuartilesNamed(BFile* file, double* Q, const char* name)
{	BString nameStr;
	char num[2];
	for(int i=0; i<5; i++)
	{	sprintf(num, "%i", i);
		nameStr.SetTo(name);
		nameStr.Append("_Q");
		nameStr.Append(num);
		file->WriteAttr(nameStr.String(), B_DOUBLE_TYPE, 0, Q+i, sizeof(double));
	}
}
/*bool einsteinium_engine::readQuartiles(BFile *file, double* Q)
{	readQuartilesNamed(file, Q+Q_SCORE_INDEX, "EIN:SCORE");
	readQuartilesNamed(file, Q+Q_FIRST_LAUNCH_INDEX, "EIN:FIRST_LAUNCH");
	readQuartilesNamed(file, Q+Q_LAST_LAUNCH_INDEX, "EIN:LAST_LAUNCH");
	readQuartilesNamed(file, Q+Q_LAST_INTERVAL_INDEX, "EIN:LAST_INTERVAL");
	readQuartilesNamed(file, Q+Q_LAUNCHES_INDEX, "EIN:LAUNCHES");
	readQuartilesNamed(file, Q+Q_TOTAL_RUN_TIME_INDEX, "EIN:TOTAL_RUN_TIME");
	return true;
}
bool einsteinium_engine::readQuartilesNamed(BFile* statsFile, double* Q, const char* name)
{	attr_info info;
	BString nameStr;
	char num[2];
	for(int i=0; i<5; i++)
	{	sprintf(num, "%i", i);
		nameStr.SetTo(name);
		nameStr.Append("_Q");
		nameStr.Append(num);
		if(statsFile->GetAttrInfo(nameStr.String(), &info) != B_NO_ERROR) { return false; }
		statsFile->ReadAttr(nameStr.String(), B_DOUBLE_TYPE, 0, Q+i, sizeof(double));
	}
	return true;
}*/


template < class itemType >
void deleteList(BList& list, itemType* item)
{	do
	{	item = static_cast<itemType *>(list.RemoveItem(int32(0)));
		if(item) delete item;
	}while(item);
}

