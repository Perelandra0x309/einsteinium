/* LauncherApp.h
 * Copyright 2013 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_APP_H
#define EINSTEINIUM_LAUNCHER_APP_H

#include <Application.h>
#include <Notification.h>
#include <Catalog.h>
#include "launcher_constants.h"
#include "ELShelfView.h"
#include "LauncherSettingsFile.h"
#include "MainWindow.h"
#include "SettingsWindow.h"

class LauncherApp : public BApplication, public EngineSubscriber {
public:
						LauncherApp();
	virtual bool		QuitRequested();
	virtual void		ReadyToRun();
//	virtual void		AppActivated(bool active);
	virtual void		ArgvReceived(int32, char**);
	virtual void		MessageReceived(BMessage*);
//	bool				GetIsEngineRunning() { return _IsEngineRunning(); };
	AppSettings*		GetAppSettings() { return &fAppSettings; };

protected:
	AppSettings				fAppSettings;

private:
	bool					fQuitRequested;
	MainWindow				*fWindow;
	SettingsWindow			*fSettingsWindow;
	LauncherSettingsFile	*fSettingsFile;
	void					_CreateExclusionsSignatureList(BMessage *exclusions);
	void					_ShowShelfView(bool showShelfView);
	void					_SaveSettingsToFile(uint32 what);
	void					_Subscribe();
	// virtual functions inherited from the EngineSubscriber class
	virtual void			_SubscribeFailed();
	virtual void			_SubscribeConfirmed();
	virtual void			_UpdateReceived(BMessage *message);
};

#endif
