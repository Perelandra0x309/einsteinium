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
	BRect viewRect(0,0,size.Width(), size.Height());

	// About box
//	fAboutBox = new BBox("About");
	fAboutBox = new BBox(viewRect, "About", B_FOLLOW_LEFT_RIGHT);
	fAboutBox->SetLabel("About Einsteinum Engine");
//	fAboutTextView = new BTextView("About text");
	viewRect.InsetBy(10, 5);
	viewRect.top += 15;
	BRect textRect(0,0,viewRect.Width(), viewRect.Height());
	fAboutTextView = new BTextView(viewRect, "About text", textRect, B_FOLLOW_ALL);
	fAboutTextView->SetText("The Einsteinium Engine is a background app which keeps statistics"
		" on every application that runs.  These statistics are available for anyone to use,"
		" and can be used to create ranked lists of applications based on certain criteria."
		"  For example, a list of the most recently run apps (the typical Recent Applications"
		" list), a list of apps with the longest total running time, a list of apps with the"
		" highest number of launches, and more are possible using the statictics gathered by"
		" the Einsteinium Engine.\n\n"
		"Maintenance:\n"
		"If you are having trouble with applications not ranking properly you can try running"
		" a rescan of the database in the Maintenance section.");
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);
/*	fAboutBox->AddChild(BGroupLayoutBuilder(B_HORIZONTAL, 5)
		.Add(fAboutTextView)
		.SetInsets(5, 5, 5, 5)
	);*/

	// Status Box
	SetViewColor(bg_color);
	BEntry serviceEntry = GetEntryFromSig(e_engine_sig);
	if(serviceEntry.Exists())
	{
		fStatusBox = new SystemStatusBox("Engine Running Status", serviceEntry, e_engine_sig);
		fStatusBox->ResizeToPreferred();
	}

	// Layout
/*	SetLayout(new BGroupLayout(B_HORIZONTAL));
	BGroupLayoutBuilder builder(B_VERTICAL, 10);
	builder.Add(fAboutBox);
	if(fStatusBox != NULL)
		builder.Add(fStatusBox);
	AddChild(builder
		.AddGlue()
	);*/

	fAboutBox->AddChild(fAboutTextView);
	AddChild(fAboutBox);
	if(fStatusBox)
		AddChild(fStatusBox);
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
/*	BSize size = fAboutTextView->ExplicitMinSize();
	size.height = fAboutTextView->TextHeight(0, fAboutTextView->TextLength());
	fAboutTextView->SetExplicitMinSize(size);
	fAboutTextView->SetExplicitMaxSize(size);*/
	BRect textRect = fAboutTextView->Bounds();
	fAboutTextView->SetTextRect(textRect);
	textRect.bottom = fAboutTextView->TextHeight(0, fAboutTextView->TextLength());
	fAboutTextView->SetTextRect(textRect);
	BRect aboutFrame = fAboutBox->Frame();
	fAboutBox->ResizeTo(aboutFrame.Width(), textRect.bottom + 25);
	fStatusBox->MoveTo(aboutFrame.left, aboutFrame.bottom + 10);
	fStatusBox->ResizeTo(aboutFrame.Width(), fStatusBox->Frame().Height());
	fStatusBox->ResizeStatusText();
	fAboutBox->Invalidate();
	fAboutTextView->Invalidate();
	fStatusBox->Invalidate();
	Invalidate();
	BView::FrameResized(width, height);
}

/*
void
EngineStatusView::ResizeStatusBox()
{
	fStatusBox->ResizeStatusText();
}*/
