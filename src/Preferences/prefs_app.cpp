/*prefs_app.cpp
	create the main application and window
*/
#include "prefs_app.h"

prefs_app::prefs_app() :BApplication(app_sig)
{	mainWindow = new prefsWindow(BRect(200,50,800,500)); }
prefs_app::~prefs_app()
{	}
