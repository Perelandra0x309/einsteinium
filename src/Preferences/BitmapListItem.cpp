/*BitmapListItem.h
	
*/
#include "BitmapListItem.h"

BitmapListItem::BitmapListItem(const uint8 *iconBits, BRect iconRect,
					const color_space colorSpace, char *text)
	:BStringItem(text)
	,iconSize(0)
	,icon(NULL)
{
	icon = new BBitmap(iconRect, colorSpace);
	icon->SetBits(iconBits, icon->BitsLength(), 0, colorSpace);
	iconSize = icon->Bounds().Height();
}

BitmapListItem::~BitmapListItem()
{
	delete icon;
}
void BitmapListItem::DrawItem(BView *owner, BRect item_rect, bool complete)
{
	float offset_width = 0, offset_height = 0;
	float listItemHeight = Height();
	
	// draw icon with the same offset for width and height
	if (icon) {
		offset_width = floor( (listItemHeight - iconSize)/2) ;
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmap(icon, BPoint(item_rect.left + offset_width, item_rect.top + offset_width));
		owner->SetDrawingMode(B_OP_COPY);
		// calculate offsets for text
		offset_width += iconSize + offset_width + 1;
		float iconSpacing = iconSize + (2*iconInset);
		if(iconSpacing > textHeight) {
			offset_height += floor( (iconSpacing - textHeight)/2 );
		}
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
void BitmapListItem::Update(BView *owner, const BFont *font)
{
	BStringItem::Update(owner, font);
	
	// Make sure there is enough space for the icon plus insets with smaller fonts
	textHeight = Height();
	float iconSpacing = iconSize + (2*iconInset);
	if( iconSpacing > textHeight )
	{
		SetHeight(iconSpacing);
	}
}
float BitmapListItem::GetWidth(const BFont *font)
{
	float width = 0;
	font_height fheight;
	font->GetHeight(&fheight);
	width = font->StringWidth(Text()) + fheight.ascent + fheight.descent + fheight.leading;
	// not perfect
	
	return width;
}
