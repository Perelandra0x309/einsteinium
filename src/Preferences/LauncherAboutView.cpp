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

	BGroupLayout *boxLayout = new BGroupLayout(B_VERTICAL);
	fAboutBox->SetLayout(boxLayout);
	BLayoutBuilder::Group<>(boxLayout)
		.Add(fAboutTextView)
		.SetInsets(10, 20, 10, 10)
	;

	// Layout
	fAboutBox->SetExplicitMinSize(BSize(0,120));
	BGroupLayout *layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
	BLayoutBuilder::Group<>(layout)
		.Add(fAboutBox, 0)
		.AddGlue()
	;

	// TODO this isn't quite right, need to calculate based on TextView preferred size?
//	BSize minSize(PreferredSize());
//	minSize.height += 565;
//	fAboutBox->SetExplicitMinSize(minSize);
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
