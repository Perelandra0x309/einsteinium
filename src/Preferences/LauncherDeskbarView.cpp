/* LauncherDeskbarView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherDeskbarView.h"


LauncherDeskbarView::LauncherDeskbarView(BRect size)
	:
	BView(size, "Deskbar Settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	SetViewColor(bg_color);
	BRect viewRect;
	fDeskbarBox = new BBox("Deskbar");
	fDeskbarBox->SetLabel("Deskbar Menu Settings");
	fItemCountTC = new BTextControl("Show this many applications:", "",
						new BMessage(EL_DESKBAR_CHANGED));
//	fItemCountTC->SetDivider(be_plain_font->StringWidth(fItemCountTC->Label()) + 4);
	fItemCountTC->SetExplicitMaxSize(BSize(be_plain_font->StringWidth(fItemCountTC->Label())
				+ be_plain_font->StringWidth("00000000"), B_SIZE_UNSET));
	long i;
	BTextView *textView = fItemCountTC->TextView();
	for (i = 0; i < 256; i++)
		textView->DisallowChar(i);
	for (i = '0'; i <= '9'; i++)
		textView->AllowChar(i);
	textView->AllowChar(B_BACKSPACE);

	fDeskbarBox->AddChild(BGroupLayoutBuilder(B_HORIZONTAL, 5)
		.Add(fItemCountTC)
		.AddGlue()
		.SetInsets(5, 5, 5, 5)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.Add(fDeskbarBox)
		.AddGlue()
	);
}


void
LauncherDeskbarView::FrameResized(float width, float height)
{
	fDeskbarBox->Invalidate();
	fItemCountTC->Invalidate();
	BView::FrameResized(width, height);
}


void
LauncherDeskbarView::SetDeskbarCount(int count)
{
	BString number;
	number << count;
	fItemCountTC->SetText(number.String());
}
