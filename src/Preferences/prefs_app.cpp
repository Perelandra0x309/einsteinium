/* prefs_app.cpp
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "prefs_app.h"

prefs_app::prefs_app()
	:
	BApplication(app_sig)
{
	fMainWindow = new prefsWindow(BRect(200,50,800,500));
}


prefs_app::~prefs_app()
{	}
