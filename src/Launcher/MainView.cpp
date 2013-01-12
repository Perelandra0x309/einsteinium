/* MainView.cpp
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "MainView.h"


MainView::MainView(BRect size, AppSettings settings)
	:
	BTabView("MainView", B_WIDTH_FROM_LABEL),
	fSelectedListView(NULL),
	fLastRecentDocRef()
{
	fCurrentSettings = settings;
	BRect viewRect(Bounds());
//	viewRect.InsetBy(-2,-2);
//	viewRect.right-=(B_V_SCROLL_BAR_WIDTH+6);
//	viewRect.bottom-=(TabHeight()+6);
	fAppsListView = new AppsListView(viewRect);
	fAppsListView->SetFontSizeForValue(settings.fontSize);
	fAppsScrollView = new BScrollView("Apps", fAppsListView, /*B_FOLLOW_ALL_SIDES, */0, false, true, B_NO_BORDER);

	fDocsListView = new RecentDocsBListView(viewRect);
	fDocsListView->SetFontSizeForValue(settings.fontSize);
	fDocsScrollView = new BScrollView("Files", fDocsListView, /*B_FOLLOW_ALL_SIDES, */0, false, true, B_NO_BORDER);

	fFoldersListView = new RecentFoldersBListView(viewRect);
	fFoldersListView->SetFontSizeForValue(settings.fontSize);
	fFoldersScrollView = new BScrollView("Folders & Queries", fFoldersListView, /*B_FOLLOW_ALL_SIDES, */0, false, true, B_NO_BORDER);

	fAppsTab = new BTab();
	AddTab(fAppsScrollView, fAppsTab);
	fRecentDocsTab = new BTab();
	AddTab(fDocsScrollView, fRecentDocsTab);
	fRecentFoldersTab = new BTab();
	AddTab(fFoldersScrollView, fRecentFoldersTab);

	fTabCount = CountTabs();
}


void
MainView::AllAttached()
{
	BTabView::AllAttached();
	SelectDefaultTab();
	be_app->PostMessage(EL_UPDATE_RECENT_LISTS);
}

/*
void
MainView::AttachedToWindow()
{
	BTabView::AttachedToWindow();
	//be_app->PostMessage(EL_UPDATE_RECENT_LISTS);
}*/

void
MainView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
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
			else if(fSelectedListView==fFoldersListView)
				fFoldersListView->HandleMouseWheelChanged(msg);
			break;
		}
		case EL_UPDATE_RECENT_LISTS:
		{
			_BuildDocsListView();
			_BuildFoldersListView();
			break;
		}
		case EL_UPDATE_RECENT_LISTS_FORCE:
		{
			_BuildDocsListView(true);
			_BuildFoldersListView(true);
			break;
		}
		case B_MODIFIERS_CHANGED:
		{
			// Invalidate the layout to force all ListItem objects to update
			// when a modifier key is pressed or released
			if(fSelectedListView==fAppsListView)
			{
				fAppsListView->Invalidate();
				Window()->UpdateIfNeeded();
			}
			break;
		}
		default:
			BTabView::MessageReceived(msg);
	}
}


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
				if( bytes[0] >= 'A' && bytes[0] <= 'z')
				{
				//	printf("Key: %c\n", bytes[0]);
					if(fSelectedListView==fAppsListView)
						fAppsListView->ScrollToNextAppBeginningWith(bytes[0]);
				}
				else
					BTabView::KeyDown(bytes, numbytes);
			}
		}
	}
	else
	{
		BTabView::KeyDown(bytes, numbytes);
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
		case EL_FOLDER_COUNT_OPTION_CHANGED:
		case EL_QUERY_COUNT_OPTION_CHANGED:
		{
			_BuildFoldersListView(true);
			break;
		}
		default:
			fAppsListView->SettingsChanged(what, settings);
			fDocsListView->SettingsChanged(what, settings);
			fFoldersListView->SettingsChanged(what, settings);
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
			fAppsListView->SendInfoViewUpdate();
			break;
		}
		case 1: {
			fSelectedListView = fDocsListView;
			_BuildDocsListView();
			fDocsListView->SendInfoViewUpdate();
			break;
		}
		case 2: {
			fSelectedListView = fFoldersListView;
			_BuildFoldersListView();
			fFoldersListView->SendInfoViewUpdate();
			break;
		}
	}
	fAppsListView->SetShowing(fSelectedListView==fAppsListView);
	fDocsListView->SetShowing(fSelectedListView==fDocsListView);
	fFoldersListView->SetShowing(fSelectedListView==fFoldersListView);
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
	// TODO
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


void
MainView::_BuildDocsListView(bool force=false)
{
//	Window()->Lock();
	fDocsListView->BuildList(&fCurrentSettings, force);
	MakeFocus();
//	Window()->Unlock();
}

void
MainView::_BuildFoldersListView(bool force=false)
{
//	Window()->Lock();
	fFoldersListView->BuildList(&fCurrentSettings, force);
	MakeFocus();
//	Window()->Unlock();
}

