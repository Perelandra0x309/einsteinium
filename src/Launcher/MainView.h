/* MainView.h
 * Copyright 2013-2017 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_MAIN_VIEW_H
#define EINSTEINIUM_LAUNCHER_MAIN_VIEW_H

#include <InterfaceKit.h>
#include <SupportKit.h>
#include <StorageKit.h>
#include <fs_attr.h>
#include <GridLayout.h>
#include <GridLayoutBuilder.h>
#include <StringList.h>
#include <Catalog.h>
#include "launcher_constants.h"
#include "AppsListView.h"
#include "RecentDocsBListView.h"
#include "RecentFoldersBListView.h"


class MainView : public BView {
public:
						MainView(BRect size);
	virtual void		MessageReceived(BMessage*);
	virtual void		Select(int32 which);
			void		SelectDefaultView();
			void		SelectedListViewChanged();
			void		SettingsChanged(uint32 what);
			void		BuildAppsListView(BMessage *message);
			void		UpdateInfoView();
			void		InvalidateFocusView();
private:
	BScrollView				*fAppsScrollView, *fDocsScrollView, *fFoldersScrollView;
	AppsListView			*fAppsListView;
	RecentDocsBListView		*fDocsListView;
	RecentFoldersBListView	*fFoldersListView;
	entry_ref				fLastRecentDocRef;
	void					_UpdateSelectedListView();
	void					_BuildAppsListViewFromRecent(bool force=false);
	void					_BuildDocsListView(bool force=false);
	void					_BuildFoldersListView(bool force=false);
};

#endif
