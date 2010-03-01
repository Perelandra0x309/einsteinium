/*einsteinium_daemon.h
	Main daemon application object definitions
*/
#ifndef EINSTEINIUM_DAEMON_H
#define EINSTEINIUM_DAEMON_H

#include <AppKit.h>
#include <InterfaceKit.h>
#include <SupportKit.h>

#include "daemon_constants.h"
#include "EDSettingsFile.h"
#include "RememberChoiceAlert.h"

//Einsteinium Daemon is an inherited subclass of BApplication to
//enabled the use of BMessages to the system and other apps
class einsteinium_daemon : public BApplication
{public:
					einsteinium_daemon();
//					~einsteinium_daemon();
	virtual bool	QuitRequested();//Clean up data
	virtual void	ReadyToRun();
	virtual void	ArgvReceived(int32, char**);//command line options
	virtual void	MessageReceived(BMessage*);//received BMessages
private:
	EDSettingsFile	*settingsFile;
	bool			watchingRoster;//will be true when roster is being watched
};

#endif
