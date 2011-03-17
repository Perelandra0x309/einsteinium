/* einsteinium_engine.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "einsteinium_engine.h"


einsteinium_engine::einsteinium_engine()//Einsteinium Engine Constructor
	:
	BApplication(e_engine_sig),
	fWatchingRoster(false),
	fEngineSession(time(NULL))
{
//	printf("fEngineSession=%i\n", fEngineSession);
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
	// TODO do we even use the settings file here?
	fSettingsFile = new EESettingsFile();
	status_t result = fSettingsFile->CheckStatus();
	if(result!=B_OK){
		printf("Error creating Einsteinium Engine settings file.  Cannot continue.\n");
		be_app->PostMessage(B_QUIT_REQUESTED);
			//Quit. Can we do anthything w/o settings?
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
//	if(fQuartileRunner != NULL)
//		delete fQuartileRunner;

	// TODO wait for all messages to clear the message queue?

	// Delete subscribers list items
	Subscriber *sub;
	AppStats *stats;
	int subscribersCount = fSubscribersList.CountItems();
	for(int i=0; i<subscribersCount; i++)
	{
		sub = (Subscriber*)fSubscribersList.ItemAt(i);
		DeleteList(sub->appStatsList, stats);
	}
	DeleteList(fSubscribersList, sub);

	delete fSettingsFile;

	printf("Einsteinium engine quitting.\n");
	return BApplication::QuitRequested();
}


void
einsteinium_engine::ReadyToRun()
{
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
}


void
einsteinium_engine::ArgvReceived(int32 argc, char** argv)
{	if(argc>1)
	{	if(strcmp(argv[1],"-q")==0 || strcmp(argv[1],"--quit")==0)//option to quit
		{	PostMessage(B_QUIT_REQUESTED); }
/*		else if(strcmp(argv[1], "--rank")==0)//request list of apps printed to terminal
		{	PostMessage(E_PRINT_RANKING_APPS); }
		else if(strcmp(argv[1], "--recent")==0)//request list of apps printed to terminal
		{	PostMessage(E_PRINT_RECENT_APPS); }*/
//		else if(strcmp(argv[1], "--rescanData")==0)//
//		{	PostMessage(E_RESCAN_DATA_FILES); }
		else if(strcmp(argv[1], "--testMode")==0)
		{	PostMessage(E_TEST_MODE_INIT); }
		else//option is not recognized
		{	printf("Usage: einsteinium_engine [option]\n"
					"Options:\n"
					"-q or --quit		Quit\n"
//					"--rank				List apps by overall rank\n"
//					"--recent			List most recent apps\n"
					"--rescanData		Rescan database files\n"); }
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
				// Automatically ignore binary commands, tracker addons and system servers
				BPath appPath(&appEntry);
				BString pathString(appPath.Path());
				if(pathString.FindFirst(PATH_SYSTEM_BIN) == 0 ||
					pathString.FindFirst(PATH_SYSTEM_SERVERS) == 0 ||
					pathString.FindFirst(PATH_SYSTEM_TRACKER_ADDONS) == 0 ||
					pathString.FindFirst(PATH_COMMON_BIN) == 0 ||
					pathString.FindFirst(PATH_COMMON_TRACKER_ADDONS) == 0 ||
					pathString.FindFirst(PATH_HOME_BIN) == 0 ||
					pathString.FindFirst(PATH_HOME_TRACKER_ADDONS) == 0 )
				{
					break;
				}

				AppAttrFile appFile(sig, &appEntry);
					//Open application's Einsteinium attribute file
				bool appHasWatchers = false;
				int subscribersCount = fSubscribersList.CountItems();
				for(int i=0; i<subscribersCount; i++)
				{
					Subscriber *currentSub = (Subscriber*)fSubscribersList.ItemAt(i);
					currentSub->currentAppIsExcluded = _DetermineExclusion(currentSub, sig);
					if(!currentSub->currentAppIsExcluded)
					{
						// Only if the list is not already sorted by score do we need to sort
						if(currentSub->appStatsSortOrder!=SORT_BY_SCORE)
							_SortAppStatsList(currentSub, SORT_BY_SCORE);
						currentSub->currentAppPreviousRank = _FindAppStatsRank(currentSub, sig);
						// Indicate that at least one subscriber may need to be updated later
						appHasWatchers = true;
					}
					else
						currentSub->currentAppPreviousRank = 0;
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

				if(appHasWatchers)
				{
					int subscribersCount = fSubscribersList.CountItems();
					for(int i=0; i<subscribersCount; i++)
					{
						Subscriber *currentSub = (Subscriber*)fSubscribersList.ItemAt(i);
						if(!currentSub->currentAppIsExcluded)
						{
							// Get the new rank
							AppStats *newStats = appFile.CloneAppStats();
							_CalculateScore(currentSub, newStats);
							AppStats *oldStats = (AppStats*)(currentSub->appStatsList.RemoveItem(currentSub->currentAppPreviousRank - 1));
							if(oldStats)
								delete oldStats;
							currentSub->appStatsList.AddItem(newStats);
							_SortAppStatsList(currentSub, SORT_BY_SCORE);
							uint currentRank = _FindAppStatsRank(currentSub, sig);
							if(currentSub->currentAppPreviousRank != currentRank)
							{
								// The rank has changed, so we may need to update subscriber
								if( (currentSub->currentAppPreviousRank<=currentSub->count)
									|| (currentRank<=currentSub->count) )
								{
									_SendListToSubscriber(currentSub);
								}
							}
						}
					}
				}
			}
			//no signature found in message
			else
				printf("Application has no signature.\n");
			break;
		}

		//---- Subscriber messages ----//
		case E_SUBSCRIBE_RANKED_APPS: {
			printf("Received subscription message\n");
			Subscriber* newSubscriber = new Subscriber();
			status_t result1 = msg->FindMessenger(E_SUBSCRIPTION_MESSENGER, &(newSubscriber->messenger));
			status_t result2 = msg->FindInt16(E_SUBSCRIPTION_COUNT, &(newSubscriber->count));
			// TODO create a unique ID and pass back to the subscriber
			status_t result3 = msg->FindInt32("uniqueID", &(newSubscriber->uniqueID));
			// Get the settings for scale values (default to 0 for any not defined)
			newSubscriber->launch_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_LAUNCH_SCALE, &(newSubscriber->launch_scale));
			newSubscriber->first_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_FIRST_SCALE, &(newSubscriber->first_scale));
			newSubscriber->last_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_LAST_SCALE, &(newSubscriber->last_scale));
			newSubscriber->interval_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_INTERVAL_SCALE, &(newSubscriber->interval_scale));
			newSubscriber->runtime_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_RUNTIME_SCALE, &(newSubscriber->runtime_scale));
			// There must be at least one scale defined to a non zero value
			bool noScalesDefined = ( newSubscriber->launch_scale == 0
									&& newSubscriber->first_scale == 0
									&& newSubscriber->last_scale == 0
									&& newSubscriber->interval_scale == 0
									&& newSubscriber->runtime_scale == 0 );
			if(result1 != B_OK || result2 != B_OK || result3 != B_OK || noScalesDefined)
			{
				printf("Einsteinium Engine received invalid subscribe message.\n");
				// Attempt to deliver a reply
				status_t result = B_ERROR;
				if(msg->IsSourceWaiting())
					result = msg->SendReply(E_SUBSCRIBE_FAILED);
				else
					printf("Source is not waiting\n");
				if(result != B_OK && newSubscriber->messenger.IsValid())
					newSubscriber->messenger.SendMessage(E_SUBSCRIBE_FAILED);
				else
					printf("Cannot send \'Subscribe Failed\' message.  Result=%i, valid messenger=%i\n", result, newSubscriber->messenger.IsValid());
				delete newSubscriber;
				break;
			}
			_Unsubscribe(newSubscriber->uniqueID);
				// if there already is a subscriber listed with the unique ID remove it
			fSubscribersList.AddItem(newSubscriber);
		//	printf("Sending subscribe confirmed reply\n");
			newSubscriber->messenger.SendMessage(E_SUBSCRIBE_CONFIRMED);
			_CreateAppStatsList(newSubscriber, SORT_BY_SCORE);
			_SendListToSubscriber(newSubscriber);
			break;
		}
		case E_UNSUBSCRIBE_RANKED_APPS: {
		//	printf("Received unsubscribe message\n");
			int32 uniqueID;
			status_t result = msg->FindInt32("uniqueID", &uniqueID);
			if(result!=B_OK)
			{
				printf("Einsteinium Engine received invalid unsubscribe message.\n");
				msg->SendReply(E_UNSUBSCRIBE_FAILED);
				break;
			}
			_Unsubscribe(uniqueID);
			msg->SendReply(E_UNSUBSCRIBE_CONFIRMED);
			break;
		}

		//---- External requests ----//
		// TODO Impliment these
/*		case E_PRINT_RECENT_APPS:
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

			//Send message
			_PopulateAppRankMessage(&appStatsList, &replyMessage, replyCount);
			msg->SendReply(&replyMessage);
			_EmptyAppStatsList(appStatsList);
			break; }*/
/*		case E_RESCAN_DATA_FILES:
		{
			_RescanAllAttrFiles();
			break;
		}*/
		// TODO Make these subscriber specific
//		case E_UPDATE_QUARTILES:
//			_UpdateQuartiles();
//		case E_UPDATE_SCORES:
//			_UpdateAllAttrScores();

		//---- Test Mode ----//
		case E_TEST_MODE_INIT:
		{
			printf("EE Test mode init request received\n");
			BMessage subscribeMsg(E_SUBSCRIBE_RANKED_APPS);
			subscribeMsg.AddInt32("uniqueID", 777);
			subscribeMsg.AddInt16(E_SUBSCRIPTION_COUNT, 20);
			subscribeMsg.AddInt8(E_SUBSCRIPTION_LAST_SCALE, 1);
			subscribeMsg.AddMessenger(E_SUBSCRIPTION_MESSENGER, BMessenger(this));
			BMessenger EsMessenger(e_engine_sig);
			EsMessenger.SendMessage(&subscribeMsg, this);

			break;
		}
		case E_SUBSCRIBE_FAILED:
		{
			printf("EE Test mode init failed!\n");
			break;
		}
		case E_SUBSCRIBE_CONFIRMED:
		{
			printf("EE Test mode confirmed!\n");
			break;
		}
		case E_SUBSCRIBER_UPDATE_RANKED_APPS:
		{
			printf("\nTest mode subscriber updated:\n");
			int32 countFound;
			type_code typeFound;
			msg->GetInfo("refs", &typeFound, &countFound);
//			printf("Found %i refs\n", countFound);
			printf("Highest %i ranked applications:\n", countFound);
			entry_ref newref;
			for(int i=0; i<countFound; i++)
			{
				msg->FindRef("refs", i, &newref);
				printf("%i. %s\n", i+1, newref.name);

			}
			break;
		}
		default:
			BApplication::MessageReceived(msg);
	}
}


void
einsteinium_engine::_Unsubscribe(int32 uniqueID)
{
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
}


void
einsteinium_engine::_SendListToSubscriber(Subscriber *subscriber)
{
	BMessage rankMsg(E_SUBSCRIBER_UPDATE_RANKED_APPS);
	int appsCount = min_c(subscriber->count, subscriber->appStatsList.CountItems());
	_PopulateAppRankMessage(&subscriber->appStatsList, &rankMsg, appsCount);
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

/*
void
//einsteinium_engine::_ForEachAttrFile(int action, BList *appStatsList = NULL)
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
		{//	case UPDATE_ATTR_SCORE: {
		//		_UpdateAttrScore(&attrEntry);
		//		break; }
			case RESCAN_ATTR_DATA: {
				_RescanAttrFile(&attrEntry);
				break; }
			case CREATE_APP_LIST: {
				AppAttrFile attrFile(&attrEntry);
				// check ignore value- skip if ignore flag is true
//				if(attrFile.GetIgnore() == false)
//				{
					AppStats *appStatsData = attrFile.CloneAppStats();
					appStatsList->AddItem(appStatsData);
//					attrEntry.GetPath(&appAttrDirPath);//create path from entry
//					printf("Adding app %s to list\n", appAttrDirPath.Path());//debug info
//				}
				break;
			}
		}
	}
}


void
//einsteinium_engine::_RescanAllAttrFiles()
{
	// TODO only do this if there are no subscribers
	_ForEachAttrFile(RESCAN_ATTR_DATA);
}

void
//einsteinium_engine::_RescanAttrFile(BEntry* entry)
{
	AppAttrFile attrFile(entry);
	attrFile.RescanData();
}*/


bool
einsteinium_engine::_DetermineExclusion(Subscriber *subscriber, const char *signature)
{
	return false;
}


void
einsteinium_engine::_CreateAppStatsList(Subscriber *subscriber, int sortAction=SORT_BY_NONE)
{
	_EmptyAppStatsList(subscriber->appStatsList);

	// TODO what about missing app attribute files?

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
		AppAttrFile attrFile(&attrEntry);
		// check ignore value- skip if ignore flag is true
//		if(attrFile.GetIgnore() == false)
//		{
			AppStats *appStatsData = attrFile.CloneAppStats();
			subscriber->appStatsList.AddItem(appStatsData);
			attrEntry.GetPath(&appAttrDirPath);//create path from entry
			printf("Adding app %s to list\n", appAttrDirPath.Path());//debug info
//		}
	}

	_UpdateQuartiles(subscriber);
	_CalculateScores(subscriber);
	_SortAppStatsList(subscriber, sortAction);
}


void
einsteinium_engine::_SortAppStatsList(Subscriber *subscriber, int sortAction)
{
	// TODO change to return a status_t?
	switch(sortAction)
	{
		case SORT_BY_SCORE:
		{	subscriber->appStatsList.SortItems(AppStatsSortScore);
			break;
		}
		case SORT_BY_LAST_LAUNCHTIME:
		{	subscriber->appStatsList.SortItems(AppStatsSortLastLaunch);
			break;
		}
		case SORT_BY_FIRST_LAUNCHTIME:
		{	subscriber->appStatsList.SortItems(AppStatsSortFirstLaunch);
			break;
		}
		case SORT_BY_LAUNCH_COUNT:
		{	subscriber->appStatsList.SortItems(AppStatsSortLaunchCount);
			break;
		}
		case SORT_BY_LAST_INTERVAL:
		{	subscriber->appStatsList.SortItems(AppStatsSortLastInterval);
			break;
		}
		case SORT_BY_TOTAL_RUNNING_TIME:
		{	subscriber->appStatsList.SortItems(AppStatsSortRunningTime);
			break;
		}
		default:
			return;
	}
	subscriber->appStatsSortOrder = sortAction;
}


void
einsteinium_engine::_EmptyAppStatsList(BList &list)
{
	AppStats* t;
	DeleteList(list, t);
}

uint
einsteinium_engine::_FindAppStatsRank(Subscriber *subscriber, const char* signature)
{
	uint rank = 0-1;
	int count = subscriber->appStatsList.CountItems();
	for(int i=0; i<count; i++)
	{
		AppStats* stats = (AppStats*)(subscriber->appStatsList.ItemAt(i));
		if(stats->app_sig.Compare(signature) == 0)
		{
			rank = i + 1;
			i = count;
		}
	}
	return rank;
}


void
einsteinium_engine::_UpdateQuartiles(Subscriber *subscriber)
{
	//Update last launch time
	printf("Getting quartiles for last launch time\n");
	_SortAppStatsList(subscriber, SORT_BY_LAST_LAUNCHTIME);
	_GetQuartiles(GetStatsLastLaunch, subscriber->appStatsList, subscriber->fQuartiles+Q_LAST_LAUNCH_INDEX);
	//Update first launch time
	printf("Getting quartiles for first launch time\n");
	_SortAppStatsList(subscriber, SORT_BY_FIRST_LAUNCHTIME);
	_GetQuartiles(GetStatsFirstLaunch, subscriber->appStatsList, subscriber->fQuartiles+Q_FIRST_LAUNCH_INDEX);
	//Update last interval
	printf("Getting quartiles for last interval\n");
	// Create a copy of the list and remove apps that have a last interval of 0 so they don't
	// skew the quartiles with lots of 0's
	BList intervalList;
	for(int i=0; i<subscriber->appStatsList.CountItems(); i++)
	{
		AppStats *stats = (AppStats*)(subscriber->appStatsList.ItemAt(i));
		if(stats->last_interval != 0) intervalList.AddItem(stats);
	}
	intervalList.SortItems(AppStatsSortLastInterval);
	_GetQuartiles(GetStatsLastInterval, intervalList, subscriber->fQuartiles+Q_LAST_INTERVAL_INDEX);
	intervalList.MakeEmpty();
	//Update Total Run Time
	printf("Getting quartiles for total running time\n");
	_SortAppStatsList(subscriber, SORT_BY_TOTAL_RUNNING_TIME);
	_GetQuartiles(GetStatsRunningTime, subscriber->appStatsList, subscriber->fQuartiles+Q_TOTAL_RUN_TIME_INDEX);
	//Update Launch Count
	printf("Getting quartiles for launch count\n");
	_SortAppStatsList(subscriber, SORT_BY_LAUNCH_COUNT);
	_GetQuartiles(GetStatsLaunchCount, subscriber->appStatsList, subscriber->fQuartiles+Q_LAUNCHES_INDEX);

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
	itemType qLower, qUpper;
	for(int i=0; i<5; i++)
	{
		index = ((4.0-i)/4.0)*(count-1);
			// index is the location in workingList where the quartile Q0, Q1, Q2, Q3 or Q4
			// falls.  index can be a non integer, in which case the quartile value actually
			// falls in between the values of two items in workingList.
		qUpper = getFunc( (AppStats*)workingList.ItemAt(int(index)) );
			// qUpper is the upper level of the quartile value
		if(i==0 || i==4)
			// For Q0 and Q4 the quartile value is exactly the value of the first and last
			// item respectively
		{	Q[i] = qUpper; }
		else
			// Q1, Q2 and Q3 values usually fall in between item values
		{	qLower = getFunc( (AppStats*)workingList.ItemAt(int(index)+1) );
				// qLower is the value lower than the actual quartile value (remember index 0
				// is the highest value, then the values decrease as the index increases)
			Q[i] = qLower + double(qUpper - qLower)*(index-int(index));
				// double(qUpper - qLower) is the difference between the two values.
				// (index-int(index)) is the fractional place between the the items where the quartile falls
				// Multiply the two and get the fraction of the difference to add to the lower
				// item value to get the final quartile value.
		}
		printf("Q%i (index value %f): %f\n", i, index, Q[i]);
	}
}


void
einsteinium_engine::_CalculateScores(Subscriber *subscriber)
{
	printf("Calculating scores\n");
	int count = subscriber->appStatsList.CountItems();
	for(int i=0; i<count; i++)
	{
		AppStats* stats = (AppStats*)(subscriber->appStatsList.ItemAt(i));
		_CalculateScore(subscriber, stats);
	}
}


void
einsteinium_engine::_CalculateScore(Subscriber *subscriber, AppStats *appStats)
{
	const double *Quart = subscriber->fQuartiles;
	float launch_value = _GetQuartileValue(Quart+Q_LAUNCHES_INDEX, appStats->launch_count);
	float first_launch_value = _GetQuartileValue(Quart+Q_FIRST_LAUNCH_INDEX, appStats->first_launch);
	float last_launch_value = _GetQuartileValue(Quart+Q_LAST_LAUNCH_INDEX, appStats->last_launch);
	float last_interval_value = _GetQuartileValue(Quart+Q_LAST_INTERVAL_INDEX, appStats->last_interval);
	float total_run_time_value = _GetQuartileValue(Quart+Q_TOTAL_RUN_TIME_INDEX, appStats->total_run_time);

	// If last interval is zero (only one launch) put quartile value at .5 so that this
	// statistic does not adversly effect the score
	if(appStats->last_interval == 0)
		last_interval_value = .5;

	int max_scale = 100000;
	appStats->score = int(  max_scale*(launch_value * subscriber->launch_scale)
				+ max_scale*(first_launch_value * subscriber->first_scale)
				+ max_scale*(last_launch_value * subscriber->last_scale)
				+ max_scale*((1 - last_interval_value) * subscriber->interval_scale)
					// Need to reverse interval scale, because longer intervals decrease the score
					// TODO or do I change the sorting method to sort descending??
				+ max_scale*(total_run_time_value * subscriber->runtime_scale));
	printf("Score for %s is %i\n", appStats->app_sig.String(), appStats->score);
}


// Calculate the quartile value of where d lies in the quartile range Q
float
einsteinium_engine::_GetQuartileValue(const double *Q, double d)
{//	printf("EIN: quartiles\n");
//	printf("Q4: %f, Q3: %f, Q2: %f, Q1: %f, Q0: %f\n", Q[4], Q[3], Q[2], Q[1], Q[0]);
	int index, index_offset = 0;
	if(d >= Q[0] && d<Q[1]) { index = 0; }
	else if(d >= Q[1] && d<Q[2]) { index = 1; }
	else if(d >= Q[2] && d<Q[3]) { index = 2; }
	else if(d >= Q[3] && d<Q[4]) { index = 3; }
	else if(d == Q[4]) { return 1; }
	// For values that lie outside the current quartile range
	else if(d < Q[0]) { index = 0; }
	else if(d > Q[4])
	{	index = 4;
		index_offset = 1;
		// since there is no Q[5], index offset will modify the calculation below to
		// use Q[3] and Q[4] to find the quartile value range
	}
	else { return .5; }

	return (.25*index + .25*((d - Q[index])/(Q[index+1-index_offset] - Q[index-index_offset])));
}


template < class itemType >
void
DeleteList(BList& list, itemType* item)
{
	do
	{	item = static_cast<itemType *>(list.RemoveItem(int32(0)));
		if(item) delete item;
	}while(item);
}

