/* BitmapListItem.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "BitmapListItem.h"


BitmapListItem::BitmapListItem(const uint8 *iconBits, BRect iconRect,
					const color_space colorSpace, const char *text)
	:
	BStringItem(text),
	fIcon(NULL)
{
	fIcon = new BBitmap(iconRect, colorSpace);
	fIcon->SetBits(iconBits, fIcon->BitsLength(), 0, colorSpace);
	fIconSize = fIcon->Bounds().Height();
}


BitmapListItem::BitmapListItem(const char *signature, const char *text)
	:
	BStringItem(text),
	fIcon(NULL)
{
	BEntry appEntry = GetEntryFromSig(signature);
	if(appEntry.Exists())
	{
		entry_ref ref;
		appEntry.GetRef(&ref);
		fIconSize = 16;
		_GetIcon(ref);
	}
	else
	{
		fIconSize = 0;
	}
}


BitmapListItem::~BitmapListItem()
{
	delete fIcon;
}


void
BitmapListItem::_GetIcon(entry_ref entryRef)
{
	delete fIcon;
	fIcon = NULL;
	if(fIconSize == 0)
		return;
	BNode node;
	BNodeInfo nodeInfo;
	if (node.SetTo(&entryRef) == B_OK) {
		BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
		fIcon = new BBitmap(iconRect, 0, B_RGBA32);
		status_t result = BIconUtils::GetVectorIcon(&node, "BEOS:ICON", fIcon);
		if(result != B_OK)
		{
			// attempt to get mini or large icon
			delete fIcon;
			fIcon = NULL;
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
		}
	}
}


void
BitmapListItem::DrawItem(BView *owner, BRect item_rect, bool complete)
{
	float offset_width = 0, offset_height = 0;
	float listItemHeight = Height();

	if(IsSelected())
	{
		owner->SetHighColor(selected_color);
		owner->FillRect(item_rect);
		owner->SetHighColor(enabled_color);
	}

	// draw icon with the same offset for width and height
	if (fIcon) {
		offset_width = floor( (listItemHeight - fIconSize)/2) ;
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmap(fIcon, BPoint(item_rect.left + offset_width, item_rect.top + offset_width));
		owner->SetDrawingMode(B_OP_COPY);
		// calculate offsets for text
		offset_width += fIconSize + offset_width + 1;
		float iconSpacing = fIconSize + (2*kIconInset);
		if(iconSpacing > fTextHeight)
			offset_height += floor( (iconSpacing - fTextHeight)/2 );
	}

	// Draw text next to the icon
	BRect textRect(item_rect);
	textRect.left += offset_width;
	textRect.top += offset_height;
	BStringItem::DrawItem(owner, textRect, complete);
}


void
BitmapListItem::Update(BView *owner, const BFont *font)
{
	BStringItem::Update(owner, font);

	// Make sure there is enough space for the icon plus insets with smaller fonts
	fTextHeight = Height();
	float iconSpacing = fIconSize + (2*kIconInset);
	if( iconSpacing > fTextHeight )
		SetHeight(iconSpacing);
}


float
BitmapListItem::GetWidth(const BFont *font)
{
	float width = font->StringWidth(Text());
	if(fIcon)
		width += fIconSize + (2*kIconInset);
	return width;
}
