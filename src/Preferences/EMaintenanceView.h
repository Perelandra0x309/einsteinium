/* EMaintenanceView.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_EMAINTENANCE_VIEW
#define EP_EMAINTENANCE_VIEW

#include <InterfaceKit.h>
#include <fs_attr.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include "engine_constants.h"
#include "methods.h"
#include "prefs_constants.h"
#include "SystemStatusBox.h"

class EMaintenanceView : public BView {
public:
					EMaintenanceView(BRect);
					~EMaintenanceView();
	virtual void	AllAttached();
	virtual void	MessageReceived(BMessage*);
	void			ResizeStatusBox();
private:
	bool			fWatchingRoster;
	BButton			*fRanksB, *fQuartilesB, *fDataB;
	BTextView		*fRanksTV, *fQuartilesTV, *fDataTV;
	BBox			*fRankBox;
	SystemStatusBox *fStatusBox;
	void			_SetButtonEnabledState();
};

#endif
