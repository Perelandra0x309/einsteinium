/* MainWindow.h
 * Copyright 2013-2017 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_WINDOW_H
#define EINSTEINIUM_LAUNCHER_WINDOW_H


#include <Catalog.h>

#include <InterfaceKit.h>
#include "launcher_constants.h"
#include "MainView.h"

class MainWindow : public BWindow {
public:
							MainWindow(BRect, window_look look);
	virtual	bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
//	virtual void			FrameMoved(BPoint new_position);
	virtual void			FrameResized(float new_width, float new_height);
			void			SettingsChanged(uint32 what);
			void			SelectDefaultView();
			void			BuildAppsListView(BMessage *msg);
private:
	void					_RedrawInfoText();
	
	MainView*				fView;
	BStringView*			fInfoView;
	BMenuBar*				fMenuBar;
	BMenu*					fFileMenu;
	BMenu*					fSettingsMenu;
	BString					fInfoString;
};

#endif
