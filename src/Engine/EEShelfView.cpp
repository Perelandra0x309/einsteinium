/* DeskbarView - mail_daemon's deskbar menu and view
 *
 * Copyright 2001 Dr. Zoidberg Enterprises. All rights reserved.
 */


#include "EEShelfView.h"

//const char* kTrackerSignature = "application/x-vnd.Be-TRAK";


/*
BView* instantiate_deskbar_item(void)
{
	return new EEShelfView(BRect(0, 0, 15, 15));
}
*/



EEShelfView::EEShelfView(BRect frame)
	:BView(frame, EE_SHELFVIEW_NAME, B_FOLLOW_NONE,
		B_WILL_DRAW | B_FRAME_EVENTS /*B_PULSE_NEEDED*/),
	fIcon(NULL)
{
	_InitBitmaps();
}


EEShelfView::EEShelfView(BMessage *message)
	:BView(message),
	fIcon(NULL)
{
	_InitBitmaps();
}


EEShelfView::~EEShelfView()
{
	if(fIcon)
		delete fIcon;
}


void EEShelfView::AttachedToWindow()
{
	BView::AttachedToWindow();
/*	if (Parent())
		SetViewColor(Parent()->ViewColor());
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ViewColor());*/

	if (!be_roster->IsRunning(e_engine_sig)) {
		BDeskbar deskbar;
		deskbar.RemoveItem(EE_SHELFVIEW_NAME);
	}
	else
	{
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
	return B_NO_ERROR;
}


void EEShelfView::Draw(BRect rect)
{
	printf("Drawing shelf\n");
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
		case E_SHELFVIEW_OPEN:
			be_roster->Launch("application/x-vnd.Einsteinium_Preferences");
			break;
		default:
			BView::MessageReceived(msg);
	}
}


void EEShelfView::_InitBitmaps()
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
		if(BIconUtils::GetVectorIcon(rawIcon, size, fIcon) != B_OK)
		{
			printf("Error getting Vector\n");
			delete fIcon;
			fIcon = NULL;
		}
	}
	printf("Icon = %i\n", fIcon);
}

/*
void EEShelfView::Pulse()
{
	// TODO: Check if einsteinium engine is still running
}*/


void EEShelfView::MouseDown(BPoint pos)
{
	ConvertToScreen(&pos);

	// TODO don't need to rebuild menu every time
	BPopUpMenu* menu = _BuildMenu();
	if (menu) {
		menu->Go(pos, true, true, BRect(pos.x - 2, pos.y - 2,
			pos.x + 2, pos.y + 2), true);
	}
}

BPopUpMenu* EEShelfView::_BuildMenu()
{
	BPopUpMenu* menu = new BPopUpMenu(B_EMPTY_STRING, false, false);
	menu->SetFont(be_plain_font);

//	BMessenger tracker(kTrackerSignature);
	BMenuItem* item;
	BMessage* msg;
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Preferences"B_UTF8_ELLIPSIS,
		new BMessage(E_SHELFVIEW_OPEN)));

	for (int32 i = menu->CountItems(); i-- > 0;) {
		item = menu->ItemAt(i);
		if (item && (msg = item->Message()) != NULL) {
			//if (msg->what == B_REFS_RECEIVED)
			//	item->SetTarget(tracker);
			//else
				item->SetTarget(this);
		}
	}
	return menu;
}
