/* AttrSettingsView.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "AttrSettingsView.h"


AttrSettingsView::AttrSettingsView(BRect size)
	:
	BView(size, "Recent Apps Rank", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	SetViewColor(bg_color);
	BRect viewRect;

	// TODO create directory if not found
	find_directory(B_USER_SETTINGS_DIRECTORY, &fAppAttrDirPath);
	fAppAttrDirPath.Append(e_settings_dir);
	fAppAttrDirPath.Append(e_settings_app_dir);
	BEntry dirEntry(fAppAttrDirPath.Path());
	dirEntry.GetNodeRef(&fAttrDirNodeRef);

	// Default Settings
	fDefaultSettingsBox = new BBox("Default Settings BBox");
	fDefaultSettingsBox->SetLabel("When a new application is detected:");
	fPromptRB = new BRadioButton("New App Prompt",
						"Ask me whether to include the application in rank lists",
						new BMessage(EE_LIST_INCLUSION_CHANGED));
	fIncludeRB = new BRadioButton("New App Include",
						"Automatically include the application in rank lists",
						new BMessage(EE_LIST_INCLUSION_CHANGED));
	fIgnoreRB = new BRadioButton("New App Ignore",
						"Automatically do not include the application in rank lists",
						new BMessage(EE_LIST_INCLUSION_CHANGED));
	fDefaultSettingsBox->AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fIncludeRB)
		.Add(fPromptRB)
		.Add(fIgnoreRB)
		.SetInsets(5, 5, 5, 5)
	);

	// Buttons
	fAddB = new BButton("Add", "Add" B_UTF8_ELLIPSIS, new BMessage(EE_ADD_ATTR_FILE));
	fAddB->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fRemoveB = new BButton("Remove", "Remove", new BMessage(EE_REMOVE_ATTR_FILE));
	fRemoveB->SetEnabled(false);

	// Create list for applications
	fAttrLView = new BListView(viewRect, "Attr List View", B_SINGLE_SELECTION_LIST,
								B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fAttrLView->SetSelectionMessage(new BMessage(EE_APPATTR_SELECTION_CHANGED));
	fAttrSView = new BScrollView("Attr List Scroll View", fAttrLView, B_FOLLOW_ALL_SIDES, 0,
								false, true);

	// Inidividual app settings
	fSettingsBox = new BBox("Application Attribute Settings");
	fSettingsBox->SetLabel("Detected Applications");
	fIgnoreCB = new BCheckBox(viewRect, "Ignore", "Do not include this app in ranking lists",
										new BMessage(EE_IGNORE_ATTR_CHANGED));
	fIgnoreCB->SetEnabled(false);
	fSettingsBox->AddChild(BGridLayoutBuilder(5, 5)
		.Add(fAttrSView, 0, 0, 1, 3)
		.Add(fAddB, 1, 0)
		.Add(fRemoveB, 1, 1)
		.Add(fIgnoreCB, 0, 3)
		.SetInsets(5, 5, 5, 5)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.Add(fDefaultSettingsBox)
		.Add(fSettingsBox)
	);

	//File Panel
	fAppFilter = new AppRefFilter();
	fAppsPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL, fAppFilter);

	// Build app list
	_RebuildAttrList();

	// TODO how to watch the settings folder for attributes in files changing to trigger
	// rebuilding the list?

}


AttrSettingsView::~AttrSettingsView()
{
	_StopWatching();
	_EmptyAttrList();
	delete fAppsPanel;
	delete fAppFilter;
}


void
AttrSettingsView::AllAttached()
{
	BView::AllAttached();
	_StartWatching();
}


void
AttrSettingsView::_StartWatching(){
	status_t error = watch_node(&fAttrDirNodeRef, B_WATCH_DIRECTORY, BMessenger(this));
	fWatchingAttributesDir = (error==B_OK);
	printf("Watching attributes folder was %ssuccessful.\n", fWatchingAttributesDir? "": "not ");
	if(!fWatchingAttributesDir)
		printf("Error=%i (B_BAD_VALUE=%i, B_NO_MEMORY=%i, B_ERROR=%i)\n",
			error, B_BAD_VALUE, B_NO_MEMORY, B_ERROR);
}


void
AttrSettingsView::_StopWatching(){
	if(fWatchingAttributesDir)
	{	watch_node(&fAttrDirNodeRef, B_STOP_WATCHING, BMessenger(this));
		printf("Stopped watching engine settings node\n");
		fWatchingAttributesDir = false;
	}
}


void
AttrSettingsView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{
		case EE_LIST_INCLUSION_CHANGED: {
			//Automatically include new applications
			if(fIncludeRB->Value())
				_SaveIncludeSetting(EE_XMLTEXT_VALUE_INCLUDE);

			//prompt whether to include in lists
			else if(fPromptRB->Value())
				_SaveIncludeSetting(EE_XMLTEXT_VALUE_PROMPT);

			//Automatically exclude
			else if(fIgnoreRB->Value())
				_SaveIncludeSetting(EE_XMLTEXT_VALUE_EXCLUDE);
			break;
		}
		case EE_APPATTR_SELECTION_CHANGED: {
			Window()->Lock();
			_ClearItemSettings();
			_UpdateSelectedItem();
			_RecallItemSettings();
			Window()->Unlock();
			break; }
		case EE_ADD_ATTR_FILE: {
			BMessage addmsg(EE_ADD_ATTR_FILE_REF);
			fAppsPanel->SetMessage(&addmsg);
			fAppsPanel->SetTarget(this);
			fAppsPanel->Show();
			break; }
		case EE_ADD_ATTR_FILE_REF: {
			Window()->Lock();
			entry_ref srcRef;
			msg->FindRef("refs", &srcRef);
			BEntry srcEntry(&srcRef, true);
			BNode node(&srcEntry);
			char *buf = new char[B_ATTR_NAME_LENGTH];
			ssize_t size;
			if( (size = node.ReadAttr("BEOS:APP_SIG",0,0,buf,B_ATTR_NAME_LENGTH)) > 0 )
			{
				BString sigString(buf);
				BPath appPath(&srcEntry), appAttrPath(fAppAttrDirPath);
				appAttrPath.Append(appPath.Leaf());
				BFile appAttrFile;
				status_t result = appAttrFile.SetTo(appAttrPath.Path(),
							B_READ_WRITE | B_CREATE_FILE | B_FAIL_IF_EXISTS);
				if(result == B_OK)
				{
					BNodeInfo attrNodeInfo(&appAttrFile);
					attrNodeInfo.SetType(e_app_attr_filetype);//Set MIME type
					appAttrFile.WriteAttrString(ATTR_APP_SIG_NAME, &sigString);
					BString filenameString(appPath.Leaf());
					appAttrFile.WriteAttrString(ATTR_APP_FILENAME_NAME, &filenameString);
					BString appPathString(appPath.Path());
					appAttrFile.WriteAttrString(ATTR_APP_PATH_NAME, &appPathString);
				}
				appAttrFile.Unset();
				if(result == B_OK || result == B_FILE_EXISTS)
				{
					_RebuildAttrList();
					// select the added application's list item
					int count=fAttrLView->CountItems();
					for(int index=0; index<count; index++)
					{
						AppAttrItem* item = (AppAttrItem*)fAttrLView->ItemAt(index);
						if(sigString.Compare(item->fAppSig)==0)
						{
							fAttrLView->Select(index);
							fAttrLView->ScrollToSelection();
							index = count;
						}
					}
				}
			}
			else
				(new BAlert("","Excecutable does not have an application signature","OK"))->Go();
			delete[] buf;
			Window()->Unlock();
			break; }
		case EE_REMOVE_ATTR_FILE: {
			int index = fAttrLView->CurrentSelection();
			if(index<0)
				break;
			AppAttrItem *item = (AppAttrItem*)fAttrLView->ItemAt(index);
			BString str("Are you sure you want to delete all data for application ");
			str.Append(item->fAppName);
			if((new BAlert("",str.String(), "Yes", "No!"))->Go())
				break;
			BPath trashPath;
			find_directory(B_TRASH_DIRECTORY, &trashPath);
			BDirectory trashDir(trashPath.Path());
			BString ent(item->fAttrPath.String());
			BEntry entry(ent.String());
			// TODO move to trash doesn't work if there is already a file of the same name in the trash
			if(entry.InitCheck()==B_OK)
				entry.MoveTo(&trashDir);
			ent.Append(".db");
			entry.SetTo(ent.String());
			if(entry.InitCheck()==B_OK)
				entry.MoveTo(&trashDir);
			fAttrLView->RemoveItem(index);
			// TODO update engine subscribers
			delete item;
			break; }
		case EE_IGNORE_ATTR_CHANGED: {
			_SaveItemSettings();
			break; }
		case B_NODE_MONITOR: {
			//the attributes directory has changed
			int32 opcode;
			if (msg->FindInt32("opcode", &opcode) == B_OK)
			{	switch (opcode)
				{
					case B_ENTRY_CREATED:
					case B_ENTRY_REMOVED:
					case B_ENTRY_MOVED:{
						node_ref nref;
						msg->FindInt32("device", &nref.device);
						msg->FindInt64("directory", &nref.node);
						if(nref == fAttrDirNodeRef)
						{	printf("An attribute file changed, reloading...\n");
							_RebuildAttrList();
						}
						break;
					}
				}
			}
			break;
		}
	}
}


void
AttrSettingsView::SetLinkInclusionDefault(const char* value)
{
	if( strcmp(value,EE_XMLTEXT_VALUE_INCLUDE)==0 )
		fIncludeRB->SetValue(true);

	else if(strcmp(value,EE_XMLTEXT_VALUE_PROMPT)==0 )
		fPromptRB->SetValue(true);

	else if(strcmp(value,EE_XMLTEXT_VALUE_EXCLUDE)==0 )
		fIgnoreRB->SetValue(true);
}

BSize
AttrSettingsView::GetMinSize()
{
	BSize size(B_SIZE_UNSET, B_SIZE_UNSET);
	size.width = fIgnoreRB->MinSize().width + 20;
	size.height = (4 * fIgnoreRB->MinSize().height)
					+ (2 * fAddB->MinSize().height) + 90;
	return size;
}


void
AttrSettingsView::_RebuildAttrList()
{
	_EmptyAttrList();

	BDirectory appAttrDir(fAppAttrDirPath.Path());
	BPath attrPath;
	BEntry attrEntry;
	BNode attrNode;
	BNodeInfo attrNodeInfo;
	attr_info info;
	char nodeType[B_MIME_TYPE_LENGTH];
	BString sig;
	AppAttrItem *item=NULL;
	appAttrDir.Rewind();
	while(appAttrDir.GetNextEntry(&attrEntry) == B_OK)
	{	if( (attrNode.SetTo(&attrEntry)) != B_OK)
			continue;
		if( (attrNodeInfo.SetTo(&attrNode)) != B_OK)
			continue;
		if( (attrNodeInfo.GetType(nodeType)) != B_OK ||
				strcmp(nodeType, e_app_attr_filetype) != 0)
			continue;
		if(attrNode.GetAttrInfo(ATTR_APP_SIG_NAME, &info) != B_OK)
			continue;
		attrNode.ReadAttrString(ATTR_APP_SIG_NAME, &sig);
		attrPath.SetTo(&attrEntry);
		item = new AppAttrItem(sig.String(), attrPath);
		bool E_ignore=false;
		if(attrNode.GetAttrInfo(ATTR_IGNORE_NAME, &info) == B_NO_ERROR)
			attrNode.ReadAttr(ATTR_IGNORE_NAME, B_BOOL_TYPE, 0, &E_ignore, sizeof(E_ignore));
		item->fIgnore = E_ignore;
		fAttrLView->AddItem(item);
		attrNode.Unset();
	}

	// list items alphabetically ignoring case
	fAttrLView->SortItems(SortAppAttrItems);
}


void
AttrSettingsView::_EmptyAttrList()
{
	//Remove List Items
	AppAttrItem *Item;
	int32 count;
	do
	{	Item = (AppAttrItem*)fAttrLView->RemoveItem(int32(0));
		if(Item)
			delete Item;
	}while(Item);
}


void
AttrSettingsView::_SaveChangedItem(AppAttrItem *item)
{
	BNode attrNode(item->fAttrPath.String());
	if(attrNode.InitCheck()!=B_OK)
		// TODO error message?
		return;
	app_info info;
	status_t result = be_roster->GetAppInfo(e_engine_sig, &info);
	if(result==B_OK)
	{
		BMessenger messenger(e_engine_sig);
		BMessage msg(E_SET_IGNORE_ATTR);
		msg.AddString("app_signature", item->fAppSig);
		msg.AddInt32("which", item->fIgnore ? 0 : 1);
		messenger.SendMessage(&msg);
	}
	else
		attrNode.WriteAttr(ATTR_IGNORE_NAME, B_BOOL_TYPE, 0, &(item->fIgnore), sizeof(bool));
}


void
AttrSettingsView::_SaveItemSettings()//save user configureable settings
{
	if(fSelectedItem != NULL)
	{
		bool ignore = fIgnoreCB->Value();
		if(ignore!=fSelectedItem->fIgnore)
		{
			fSelectedItem->fIgnore = ignore;
			_SaveChangedItem(fSelectedItem);
			fAttrLView->InvalidateItem(fAttrLView->IndexOf(fSelectedItem));
		}
	}
}


void
AttrSettingsView::_ClearItemSettings()
{
	fIgnoreCB->SetValue(0);
}


void
AttrSettingsView::_UpdateSelectedItem()
{
	int32 index = fAttrLView->CurrentSelection();
	//No selection
	if(index < 0)
		fSelectedItem = NULL;
	//Item selected
	else
		fSelectedItem = (AppAttrItem*)fAttrLView->ItemAt(index);
}


void
AttrSettingsView::_RecallItemSettings()
{
	//no selected item
	if(fSelectedItem == NULL)
	{
		fIgnoreCB->SetEnabled(false);
		fRemoveB->SetEnabled(false);
	}
	else
	{
		fIgnoreCB->SetEnabled(true);
		fIgnoreCB->SetValue(fSelectedItem->fIgnore);
		fRemoveB->SetEnabled(true);
	}
}


void
AttrSettingsView::_SaveIncludeSetting(const char *value)
{
	EESettingsFile *eeSettings = new EESettingsFile();
	eeSettings->SaveLinkInclusionDefaultValue(value);
	delete eeSettings;
}


AppAttrItem::AppAttrItem(const char* sig, BPath& path)
	:
	BListItem(),
	fIgnore(false),
	fAttrPath(path.Path()),
	fAppSig(sig),
	fAppName(path.Leaf())
	// TODO appName may not be path.Leaf()
{	}


void
AppAttrItem::DrawItem(BView* owner, BRect item_rect, bool complete)
{
	rgb_color color;
	bool selected = IsSelected();
	if(selected)
		color = selected_color;
	else
		color = owner->ViewColor();
	owner->SetLowColor(color);
	owner->SetDrawingMode(B_OP_COPY);
	if(selected || complete)
	{	owner->SetHighColor(color);
		owner->FillRect(item_rect);
	}
	if(!fIgnore)
		owner->SetHighColor(enabled_color);
	else
		owner->SetHighColor(disabled_color);
	BString text(fAppName);
	text.Append(" (").Append(fAppSig).Append(")");
	owner->DrawString(text.String(), BPoint(item_rect.left+5.0,item_rect.bottom - 2.0));
}


int
AppAttrItem::ICompare(AppAttrItem *item)
{
	return fAppName.ICompare(item->fAppName);
}

int
SortAppAttrItems(const void* item1, const void* item2)
{
	AppAttrItem *first = *(AppAttrItem**)item1, *second = *(AppAttrItem**)item2;
	return (first->ICompare(second));
}
