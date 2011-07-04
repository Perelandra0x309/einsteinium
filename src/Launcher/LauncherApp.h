/* LauncherApp.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_APP_H
#define EINSTEINIUM_LAUNCHER_APP_H

#include <Application.h>

#include "launcher_constants.h"
#include "ELShelfView.h"

class LauncherApp : public BApplication {
public:
					LauncherApp();
	virtual void	ReadyToRun();
	virtual void	ArgvReceived(int32, char**);
private:
	void			_ShowShelfView(bool showShelfView);
	bool			fQuitRequested;
};

#endif