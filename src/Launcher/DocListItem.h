/* DocListItem.h
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_DOCLISTITEM_H
#define EINSTEINIUM_LAUNCHER_DOCLISTITEM_H

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <Roster.h>
#include <SupportKit.h>
#include "IconUtils.h"
#include "launcher_constants.h"


class DocListItem : public BListItem
{
public:
							DocListItem(entry_ref *entry, AppSettings *settings);
							~DocListItem();
	virtual void			DrawItem(BView *owner, BRect item_rect, bool complete = false);
	virtual void			Update(BView *owner, const BFont *font);
			void			ProcessMessage(BMessage*);
			status_t		Launch();
			status_t		ShowInTracker();
//			void			SetDrawTwoLines(bool value);
			void			SetIconSize(int value);
//			void			SetIconSize(int minIconSize, int maxIconSize, int totalCount, int index);
			const char*		GetSuperTypeName();
			status_t		InitStatus() { return fInitStatus; }
private:
	status_t				fInitStatus;
	entry_ref				fEntryRef;
	BString					fName;
	float					fFontHeight, fFontAscent, fTextOnlyHeight;
	BBitmap					*fIcon, *fShadowIcon;
//	bool					fDrawTwoLines;
	int						fIconSize;
	BMimeType				fSuperMimeType;
	void					_GetIcon();
//	BBitmap*				_ConvertToGrayscale(const BBitmap* bitmap) const;
	status_t				_OpenDoc();
	void					_UpdateHeight(const BFont*);

};

#endif
