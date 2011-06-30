/* EngineStatusView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "EngineStatusView.h"

EngineStatusView::EngineStatusView(BRect size)
	:
	BView(size, "Engine Status", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fWatchingRoster(false),
	fStatusBox(NULL)
{
	// About box
	fAboutBox = new BBox("About");
	fAboutBox->SetLabel("About Einsteinum Engine");
	fAboutTextView = new BTextView("About text");
	fAboutTextView->SetText("The Einsteinium Engine is a background app which keeps statistics"
		" on every application that runs.  These statistics are available for anyone to use,"
		" and can be used to create ranked lists of applications based on certain criteria."
		"  For example, a list of the most recently run apps (the typical Recent Applications"
		" list), a list of apps with the longest total running time, a list of apps with the"
		" highest number of launches, and more are possible using the statictics gathered by"
		" the Einsteinium Engine.");
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);
	BGroupLayout *boxLayout = new BGroupLayout(B_VERTICAL);
	fAboutBox->SetLayout(boxLayout);
	BLayoutBuilder::Group<>(boxLayout)
		.Add(fAboutTextView)
		.SetInsets(10, 20, 10, 10)
	;

	// Status Box
	SetViewColor(bg_color);
	BEntry serviceEntry = GetEntryFromSig(e_engine_sig);
	if(serviceEntry.Exists())
		fStatusBox = new SystemStatusBox("Engine Running Status", serviceEntry, e_engine_sig);

	// Layout
	BGroupLayout *layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
	BGroupLayoutBuilder builder(layout);
	builder.Add(fAboutBox, 0);
	if(fStatusBox != NULL)
		builder.Add(fStatusBox, 1);
	builder.AddGlue();

	// TODO this isn't quite right, need to calculate based on TextView preferred size?
	BSize minSize(PreferredSize());
	minSize.height += 50;
	fAboutBox->SetExplicitMinSize(minSize);
}


EngineStatusView::~EngineStatusView()
{
	//Need to stop watching the roster
	if(fWatchingRoster)
		be_roster->StopWatching(BMessenger(this));
}


void
EngineStatusView::AttachedToWindow()
{
	SetViewColor(Parent()->ViewColor());
	fAboutTextView->SetViewColor(Parent()->ViewColor());
	BView::AttachedToWindow();

	//Start watching the application roster for launches and quits of services
	status_t result = be_roster->StartWatching(BMessenger(this),
							B_REQUEST_QUIT | B_REQUEST_LAUNCHED);
	if(result  != B_OK)
		//roster failed to be watched.  Show warning, but we can continue.
		(new BAlert("Watching Warning", "Warning: This app was not able to succeesfully start "
					"watching the roster for aplication quit and launch messages.  "
					"The status of the Einsteinium Engine may not be up to date at any time.",
					"OK"))->Go();
	else
		//watching was sucessful
		fWatchingRoster = true;
}


void
EngineStatusView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{
		case B_SOME_APP_QUIT:
		case B_SOME_APP_LAUNCHED:
		{
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
			if(strcmp(sig, e_engine_sig) == 0)//Found a match for the einsteinium engine signature
			{
				BMessenger messenger(fStatusBox);
				messenger.SendMessage(msg);
			}
		}
		default: {
			BView::MessageReceived(msg);
			break; }
	}
}


void
EngineStatusView::FrameResized(float width, float height)
{
	fAboutBox->Invalidate();
	fAboutTextView->Invalidate();
	fStatusBox->Invalidate();
	Invalidate();
	BView::FrameResized(width, height);
}

