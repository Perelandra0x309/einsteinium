/* DaemonRelaunchView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "DaemonRelaunchView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Daemon relaunch view"

DaemonRelaunchView::DaemonRelaunchView(BRect size)
	:
	BView(size, "Auto-Relaunch", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fSelectedItem(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	fAddAppB = new BButton("Add", B_TRANSLATE_COMMENT("Add" B_UTF8_ELLIPSIS, "Button label"), new BMessage(ED_ADD_APPITEM));
	fRemoveAppB = new BButton("Remove", B_TRANSLATE_COMMENT("Remove", "Button label"), new BMessage(ED_REMOVE_APPITEM));
	fRemoveAppB->SetEnabled(false);
	fRelaunchBox = new BBox("Application Relaunch Behavior");
	fRelaunchBox->SetLabel(B_TRANSLATE_COMMENT("Application Relaunch Settings", "Box label"));
	// TODO Can the radio button labels be color coded to match the list view?
	fAutoRelaunchRB = new BRadioButton("Auto Relaunch",
						B_TRANSLATE_COMMENT("Automatically relaunch this application when it quits", "Radio button label"),
						new BMessage(ED_AUTO_RELAUNCH_CHANGED));
	fPromptRelaunchRB = new BRadioButton("Prompt Relaunch",
						B_TRANSLATE_COMMENT("Ask me whether to relaunch this application when it quits", "Radio button label"),
						new BMessage(ED_AUTO_RELAUNCH_CHANGED));
	fDontRelaunchRB = new BRadioButton("Ignore Relaunch",
						B_TRANSLATE_COMMENT("Do not relaunch this application when it quits", "Radio button label"),
						new BMessage(ED_AUTO_RELAUNCH_CHANGED));

	fAppsLView = new BListView("App List View", B_SINGLE_SELECTION_LIST);
	fAppsLView->SetSelectionMessage(new BMessage(ED_RELAPP_SELECTION_CHANGED));
	fAppsSView = new BScrollView("Apps List Scroll View", fAppsLView, 0, false, true);
	fAppsSView->SetToolTip(B_TRANSLATE_COMMENT("Use this list to specify what actions you want the Einsteinium Daemon to take\n"
							"when an application quits.  The \"Default setting\" item specifies the action\n"
							"to take for any apps that are not in this list.  Add specific apps to this list\n"
							"to override the default action for that specific app.  Each app in this list will\n"
							"be color coded based on the action specified, so you can quickly view all your\n"
							"app settings.", "Tootip text"));

	BGroupLayout *boxLayout = BLayoutBuilder::Group<>(B_VERTICAL, 10)
		.Add(fAppsSView)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fAddAppB)
			.Add(fRemoveAppB)
		.End()
		.Add(fAutoRelaunchRB)
		.Add(fPromptRelaunchRB)
		.Add(fDontRelaunchRB)
		.SetInsets(10, 10, 10, 10);
	fRelaunchBox->AddChild(boxLayout->View());

	BGroupLayout *layout = new BGroupLayout(B_VERTICAL, 10);
	SetLayout(layout);
	BLayoutBuilder::Group<>(layout)
		.Add(fRelaunchBox);

	//File Panel
	fAppFilter = new AppRefFilter();
	fAppsPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL, fAppFilter);

	_RecallItemSettings();

	//Default relaunch settings item
	BPath path("/dummy/Default setting");
	fDefaultSettings = new AppRelaunchSettings(B_TRANSLATE_COMMENT("for all applications not in this list", "Default list item label"), path);

	//Settings file
	fDaemonSettings = new EDSettingsFile(this);
}


DaemonRelaunchView::~DaemonRelaunchView()
{	//Remove List Items
	RelaunchAppItem *Item;
	int32 count;
	do
	{	Item = (RelaunchAppItem*)fAppsLView->RemoveItem(int32(0));
		delete Item;
	}while(Item);
	delete fAppsPanel;
	delete fAppFilter;
	delete fDaemonSettings;
}


void
DaemonRelaunchView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{	case ED_RELAPP_SELECTION_CHANGED: {
			Window()->Lock();
			_SaveSelectedItemSettings();
			_ClearItemSettings();
			_UpdateSelectedItem();
			_RecallItemSettings();
			Window()->Unlock();
			break; }
		case ED_ADD_APPITEM: {
			BMessage addmsg(ED_ADD_APPITEM_REF);
			fAppsPanel->SetMessage(&addmsg);
			fAppsPanel->SetTarget(this);
			fAppsPanel->Show();
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
				fAppsLView->AddItem(item);
				fAppsLView->SortItems(SortRelaunchAppItems);
				fAppsLView->Select(fAppsLView->IndexOf(item));
				fAppsLView->ScrollToSelection();
				Window()->Unlock();
				fDaemonSettings->UpdateActionForApp(buf, item->fSettings->relaunchAction);
			}
			else (new BAlert("",B_TRANSLATE_COMMENT("Executable does not have an application signature", "Alert message"),"OK"))->Go();
			delete[] buf;
			break; }
		case ED_REMOVE_APPITEM: {
			int32 count = fAppsLView->CountItems();
			RelaunchAppItem *item;
			for(int i=count-1; i>=0; i--)
			{	if(fAppsLView->IsItemSelected(i))
				{	item = dynamic_cast<RelaunchAppItem *>(fAppsLView->RemoveItem(i));
					fDaemonSettings->RemoveApp(item->fSettings->appSig.String());
					delete item;
				}
			}
			_SaveSelectedItemSettings();
			_ClearItemSettings();
			_UpdateSelectedItem();
			_RecallItemSettings();
			break; }
		case ED_AUTO_RELAUNCH_CHANGED: {
			_SaveSelectedItemSettings();
			if(fSelectedItem != NULL)
			{
				int32 index = fAppsLView->CurrentSelection();
				if(index == 0)
					fDaemonSettings->UpdateDefaultAction(fSelectedItem->fSettings->relaunchAction);
				else
					fDaemonSettings->UpdateActionForApp(fSelectedItem->fSettings->appSig.String(),
									fSelectedItem->fSettings->relaunchAction);
			}
			break; }
		default: {
			BView::MessageReceived(msg);
			break; }
	}
	return;
}


void
DaemonRelaunchView::FrameResized(float width, float height)
{
	fAppsLView->Invalidate();
	fAppsSView->Invalidate();
	fAddAppB->Invalidate();
	fRemoveAppB->Invalidate();
	fRelaunchBox->Invalidate();
	BView::FrameResized(width, height);
}


/*void DaemonRelaunchView::WriteSettings()
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


void
DaemonRelaunchView::ReadSettings()
{
	//Remove List Items
	RelaunchAppItem *Item;
	do
	{	Item = (RelaunchAppItem*)fAppsLView->RemoveItem(int32(0));
		if(Item)
			delete Item;
	}while(Item);

	// Add default settings item
	fDefaultSettings->relaunchAction = fDaemonSettings->GetDefaultRelaunchAction();
	fAppsLView->AddItem(new RelaunchAppItem(fDefaultSettings));

	// Read settings from file
	BList settingsList = fDaemonSettings->GetSettingsList();
	if(settingsList.IsEmpty()) return;
	int count = settingsList.CountItems();
	for(int i=0; i<count; i++){
		AppRelaunchSettings* settings = (AppRelaunchSettings *)settingsList.ItemAt(i);
		fAppsLView->AddItem(new RelaunchAppItem(settings));
	}

	// list items alphabetically ignoring case
	fAppsLView->SortItems(SortRelaunchAppItems);
}


BSize
DaemonRelaunchView::GetMinSize()
{
	BSize size(B_SIZE_UNSET, B_SIZE_UNSET);
	size.width = fPromptRelaunchRB->MinSize().width + 20;
	size.height = (3 * fPromptRelaunchRB->MinSize().height)
					+ (2 * fAddAppB->MinSize().height) + 60;
	return size;
}


void
DaemonRelaunchView::_SaveSelectedItemSettings()//save user configureable settings
{
	if(fSelectedItem != NULL)
	{
		//Automatically relaunch app without prompting
		if(fAutoRelaunchRB->Value())
			fSelectedItem->fSettings->relaunchAction = ACTION_AUTO;

		//prompt whether to relaunch
		else if(fPromptRelaunchRB->Value())
			fSelectedItem->fSettings->relaunchAction = ACTION_PROMPT;

		//dont relaunch and dont ask
		else if(fDontRelaunchRB->Value())
			fSelectedItem->fSettings->relaunchAction = ACTION_IGNORE;

		//somehow no radio button selected?  Prompt
		else
			fSelectedItem->fSettings->relaunchAction = ACTION_PROMPT;

		fAppsLView->InvalidateItem(fAppsLView->IndexOf(fSelectedItem));
	}
}


void
DaemonRelaunchView::_ClearItemSettings()
{
	fAutoRelaunchRB->SetValue(false);
	fPromptRelaunchRB->SetValue(false);
	fDontRelaunchRB->SetValue(false);
}


void
DaemonRelaunchView::_UpdateSelectedItem()
{
	int32 index = fAppsLView->CurrentSelection();
	if(index < 0)
		//No selection
		fSelectedItem = NULL;
	else
		fSelectedItem = (RelaunchAppItem*)fAppsLView->ItemAt(index);
}


void
DaemonRelaunchView::_RecallItemSettings()
{
	//Enable or disable objects
	if(fSelectedItem == NULL)//no selected item
	{
		fAutoRelaunchRB->SetEnabled(false);
		fPromptRelaunchRB->SetEnabled(false);
		fDontRelaunchRB->SetEnabled(false);
		fAutoRelaunchRB->SetValue(false);
		fPromptRelaunchRB->SetValue(false);
		fDontRelaunchRB->SetValue(false);
		fRemoveAppB->SetEnabled(false);
	}
	else
	{
		fAutoRelaunchRB->SetEnabled(true);
		fPromptRelaunchRB->SetEnabled(true);
		fDontRelaunchRB->SetEnabled(true);
		switch(fSelectedItem->fSettings->relaunchAction)
		{
			case ACTION_AUTO: {
				fAutoRelaunchRB->SetValue(true);
				break;
			}
			case ACTION_PROMPT: {
				fPromptRelaunchRB->SetValue(true);
				break;
			}
			case ACTION_IGNORE: {
				fDontRelaunchRB->SetValue(true);
				break;
			}
			default: {
				// default to prompt
				fPromptRelaunchRB->SetValue(true);
				break;
			}
		}
		// Disable the Remove button for the default settings list item (index 0) and no selection
		if(fAppsLView->CurrentSelection())
			fRemoveAppB->SetEnabled(true);
		else
			fRemoveAppB->SetEnabled(false);
	}
}


RelaunchAppItem::RelaunchAppItem(const char* sig, BPath path)
	:
	BListItem()
{
	fSettings = new AppRelaunchSettings(sig, path);
}


RelaunchAppItem::RelaunchAppItem(AppRelaunchSettings *set)
	:
	BListItem()
{
	fSettings = new AppRelaunchSettings(set);
}


RelaunchAppItem::~RelaunchAppItem()
{
	delete fSettings;
}


void
RelaunchAppItem::DrawItem(BView* owner, BRect item_rect, bool complete)
{
	rgb_color color;
	bool selected = IsSelected();
	// Background redraw
	if(selected) {
		color = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	}
	else {
		color = ui_color(B_LIST_BACKGROUND_COLOR);
	}
	owner->SetLowColor(color);
	owner->SetDrawingMode(B_OP_COPY);
	if(selected || complete)
	{	owner->SetHighColor(color);
		owner->FillRect(item_rect);
	}
	// Text redraw
	switch(fSettings->relaunchAction)
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
	BString text(fSettings->name);
	text.Append(" (").Append(fSettings->appSig).Append(")");
	owner->DrawString(text.String(), BPoint(item_rect.left+5.0,item_rect.bottom - 2.0));
}


int
RelaunchAppItem::ICompare(RelaunchAppItem *item)
{
	// Always set the default list item before all others
	if(fSettings->appPath.Compare("/dummy/Default setting") == 0) return -1;
	if(item->fSettings->appPath.Compare("/dummy/Default setting") == 0) return 1;
	// sort by app name
	return fSettings->name.ICompare(item->fSettings->name);
}


int
SortRelaunchAppItems(const void* item1, const void* item2)
{
	RelaunchAppItem *first = *(RelaunchAppItem**)item1, *second = *(RelaunchAppItem**)item2;
	return (first->ICompare(second));
}
