/* einsteinium_engine.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "einsteinium_engine.h"


einsteinium_engine::einsteinium_engine()//Einsteinium Engine Constructor
	:
	BApplication(e_engine_sig),
	fWatchingRoster(false),
	fEngineSession(time(NULL)),
	fShelfViewId(0)
{
//	printf("ee_session=%i\n", ee_session);
	//Create settings directories if they are not found
	BDirectory settingsDir;
	find_directory(B_USER_SETTINGS_DIRECTORY, &fSettingsDirPath);
	fSettingsDirPath.Append(e_settings_dir);
	if( settingsDir.SetTo(fSettingsDirPath.Path()) == B_ENTRY_NOT_FOUND )
	{	printf("Settings directory not found, creating directory\n    %s\n",
					fSettingsDirPath.Path());
		if(settingsDir.CreateDirectory(fSettingsDirPath.Path(), &settingsDir)!=B_OK)
		{	//Creating directory failed
			printf("Error creating Einsteinium settings folder.  Cannot continue.\n");
			be_app->PostMessage(B_QUIT_REQUESTED);//Quit. Can we do anthything w/o settings?
			return;//Cannot continue, exit constructor
		}
	}
	//find ~/config/settings/Einsteinium/Applications directory
	BPath appSettingsPath(fSettingsDirPath);
	appSettingsPath.Append(e_settings_app_dir);
	if( settingsDir.SetTo(appSettingsPath.Path()) == B_ENTRY_NOT_FOUND )
	{	printf("Settings directory not found, creating directory\n    %s\n",
					appSettingsPath.Path());
		if(settingsDir.CreateDirectory(appSettingsPath.Path(), &settingsDir)!=B_OK)
		{	//Creating directory failed
			printf("Error creating %s settings folder.  "
					"Cannot continue.\n", appSettingsPath.Path());
			be_app->PostMessage(B_QUIT_REQUESTED);
			return;
		}
	}

	//Initialize the settings file object and check to see if it instatiated correctly
	fSettingsFile = new EESettingsFile();
	status_t result = fSettingsFile->CheckStatus();
	if(result!=B_OK){
		printf("Error creating Einsteinium Daemon settings file.  Cannot continue.\n");
		be_app->PostMessage(B_QUIT_REQUESTED);//Quit. Can we do anthything w/o settings?
	}

	// TODO Register the signature of the preferences application
}

/*
einsteinium_engine::~einsteinium_engine()//Final cleanup (most stuff done in QuitRequested())
{	}*/


bool
einsteinium_engine::QuitRequested()
{
	//Need to stop watching the roster
	if(fWatchingRoster)
	{	be_roster->StopWatching(be_app_messenger); }

	//disable timer
	if(fQuartileRunner != NULL)
		delete fQuartileRunner;

	// TODO wait for all messages to clear the message queue?

	// Delete subscribers list items
	Subscriber *sub;
	DeleteList(fSubscribersList, sub);

	// Remove shelf view
	_ShowShelfView(false, 0);

	delete fSettingsFile;

	printf("Einsteinium engine quitting.\n");
	return BApplication::QuitRequested();
}


void
einsteinium_engine::ReadyToRun()
{
	// Create the attribute file indexes
/*	BVolumeRoster v_roster;
	BVolume vol;
	v_roster.GetBootVolume(&vol);
	dev_t device = vol.Device();
	//fs_create_index(device, stringIndices[i], B_STRING_TYPE, 0);
	fs_create_index(device, ATTR_IGNORE_NAME, B_BOOL_TYPE, 0);*/

	//Start watching the application roster for launches/quits/activations
	if (be_roster->StartWatching(be_app_messenger,
				B_REQUEST_QUIT | B_REQUEST_LAUNCHED/* | B_REQUEST_ACTIVATED*/) != B_OK)
	{	//roster failed to be watched.  Should we continue or just quit???
		printf("Error initializing watching the roster.\n");
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
	else
	{	fWatchingRoster = true;
			//set the flag to indicate we're watching the roster
		printf("Einsteinium engine running.\n");
	}

	//update quartiles, set up timer to update quartiles regularly
	_UpdateQuartiles();
	fQuartileRunner = new BMessageRunner(this, new BMessage(E_UPDATE_QUARTILES), 3600*10e6, -1);
	if(fQuartileRunner->InitCheck()!=B_OK)
	{	delete fQuartileRunner;
		fQuartileRunner = NULL;
	}

	// Add the shelf view
	bool showShelfView;
	int shelfViewCount;
	fSettingsFile->GetDeskbarSettings(showShelfView, shelfViewCount);
	_ShowShelfView(showShelfView, shelfViewCount);
}


void
einsteinium_engine::ArgvReceived(int32 argc, char** argv)
{	if(argc>1)
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
void
einsteinium_engine::MessageReceived(BMessage *msg)
{	switch(msg->what)
	{
		//---- Core functional messages ----//
		case B_SOME_APP_QUIT:
		case B_SOME_APP_LAUNCHED: {
			//Find the signature
			char* sig;
			if (msg->FindString("be:signature", (const char**)&sig) == B_OK)
			{
				entry_ref appRef;
				BEntry appEntry;
				if (msg->FindRef("be:ref", &appRef) != B_OK)
				{	printf("App entry not found in message.  Attempting to query app.\n");
					appEntry = GetEntryFromSig(sig);
						//Query app by signature
				}
				else
					appEntry.SetTo(&appRef);
						//entry_ref found in Message

				//Entry doesn't exist.  Shouldn't happen, but just in case.
				if(!appEntry.Exists())
				{	printf("Entry for app %s could not be found.\n", sig);
					break;
						//Can't do anything, so exit switch
				}

				AppAttrFile appFile(sig, &appEntry);
					//Open application's Einsteinium attribute file

				// If this app is not ignored, get the current rank
				uint previous_rank;
				bool haveSubscribers = fSubscribersList.CountItems() > 0;
				bool ignoredApp = appFile.GetIgnore();
				BList appStatsList;
				if(haveSubscribers && !ignoredApp)
				{
					appStatsList = _CreateAppStatsList(SORT_BY_SCORE);
					previous_rank = _FindAppStatsRank(appStatsList, sig);
					AppStats *ptr = (AppStats*)(appStatsList.RemoveItem(previous_rank - 1));
					if(ptr)
						delete ptr;
				}

				//Update object's data (times, score)
				switch(msg->what)
				{	case B_SOME_APP_LAUNCHED: {
						appFile.UpdateAppLaunched();
						break;
					}
					case B_SOME_APP_QUIT: {
						appFile.UpdateAppQuit();
						break;
					}
				}

				if(haveSubscribers && !ignoredApp)
				{
					// Get the new rank
					AppStats *newStats = new AppStats();
					appFile.CopyAppStatsInto(newStats);
					appStatsList.AddItem(newStats);
					_SortAppStatsList(appStatsList, SORT_BY_SCORE);
					uint current_rank = _FindAppStatsRank(appStatsList, sig);
					if(previous_rank != current_rank)
					{
						// The rank has changed, so we may need to update subscribers
						int subscribersCount = fSubscribersList.CountItems();
						for(int i=0; i<subscribersCount; i++)
						{
							Subscriber *currentSub = (Subscriber*)fSubscribersList.ItemAt(i);
							if( (previous_rank<=currentSub->count)
								|| (current_rank<=currentSub->count) )
							{
								_SendListToSubscriber(&appStatsList, currentSub);
							}
						}
					}
					_EmptyAppStatsList(appStatsList);
				}
			}
			//no signature found in message
			else
				printf("Application has no signature.\n");
			break;
		}
	/*	case B_SOME_APP_ACTIVATED: {//An application brought to the front??
			break; }*/
		case E_SET_IGNORE_ATTR: {
			char* sig;
			BEntry appEntry;
			int32 which;
			bool ignore;
			int i=0;
			BList appStatsList;
			uint lowestRankChanged = 0-1;
				//make this uint the largest number possible
			bool haveSubscribers = fSubscribersList.CountItems() > 0;
			if(haveSubscribers)
				appStatsList = _CreateAppStatsList(SORT_BY_SCORE);
			while(msg->FindString("app_signature", i++, (const char**)&sig) == B_OK)
			{
				appEntry = GetEntryFromSig(sig);
				if(!appEntry.Exists())
				{	printf("Entry for app %s could not be found.\n", sig);
					continue;
						//Can't do anything, so go to next sig
				}
				if(msg->FindInt32("which", &which) == B_OK)
				{	ignore = (which == 0); }
				else continue;
				AppAttrFile attrFile(sig, &appEntry);
				bool currentIgnoreVal = attrFile.GetIgnore();
				// if the value has changed
				if(ignore != currentIgnoreVal)
				{
					attrFile.SetIgnore(ignore);
					if(haveSubscribers)
					{
						if(!ignore)
						{
							//add stats to apps list
							AppStats *appStatsData = new AppStats();
							attrFile.CopyAppStatsInto(appStatsData);
							appStatsList.AddItem(appStatsData);
							_SortAppStatsList(appStatsList, SORT_BY_SCORE);
						}
						uint currentRank = _FindAppStatsRank(appStatsList, sig);
						lowestRankChanged = min_c(lowestRankChanged, currentRank);
						if(ignore)
						{
							//remove from list
							AppStats *ptr = (AppStats*)(appStatsList.RemoveItem(currentRank - 1));
							if(ptr)
								delete ptr;
						}
					}
				}
			}
			if(haveSubscribers)
			{
				int subscribersCount = fSubscribersList.CountItems();
				for(int j=0; j<subscribersCount; j++)
				{
					Subscriber *currentSub = (Subscriber*)fSubscribersList.ItemAt(j);
					if( lowestRankChanged<=currentSub->count )
						_SendListToSubscriber(&appStatsList, currentSub);
				}
				_EmptyAppStatsList(appStatsList);
			}
			break; }
		case E_UPDATE_SHELFVIEW_SETTINGS:
		{
			bool showShelfView;
			int16 count;
			status_t result1 = msg->FindBool("show", &showShelfView);
			status_t result2 = msg->FindInt16("count", &count);
			if(result1 != B_OK || result2 != B_OK)
			{
				printf("Einsteinium Engine received invalid ShelfView settings message.\n");
				break;
			}
			_ShowShelfView(showShelfView, count);
			break;
		}

		//---- Subscriber messages ----//
		case E_SUBSCRIBE_RANKED_APPS: {
		//	printf("Received subscription message\n");
			Subscriber* newSubscriber = new Subscriber();
			status_t result1 = msg->FindMessenger("messenger", &(newSubscriber->messenger));
			status_t result2 = msg->FindInt16("count", &(newSubscriber->count));
			status_t result3 = msg->FindInt32("uniqueID", &(newSubscriber->uniqueID));
			if(result1 != B_OK || result2 != B_OK || result3 != B_OK)
			{
				printf("Einsteinium Engine received invalid subscribe message.\n");
				delete newSubscriber;
				// TODO send reply?
				break;
			}
			// TODO if there already is a subscriber with the unique ID update it
			fSubscribersList.AddItem(newSubscriber);
			BList appStatsList = _CreateAppStatsList(SORT_BY_SCORE);
			_SendListToSubscriber(&appStatsList, newSubscriber);
			_EmptyAppStatsList(appStatsList);
			break;
		}
		case E_UNSUBSCRIBE_RANKED_APPS: {
		//	printf("Received unsubscribe message\n");
			int32 uniqueID;
			status_t result = msg->FindInt32("uniqueID", &uniqueID);
			if(result!=B_OK)
			{
				printf("Einsteinium Engine received invalid unsubscribe message.\n");
				break;
			}
			int subscribersCount = fSubscribersList.CountItems();
			for(int j=0; j<subscribersCount; j++)
			{
				Subscriber *currentSub = (Subscriber*)fSubscribersList.ItemAt(j);
				if( uniqueID==currentSub->uniqueID )
				{
					fSubscribersList.RemoveItem(currentSub);
					delete currentSub;
					j = subscribersCount;
				//	printf("Successful unsubscribe\n");
				}
			}
			break;
		}

		//---- External requests ----//
		case E_PRINT_RECENT_APPS:
		case E_PRINT_RANKING_APPS: {
		//	printf("Constructing Recent Apps List:\n");
			//create list of eligible apps
			BList appStatsList = _CreateAppStatsList();
			switch(msg->what){
				case E_PRINT_RECENT_APPS: {
					printf("Sorting list by most recent...\n");
					_SortAppStatsList(appStatsList, SORT_BY_LAST_LAUNCHTIME);
					break; }
				case E_PRINT_RANKING_APPS: {
					printf("Sorting list by score...\n");
					_SortAppStatsList(appStatsList, SORT_BY_SCORE);
					break; }
			}
			//print results
			printf("Ordered list of apps:\n");
			AppStats *stats;
			int count = appStatsList.CountItems();
			for(int i=0; i<count; i++)
			{	stats = (AppStats*)(appStatsList.ItemAt(i));
				printf("%i: %s (%s)\n", i+1, stats->GetSig(), stats->GetFilename() );
			}
			_EmptyAppStatsList(appStatsList);
			break; }
		//Send a reply back to another app with data from the recent apps list
		case E_REPLY_RECENT_APPS:
		case E_REPLY_RANKING_APPS: {
			int16 replyCount = 0;
			msg->FindInt16("count", &replyCount);
				//number of apps requested to return
			//create list of eligible apps
			BList appStatsList = _CreateAppStatsList();
			BMessage replyMessage;
			switch(msg->what)
			{	case E_REPLY_RECENT_APPS: {
					printf("Sorting list by most recent...\n");
					_SortAppStatsList(appStatsList, SORT_BY_LAST_LAUNCHTIME);
					replyMessage.what = E_RECENT_APPS_REPLY;
					break; }
				case E_REPLY_RANKING_APPS: {
					printf("Sorting list by score...\n");
					_SortAppStatsList(appStatsList, SORT_BY_SCORE);
					replyMessage.what = E_RANKING_APPS_REPLY;
					break; }
			}
			if(replyCount == 0)
				replyCount = appStatsList.CountItems();
					//add all apps to message
			else
				replyCount = min_c(replyCount, appStatsList.CountItems());
					//add only requested count or number of apps in list, whichever is least

		/*	AppStats *stats;
			for(int i=0; i<recent_count; i++)//for each AppAttrFile object starting at index 0
			{	stats = (AppStats*)appsList.ItemAt(i);
				replymsg.AddString("app_signature", stats->getSig());//add app signature to message
				replymsg.AddString("app_path", stats->getPath());//add app path to message
				replymsg.AddInt32("score", stats->getScore());//add app score to message
				replymsg.AddInt32("rank", i+1);//add app rank
				replymsg.AddInt32("last_launch", stats->getLastLaunch());//add app last_date to message
			}*/

			//Send message
			_PopulateAppRankMessage(&appStatsList, &replyMessage, replyCount);
			msg->SendReply(&replyMessage);
			_EmptyAppStatsList(appStatsList);
			break; }
		case E_RESCAN_DATA_FILES:
			_RescanAllAttrFiles();
		case E_UPDATE_QUARTILES:
			_UpdateQuartiles();
		case E_UPDATE_SCORES:
		{
			_UpdateAllAttrScores();
			// update subscribers
			int subscribersCount = fSubscribersList.CountItems();
			if(subscribersCount>0)
			{
				BList appStatsList = _CreateAppStatsList(SORT_BY_SCORE);
				for(int j=0; j<subscribersCount; j++)
				{
					Subscriber *currentSub = (Subscriber*)fSubscribersList.ItemAt(j);
					_SendListToSubscriber(&appStatsList, currentSub);
				}
				_EmptyAppStatsList(appStatsList);
			}
			break;
		}
		default:
			BApplication::MessageReceived(msg);
	}
}


void
einsteinium_engine::_SendListToSubscriber(BList *appStatsList, Subscriber *subscriber)
{
	BMessage rankMsg(E_SUBSCRIBER_UPDATE_RANKED_APPS);
	int appsCount = min_c(subscriber->count, appStatsList->CountItems());
	_PopulateAppRankMessage(appStatsList, &rankMsg, appsCount);
	subscriber->messenger.SendMessage(&rankMsg);
}


void
einsteinium_engine::_PopulateAppRankMessage(BList *appStatsList, BMessage *message, int count)
{
	AppStats* stats;
	BEntry appEntry;
	entry_ref appRef;
	for(int j=0; j<count; j++)
	{
		stats = (AppStats*)appStatsList->ItemAt(j);
		if(appEntry.SetTo(stats->app_path.String()) == B_OK)
		{
			appEntry.GetRef(&appRef);
			message->AddRef("refs", &appRef);
		}
	}
}


void
einsteinium_engine::_ShowShelfView(bool showShelfView, int shelfViewCount)
{
	BDeskbar deskbar;
	if(fShelfViewId)// shelf view is currently showing
	{
		deskbar.RemoveItem(fShelfViewId);
		fShelfViewId = 0;
	}
	if(showShelfView)
	{
		BView *shelfView = new EEShelfView(BRect(0, 0, 15, 15), shelfViewCount);
		deskbar.AddItem(shelfView, &fShelfViewId);
		delete shelfView;
	}
}


/*void
einsteinium_engine::_DoRankQuery()
{
	printf("Running rank query\n");
	BVolumeRoster v_roster;
	BVolume vol;
	v_roster.GetBootVolume(&vol);
	rankQuery.Clear();
	rankQuery.PushAttr("BEOS:TYPE");
	rankQuery.PushString(e_app_attr_filetype);
	rankQuery.PushOp(B_EQ);
	rankQuery.SetVolume(&vol);
	if(rankQuery.Fetch()!=B_OK)
	{
		printf("Error fetching query\n");
		return;
	}
	entry_ref entry;

	while(rankQuery.GetNextRef(&entry) == B_OK)
	{
		printf("Rank Query found entry %s\n", entry.name);
	}
	printf("Done query\n");

}*/


void
einsteinium_engine::_ForEachAttrFile(int action, BList *appStatsList = NULL)
{
	//create path for the application attribute files directory
	BPath appAttrDirPath(fSettingsDirPath);
	appAttrDirPath.Append(e_settings_app_dir);
	BDirectory appAttrDir(appAttrDirPath.Path());
	BEntry attrEntry;
	BNode attrNode;
	BNodeInfo attrNodeInfo;
	char nodeType[B_MIME_TYPE_LENGTH];
	appAttrDir.Rewind();
	while(appAttrDir.GetNextEntry(&attrEntry) == B_OK)
	{	if( (attrNode.SetTo(&attrEntry)) != B_OK) { continue; }
		if( (attrNodeInfo.SetTo(&attrNode)) != B_OK) { continue; }
		if( (attrNodeInfo.GetType(nodeType)) != B_OK ||
			strcmp(nodeType, e_app_attr_filetype) != 0) { continue; }
		attrNode.Unset();
		switch(action)
		{	case UPDATE_ATTR_SCORE: {
				_UpdateAttrScore(&attrEntry);
				break; }
			case RESCAN_ATTR_DATA: {
				_RescanAttrFile(&attrEntry);
				break; }
			case CREATE_APP_LIST: {
				AppAttrFile attrFile(&attrEntry);
				// check ignore value- skip if ignore flag is true
				if(attrFile.GetIgnore() == false)
				{
					AppStats *appStatsData = new AppStats();
					attrFile.CopyAppStatsInto(appStatsData);
					appStatsList->AddItem(appStatsData);
//					attrEntry.GetPath(&appAttrDirPath);//create path from entry
//					printf("Adding app %s to list\n", appAttrDirPath.Path());//debug info
				}
				break;
			}
		}
	}
}


void
einsteinium_engine::_RescanAllAttrFiles()
{
	_ForEachAttrFile(RESCAN_ATTR_DATA);
}

void
einsteinium_engine::_RescanAttrFile(BEntry* entry)
{
	AppAttrFile attrFile(entry);
	attrFile.RescanData();
}


void
einsteinium_engine::_UpdateAllAttrScores()
{
	_ForEachAttrFile(UPDATE_ATTR_SCORE);
}

void
einsteinium_engine::_UpdateAttrScore(BEntry *entry)
{
//	BPath path(entry);
//	printf("Updating attribute %s\n", path.Path());
	AppAttrFile attrFile(entry);
	attrFile.CalculateScore();
}


BList
einsteinium_engine::_CreateAppStatsList(int sortAction=SORT_BY_NONE)
{
	BList newList;
	_ForEachAttrFile(CREATE_APP_LIST, &newList);
	_SortAppStatsList(newList, sortAction);
	return newList;
}


void
einsteinium_engine::_SortAppStatsList(BList &list, int sortAction)
{
	switch(sortAction)
	{
		case SORT_BY_SCORE:
		{	list.SortItems(AppStatsSortScore);
			break;
		}
		case SORT_BY_LAST_LAUNCHTIME:
		{	list.SortItems(AppStatsSortLastLaunch);
			break;
		}
		case SORT_BY_FIRST_LAUNCHTIME:
		{	list.SortItems(AppStatsSortFirstLaunch);
			break;
		}
		case SORT_BY_LAUNCH_COUNT:
		{	list.SortItems(AppStatsSortLaunchCount);
			break;
		}
		case SORT_BY_LAST_INTERVAL:
		{	list.SortItems(AppStatsSortLastInterval);
			break;
		}
		case SORT_BY_TOTAL_RUNNING_TIME:
		{	list.SortItems(AppStatsSortRunningTime);
			break;
		}
	}
}


void
einsteinium_engine::_EmptyAppStatsList(BList &list)
{
	AppStats* t;
	DeleteList(list, t);
}

uint
einsteinium_engine::_FindAppStatsRank(BList &appStatsList, const char* signature)
{
	uint rank = 0-1;
	int count = appStatsList.CountItems();
	for(int i=0; i<count; i++)
	{
		AppStats* stats = (AppStats*)appStatsList.ItemAt(i);
		if(stats->app_sig.Compare(signature) == 0)
		{
			rank = i + 1;
			i = count;
		}
	}
	return rank;
}


void
einsteinium_engine::_UpdateQuartiles()
{
	BList appsList = _CreateAppStatsList();
	//Update Overall Score
	printf("Getting quartiles for score\n");
	appsList.SortItems(AppStatsSortScore);
	_GetQuartiles(GetStatsScore, appsList, fQuartiles+Q_SCORE_INDEX);
	//Update last launch time
	printf("Getting quartiles for last launch time\n");
	appsList.SortItems(AppStatsSortLastLaunch);
	_GetQuartiles(GetStatsLastLaunch, appsList, fQuartiles+Q_LAST_LAUNCH_INDEX);
	//Update first launch time
	printf("Getting quartiles for first launch time\n");
	appsList.SortItems(AppStatsSortFirstLaunch);
	_GetQuartiles(GetStatsFirstLaunch, appsList, fQuartiles+Q_FIRST_LAUNCH_INDEX);
	//Update last interval
	printf("Getting quartiles for last interval\n");
	// Create a copy of the list and remove apps that have a last interval of 0 so they don't
	// skew the quartiles with lots of 0's
	BList intervalList;
	for(int i=0; i<appsList.CountItems(); i++)
	{
		AppStats *stats = (AppStats*)appsList.ItemAt(i);
		if(stats->last_interval != 0) intervalList.AddItem(stats);
	}
	intervalList.SortItems(AppStatsSortLastInterval);
	_GetQuartiles(GetStatsLastInterval, intervalList, fQuartiles+Q_LAST_INTERVAL_INDEX);
	intervalList.MakeEmpty();
	//Update Total Run Time
	printf("Getting quartiles for total running time\n");
	appsList.SortItems(AppStatsSortRunningTime);
	_GetQuartiles(GetStatsRunningTime, appsList, fQuartiles+Q_TOTAL_RUN_TIME_INDEX);
	//Update Launch Count
	printf("Getting quartiles for launch count\n");
	appsList.SortItems(AppStatsSortLaunchCount);
	_GetQuartiles(GetStatsLaunchCount, appsList, fQuartiles+Q_LAUNCHES_INDEX);

	// Write to file
	BPath EstatsPath(fSettingsDirPath);
	EstatsPath.Append("engine_quartiles");
	BFile statsFile(EstatsPath.Path(), B_READ_WRITE | B_CREATE_FILE);
	if(statsFile.InitCheck() != B_OK) return;
	_WriteQuartiles(&statsFile, fQuartiles);
	statsFile.Unset();
	_EmptyAppStatsList(appsList);
}

// TODO there is a bug when there are less than 10 data sets available- score becomes
// severly negative for apps that have more than one launch.  Needs to investigate this.


template < class itemType >
void
einsteinium_engine::_GetQuartiles(itemType (*getFunc)(AppStats*), BList &workingList,
						double *Q)
{
	int count = workingList.CountItems();
	if(count<5)
		return;
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


void
einsteinium_engine::_WriteQuartiles(BFile* file, double *Q)
{
	_WriteQuartilesNamed(file, Q+Q_SCORE_INDEX, "EIN:SCORE");
	_WriteQuartilesNamed(file, Q+Q_FIRST_LAUNCH_INDEX, "EIN:FIRST_LAUNCH");
	_WriteQuartilesNamed(file, Q+Q_LAST_LAUNCH_INDEX, "EIN:LAST_LAUNCH");
	_WriteQuartilesNamed(file, Q+Q_LAST_INTERVAL_INDEX, "EIN:LAST_INTERVAL");
	_WriteQuartilesNamed(file, Q+Q_LAUNCHES_INDEX, "EIN:LAUNCHES");
	_WriteQuartilesNamed(file, Q+Q_TOTAL_RUN_TIME_INDEX, "EIN:TOTAL_RUN_TIME");
}


void
einsteinium_engine::_WriteQuartilesNamed(BFile* file, double* Q, const char* name)
{
	BString nameStr;
	char num[2];
	for(int i=0; i<5; i++)
	{	sprintf(num, "%i", i);
		nameStr.SetTo(name);
		nameStr.Append("_Q");
		nameStr.Append(num);
		file->WriteAttr(nameStr.String(), B_DOUBLE_TYPE, 0, Q+i, sizeof(double));
	}
}


/*bool einsteinium_engine::_ReadQuartiles(BFile *file, double* Q)
{
	_ReadQuartilesNamed(file, Q+Q_SCORE_INDEX, "EIN:SCORE");
	_ReadQuartilesNamed(file, Q+Q_FIRST_LAUNCH_INDEX, "EIN:FIRST_LAUNCH");
	_ReadQuartilesNamed(file, Q+Q_LAST_LAUNCH_INDEX, "EIN:LAST_LAUNCH");
	_ReadQuartilesNamed(file, Q+Q_LAST_INTERVAL_INDEX, "EIN:LAST_INTERVAL");
	_ReadQuartilesNamed(file, Q+Q_LAUNCHES_INDEX, "EIN:LAUNCHES");
	_ReadQuartilesNamed(file, Q+Q_TOTAL_RUN_TIME_INDEX, "EIN:TOTAL_RUN_TIME");
	return true;
}
bool einsteinium_engine::_ReadQuartilesNamed(BFile* statsFile, double* Q, const char* name)
{
	attr_info info;
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
void
DeleteList(BList& list, itemType* item)
{
	do
	{	item = static_cast<itemType *>(list.RemoveItem(int32(0)));
		if(item) delete item;
	}while(item);
}

