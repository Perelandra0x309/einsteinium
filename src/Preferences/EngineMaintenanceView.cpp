/* EngineMaintenanceView.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "EngineMaintenanceView.h"

EngineMaintenanceView::EngineMaintenanceView(BRect size)
	:
	BView(size, "Engine Maintenance", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fWatchingRoster(false)
{
	BRect viewRect(0,0,size.Width(), size.Height());

	//Engine commands
//	fMaintBox = new BBox("Maintenance BBox");
	fMaintBox = new BBox(viewRect, "Maintenance BBox", B_FOLLOW_LEFT_RIGHT);
	fMaintBox->SetLabel("Database Maintenance");
	fDataB = new BButton("data", "Rescan Data", new BMessage(E_RESCAN_DATA));
	fDataB->ResizeToPreferred();
//	fDataTV = new BTextView("Rescan Data Description");
	viewRect.InsetBy(10, 5);
	viewRect.top += 15;
	BRect textRect(0,0,viewRect.Width(), viewRect.Height());
	fDataTV = new BTextView(viewRect, "Rescan Data Description", textRect, B_FOLLOW_ALL);
	fDataTV->SetText("Press the \"Rescan Data\" button below to have the Engine rescan the database to recreate "
		"application statistics.  The Engine must be running to perform this task.");
	fDataTV->MakeSelectable(false);
	fDataTV->MakeEditable(false);

/*	fMaintBox->AddChild(BGroupLayoutBuilder(B_VERTICAL, 5)
		.Add(fDataTV)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL, 5)
			.AddGlue()
			.Add(fDataB)//, 0, 4)
			.AddGlue()
		)
		.SetInsets(5, 5, 5, 5)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.Add(fMaintBox)
		.AddGlue()
	);*/
	fMaintBox->AddChild(fDataTV);
	fMaintBox->AddChild(fDataB);
	AddChild(fMaintBox);
}


EngineMaintenanceView::~EngineMaintenanceView()
{
	//Need to stop watching the roster
	if(fWatchingRoster)
		be_roster->StopWatching(BMessenger(this));
}


void
EngineMaintenanceView::AllAttached()
{
	BView::AllAttached();

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

	_SetButtonEnabledState();
}


void
EngineMaintenanceView::AttachedToWindow()
{
	SetViewColor(Parent()->ViewColor());
	fDataTV->SetViewColor(Parent()->ViewColor());
	BView::AttachedToWindow();
}


void
EngineMaintenanceView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{
		case B_SOME_APP_QUIT:
		case B_SOME_APP_LAUNCHED:
		{
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
			if(strcmp(sig, e_engine_sig) == 0)//Found a match for the einsteinium engine signature
			{
				_SetButtonEnabledState();
			}
		}
		default: {
			BView::MessageReceived(msg);
			break; }
	}
}


void
EngineMaintenanceView::FrameResized(float width, float height)
{
	BRect textRect = fDataTV->Bounds();
	fDataTV->SetTextRect(textRect);
	textRect.bottom = fDataTV->TextHeight(0, fDataTV->TextLength());
	fDataTV->SetTextRect(textRect);
	fDataTV->ResizeTo(fDataTV->Bounds().Width(), textRect.Height());
	fDataB->MoveTo((textRect.Width() - fDataB->Frame().Width())/2.0, fDataTV->Frame().bottom + 10);
	fMaintBox->ResizeTo(fMaintBox->Frame().Width(), fDataB->Frame().bottom + 10);
	fDataB->Invalidate();
	fDataTV->Invalidate();
	fMaintBox->Invalidate();
	BView::FrameResized(width, height);
}


void
EngineMaintenanceView::_SetButtonEnabledState()
{
	bool running = be_roster->IsRunning(e_engine_sig);
	fDataB->SetEnabled(running);
}
