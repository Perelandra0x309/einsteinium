/* RecentFoldersBListView.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "RecentFoldersBListView.h"

RecentFoldersBListView::RecentFoldersBListView(BRect size)
	:
	RecentDocsBListView(size),
	fFolderSuperListItem(NULL),
	fQuerySuperListItem(NULL)
{
	fQueryTypes[0]=kQueryType;
	fQueryTypes[1]=kQueryTemplateType;
}

void
RecentFoldersBListView::SettingsChanged(uint32 what)
{
	Window()->Lock();
	int32 currentSelection = CurrentSelection();
	AppSettings* settings = GetAppSettings();
	switch(what)
	{
		case EL_DOC_ICON_OPTION_CHANGED:
		{
			// Update super type items icon size
			if(fFolderSuperListItem)
				((SuperTypeListItem*)fFolderSuperListItem)->SetIconSize(settings->docIconSize);
			if(fQuerySuperListItem)
				((SuperTypeListItem*)fQuerySuperListItem)->SetIconSize(settings->docIconSize);
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
RecentFoldersBListView::BuildList(bool force=false)
{
	if(!fWindow)
	{
	//	printf("No window found\n");
		return;
	}
	// Check if we need to update list
	BMessage refList;
	if(!force)
	{
		entry_ref recentRef;
		be_roster->GetRecentFolders(&refList, 1);
		status_t result = refList.FindRef("refs", 0, &recentRef);
		if(result==B_OK && recentRef==fLastRecentFolderRef)
		{
			fWindow->UpdateIfNeeded();
			return;
		}
		refList.MakeEmpty();
	}

	AppSettings* settings = GetAppSettings();

	// Remove existing items
	fWindow->Lock();
	int fullCount = FullListCountItems();
	for(int count=fullCount-1; count>=0; count--)
	{
		BListItem *item = RemoveItem(count);
		// Do not delete level 0 list items, they will be reused to preserve
		// their expanded status
		if(item->OutlineLevel())
			delete item;
	}

	// Create folder super list item
	if(fFolderSuperListItem==NULL)
	{
		BMimeType folderSuperType(kDirectoryType);
		if(folderSuperType.IsValid())
		{
			fFolderSuperListItem = new SuperTypeListItem(&folderSuperType, settings->docIconSize);
			if(fFolderSuperListItem->InitStatus()==B_OK)
				fFolderSuperListItem->SetName("Folders");
			else
				fFolderSuperListItem = NULL;
		}
	}

	// Create query super list item
	if(fQuerySuperListItem==NULL)
	{
		BMimeType querySuperType(kQueryType);
		if(querySuperType.IsValid())
		{
			fQuerySuperListItem = new SuperTypeListItem(&querySuperType, settings->docIconSize);
			if(fQuerySuperListItem->InitStatus()==B_OK)
				fQuerySuperListItem->SetName("Queries");
			else
				fQuerySuperListItem = NULL;
		}
	}

	// Get recent folders with a buffer of extra in case there are any that
	// no longer exist
	int folderCount = settings->recentFolderCount;
	be_roster->GetRecentFolders(&refList, 2*folderCount);

	// Add any refs found
	if(!refList.IsEmpty() && fFolderSuperListItem)
	{
		int32 refCount = 0, totalCount = 0;
		type_code typeFound;
		refList.GetInfo("refs", &typeFound, &refCount);
		entry_ref newref;
		BEntry newEntry;
		bool needFirstRecentFolder = true;

		// Create DocListItems
		AddItem(fFolderSuperListItem);
		for(int i=0; i<refCount && totalCount<folderCount; i++)
		{
			refList.FindRef("refs", i, &newref);
			printf("Found ref: %s\n", newref.name);
			newEntry.SetTo(&newref);
			if(newEntry.Exists())
			{
				DocListItem *newItem = new DocListItem(&newref, settings, 1);
				if(newItem->InitStatus() == B_OK)
				{
					AddItem(newItem);
					totalCount++;
					// Save first recent doc entry
					if(needFirstRecentFolder)
					{
						fLastRecentFolderRef = newref;
						needFirstRecentFolder = false;
					}
				}
				else
					delete newItem;
			}
		}

		fWindow->UpdateIfNeeded();
		if(!FullListIsEmpty())
		{
			Select(0);
		}
	}

	// Get recent queries with a buffer of extra in case there are any that
	// no longer exist
	int queryCount = settings->recentQueryCount;
	be_roster->GetRecentDocuments(&refList, 2*queryCount, fQueryTypes, 2);

	// Add any refs found
	if(!refList.IsEmpty() && fQuerySuperListItem)
	{
		int32 refCount = 0, totalCount = 0;
		type_code typeFound;
		refList.GetInfo("refs", &typeFound, &refCount);
		entry_ref newref;
		BEntry newEntry;
		bool needFirstRecentQuery = true;

		// Create DocListItems
		AddItem(fQuerySuperListItem);
	//	printf("Added query\n");
	//	printf("Querycount=%i,refCount=%i,totalCount=%i\n",queryCount, refCount, totalCount);
		for(int i=0; i<refCount && totalCount<queryCount; i++)
		{
			refList.FindRef("refs", i, &newref);
		//	printf("Found ref: %s\n", newref.name);
			newEntry.SetTo(&newref);
			if(newEntry.Exists())
			{
				DocListItem *newItem = new DocListItem(&newref, settings, 1);
				if(newItem->InitStatus() == B_OK)
				{
					AddItem(newItem);
					totalCount++;
					// Save first recent doc entry
					if(needFirstRecentQuery)
					{
						fLastRecentQueryRef = newref;
						needFirstRecentQuery = false;
					}
				}
				else
					delete newItem;
			}
		}

		fWindow->UpdateIfNeeded();
		if(!FullListIsEmpty())
		{
			Select(0);
		}
	}

	fWindow->Unlock();
}

