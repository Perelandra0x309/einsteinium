/* AppListItem.cpp
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "AppListItem.h"


AppListItem::AppListItem(BEntry entry, const char * sig, AppSettings *settings)
	:
	BListItem(),
//	fDrawTwoLines(false),
	fRemoveName("Exclude "),
	fInitStatus(B_ERROR)
{
	// Draw Settings
//	fDrawTwoLines = settings->drawTwoLines;
	fIconSize = settings->maxIconSize;
	fIcon = NULL;
	fShadowIcon = NULL;

	if(!entry.Exists())
		return;

	BPath path;
	entry.GetPath(&path);
	fName.SetTo(path.Leaf());
	fRemoveName.Append(fName);
	fSignature.SetTo(sig);
	entry.GetRef(&fEntryRef);
	fNameFirstChar = BString(fName).Truncate(1).ToLower()[0];

//	_GetIcon();
	fInitStatus = B_OK;
}


AppListItem::~AppListItem()
{
	delete fIcon;
	delete fShadowIcon;
}


void
AppListItem::ProcessMessage(BMessage* msg)
{
	switch(msg->what)
	{
		case EL_START_SERVICE:
		{
			status_t rc = _StartService();
			break;
		}
		case EL_STOP_SERVICE:
		{
			status_t rc = _StopService();
			break;
		}
	}
}


/*
void
AppListItem::SetDrawTwoLines(bool value)
{
	fDrawTwoLines = value;
}
*/

void
AppListItem::SetIconSize(int value)
{
	fIconSize = value;
	_GetIcon();
}


void
AppListItem::SetIconSize(int minIconSize, int maxIconSize, int totalCount, int index)
{
	int iconSize = int( maxIconSize - (index*(maxIconSize-minIconSize)/(totalCount-1)) );
	SetIconSize(iconSize);
}


bool
AppListItem::IsRunning()
{
	return be_roster->IsRunning(fSignature.String());
}


status_t
AppListItem::Launch()
{
	return _StartService();
}


status_t
AppListItem::ShowInTracker()
{
	BEntry folderEntry;
	BEntry fileEntry(&fEntryRef);
	fileEntry.GetParent(&folderEntry);
	entry_ref folderRef;
	folderEntry.GetRef(&folderRef);
	BMessenger tracker("application/x-vnd.Be-TRAK");
	BMessage message(B_REFS_RECEIVED);
	message.AddRef("refs", &folderRef);
	status_t rc = tracker.SendMessage(&message);
	return rc;
}


BString
AppListItem::GetPath()
{
	BPath docPath(&fEntryRef);
	BString pathStr(docPath.Path());
	return pathStr;
}


bool
AppListItem::BeginsWith(char letter)
{
	return (fNameFirstChar==letter);
}


void
AppListItem::_GetIcon()
{
	delete fIcon;
	delete fShadowIcon;
	fIcon = NULL;
	fShadowIcon = NULL;
	if(fIconSize == 0)
		return;

	BNode node;
	status_t result = B_ERROR;
	if (node.SetTo(&fEntryRef) == B_OK) {
		BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
		fIcon = new BBitmap(iconRect, 0, B_RGBA32);
		BNodeInfo nodeInfo(&node);
		result = nodeInfo.GetTrackerIcon(fIcon, icon_size(fIconSize));
		if(result!=B_OK)
		{
			// Get icon from mime type
			char mimeString[B_MIME_TYPE_LENGTH];
			uint8* data;
			size_t size;
			BMimeType nodeType;
			if(nodeInfo.GetType(mimeString) == B_OK)
			{
				nodeType.SetTo(mimeString);
				nodeType.GetIcon(&data, &size);
				result = BIconUtils::GetVectorIcon(data, size, fIcon);
			}
			// Get super type icon
			if(result!=B_OK)
			{
				BMimeType superType;
				if(nodeType.GetSupertype(&superType)==B_OK)
				{
					superType.GetIcon(&data, &size);
					result = BIconUtils::GetVectorIcon(data, size, fIcon);
				}

			}
		}

	/*	if(result==B_OK)
		{
			iconRect.Set(0, 0, fIconSize + 2*kIconMargin - 1, fIconSize + 2*kIconMargin - 1);
			BBitmap tempIcon = new BBitmap(iconRect, 0, B_RGBA32);
			result = BIconUtils::GetVectorIcon(&node, "BEOS:ICON", &tempIcon);
			if(result==B_OK)
				fShadowIcon = _ConvertToGrayscale(&tempIcon);

		}*/
	}
	if(result!=B_OK)
	{
		delete fIcon;
		fIcon = NULL;
	}
/*	delete fIcon;
	delete fShadowIcon;
	if(fIconSize == 0)
	{
		fIcon = NULL;
		fShadowIcon = NULL;
		return;
	}
	BNode node;
	BNodeInfo nodeInfo;
	if (node.SetTo(&fEntryRef) == B_OK) {
		BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
		fIcon = new BBitmap(iconRect, 0, B_RGBA32);
		status_t result = BIconUtils::GetVectorIcon(&node, "BEOS:ICON", fIcon);
		if(result != B_OK)
		{
			// attempt to get mini or large icon
			delete fIcon;
			if(nodeInfo.SetTo(&node) == B_OK)
			{
				if(fIconSize<32)
				{
					iconRect.Set(0, 0, 15, 15);
					fIcon = new BBitmap(iconRect, 0, B_RGBA32);
					result = nodeInfo.GetTrackerIcon(fIcon, B_MINI_ICON);
				}
				else
				{
					iconRect.Set(0, 0, 31, 31);
					fIcon = new BBitmap(iconRect, 0, B_RGBA32);
					result = nodeInfo.GetTrackerIcon(fIcon, B_LARGE_ICON);
				}
				if(result != B_OK)
				{
					delete fIcon;
					fIcon = NULL;
				}
			}
			else
				fIcon = NULL;
		}
	}
	else {
		fIcon = NULL;
		fShadowIcon = NULL;
	}*/
}

/*
BBitmap*
AppListItem::_ConvertToGrayscale(const BBitmap* bitmap) const
{
    BBitmap* convertedBitmap = new BBitmap(bitmap->Bounds(),
            B_BITMAP_ACCEPTS_VIEWS, B_GRAY8);

    if (convertedBitmap && convertedBitmap->IsValid()) {
        memset(convertedBitmap->Bits(), 0, convertedBitmap->BitsLength());
        BView* helper = new BView(bitmap->Bounds(), "helper", B_FOLLOW_NONE, B_WILL_DRAW);
        if (convertedBitmap->Lock()) {
            convertedBitmap->AddChild(helper);
            helper->SetDrawingMode(B_OP_OVER);
            helper->DrawBitmap(bitmap, BPoint(0.0, 0.0));
            helper->Sync();
            convertedBitmap->Unlock();
        }
    } else {
        delete convertedBitmap;
        convertedBitmap = NULL;
    }

    return convertedBitmap;
}*/

status_t
AppListItem::_StartService()
{
	status_t rc = be_roster->Launch(fSignature.String());
	return rc;
}


status_t
AppListItem::_StopService()
{
	status_t rc = B_ERROR;
	BMessenger appMessenger(fSignature.String());
	if(!appMessenger.IsValid())
	{
		BString err("Error: Could not create messenger for service ");
		err.Append(fName);
		(new BAlert("", err.String(),"OK"))->Go(NULL);
		return rc;
	}
	rc = appMessenger.SendMessage(B_QUIT_REQUESTED);
	return rc;
}


void
AppListItem::DrawItem(BView *owner, BRect item_rect, bool complete = false)
{
	float offset_width = 0, offset_height = fFontAscent;
	float listItemHeight = Height();
	rgb_color backgroundColor;
	uint32 modifier = modifiers();
	bool showRemoveLabel = modifier & kExcludeAppModifier;//modifier == B_LEFT_CONTROL_KEY || modifier == B_RIGHT_CONTROL_KEY;

	//background
	if(IsSelected()) {
		backgroundColor = ui_color(B_MENU_SELECTED_BACKGROUND_COLOR);
	}
	else {
		backgroundColor = ui_color(B_DOCUMENT_BACKGROUND_COLOR);
	}
	owner->SetHighColor(backgroundColor);
	owner->SetLowColor(backgroundColor);
	owner->FillRect(item_rect);

	//icon
/*	if (IsSelected() && fShadowIcon) {
		float offsetMarginHeight = floor( (listItemHeight - fShadowIcon->Bounds().Height())/2);
		float offsetMarginWidth = floor( (fIconSize - fShadowIcon->Bounds().Width())/2 ) + kIconMargin;
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmap(fShadowIcon, BPoint(item_rect.left + offsetMarginWidth,
							item_rect.top + offsetMarginHeight));
		owner->SetDrawingMode(B_OP_COPY);
//		offset_width += fIconSize + 2*kIconMargin;
	}*/
	if (fIcon) {
//		float offsetMarginHeight = floor( (listItemHeight - fIcon->Bounds().Height())/2);
//		float offsetMarginWidth = floor( (fIconSize - fIcon->Bounds().Width())/2 ) + kIconMargin;
		float offsetMarginHeight = floor( (listItemHeight - fIconSize)/2);
		float offsetMarginWidth = kIconMargin;
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmap(fIcon, BPoint(item_rect.left + offsetMarginWidth,
							item_rect.top + offsetMarginHeight));
		owner->SetDrawingMode(B_OP_COPY);
		offset_width += fIconSize + 2*kIconMargin;
	}
	// no icon, give the text some margin space
	else
	{
		offset_width += kTextMargin;
	}

	//text
	if(listItemHeight > fTextOnlyHeight)
		offset_height += floor( (listItemHeight - fTextOnlyHeight)/2 );
			// center the text vertically
	BPoint cursor(item_rect.left + offset_width, item_rect.top + offset_height + kTextMargin);
	owner->SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
	owner->MovePenTo(cursor.x, cursor.y);
	if(showRemoveLabel)
		owner->DrawString(fRemoveName);
	else
		owner->DrawString(fName);
/*	if(fDrawTwoLines)
		owner->MovePenTo(cursor.x, cursor.y + fFontHeight + kTextMargin);
	else
		owner->DrawString(" ");
	owner->DrawString(fStatusText);*/
}



void
AppListItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner, font);
	_UpdateHeight(font);
}


void
AppListItem::_UpdateHeight(const BFont *font)
{
	font_height fontHeight;
	font->GetHeight(&fontHeight);
	fFontHeight = fontHeight.ascent + fontHeight.descent + fontHeight.leading;
	fFontAscent = fontHeight.ascent;
//	if(fDrawTwoLines)
//		fTextOnlyHeight = 2*fFontHeight + 3*kTextMargin;
//	else
		fTextOnlyHeight = fFontHeight + 2*kTextMargin;

	if(fIcon) {
		float iconSpacing = fIconSize + 2*kIconMargin;
		if(iconSpacing > fTextOnlyHeight)
			SetHeight(iconSpacing);
		else
			SetHeight(fTextOnlyHeight);
	}
	else
		SetHeight(fTextOnlyHeight);
}
