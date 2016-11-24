/* DaemonStatusView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "DaemonStatusView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Daemon status view"

DaemonStatusView::DaemonStatusView(BRect size)
	:
	BView(size, "Daemon Status", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fWatchingRoster(false),
	fStatusBox(NULL)
{
	// Boot settings file
	find_directory(B_USER_SETTINGS_DIRECTORY, &fBootSettingsPath);
	fBootSettingsPath.Append("Einsteinium/boot");
	
	// About box
	fAboutBox = new BBox("About");
	fAboutBox->SetLabel(B_TRANSLATE_COMMENT("About Einsteinum Daemon", "Box label"));
	fAboutTextView = new BTextView("About text");
	fAboutTextView->SetText(B_TRANSLATE_COMMENT("The Einsteinium Daemon is a background app that watches"
		" all the apps which are running.  The daemon can automatically restart an app which"
		" quits or crashes, provide a prompt to ask whether the app should be restarted, or"
		" allow the app to quit without restarting it.  The Einsteinium Daemon determines the"
		" appropriate action to take for each app by the settings you specify.", "Description text"));
	fAboutTextView->MakeSelectable(false);
	fAboutTextView->MakeEditable(false);

	BLayoutBuilder::Group<>(fAboutBox, B_VERTICAL, 10)
		.Add(fAboutTextView, 1)
		.SetInsets(10, 20, 10, 10);

	// Status Box
	BEntry serviceEntry = GetEntryFromSig(e_daemon_sig);
	if(serviceEntry.Exists())
		fStatusBox = new SystemStatusBox(B_TRANSLATE_COMMENT("Daemon Running Status", "Box label"), serviceEntry, e_daemon_sig);
	
	// Boot launch setting
	fLaunchCB = new BCheckBox("launch", B_TRANSLATE_COMMENT("Start Daemon when Haiku boots", "Checkbox label"),
								new BMessage(BOOT_SETTINGS_CHANGED));
	BFile bootSettings(fBootSettingsPath.Path(), B_READ_WRITE | B_CREATE_FILE);
	if(bootSettings.InitCheck() == B_OK)
	{
		BString value;
		bootSettings.ReadAttrString("launch_daemon", &value);
		if(value=="True")
			fLaunchCB->SetValue(1);
		else if(value=="False")
			fLaunchCB->SetValue(0);
		else{
			fLaunchCB->SetValue(1);
			value.SetTo("True");
			bootSettings.WriteAttrString("launch_daemon", &value);
		}
	}
	
	// Layout
	BGroupLayout *layout = new BGroupLayout(B_VERTICAL, 10);
	SetLayout(layout);
	BGroupLayoutBuilder builder(layout);
	builder.Add(fAboutBox, 0);
	if(fStatusBox != NULL)
		builder.Add(fStatusBox, 0);
	builder.Add(fLaunchCB, 0).AddGlue();
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
	fLaunchCB->SetTarget(this);

	//Start watching the application roster for launches and quits of services
	status_t result = be_roster->StartWatching(BMessenger(this),
							B_REQUEST_QUIT | B_REQUEST_LAUNCHED);
	if(result  != B_OK)
		//roster failed to be watched.  Show warning, but we can continue.
		(new BAlert("Watching Warning", B_TRANSLATE_COMMENT("Warning: This app was not able to succeesfully start "
					"watching the roster for aplication quit and launch messages.  "
					"The status of the Einsteinium Engine may not be up to date at any time.", "Alert message text"),
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
		case BOOT_SETTINGS_CHANGED: {
			BFile bootSettings(fBootSettingsPath.Path(), B_READ_WRITE | B_CREATE_FILE);
			if(bootSettings.InitCheck() == B_OK)
			{
				BString value = fLaunchCB->Value() ? "True" : "False";
				bootSettings.WriteAttrString("launch_daemon", &value);
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

