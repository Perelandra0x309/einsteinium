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

	// About box
	fAboutBox = new BBox("About");
	fAboutBox->SetLabel("About Einsteinum Daemon");
	fAboutTextView = new BTextView("About text");
	fAboutTextView->SetText("The Einsteinium Daemon is a background app that watches"
		" all the apps which are running.  The daemon can automatically restart an app which"
		" quits or crashes, provide a prompt to ask whether the app should be restarted, or"
		" allow the app to quit without restarting it.  The Einsteinium Daemon determines the"
		" appropriate action to take for each app by the settings you specify.");
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);

	BGroupLayout *boxLayout = new BGroupLayout(B_VERTICAL);
	fAboutBox->SetLayout(boxLayout);
	BLayoutBuilder::Group<>(boxLayout)
		.Add(fAboutTextView)
		.SetInsets(10, 20, 10, 10)
	;

	// Status Box
	BEntry serviceEntry = GetEntryFromSig(e_daemon_sig);
	if(serviceEntry.Exists())
		fStatusBox = new SystemStatusBox("Daemon Running Status", serviceEntry, e_daemon_sig);

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
	minSize.height += 20;
	fAboutBox->SetExplicitMinSize(minSize);
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
	fAboutBox->Invalidate();
	fAboutTextView->Invalidate();
	fStatusBox->Invalidate();
	InvalidateLayout();
	BView::FrameResized(width, height);
}

