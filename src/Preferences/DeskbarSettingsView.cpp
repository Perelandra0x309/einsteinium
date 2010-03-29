/* DeskbarSettingsView.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "DeskbarSettingsView.h"


DeskbarSettingsView::DeskbarSettingsView(BRect size)
	:
	BView(size, "Deskbar Settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	SetViewColor(bg_color);
	BRect viewRect;
	fDeskbarBox = new BBox("Deskbar");
	fDeskbarBox->SetLabel("Deskbar Settings");
	fShowDeskbarCB = new BCheckBox(viewRect, "Show Deskbar",
						"Show a list of ranked applications in the Deskbar",
						new BMessage(EE_DESKBAR_CHANGED));
	fItemCountTC = new BTextControl("Show this many applications:", "",
						new BMessage(EE_DESKBAR_CHANGED));
	//fItemCountTC->SetDivider(be_plain_font->StringWidth(fItemCountTC->Label()) + 4);
	fItemCountTC->SetExplicitMaxSize(BSize(be_plain_font->StringWidth(fItemCountTC->Label())
				+ be_plain_font->StringWidth("00000000"), B_SIZE_UNSET));
	long i;
	BTextView *textView = fItemCountTC->TextView();
	for (i = 0; i < 256; i++)
		textView->DisallowChar(i);
	for (i = '0'; i <= '9'; i++)
		textView->AllowChar(i);
	textView->AllowChar(B_BACKSPACE);

	fDeskbarBox->AddChild(BGroupLayoutBuilder(B_VERTICAL, 5)
		.Add(fShowDeskbarCB)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL, 5)
			.Add(fItemCountTC)
			.AddGlue()
			.SetInsets(15, 0, 0, 0)
		)
		.SetInsets(5, 5, 5, 5)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.Add(fDeskbarBox)
		.AddGlue()
	);
}


/*DeskbarSettingsView::~DeskbarSettingsView()
{
}*/


void
DeskbarSettingsView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{
		case EE_DESKBAR_CHANGED: {
			bool showDeskbar = fShowDeskbarCB->Value();
			int16 count = strtol(fItemCountTC->Text(), NULL, 0);
			EESettingsFile settings;
			settings.SaveDeskbarSettings(showDeskbar, count);

			app_info info;
			status_t result = be_roster->GetAppInfo(e_engine_sig, &info);
			if(result==B_OK)
			{
				// Send message to update the deskbar menu
				BMessenger messenger(e_engine_sig);
				BMessage msg(E_UPDATE_SHELFVIEW_SETTINGS);
				msg.AddBool("show", showDeskbar);
				msg.AddInt16("count", count);
				messenger.SendMessage(&msg);
			}

			fItemCountTC->SetEnabled(showDeskbar);
			break;
		}
	}
}


void
DeskbarSettingsView::SetDeskbarValues(bool show, int count)
{
	fShowDeskbarCB->SetValue(show);
	BString number;
	number << count;
	fItemCountTC->SetText(number.String());
	fItemCountTC->SetEnabled(show);
}
