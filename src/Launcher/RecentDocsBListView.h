/* RecentDocsBListView.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_RECDOCSBLISTVIEW_H
#define EINSTEINIUM_LAUNCHER_RECDOCSBLISTVIEW_H

#include <InterfaceKit.h>
#include <Catalog.h>
#include "launcher_constants.h"
#include "DocListItem.h"
#include "SuperTypeListItem.h"
#include "LPopUpMenu.h"
#include "SettingsWindow.h"


class RecentDocsBListView : public BOutlineListView {
public:
						RecentDocsBListView(BRect size);
	virtual void		AttachedToWindow();
	virtual void		MessageReceived(BMessage*);
	virtual void		MouseDown(BPoint pos);
	virtual void		KeyDown(const char* bytes, int32 numbytes);
	virtual void		SelectionChanged();
			void		SendInfoViewUpdate();
			void		HandleMouseWheelChanged(BMessage *msg);
			void		SetShowing(bool showing) { isShowing = showing; };
			void		SettingsChanged(uint32 what);
			void		SetFontSizeForValue(float fontSize);
			void		ScrollToNextDocBeginningWith(char letter);
			void		BuildList(bool force=false);
protected:
	BWindow				*fWindow;

private:
	LPopUpMenu			*fMenu;
	BMenuItem			*fSettingsMI, *fTrackerMI;
	bool				isShowing;
	entry_ref			fLastRecentDocRef;
	BMessage			fSuperListPointers;
	SuperTypeListItem	*fGenericSuperItem;
	SuperTypeListItem*		_GetSuperItem(const char *mimeString, int docIconSize);
	SuperTypeListItem*		_GetGenericSuperItem(int docIconSize);
	virtual status_t	_InvokeSelectedItem();
	virtual void		_InitPopUpMenu(int32 selectedIndex);
	void				_HideApp(uint32 modifier=0);
};

#endif
