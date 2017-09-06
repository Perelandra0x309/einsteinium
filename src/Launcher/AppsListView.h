/* AppsListView.h
 * Copyright 2013-2017 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_APPLISTVIEW_H
#define EINSTEINIUM_LAUNCHER_APPLISTVIEW_H

#include <InterfaceKit.h>
#include <StringList.h>
#include <Catalog.h>
#include <fs_attr.h>
#include "launcher_constants.h"
#include "AppListItem.h"
#include "LPopUpMenu.h"
#include "SettingsWindow.h"
#include "EngineSubscriber.h"


class AppsListView : public BListView {
public:
						AppsListView(BRect size);
	virtual void		AttachedToWindow();
	virtual void		MessageReceived(BMessage*);
	virtual void		MouseDown(BPoint pos);
	virtual void		KeyDown(const char* bytes, int32 numbytes);
	void				Draw(BRect rect);
	void				FrameResized(float w, float h);
			void		SettingsChanged(uint32 what);
	virtual void		SelectionChanged();
			void		SendInfoViewUpdate();
			void		SetFontSizeForValue(float fontSize);
			void		SetIsShowing(bool _showing);
			bool		GetIsShowing() { return isShowing; };
			void		ScrollToNextAppBeginningWith(char letter);
			void		BuildAppsListView(BMessage *message);
			void		BuildAppsListFromRecent(bool force=false);

protected:
	BWindow				*fWindow;

private:
	LPopUpMenu			*fMenu;
	bool				isShowing;
	entry_ref			fLastRecentAppRef;
//	BMenuItem			*fStartStopMI, *fSettingsMI;
	BMenuItem			*fTrackerMI, *fRemoveMI;
	status_t			_AddAppListItem(BEntry appEntry, int totalCount, int index, AppSettings *settings);
	virtual status_t	_InvokeSelectedItem();
	virtual void		_InitPopUpMenu(int32 selectedIndex);
	void				_HideApp(uint32 modifier=0);
};

#endif
