/*EMaintenanceView.cpp
	Create view for maintaining engine application data
*/
#include "EMaintenanceView.h"

EMaintenanceView::EMaintenanceView(BRect size)
	:BView(size, "Engine Maintenance", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
	,watchingRoster(false)
{	SetViewColor(bg_color);

	BEntry serviceEntry = getEntryFromSig(e_engine_sig);
	statusBox = NULL;
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
							statusBox = new SystemStatusBox(serviceEntry, sig.String());
							// Override the standard look used in the Services view
							statusBox->SetBorder(B_FANCY_BORDER);
							statusBox->SetLabel("Engine Status");
						}
					}
				}
			}
		}

	}

	//Engine commands
	rankBox = new BBox("Rank BBox");
	rankBox->SetLabel("Rank Commands");
	ranksB = new BButton("rank", "Recalculate Scores", new BMessage(E_RECALC_SCORES));
	ranksB->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	ranksTV = new BTextView("Rank Description");
	ranksTV->SetText("Recalculate the rank score of each application.");
	ranksTV->SetViewColor(bg_color);
	ranksTV->MakeSelectable(false);
	ranksTV->MakeEditable(false);
	quartilesB = new BButton("quartiles", "Recalculate Quartiles", new BMessage(E_RECALC_QUARTS));
	quartilesTV = new BTextView("Quartiles Description");
	quartilesTV->SetText("Update the statitical quartiles for each measure used in the rank calculation.");
	quartilesTV->SetViewColor(bg_color);
	quartilesTV->MakeSelectable(false);
	quartilesTV->MakeEditable(false);
	dataB = new BButton("data", "Rescan Data", new BMessage(E_RESCAN_DATA));
	dataB->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	dataTV = new BTextView("Rescan Data Description");
	dataTV->SetText("Rescan all the application start and quit times from the database to recreate application statistics.");
	dataTV->SetViewColor(bg_color);
	dataTV->MakeSelectable(false);
	dataTV->MakeEditable(false);
	rankBox->AddChild(BGridLayoutBuilder(5, 5)
		.Add(ranksB, 0, 0)
		.Add(ranksTV, 1, 0, 1, 2)
		.Add(quartilesB, 0, 2)
		.Add(quartilesTV, 1, 2, 1, 2)
		.Add(dataB, 0, 4)
		.Add(dataTV, 1, 4, 1, 2)
		.SetInsets(5, 5, 5, 5)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	BGroupLayoutBuilder builder(B_VERTICAL, 10);
	if(statusBox != NULL) { builder.Add(statusBox); }
	AddChild(builder
		.Add(rankBox)
		.AddGlue()
	);

	// TODO check to see if the engine is running.  Do we launch it if it is not?

}

EMaintenanceView::~EMaintenanceView()
{
	//Need to stop watching the roster
	if(watchingRoster)
	{	be_roster->StopWatching(BMessenger(this)); }
}

void EMaintenanceView::AllAttached()
{
	BView::AllAttached();

	//Start watching the application roster for launches and quits of services
	status_t result = be_roster->StartWatching(BMessenger(this));
	if(result  != B_OK)
	{	//roster failed to be watched.  Show warning, but we can continue.
		(new BAlert("Watching Warning", "Warning: This app was not able to succeesfully start "
					"watching the roster for aplication quit and launch messages.  "
					"The status of the Einsteinium Engine may not be up to date at any time.", "OK"))->Go();
	}
	else//watching was sucessful
	{	watchingRoster = true;
	}
}

void EMaintenanceView::MessageReceived(BMessage* msg)
{	switch(msg->what)
	{
		case B_SOME_APP_QUIT:
		case B_SOME_APP_LAUNCHED:
		{
			const char* sig;
			if (msg->FindString("be:signature", &sig) != B_OK) break;
			if(strcmp(sig, e_engine_sig) == 0)//Found a match for the app signature
			{
				BMessenger messenger(statusBox);
				messenger.SendMessage(msg);
			}
		}
	}
}

