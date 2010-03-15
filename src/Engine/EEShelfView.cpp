/*
 */


#include "EEShelfView.h"

const char* kTrackerSignature = "application/x-vnd.Be-TRAK";


/*
BView* instantiate_deskbar_item(void)
{
	return new EEShelfView(BRect(0, 0, 15, 15));
}
*/



EEShelfView::EEShelfView(BRect frame)
	:BView(frame, EE_SHELFVIEW_NAME, B_FOLLOW_NONE,
		B_WILL_DRAW/* | B_FRAME_EVENTS /*B_PULSE_NEEDED*/),
	fIcon(NULL)
{
	app_info info;
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, B_READ_ONLY);
	
	if (file.InitCheck() != B_OK)
		return;
	
	BResources resources(&file);
	size_t size = 0;
	const uint8* rawIcon;
	rawIcon = (const uint8*)resources.LoadResource(B_VECTOR_ICON_TYPE,
		ES_ICON_ENGINE_SHELF, &size);
	
	if (rawIcon != NULL)
	{
		fIcon = new BBitmap(Bounds(), B_RGBA32);
		if (fIcon->InitCheck() == B_OK)
		{
			if(BIconUtils::GetVectorIcon(rawIcon, size, fIcon) != B_OK)
			{
				printf("Error getting Vector\n");
				delete fIcon;
				fIcon = NULL;
			}
		}
		else
		{
			printf("Error creating bitmap\n");
		}
	}
	else printf("rawIcon was NULL\n");
//	printf("Icon = %i\n", fIcon);

	SetToolTip("Einsteinium  \nRanked\nApplications");
}


EEShelfView::EEShelfView(BMessage *message)
	:BView(message),
	fIcon(NULL)
{
	BMessage iconArchive;
	status_t result = message->FindMessage("fIconArchive", &iconArchive);
	if(result == B_OK)
	{
		fIcon = new BBitmap(&iconArchive);
	}
	// Apparently Haiku does not yet archive tool tips (Alpha-1 R1)
	SetToolTip("Einsteinium  \nRanked\nApplications");
}


EEShelfView::~EEShelfView()
{
	if(fIcon)
		delete fIcon;
}

// TODO Detached from Window- unsubscribe


void EEShelfView::AttachedToWindow()
{
	BView::AttachedToWindow();
	if (Parent())
		SetViewColor(Parent()->ViewColor());
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ViewColor());
	
	if (!be_roster->IsRunning(e_engine_sig)) {
		BDeskbar deskbar;
		deskbar.RemoveItem(EE_SHELFVIEW_NAME);
	}
	else
	{
		// Subscribe with the Einsteinium Engine to receive updates
		BMessage subscribeMsg(E_SUBSCRIBE_RANKED_APPS);
		subscribeMsg.AddInt16("count", 20);
		subscribeMsg.AddMessenger("messenger", BMessenger(this));
		BMessenger EsMessenger(e_engine_sig);
		EsMessenger.SendMessage(&subscribeMsg);
		
		Invalidate();
	}
}


EEShelfView* EEShelfView::Instantiate(BMessage *data)
{
	if (!validate_instantiation(data, "EEShelfView"))
		return NULL;
	return new EEShelfView(data);
}


status_t EEShelfView::Archive(BMessage *data, bool deep) const
{
	BView::Archive(data, deep);
	data->AddString("add_on", e_engine_sig);
	data->AddString("class", "EEShelfView");
	if(fIcon != NULL)
	{
		BMessage archive;
		fIcon->Archive(&archive);
		data->AddMessage("fIconArchive", &archive);
	}
	return B_NO_ERROR;
}


void EEShelfView::Draw(BRect rect)
{
	if (fIcon == NULL)
		return;
	
	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(fIcon);
	SetDrawingMode(B_OP_COPY);
}


void EEShelfView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case E_SHELFVIEW_OPENPREFS: {
			be_roster->Launch("application/x-vnd.Einsteinium_Preferences");
			break;
		}
		case E_SHELFVIEW_OPEN: {
			entry_ref ref;
			if(msg->FindRef("refs", &ref) == B_OK)
				be_roster->Launch(&ref);
			break;
		}
		case E_SUBSCRIBER_UPDATE_RANKED_APPS: {
//			printf("Received test subscribe reply from Es\n");
			_BuildMenu(msg);
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


/*
void EEShelfView::Pulse()
{
	// TODO: Check if einsteinium engine is still running
}*/


void EEShelfView::MouseDown(BPoint pos)
{
	ConvertToScreen(&pos);
	if (fMenu) {
		fMenu->Go(pos, true, true, BRect(pos.x - 2, pos.y - 2,
			pos.x + 2, pos.y + 2), true);
	}
}

BPopUpMenu* EEShelfView::_BuildMenu(BMessage *message)
{

	fMenu = new BPopUpMenu(B_EMPTY_STRING, false, false);
	fMenu->SetFont(be_plain_font);
	
	// Add any refs found
	int32 countFound;
	type_code typeFound;
	message->GetInfo("refs", &typeFound, &countFound);
	printf("Found %i refs\n", countFound);
	entry_ref newref;
	for(int i=0; i<countFound; i++)
	{
		message->FindRef("refs", i, &newref);
		printf("Found ref %s\n", newref.name);
		BNode refNode(&newref);
		BNodeInfo refNodeInfo(&refNode);
		BMessage *newMsg = new BMessage(E_SHELFVIEW_OPEN);
		newMsg->AddRef("refs", &newref);
		fMenu->AddItem(new IconMenuItem(newref.name, newMsg, &refNodeInfo, B_MINI_ICON));
		// TODO how to get a vector icon?
	}
	
	fMenu->AddSeparatorItem();
	fMenu->AddItem(new BMenuItem("Preferences"B_UTF8_ELLIPSIS,
		new BMessage(E_SHELFVIEW_OPENPREFS)));

//	BMessenger tracker(kTrackerSignature);
	BMenuItem* item;
	BMessage* msg;
	for (int32 i = fMenu->CountItems(); i-- > 0;) {
		item = fMenu->ItemAt(i);
		if (item && (msg = item->Message()) != NULL) {
			//if (msg->what == B_REFS_RECEIVED)
			//	item->SetTarget(tracker);
			//else
				item->SetTarget(this);
		}
	}
	return fMenu;
}
