/* AppMenuItem.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_APPMENUITEM_H
#define EINSTEINIUM_LAUNCHER_APPMENUITEM_H

#include <InterfaceKit.h>
#include <StorageKit.h>
#include "launcher_constants.h"


class AppMenuItem : public BMenuItem
{
public:
							AppMenuItem(entry_ref entry, BMessage *message);
							~AppMenuItem();
	virtual void			DrawContent();
	virtual void			GetContentSize(float *_width, float *_height);
			void			SetIconSize(int value);
			status_t		InitStatus() { return fInitStatus; }
private:
	status_t				fInitStatus;
	BMenu					*fSuper;
	entry_ref				fEntryRef;
	BString					fName;
	BBitmap					*fIcon;
	int						fIconSize;
	float					fOffsetWidth;
	font_height				fFontHeight;
	void					_GetIcon();

};

#endif
