/* MainView.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_MAIN_VIEW_H
#define EINSTEINIUM_LAUNCHER_MAIN_VIEW_H

#include <InterfaceKit.h>
#include <SupportKit.h>
#include <StorageKit.h>
#include <fs_attr.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <StringList.h>
#include "launcher_constants.h"
#include "AppsListView.h"
#include "RecentDocsBListView.h"
#include "RecentFoldersBListView.h"


class MainView : public BTabView {
public:
						MainView(BRect size);
//	virtual void		AllAttached();
//	virtual void		AttachedToWindow();
	virtual void		MessageReceived(BMessage*);
	virtual void		KeyDown(const char* bytes, int32 numbytes);
	virtual void		Select(int32 tab);
			void		SelectDefaultTab();
			void		SettingsChanged(uint32 what);
			void		BuildAppsListView(BMessage *message);
private:
	BTab					*fAppsTab, *fRecentDocsTab, *fRecentFoldersTab;
	BScrollView				*fAppsScrollView, *fDocsScrollView, *fFoldersScrollView;
	AppsListView			*fAppsListView;
	RecentDocsBListView		*fDocsListView;
	RecentFoldersBListView	*fFoldersListView;
	int32					fTabCount;
	BListView				*fSelectedListView;
	entry_ref				fLastRecentDocRef;
	void					_UpdateSelectedListView();
	void					_BuildAppsListViewFromRecent(bool force=false);
	void					_BuildDocsListView(bool force=false);
	void					_BuildFoldersListView(bool force=false);
};

#endif
