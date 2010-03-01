/*BitmapListItem.h
	
*/
#ifndef EP_BITMAPLISTITEM_H
#define EP_BITMAPLISTITEM_H

#include <InterfaceKit.h>
#include "prefs_constants.h"

class BitmapListItem : public BStringItem
{
public:
					BitmapListItem(const uint8 *iconBits, BRect iconRect,
									const color_space colorSpace, char *text);
	
	virtual			~BitmapListItem();
	virtual void	DrawItem(BView *owner, BRect item_rect, bool complete = false);
	virtual void	Update(BView *owner, const BFont *font);
	float			GetWidth(const BFont*);
private:
	static const float iconInset = 2;
	BBitmap			*icon;
	float			iconSize, textHeight;
};

#endif
