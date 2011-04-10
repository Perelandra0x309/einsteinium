/* LauncherDeskbarView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_L_DESKBAR_VIEW
#define EP_L_DESKBAR_VIEW

#include <InterfaceKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include "prefs_constants.h"
#include "stdlib.h"

class LauncherDeskbarView : public BView {
public:
					LauncherDeskbarView(BRect);
	virtual void	FrameResized(float width, float height);
	void			SetDeskbarCount(int count);
	int				GetDeskbarCount() { return strtol(fItemCountTC->Text(), NULL, 0); }
private:
	BTextControl	*fItemCountTC;
	BBox			*fDeskbarBox;
};

#endif
