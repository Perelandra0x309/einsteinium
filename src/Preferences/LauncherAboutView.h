/* LauncherAboutView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_L_ABOUT_VIEW
#define EP_L_ABOUT_VIEW

#include <InterfaceKit.h>
#include <GroupLayout.h>
#include <LayoutBuilder.h>
#include "prefs_constants.h"

class LauncherAboutView : public BView {
public:
					LauncherAboutView(BRect);
	virtual void	AttachedToWindow();
	virtual void	FrameResized(float width, float height);
private:
	BBox			*fAboutBox, *fSettingsBox;
	BTextView		*fAboutTextView, *fSettingsTV;
	BButton			*fSettingsButton;
};

#endif
