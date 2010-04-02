/* EEShelfView.h
 * Copyright 2010 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef EE_SHELF_VIEW_H
#define EE_SHELF_VIEW_H


#include <Application.h>
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <Deskbar.h>
#include <Resources.h>
#include <Roster.h>
#include <stdio.h>
#include "engine_constants.h"
#include "IconMenuItem.h"

class _EXPORT EEShelfView : public BView {
public:
							EEShelfView(BRect frame, int16 count);
							EEShelfView(BMessage* data);
	virtual					~EEShelfView();

	virtual void			Draw(BRect rect);
	virtual void			AttachedToWindow();
	virtual void			DetachedFromWindow();
	static EEShelfView*		Instantiate(BMessage* data);
	virtual	status_t		Archive(BMessage* data, bool deep = true) const;
	virtual void	 		MouseDown(BPoint);
	virtual void			MessageReceived(BMessage* message);
//	virtual void			Pulse();

private:
	void					_BuildMenu(BMessage *message);
	BBitmap*				fIcon;
	BPopUpMenu*				fMenu;
	int16					fInitialCount;
	int32					fUniqueID;
};

#endif	/* EE_SHELF_VIEW_H */
