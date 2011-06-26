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

	fDeskbarBox = new BBox("Deskbar");
	fDeskbarBox->SetLabel("Deskbar Menu Settings");

	// Deskbar menu item count
	fItemCountTC = new BTextControl("Show this many applications:", "",
						new BMessage(EL_DESKBAR_CHANGED));
//	fItemCountTC->SetDivider(be_plain_font->StringWidth(fItemCountTC->Label()) + 4);
	fItemCountTC->SetToolTip("Change this value to be the number of applications\n"
							"you want to have listed in the Launcher deskbar menu.");
	fItemCountTC->SetExplicitMaxSize(BSize(be_plain_font->StringWidth(fItemCountTC->Label())
				+ be_plain_font->StringWidth("00000000"), B_SIZE_UNSET));
	long i;
	BTextView *textView = fItemCountTC->TextView();
	for (i = 0; i < 256; i++)
		textView->DisallowChar(i);
	for (i = '0'; i <= '9'; i++)
		textView->AllowChar(i);
	textView->AllowChar(B_BACKSPACE);

	// Engine auto launch setting
	fEngineLaunchCB = new BCheckBox("LaunchValue",
		"Automatically start the Engine when the Launcher starts.",
		new BMessage(EL_ENGINELAUNCH_CHANGED));
	fEngineLaunchCB->SetToolTip("If this option is checked, when the Launcher starts\n"
								"it will also automatically start the Engine.");

	BGroupLayout *boxLayout = new BGroupLayout(B_VERTICAL, 5);
	fDeskbarBox->SetLayout(boxLayout);
	BLayoutBuilder::Group<>(boxLayout)
		.Add(fItemCountTC)
		.Add(fEngineLaunchCB)
		.SetInsets(10, 20, 10, 10);
	fItemCountTC->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));

	BGroupLayout *layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
	BLayoutBuilder::Group<>(layout)
		.Add(fDeskbarBox)
		.AddGlue();

}


void
LauncherDeskbarView::FrameResized(float width, float height)
{
	fDeskbarBox->Invalidate();
	fItemCountTC->Invalidate();
	fEngineLaunchCB->Invalidate();
//	InvalidateLayout();
	BView::FrameResized(width, height);
}


BSize
LauncherDeskbarView::GetMinSize()
{
	BSize size(B_SIZE_UNSET, B_SIZE_UNSET);
	size.width = fEngineLaunchCB->MinSize().width + 30;
	return size;
}


void
LauncherDeskbarView::SetDeskbarCount(int count)
{
	BString number;
	number << count;
	fItemCountTC->SetText(number.String());
}


void
LauncherDeskbarView::SetEngineAutoLaunch(bool autoLaunch)
{
	fEngineLaunchCB->SetValue(autoLaunch);
}
