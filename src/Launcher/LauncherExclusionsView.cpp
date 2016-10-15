/* LauncherExclusionsView.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherExclusionsView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Settings 'App Exclusions' tab"

LauncherExclusionsView::LauncherExclusionsView(BRect size, BMessage *appExclusions)
	:
	BView(size, "Apps Exclusions", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
//	SetViewColor(bg_color);
	BRect viewRect;

	// Default Settings
/*	fDefaultSettingsBox = new BBox("Default Settings BBox");
	fDefaultSettingsBox->SetLabel("When a new application is detected:");
	// TODO how to implement this?
	fPromptRB = new BRadioButton("New App Prompt",
						"Ask me whether to include the application in rank lists",
						new BMessage(EL_LIST_INCLUSION_CHANGED));
	fIncludeRB = new BRadioButton("New App Include",
						"Automatically include the application in rank lists",
						new BMessage(EL_LIST_INCLUSION_CHANGED));
	fIgnoreRB = new BRadioButton("New App Ignore",
						"Automatically do not include the application in rank lists",
						new BMessage(EL_LIST_INCLUSION_CHANGED));
	fDefaultSettingsBox->AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fIncludeRB)
		.Add(fPromptRB)
		.Add(fIgnoreRB)
		.SetInsets(5, 5, 5, 5)
	);*/
	// TODO Make a double pane view to easily move applications into the excluded list.
	// Subscribe to the engine to get a current list of applications.

	// Buttons
	fAddB = new BButton("Add",B_TRANSLATE_COMMENT("Add" B_UTF8_ELLIPSIS, "Button label"), new BMessage(EL_ADD_EXCLUSION));
	fAddB->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fRemoveB = new BButton("Remove", B_TRANSLATE_COMMENT("Remove", "Button label"), new BMessage(EL_REMOVE_EXCLUSION));
	fRemoveB->SetEnabled(false);

	// Create list for applications
	fExclusionLView = new BListView(viewRect, "Attr List View", B_SINGLE_SELECTION_LIST,
								B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fExclusionLView->SetSelectionMessage(new BMessage(EL_EXCLUSION_SELECTION_CHANGED));
	fExclusionSView = new BScrollView("Attr List Scroll View", fExclusionLView, B_FOLLOW_ALL_SIDES, 0,
								false, true);
	fExclusionSView->SetToolTip(B_TRANSLATE_COMMENT("Applications in this list will be completely excluded from\n"
								"the Launcher deskbar menu.  Adding or removing applications\n"
								"from this list will instantly update the Launcher.", "List tooltip"));

	// Inidividual app settings
	fSettingsBox = new BBox("Exclusion Settings");
	fSettingsBox->SetLabel(B_TRANSLATE_COMMENT("Exclude These Apps From The Launcher", "Box label"));

	BGridLayout *boxLayout = new BGridLayout(5, 5);
	fSettingsBox->SetLayout(boxLayout);
	BLayoutBuilder::Grid<>(boxLayout)
		.Add(fExclusionSView, 0, 0, 1, 3)
		.Add(fAddB, 1, 0)
		.Add(fRemoveB, 1, 1)
		.SetInsets(10, 20, 10, 10)
	;

	BGroupLayout *layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
//	BLayoutBuilder::Group<>(layout)
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 3)
//		.Add(fDefaultSettingsBox)
		.Add(fSettingsBox)
		.SetInsets(4, 4, 4, 4)
	);

	//File Panel
	fAppFilter = new AppRefFilter();
	fAppsPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL, fAppFilter);
	fAppsPanel->Window()->SetFeel(B_MODAL_ALL_WINDOW_FEEL);

	_RebuildExclusionsList(*appExclusions);
}


LauncherExclusionsView::~LauncherExclusionsView()
{
	_EmptyExclusionsList();
}


void
LauncherExclusionsView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fAddB->SetTarget(this);
	fRemoveB->SetTarget(this);
	fExclusionLView->SetTarget(this);
}


void
LauncherExclusionsView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case EL_ADD_EXCLUSION: {
			BMessage addmsg(EL_ADD_EXCLUSION_REF);
			fAppsPanel->SetMessage(&addmsg);
			fAppsPanel->SetTarget(this);
			fAppsPanel->Show();
			break; }
		case EL_ADD_EXCLUSION_REF: {
			bool success = AddExclusion(msg);
			if(success)
				be_app->PostMessage(EL_EXCLUSIONS_CHANGED);
			break; }
		case EL_REMOVE_EXCLUSION: {
			bool success = RemoveSelectedExclusion();
			if(success)
				be_app->PostMessage(EL_EXCLUSIONS_CHANGED);
			break; }
		case EL_EXCLUSION_SELECTION_CHANGED: {
			UpdateSelectedItem();
			break; }
	}
	BView::MessageReceived(msg);
}


void
LauncherExclusionsView::FrameResized(float width, float height)
{
	fSettingsBox->Invalidate();
	fAddB->Invalidate();
	fRemoveB->Invalidate();
	fExclusionLView->Invalidate();
	fExclusionSView->Invalidate();
	BView::FrameResized(width, height);
}


bool
LauncherExclusionsView::AddExclusion(BMessage* refMsg)
{
	bool addedApp = false;
	Window()->Lock();
	entry_ref srcRef;
	refMsg->FindRef("refs", &srcRef);
	BEntry srcEntry(&srcRef, true);
	BNode node(&srcEntry);
	char *buf = new char[B_ATTR_NAME_LENGTH];
	ssize_t size = node.ReadAttr("BEOS:APP_SIG",0,0,buf,B_ATTR_NAME_LENGTH);
//	BPath path;
//	BEntry entry(&srcRef);
//	entry.GetPath(&path);
	if( size > 0 )
	{
		ExcludeItem *item = new ExcludeItem(buf, srcRef.name);
		fExclusionLView->AddItem(item);
		fExclusionLView->SortItems(SortExcludeItems);
		fExclusionLView->Select(fExclusionLView->IndexOf(item));
		fExclusionLView->ScrollToSelection();
		addedApp = true;
	}
	else
	{
		BString text(srcRef.name);
		text.Append(B_TRANSLATE_COMMENT(" does not have an application signature", "Alert message.  Note space needed as first character"));
		(new BAlert("",text,B_TRANSLATE_COMMENT("OK", "Button label"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go(NULL);
	}
	delete[] buf;
	Window()->Unlock();
	return addedApp;
}


bool
LauncherExclusionsView::RemoveSelectedExclusion()
{
	int index = fExclusionLView->CurrentSelection();
	if(index<0)
		return false;

	ExcludeItem *item = (ExcludeItem*)fExclusionLView->ItemAt(index);
	Window()->Lock();
	fExclusionLView->RemoveItem(index);
	Window()->Unlock();
	delete item;
	return true;
}

/*
void
LauncherExclusionsView::SetLinkInclusionDefault(const char* value)
{
	if( strcmp(value,EL_XMLTEXT_VALUE_INCLUDE)==0 )
		fIncludeRB->SetValue(true);

	else if(strcmp(value,EL_XMLTEXT_VALUE_PROMPT)==0 )
		fPromptRB->SetValue(true);

	else if(strcmp(value,EL_XMLTEXT_VALUE_EXCLUDE)==0 )
		fIgnoreRB->SetValue(true);
}


void
LauncherExclusionsView::GetLinkInclusionDefault(BString &value)
{
	// Automatically include new applications
	if(fIncludeRB->Value())
	value.SetTo(EL_XMLTEXT_VALUE_INCLUDE);
	// Prompt whether to include in lists
	else if(fPromptRB->Value())
	value.SetTo(EL_XMLTEXT_VALUE_PROMPT);
	// Automatically exclude
	else if(fIgnoreRB->Value())
	value.SetTo(EL_XMLTEXT_VALUE_EXCLUDE);
}
*/

void
LauncherExclusionsView::PopulateExclusionsList(BMessage &exclusionsList)
{
	Window()->Lock();
	_RebuildExclusionsList(exclusionsList);
	Window()->Unlock();
}


void
LauncherExclusionsView::GetExclusionsList(BMessage &list)
{
	if(list.what != EL_MESSAGE_WHAT_EXCLUDED_APPS)
		return;

	ExcludeItem *item;
	int count = fExclusionLView->CountItems();
	for(int i=0; i<count; i++)
	{
		item = (ExcludeItem*)fExclusionLView->ItemAt(i);
		list.AddString(EL_EXCLUDE_SIGNATURE, item->fAppSig.String());
		list.AddString(EL_EXCLUDE_NAME, item->fAppName.String());
	}
}

/*
BSize
LauncherExclusionsView::GetMinSize()
{
	BSize size(B_SIZE_UNSET, B_SIZE_UNSET);
	size.width = fIgnoreRB->MinSize().width + 20;
	size.height = (4 * fIgnoreRB->MinSize().height)
					+ (2 * fAddB->MinSize().height) + 90;

	return size;
}*/


void
LauncherExclusionsView::_RebuildExclusionsList(BMessage &exclusionsList)
{
	type_code typeFound;
	int32 signatureCount, nameCount;
	status_t result1 = exclusionsList.GetInfo(EL_EXCLUDE_SIGNATURE, &typeFound, &signatureCount);
	status_t result2 = exclusionsList.GetInfo(EL_EXCLUDE_NAME, &typeFound, &nameCount);
	if(signatureCount != nameCount)
	{
		printf("Error building exclusions list: signature and name counts are not the same");
		return;
	}

	BString sig, name;
	_EmptyExclusionsList();
	for(int i=0; i<signatureCount; i++)
	{
		exclusionsList.FindString(EL_EXCLUDE_SIGNATURE, i, &sig);
		exclusionsList.FindString(EL_EXCLUDE_NAME, i, &name);
		ExcludeItem *item = new ExcludeItem(sig.String(), name.String());
		fExclusionLView->AddItem(item);
	}

	// list items alphabetically ignoring case
	fExclusionLView->SortItems(SortExcludeItems);
}


void
LauncherExclusionsView::_EmptyExclusionsList()
{
	//Remove List Items
	ExcludeItem *Item;
	do
	{	Item = (ExcludeItem*)fExclusionLView->RemoveItem(int32(0));
		if(Item)
			delete Item;
	}while(Item);
}


void
LauncherExclusionsView::UpdateSelectedItem()
{
	Window()->Lock();
	int32 index = fExclusionLView->CurrentSelection();
	//No selection
	if(index < 0)
	{
		fSelectedItem = NULL;
		fRemoveB->SetEnabled(false);
	}
	//Item selected
	else
	{
		fSelectedItem = (ExcludeItem*)fExclusionLView->ItemAt(index);
		fRemoveB->SetEnabled(true);
	}
	Window()->Unlock();
}


ExcludeItem::ExcludeItem(const char *sig, const char *name)
	:
	BListItem(),
	fAppSig(sig),
	fAppName(name)
{	}


void
ExcludeItem::DrawItem(BView* owner, BRect item_rect, bool complete)
{
	rgb_color color;
	bool selected = IsSelected();
	if(selected)
		color = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	else
		color = ui_color(B_LIST_BACKGROUND_COLOR);
	owner->SetLowColor(color);
	owner->SetDrawingMode(B_OP_COPY);
	if(selected || complete)
	{	owner->SetHighColor(color);
		owner->FillRect(item_rect);
	}
	if(IsSelected())
		owner->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
	else
		owner->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));
	BString text(fAppName);
	text.Append(" (").Append(fAppSig).Append(")");
	owner->DrawString(text.String(), BPoint(item_rect.left+5.0,item_rect.bottom - 2.0));
}


int
ExcludeItem::ICompare(ExcludeItem *item)
{
	return fAppName.ICompare(item->fAppName);
}


int
SortExcludeItems(const void* item1, const void* item2)
{
	ExcludeItem *first = *(ExcludeItem**)item1, *second = *(ExcludeItem**)item2;
	return (first->ICompare(second));
}

