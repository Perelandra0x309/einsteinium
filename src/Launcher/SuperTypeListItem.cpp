/* SuperTypeListItem.cpp
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "SuperTypeListItem.h"


SuperTypeListItem::SuperTypeListItem(BMimeType *type, int docIconSize)
	:
	BListItem(),
	fInitStatus(B_ERROR)
{
	// Draw Settings
	fIconSize = docIconSize;
	fIcon = NULL;
	fShadowIcon = NULL;

	fMimeType.SetTo(type->Type());
	fName.SetTo(type->Type());
	fName.Capitalize();

	_GetIcon();
	fInitStatus = B_OK;
}


SuperTypeListItem::~SuperTypeListItem()
{
	delete fIcon;
	delete fShadowIcon;
}


void
SuperTypeListItem::SetIconSize(int value)
{
	fIconSize = value;
	_GetIcon();
}


void
SuperTypeListItem::_GetIcon()
{
	delete fIcon;
	delete fShadowIcon;
	fIcon = NULL;
	fShadowIcon = NULL;
	if(fIconSize == 0)
		return;

//	BNode node;
	status_t result = B_ERROR;
/*	if (node.SetTo(&fEntryRef) == B_OK) {
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

	}*/
	BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
	fIcon = new BBitmap(iconRect, 0, B_RGBA32);
	result = fMimeType.GetIcon(fIcon, icon_size(fIconSize));

	if(result!=B_OK)
	{
		delete fIcon;
		fIcon = NULL;
	}
}

/*
BBitmap*
SuperTypeListItem::_ConvertToGrayscale(const BBitmap* bitmap) const
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


void
SuperTypeListItem::DrawItem(BView *owner, BRect item_rect, bool complete = false)
{
	float offset_width = 0, offset_height = fFontAscent;
	float listItemHeight = Height();
	rgb_color backgroundColor;

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
		float offsetMarginHeight = floor( (listItemHeight - fIconSize)/2);
		float offsetMarginWidth = kIconMargin;

		//owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		owner->SetDrawingMode(B_OP_ALPHA);
		if(fIcon->Bounds().IntegerWidth()+1 != fIconSize)
		{
//			printf("Bounds width=%i, iconSize=%i\n", fIcon->Bounds().IntegerWidth()+1,fIconSize);
			BRect destRect(item_rect.left + offsetMarginWidth,
							item_rect.top + offsetMarginHeight,
							item_rect.left + offsetMarginWidth + fIconSize,
							item_rect.top + offsetMarginHeight + fIconSize);
			owner->DrawBitmap(fIcon, destRect);
		}
		else
		{
			owner->DrawBitmap(fIcon, BPoint(item_rect.left + offsetMarginWidth,
							item_rect.top + offsetMarginHeight));
		}
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
	owner->DrawString(fName);
}



void
SuperTypeListItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner, font);
	_UpdateHeight(font);
}


void
SuperTypeListItem::_UpdateHeight(const BFont *font)
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
