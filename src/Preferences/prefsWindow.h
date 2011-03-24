/* prefsWindow.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_WINDOW_H
#define EP_WINDOW_H

#include <InterfaceKit.h>
#include "BitmapListItem.h"
#include "EMaintenanceView.h"
#include "LauncherDeskbarView.h"
#include "LauncherExclusionsView.h"
#include "LauncherRankingsView.h"
#include "RelaunchSettingsView.h"
#include "daemon_constants.h"
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
	scale_settings			fScales;
	BView					*fMainView, *fEmptySettingsView, *fCurrentView;
	BListView				*fPrefsListView;
	BitmapListItem			*fDaemonBLI, *fEngineBLI, *fLauncherBLI;
	BStringItem				*fDAppLaunchSI, *fLRankSI, *fLExclusionsSI, *fLDeskbarSI, *fEMaintSI;
	RelaunchSettingsView	*fAppLaunchView;
	LauncherRankingsView	*fLRankingsView;
	LauncherExclusionsView	*fLExclusionsView;
	LauncherDeskbarView		*fLDeskbarView;
	EMaintenanceView		*fMaintenanceView;
	LauncherSettingsFile	*fLauncherSettings;
	AppRefFilter			*fAppFilter;
	BFilePanel				*fAppsPanel;
//	void					_StoreSettings();
	void					_ReadAllSettings();
//	void					_ReadDaemonSettings();
//	void					_WriteDaemonSettings();
	void					_ReadLauncherSettings();
	void					_WriteLauncherScaleSettings();
	void					_WriteLauncherListInclusionSetting();
	void					_WriteLauncherDeskbarSettings();
	void					_WriteLauncherExclusions();
};

#endif
