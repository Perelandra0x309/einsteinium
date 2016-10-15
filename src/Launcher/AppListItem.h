/* AppListItem.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_LISTITEM_H
#define EINSTEINIUM_LAUNCHER_LISTITEM_H

#include <InterfaceKit.h>
#include <StorageKit.h>
#include <Roster.h>
#include <Catalog.h>
#include "AppSettings.h"
#include "IconUtils.h"
#include "launcher_constants.h"


class AppListItem : public BListItem
{
public:
							AppListItem(BEntry entry, const char * sig, int iconSize);
							~AppListItem();
	virtual void			DrawItem(BView *owner, BRect item_rect, bool complete = false);
	virtual void			Update(BView *owner, const BFont *font);
			void			ProcessMessage(BMessage*);
	const	char*			GetSignature() { return fSignature.String(); }
	const	char*			GetName() { return fName.String(); }
			bool			IsRunning();
			status_t		Launch();
			status_t		ShowInTracker();
			BString			GetPath();
			bool			BeginsWith(char letter);
//			void			SetDrawTwoLines(bool value);
			void			SetIconSize(int value);
			void			SetIconSize(int minIconSize, int maxIconSize, int totalCount, int index);
			status_t		InitStatus() { return fInitStatus; }
private:
	status_t				fInitStatus;
	entry_ref				fEntryRef;
	BString					fSignature, fName, fRemoveName;
	char					fNameFirstChar;
	float					fFontHeight, fFontAscent, fTextOnlyHeight;
	BBitmap					*fIcon, *fShadowIcon;
//	bool					fDrawTwoLines;
	int						fIconSize;
	void					_GetIcon();
//	BBitmap*				_ConvertToGrayscale(const BBitmap* bitmap) const;
	status_t				_StopService();
	status_t				_StartService();
	void					_UpdateHeight(const BFont*);

};

#endif
