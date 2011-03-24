/* LauncherDeskbarView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_L_DESKBAR_VIEW
#define EP_L_DESKBAR_VIEW

#include <AppKit.h>
#include <InterfaceKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include "LauncherSettingsFile.h"
#include "launcher_constants.h"
#include "prefs_constants.h"

class LauncherDeskbarView : public BView {
public:
					LauncherDeskbarView(BRect);
//					~LauncherDeskbarView();
//	virtual void	MessageReceived(BMessage*);
//	void			SetDeskbarValues(bool show, int count);
	void			SetDeskbarCount(int count);
	int				GetDeskbarCount() { return strtol(fItemCountTC->Text(), NULL, 0); }
private:
//	BCheckBox		*fShowDeskbarCB;
	BTextControl	*fItemCountTC;
	BBox			*fDeskbarBox;
};

#endif
