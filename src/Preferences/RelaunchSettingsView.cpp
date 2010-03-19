/*RealunchSettingsView.cpp
	Functions for daemon relaunch apps settings
*/
#include "RelaunchSettingsView.h"

RelaunchSettingsView::RelaunchSettingsView(BRect size)
	:BView(size, "Auto-Relaunch", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{	SetViewColor(bg_color);
	BRect viewRect;

	addAppB = new BButton(viewRect, "Add", "Add" B_UTF8_ELLIPSIS,
						new BMessage(ED_ADD_APPITEM), B_FOLLOW_RIGHT);
	addAppB->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	removeAppB = new BButton(viewRect, "Remove", "Remove",
						new BMessage(ED_REMOVE_APPITEM), B_FOLLOW_RIGHT);
	removeAppB->SetEnabled(false);
	relaunchBox = new BBox("Application Relaunch Behavior");
	relaunchBox->SetLabel("Application Relaunch Settings");

	autoRelaunchRB = new BRadioButton("Auto Relaunch",
						"Automatically relaunch this application when it quits",
						new BMessage(ED_AUTO_RELAUNCH_CHANGED));
	promptRelaunchRB = new BRadioButton("Prompt Relaunch",
						"Ask me whether to relaunch this application when it quits",
						new BMessage(ED_AUTO_RELAUNCH_CHANGED));
	dontRelaunchRB = new BRadioButton("Ignore Relaunch",
						"Do not relaunch this application when it quits",
						new BMessage(ED_AUTO_RELAUNCH_CHANGED));

	appsLView = new BListView(viewRect, "App List View", B_SINGLE_SELECTION_LIST,
								B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	appsLView->SetSelectionMessage(new BMessage(ED_RELAPP_SELECTION_CHANGED));
	appsSView = new BScrollView("Apps List Scroll View", appsLView, B_FOLLOW_ALL_SIDES, 0,
								false, true);

	relaunchBox->AddChild(BGridLayoutBuilder(5, 5)
		.Add(appsSView, 0, 0, 1, 3)
		.Add(addAppB, 1, 0)
		.Add(removeAppB, 1, 1)
		.Add(autoRelaunchRB, 0, 3, 2, 1)
		.Add(promptRelaunchRB, 0, 4, 2, 1)
		.Add(dontRelaunchRB, 0, 5, 2, 1)
		.SetInsets(5, 5, 5, 5)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.Add(relaunchBox)
	);

	//File Panel
	appFilter = new AppRefFilter();
	appsPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL, appFilter);

	recallItemSettings();

	//Default relaunch settings item
	BPath path("/dummy/Default setting");
	defaultSettings = new AppRelaunchSettings("for all applications not in this list", path);

	//Settings file
	edSettings = new EDSettingsFile();
	ReadSettings();
}

RelaunchSettingsView::~RelaunchSettingsView()
{	//Remove List Items
	RelaunchAppItem *Item;
	int32 count;
	do
	{	Item = (RelaunchAppItem*)appsLView->RemoveItem(int32(0));
		delete Item;
	}while(Item);
	delete appsPanel;
	delete appFilter;
	delete edSettings;
}

void RelaunchSettingsView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{	case ED_RELAPP_SELECTION_CHANGED: {
			Window()->Lock();
			saveSelectedItemSettings();
			clearItemSettings();
			updateSelectedItem();
			recallItemSettings();
			Window()->Unlock();
			break; }
		case ED_ADD_APPITEM: {
			BMessage addmsg(ED_ADD_APPITEM_REF);
			appsPanel->SetMessage(&addmsg);
			appsPanel->SetTarget(this);
			appsPanel->Show();
			break; }
		case ED_ADD_APPITEM_REF: {
			entry_ref srcRef;
			msg->FindRef("refs", &srcRef);
			BEntry srcEntry(&srcRef, true);
			BPath path(&srcEntry);
			BNode node(&srcEntry);
			char *buf = new char[B_ATTR_NAME_LENGTH];
			ssize_t size;
			if( (size = node.ReadAttr("BEOS:APP_SIG",0,0,buf,B_ATTR_NAME_LENGTH)) > 0 )
			{
				Window()->Lock();
				RelaunchAppItem* item = new RelaunchAppItem(buf, path);
				appsLView->AddItem(item);
				appsLView->SortItems(SortRelaunchAppItems);
				appsLView->Select(appsLView->IndexOf(item));
				appsLView->ScrollToSelection();
				Window()->Unlock();
				edSettings->UpdateActionForApp(buf, item->settings->GetRelaunchActionString().String());

			}
			else (new BAlert("","Excecutable does not have an application signature","OK"))->Go();
			delete[] buf;
			break; }
		case ED_REMOVE_APPITEM: {
			int32 count = appsLView->CountItems();
			RelaunchAppItem *item;
			for(int i=count-1; i>=0; i--)
			{	if(appsLView->IsItemSelected(i))
				{	item = dynamic_cast<RelaunchAppItem *>(appsLView->RemoveItem(i));
					edSettings->RemoveApp(item->settings->appSig.String());
					delete item;
				}
			}
			saveSelectedItemSettings();
			clearItemSettings();
			updateSelectedItem();
			recallItemSettings();
			break; }
		case ED_AUTO_RELAUNCH_CHANGED: {
			saveSelectedItemSettings();
			edSettings->UpdateActionForApp(selectedItem->settings->appSig.String(),
									selectedItem->settings->GetRelaunchActionString().String());
			break; }
		default: { break; }
	}
	return;
}

/*void RelaunchSettingsView::WriteSettings()
{
	// if no changed were made no need to update settings file
	if(!settingsChanged) return;
	// shouldn't happen, but just in case BListView is empty...
	int count = appsLView->CountItems();
	if(count==0) return;

	BList settingsList;
	int defaultAction;
	RelaunchAppItem* item;
	// default action value
	item = (RelaunchAppItem *)appsLView->ItemAt(0);
	defaultAction = item->settings->relaunchAction;
	for(int i=1; i<count; i++)
	{
		item = (RelaunchAppItem *)appsLView->ItemAt(i);
//		printf("EP: Settings for %s have changed to %i.\n", item->settings->appSig.String(),
//							item->settings->relaunchAction);
		settingsList.AddItem(item->settings);
	}
	//printf("EP: Sending changed settings to daemon.\n");
//	EDSettingsFile edSettings;
	edSettings->SaveSettings(&settingsList, defaultAction);
	settingsList.MakeEmpty();

}*/


void RelaunchSettingsView::ReadSettings()
{
	// Add default settings item
	defaultSettings->relaunchAction = edSettings->GetDefaultRelaunchAction();
	appsLView->AddItem(new RelaunchAppItem(defaultSettings));

	// Read settings from file
	BList settingsList = edSettings->GetSettingsList();
	if(settingsList.IsEmpty()) return;
	int count = settingsList.CountItems();
	for(int i=0; i<count; i++){
		AppRelaunchSettings* settings = (AppRelaunchSettings *)settingsList.ItemAt(i);
		appsLView->AddItem(new RelaunchAppItem(settings));
	}

	// list items alphabetically ignoring case
	appsLView->SortItems(SortRelaunchAppItems);
}


BSize RelaunchSettingsView::GetMinSize()
{
	BSize size(B_SIZE_UNSET, B_SIZE_UNSET);
	size.width = promptRelaunchRB->MinSize().width + 20;
	size.height = (3 * promptRelaunchRB->MinSize().height)
					+ (2 * addAppB->MinSize().height) + 60;
	return size;
}


void RelaunchSettingsView::saveSelectedItemSettings()//save user configureable settings
{
	if(selectedItem != NULL)
	{
		//Automatically relaunch app without prompting
		if(autoRelaunchRB->Value())
		{
			selectedItem->settings->relaunchAction = ACTION_AUTO;
		}
		//prompt whether to relaunch
		else if(promptRelaunchRB->Value())
		{
			selectedItem->settings->relaunchAction = ACTION_PROMPT;
		}
		//dont relaunch and dont ask
		else if(dontRelaunchRB->Value())
		{
			selectedItem->settings->relaunchAction = ACTION_IGNORE;
		}
		//somehow no radio button selected?  Prompt
		else
		{
			selectedItem->settings->relaunchAction = ACTION_PROMPT;
		}

		appsLView->InvalidateItem(appsLView->IndexOf(selectedItem));
	}
	return;
}


void RelaunchSettingsView::clearItemSettings()
{
	autoRelaunchRB->SetValue(false);
	promptRelaunchRB->SetValue(false);
	dontRelaunchRB->SetValue(false);
	return;
}


void RelaunchSettingsView::updateSelectedItem()
{
	int32 index = appsLView->CurrentSelection();
	if(index < 0)//No selection
	{	selectedItem = NULL; }
	else//Item selected
	{	selectedItem = (RelaunchAppItem*)appsLView->ItemAt(index); }
	return;
}


void RelaunchSettingsView::recallItemSettings()
{	//Enable or disable objects
	if(selectedItem == NULL)//no selected item
	{
		autoRelaunchRB->SetEnabled(false);
		promptRelaunchRB->SetEnabled(false);
		dontRelaunchRB->SetEnabled(false);
		autoRelaunchRB->SetValue(false);
		promptRelaunchRB->SetValue(false);
		dontRelaunchRB->SetValue(false);
		removeAppB->SetEnabled(false);
	}
	else
	{
		autoRelaunchRB->SetEnabled(true);
		promptRelaunchRB->SetEnabled(true);
		dontRelaunchRB->SetEnabled(true);
		switch(selectedItem->settings->relaunchAction)
		{
			case ACTION_AUTO: {
				autoRelaunchRB->SetValue(true);
				break;
			}
			case ACTION_PROMPT: {
				promptRelaunchRB->SetValue(true);
				break;
			}
			case ACTION_IGNORE: {
				dontRelaunchRB->SetValue(true);
				break;
			}
			default: {
				// default to prompt
				promptRelaunchRB->SetValue(true);
				break;
			}
		}
		// Disable the Remove button for the default settings list item (index 0) and no selection
		if(appsLView->CurrentSelection()) removeAppB->SetEnabled(true);
		else removeAppB->SetEnabled(false);
	}
	return;
}


RelaunchAppItem::RelaunchAppItem(const char* sig, BPath path)
	:BListItem()
{
	settings = new AppRelaunchSettings(sig, path);
}


RelaunchAppItem::RelaunchAppItem(AppRelaunchSettings *set)
	:BListItem()
{
	settings = new AppRelaunchSettings(set);
}


RelaunchAppItem::~RelaunchAppItem()
{
	delete settings;
}


void RelaunchAppItem::DrawItem(BView* owner, BRect item_rect, bool complete)
{	rgb_color color;
	bool selected = IsSelected();
	// Background redraw
	if(selected) color = selected_color;
	else color = owner->ViewColor();
	owner->SetLowColor(color);
	owner->SetDrawingMode(B_OP_COPY);
	if(selected || complete)
	{	owner->SetHighColor(color);
		owner->FillRect(item_rect);
	}
	// Text redraw
	switch(settings->relaunchAction)
	{
		case ACTION_AUTO: {
			owner->SetHighColor(auto_relaunch_color);
			break;
		}
		case ACTION_PROMPT: {
			owner->SetHighColor(prompt_relaunch_color);
			break;
		}
		case ACTION_IGNORE: {
			owner->SetHighColor(dontrelaunch_color);
			break;
		}
	}
	BString text(settings->name);
	text.Append(" (").Append(settings->appSig).Append(")");
	owner->DrawString(text.String(), BPoint(item_rect.left+5.0,item_rect.bottom - 2.0));
}


int RelaunchAppItem::ICompare(RelaunchAppItem *item)
{
	// Always set the default list item before all others
	if(settings->appPath.Compare("/dummy/Default setting") == 0) return -1;
	if(item->settings->appPath.Compare("/dummy/Default setting") == 0) return 1;
	// sort by app name
	return settings->name.ICompare(item->settings->name);
}

int SortRelaunchAppItems(const void* item1, const void* item2)
{
	RelaunchAppItem *first = *(RelaunchAppItem**)item1, *second = *(RelaunchAppItem**)item2;
	return (first->ICompare(second));
}
