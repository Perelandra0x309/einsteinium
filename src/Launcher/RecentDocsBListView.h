/* RecentDocsBListView.h
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_RECDOCSBLISTVIEW_H
#define EINSTEINIUM_LAUNCHER_RECDOCSBLISTVIEW_H

#include <InterfaceKit.h>
#include "launcher_constants.h"
#include "DocListItem.h"
#include "SuperTypeListItem.h"
#include "LPopUpMenu.h"
#include "SettingsWindow.h"


class RecentDocsBListView : public BOutlineListView {
public:
						RecentDocsBListView(BRect size);
//			void		AttachedToWindow();
	virtual void		MessageReceived(BMessage*);
	virtual void		MouseDown(BPoint pos);
	virtual void		KeyDown(const char* bytes, int32 numbytes);
//			bool		AddDoc(entry_ref *fileRef, AppSettings *settings);
			void		HandleMouseWheelChanged(BMessage *msg);
			void		SetShowing(bool showing) { isShowing = showing; };
			void		SettingsChanged(uint32 what, AppSettings settings);
			void		SetFontSizeForValue(float fontSize);
			void		BuildList(AppSettings *settings, bool force=false);
private:
	LPopUpMenu			*fMenu;
	BMenuItem			*fSettingsMI, *fTrackerMI;
	bool				isShowing;
	entry_ref			fLastRecentDocRef;
	SuperTypeListItem	*fGenericSuperItem;
	BMessage			fSuperListPointers;
	SuperTypeListItem*		_GetSuperItem(const char *mimeString, AppSettings *settings);
	SuperTypeListItem*		_GetGenericSuperItem(AppSettings *settings);
	virtual status_t	_InvokeSelectedItem();
	virtual void		_InitPopUpMenu(int32 selectedIndex);
	void				_HideApp(uint32 modifier=0);
};

#endif
