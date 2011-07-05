/* BitmapListItem.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_BITMAPLISTITEM_H
#define EP_BITMAPLISTITEM_H

#include <InterfaceKit.h>
#include <IconUtils.h>
#include "methods.h"
#include "prefs_constants.h"

class BitmapListItem : public BStringItem {
public:
					BitmapListItem(const uint8 *iconBits, BRect iconRect,
									const color_space colorSpace, const char *text);
					BitmapListItem(const char *signature, const char *text);
	virtual			~BitmapListItem();
	virtual void	DrawItem(BView *owner, BRect item_rect, bool complete = false);
	virtual void	Update(BView *owner, const BFont *font);
	float			GetWidth(const BFont*);
private:
	static const float kIconInset = 2;
	BBitmap			*fIcon;
	float			fIconSize, fTextHeight;
	void			_GetIcon(entry_ref entryRef);
};

#endif
