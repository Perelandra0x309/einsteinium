/* AppsListView.cpp
 * Copyright 2013-2017 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "AppsListView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "'Apps' list"


AppsListView::AppsListView(BRect size)
	:
	BListView("Apps List"),
	fMenu(NULL),
	fLastRecentAppRef(),
	isShowing(false),
	fWindow(NULL)
{

}


void
AppsListView::AttachedToWindow()
{
	fWindow = Window();
}


void
AppsListView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case EL_START_SERVICE:
		case EL_STOP_SERVICE:
		{
			int32 selectedIndex = CurrentSelection();
			if(selectedIndex < 0)
				return;
			uint32 modifier = modifiers();
			AppListItem *selectedItem = (AppListItem*)ItemAt(selectedIndex);
			selectedItem->ProcessMessage(msg);
			_HideApp(modifier);
			break;
		}
		case EL_SHOW_IN_TRACKER:
		{
			int32 selectedIndex = CurrentSelection();
			if(selectedIndex < 0)
				break;
			uint32 modifier = modifiers();
			AppListItem *selectedItem = (AppListItem*)ItemAt(selectedIndex);
			selectedItem->ShowInTracker();
			_HideApp(modifier);
			break;
		}
		default:
			BListView::MessageReceived(msg);
	}
}


void
AppsListView::MouseDown(BPoint pos)
{
	int32 button=0;
	Looper()->CurrentMessage()->FindInt32("buttons", &button);
	if ( button & B_PRIMARY_MOUSE_BUTTON )
	{
		// Select list item under mouse pointer, then launch item
		int32 index = IndexOf(pos);
		if(index>=0)
		{
			Select(index);
			_InvokeSelectedItem();
		}
	}
	else if (button & B_SECONDARY_MOUSE_BUTTON)
	{
		// Select list item under mouse pointer and show menu
		int32 index = IndexOf(pos);
		if(index < 0)
			return;
		Select(index);
		int32 selectedIndex = CurrentSelection();
		if(selectedIndex < 0)
			return;

		_InitPopUpMenu(selectedIndex);
		ConvertToScreen(&pos);
		fMenu->Go(pos, true, true, BRect(pos.x - 2, pos.y - 2,
			pos.x + 2, pos.y + 2), true);
	}
}


void
AppsListView::KeyDown(const char* bytes, int32 numbytes)
{
	if(numbytes == 1) {
		switch(bytes[0]) {
			case B_RETURN:
			case B_SPACE:
			{
				_InvokeSelectedItem();
				break;
			}
			case B_DOWN_ARROW:
			{
				int32 currentSelection = CurrentSelection();
				if(currentSelection==(CountItems()-1))
				{
					Select(0);
					ScrollToSelection();
				}
				else
					BListView::KeyDown(bytes, numbytes);
				break;
			}
			case B_UP_ARROW:
			{
				int32 currentSelection = CurrentSelection();
				if(currentSelection==0)
				{
					Select(CountItems()-1);
					ScrollToSelection();
				}
				else
					BListView::KeyDown(bytes, numbytes);
				break;
			}
			case B_ESCAPE:
			{
				be_app->PostMessage(EL_HIDE_APP);
				break;
			}
			default:
			{
				if( bytes[0] >= 'A' && bytes[0] <= 'z')
					ScrollToNextAppBeginningWith(bytes[0]);
				else
					BListView::KeyDown(bytes, numbytes);
				
				if (bytes[0] == B_TAB) {
					Window()->PostMessage(EL_LISTVIEW_CHANGED);
					Invalidate();
						// Redraw the selected list item
				}
			}
		}
	}
	else
		BListView::KeyDown(bytes, numbytes);
}


void
AppsListView::Draw(BRect rect)
{
	if(IsEmpty()) {
		float width, height;
		BFont font;
		BString message(B_TRANSLATE_COMMENT("No applications in recent list",
			"Message in empty apps list view"));
		SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		FillRect(rect);
		GetPreferredSize(&width, &height);
		GetFont(&font);
		float messageWidth = font.StringWidth(message.String());
		MovePenTo((width - messageWidth) / 2, (height + font.Size()) / 2);
		SetHighColor(ui_color(B_MENU_SELECTED_BACKGROUND_COLOR));
		DrawString(message.String());
	} else {
		BRect bounds(Bounds());
		bounds.top = ItemFrame(CountItems() - 1).bottom;
		SetHighColor(ui_color(B_LIST_BACKGROUND_COLOR));
		FillRect(bounds);
		BListView::Draw(rect);
	}
}


void
AppsListView::FrameResized(float w, float h)
{
	BListView::FrameResized(w, h);
	Invalidate();
}


void
AppsListView::SettingsChanged(uint32 what)
{
	Window()->Lock();
	int32 currentSelection = CurrentSelection();
	AppSettings* settings = GetAppSettings();
	switch(what)
	{
		case EL_APP_ICON_OPTION_CHANGED:
		{
			int count = CountItems();
			for(int i=0; i<count; i++)
			{
				AppListItem* item = (AppListItem*)(ItemAt(0));
				item->SetIconSize(settings->minIconSize, settings->maxIconSize, count, i);
				RemoveItem(item);
				AddItem(item);
			}
			Select(currentSelection);
			ScrollToSelection();
			break;
		}
		case EL_FONT_OPTION_CHANGED:
		{
			SetFontSizeForValue(settings->fontSize);
			int count = CountItems();
			for(int i=0; i<count; i++)
			{
				AppListItem* item = (AppListItem*)(ItemAt(0));
				RemoveItem(item);
				AddItem(item);
			}
			Select(currentSelection);
			ScrollToSelection();
			break;
		}
	}
	Window()->Unlock();
}


void
AppsListView::SelectionChanged()
{
	SendInfoViewUpdate();
}


void
AppsListView::SendInfoViewUpdate()
{
	BString infoString("");
	int32 currentSelection = CurrentSelection();
	if(currentSelection>=0)
	{
		BListItem *selectedItem = ItemAt(currentSelection);
		infoString.SetTo(((AppListItem*)selectedItem)->GetPath());
	}
	BMessage infoMsg(EL_UPDATE_INFOVIEW);
	infoMsg.AddString(EL_INFO_STRING, infoString);
	be_app->PostMessage(&infoMsg);
}


void
AppsListView::SetFontSizeForValue(float fontSize)
{
	if(fontSize==0)
		fontSize = be_plain_font->Size();
	BListView::SetFontSize(fontSize);
}


void
AppsListView::SetIsShowing(bool _showing)
{
	isShowing = _showing;
}


void
AppsListView::ScrollToNextAppBeginningWith(char letter)
{
	int32 selectedIndex = CurrentSelection();
	// If no item is selected start at beginning
	if(selectedIndex<0)
		selectedIndex=-1;
	letter = BString(&letter).Truncate(1).ToLower()[0];
	int totalCount = CountItems();
	for(int i=selectedIndex+1; i<totalCount; i++)
	{
		AppListItem* item = (AppListItem*)(ItemAt(i));
		bool match = item->BeginsWith(letter);
		if(match)
		{
			Select(i);
			ScrollToSelection();
			return;
		}
	}
	// If we are here, we reached the end of the list without finding
	// a match.  Now start at the beginning of the list
	for(int i=0; i<selectedIndex; i++)
	{
		AppListItem* item = (AppListItem*)(ItemAt(i));
		bool match = item->BeginsWith(letter);
		if(match)
		{
			Select(i);
			ScrollToSelection();
			return;
		}
	}
}


void
AppsListView::BuildAppsListView(BMessage *message)
{
	if(!fWindow)
		return;

	AppSettings* settings = GetAppSettings();

	// Remove existing items
	fWindow->Lock();
	while(!IsEmpty())
	{
		BListItem *item = RemoveItem(int32(0));
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
		int entryCount=0;
		for(int i=0; i<subscriptionRefCount; i++)
		{
			message->FindRef("refs", i, &newref);
	//		printf("Found ref %s\n", newref.name);
			BEntry newEntry(&newref);
			if(newEntry.Exists())
			{
				status_t result = _AddAppListItem(newEntry, subscriptionRefCount, entryCount, settings);
				if(result == B_OK)
					entryCount++;
				else
					printf("Error initializing new list item for %s.\n", newref.name);
			}
		}
		if(!IsEmpty())
			Select(0);
	}
	fWindow->Unlock();
}


void
AppsListView::BuildAppsListFromRecent(bool force)
{
	if(!fWindow)
		return;

	AppSettings* settings = GetAppSettings();
	if(settings->subscribedToEngine)
		return;

	// Check if we need to update list
	BMessage refList;
	if(!force)
	{
		entry_ref recentRef;
		be_roster->GetRecentApps(&refList, 1);
		status_t result = refList.FindRef("refs", 0, &recentRef);
		if(result==B_OK && recentRef==fLastRecentAppRef)
		{
			fWindow->UpdateIfNeeded();
			return;
		}
		refList.MakeEmpty();
	}


	// Remove existing items
	fWindow->Lock();
	while(!IsEmpty())
	{
		BListItem *item = RemoveItem(int32(0));
		delete item;
	}

	// Get recent documents with a buffer of extra in case there are any that
	// no longer exist
	int appCount = settings->appCount;
	be_roster->GetRecentApps(&refList, 2*appCount);
	int32 refCount = 0, totalCount = 0;
	type_code typeFound;
	refList.GetInfo("refs", &typeFound, &refCount);
	entry_ref newref;
	bool needFirstRecentApp = true;
	BList recentAppsList;

	for(int i=0; i<refCount && totalCount<appCount; i++)
	{
		refList.FindRef("refs", i, &newref);
	//		printf("Found ref %s\n", newref.name);
		BFile appFile(&newref, B_READ_ONLY);
		if((appFile.InitCheck())==B_OK)
		{
			char appSignature[B_MIME_TYPE_LENGTH];
			BAppFileInfo appInfo(&appFile);
			if(appInfo.GetSignature(appSignature)==B_OK)
			{
				if(!(settings->exclusionsSignatureList.HasString(appSignature)))
				{
					BEntry *newEntry = new BEntry(&newref);
					recentAppsList.AddItem(newEntry);
					totalCount++;
					// Save first recent app entry
					if(needFirstRecentApp)
					{
						fLastRecentAppRef = newref;
						needFirstRecentApp = false;
					}
				}
			}
		}
	}
	int entryCount=0;
	while(!recentAppsList.IsEmpty())
	{
		BEntry *item = (BEntry*)(recentAppsList.RemoveItem(int32(0)));
		status_t result = _AddAppListItem(*item, totalCount, entryCount, settings);
		delete item;
		if(result==B_OK)
			entryCount++;
	}
	fWindow->UpdateIfNeeded();
	if(!IsEmpty())
		Select(0);
	fWindow->Unlock();
}


status_t
AppsListView::_AddAppListItem(BEntry appEntry, int totalCount, int index, AppSettings *settings)
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
	AppListItem *newItem = new AppListItem(appEntry, sig.String(), settings->maxIconSize);
	newItem->SetIconSize(settings->minIconSize, settings->maxIconSize, totalCount, index);
	status_t initStatus = newItem->InitStatus();
	if(initStatus == B_OK)
		AddItem(newItem);
	else
		delete newItem;

	return initStatus;
}


status_t
AppsListView::_InvokeSelectedItem()
{
	status_t result = B_OK;
	int32 selectedIndex = CurrentSelection();
	if(selectedIndex >= 0)
	{
		uint32 modifier = modifiers();
		if(modifier & kExcludeAppModifier)
		{
			AppListItem *selectedItem = (AppListItem*)ItemAt(selectedIndex);
			BMessage message(EL_ADD_APP_EXCLUSION);
			message.AddString(EL_EXCLUDE_SIGNATURE, selectedItem->GetSignature());
			message.AddRef(EL_EXCLUDE_REF, selectedItem->GetEntryRef());
			be_app->PostMessage(&message);
		}
		else
		{
			AppListItem *selectedItem = (AppListItem*)ItemAt(selectedIndex);
			result = selectedItem->Launch();
			_HideApp(modifier);
		}
	}
	return result;
}


void
AppsListView::_InitPopUpMenu(int32 selectedIndex)
{
	if(fMenu==NULL)
	{
		fMenu = new LPopUpMenu(B_EMPTY_STRING);
		fTrackerMI = new BMenuItem(B_TRANSLATE_COMMENT("Show in Tracker", "Application pop-up menu"),
			new BMessage(EL_SHOW_IN_TRACKER));
		fMenu->AddItem(fTrackerMI);
		// TODO Add 'E' shortcut?
		fRemoveMI = new BMenuItem(B_TRANSLATE_COMMENT("Exclude from Apps list", "Application pop-up menu"),
			new BMessage(EL_ADD_APP_EXCLUSION));
		fMenu->AddItem(fRemoveMI);
		fTrackerMI->SetTarget(this);
		fRemoveMI->SetTarget(be_app);
	}
	if(selectedIndex>=0)
	{
		AppListItem *selectedItem = (AppListItem*)ItemAt(selectedIndex);
		BMessage *message = fRemoveMI->Message();
		message->MakeEmpty();
		message->AddString(EL_EXCLUDE_SIGNATURE, selectedItem->GetSignature());
		message->AddRef(EL_EXCLUDE_REF, selectedItem->GetEntryRef());
	}
}


void
AppsListView::_HideApp(uint32 modifier)
{
	if(!(modifier & kPreventHideModifier) )
		be_app->PostMessage(EL_HIDE_APP);
}
