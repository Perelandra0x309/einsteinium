/* DaemonStatusView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "DaemonStatusView.h"

DaemonStatusView::DaemonStatusView(BRect size)
	:
	BView(size, "Daemon Status", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fWatchingRoster(false),
	fStatusBox(NULL)
{
	BRect viewRect(0,0,size.Width(), size.Height());

	// About box
//	fAboutBox = new BBox("About");
	fAboutBox = new BBox(viewRect, "About", B_FOLLOW_LEFT_RIGHT);
	fAboutBox->SetLabel("About Einsteinum Daemon");
//	fAboutTextView = new BTextView("About text");
	viewRect.InsetBy(10, 5);
	viewRect.top += 15;
	BRect textRect(0,0,viewRect.Width(), viewRect.Height());
	fAboutTextView = new BTextView(viewRect, "About text", textRect, B_FOLLOW_ALL);
	fAboutTextView->SetText("The Einsteinium Daemon is a background app that watches"
		" all the apps which are running.  The daemon can automatically restart an app which"
		" quits or crashes, provide a prompt to ask whether the app should be restarted, or"
		" allow the app to quit without restarting it.  The Einsteinium Daemon determines the"
		" appropriate action to take for each app by the settings you specify.\n\n"
		"App Relaunch:\n"
		"Use the \"App Relaunch\" section to specify what actions you want the daemon to"
		" take.  The \"Default setting\" item specifies the action to take for any apps"
		" that are not in the list.  Add specific apps to the list to override the default"
		" action for that specific app.  Each specific app will be color coded based on the"
		" action specified, so you can quickly view all your app settings.");
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);
/*	fAboutBox->AddChild(BGroupLayoutBuilder(B_HORIZONTAL, 5)
		.Add(fAboutTextView)
		.SetInsets(5, 5, 5, 5)
	);*/

	// Status Box
	BEntry serviceEntry = GetEntryFromSig(e_daemon_sig);
	if(serviceEntry.Exists())
	{
		fStatusBox = new SystemStatusBox("Daemon Running Status", serviceEntry, e_daemon_sig);
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


DaemonStatusView::~DaemonStatusView()
{
	//Need to stop watching the roster
	if(fWatchingRoster)
		be_roster->StopWatching(BMessenger(this));
}


void
DaemonStatusView::AttachedToWindow()
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
DaemonStatusView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{
		case B_SOME_APP_QUIT:
		case B_SOME_APP_LAUNCHED:
		{
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
			if(strcmp(sig, e_daemon_sig) == 0)//Found a match for the einsteinium engine signature
			{
				BMessenger messenger(fStatusBox);
				messenger.SendMessage(msg);
			}
			break;
		}
		default: {
			BView::MessageReceived(msg);
			break; }
	}
}


void
DaemonStatusView::FrameResized(float width, float height)
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
DaemonStatusView::ResizeStatusBox()
{
	fStatusBox->ResizeStatusText();
}*/
