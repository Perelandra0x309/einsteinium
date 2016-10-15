/* einsteinium_daemon.h
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_DAEMON_H
#define EINSTEINIUM_DAEMON_H

#include <AppKit.h>
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <Catalog.h>
#include "daemon_constants.h"
#include "EDSettingsFile.h"
#include "RememberChoiceAlert.h"
#include "signatures.h"

class einsteinium_daemon : public BApplication {
public:
					einsteinium_daemon();
//					~einsteinium_daemon();
	virtual bool	QuitRequested();
	virtual void	ReadyToRun();
	virtual void	ArgvReceived(int32, char**);
	virtual void	MessageReceived(BMessage*);
private:
	EDSettingsFile	*fSettingsFile;
	bool			fWatchingRoster;//will be true when roster is being watched
	entry_ref		fTrueRef;
	status_t		fTrueRefStatus;
	BString			kSysAddonsPath, kSysBinPath, kSysPreferencesPath, kSysNonPackagedAddonsPath, kSysNonPackagedBinPath;
	BString			kUserAddonsPath, kUserBinPath, kUserPreferencesPath, kUserNonPackagedAddonsPath, kUserNonPackagedBinPath;
	BString			kPreferencesPath;
};

#endif
