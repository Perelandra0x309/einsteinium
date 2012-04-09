/* AppsListView.h
 * Copyright 2012 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EINSTEINIUM_LAUNCHER_APPLISTVIEW_H
#define EINSTEINIUM_LAUNCHER_APPLISTVIEW_H

#include <InterfaceKit.h>
#include "launcher_constants.h"
#include "AppListItem.h"
#include "LPopUpMenu.h"
#include "SettingsWindow.h"
#include "EngineSubscriber.h"


class AppsListView : public BListView {
public:
						AppsListView(BRect size);
//			void		AttachedToWindow();
	virtual void		MessageReceived(BMessage*);
	virtual void		MouseDown(BPoint pos);
	virtual void		KeyDown(const char* bytes, int32 numbytes);
			void		HandleMouseWheelChanged(BMessage *msg);
			void		SettingsChanged(uint32 what, AppSettings settings);
			void		SetFontSizeForValue(float fontSize);
			void		SetShowing(bool showing) { isShowing = showing; };
			void		ScrollToNextAppBeginningWith(char letter);

private:
	LPopUpMenu			*fMenu;
	bool				isShowing;
	BMenuItem			*fStartStopMI, *fTrackerMI, *fRemoveMI, *fSettingsMI;
	virtual status_t	_InvokeSelectedItem();
	virtual void		_InitPopUpMenu(int32 selectedIndex);
	void				_HideApp(uint32 modifier=0);
};

#endif
