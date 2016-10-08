/* prefsWindow.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_WINDOW_H
#define EP_WINDOW_H

#include <InterfaceKit.h>
#include "BitmapListItem.h"
#include "DaemonRelaunchView.h"
#include "DaemonStatusView.h"
#include "EngineMaintenanceView.h"
#include "EngineStatusView.h"
#include "LauncherAboutView.h"
#include "engine_constants.h"
#include "launcher_constants.h"


class prefsWindow : public BWindow {
public:
							prefsWindow(BRect);
							~prefsWindow();
	virtual	bool			QuitRequested();
	virtual void			FrameResized(float width, float height);
	virtual void			MessageReceived(BMessage*);

private:
	BView					*fMainView, *fEmptySettingsView, *fCurrentView;
	BBox					*fAboutBox;
	BTextView				*fAboutTextView;
	BStringView				*fCopyrightStringView;
	BListView				*fPrefsListView;
	BScrollView				*fPrefsScrollView;
	BitmapListItem			*fDaemonBLI, *fEngineBLI, *fLauncherBLI;
	BStringItem				*fDAppLaunchSI, *fEMaintSI;
	BList					fSettingsViews;
	DaemonRelaunchView		*fDRelaunchView;
	DaemonStatusView		*fDStatusView;
	LauncherAboutView		*fLAboutView;
	EngineMaintenanceView	*fMaintenanceView;
	EngineStatusView		*fEStatusView;
	AppRefFilter			*fAppFilter;
	BFilePanel				*fAppsPanel;
	void					_AddSettingsView(BListItem*, BView*);
	void					_ReadAllSettings();
	void					_ReadDaemonSettings();
};

#endif
