/* EEShelfView.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "EEShelfView.h"


EEShelfView::EEShelfView(BRect frame, int16 count)
	:
	BView(frame, EE_SHELFVIEW_NAME, B_FOLLOW_NONE,
		B_WILL_DRAW/* | B_PULSE_NEEDED*/),
	fIcon(NULL),
	fMenu(NULL)
{
	fInitialCount = count;

	app_info info;
	be_app->GetAppInfo(&info);
	fIcon = new BBitmap(Bounds(), B_RGBA32);
	if (fIcon->InitCheck() == B_OK)
	{
		status_t result = BNodeInfo::GetTrackerIcon(&info.ref, fIcon, B_MINI_ICON);
		if(result != B_OK)
		{
			printf("Error getting tracker icon\n");
			delete fIcon;
			fIcon = NULL;
		}
	}
	else
	{
		printf("Error creating bitmap\n");
		delete fIcon;
		fIcon = NULL;
	}

	SetToolTip("Einsteinium  \nRanked\nApplications");
}


EEShelfView::EEShelfView(BMessage *message)
	:
	BView(message),
	fIcon(NULL),
	fMenu(NULL)
{
	BMessage iconArchive;
	status_t result = message->FindMessage("fIconArchive", &iconArchive);
	if(result == B_OK)
		fIcon = new BBitmap(&iconArchive);
	message->FindInt16("count", &fInitialCount);
	// Apparently Haiku does not yet archive tool tips (Release 1 Alpha 2)
	SetToolTip("Einsteinium  \nRanked\nApplications");
}


EEShelfView::~EEShelfView()
{
	delete fIcon;
	delete fMenu;
}


void
EEShelfView::AttachedToWindow()
{
	BView::AttachedToWindow();
	if (Parent())
		SetViewColor(Parent()->ViewColor());
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ViewColor());

	_BuildMenu(NULL);

	fUniqueID = time(NULL);
	if (!be_roster->IsRunning(e_engine_sig)) {
		BDeskbar deskbar;
		deskbar.RemoveItem(EE_SHELFVIEW_NAME);
	}
	else
	{
		// Subscribe with the Einsteinium Engine to receive updates
		BMessage subscribeMsg(E_SUBSCRIBE_RANKED_APPS);
		subscribeMsg.AddInt32("uniqueID", fUniqueID);
		subscribeMsg.AddInt16("count", fInitialCount);
		subscribeMsg.AddMessenger("messenger", BMessenger(this));
		BMessenger EsMessenger(e_engine_sig);
		EsMessenger.SendMessage(&subscribeMsg, this);
		// TODO trying to get reply synchronously freezes
	//	BMessage reply;
	//	EsMessenger.SendMessage(&subscribeMsg, &reply);
		Invalidate();
	}
}


void
EEShelfView::DetachedFromWindow()
{
	// Unsubscribe from the Einsteinium Engine
	if (be_roster->IsRunning(e_engine_sig))
	{
		BMessage unsubscribeMsg(E_UNSUBSCRIBE_RANKED_APPS);
		unsubscribeMsg.AddInt32("uniqueID", fUniqueID);
		BMessenger EsMessenger(e_engine_sig);
		EsMessenger.SendMessage(&unsubscribeMsg, this);
	}
}


EEShelfView*
EEShelfView::Instantiate(BMessage *data)
{
	if (!validate_instantiation(data, "EEShelfView"))
		return NULL;
	return new EEShelfView(data);
}


status_t
EEShelfView::Archive(BMessage *data, bool deep) const
{
	BView::Archive(data, deep);
	data->AddString("add_on", e_engine_sig);
	data->AddString("class", "EEShelfView");
	data->AddInt16("count", fInitialCount);
	if(fIcon != NULL)
	{
		BMessage archive;
		fIcon->Archive(&archive);
		data->AddMessage("fIconArchive", &archive);
	}
	return B_NO_ERROR;
}


void
EEShelfView::Draw(BRect rect)
{
	if (fIcon == NULL)
		return;

	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(fIcon);
	SetDrawingMode(B_OP_COPY);
}


void
EEShelfView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case E_SUBSCRIBE_FAILED: {
			BDeskbar deskbar;
			deskbar.RemoveItem(EE_SHELFVIEW_NAME);
			break;
		}
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
			_BuildMenu(msg);
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


/*
void
EEShelfView::Pulse()
{
	// TODO: Check if einsteinium engine is still running
}*/


void
EEShelfView::MouseDown(BPoint pos)
{
	ConvertToScreen(&pos);
	if (fMenu)
		fMenu->Go(pos, true, true, BRect(pos.x - 2, pos.y - 2,
			pos.x + 2, pos.y + 2), true);
}


void
EEShelfView::_BuildMenu(BMessage *message)
{
	if(fMenu)
		delete fMenu;

	fMenu = new BPopUpMenu(B_EMPTY_STRING, false, false);
	fMenu->SetFont(be_plain_font);

	// Add any refs found
	if(message)
	{
		int32 countFound;
		type_code typeFound;
		message->GetInfo("refs", &typeFound, &countFound);
	//	printf("Found %i refs\n", countFound);
		entry_ref newref;
		for(int i=0; i<countFound; i++)
		{
			message->FindRef("refs", i, &newref);
	//		printf("Found ref %s\n", newref.name);
			BNode refNode(&newref);
			BNodeInfo refNodeInfo(&refNode);
			BMessage *newMsg = new BMessage(E_SHELFVIEW_OPEN);
			newMsg->AddRef("refs", &newref);
			fMenu->AddItem(new IconMenuItem(newref.name, newMsg, &refNodeInfo, B_MINI_ICON));
		}
	}

	// Preferences link
	fMenu->AddSeparatorItem();
	fMenu->AddItem(new BMenuItem("Preferences"B_UTF8_ELLIPSIS,
		new BMessage(E_SHELFVIEW_OPENPREFS)));

	BMenuItem* item;
	BMessage* msg;
	for (int32 i = fMenu->CountItems(); i-- > 0;) {
		item = fMenu->ItemAt(i);
		if (item && (msg = item->Message()) != NULL)
			item->SetTarget(this);
	}
}
