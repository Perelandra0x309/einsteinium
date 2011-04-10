/* LauncherAboutView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_L_ABOUT_VIEW
#define EP_L_ABOUT_VIEW

#include <InterfaceKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>

class LauncherAboutView : public BView {
public:
					LauncherAboutView(BRect);
	virtual void	AttachedToWindow();
	virtual void	FrameResized(float width, float height);
private:
	BBox			*fAboutBox;
	BTextView		*fAboutTextView;
};

#endif
