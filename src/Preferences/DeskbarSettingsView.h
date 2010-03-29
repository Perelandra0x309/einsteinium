/* DeskbarSettingsView.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_DESKBARSETTINGS_VIEW
#define EP_DESKBARSETTINGS_VIEW

#include <AppKit.h>
#include <InterfaceKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include "EESettingsFile.h"
#include "engine_constants.h"
#include "prefs_constants.h"

class DeskbarSettingsView : public BView {
public:
					DeskbarSettingsView(BRect);
//					~DeskbarSettingsView();
	virtual void	MessageReceived(BMessage*);
	void			SetDeskbarValues(bool show, int count);
private:
	BCheckBox		*fShowDeskbarCB;
	BTextControl	*fItemCountTC;
	BBox			*fDeskbarBox;
};

#endif
