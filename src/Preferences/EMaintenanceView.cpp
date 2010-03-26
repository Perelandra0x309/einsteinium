/* EMaintenanceView.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "EMaintenanceView.h"

EMaintenanceView::EMaintenanceView(BRect size)
	:
	BView(size, "Engine Maintenance", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fWatchingRoster(false)
{
	SetViewColor(bg_color);
	BEntry serviceEntry = GetEntryFromSig(e_engine_sig);
	fStatusBox = NULL;
	if(serviceEntry.Exists())
	{
		BNode serviceNode;
		BNodeInfo serviceNodeInfo;
		char nodeType[B_MIME_TYPE_LENGTH];
		attr_info info;
		BString sig;
		if( (serviceNode.SetTo(&serviceEntry)) == B_OK) {
			if( (serviceNodeInfo.SetTo(&serviceNode)) == B_OK) {
				if( (serviceNodeInfo.GetType(nodeType)) == B_OK) {
					if( (strcmp(nodeType, "application/x-vnd.Be-elfexecutable") == 0
						|| strcmp(nodeType, "application/x-vnd.be-elfexecutable") == 0 )) {
						if(serviceNode.GetAttrInfo("BEOS:APP_SIG", &info) == B_OK) {
							serviceNode.ReadAttrString("BEOS:APP_SIG", &sig);
							//printf("Found server: %s\n", sig.String());
							fStatusBox = new SystemStatusBox(serviceEntry, sig.String());
							// Override the standard look used in the Services view
							fStatusBox->SetBorder(B_FANCY_BORDER);
							fStatusBox->SetLabel("Engine Status");
						}
					}
				}
			}
		}

	}

	//Engine commands
	fRankBox = new BBox("Rank BBox");
	fRankBox->SetLabel("Rank Commands");
	fRanksB = new BButton("rank", "Recalculate Scores", new BMessage(E_RECALC_SCORES));
	fRanksB->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fRanksTV = new BTextView("Rank Description");
	fRanksTV->SetText("Recalculate the rank score of each application.");
	fRanksTV->SetViewColor(bg_color);
	fRanksTV->MakeSelectable(false);
	fRanksTV->MakeEditable(false);
	fQuartilesB = new BButton("quartiles", "Recalculate Quartiles", new BMessage(E_RECALC_QUARTS));
	fQuartilesTV = new BTextView("Quartiles Description");
	fQuartilesTV->SetText("Update the statitical quartiles for each measure used in the rank calculation.");
	fQuartilesTV->SetViewColor(bg_color);
	fQuartilesTV->MakeSelectable(false);
	fQuartilesTV->MakeEditable(false);
	fDataB = new BButton("data", "Rescan Data", new BMessage(E_RESCAN_DATA));
	fDataB->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fDataTV = new BTextView("Rescan Data Description");
	fDataTV->SetText("Rescan all the application start and quit times from the database to recreate application statistics.");
	fDataTV->SetViewColor(bg_color);
	fDataTV->MakeSelectable(false);
	fDataTV->MakeEditable(false);
	fRankBox->AddChild(BGridLayoutBuilder(5, 5)
		.Add(fRanksB, 0, 0)
		.Add(fRanksTV, 1, 0, 1, 2)
		.Add(fQuartilesB, 0, 2)
		.Add(fQuartilesTV, 1, 2, 1, 2)
		.Add(fDataB, 0, 4)
		.Add(fDataTV, 1, 4, 1, 2)
		.SetInsets(5, 5, 5, 5)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	BGroupLayoutBuilder builder(B_VERTICAL, 10);
	if(fStatusBox != NULL)
		builder.Add(fStatusBox);
	AddChild(builder
		.Add(fRankBox)
		.AddGlue()
	);
}


EMaintenanceView::~EMaintenanceView()
{
	//Need to stop watching the roster
	if(fWatchingRoster)
		be_roster->StopWatching(BMessenger(this));
}


void
EMaintenanceView::AllAttached()
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
EMaintenanceView::MessageReceived(BMessage* msg)
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
				_SetButtonEnabledState();
			}
		}
	}
}


void
EMaintenanceView::ResizeStatusBox()
{
	fStatusBox->ResizeStatusText();
}


void
EMaintenanceView::_SetButtonEnabledState()
{
	bool running = be_roster->IsRunning(e_engine_sig);
	fRanksB->SetEnabled(running);
	fQuartilesB->SetEnabled(running);
	fDataB->SetEnabled(running);
}
