/* SuperTypeListItem.h
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_SUPERTYPELISTITEM_H
#define EINSTEINIUM_LAUNCHER_SUPERTYPELISTITEM_H

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <Roster.h>
#include <SupportKit.h>
#include "IconUtils.h"
#include "launcher_constants.h"


class SuperTypeListItem : public BListItem
{
public:
							SuperTypeListItem(BMimeType *type, AppSettings *settings);
							~SuperTypeListItem();
	virtual void			DrawItem(BView *owner, BRect item_rect, bool complete = false);
	virtual void			Update(BView *owner, const BFont *font);
//			void			ProcessMessage(BMessage*);
			void			SetIconSize(int value);
			void			SetName(const char *name) { fName.SetTo(name); }
			status_t		InitStatus() { return fInitStatus; }
private:
	status_t				fInitStatus;
	BMimeType				fMimeType;
	BString					fName;
	float					fFontHeight, fFontAscent, fTextOnlyHeight;
	BBitmap					*fIcon, *fShadowIcon;
	int						fIconSize;
	void					_GetIcon();
//	BBitmap*				_ConvertToGrayscale(const BBitmap* bitmap) const;
//	status_t				_StopService();
	void					_UpdateHeight(const BFont*);

};

#endif
