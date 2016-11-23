/* DaemonStatusView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_D_STATUS_VIEW
#define EP_D_STATUS_VIEW

#include <InterfaceKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <Catalog.h>
#include "methods.h"
#include "prefs_constants.h"
#include "SystemStatusBox.h"
#include "signatures.h"

class DaemonStatusView : public BView {
public:
					DaemonStatusView(BRect);
					~DaemonStatusView();
	virtual void	AttachedToWindow();
	virtual void	MessageReceived(BMessage*);
	virtual void	FrameResized(float width, float height);
private:
	BBox			*fAboutBox;
	BTextView		*fAboutTextView;
	BCheckBox		*fLaunchCB;
	bool			fWatchingRoster;
	SystemStatusBox *fStatusBox;
	BPath			fBootSettingsPath;
};

#endif
