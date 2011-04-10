/* EngineMaintenanceView.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_E_MAINTENANCE_VIEW
#define EP_E_MAINTENANCE_VIEW

#include <InterfaceKit.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include "engine_constants.h"
#include "SystemStatusBox.h"

class EngineMaintenanceView : public BView {
public:
					EngineMaintenanceView(BRect);
					~EngineMaintenanceView();
	virtual void	AllAttached();
	virtual void	AttachedToWindow();
	virtual void	MessageReceived(BMessage*);
	virtual void	FrameResized(float width, float height);
private:
	bool			fWatchingRoster;
	BButton			*fDataB;
	BTextView		*fDataTV;
	BBox			*fMaintBox;
	void			_SetButtonEnabledState();
};

#endif
