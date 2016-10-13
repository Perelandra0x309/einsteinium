/* BitmapListItem.cpp
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "BitmapListItem.h"


BitmapListItem::BitmapListItem(const uint8 *iconBits, BRect iconRect,
					const color_space colorSpace, const char *text)
	:
	BListItem(),
	fText(text),
	fIcon(NULL)
{
	fIcon = new BBitmap(iconRect, colorSpace);
	fIcon->SetBits(iconBits, fIcon->BitsLength(), 0, colorSpace);
	fIconSize = fIcon->Bounds().Height();
}


BitmapListItem::BitmapListItem(const char *signature, const char *text)
	:
	BListItem(),
	fText(text),
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
	float offset_width = 0, offset_height = fFontAscent;
	float listItemHeight = Height();
	rgb_color backgroundColor;

	//background
	if(IsSelected()) {
		backgroundColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	}
	else {
		backgroundColor = ui_color(B_LIST_BACKGROUND_COLOR);
	}
	owner->SetHighColor(backgroundColor);
	owner->SetLowColor(backgroundColor);
	owner->FillRect(item_rect);

	//icon
	if (fIcon) {
		float offsetMarginHeight = floor( (listItemHeight - fIconSize)/2);
		float offsetMarginWidth = kIconInset;
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmap(fIcon, BPoint(item_rect.left + offsetMarginWidth,
							item_rect.top + offsetMarginHeight));
		owner->SetDrawingMode(B_OP_COPY);
		offset_width += fIconSize + 2*kIconInset;
	}
	// no icon, give the text some margin space
	else
	{
		offset_width += kTextInset;
	}

	//text
	if(listItemHeight > fTextOnlyHeight)
		offset_height += floor( (listItemHeight - fTextOnlyHeight)/2 );
			// center the text vertically
	BPoint cursor(item_rect.left + offset_width, item_rect.top + offset_height + kTextInset);
	if(IsSelected())
		owner->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
	else
		owner->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));
	owner->MovePenTo(cursor.x, cursor.y);
	owner->DrawString(fText);

}


void
BitmapListItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner, font);
	// Make sure there is enough space for the icon plus insets with smaller fonts
	_UpdateHeight(font);
}


void
BitmapListItem::_UpdateHeight(const BFont *font)
{
	font_height fontHeight;
	font->GetHeight(&fontHeight);
	fFontHeight = fontHeight.ascent + fontHeight.descent + fontHeight.leading;
	fFontAscent = fontHeight.ascent;
	fTextOnlyHeight = fFontHeight + 2*kTextInset;

	if(fIcon) {
		float iconSpacing = fIconSize + 2*kIconInset;
		if(iconSpacing > fTextOnlyHeight)
			SetHeight(iconSpacing);
		else
			SetHeight(fTextOnlyHeight);
	}
	else
		SetHeight(fTextOnlyHeight);
}


float
BitmapListItem::GetWidth(const BFont *font)
{
	float width = font->StringWidth(fText);
	if(fIcon)
		width += fIconSize + (2*kIconInset);
	return width;
}
