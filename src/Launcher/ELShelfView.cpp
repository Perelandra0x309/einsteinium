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
	// Apparently Haiku does not yet archive tool tips (Release 1 Alpha 2)
	SetToolTip(EL_TOOLTIP_TEXT);
}


ELShelfView::~ELShelfView()
{
	delete fIcon;
	delete fMenu;
//	delete fSettingsFile;
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
			// Look for the einsteinium engine signature
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
			if(strcmp(sig, einsteinium_engine_sig) == 0)
			{
				// Build an empty menu with an item that can be chosen to start the Engine
				_BuildMenu(NULL);
				// Display an alert that the engine is not running
				_CheckEngineStatus(true);
			}
			break;
		}
		case B_SOME_APP_LAUNCHED: {
			// Look for the einsteinium engine signature
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
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
			BMessage goToMessage(EL_GOTO_LAUNCHER_SETTINGS);
			be_roster->Launch("application/x-vnd.Einsteinium_Preferences", &goToMessage);
			break;
		}
		case EL_SHELFVIEW_LAUNCH_REF: {
			entry_ref ref;
			if(msg->FindRef("refs", &ref) == B_OK)
				be_roster->Launch(&ref);
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
	fMenu->SetFont(be_plain_font);

	// Add any refs found
	int32 countFound = 0;
	if(message)
	{
		type_code typeFound;
		message->GetInfo("refs", &typeFound, &countFound);
	//	printf("Found %i refs\n", countFound);
		entry_ref newref;
		for(int i=0; i<countFound; i++)
		{
			message->FindRef("refs", i, &newref);
	//		printf("Found ref %s\n", newref.name);
			BNode refNode(&newref);
			BNodeInfo refNodeInfo(&refNode);
			BMessage *newMsg = new BMessage(EL_SHELFVIEW_LAUNCH_REF);
			newMsg->AddRef("refs", &newref);
			fMenu->AddItem(new IconMenuItem(newref.name, newMsg, &refNodeInfo, B_MINI_ICON));
		}
	}
	// No applications to display- create a helpful message
	bool showStartEngineItem = false;
	if(message==NULL || countFound==0)
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
	fMenu->AddItem(new BMenuItem("Close Launch Menu", new BMessage(EL_SHELFVIEW_MENU_QUIT)));

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
	// Subscribe to the Einsteinium Engine.  If already subscribed, this will update the settings
	fItemCount = fSettingsFile->GetDeskbarCount();
	const int *scales = fSettingsFile->GetScales();
	BMessage exclusionsList = fSettingsFile->GetExclusionsList();
	_SubscribeToEngine(fItemCount, scales[LAUNCH_INDEX], scales[FIRST_INDEX], scales[LAST_INDEX],
						scales[INTERVAL_INDEX], scales[RUNTIME_INDEX], &exclusionsList);
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
