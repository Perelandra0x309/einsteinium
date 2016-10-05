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

	//Get paths to ignore
	BPath tempPath;
	if( find_directory(B_SYSTEM_ADDONS_DIRECTORY, &tempPath) == B_OK)
		kSysAddonsPath = tempPath.Path();
	if( find_directory(B_SYSTEM_BIN_DIRECTORY, &tempPath) == B_OK)
		kSysBinPath = tempPath.Path();
	if( find_directory(B_SYSTEM_SERVERS_DIRECTORY, &tempPath) == B_OK)
		kSysServersPath = tempPath.Path();
	if( find_directory(B_SYSTEM_NONPACKAGED_ADDONS_DIRECTORY, &tempPath) == B_OK)
		kSysNonPackagedAddonsPath = tempPath.Path();
	if( find_directory(B_SYSTEM_NONPACKAGED_BIN_DIRECTORY, &tempPath) == B_OK)
		kSysNonPackagedBinPath = tempPath.Path();
	if( find_directory(B_USER_ADDONS_DIRECTORY, &tempPath) == B_OK)
		kUserAddonsPath = tempPath.Path();
	if( find_directory(B_USER_BIN_DIRECTORY, &tempPath) == B_OK)
		kUserBinPath = tempPath.Path();
	if( find_directory(B_USER_NONPACKAGED_ADDONS_DIRECTORY, &tempPath) == B_OK)
		kUserNonPackagedAddonsPath = tempPath.Path();
	if( find_directory(B_USER_NONPACKAGED_BIN_DIRECTORY, &tempPath) == B_OK)
		kUserNonPackagedBinPath = tempPath.Path();
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

	//The engine will not receive quit or launch messages for itself, so simulate one
	BMessage launchedMessage(B_SOME_APP_QUIT);
	app_info appInfo;
	status_t result = GetAppInfo(&appInfo);
	if(result == B_OK)
	{
	//	printf("Sending engine quit message\n");
		launchedMessage.AddRef("be:ref", &(appInfo.ref));
		launchedMessage.AddString("be:signature", appInfo.signature);
		MessageReceived(&launchedMessage);
			// Need to call MessageReceived directly, otherwise this message will be discarded
	}

	// TODO wait for all messages to clear the message queue?

	// Delete subscribers list items
	Subscriber *sub;
	int subscribersCount = fSubscribersList.CountItems();
	for(int i=0; i<subscribersCount; i++)
	{
		// TODO notify subscribers of shutdown?
		sub = (Subscriber*)fSubscribersList.ItemAt(0);
		_DeleteSubscriber(sub);
	}

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

	//The engine will not receive quit or launch messages for itself, so simulate one
	BMessage launchedMessage(B_SOME_APP_LAUNCHED);
	app_info appInfo;
	status_t result = GetAppInfo(&appInfo);
	if(result == B_OK)
	{
	//	printf("Sending engine launch message\n");
		launchedMessage.AddRef("be:ref", &(appInfo.ref));
		launchedMessage.AddString("be:signature", appInfo.signature);
		be_app->PostMessage(&launchedMessage);
	}
}


void
einsteinium_engine::ArgvReceived(int32 argc, char** argv)
{	if(argc>1)
	{	if(strcmp(argv[1],"-q")==0 || strcmp(argv[1],"--quit")==0)//option to quit
		{	PostMessage(B_QUIT_REQUESTED); }
//		else if(strcmp(argv[1], "--rank")==0)//request list of apps printed to terminal
//		{	PostMessage(E_PRINT_RANKING_APPS); }
		// Request list of recently launched apps printed to terminal
		else if(strcmp(argv[1], "--recent")==0)
		{
			if(argc==3)
			{
				BMessage printMsg(E_PRINT_RECENT_APPS);
				int count = atoi(argv[2]);
				if(count>0)
					printMsg.AddInt16(E_SUBSCRIPTION_COUNT, count);
				PostMessage(&printMsg);
			}
			else
				PostMessage(E_PRINT_RECENT_APPS);
		}
		// Have all the data files scanned to recreate the attribute files
		else if(strcmp(argv[1], "--rescanData")==0)
		{	PostMessage(E_RESCAN_DATA_FILES); }
		// Create a subscriber to print ranked list to the terminal
		else if(strcmp(argv[1], "--testMode")==0)
		{	PostMessage(E_TEST_MODE_INIT); }
		//option is not recognized
		else
		{	printf("Usage: einsteinium_engine [option]\n"
					"Options:\n"
					"-q or --quit			Quit\n"
//					"--rank					List apps by overall rank\n"
					"--recent [count]		List the [count] most recent apps (10 if [count] omitted)\n"
					"--rescanData			Rescan database files\n"); }
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
				if(pathString.StartsWith(kSysAddonsPath) ||
					pathString.StartsWith(kSysBinPath) ||
					pathString.StartsWith(kSysNonPackagedAddonsPath) ||
					pathString.StartsWith(kSysNonPackagedBinPath) ||
					pathString.StartsWith(kUserAddonsPath) ||
					pathString.StartsWith(kUserBinPath) ||
					pathString.StartsWith(kUserNonPackagedAddonsPath) ||
					pathString.StartsWith(kUserNonPackagedBinPath) )
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
					currentSub->currentAppIsExcluded = _AppIsExcluded(currentSub, sig);
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
							// Update stats
							AppStats *newStats = appFile.CloneAppStats();
							AppStats *oldStats = (AppStats*)(currentSub->appStatsList.RemoveItem(currentSub->currentAppPreviousRank - 1));
							// TODO the list is already sorted, can we just add item to correct index?
							if(oldStats)
							{
								//insert new stats at the same place as the old stats- may help reduce sorting time
								delete oldStats;
								currentSub->appStatsList.AddItem(newStats, currentSub->currentAppPreviousRank - 1);
							}
							else
							{
								currentSub->appStatsList.AddItem(newStats, 0);
							}
							// Determine if the quartiles need to be recalculated
							// TODO recalculate quartiles if individual quartile value is < -.25 or > 1.25
							currentSub->countSinceQuartilesUpdated++;
							if(currentSub->countSinceQuartilesUpdated > currentSub->recalcQuartilesCount)
							{
								_UpdateQuartiles(currentSub);
								_CalculateScores(currentSub);
							}
							else
								_CalculateScore(currentSub, newStats);

							// Get the new rank
							_SortAppStatsList(currentSub, SORT_BY_SCORE);
							uint currentRank = currentSub->appStatsList.IndexOf(newStats) + 1;
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
			// Check to make sure the message is valid
			int32 uniqueID;
			status_t result1 = msg->FindInt32(E_SUBSCRIPTION_UNIQUEID, &uniqueID);
			BMessenger messenger;
			status_t result2 = msg->FindMessenger(E_SUBSCRIPTION_MESSENGER, &messenger);
			int16 count;
			status_t result3 = msg->FindInt16(E_SUBSCRIPTION_COUNT, &count);
			int8 launchScale, firstScale, lastScale, intervalScale, runtimeScale;
			msg->FindInt8(E_SUBSCRIPTION_LAUNCH_SCALE, &launchScale);
			msg->FindInt8(E_SUBSCRIPTION_FIRST_SCALE, &firstScale);
			msg->FindInt8(E_SUBSCRIPTION_LAST_SCALE, &lastScale);
			msg->FindInt8(E_SUBSCRIPTION_INTERVAL_SCALE, &intervalScale);
			msg->FindInt8(E_SUBSCRIPTION_RUNTIME_SCALE, &runtimeScale);
			bool noScalesDefined = ( launchScale == 0
									&& firstScale == 0
									&& lastScale == 0
									&& intervalScale == 0
									&& runtimeScale == 0 );
			if(result1 != B_OK || result2 != B_OK || result3 != B_OK || noScalesDefined)
			{
				printf("Einsteinium Engine received invalid subscribe message.\n");
				// Attempt to deliver a reply
				status_t result = B_ERROR;
				if(msg->IsSourceWaiting())
					result = msg->SendReply(E_SUBSCRIBE_FAILED);
				else
					printf("Source is not waiting\n");
				if(result != B_OK && messenger.IsValid())
					messenger.SendMessage(E_SUBSCRIBE_FAILED);
				else
					printf("Cannot send \'Subscribe Failed\' message.  Result=%i, valid messenger=%i\n", result, messenger.IsValid());
				break;
			}
			// Get excluded apps
			type_code typeFound;
			int32 countFound;
			status_t exStatus = msg->GetInfo(E_SUBSCRIPTION_EXCLUSIONS, &typeFound, &countFound);
			BList exList;
			BString *appSig;
			if(exStatus==B_OK)
			{
				for(int i=0; i<countFound; i++)
				{
					appSig = new BString();
					msg->FindString(E_SUBSCRIPTION_EXCLUSIONS, i, appSig);
					exList.AddItem(appSig);
				}
			}
			// Determine if subscriber exists to update it or if we create a new one
			Subscriber *subscriber = _FindSubscriber(uniqueID);
			if(subscriber==NULL)
			{
				subscriber = new Subscriber();
				subscriber->uniqueID = uniqueID;
				fSubscribersList.AddItem(subscriber);
			}
			else
				DeleteList(subscriber->appExclusionList, appSig);
			subscriber->messenger = messenger;
			subscriber->count = count;
			subscriber->launch_scale = launchScale;
			subscriber->first_scale = firstScale;
			subscriber->last_scale = lastScale;
			subscriber->interval_scale = intervalScale;
			subscriber->runtime_scale = runtimeScale;
			subscriber->appExclusionList = exList;
			// Send subscription confirmed and updated apps list messages
			subscriber->messenger.SendMessage(E_SUBSCRIBE_CONFIRMED);
			_CreateAppStatsList(subscriber, SORT_BY_SCORE);
			_SendListToSubscriber(subscriber);
			break;
		}
		case E_UNSUBSCRIBE_RANKED_APPS: {
		//	printf("Received unsubscribe message\n");
			int32 uniqueID;
			status_t result = msg->FindInt32(E_SUBSCRIPTION_UNIQUEID, &uniqueID);
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
		// TODO Enable subscriber to force quartile update
//		case E_UPDATE_QUARTILES: {
//			_UpdateQuartiles();
//			break;
//		}

		//---- External requests ----//
//		case E_PRINT_RANKING_APPS:
		case E_PRINT_RECENT_APPS: {
			Subscriber* tempSubscriber = new Subscriber();
			tempSubscriber->launch_scale = 0;
			tempSubscriber->first_scale = 0;
			tempSubscriber->last_scale = 1;
			tempSubscriber->interval_scale = 0;
			tempSubscriber->runtime_scale = 0;
			status_t result = msg->FindInt16(E_SUBSCRIPTION_COUNT, &(tempSubscriber->count));
			if(result != B_OK)
				tempSubscriber->count = 10;
			printf("Sorting list by most recent...\n");
			_CreateAppStatsList(tempSubscriber, SORT_BY_SCORE);
			printf("Ordered list of apps:\n");
			AppStats *stats;
			int appsCount = min_c(tempSubscriber->count, tempSubscriber->appStatsList.CountItems());
			for(int i=0; i<appsCount; i++)
			{	stats = (AppStats*)(tempSubscriber->appStatsList.ItemAt(i));
				printf("%i: %s (%s)\n", i+1, stats->GetSig(), stats->GetFilename() );
			}
			_EmptyAppStatsList(tempSubscriber->appStatsList);
			delete tempSubscriber;
			break; }
		//Send a reply back to another app with the ranked apps list
		case E_REPLY_RANKING_APPS: {
			printf("Received ranking apps request message\n");
			// Create a temporary subscriber- solely for the purpose of creating this one time message
			Subscriber* tempSubscriber = new Subscriber();
			status_t result1 = msg->FindMessenger(E_SUBSCRIPTION_MESSENGER, &(tempSubscriber->messenger));
			tempSubscriber->count = 0;
			status_t result2 = msg->FindInt16(E_SUBSCRIPTION_COUNT, &(tempSubscriber->count));
			// Get the settings for scale values (default to 0 for any not defined)
			tempSubscriber->launch_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_LAUNCH_SCALE, &(tempSubscriber->launch_scale));
			tempSubscriber->first_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_FIRST_SCALE, &(tempSubscriber->first_scale));
			tempSubscriber->last_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_LAST_SCALE, &(tempSubscriber->last_scale));
			tempSubscriber->interval_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_INTERVAL_SCALE, &(tempSubscriber->interval_scale));
			tempSubscriber->runtime_scale = 0;
			msg->FindInt8(E_SUBSCRIPTION_RUNTIME_SCALE, &(tempSubscriber->runtime_scale));
			// There must be at least one scale defined to a non zero value
			bool noScalesDefined = ( tempSubscriber->launch_scale == 0
									&& tempSubscriber->first_scale == 0
									&& tempSubscriber->last_scale == 0
									&& tempSubscriber->interval_scale == 0
									&& tempSubscriber->runtime_scale == 0 );
			if(result1 != B_OK || noScalesDefined)
			{
				printf("Einsteinium Engine received invalid subscribe message.\n");
				// Attempt to deliver a reply
				status_t result = B_ERROR;
				if(msg->IsSourceWaiting())
					result = msg->SendReply(E_SUBSCRIBE_FAILED);
				else
					printf("Source is not waiting\n");
				if(result != B_OK && tempSubscriber->messenger.IsValid())
					tempSubscriber->messenger.SendMessage(E_SUBSCRIBE_FAILED);
				else
					printf("Cannot send \'Subscribe Failed\' message.  Result=%i, valid messenger=%i\n", result, tempSubscriber->messenger.IsValid());
				delete tempSubscriber;
				break;
			}
			_CreateAppStatsList(tempSubscriber, SORT_BY_SCORE);
			// If no count defined, set to the list count to send entire list
			if(tempSubscriber->count == 0)
				tempSubscriber->count = tempSubscriber->appStatsList.CountItems();
			_SendListToSubscriber(tempSubscriber, E_RANKING_APPS_REPLY);
			_EmptyAppStatsList(tempSubscriber->appStatsList);
			delete tempSubscriber;
			break; }
		case E_RESCAN_DATA_FILES:
		{
			_RescanAllAttrFiles();
			break;
		}

		//---- Test Mode ----//
		case E_TEST_MODE_INIT:
		{
			printf("EE Test mode init request received\n");
			BMessage subscribeMsg(E_SUBSCRIBE_RANKED_APPS);
			subscribeMsg.AddInt32(E_SUBSCRIPTION_UNIQUEID, 777);
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
	Subscriber *subscriber = _FindSubscriber(uniqueID);
	_DeleteSubscriber(subscriber);
}


Subscriber*
einsteinium_engine::_FindSubscriber(int32 uniqueID)
{
	int subscribersCount = fSubscribersList.CountItems();
	for(int j=0; j<subscribersCount; j++)
	{
		Subscriber *currentSub = (Subscriber*)fSubscribersList.ItemAt(j);
		if( uniqueID==currentSub->uniqueID )
		{
			return currentSub;
		}
	}
	return NULL;
}


void
einsteinium_engine::_DeleteSubscriber(Subscriber *subscriber)
{
	if(subscriber)
	{
		fSubscribersList.RemoveItem(subscriber);
		_EmptyAppStatsList(subscriber->appStatsList);
		BString* item;
		DeleteList(subscriber->appExclusionList, item);
		delete subscriber;
	}
}


void
einsteinium_engine::_SendListToSubscriber(Subscriber *subscriber, uint32 msgWhat = E_SUBSCRIBER_UPDATE_RANKED_APPS)
{
	BMessage rankMsg(msgWhat);
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
}*/


void
einsteinium_engine::_RescanAllAttrFiles()
{
	// TODO only do this if there are no subscribers

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
		_RescanAttrFile(&attrEntry);
	}
}


void
einsteinium_engine::_RescanAttrFile(BEntry* entry)
{
	AppAttrFile attrFile(entry);
	attrFile.RescanData();
}


bool
einsteinium_engine::_AppIsExcluded(Subscriber *subscriber, const char *signature)
{
	int count = subscriber->appExclusionList.CountItems();
	BString *appSig;
	for(int i=0; i<count; i++)
	{
		appSig = (BString*)subscriber->appExclusionList.ItemAt(i);
		if(appSig->Compare(signature)==0)
			return true;
	}
	return false;
}


void
einsteinium_engine::_CreateAppStatsList(Subscriber *subscriber, int sortAction=SORT_BY_NONE)
{
	_EmptyAppStatsList(subscriber->appStatsList);

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
		// check exclusion
		if(!_AppIsExcluded(subscriber, attrFile.GetSig()))
		{
			AppStats *appStatsData = attrFile.CloneAppStats();
			subscriber->appStatsList.AddItem(appStatsData);
			attrEntry.GetPath(&appAttrDirPath);//create path from entry
			printf("Adding app %s to list\n", appAttrDirPath.Path());//debug info
		}
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
	// Reset the count and calculate what the count should be to recalculate quartiles
	subscriber->countSinceQuartilesUpdated = 0;
	subscriber->recalcQuartilesCount = 2*int(sqrt(subscriber->appStatsList.CountItems()));
//s	printf("Count to recalculate quartiles=%i\n", subscriber->recalcQuartilesCount);
}


template < class itemType >
void
einsteinium_engine::_GetQuartiles(itemType (*getFunc)(AppStats*), BList &workingList,
						double *Q)
{
	int count = workingList.CountItems();
	if(count<5)
	{
		// No data if count is zero
		if(count==0)
			return;
		// There are too few items to do quartiles- simulate quartiles by creating evenly spaced
		// quartile values between the highest and lowest values
		Q[4] = getFunc( (AppStats*)workingList.ItemAt(0) );
		Q[0] = getFunc( (AppStats*)workingList.ItemAt(count-1) );
		double delta = Q[4] - Q[0];
		Q[1] = Q[0] + delta/4.0;
		Q[2] = Q[0] + delta/2.0;
		Q[3] = Q[0] + delta*3.0/4.0;
//		printf("Q0=%f\nQ1=%f\nQ2=%f\nQ3=%f\nQ4=%f\n", Q[0], Q[1], Q[2], Q[3], Q[4]);
		return;
	}
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
//		printf("Q%i (index value %f): %f\n", i, index, Q[i]);
	}
}


void
einsteinium_engine::_CalculateScores(Subscriber *subscriber)
{
//	printf("Calculating scores\n");
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
	appStats->score = int( (max_scale * launch_value * subscriber->launch_scale)
				+ (max_scale * first_launch_value * subscriber->first_scale)
				+ (max_scale * last_launch_value * subscriber->last_scale)
				+ (max_scale * (1 - last_interval_value) * subscriber->interval_scale)
					// Need to reverse interval scale, because longer intervals decrease the score
				+ (max_scale * total_run_time_value * subscriber->runtime_scale) );
/*	// Big debug string generator since printf does seem to work well referencing values inside objects
	printf("--Score for %s is %i--\n", appStats->app_sig.String(), appStats->score);
	BString test("Values: launch_value=");
	test<<launch_value;
	test.Append(" * launch_scale=");
	int launch_scale = subscriber->launch_scale;
	test<<launch_scale;
	test.Append("\n+ first_value=");
	test<<first_launch_value;
	test.Append(" * first_scale=");
	int first_scale = subscriber->first_scale;
	test<<first_scale;
	test.Append("\n+ last_value=");
	test<<last_launch_value;
	test.Append(" *  last_scale=");
	int last_scale = subscriber->last_scale;
	test<<last_scale;
	test.Append("\n+ interval_value=");
	test<<last_interval_value;
	test.Append(" * interval_scale=");
	int interval_scale = subscriber->interval_scale;
	test<<interval_scale;
	test.Append("\n+ runtime_value=");
	test<<total_run_time_value;
	test.Append(" * runtime_scale=");
	int runtime_scale = subscriber->runtime_scale;
	test<<runtime_scale;
	test.Append("\n");
	printf("%s", test.String());*/
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
	// For values that lie outside the current quartile range
	else if(d < Q[0]) { index = 0; }
	else if(d >= Q[4])
	{	index = 4;
		index_offset = 1;
		// since there is no Q[5], index offset will modify the calculation below to
		// use Q[3] and Q[4] to find the quartile value range
	}
	else { return .5; }

	float value = .25*index;
	// Avoid divide by zero problems when both quartile values are the same
	if(Q[index+1-index_offset] != Q[index-index_offset])
		value += .25*((d - Q[index])/(Q[index+1-index_offset] - Q[index-index_offset]));
	return value;
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

