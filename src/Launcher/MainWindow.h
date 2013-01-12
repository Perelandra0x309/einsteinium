/* MainWindow.h
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_WINDOW_H
#define EINSTEINIUM_LAUNCHER_WINDOW_H

#include <InterfaceKit.h>
#include "launcher_constants.h"
#include "MainView.h"

class MainWindow : public BWindow {
public:
							MainWindow(BRect, AppSettings);
	virtual	bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
	virtual void			FrameMoved(BPoint new_position);
	virtual void			FrameResized(float new_width, float new_height);
			void			SettingsChanged(uint32 what, AppSettings settings);
			void			SelectDefaultTab();
			void			BuildAppsListView(BMessage *msg);
private:
	MainView				*fView;
	BStringView				*fInfoView;
//	BTextView				*fAboutTextView;
};

#endif
