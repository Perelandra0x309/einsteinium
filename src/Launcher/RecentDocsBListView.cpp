/* RecentDocsBListView.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "RecentDocsBListView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "'Files' tab"

RecentDocsBListView::RecentDocsBListView(BRect size)
	:
	BOutlineListView(size, "Files List", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES),
	fMenu(NULL),
	fGenericSuperItem(NULL),
	fLastRecentDocRef(),
	isShowing(false),
	fWindow(NULL)
{

}


void
RecentDocsBListView::AttachedToWindow()
{
	fWindow = Window();
}


void
RecentDocsBListView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
/*		case B_MOUSE_WHEEL_CHANGED:
		{

		//	printf("Mouse wheel\n");
			// Prevent scrolling while the menu is showing
			if(fMenu && fMenu->IsShowing())
				break;
			// If this list view isn't currently selected redirect message
			// to the main view to handle (fixes bug in Haiku R1A3)
			if(!isShowing)
			{
				msg->what = EL_REDIRECTED_MOUSE_WHEEL_CHANGED;
				Parent()->MessageReceived(msg);
				break;
			}
			HandleMouseWheelChanged(msg);
			break;
		}*/
		case EL_SHOW_IN_TRACKER:
		{
			int32 selectedIndex = CurrentSelection();
			if(selectedIndex < 0)
				break;
			uint32 modifier = modifiers();
			DocListItem *selectedItem = (DocListItem*)ItemAt(selectedIndex);
			selectedItem->ShowInTracker();
			_HideApp(modifier);
			break;
		}
		default:
			BOutlineListView::MessageReceived(msg);
	}
}


void
RecentDocsBListView::MouseDown(BPoint pos)
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
/*	else if ( button & B_TERTIARY_MOUSE_BUTTON )
	{
		// Launch the currently selected item
		_InvokeSelectedItem();
	}*/
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
		// Ignore for super types list items
		BListItem *selectedItem = ItemAt(selectedIndex);
		if(selectedItem->OutlineLevel()==0)
			return;

		_InitPopUpMenu(selectedIndex);

		ConvertToScreen(&pos);
		//fMenu->Go(pos, false, false, true);
		fMenu->Go(pos, true, true, BRect(pos.x - 2, pos.y - 2,
			pos.x + 2, pos.y + 2), true);
	}
}


void
RecentDocsBListView::KeyDown(const char* bytes, int32 numbytes)
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
				{
					BOutlineListView::KeyDown(bytes, numbytes);
				//	Select(currentSelection+1);
				//	ScrollToSelection();
				}
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
					BOutlineListView::KeyDown(bytes, numbytes);
				break;
			}
	/*		case B_RIGHT_ARROW:
			{
				// Fix a bug in R1A4 where the 0 level list items do not
				// always expand when the right arrow is pressed
				int32 currentSelection = CurrentSelection();
				BListItem *selectedItem = ItemAt(currentSelection);
				if((selectedItem->OutlineLevel()==0) && (!selectedItem->IsExpanded()) )
				{
					Expand(selectedItem);
					break;
				}
				BOutlineListView::KeyDown(bytes, numbytes);
				break;
			}*/
			case B_ESCAPE:
			{
				be_app->PostMessage(EL_HIDE_APP);
				break;
			}
			default:
			{
				if( bytes[0] >= 'A' && bytes[0] <= 'z')
					ScrollToNextDocBeginningWith(bytes[0]);
				else
					BOutlineListView::KeyDown(bytes, numbytes);
				
				if (bytes[0] == B_TAB) {
					Window()->PostMessage(EL_LISTVIEW_CHANGED);
					Invalidate();
						// Redraw the selected list item
				}
			}
		}
	}
	else
		BOutlineListView::KeyDown(bytes, numbytes);
}


void
RecentDocsBListView::SelectionChanged()
{
	SendInfoViewUpdate();
}


void
RecentDocsBListView::SendInfoViewUpdate()
{
	BString infoString("");
	int32 currentSelection = CurrentSelection();
	if(currentSelection>=0)
	{
		BListItem *selectedItem = ItemAt(currentSelection);
		// Only populate info for files
		if(selectedItem->OutlineLevel())
		{
			infoString.SetTo(((DocListItem*)selectedItem)->GetPath());
		}
	}
	BMessage infoMsg(EL_UPDATE_INFOVIEW);
	infoMsg.AddString(EL_INFO_STRING, infoString);
	be_app->PostMessage(&infoMsg);
}

/*
void
RecentDocsBListView::HandleMouseWheelChanged(BMessage *msg)
{
	if(msg->what!=B_MOUSE_WHEEL_CHANGED
		&& msg->what!=EL_REDIRECTED_MOUSE_WHEEL_CHANGED)
		return;

	float deltaY=0;
//	printf("Mouse wheel changed\n");
	status_t result = msg->FindFloat("be:wheel_delta_y", &deltaY);
	if(result!=B_OK)
		return;
	if(deltaY>0)
	{
		int32 currentSelection = CurrentSelection();
		if(currentSelection==(CountItems()-1))
			Select(0);
		else
			Select(currentSelection+1);
		ScrollToSelection();
	}
	else
	{
		int32 currentSelection = CurrentSelection();
		if(currentSelection==0)
			Select(CountItems()-1);
		else
			Select(currentSelection-1);
		ScrollToSelection();
	}
}
*/

void
RecentDocsBListView::SettingsChanged(uint32 what)
{
	Window()->Lock();
	int32 currentSelection = CurrentSelection();
	AppSettings* settings = GetAppSettings();
	switch(what)
	{
		case EL_DOC_ICON_OPTION_CHANGED:
		{
			// Update super type items icon size
			int index=0;
			BString nameString;
			void *item;
			while(fSuperListPointers.FindString(EL_SUPERTYPE_NAMES, index, &nameString)==B_OK)
			{
				status_t result = fSuperListPointers.FindPointer(nameString, &item);
				if(result==B_OK)
				{
					((SuperTypeListItem*)item)->SetIconSize(settings->docIconSize);
				}
				index++;
			}
			BuildList(true);
			Select(currentSelection);
			ScrollToSelection();
			break;
		}
		case EL_FONT_OPTION_CHANGED:
		{
			SetFontSizeForValue(settings->fontSize);
			BuildList(true);
			Select(currentSelection);
			ScrollToSelection();
			break;
		}
	}
	Window()->Unlock();
}


void
RecentDocsBListView::SetFontSizeForValue(float fontSize)
{
	if(fontSize==0)
		fontSize = be_plain_font->Size();
	BListView::SetFontSize(fontSize);
}


void
RecentDocsBListView::ScrollToNextDocBeginningWith(char letter)
{
	int32 selectedIndex = CurrentSelection();
	// If no item is selected start at beginning
	if(selectedIndex<0)
		selectedIndex=-1;
	letter = BString(&letter).Truncate(1).ToLower()[0];
	int totalCount = CountItems();
	for(int i=selectedIndex+1; i<totalCount; i++)
	{
		BListItem *selectedItem = ItemAt(i);
		if(selectedItem->OutlineLevel()==0)
			continue;
		DocListItem* item = (DocListItem*)selectedItem;
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
		BListItem *selectedItem = ItemAt(i);
		if(selectedItem->OutlineLevel()==0)
			continue;
		DocListItem* item = (DocListItem*)selectedItem;
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
RecentDocsBListView::BuildList(bool force)
{
	if(!fWindow)
	{
//		printf("No window found\n");
		return;
	}
//	else
//		printf("Found window\n");
	// Check if we need to update list
	BMessage refList;
	if(!force)
	{
		entry_ref recentRef;
		be_roster->GetRecentDocuments(&refList, 1);
		status_t result = refList.FindRef("refs", 0, &recentRef);
		if(result==B_OK && recentRef==fLastRecentDocRef)
		{
			fWindow->UpdateIfNeeded();
			return;
		}
		refList.MakeEmpty();
	}

	AppSettings* settings = GetAppSettings();

	// Remove existing items
	fWindow->Lock();
	while(!IsEmpty())
	{
		BListItem *item = RemoveItem(int32(0));
		// Do not delete level 0 list items, they will be reused to preserve
		// their expanded status
		if(item->OutlineLevel())
			delete item;
	}

	// Get recent documents with a buffer of extra in case there are any that
	// no longer exist
	int fileCount = settings->recentDocCount;
	be_roster->GetRecentDocuments(&refList, 2*fileCount);

	// Add any refs found
	if(!refList.IsEmpty())
	{
		int32 refCount = 0, totalCount = 0;
		type_code typeFound;
		refList.GetInfo("refs", &typeFound, &refCount);
		entry_ref newref;
		BEntry newEntry;
		bool needFirstRecentDoc = true;
		BList docListItems, superListItems;
		BMessage superTypesAdded;

		// Create DocListItems and supertypes
		for(int i=0; i<refCount && totalCount<fileCount; i++)
		{
			refList.FindRef("refs", i, &newref);
		//	printf("Found ref: %s\n", newref.name);
			newEntry.SetTo(&newref);
			if(newEntry.Exists())
			{
				DocListItem *newItem = new DocListItem(&newref, settings);
				if(newItem->InitStatus() == B_OK)
				{
					const char *superTypeName = newItem->GetSuperTypeName();
					if(strcmp(superTypeName, "application")==0)
					{
						BString typeName(newItem->GetTypeName());
						// Ignore queries and query templates
						if( (typeName.ICompare(kQueryType)==0) || (typeName.ICompare(kQueryTemplateType)==0) )
							continue;
						// Ignore applications
						if(typeName.ICompare(kApplicationType)==0)
							continue;

					}

					docListItems.AddItem(newItem);
					bool foundBool;
					if(superTypesAdded.FindBool(superTypeName, &foundBool)!=B_OK)
					{
						void* superItemPointer;
						if(fSuperListPointers.FindPointer(superTypeName, &superItemPointer)==B_OK)
						{
							AddItem((SuperTypeListItem*)superItemPointer);
						}
						else
						{
							SuperTypeListItem *superItem = _GetSuperItem(superTypeName, settings->docIconSize);
							// Change application supertype to display "Other" instead
							if(strcmp(superTypeName, "application")==0)
								superItem->SetName(B_TRANSLATE_COMMENT("Other", "Folder label"));
							fSuperListPointers.AddPointer(superTypeName, superItem);
							fSuperListPointers.AddString(EL_SUPERTYPE_NAMES, superTypeName);
							AddItem(superItem);
							// TODO add pointer to doc item?
						}
						superTypesAdded.AddBool(superTypeName, true);
					}
					totalCount++;
					// Save first recent doc entry
					if(needFirstRecentDoc)
					{
						fLastRecentDocRef = newref;
						needFirstRecentDoc = false;
					}
				}
				else
					delete newItem;
			}
		}

		// Add items to list
		int refreshCount = 20;
		for(int i=docListItems.CountItems()-1; i>=0; i--)
		{
			DocListItem *docItem = (DocListItem*)docListItems.ItemAt(i);
			const char *superTypeName = docItem->GetSuperTypeName();
			void* superItemPointer;
			status_t result = fSuperListPointers.FindPointer(superTypeName, &superItemPointer);
			if(result!=B_OK)
			{
				superItemPointer = _GetGenericSuperItem(settings->docIconSize);
				if(!HasItem((SuperTypeListItem*)superItemPointer))
					AddItem((SuperTypeListItem*)superItemPointer);
			}
			AddUnder(docItem, (SuperTypeListItem*)superItemPointer);
			refreshCount--;
			if(!refreshCount)
			{
				Select(0);
				fWindow->UpdateIfNeeded();
				refreshCount = 20;
			}
		}

		fWindow->UpdateIfNeeded();
		if(!IsEmpty())
		{
			Select(0);
		}
	}

	fWindow->Unlock();
}


SuperTypeListItem*
RecentDocsBListView::_GetSuperItem(const char *mimeString, int docIconSize)
{
/*	BNode node;
	if (node.SetTo(newref) != B_OK)
		return _GetGenericSuperItem(settings);
	BNodeInfo nodeInfo(&node);
	char mimeString[B_MIME_TYPE_LENGTH];
	if(nodeInfo.GetType(mimeString) != B_OK)
		return _GetGenericSuperItem(settings);
	BMimeType nodeType;
	nodeType.SetTo(mimeString);
	BMimeType superType;
	if(nodeType.GetSupertype(&superType)!=B_OK)
		return _GetGenericSuperItem(settings);
	*/
	BMimeType superType(mimeString);
	if(superType.IsValid())
	{
		SuperTypeListItem *item = new SuperTypeListItem(&superType, docIconSize);
		if(item->InitStatus()==B_OK)
		{
//			AddItem(item);
			return item;
		}
	}

	return _GetGenericSuperItem(docIconSize);
}


SuperTypeListItem*
RecentDocsBListView::_GetGenericSuperItem(int docIconSize)
{
	if(fGenericSuperItem==NULL)
	{
		BMimeType genericType("application/octet-stream");
		BMimeType superType;
		if(genericType.GetSupertype(&superType)!=B_OK)
			return NULL;
		fGenericSuperItem = new SuperTypeListItem(&superType, docIconSize);
		if(fGenericSuperItem->InitStatus()!=B_OK)
		{
			delete fGenericSuperItem;
			fGenericSuperItem = NULL;
		}
		else
		{
			fGenericSuperItem->SetName(B_TRANSLATE_COMMENT("Unknown type", "Label for unknown document type"));
//			AddItem(fGenericSuperItem);
		}
	}
	return fGenericSuperItem;
}


status_t
RecentDocsBListView::_InvokeSelectedItem()
{
	status_t result = B_OK;
	int32 selectedIndex = CurrentSelection();
	if(selectedIndex >= 0)
	{
		uint32 modifier = modifiers();
		BListItem *selectedItem = ItemAt(selectedIndex);
		// file or folder
		if(selectedItem->OutlineLevel())
		{
			result = ((DocListItem*)selectedItem)->Launch();
			_HideApp(modifier);
		}
		// super type
		else
		{
			SuperTypeListItem *item = (SuperTypeListItem*)selectedItem;
			if(item->IsExpanded())
				Collapse(item);
			else
				Expand(item);
		}
	}

	return result;
}


void
RecentDocsBListView::_InitPopUpMenu(int32 selectedIndex)
{
	if(fMenu==NULL)
	{
		fMenu = new LPopUpMenu(B_EMPTY_STRING);
		fTrackerMI = new BMenuItem(B_TRANSLATE_COMMENT("Show in Tracker", "Popup menu option"), new BMessage(EL_SHOW_IN_TRACKER));
//		fSettingsMI = new BMenuItem(B_TRANSLATE_COMMENT("Settings" B_UTF8_ELLIPSIS, "Popup menu option"), new BMessage(EL_SHOW_SETTINGS));
		fMenu->AddItem(fTrackerMI);
//		fMenu->AddSeparatorItem();
//		fMenu->AddItem(fSettingsMI);
		fMenu->SetTargetForItems(this);
//		fSettingsMI->SetTarget(be_app);
	}
}


void
RecentDocsBListView::_HideApp(uint32 modifier)
{
	if(!(modifier & kPreventHideModifier) )
		be_app->PostMessage(EL_HIDE_APP);
}
