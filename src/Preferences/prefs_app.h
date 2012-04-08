/* prefs_app.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_PREFERENCES_APP_H
#define EINSTEINIUM_PREFERENCES_APP_H

#include <Application.h>
#include "prefsWindow.h"

class prefs_app : public BApplication {
public:
					prefs_app();
					~prefs_app();
//	virtual void	MessageReceived(BMessage*);
private:
	prefsWindow		*fMainWindow;
};

#endif