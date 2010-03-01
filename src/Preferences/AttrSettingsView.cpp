/*AttrSettingsView.cpp

*/
#include "AttrSettingsView.h"

AttrSettingsView::AttrSettingsView(BRect size)
	:BView(size, "Recent Apps Rank", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{	SetViewColor(bg_color);
	BRect viewRect;

	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append(e_settings_dir);

	// Default Settings
	defaultSettingsBox = new BBox("Default Settings BBox");
	defaultSettingsBox->SetLabel("When a new application is detected:");
	promptRB = new BRadioButton("New App Prompt",
						"Ask me whether to include the application in rank lists",
						new BMessage(EE_LIST_INCLUSION_CHANGED));
	includeRB = new BRadioButton("New App Include",
						"Automatically include the application in rank lists",
						new BMessage(EE_LIST_INCLUSION_CHANGED));
	ignoreRB = new BRadioButton("New App Ignore",
						"Automatically do not include the application in rank lists",
						new BMessage(EE_LIST_INCLUSION_CHANGED));
	defaultSettingsBox->AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(includeRB)
		.Add(promptRB)
		.Add(ignoreRB)
		.SetInsets(5, 5, 5, 5)
	);

	// Buttons
	addB = new BButton("Add", "Add" B_UTF8_ELLIPSIS, new BMessage(EE_ADD_ATTR_FILE));
	addB->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	removeB = new BButton("Remove", "Remove", new BMessage(EE_REMOVE_ATTR_FILE));
	removeB->SetEnabled(false);

	// Create list for applications
	attrLView = new BListView(viewRect, "Attr List View", B_SINGLE_SELECTION_LIST,
								B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	attrLView->SetSelectionMessage(new BMessage(EE_APPATTR_SELECTION_CHANGED));
	attrSView = new BScrollView("Attr List Scroll View", attrLView, B_FOLLOW_ALL_SIDES, 0,
								false, true);

	// Inidividual app settings
	settingsBox = new BBox("Application Attribute Settings");
	settingsBox->SetLabel("Detected Applications");
	ignoreCB = new BCheckBox(viewRect, "Ignore", "Do not include this app in ranking lists",
										new BMessage(EE_IGNORE_ATTR_CHANGED));
	ignoreCB->SetEnabled(false);
	settingsBox->AddChild(BGridLayoutBuilder(5, 5)
		.Add(attrSView, 0, 0, 1, 3)
		.Add(addB, 1, 0)
		.Add(removeB, 1, 1)
		.Add(ignoreCB, 0, 3)
		.SetInsets(5, 5, 5, 5)
	);


	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.Add(defaultSettingsBox)
		.Add(settingsBox)
	);

	//File Panel
	appFilter = new AppRefFilter();
	appsPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL, appFilter);

	// Build app list
	RebuildAttrList();

	// TODO watch the settings folder for added or removed attribute files to trigger
	// rebuilding the list
}


AttrSettingsView::~AttrSettingsView()
{
	EmptyAttrList();
	delete appsPanel;
	delete appFilter;
}


void AttrSettingsView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{
		case EE_LIST_INCLUSION_CHANGED: {
			//Automatically include new applications
			if(includeRB->Value())
			{
				saveIncludeSetting(EE_XMLTEXT_VALUE_INCLUDE);
			}
			//prompt whether to include in lists
			else if(promptRB->Value())
			{
				saveIncludeSetting(EE_XMLTEXT_VALUE_PROMPT);
			}
			//Automatically exclude
			else if(ignoreRB->Value())
			{
				saveIncludeSetting(EE_XMLTEXT_VALUE_EXCLUDE);
			}
			break;
		}
		case EE_APPATTR_SELECTION_CHANGED: {
			Window()->Lock();
			clearItemSettings();
			updateSelectedItem();
			recallItemSettings();
			Window()->Unlock();
			break; }
		case EE_ADD_ATTR_FILE: {
			BMessage addmsg(EE_ADD_ATTR_FILE_REF);
			appsPanel->SetMessage(&addmsg);
			appsPanel->SetTarget(this);
			appsPanel->Show();
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
				BPath appPath(&srcEntry), appAttrPath(settingsPath);
				appAttrPath.Append(e_settings_app_dir);
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
					RebuildAttrList();
					// select the added application's list item
					int count=attrLView->CountItems();
					for(int index=0; index<count; index++)
					{
						AppAttrItem* item = (AppAttrItem*)attrLView->ItemAt(index);
						if(sigString.Compare(item->appSig)==0)
						{
							attrLView->Select(index);
							attrLView->ScrollToSelection();
							index = count;
						}
					}
				}
			}
			else (new BAlert("","Excecutable does not have an application signature","OK"))->Go();
			delete[] buf;
			Window()->Unlock();
			break; }
		case EE_REMOVE_ATTR_FILE: {
			int index = attrLView->CurrentSelection();
			if(index<0) { break; }
			AppAttrItem *item = (AppAttrItem*)attrLView->ItemAt(index);
			BString str("Are you sure you want to delete all data for application ");
			str.Append(item->appName);
			if((new BAlert("",str.String(), "Yes", "No!"))->Go()) { break; }
			BPath trashPath;
			find_directory(B_TRASH_DIRECTORY, &trashPath);
			BDirectory trashDir(trashPath.Path());
			BString ent(item->attrPath.String());
			BEntry entry(ent.String());
			// TODO move to trash doesn't work if there is already a file of the same name in the trash
			if(entry.InitCheck()==B_OK) { entry.MoveTo(&trashDir); }
			ent.Append(".db");
			entry.SetTo(ent.String());
			if(entry.InitCheck()==B_OK) { entry.MoveTo(&trashDir); }
			attrLView->RemoveItem(index);
			delete item;
			break; }
		case EE_IGNORE_ATTR_CHANGED: {
			saveItemSettings();
			break; }
	}
}


void AttrSettingsView::SetLinkInclusionDefault(const char* value)
{
	if( strcmp(value,EE_XMLTEXT_VALUE_INCLUDE)==0 )
	{
		includeRB->SetValue(true);
	}
	else if(strcmp(value,EE_XMLTEXT_VALUE_PROMPT)==0 )
	{
		promptRB->SetValue(true);
	}
	else if(strcmp(value,EE_XMLTEXT_VALUE_EXCLUDE)==0 )
	{
		ignoreRB->SetValue(true);
	}
}

BSize AttrSettingsView::GetMinSize()
{
	BSize size(B_SIZE_UNSET, B_SIZE_UNSET);
	size.width = ignoreRB->MinSize().width + 20;
	size.height = (4 * ignoreRB->MinSize().height)
					+ (2 * addB->MinSize().height) + 90;
	return size;
}

/*void AttrSettingsView::SaveChanges()
{	saveItemSettings();
	AppAttrItem* item;
	BNode attrNode;
	int count = attrLView->CountItems();
	for(int i=0; i<count; i++)
	{	item = (AppAttrItem*)(attrLView->ItemAt(i));
		if(!item->changed) { continue; }
		attrNode.SetTo(item->attrPath.String());
		if(attrNode.InitCheck()!=B_OK) { continue; }
		attrNode.WriteAttr("EIN:IGNORE", B_BOOL_TYPE, 0, &(item->s_ignore), sizeof(bool));
		item->changed = false;
		attrLView->InvalidateItem(i);
	}

}*/

void AttrSettingsView::RebuildAttrList()
{	EmptyAttrList();

	BPath appAttrDirPath(settingsPath), attrPath;//create path for the application attribute files directory
	appAttrDirPath.Append(e_settings_app_dir);
	BDirectory appAttrDir(appAttrDirPath.Path());//create a directory object from the path
	BEntry attrEntry;//a BEntry to get the entry for each file in the directory
	BNode attrNode;
	BNodeInfo attrNodeInfo;
	attr_info info;
	char nodeType[B_MIME_TYPE_LENGTH];
	BString sig;
	AppAttrItem *item=NULL;
	appAttrDir.Rewind();//start at beginning of entry list
	while(appAttrDir.GetNextEntry(&attrEntry) == B_OK)//get each entry in the directory
	{	if( (attrNode.SetTo(&attrEntry)) != B_OK) { continue; }
		if( (attrNodeInfo.SetTo(&attrNode)) != B_OK) { continue; }
		if( (attrNodeInfo.GetType(nodeType)) != B_OK ||
			strcmp(nodeType, e_app_attr_filetype) != 0) { continue; }

		if(attrNode.GetAttrInfo(ATTR_APP_SIG_NAME, &info) != B_OK) { continue; }
		attrNode.ReadAttrString(ATTR_APP_SIG_NAME, &sig);
		attrPath.SetTo(&attrEntry);
		item = new AppAttrItem(sig.String(), attrPath);
		bool E_ignore=false;
		if(attrNode.GetAttrInfo(ATTR_IGNORE_NAME, &info) == B_NO_ERROR)
		{	attrNode.ReadAttr(ATTR_IGNORE_NAME, B_BOOL_TYPE, 0, &E_ignore, sizeof(E_ignore)); }
		item->s_ignore = E_ignore;
		attrLView->AddItem(item);
		attrNode.Unset();
	}

	// list items alphabetically ignoring case
	attrLView->SortItems(SortAppAttrItems);
}



void AttrSettingsView::EmptyAttrList()
{	//Remove List Items
	AppAttrItem *Item;
	int32 count;
	do
	{	Item = (AppAttrItem*)attrLView->RemoveItem(int32(0));
		if(Item) delete Item;
	}while(Item);
}


void AttrSettingsView::SaveChangedItem(AppAttrItem *item)
{
	BNode attrNode(item->attrPath.String());
	if(attrNode.InitCheck()!=B_OK)
	{
		// TODO error message?
		return;
	}
	attrNode.WriteAttr(ATTR_IGNORE_NAME, B_BOOL_TYPE, 0, &(item->s_ignore), sizeof(bool));
}


void AttrSettingsView::saveItemSettings()//save user configureable settings
{	if(selectedItem != NULL)
	{	bool ignore = ignoreCB->Value();
		if(ignore!=selectedItem->s_ignore)
		{	selectedItem->s_ignore = ignore;
		//	selectedItem->changed = !(selectedItem->changed);
			SaveChangedItem(selectedItem);
			attrLView->InvalidateItem(attrLView->IndexOf(selectedItem));
		}
	}
	return;
}


void AttrSettingsView::clearItemSettings()
{
	ignoreCB->SetValue(0);
	return;
}


void AttrSettingsView::updateSelectedItem()
{
	int32 index = attrLView->CurrentSelection();
	if(index < 0)//No selection
	{	selectedItem = NULL; }
	else//Item selected
	{	selectedItem = (AppAttrItem*)attrLView->ItemAt(index); }
	return;
}


void AttrSettingsView::recallItemSettings()
{
	if(selectedItem == NULL)//no selected item
	{	ignoreCB->SetEnabled(false);
		removeB->SetEnabled(false);
	}
	else
	{
		ignoreCB->SetEnabled(true);
		ignoreCB->SetValue(selectedItem->s_ignore);
		removeB->SetEnabled(true);
	}
	return;
}


void AttrSettingsView::saveIncludeSetting(const char *value)
{
	EESettingsFile *eeSettings = new EESettingsFile();
	eeSettings->SaveLinkInclusionDefaultValue(value);
	eeSettings->QuitRequested();
}


AppAttrItem::AppAttrItem(const char* sig, BPath& path)
	:BListItem(), /*changed(false), */s_ignore(false)
	,attrPath(path.Path()), appSig(sig), appName(path.Leaf())
	// TODO appName may not be path.Leaf()
{	}


void AppAttrItem::DrawItem(BView* owner, BRect item_rect, bool complete)
{	rgb_color color;
	bool selected = IsSelected();
	if(selected) color = selected_color;
	else color = owner->ViewColor();
	owner->SetLowColor(color);
	owner->SetDrawingMode(B_OP_COPY);
	if(selected || complete)
	{	owner->SetHighColor(color);
		owner->FillRect(item_rect);
	}
	if(!s_ignore) owner->SetHighColor(enabled_color);
	else owner->SetHighColor(disabled_color);
	BString text(appName);
//	if(changed) text.Append("*");
	text.Append(" (").Append(appSig).Append(")");
	owner->DrawString(text.String(), BPoint(item_rect.left+5.0,item_rect.bottom - 2.0));
}


int AppAttrItem::ICompare(AppAttrItem *item)
{
	return appName.ICompare(item->appName);
}

int SortAppAttrItems(const void* item1, const void* item2)
{
	AppAttrItem *first = *(AppAttrItem**)item1, *second = *(AppAttrItem**)item2;
	return (first->ICompare(second));
}
