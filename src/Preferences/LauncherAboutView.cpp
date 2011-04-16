/* LauncherAboutView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "LauncherAboutView.h"

LauncherAboutView::LauncherAboutView(BRect size)
	:
	BView(size, "Launcher Status", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	BRect viewRect(0,0,size.Width(), size.Height());

	// About box
//	fAboutBox = new BBox("About");
	fAboutBox = new BBox(viewRect, "About", B_FOLLOW_LEFT_RIGHT);
	fAboutBox->SetLabel("About Einsteinum Launcher");
//	fAboutTextView = new BTextView("About text");
	viewRect.InsetBy(10, 5);
	viewRect.top += 15;
	BRect textRect(0,0,viewRect.Width(), viewRect.Height());
	fAboutTextView = new BTextView(viewRect, "About text", textRect, B_FOLLOW_ALL);
	// TODO add text about exclusions
	fAboutTextView->SetText("The Einsteinium Launcher is a menu of"
		" applications that resides in the deskbar.  This menu uses the Einsteinium Engine"
		" to retrieve a list of ranked applications which are ordered by criteria set in"
		" this preferences application.  Each application in the Launcher menu list can be"
		" run by clicking on that menu item.\n\n"
		"Deskbar Menu:\n"
		"Under the \"Deskbar Menu\" settings, change the \"Show this many applications:\" value"
		" to set how many applications are listed in the deskbar menu.\n\n"
		"App Rankings:\n"
		"To set the criteria that determines the application order in the deskbar menu,"
		" select the \"App Rankings\" item in the left side panel.  Then move each slider"
		" to select what weight you want to give each criteria.  For example, if you want"
		" a list of the applications run the most number of times, move the slider under"
		" \"Total number of launches:\" to the right and set all other sliders so they say"
		" \"...will have no effect\".  Then click the \"Save and Recalculate Scores\" button"
		" and the Launcher menu will be updated with your new criteria.");
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);

/*	fAboutBox->AddChild(BGroupLayoutBuilder(B_HORIZONTAL, 5)
		.Add(fAboutTextView)
		.SetInsets(5, 5, 5, 5)
	);

	// Layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.Add(fAboutBox)
		.AddGlue()
	);*/
	fAboutBox->AddChild(fAboutTextView);
	AddChild(fAboutBox);
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
/*	BSize size = fAboutTextView->ExplicitMinSize();
	size.height = fAboutTextView->TextHeight(0, fAboutTextView->TextLength());
	fAboutTextView->SetExplicitMinSize(size);
	fAboutTextView->SetExplicitMaxSize(size);*/
	BRect textRect = fAboutTextView->Bounds();
	fAboutTextView->SetTextRect(textRect);
	textRect.bottom = fAboutTextView->TextHeight(0, fAboutTextView->TextLength());
	fAboutTextView->SetTextRect(textRect);
	fAboutBox->ResizeTo(fAboutBox->Frame().Width(), textRect.bottom + 25);
	fAboutBox->Invalidate();
	fAboutTextView->Invalidate();
	Invalidate();
	BView::FrameResized(width, height);
}
