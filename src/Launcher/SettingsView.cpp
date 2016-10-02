/* SettingsView.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "SettingsView.h"

SettingsView::SettingsView(BRect size, AppSettings* settings)
	:
	BView(size, "Layout", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
/*	fOneLineRB = new BRadioButton("One Line", "Show application name and status on one line",
		new BMessage(LINES_OPTION_CHANGED));
	fTwoLinesRB = new BRadioButton("Two Lines", "Show application name and status on two lines",
		new BMessage(LINES_OPTION_CHANGED));*/

/*	fMaxIconSizeMenu = new BPopUpMenu("Max Icon Size Menu");
//	fMaxIconSizeMenu->AddItem(new BMenuItem("No Icon", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMaxIconSizeMenu->AddItem(new BMenuItem("8", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMaxIconSizeMenu->AddItem(new BMenuItem("16", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMaxIconSizeMenu->AddItem(new BMenuItem("24", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMaxIconSizeMenu->AddItem(new BMenuItem("32", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMaxIconSizeMenu->AddItem(new BMenuItem("48", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMaxIconSizeMenu->AddItem(new BMenuItem("64", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMaxIconSizeMF = new BMenuField("App Max Icon Size Field", "App highest rank icon size:", fMaxIconSizeMenu);

	fMinIconSizeMenu = new BPopUpMenu("Min Icon Size Menu");
//	fMinIconSizeMenu->AddItem(new BMenuItem("No Icon", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMinIconSizeMenu->AddItem(new BMenuItem("8", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMinIconSizeMenu->AddItem(new BMenuItem("16", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMinIconSizeMenu->AddItem(new BMenuItem("24", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMinIconSizeMenu->AddItem(new BMenuItem("32", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMinIconSizeMenu->AddItem(new BMenuItem("48", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMinIconSizeMenu->AddItem(new BMenuItem("64", new BMessage(APP_ICON_OPTION_CHANGED)));
	fMinIconSizeMF = new BMenuField("App Min Icon Size Field", "App lowest rank icon size:", fMinIconSizeMenu);

	fDocIconSizeMenu = new BPopUpMenu("Recent Docs Icon Size Menu");
//	fDocIconSizeMenu->AddItem(new BMenuItem("No Icon", new BMessage(DOC_ICON_OPTION_CHANGED)));
	fDocIconSizeMenu->AddItem(new BMenuItem("8", new BMessage(DOC_ICON_OPTION_CHANGED)));
	fDocIconSizeMenu->AddItem(new BMenuItem("16", new BMessage(DOC_ICON_OPTION_CHANGED)));
	fDocIconSizeMenu->AddItem(new BMenuItem("24", new BMessage(DOC_ICON_OPTION_CHANGED)));
	fDocIconSizeMenu->AddItem(new BMenuItem("32", new BMessage(DOC_ICON_OPTION_CHANGED)));
	fDocIconSizeMenu->AddItem(new BMenuItem("48", new BMessage(DOC_ICON_OPTION_CHANGED)));
	fDocIconSizeMenu->AddItem(new BMenuItem("64", new BMessage(DOC_ICON_OPTION_CHANGED)));
	fDocIconSizeMF = new BMenuField("Recent Docs Icon Size Field", "Recent documents icon size:", fDocIconSizeMenu);
*/
	fAppsCountTC = new BTextControl("Show this many apps:", "",
							new BMessage(EL_APP_COUNT_OPTION_CHANGED));
	BTextView *textView = fAppsCountTC->TextView();
	uint32 i;
	for (i=0; i<256; i++) textView->DisallowChar(i);
	for (i ='0'; i<='9'; i++) textView->AllowChar(i);
	textView->AllowChar(B_BACKSPACE);
	textView->SetMaxBytes(3);
//	textView->SetExplicitMaxSize(BSize(be_plain_font->StringWidth("0000"), B_SIZE_UNSET));

	fMaxIconsizeS = new BSlider("Max Icon Size", "Highest rank icon size:",
						NULL, 1, 8, B_HORIZONTAL, B_TRIANGLE_THUMB);
	fMaxIconsizeS->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fMaxIconsizeS->SetHashMarkCount(8);
	BString minLabel, maxLabel;
	minLabel << _IconSizeForSliderValue(1);
	maxLabel << _IconSizeForSliderValue(fMaxIconsizeS->HashMarkCount());
	fMaxIconsizeS->SetLimitLabels(minLabel.String(),maxLabel.String());
	fMaxIconsizeS->SetModificationMessage(new BMessage(EL_APP_ICON_OPTION_DRAG));

	fMinIconsizeS = new BSlider("Min Icon Size", "Lowest rank icon size:",
						NULL, 1, 8, B_HORIZONTAL, B_TRIANGLE_THUMB);
	fMinIconsizeS->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fMinIconsizeS->SetHashMarkCount(8);
	fMinIconsizeS->SetLimitLabels(minLabel.String(),maxLabel.String());
	fMinIconsizeS->SetModificationMessage(new BMessage(EL_APP_ICON_OPTION_DRAG));

	fDocIconsizeS = new BSlider("Doc Icon Size", "Recent files icon size:",
						NULL, 1, 8, B_HORIZONTAL, B_TRIANGLE_THUMB);
	fDocIconsizeS->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fDocIconsizeS->SetHashMarkCount(8);
	fDocIconsizeS->SetLimitLabels(minLabel.String(),maxLabel.String());
	fDocIconsizeS->SetModificationMessage(new BMessage(EL_DOC_ICON_OPTION_DRAG));

	fRecentFilesCountTC = new BTextControl("Show this many recent files:", "",
								new BMessage(EL_DOC_COUNT_OPTION_CHANGED));
	textView = fRecentFilesCountTC->TextView();
	for (i=0; i<256; i++) textView->DisallowChar(i);
	for (i ='0'; i<='9'; i++) textView->AllowChar(i);
	textView->AllowChar(B_BACKSPACE);
	textView->SetMaxBytes(3);

	fRecentFoldersCountTC = new BTextControl("Show this many recent folders:", "",
								new BMessage(EL_FOLDER_COUNT_OPTION_CHANGED));
	textView = fRecentFoldersCountTC->TextView();
	for (i=0; i<256; i++) textView->DisallowChar(i);
	for (i ='0'; i<='9'; i++) textView->AllowChar(i);
	textView->AllowChar(B_BACKSPACE);
	textView->SetMaxBytes(3);

	fRecentQueriesCountTC = new BTextControl("Show this many recent queries:", "",
								new BMessage(EL_QUERY_COUNT_OPTION_CHANGED));
	textView = fRecentQueriesCountTC->TextView();
	for (i=0; i<256; i++) textView->DisallowChar(i);
	for (i ='0'; i<='9'; i++) textView->AllowChar(i);
	textView->AllowChar(B_BACKSPACE);
	textView->SetMaxBytes(3);

	fFontSizeMenu = new BPopUpMenu("Font Size Menu");
	fFontSizeMenu->AddItem(new BMenuItem("System size", new BMessage(EL_FONT_OPTION_CHANGED)));
	BString fontSize;
	for(int i=8; i<25; i++)
	{
		fontSize.SetTo("");
		fontSize << i;
		fFontSizeMenu->AddItem(new BMenuItem(fontSize.String(), new BMessage(EL_FONT_OPTION_CHANGED)));
	}
	fFontSizeMF = new BMenuField("Font Size Field", "Font size:", fFontSizeMenu);

	fWindowLookMenu = new BPopUpMenu("Window Look Menu");
	fWindowLookMenu->AddItem(new BMenuItem("Normal Title Bar", new BMessage(EL_LOOK_OPTION_CHANGED)));
	fWindowLookMenu->AddItem(new BMenuItem("Small Title Bar", new BMessage(EL_LOOK_OPTION_CHANGED)));
	fWindowLookMenu->AddItem(new BMenuItem("No Title Bar", new BMessage(EL_LOOK_OPTION_CHANGED)));
	fWindowLookMenu->AddItem(new BMenuItem("No Border", new BMessage(EL_LOOK_OPTION_CHANGED)));
	fWindowLookMF = new BMenuField("Window Look Field", "Window look:", fWindowLookMenu);

	fDeskbarShowCB = new BCheckBox("Show Deskbar Menu", new BMessage(EL_DESKBAR_OPTION_CHANGED));

//	fFloatCB = new BCheckBox("Float above all windows", new BMessage(FLOAT_OPTION_CHANGED));

//	fAboutB = new BButton("About", "About" B_UTF8_ELLIPSIS, new BMessage(B_ABOUT_REQUESTED));

	BBox *appsBox = new BBox("Apps");
	appsBox->SetLabel("Apps");
	BGroupLayout *appsBoxLayout = new BGroupLayout(B_VERTICAL);
	appsBox->SetLayout(appsBoxLayout);
	BLayoutBuilder::Group<>(appsBoxLayout)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL)
			.Add(fAppsCountTC)
			.AddGlue()
		)
		.Add(fMaxIconsizeS)
		.Add(fMinIconsizeS)
		.SetInsets(8,20,8,8)
	;

	BBox *filesBox = new BBox("Recent Files");
	filesBox->SetLabel("Files, Folders and Queries");
	BGroupLayout *filesBoxLayout = new BGroupLayout(B_VERTICAL);
	filesBox->SetLayout(filesBoxLayout);
	BLayoutBuilder::Group<>(filesBoxLayout)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL)
			.Add(fRecentFilesCountTC)
			.AddGlue()
		)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL)
			.Add(fRecentFoldersCountTC)
			.AddGlue()
		)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL)
			.Add(fRecentQueriesCountTC)
			.AddGlue()
		)
		.Add(fDocIconsizeS)
		.SetInsets(8,20,8,8)
	;

	BBox *allBox = new BBox("Launcher");
	allBox->SetLabel("Launcher");
	BGroupLayout *allBoxLayout = new BGroupLayout(B_VERTICAL);
	allBox->SetLayout(allBoxLayout);
	BLayoutBuilder::Group<>(allBoxLayout)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL)
			.Add(fFontSizeMF)
			.AddGlue()
		)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL)
			.Add(fWindowLookMF)
			.AddGlue()
		)
		.Add(fDeskbarShowCB)
		.SetInsets(8,20,8,8)
	;

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 3)
		.Add(appsBox)
		.Add(filesBox)
		.Add(allBox)
		.AddGlue()
		.SetInsets(4, 4, 4, 4)
	);

	SetAppSettings(settings);
}


void
SettingsView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fAppsCountTC->SetTarget(this);
	fRecentFilesCountTC->SetTarget(be_app);
	fRecentFoldersCountTC->SetTarget(be_app);
	fRecentQueriesCountTC->SetTarget(be_app);
	fFontSizeMenu->SetTargetForItems(be_app);
	fWindowLookMenu->SetTargetForItems(be_app);
	MakeFocus();
}


void
SettingsView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case EL_APP_COUNT_OPTION_CHANGED:
		{
			// Must have at least 2 apps
			int count = GetAppCount();
			if(count<2)
				SetAppCount(2);
			be_app->PostMessage(EL_APP_COUNT_OPTION_CHANGED);
			break;
		}
		case EL_DESKBAR_OPTION_CHANGED:
		{
			be_app->PostMessage(EL_DESKBAR_OPTION_CHANGED);
			break;
		}
		case EL_APP_ICON_OPTION_DRAG:
		{
			// Update labels
			_SetMaxIconLabel(GetMaxIconSize());
			_SetMinIconLabel(GetMinIconSize());
			be_app->PostMessage(EL_APP_ICON_OPTION_CHANGED);
			break;
		}
		case EL_DOC_ICON_OPTION_DRAG:
		{
			// Update label
			_SetDocIconLabel(GetDocIconSize());
			be_app->PostMessage(EL_DOC_ICON_OPTION_CHANGED);
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


void
SettingsView::KeyDown(const char* bytes, int32 numbytes)
{
	if(numbytes == 1) {
		switch(bytes[0]) {
			case B_ESCAPE:
			{
				Window()->Hide();
				break;
			}
			default:
			{
				BView::KeyDown(bytes, numbytes);
			}
		}
	}
	else
	{
		BView::KeyDown(bytes, numbytes);
	}
}

/*
bool
SettingsView::GetDrawTwoLines()
{
	if(fOneLineRB->Value())
		return false;
	else if(fTwoLinesRB->Value())
		return true;
	else
		return true;
}*/


uint
SettingsView::GetAppCount()
{
	return atoi(fAppsCountTC->Text());
}


void
SettingsView::SetAppCount(int value)
{
	BString count;
	count << value;
	fAppsCountTC->SetText(count);
}


int
SettingsView::GetMinIconSize()
{
	int size = _IconSizeForSliderValue(fMinIconsizeS->Value());
	return size;
}


void
SettingsView::SetMinIconSize(int value)
{
	fMinIconsizeS->SetValue(_SliderValueForIconSize(value));
	_SetMinIconLabel(value);
}


int
SettingsView::GetMaxIconSize()
{
	int size = _IconSizeForSliderValue(fMaxIconsizeS->Value());
	return size;
}


void
SettingsView::SetMaxIconSize(int value)
{
	fMaxIconsizeS->SetValue(_SliderValueForIconSize(value));
	_SetMaxIconLabel(value);
}


int
SettingsView::GetDocIconSize()
{
	int size = _IconSizeForSliderValue(fDocIconsizeS->Value());
	return size;
}

void
SettingsView::SetDocIconSize(int value)
{
	fDocIconsizeS->SetValue(_SliderValueForIconSize(value));
	_SetDocIconLabel(value);
}


uint
SettingsView::GetRecentDocCount()
{
	return atoi(fRecentFilesCountTC->Text());
}


void
SettingsView::SetRecentDocCount(int value)
{
	BString count;
	count << value;
	fRecentFilesCountTC->SetText(count);
}


uint
SettingsView::GetRecentFolderCount()
{
	return atoi(fRecentFoldersCountTC->Text());
}


void
SettingsView::SetRecentFolderCount(int value)
{
	BString count;
	count << value;
	fRecentFoldersCountTC->SetText(count);
}


uint
SettingsView::GetRecentQueryCount()
{
	return atoi(fRecentQueriesCountTC->Text());
}


void
SettingsView::SetRecentQueryCount(int value)
{
	BString count;
	count << value;
	fRecentQueriesCountTC->SetText(count);
}


float
SettingsView::GetFontSize()
{
	float size = 0;
	BMenuItem *item = fFontSizeMenu->FindMarked();
	if(item != NULL && fFontSizeMenu->IndexOf(item) != 0)
		size = atoi(item->Label());
//	else
//		size = be_plain_font->Size();
	return size;
}


void
SettingsView::_SetFontSize(float size)
{
	int selection = 0;
	if(size!=0)
	{
		selection = int(size-7);
	}
	BMenuItem* item = fFontSizeMenu->ItemAt(selection);
	if(item != NULL)
		item->SetMarked(true);
}


window_look
SettingsView::GetWindowLook()
{
	window_look look = B_TITLED_WINDOW_LOOK;
	BMenuItem *item = fWindowLookMenu->FindMarked();
	if(item != NULL)
	{
		switch(fWindowLookMenu->IndexOf(item))
		{
			case 1: {
				look = B_FLOATING_WINDOW_LOOK;
				break;
			}
			case 2: {
				look = B_MODAL_WINDOW_LOOK;
				break;
			}
			case 3: {
				look = B_BORDERED_WINDOW_LOOK;
				break;
			}
		}
	}
	return look;
}


void
SettingsView::_SetWindowLook(window_look look)
{
	int selection = 0;
	switch(look)
	{
		case B_FLOATING_WINDOW_LOOK:
		{
			selection = 1;
			break;
		}
		case B_MODAL_WINDOW_LOOK:
		{
			selection = 2;
			break;
		}
		case B_BORDERED_WINDOW_LOOK:
		{
			selection = 3;
			break;
		}
	}
	BMenuItem* item = fWindowLookMenu->ItemAt(selection);
	if(item != NULL)
	{
	//	Window()->Lock();
		item->SetMarked(true);
	//	Window()->Unlock();
	}
}


bool
SettingsView::GetShowDeskbarMenu()
{
	return fDeskbarShowCB->Value();
}


void
SettingsView::SetShowDeskbarMenu(bool value)
{
	fDeskbarShowCB->SetValue(value);
}

/*
window_feel
SettingsView::GetFloat()
{
// TODO: remove permanantly?
//	if(fFloatCB->Value())
		return B_MODAL_ALL_WINDOW_FEEL;//B_FLOATING_ALL_WINDOW_FEEL;
//	else
//		return B_NORMAL_WINDOW_FEEL;
}*/


void
SettingsView::PopulateAppSettings(AppSettings *settings)
{
	settings->appCount = GetAppCount();
	settings->minIconSize = GetMinIconSize();
	settings->maxIconSize = GetMaxIconSize();
	settings->docIconSize = GetDocIconSize();
	settings->recentDocCount = GetRecentDocCount();
	settings->recentFolderCount = GetRecentFolderCount();
	settings->recentQueryCount = GetRecentQueryCount();
	settings->fontSize = GetFontSize();
	settings->windowLook = GetWindowLook();
	settings->showDeskbarMenu = GetShowDeskbarMenu();
}


void
SettingsView::SetAppSettings(AppSettings* settings)
{
	SetAppCount(settings->appCount);
	SetMinIconSize(settings->minIconSize);
	SetMaxIconSize(settings->maxIconSize);
	SetDocIconSize(settings->docIconSize);
	SetRecentDocCount(settings->recentDocCount);
	SetRecentFolderCount(settings->recentFolderCount);
	SetRecentQueryCount(settings->recentQueryCount);
	_SetFontSize(settings->fontSize);
	_SetWindowLook(settings->windowLook);
	SetShowDeskbarMenu(settings->showDeskbarMenu);
}


/*
// Settings to save to file
void
SettingsView::GetSettingsToSave(BMessage* message)
{
//	message->AddBool(NAME_LINES_OPTION, GetDrawTwoLines());

	int8 markedItem;
	BMenuItem *item;
	message->AddInt8(NAME_MAX_ICON_OPTION, GetMaxIconSize());
	message->AddInt8(NAME_MIN_ICON_OPTION, GetMinIconSize());

	message->AddInt8(NAME_DOC_ICON_OPTION, GetDocIconSize());
	message->AddInt16(NAME_DOC_COUNT_OPTION, GetRecentDocCount());

	item = fFontSizeMenu->FindMarked();
	if(item != NULL)
		markedItem = fFontSizeMenu->IndexOf(item);
	else
		markedItem = 0;
	message->AddInt8(NAME_FONT_OPTION, markedItem);

	item = fWindowLookMenu->FindMarked();
	if(item != NULL)
		markedItem = fWindowLookMenu->IndexOf(item);
	else
		markedItem = 0;
	message->AddInt8(NAME_LOOK_OPTION, markedItem);

//	message->AddBool(NAME_FEEL_OPTION, fFloatCB->Value());
}


// Settings read from file
void
SettingsView::SetSavedSettings(BMessage *settings)
{
	bool boolValue;
	status_t result;

	int8 value;
	BMenuItem *item;

	result = settings->FindInt8(NAME_MAX_ICON_OPTION, &value);
	SetMaxIconSize(value);

	result = settings->FindInt8(NAME_MIN_ICON_OPTION, &value);
	SetMinIconSize(value);

	result = settings->FindInt8(NAME_DOC_ICON_OPTION, &value);
	SetDocIconSize(value);

	int16 count;
	result = settings->FindInt16(NAME_DOC_COUNT_OPTION, &count);
	if(result != B_OK)
		count = 25;
	SetRecentDocCount(count);

	result = settings->FindInt8(NAME_FONT_OPTION, &value);
	if(result != B_OK)
		value = 0;
	item = fFontSizeMenu->ItemAt(value);
	if(item != NULL)
		item->SetMarked(true);

	result = settings->FindInt8(NAME_LOOK_OPTION, &value);
	if(result != B_OK)
		value = 0;
	item = fWindowLookMenu->ItemAt(value);
	if(item != NULL)
		item->SetMarked(true);

}*/


int
SettingsView::_IconSizeForSliderValue(int value)
{
	return value*8;
}


int
SettingsView::_SliderValueForIconSize(int value)
{
	return value/8;
}


void
SettingsView::_SetMaxIconLabel(int value)
{
	BString label("Highest rank icon size: ");
	label << value;
	fMaxIconsizeS->SetLabel(label.String());
}


void
SettingsView::_SetMinIconLabel(int value)
{
	BString label("Lowest rank icon size: ");
	label << value;
	fMinIconsizeS->SetLabel(label.String());
}


void
SettingsView::_SetDocIconLabel(int value)
{
	BString label("Icon size: ");
	label << value;
	fDocIconsizeS->SetLabel(label.String());
}
