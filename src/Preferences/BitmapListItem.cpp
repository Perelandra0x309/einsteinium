/* BitmapListItem.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "BitmapListItem.h"


BitmapListItem::BitmapListItem(const uint8 *iconBits, BRect iconRect,
					const color_space colorSpace, char *text)
	:
	BStringItem(text),
	fIconSize(0),
	fIcon(NULL)
{
	fIcon = new BBitmap(iconRect, colorSpace);
	fIcon->SetBits(iconBits, fIcon->BitsLength(), 0, colorSpace);
	fIconSize = fIcon->Bounds().Height();
}


BitmapListItem::~BitmapListItem()
{
	delete fIcon;
}


void
BitmapListItem::DrawItem(BView *owner, BRect item_rect, bool complete)
{
	float offset_width = 0, offset_height = 0;
	float listItemHeight = Height();

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

	BRect textRect(item_rect);
	textRect.left += offset_width;

	// If the text height is smaller than the icon, need to color background for selected items
	// before calling BStringItem::DrawItem
	if(IsSelected() && offset_height) {
		owner->SetHighColor(selected_color);
		owner->FillRect(textRect);
	}

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
	float width = 0;
	font_height fheight;
	font->GetHeight(&fheight);
	width = font->StringWidth(Text()) + fheight.ascent + fheight.descent + fheight.leading;
		// not perfect

	return width;
}
