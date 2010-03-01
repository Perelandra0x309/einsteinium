/*prefs_app.h
	The main application definitions and objects
*/
#ifndef EINSTEINIUM_PREFERENCES_APP_H
#define EINSTEINIUM_PREFERENCES_APP_H
#include <Application.h>
#include "prefsWindow.h"

class prefs_app : public BApplication
{public:
					prefs_app();
					~prefs_app();
private:
	prefsWindow		*mainWindow;
};

#endif