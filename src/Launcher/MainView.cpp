/* MainView.cpp
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "MainView.h"

//inline int32 _get_object_token_(const BHandler* object)
//{	return object->fToken; }

MainView::MainView(BRect size, AppSettings settings)
	:
	BTabView("Services", B_WIDTH_FROM_LABEL),
//	EngineSubscriber(),
//	fWatchingRoster(false),
//	fSettingsFile(NULL),
	fSelectedListView(NULL),
	fLastRecentDocRef()
{
	fCurrentSettings = settings;

	fAppsListView = new AppsListView(size);
	fAppsListView->SetFontSizeForValue(settings.fontSize);
	fAppsScrollView = new BScrollView("Apps", fAppsListView, 0, false, true);

	fDocsListView = new RecentDocsBListView(size);
	fDocsListView->SetFontSizeForValue(settings.fontSize);
	fDocsScrollView = new BScrollView("Recent Files", fDocsListView, 0, false, true);

	fAppsTab = new BTab();
	AddTab(fAppsScrollView, fAppsTab);
	fRecentDocsTab = new BTab();
	AddTab(fDocsScrollView, fRecentDocsTab);

	fTabCount = CountTabs();
}

/*
MainView::~MainView()
{

}*/


void
MainView::AttachedToWindow()
{
	BTabView::AttachedToWindow();

	fSelectedListView = fAppsListView;

	//Start watching the application roster for launches and quits of services
/*	status_t result = be_roster->StartWatching(BMessenger(this));
	if(result  != B_OK)
	{	//roster failed to be watched.  Show warning.
		(new BAlert("Watching Warning", "Warning: This app was not able to succeesfully start "
					"watching the roster for aplication quit and launch messages.  "
					"The status of the system services may not be up to date at any time.", "OK"))->Go();
	}
	else//watching was sucessful
		fWatchingRoster = true;
*/
	//Initialize the settings file object and check to see if it instatiated correctly
/*	fSettingsFile = new LauncherSettingsFile(this);
	status_t result = fSettingsFile->CheckStatus();
	if(result!=B_OK){
		printf("Error creating Einsteinium Launcher settings file.  Cannot continue.\n");
//		_Quit();
			// TODO Can we do anything w/o settings?
		return;
	}

	_Subscribe();
	_BuildDocsListView();*/
	SelectDefaultTab();
//	MakeFocus();
}


void
MainView::DetachedFromWindow()
{
	//Need to stop watching the roster
//	if(fWatchingRoster)
//		be_roster->StopWatching(BMessenger(this));

/*	// Unsubscribe from the Einsteinium Engine
	_UnsubscribeFromEngine();

	delete fSettingsFile;
	fSettingsFile = NULL;*/

	BTabView::DetachedFromWindow();
}


void
MainView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
/*		case B_SOME_APP_QUIT:
		case B_SOME_APP_LAUNCHED:
		{
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK)
				break;
			ServiceListItem *item = NULL;
			int count = fListView->CountItems();
			for(int index=0; index<count; index++)
			{
				item = static_cast<ServiceListItem *>(fListView->ItemAt(index));
				if(strcmp(sig,item->GetSignature()) == 0)
				{
					item->ProcessMessage(msg);
					fListView->InvalidateItem(index);
				}
			}
			break;
		}*/
		// Catch mouse wheel events that are redirected from a listview that
		// is not showing and send it to the correct list view
		case EL_REDIRECTED_MOUSE_WHEEL_CHANGED:
		case B_MOUSE_WHEEL_CHANGED:
		{
		//	printf("Redirected mouse wheel changed message\n");
			if(fSelectedListView==fAppsListView)
				fAppsListView->HandleMouseWheelChanged(msg);
			else if(fSelectedListView==fDocsListView)
				fDocsListView->HandleMouseWheelChanged(msg);
			break;
		}
/*		// Settings file was updated by an external application
		case EL_SETTINGS_FILE_CHANGED_EXTERNALLY:
		{
			_Subscribe();
			_BuildDocsListView(true);
			break;
		}*/
		case EL_UPDATE_RECENT_DOCS:
		{
		//	if(fSelectedListView == fDocsListView)
				_BuildDocsListView();
			break;
		}
		case EL_UPDATE_RECENT_DOCS_FORCE:
		{
			_BuildDocsListView(true);
			break;
		}
		case B_MODIFIERS_CHANGED:
		{
			// Invalidate the layout to force all ListItem objects to update
			// when a modifier key is pressed or released
			if(fSelectedListView==fAppsListView)
			{
				//printf("Modifiers changed\n");
				fAppsListView->Invalidate();
				Window()->UpdateIfNeeded();
			}
			break;
		}
/*		case ADD_APP_EXCLUSION:
		{
			fSettingsFile->AddToExclusionsList(msg);
			_Subscribe();
			break;
		}*/
		default:
			BTabView::MessageReceived(msg);
	}
}

/*
void
MainView::MouseDown(BPoint pos)
{
	int32 button=0;
	Looper()->CurrentMessage()->FindInt32("buttons", &button);
	if (button & B_TERTIARY_MOUSE_BUTTON)
		fSelectedListView->MouseDown(pos);
	else
		BTabView::MouseDown(pos);
}*/


void
MainView::KeyDown(const char* bytes, int32 numbytes)
{
	if(numbytes == 1) {
		switch(bytes[0]) {
			case B_TAB:
			{
				int32 selection = Selection();
				Select((selection+1) % fTabCount);
				break;
			}
			case B_ESCAPE:
			{
				be_app->PostMessage(EL_HIDE_APP);
				break;
			}
	/*		case B_RETURN:
			case B_SPACE:
			{
				int32 selectedIndex = fSelectedListView->CurrentSelection();
				if(selectedIndex >= 0)
				{
					uint32 modifier = modifiers();
					// Send exclusion message when holding down the shift modifier key
					if( (modifier & B_SHIFT_KEY) && (fSelectedListView==fAppsListView) )
					{
						AppListItem *item = (AppListItem*)fAppsListView->ItemAt(selectedIndex);
						fSettingsFile->AddToExclusionsList(item->GetSignature(), item->GetName());
						_Subscribe();
					}
					else
					{
						fSelectedListView->KeyDown(bytes, numbytes);
					}

				}

				break;
			}*/
			case B_RETURN:
			case B_SPACE:
			case B_DOWN_ARROW:
			case B_UP_ARROW:
			case B_LEFT_ARROW:
			case B_RIGHT_ARROW:
			{
				fSelectedListView->KeyDown(bytes, numbytes);
				break;
			}
			default:
			{
				BTabView::KeyDown(bytes, numbytes);
			}
		}
	}
	else
	{
		BTabView::KeyDown(bytes, numbytes);
	//	fSelectedListView->KeyDown(bytes, numbytes);
	}
}


void
MainView::Select(int32 tab)
{
	BTabView::Select(tab);
	_UpdateSelectedListView();
}


void
MainView::SelectDefaultTab()
{
	Select(0);
}


void
MainView::SettingsChanged(uint32 what, AppSettings settings)
{
	fCurrentSettings = settings;
	switch(what)
	{
		case EL_DOC_COUNT_OPTION_CHANGED:
		{
			_BuildDocsListView(true);
			break;
		}
		default:
			fAppsListView->SettingsChanged(what, settings);
			fDocsListView->SettingsChanged(what, settings);
	}

}


void
MainView::_UpdateSelectedListView()
{
	int32 selection = Selection();
	switch(selection) {
		case 0: {
			Window()->UpdateIfNeeded();
			fSelectedListView = fAppsListView;
			break;
		}
		case 1: {
			fSelectedListView = fDocsListView;
			_BuildDocsListView();
			break;
		}
	}
	fAppsListView->SetShowing(fSelectedListView==fAppsListView);
	fDocsListView->SetShowing(fSelectedListView==fDocsListView);
}


status_t
MainView::_AddAppListItem(BEntry appEntry, int totalCount, int index)
{
	BNode serviceNode;
	BNodeInfo serviceNodeInfo;
	char nodeType[B_MIME_TYPE_LENGTH];
	attr_info info;
	BString sig;
	if( (serviceNode.SetTo(&appEntry)) != B_OK)
		return B_ERROR;
	if( (serviceNodeInfo.SetTo(&serviceNode)) != B_OK)
		return B_ERROR;
	if( (serviceNodeInfo.GetType(nodeType)) != B_OK)
		return B_ERROR;
	if( strcmp(nodeType, "application/x-vnd.Be-elfexecutable") != 0 &&
		strcmp(nodeType, "application/x-vnd.be-elfexecutable") != 0	)
		return B_ERROR;
	if(serviceNode.GetAttrInfo("BEOS:APP_SIG", &info) != B_OK)
		return B_ERROR;
	serviceNode.ReadAttrString("BEOS:APP_SIG", &sig);
	AppListItem *newItem = new AppListItem(appEntry, sig.String(), &fCurrentSettings);
	newItem->SetIconSize(fCurrentSettings.minIconSize, fCurrentSettings.maxIconSize,
			totalCount, index);
	status_t initStatus = newItem->InitStatus();
	if(initStatus == B_OK)
		fAppsListView->AddItem(newItem);
	else
		delete newItem;

	return initStatus;
}


void
MainView::BuildAppsListView(BMessage *message)
{
	// Remove existing items
	Window()->Lock();
	while(!fAppsListView->IsEmpty())
	{
		BListItem *item = fAppsListView->RemoveItem(int32(0));
		delete item;
	}

	// Add any refs found
	int32 subscriptionRefCount = 0;
	if(message)
	{
		type_code typeFound;
		message->GetInfo("refs", &typeFound, &subscriptionRefCount);
	//	printf("Found %i refs\n", countFound);
		entry_ref newref;
		for(int i=0; i<subscriptionRefCount; i++)
		{

			message->FindRef("refs", i, &newref);
	//		printf("Found ref %s\n", newref.name);
			BEntry newEntry(&newref);
			if(newEntry.Exists())
			{
			/*	if(i==0)
				{
					_AddAppListItem(newEntry, subscriptionRefCount, i);
					fAppsListView->RemoveItem(0l);
				}*/
				status_t result = _AddAppListItem(newEntry, subscriptionRefCount, i);
				if(result != B_OK)
					printf("Error initializing new list item for %s.\n", newref.name);
			}
		}
		if(!fAppsListView->IsEmpty())
		{
			fAppsListView->Select(0);

		}
	}
	MakeFocus();

	// No applications to display- create a helpful message
/*	bool showStartEngineItem = false;
	if(message==NULL || fSubscriptionRefCount==0)
	{
		// The engine is not running
		if(!_IsEngineRunning())
		{
			BMenuItem *menuItem1 = new BMenuItem("Warning:", NULL);
			BMenuItem *menuItem2 = new BMenuItem("Einsteinium Engine is not running.", NULL);
			BMenuItem *menuItem3 = new BMenuItem("Please start the Engine to populate", NULL);
			BMenuItem *menuItem4 = new BMenuItem("this menu.", NULL);
			menuItem1->SetEnabled(false);
			menuItem2->SetEnabled(false);
			menuItem3->SetEnabled(false);
			menuItem4->SetEnabled(false);
			fMenu->AddItem(menuItem1);
			fMenu->AddItem(menuItem2);
			fMenu->AddItem(menuItem3);
			fMenu->AddItem(menuItem4);
			showStartEngineItem = true;
		}
		// The engine is running but there were no applications in the subscription message
		else
		{
			BMenuItem *menuItem1 = new BMenuItem("There are no applications to list.", NULL);
			menuItem1->SetEnabled(false);
			fMenu->AddItem(menuItem1);
		}
	}*/
	Window()->Unlock();
}

/*
void
MainView::_Subscribe()
{
	// Get values from the settings file
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
MainView::_SubscribeFailed()
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
MainView::_SubscribeConfirmed()
{
	// Don't need to do anything- just wait for an updated subscription message
//	printf("Subscription confirmed.\n");

}


void
MainView::_UpdateReceived(BMessage *message)
{
	// Rebuild the application selection view with the list received from the Engine
//	printf("Received update from Engine.\n");
	_BuildAppsListView(message);
}
*/

void
MainView::_BuildDocsListView(bool force=false)
{
	fDocsListView->BuildList(&fCurrentSettings, force);
	MakeFocus();
/*	// Check if we need to update list
	BMessage refList;
	if(!force)
	{
		entry_ref recentRef;
		be_roster->GetRecentDocuments(&refList, 1);
		status_t result = refList.FindRef("refs", 0, &recentRef);
		if(result==B_OK && recentRef==fLastRecentDocRef)
		{
			Window()->UpdateIfNeeded();
			return;
		}
		refList.MakeEmpty();
	}

	// Remove existing items
	Window()->Lock();
	int32 itemCount = fDocsListView->CountItems();
	if(itemCount)
	{
		const BListItem** itemPtr = fDocsListView->Items();
		fDocsListView->MakeEmpty();
		Window()->UpdateIfNeeded();
		for(int32 i = itemCount; i>0; i--)
		{
			delete *itemPtr;
			*itemPtr++;
		}
	}


//	const char* array[] = { "text/plain", "text/x-source-code"};
//	be_roster->GetRecentDocuments(&refList, 100, array, 2);
	// Get recent documents with a buffer of extra in case there are any that
	// no longer exist
	be_roster->GetRecentDocuments(&refList, 2*fCurrentSettings.recentDocCount);

	// Add any refs found
//	BList itemList;
	if(!refList.IsEmpty())
	{
		int32 refCount = 0, totalCount = 0;
		type_code typeFound;
		refList.GetInfo("refs", &typeFound, &refCount);
		entry_ref newref;
		BEntry newEntry;
		int refreshCount = 20;
		bool needFirstRecentDoc = true;
		for(int i=0; i<refCount && totalCount<fCurrentSettings.recentDocCount; i++)
		{
			refList.FindRef("refs", i, &newref);
		//	printf("Found ref: %s\n", newref.name);
			newEntry.SetTo(&newref);
			if(newEntry.Exists())
			{
				totalCount++;
				// Save first recent doc entry
				if(needFirstRecentDoc)
				{
					fLastRecentDocRef = newref;
					needFirstRecentDoc = false;
				}

				// Add doc to recent list
		//		DocListItem *newItem = new DocListItem(&newref, &fCurrentSettings);
		//		if(newItem->InitStatus() == B_OK)
		//		{
				//	fDocsListView->AddItem(newItem);

		//			SuperTypeListItem *superItem = _GetSuperItem(newref);
		//			if(superItem==NULL)
		//				superItem = _GetGenericSuperItem();
		//			fDocsListView->AddUnder(newItem, superItem);

					bool result = fDocsListView->AddDoc(&newref, &fCurrentSettings);

					if(result)
						refreshCount--;
					if(!refreshCount)
					{
						fDocsListView->Select(0);
						Window()->UpdateIfNeeded();
						refreshCount = 20;
					}
		//		}
		//		else
		//			delete newItem;
			}
		}
		Window()->UpdateIfNeeded();
		if(!fDocsListView->IsEmpty())
		{
			fDocsListView->Select(0);
		}
	}
//	MakeFocus();
	*/
	// No applications to display- create a helpful message
/*	bool showStartEngineItem = false;
	if(message==NULL || fSubscriptionRefCount==0)
	{
		// The engine is not running
		if(!_IsEngineRunning())
		{
			BMenuItem *menuItem1 = new BMenuItem("Warning:", NULL);
			BMenuItem *menuItem2 = new BMenuItem("Einsteinium Engine is not running.", NULL);
			BMenuItem *menuItem3 = new BMenuItem("Please start the Engine to populate", NULL);
			BMenuItem *menuItem4 = new BMenuItem("this menu.", NULL);
			menuItem1->SetEnabled(false);
			menuItem2->SetEnabled(false);
			menuItem3->SetEnabled(false);
			menuItem4->SetEnabled(false);
			fMenu->AddItem(menuItem1);
			fMenu->AddItem(menuItem2);
			fMenu->AddItem(menuItem3);
			fMenu->AddItem(menuItem4);
			showStartEngineItem = true;
		}
		// The engine is running but there were no applications in the subscription message
		else
		{
			BMenuItem *menuItem1 = new BMenuItem("There are no applications to list.", NULL);
			menuItem1->SetEnabled(false);
			fMenu->AddItem(menuItem1);
		}
	}*/
//	Window()->Unlock();

//	Invalidate();
//	InvalidateLayout();
}
