/* prefsWindow.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_WINDOW_H
#define EP_WINDOW_H

#include <InterfaceKit.h>
#include "AttrSettingsView.h"
#include "BitmapListItem.h"
#include "DeskbarSettingsView.h"
#include "EESettingsFile.h"
#include "EMaintenanceView.h"
#include "RelaunchSettingsView.h"
#include "RankingSettingsView.h"

#include "daemon_constants.h"
#include "engine_constants.h"


class prefsWindow : public BWindow {
public:
							prefsWindow(BRect);
//							~prefsWindow();
	virtual	bool			QuitRequested();
	virtual void			FrameResized(float width, float height);
	virtual void			MessageReceived(BMessage*);

private:
	engine_prefs			fEnginePrefs;
	BView					*fMainView, *fEmptySettingsView, *fCurrentView;
	BListView				*fPrefsListView;
	BitmapListItem			*fDaemonBLI, *fEngineBLI;
	BStringItem				*fAppLaunchSI, *fRankSI, *fAttrSI, *fDeskbarSI, *fMaintSI;
	RelaunchSettingsView	*fAppLaunchView;
	RankingSettingsView		*fRankingView;
	AttrSettingsView		*fAttrView;
	DeskbarSettingsView		*fDeskbarView;
	EMaintenanceView		*fMaintenanceView;
	void					_StoreSettings();
	void					_ReadSettings();
	void					_ReadEngineSettings();
	void					_WriteEngineSettings();
//	void					_ReadDaemonSettings();
//	void					_WriteDaemonSettings();
};

#endif