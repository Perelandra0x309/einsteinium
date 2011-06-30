/* SystemStatusBox.h
 * Copyright 2011 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EP_SYSTEMSTATUS_BOX
#define EP_SYSTEMSTATUS_BOX

#include <InterfaceKit.h>
#include <Path.h>
#include <Roster.h>
#include <LayoutBuilder.h>
#include "prefs_constants.h"


enum {
	STATE_NONE = 0,
	STATE_OK_RUNNING,
	STATE_OK_STOPPING,
	STATE_OK_STOPPED,
	STATE_OK_STARTING,
	STATE_OK_RESTARTING,
	STATE_ERROR_STOPPING__IS_RUNNING,
	STATE_ERROR_STOPPING__IS_NOT_RUNNING,
	STATE_ERROR_STARTING__IS_RUNNING,
	STATE_ERROR_STARTING__IS_NOT_RUNNING
};

class SystemStatusBox : public BBox {
public:
							SystemStatusBox(const char *label, BEntry entry, const char * sig);
//							~SystemStatusBox();
			void			AttachedToWindow();
			void			ResizeStatusText();
	virtual void			MessageReceived(BMessage*);
	virtual	void			Draw(BRect updateRect){ BBox::Draw(updateRect); }
private:
	BStringView				*fStatusSV;
	BString					fSignature, fName, fStringViewText;
	BButton					*fRestartB, *fStartstopB;
	float					fUsedWidth;
	int						fCurrentState;
	status_t				_StopService();
	status_t				_StartService();
	void					_SetStatusText(const char*);
	void					_GetRunningState();
	void					_SetState(int);

};

#endif
