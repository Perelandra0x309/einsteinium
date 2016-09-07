/* LauncherAboutView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherAboutView.h"

LauncherAboutView::LauncherAboutView(BRect size)
	:
	BView(size, "Launcher Status", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	// About box
	fAboutBox = new BBox("About");
	fAboutBox->SetLabel("About Einsteinum Launcher");
	fAboutTextView = new BTextView("About text");
	fAboutTextView->SetText("The Einsteinium Launcher is a menu of"
		" applications that resides in the deskbar.  This menu uses the Einsteinium Engine"
		" to retrieve a list of ranked applications which are ordered by criteria set in"
		" this preferences application.  Each application in the Launcher menu list can be"
		" started by clicking on that menu item.");
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);
	BGroupLayout *boxLayout = BLayoutBuilder::Group<>(B_VERTICAL)
		.Add(fAboutTextView)
		.SetInsets(10, 20, 10, 10)
	;
	fAboutBox->AddChild(boxLayout->View());

	// Layout
	BGroupLayout *layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
	BLayoutBuilder::Group<>(layout)
		.Add(fAboutBox, 0)
		.AddGlue()
	;
}


void
LauncherAboutView::AttachedToWindow()
{
	SetViewColor(Parent()->ViewColor());
	fAboutTextView->SetViewColor(Parent()->ViewColor());
	BView::AttachedToWindow();
}


void
LauncherAboutView::FrameResized(float width, float height)
{
	fAboutBox->Invalidate();
	fAboutTextView->Invalidate();
	InvalidateLayout();
	BView::FrameResized(width, height);
}
