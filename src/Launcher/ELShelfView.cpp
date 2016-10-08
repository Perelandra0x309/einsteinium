/* ELShelfView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "ELShelfView.h"


ELShelfView::ELShelfView()
	:
	BView(BRect(0, 0, 15, 15), EL_SHELFVIEW_NAME, B_FOLLOW_NONE, B_WILL_DRAW),
	fIcon(NULL),
	fMenu(NULL),
	fItemCount(0),
	fSettingsFile(NULL),
	fEngineAlert(NULL),
	fEngineAlertIsShowing(false),
	fWatchingRoster(false)
{
	app_info info;
	be_app->GetAppInfo(&info);
	fIcon = new BBitmap(Bounds(), B_RGBA32);
	if (fIcon->InitCheck() == B_OK)
	{
		status_t result = BNodeInfo::GetTrackerIcon(&info.ref, fIcon, B_MINI_ICON);
		if(result != B_OK)
		{
			printf("Error getting tracker icon\n");
			delete fIcon;
			fIcon = NULL;
		}
	}
	else
	{
		printf("Error creating bitmap\n");
		delete fIcon;
		fIcon = NULL;
	}

	SetToolTip(EL_TOOLTIP_TEXT);
}


ELShelfView::ELShelfView(BMessage *message)
	:
	BView(message),
	EngineSubscriber(),
	fIcon(NULL),
	fMenu(NULL),
	fItemCount(0),
	fSettingsFile(NULL),
	fEngineAlert(NULL),
	fEngineAlertIsShowing(false),
	fWatchingRoster(false)
{
	BMessage iconArchive;
	status_t result = message->FindMessage("fIconArchive", &iconArchive);
	if(result == B_OK)
		fIcon = new BBitmap(&iconArchive);
	// Apparently Haiku does not yet archive tool tips (Release 1 Alpha 3)
	SetToolTip(EL_TOOLTIP_TEXT);
}


ELShelfView::~ELShelfView()
{
	delete fIcon;
	delete fMenu;
}


void
ELShelfView::AttachedToWindow()
{
	BView::AttachedToWindow();
	if (Parent())
		SetViewColor(Parent()->ViewColor());
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ViewColor());

	_BuildMenu(NULL);

	//Initialize the settings file object and check to see if it instatiated correctly
	fSettingsFile = new LauncherSettingsFile(this);
	status_t result = fSettingsFile->CheckStatus();
	if(result!=B_OK){
		printf("Error creating Einsteinium Launcher settings file.  Cannot continue.\n");
		_Quit();
			// TODO Can we do anything w/o settings?
		return;
	}

	// Display warning if the Engine is not running
	bool engineAutoLaunch = fSettingsFile->GetEngineAutoLaunch();
	bool engineIsRunning = _CheckEngineStatus(!engineAutoLaunch);

	// Subscribe to the Einsteinium Engine
	if(engineIsRunning)
		_Subscribe();
	// Automatically start engine if not running
	else if(engineAutoLaunch)
		_LaunchEngine();

	//Start watching for launches and quits of the Einsteinium Engine
	result = be_roster->StartWatching(BMessenger(this),
							B_REQUEST_QUIT | B_REQUEST_LAUNCHED);
	if(result  != B_OK)
		//roster failed to be watched.  Show warning, but we can continue.
		(new BAlert("Watching Warning", "Warning: This app was not able to succeesfully start "
					"watching the roster for aplication quit and launch messages.  "
					"The status of the Einsteinium Engine may not be up to date at any time.",
					"OK"))->Go();
	else
		//watching was sucessful
		fWatchingRoster = true;

	Invalidate();
}


void
ELShelfView::DetachedFromWindow()
{
	//Need to stop watching for application launches and quits
	if(fWatchingRoster)
	{
		be_roster->StopWatching(BMessenger(this));
		fWatchingRoster = false;
	}

	// If the Engine warning alert is showing, close it
	_CloseEngineAlert();

	// Unsubscribe from the Einsteinium Engine
	_UnsubscribeFromEngine();

	delete fSettingsFile;
	fSettingsFile = NULL;
}


ELShelfView*
ELShelfView::Instantiate(BMessage *data)
{
	if (!validate_instantiation(data, "ELShelfView"))
		return NULL;
	return new ELShelfView(data);
}


status_t
ELShelfView::Archive(BMessage *data, bool deep) const
{
	BView::Archive(data, deep);
	data->AddString("add_on", e_launcher_sig);
//	data->AddString("class", "ELShelfView");
	if(fIcon != NULL)
	{
		BMessage archive;
		fIcon->Archive(&archive);
		data->AddMessage("fIconArchive", &archive);
	}
	return B_NO_ERROR;
}


void
ELShelfView::Draw(BRect rect)
{
	if (fIcon == NULL)
		return;

	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(fIcon);
	SetDrawingMode(B_OP_COPY);
}


void
ELShelfView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case B_SOME_APP_QUIT: {
			// Change the menu if the engine quit
			const char* sig;
			status_t result = msg->FindString("be:signature", &sig);
			if ( result != B_OK)
				break;
			if(strcmp(sig, einsteinium_engine_sig) == 0)
			{
				// Build an empty menu with an item that can be chosen to start the Engine
				_BuildMenu(NULL);
				// Display an alert that the engine is not running if the Einsteinium
				// Daemon isn't running (otherwise defer to the daemon)
				if(!(be_roster->IsRunning(e_daemon_sig)))
					_CheckEngineStatus(true);
			}
			break;
		}
		case B_SOME_APP_LAUNCHED: {
			// Subscribe if the engine has started
			const char* sig;
			status_t result = msg->FindString("be:signature", &sig);
			if ( result != B_OK)
				break;
			if(strcmp(sig, einsteinium_engine_sig) == 0)
			{
				// Renew subscription
				_Subscribe();
				// Close the alert if it is displayed
				_CloseEngineAlert();
			}
			break;
		}
		case EL_CHECK_ENGINE_STATUS: {
			_CheckEngineStatus(true);
			break;
		}
		case EL_START_ENGINE_ALERT: {
			int32 selection;
			msg->FindInt32("which", &selection);
			// If the Yes button was selected, launch the Engine
			if(selection)
				_LaunchEngine();
			fEngineAlert = NULL;
			fEngineAlertIsShowing = false;
			break;
		}
		case EL_START_ENGINE: {
			_LaunchEngine();
			break;
		}
		case EL_SHELFVIEW_OPENPREFS: {
			if(be_roster->IsRunning(e_launcher_sig))
			{
				status_t rc = B_ERROR;
				BMessenger appMessenger(e_launcher_sig, -1, &rc);
				if(!appMessenger.IsValid())
					break;
				appMessenger.SendMessage(EL_SHOW_SETTINGS);
			}
			else
			{
				BMessage goToMessage(EL_SHOW_SETTINGS);
				be_roster->Launch(e_launcher_sig, &goToMessage);
			}
			break;
		}
		case EL_SHELFVIEW_MENUITEM_INVOKED: {
			entry_ref ref;
			status_t result = msg->FindRef("refs", &ref);
			if( result == B_OK)
			{
				// Determine if the user wants to remove the app from the menu
				bool removeApp = true;
				status_t result = msg->FindBool(EL_REMOVE_APPLICATION, &removeApp);
				if(result==B_OK && removeApp==true)
				{
					// Add app to exclusions list
					BEntry refEntry(&ref, true);
					BNode refNode(&refEntry);
					char *buf = new char[B_ATTR_NAME_LENGTH];
					ssize_t size = refNode.ReadAttr("BEOS:APP_SIG",0,0,buf,B_ATTR_NAME_LENGTH);
					if( size > 0 )
					{
						BMessage exclusionsList = fSettingsFile->GetExclusionsList();
						exclusionsList.AddString(EL_EXCLUDE_SIGNATURE, buf);
						exclusionsList.AddString(EL_EXCLUDE_NAME, ref.name);
						fSettingsFile->SaveExclusionsList(exclusionsList);
						// Subscribe with new parameters
						_Subscribe();
					}
				}
				else
					be_roster->Launch(&ref);
			}
			break;
		}
		case EL_SHELFVIEW_MENU_QUIT: {
			_Quit();
			break;
		}
		// Settings file was updated by an external application
		case EL_SETTINGS_FILE_CHANGED_EXTERNALLY: {
			_Subscribe();
			break; }
		default:
			BView::MessageReceived(msg);
	}
}


void
ELShelfView::MouseDown(BPoint pos)
{
	ConvertToScreen(&pos);
	if (fMenu)
		fMenu->Go(pos, true, true, BRect(pos.x - 2, pos.y - 2,
			pos.x + 2, pos.y + 2), true);
}


void
ELShelfView::_BuildMenu(BMessage *message)
{
	delete fMenu;
	fMenu = new BPopUpMenu(B_EMPTY_STRING, false, false);
	//fMenu = new ModifierMenu(B_EMPTY_STRING, false, false);
	fMenu->SetFont(be_plain_font);

	// Add any refs found
	int32 fSubscriptionRefCount = 0;
	if(message)
	{
		BList itemList;
		type_code typeFound;
		message->GetInfo("refs", &typeFound, &fSubscriptionRefCount);
	//	printf("Found %i refs\n", countFound);
		entry_ref newref;
		for(int i=0; i<fSubscriptionRefCount; i++)
		{
			message->FindRef("refs", i, &newref);
		//	printf("Found ref %s\n", newref.name);
			BNode refNode(&newref);
			BNodeInfo refNodeInfo(&refNode);
			BMessage *newMsg = new BMessage(EL_SHELFVIEW_MENUITEM_INVOKED);
			newMsg->AddRef("refs", &newref);
			itemList.AddItem(new IconMenuItem(newref.name, newMsg, &refNodeInfo, B_MINI_ICON));
			//fMenu->AddItem(new IconMenuItem(newref.name, newMsg, &refNodeInfo, B_MINI_ICON));
			//fMenu->AddItem(new ModifierMenuItem(newref.name, newMsg, &refNodeInfo, B_MINI_ICON));
		}
		itemList.SortItems(MenuItemSortLabel);
		fMenu->AddList(&itemList,0);
	}
//	fMenu->SetRefCount(fSubscriptionRefCount);

	// No applications to display- create a helpful message
	bool showStartEngineItem = false;
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
	}

	// Preferences and close menu items
	fMenu->AddSeparatorItem();
	if(showStartEngineItem)
	{
		BMenuItem *menuItemStart = new BMenuItem("Start the Einsteinium Engine",
											new BMessage(EL_START_ENGINE));
		fMenu->AddItem(menuItemStart);
	}
	fMenu->AddItem(new BMenuItem("Preferences"B_UTF8_ELLIPSIS,
		new BMessage(EL_SHELFVIEW_OPENPREFS)));
	fMenu->AddItem(new BMenuItem("Close Deskbar Menu", new BMessage(EL_SHELFVIEW_MENU_QUIT)));

	BMenuItem* item;
	BMessage* msg;
	for (int32 i = fMenu->CountItems(); i-- > 0;) {
		item = fMenu->ItemAt(i);
		if (item && (msg = item->Message()) != NULL)
			item->SetTarget(this);
	}
}


bool
ELShelfView::_CheckEngineStatus(bool showWarning = false)
{
	bool engineIsRunning = _IsEngineRunning();

	// Show the warning if the engine is not running, the applications wants the warning shown,
	// and the warning is not already showing
	if (!engineIsRunning && showWarning && !fEngineAlertIsShowing) {
		fEngineAlert = new BAlert("StartEngine",
			"The Einsteinium Launcher has detected that the Engine is not running.  Would you like to start the Engine?",
			"No", "Yes", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		// Set the feel to FLOATING so that the alert does not block use of the Deskbar while showing
		fEngineAlert->SetFeel(B_FLOATING_ALL_WINDOW_FEEL);
		BMessage *runMessage = new BMessage(EL_START_ENGINE_ALERT);
		fEngineAlert->Go(new BInvoker(runMessage, this));
		fEngineAlertIsShowing = true;
	}
	return engineIsRunning;
}


void
ELShelfView::_CloseEngineAlert()
{
	// If the Engine warning alert is showing, close it
	if(fEngineAlert)
	{
		fEngineAlert->PostMessage(B_QUIT_REQUESTED);
		fEngineAlert = NULL;
		fEngineAlertIsShowing = false;
	}
}


void
ELShelfView::_Quit()
{
	BDeskbar deskbar;
	deskbar.RemoveItem(EL_SHELFVIEW_NAME);
}


void
ELShelfView::_Subscribe()
{
	// Get values from the settings file
	fItemCount = fSettingsFile->GetDeskbarCount();
	const int *scales = fSettingsFile->GetScales();
	BMessage exclusionsList = fSettingsFile->GetExclusionsList();

	// Subscribe to the Einsteinium Engine.  If already subscribed, this will update
	// the values used for subscription messages
	_ResetSubscriberValues();
	_SetCount(fItemCount);
	_SetTotalLaunchesScale(scales[LAUNCH_INDEX]);
	_SetFirstLaunchScale(scales[FIRST_INDEX]);
	_SetLastLaunchScale(scales[LAST_INDEX]);
	_SetLastIntervalScale(scales[INTERVAL_INDEX]);
	_SetTotalRuntimeScale(scales[RUNTIME_INDEX]);
	_SetExcludeList(&exclusionsList);
	_SubscribeToEngine();
}


void
ELShelfView::_SubscribeFailed()
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
ELShelfView::_SubscribeConfirmed()
{
	// Don't need to do anything- just wait for an updated subscription message
}


void
ELShelfView::_UpdateReceived(BMessage *message)
{
	// Rebuild the application launch menu with the list received from the Engine
	_BuildMenu(message);
}


//Sort menu items by their label
int MenuItemSortLabel(const void* item1, const void* item2)
{
	BString label1((*(IconMenuItem**)item1)->Label());//get first label
	BString label2((*(IconMenuItem**)item2)->Label());//get second label
	return label1.ICompare(label2);
}
